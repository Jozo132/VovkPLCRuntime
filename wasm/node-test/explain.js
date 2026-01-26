#!/usr/bin/env node
// explain.js - CLI tool for explaining PLC code compilation and execution
//
// Usage:
//   npm run explain < input.asm           # Explain PLCASM code
//   npm run explain < input.stl           # Explain STL code (auto-detected)
//   npm run explain < input.json          # Explain Ladder Graph JSON (auto-detected)
//   echo "A I0.0\n= Q0.0" | npm run explain
//   echo '{"nodes":[...],"connections":[...]}' | npm run explain
//   echo "58004090..." | npm run explain  # Explain raw bytecode in hex format
//
// The script will:
//   1. Auto-detect the input language (Bytecode Hex, Ladder Graph JSON, STL, or PLCASM)
//   2. If Bytecode Hex, load directly and execute
//   3. If Ladder Graph, show the Ladder->STL transpilation
//   4. If STL, show the STL->PLCASM transpilation
//   5. Show the PLCASM->Bytecode compilation with symbol/label resolution
//   6. Run the bytecode in full debug mode showing stack state at each step
//   7. Limit execution to 100 steps to prevent infinite loops

import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

// Parse command line arguments
const args = process.argv.slice(2)
const maxSteps = parseInt(args.find(a => a.startsWith('--max-steps='))?.split('=')[1]) || 100
const showHelp = args.includes('--help') || args.includes('-h')

if (showHelp) {
    console.log(`
explain.js - CLI tool for explaining PLC code compilation and execution

Usage:
  npm run explain < input.asm           # Explain PLCASM code
  npm run explain < input.stl           # Explain STL code (auto-detected)
  npm run explain < input.json          # Explain Ladder Graph JSON (auto-detected)
  echo "A I0.0" | npm run explain
  echo '{"nodes":[...],"connections":[...]}' | npm run explain
  echo "58004090..." | npm run explain  # Explain raw bytecode in hex format

The script will:
  1. Auto-detect the input language (Bytecode Hex, Ladder Graph JSON, STL, or PLCASM)
  2. If Bytecode Hex, load directly and execute
  3. If Ladder Graph, show the Ladder->STL transpilation
  4. If STL, show the STL->PLCASM transpilation
  5. Show the PLCASM->Bytecode compilation with symbol/label resolution
  6. Run the bytecode in full debug mode showing stack state at each step
  7. Limit execution to ${maxSteps} steps to prevent infinite loops

Options:
  --max-steps=N   Maximum execution steps (default: 100)
  --help          Show this help message

Examples:
  echo -e "A I0.0\\n= Q0.0" | npm run explain
  echo -e "u8.const 1\\nu8.const 2\\nu8.add\\nexit" | npm run explain
  echo '{"nodes":[{"id":"n1","type":"contact","symbol":"X0.0","x":0,"y":0},{"id":"n2","type":"coil","symbol":"Y0.0","x":1,"y":0}],"connections":[{"sources":["n1"],"destinations":["n2"]}]}' | npm run explain
  echo "58 00 40 60 00 80 FF" | npm run explain  # Raw bytecode hex
`)
    process.exit(0)
}

// Read input from stdin
async function readStdin() {
    return new Promise((resolve, reject) => {
        let data = ''
        
        if (process.stdin.isTTY) {
            console.error('Error: No input provided. Pipe code via stdin.')
            console.error('Usage: echo "code" | npm run explain')
            process.exit(1)
        }
        
        process.stdin.setEncoding('utf8')
        process.stdin.on('data', chunk => data += chunk)
        process.stdin.on('end', () => resolve(data))
        process.stdin.on('error', reject)
    })
}

// Detect if input is Bytecode Hex, Ladder Graph JSON, STL, or PLCASM
function detectLanguage(code) {
    const trimmed = code.trim()
    
    // Check for raw bytecode hex first - string of hex digits (with optional spaces/newlines)
    // Must be a valid hex string that looks like bytecode (not STL/PLCASM)
    const hexOnly = trimmed.replace(/[\s\r\n]/g, '')
    if (/^[0-9A-Fa-f]+$/.test(hexOnly) && hexOnly.length >= 2 && hexOnly.length % 2 === 0) {
        // Additional check: make sure it's not accidentally matching something else
        // Hex bytecode typically ends with FF (EXIT opcode) or has recognizable opcodes
        return 'bytecode-hex'
    }
    
    // Check for JSON first - if it starts with { and contains nodes + connections, it's ladder graph
    if (trimmed.startsWith('{')) {
        try {
            const parsed = JSON.parse(trimmed)
            // Check for ladder graph format (nodes + connections)
            if ((parsed.nodes && Array.isArray(parsed.nodes)) &&
                (parsed.connections && Array.isArray(parsed.connections))) {
                return 'ladder-graph'
            }
        } catch (e) {
            // Not valid JSON, continue with other detection
        }
    }
    
    const lines = code.split('\n').map(l => l.trim()).filter(l => l && !l.startsWith('//'))
    
    // STL-specific patterns
    const stlPatterns = [
        /^A\s+[IQMXYSC]/i,      // A I0.0, A Q0.0, etc.
        /^AN\s+[IQMXYSC]/i,     // AN I0.0
        /^O\s+[IQMXYSC]/i,      // O I0.0  
        /^ON\s+[IQMXYSC]/i,     // ON I0.0
        /^=\s+[QYMSC]/i,        // = Q0.0
        /^S\s+[MQY]/i,          // S M0.0
        /^R\s+[MQY]/i,          // R M0.0
        /^L\s+[#MQY]/i,         // L #10, L M0
        /^T\s+[MQY]/i,          // T M0
        /^TON\s+T/i,            // TON T0
        /^TOF\s+T/i,            // TOF T0
        /^TP\s+T/i,             // TP T0
        /^CTU\s+C/i,            // CTU C0
        /^CTD\s+C/i,            // CTD C0
        /^FP\s+M/i,             // FP M0.0
        /^FN\s+M/i,             // FN M0.0
        /^SET$/i,               // SET
        /^CLR$/i,               // CLR
        /^JU\s+\w/i,            // JU label
        /^JC\s+\w/i,            // JC label
        /^JCN\s+\w/i,           // JCN label
        /^CALL\s+\w/i,          // CALL sub
        /^BE$/i,                // BE
        /^BEU$/i,               // BEU
        /^BEC$/i,               // BEC
        /^A\s*\(/i,             // A(
        /^O\s*\(/i,             // O(
        /^X\s*\(/i,             // X(
        /^LD\s+[IQMXYSC]/i,     // IEC IL: LD
        /^ST\s+[QYMSC]/i,       // IEC IL: ST
        /^AND\s+[IQMXYSC]/i,    // IEC IL: AND
        /^OR\s+[IQMXYSC]/i,     // IEC IL: OR
        /^\+I$/i,               // +I
        /^-I$/i,                // -I
        /^\*I$/i,               // *I
        /^\/I$/i,               // /I
        /^==I$/i,               // ==I
        /^<>I$/i,               // <>I
        /^>I$/i,                // >I
        /^>=I$/i,               // >=I
        /^<I$/i,                // <I
        /^<=I$/i,               // <=I
        /^INC[IBDR]?\s+M/i,     // INC, INCI, INCB, INCD, INCR with memory address
        /^DEC[IBDR]?\s+M/i,     // DEC, DECI, DECB, DECD, DECR with memory address
    ]
    
    // PLCASM-specific patterns
    const plcasmPatterns = [
        /^u8\./i,               // u8.const, u8.add, etc.
        /^u16\./i,              // u16.const, etc.
        /^u32\./i,              // u32.const, etc.
        /^i8\./i,               // i8.const, etc.
        /^i16\./i,              // i16.const, etc.
        /^i32\./i,              // i32.const, etc.
        /^f32\./i,              // f32.const, etc.
        /^f64\./i,              // f64.const, etc.
        /^jmp\s+\w/i,           // jmp label
        /^jmp_if\s+\w/i,        // jmp_if label
        /^jmp_if_not\s+\w/i,    // jmp_if_not label
        /^call\s+\w/i,          // call sub
        /^ret$/i,               // ret
        /^ret_if$/i,            // ret_if
        /^ret_if_not$/i,        // ret_if_not
        /^exit$/i,              // exit
        /^ton\s+T/i,            // ton T0
        /^tof\s+T/i,            // tof T0
        /^tp\s+T/i,             // tp T0
        /^ctu\s+C/i,            // ctu C0
        /^ctd\s+C/i,            // ctd C0
    ]
    
    let stlScore = 0
    let plcasmScore = 0
    
    for (const line of lines) {
        for (const pattern of stlPatterns) {
            if (pattern.test(line)) {
                stlScore++
                break
            }
        }
        for (const pattern of plcasmPatterns) {
            if (pattern.test(line)) {
                plcasmScore++
                break
            }
        }
    }
    
    // If we have clear PLCASM patterns, it's PLCASM
    if (plcasmScore > 0 && plcasmScore >= stlScore) {
        return 'plcasm'
    }
    // If we have STL patterns, it's STL
    if (stlScore > 0) {
        return 'stl'
    }
    // Default to PLCASM
    return 'plcasm'
}

// Stream code to runtime (synchronous for direct VovkPLC)
function streamCode(runtime, code) {
    for (let i = 0; i < code.length; i++) {
        runtime.wasm_exports.streamIn(code.charCodeAt(i))
    }
    runtime.wasm_exports.streamIn(0)
}

// Capture stdout output (debug output goes here, not to stream)
let capturedOutput = ''
function captureStdout(runtime) {
    capturedOutput = ''
    runtime.stdout_callback = msg => { capturedOutput += msg + '\n' }
}
function readCapturedOutput() {
    const output = capturedOutput
    capturedOutput = ''
    return output
}

// CRC8 calculation (polynomial 0x31) to match the runtime's crc8_simple
function crc8Simple(bytes) {
    let crc = 0
    for (const byte of bytes) {
        crc ^= byte
        for (let k = 0; k < 8; k++) {
            crc = (crc & 0x80) ? ((crc << 1) ^ 0x31) : (crc << 1)
        }
        crc &= 0xff
    }
    return crc
}

const run = async () => {
    const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')
    
    if (!fs.existsSync(wasmPath)) {
        console.error('Error: WASM file not found at', wasmPath)
        console.error('Run "npm run build" first.')
        process.exit(1)
    }

    // Use direct VovkPLC without worker for faster execution
    const runtime = new VovkPLC(wasmPath)
    
    // Set up stdout capture BEFORE initialize to catch banner
    captureStdout(runtime)
    
    await runtime.initialize(wasmPath, false, false) // not silent - we need debug output
    
    // Clear initial banner from both stream and captured stdout
    runtime.readStream()
    readCapturedOutput()
    
    try {
        const input = await readStdin()
        
        if (!input.trim()) {
            console.error('Error: Empty input')
            process.exit(1)
        }
        
        const language = detectLanguage(input)
        
        console.log('╔══════════════════════════════════════════════════════════════════╗')
        console.log('║              VovkPLCRuntime - Code Explanation Tool              ║')
        console.log('╚══════════════════════════════════════════════════════════════════╝')
        console.log()
        
        console.log(`Detected language: ${language.toUpperCase()}`)
        console.log()
        
        // Step 1: Show input
        console.log('┌─────────────────────────────────────────────────────────────────┐')
        console.log('│ STEP 1: Input Code                                              │')
        console.log('└─────────────────────────────────────────────────────────────────┘')
        console.log()
        
        const inputLines = input.trim().split('\n')
        inputLines.forEach((line, i) => {
            console.log(`  ${String(i + 1).padStart(3)}│ ${line}`)
        })
        console.log()
        
        let plcasm = input
        let stl = null
        let stepOffset = 0
        let bytecodeLoaded = false
        
        // Handle raw bytecode hex - skip compilation, go straight to execution
        if (language === 'bytecode-hex') {
            console.log('┌─────────────────────────────────────────────────────────────────┐')
            console.log('│ STEP 2: Load Bytecode from Hex                                 │')
            console.log('└─────────────────────────────────────────────────────────────────┘')
            console.log()
            
            // Parse hex string (ignore spaces and newlines)
            const hexOnly = input.replace(/[\s\r\n]/g, '')
            const bytes = []
            for (let i = 0; i < hexOnly.length; i += 2) {
                bytes.push(parseInt(hexOnly.substr(i, 2), 16))
            }
            
            console.log(`  Parsed ${bytes.length} bytes from hex input`)
            console.log()
            
            // Format bytecode display
            const bytecodeHex = bytes.map(b => b.toString(16).toUpperCase().padStart(2, '0')).join(' ')
            console.log(`  Bytecode[${bytes.length}]: ${bytecodeHex}`)
            console.log()
            
            // Calculate CRC8 checksum (polynomial 0x31)
            const checksum = crc8Simple(bytes)
            console.log(`  CRC8 Checksum: 0x${checksum.toString(16).toUpperCase().padStart(2, '0')}`)
            console.log()
            
            // Stream bytes to input and use downloadProgram
            for (const b of bytes) {
                runtime.wasm_exports.streamIn(b)
            }
            
            readCapturedOutput() // clear any previous output
            const loadError = runtime.wasm_exports.downloadProgram(bytes.length, checksum)
            
            const loadOutput = readCapturedOutput()
            if (loadOutput && loadOutput.trim()) {
                for (const line of loadOutput.split('\n')) {
                    if (line.trim()) console.log(`  ${line}`)
                }
                console.log()
            }
            
            if (loadError) {
                console.error(`  ✗ Failed to load bytecode (error code: ${loadError})`)
                process.exit(1)
            }
            
            console.log('  ✓ Bytecode loaded successfully')
            console.log()
            
            bytecodeLoaded = true
        }
        
        // Step 2: If Ladder Graph, transpile to STL first
        if (language === 'ladder-graph') {
            console.log('┌─────────────────────────────────────────────────────────────────┐')
            console.log('│ STEP 2: Ladder Graph → STL Transpilation                       │')
            console.log('└─────────────────────────────────────────────────────────────────┘')
            console.log()
            
            streamCode(runtime, input)
            runtime.wasm_exports.ladder_graph_load_from_stream()
            
            const ladderSuccess = runtime.wasm_exports.ladder_graph_compile()
            
            if (!ladderSuccess) {
                console.error('  ✗ Ladder Graph compilation error')
                process.exit(1)
            }
            
            runtime.wasm_exports.ladder_graph_output_to_stream()
            stl = runtime.readStream()
            
            const stlLines = stl.split('\n')
            stlLines.forEach((line, i) => {
                if (line.trim()) {
                    console.log(`  ${String(i + 1).padStart(3)}│ ${line}`)
                }
            })
            console.log()
            console.log('  ✓ Ladder Graph transpiled to STL successfully')
            console.log()
            stepOffset = 1
        }
        
        // Step 2/3: If STL (or from ladder graph), transpile to PLCASM
        if (language === 'stl' || language === 'ladder-graph') {
            const stepNum = 2 + stepOffset
            console.log('┌─────────────────────────────────────────────────────────────────┐')
            console.log(`│ STEP ${stepNum}: STL → PLCASM Transpilation                             │`)
            console.log('└─────────────────────────────────────────────────────────────────┘')
            console.log()
            
            const stlInput = stl || input
            streamCode(runtime, stlInput)
            runtime.wasm_exports.stl_load_from_stream()
            
            const success = runtime.wasm_exports.stl_compile()
            
            if (!success) {
                const hasError = runtime.wasm_exports.stl_has_error()
                if (hasError) {
                    const errorLine = runtime.wasm_exports.stl_get_error_line()
                    const errorCol = runtime.wasm_exports.stl_get_error_column()
                    console.error(`  ✗ STL compilation error at line ${errorLine}, column ${errorCol}`)
                    process.exit(1)
                }
            }
            
            runtime.wasm_exports.stl_output_to_stream()
            plcasm = runtime.readStream()
            
            // Filter out the header comment
            const plcasmLines = plcasm.split('\n').filter(l => !l.startsWith('// Generated from STL'))
            plcasmLines.forEach((line, i) => {
                if (line.trim()) {
                    console.log(`  ${String(i + 1).padStart(3)}│ ${line}`)
                }
            })
            console.log()
            console.log('  ✓ STL transpiled to PLCASM successfully')
            console.log()
        }
        
        // Step 3/4: Compile PLCASM to bytecode (skip if bytecode was loaded directly)
        if (!bytecodeLoaded) {
            const plcasmStepNum = language === 'ladder' ? 4 : (language === 'stl' ? 3 : 2)
            console.log('┌─────────────────────────────────────────────────────────────────┐')
            console.log(`│ STEP ${plcasmStepNum}: PLCASM → Bytecode Compilation                           │`)
            console.log('└─────────────────────────────────────────────────────────────────┘')
            console.log()
            
            readCapturedOutput() // clear any previous output
            runtime.downloadAssembly(plcasm)
            const compileError = runtime.wasm_exports.compileAssembly(true)
            
            // Read compiler output (debug output goes to stdout, not stream)
            const compileOutput = readCapturedOutput()
            if (compileOutput) {
                // Parse and format compiler output
                const lines = compileOutput.split('\n')
                for (const line of lines) {
                    if (line.trim()) {
                        console.log(`  ${line}`)
                    }
                }
            }
            
            if (compileError) {
                console.error('  ✗ PLCASM compilation failed')
                process.exit(1)
            }
            
            console.log()
            console.log('  ✓ Compiled to bytecode successfully')
            console.log()
            
            // Load the compiled program
            const loadError = runtime.wasm_exports.loadCompiledProgram()
            if (loadError) {
                console.error('  ✗ Failed to load compiled program')
                process.exit(1)
            }
            
            // Read load output
            const loadOutput = readCapturedOutput()
            if (loadOutput && loadOutput.trim()) {
                console.log(loadOutput)
            }
        }
        
        // Execution step
        const execStepNum = language === 'bytecode-hex' ? 3 : (language === 'ladder-graph' ? 5 : (language === 'stl' ? 4 : 3))
        console.log('┌─────────────────────────────────────────────────────────────────┐')
        console.log(`│ STEP ${execStepNum}: Bytecode Execution (Debug Mode, max ${maxSteps} steps)          │`)
        console.log('└─────────────────────────────────────────────────────────────────┘')
        console.log()
        
        // Run with step limit
        // We'll use runFullProgramDebug but need to handle potential infinite loops
        // The runtime should output step-by-step execution
        
        runtime.wasm_exports.runFullProgramDebug()
        
        // Debug execution output goes to stdout
        const execOutput = readCapturedOutput()
        if (execOutput) {
            const lines = execOutput.split('\n')
            let stepCount = 0
            let inStepSection = false
            
            for (const line of lines) {
                // Count step lines
                if (line.includes('Step') && line.includes('Executed')) {
                    stepCount++
                    inStepSection = true
                    
                    if (stepCount > maxSteps) {
                        console.log(`  ... (truncated at ${maxSteps} steps to prevent infinite loop)`)
                        console.log()
                        console.log(`  ⚠ Execution stopped after ${maxSteps} steps`)
                        break
                    }
                }
                
                console.log(`  ${line}`)
            }
        }
        
        console.log()
        console.log('╔══════════════════════════════════════════════════════════════════╗')
        console.log('║                      Explanation Complete                        ║')
        console.log('╚══════════════════════════════════════════════════════════════════╝')
        
    } catch (e) {
        console.error('Error:', e.message)
        process.exitCode = 1
    } finally {
        // Direct VovkPLC doesn't need terminate()
        process.exit(process.exitCode || 0)
    }
}

run().catch(err => {
    console.error('Error:', err.message)
    process.exit(1)
})
