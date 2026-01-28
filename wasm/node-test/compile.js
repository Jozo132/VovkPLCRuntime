#!/usr/bin/env node
// compile.js - CLI tool for compiling PLCASM to bytecode, STL to PLCASM, or Ladder Graph to STL
//
// Usage:
//   npm run compile < input.asm           # Compile PLCASM, output raw bytecode
//   npm run compile --stl < input.stl     # Compile STL, output PLCASM
//   npm run compile --ladder < input.json # Compile Ladder Graph JSON, output STL
//   echo "u8.const 1\nexit" | npm run compile
//   echo "A I0.0\n= Q0.0" | npm run compile --stl
//   echo '{"nodes":[...],"connections":[...]}' | npm run compile --ladder
//
// Options:
//   --stl     Input is STL code, output PLCASM instead of bytecode
//   --ladder  Input is Ladder Graph JSON, output STL
//   --hex     Output bytecode as hex string instead of raw bytes
//   --help    Show this help message

import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

// Parse command line arguments
const args = process.argv.slice(2)
const isSTL = args.includes('--stl')
const isLadder = args.includes('--ladder')
const isHex = args.includes('--hex')
const showHelp = args.includes('--help') || args.includes('-h')

if (showHelp) {
    console.log(`
compile.js - CLI tool for compiling PLCASM to bytecode, STL to PLCASM, or Ladder Graph to STL

Usage:
  npm run compile < input.asm           # Compile PLCASM, output raw bytecode
  npm run compile --stl < input.stl     # Compile STL, output PLCASM
  npm run compile --ladder < input.json # Compile Ladder Graph JSON, output STL
  echo "u8.const 1" | npm run compile
  echo "A I0.0" | npm run compile --stl

Options:
  --stl     Input is STL code, output PLCASM instead of bytecode
  --ladder  Input is Ladder Graph JSON, output STL
  --hex     Output bytecode as hex string instead of raw bytes
  --help    Show this help message

Examples:
  # Compile PLCASM and get raw bytecode
  echo -e "u8.const 42\\nexit" | npm run compile > output.bin

  # Compile PLCASM and get hex bytecode  
  echo -e "u8.const 42\\nexit" | npm run compile -- --hex

  # Convert STL to PLCASM
  echo -e "A I0.0\\n= Q0.0" | npm run compile -- --stl
  
  # Convert Ladder Graph JSON to STL
  echo '{"nodes":[{"id":"n1","type":"contact","symbol":"X0.0","x":0,"y":0}],"connections":[]}' | npm run compile -- --ladder
`)
    process.exit(0)
}

// Read input from stdin
async function readStdin() {
    return new Promise((resolve, reject) => {
        let data = ''
        
        // Check if stdin is a TTY (interactive terminal)
        if (process.stdin.isTTY) {
            console.error('Error: No input provided. Pipe code via stdin.')
            console.error('Usage: echo "code" | npm run compile [--stl] [--hex]')
            process.exit(1)
        }
        
        process.stdin.setEncoding('utf8')
        process.stdin.on('data', chunk => data += chunk)
        process.stdin.on('end', () => resolve(data))
        process.stdin.on('error', reject)
    })
}

// Stream code to runtime
async function streamCode(runtime, code) {
    for (let i = 0; i < code.length; i++) {
        await runtime.callExport('streamIn', code.charCodeAt(i))
    }
    await runtime.callExport('streamIn', 0) // null terminator
}

const run = async () => {
    const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')
    
    if (!fs.existsSync(wasmPath)) {
        console.error('Error: WASM file not found at', wasmPath)
        console.error('Run "npm run build" first.')
        process.exit(1)
    }

    // Create runtime in silent mode to suppress banners
    const runtime = await VovkPLC.createWorker(wasmPath, { silent: true })
    
    // Clear any residual output
    await runtime.readStream()
    
    try {
        const input = await readStdin()
        
        if (!input.trim()) {
            console.error('Error: Empty input')
            process.exit(1)
        }
        
        if (isLadder) {
            // Ladder mode: compile Ladder Graph JSON to STL and output STL
            await streamCode(runtime, input)
            await runtime.callExport('ladder_standalone_load_stream')
            
            const success = await runtime.callExport('ladder_standalone_compile')
            
            if (!success) {
                console.error('Ladder Graph compilation error')
                process.exit(1)
            }
            
            // Output STL to stream and read it
            await runtime.callExport('ladder_standalone_output_to_stream')
            const stl = await runtime.readOutBuffer()
            
            // Output STL to stdout
            process.stdout.write(stl)
            
        } else if (isSTL) {
            // STL mode: compile STL to PLCASM and output PLCASM
            await streamCode(runtime, input)
            await runtime.callExport('stl_load_from_stream')
            
            const success = await runtime.callExport('stl_compile')
            
            if (!success) {
                const hasError = await runtime.callExport('stl_has_error')
                if (hasError) {
                    const errorLine = await runtime.callExport('stl_get_error_line')
                    const errorCol = await runtime.callExport('stl_get_error_column')
                    console.error(`STL compilation error at line ${errorLine}, column ${errorCol}`)
                    process.exit(1)
                }
            }
            
            // Output PLCASM to stream and read it
            await runtime.callExport('stl_output_to_stream')
            const plcasm = await runtime.readOutBuffer()
            
            // Output PLCASM to stdout
            process.stdout.write(plcasm)
            
        } else {
            // PLCASM mode: compile to bytecode and output bytecode
            await runtime.downloadAssembly(input)
            
            const compileError = await runtime.callExport('compileAssembly', false)
            
            // Clear compiler output stream
            await runtime.readStream()
            
            if (compileError) {
                console.error('PLCASM compilation failed')
                process.exit(1)
            }
            
            // Get the compiled bytecode
            const loadError = await runtime.callExport('loadCompiledProgram')
            
            // Clear load output stream
            await runtime.readStream()
            
            if (loadError) {
                console.error('Failed to load compiled program')
                process.exit(1)
            }
            
            // Extract bytecode
            const extracted = await runtime.extractProgram()
            
            if (extracted.size === 0) {
                console.error('Compilation produced empty bytecode')
                process.exit(1)
            }
            
            // Parse hex string to bytes
            const hexStr = extracted.output.trim()
            const bytes = []
            for (let i = 0; i < hexStr.length; i += 3) {
                const hex = hexStr.substring(i, i + 2)
                if (hex.length === 2) {
                    bytes.push(parseInt(hex, 16))
                }
            }
            
            if (isHex) {
                // Output as hex string
                process.stdout.write(bytes.map(b => b.toString(16).padStart(2, '0')).join(' ') + '\n')
            } else {
                // Output as raw bytes
                process.stdout.write(Buffer.from(bytes))
            }
        }
        
    } catch (e) {
        console.error('Error:', e.message)
        process.exitCode = 1
    } finally {
        await runtime.terminate()
        await new Promise(res => setTimeout(res, 100))
        process.exit(process.exitCode || 0)
    }
}

run().catch(err => {
    console.error('Error:', err.message)
    process.exit(1)
})
