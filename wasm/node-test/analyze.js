#!/usr/bin/env node
// analyze.js - CLI tool for WCET (Worst-Case Execution Time) analysis
//
// Usage:
//   npm run analyze < input.asm           # Analyze PLCASM code
//   npm run analyze < input.stl           # Analyze STL code (auto-detected)
//   npm run analyze < input.json          # Analyze Ladder Graph JSON (auto-detected)
//   echo "u8.const 10\nu8.const 20\nu8.add\nexit" | npm run analyze
//   echo "A I0.0\n= Q0.0" | npm run analyze
//   echo "03 0A 03 14 20 03 FF" | npm run analyze  # Analyze raw bytecode hex
//
// The script will:
//   1. Auto-detect the input language (Bytecode Hex, Ladder Graph JSON, PLCScript, STL, or PLCASM)
//   2. Compile to bytecode as needed
//   3. Run WCET static analysis on the resulting bytecode
//   4. Print the formatted analysis report
//   5. Output the full JSON report
//
// Options:
//   --json         Output only JSON report (no human-readable report)
//   --silent       Suppress compilation output
//   --help         Show this help message

import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

// Parse command line arguments
const args = process.argv.slice(2)
const jsonOnly = args.includes('--json')
const silent = args.includes('--silent')
const showHelp = args.includes('--help') || args.includes('-h')

if (showHelp) {
    console.log(`
analyze.js - WCET (Worst-Case Execution Time) analysis tool

Usage:
  npm run analyze < input.asm           # Analyze PLCASM code
  npm run analyze < input.stl           # Analyze STL code (auto-detected)
  npm run analyze < input.json          # Analyze Ladder Graph JSON (auto-detected)
  echo "u8.const 10" | npm run analyze
  echo "A I0.0" | npm run analyze
  echo "03 0A 03 14 20 03 FF" | npm run analyze  # Raw bytecode hex

Options:
  --json         Output only JSON report (no human-readable report)
  --silent       Suppress compilation output
  --help         Show this help message
`)
    process.exit(0)
}

// Read input from stdin
async function readStdin() {
    return new Promise((resolve, reject) => {
        let data = ''
        if (process.stdin.isTTY) {
            console.error('Error: No input provided. Pipe code via stdin.')
            console.error('Usage: echo "u8.const 10\\nu8.const 20\\nu8.add\\nexit" | npm run analyze')
            process.exit(1)
        }
        process.stdin.setEncoding('utf8')
        process.stdin.on('data', chunk => data += chunk)
        process.stdin.on('end', () => resolve(data))
        process.stdin.on('error', reject)
    })
}

// Detect input language (same as explain.js)
function detectLanguage(code) {
    const trimmed = code.trim()
    if (/^(VOVKPLC)?PROJECT\s+\w+/i.test(trimmed)) return 'project'
    const hexOnly = trimmed.replace(/[\s\r\n]/g, '')
    if (/^[0-9A-Fa-f]+$/.test(hexOnly) && hexOnly.length >= 2 && hexOnly.length % 2 === 0) return 'bytecode-hex'
    if (trimmed.startsWith('{')) {
        try {
            const parsed = JSON.parse(trimmed)
            if (parsed.nodes && Array.isArray(parsed.nodes) && parsed.connections && Array.isArray(parsed.connections)) return 'ladder-graph'
        } catch (e) { /* not JSON */ }
    }
    const lines = code.split('\n').map(l => l.trim()).filter(l => l && !l.startsWith('//'))
    const stlPatterns = [/^A\s+[IQMXYSC]/i, /^AN\s+[IQMXYSC]/i, /^O\s+[IQMXYSC]/i, /^ON\s+[IQMXYSC]/i, /^=\s+[QYMSC]/i, /^S\s+[MQY]/i, /^R\s+[MQY]/i, /^L\s+[#MQY]/i, /^T\s+[MQY]/i, /^TON\s+T/i, /^TOF\s+T/i, /^TP\s+T/i, /^CTU\s+C/i, /^CTD\s+C/i, /^LD\s+[IQMXYSC]/i, /^ST\s+[QYMSC]/i]
    const plcscriptPatterns = [/^let\s+\w+\s*:/i, /^const\s+\w+\s*:/i, /^function\s+\w+\s*\(/i, /^for\s*\(/i, /^while\s*\(/i, /^if\s*\(/i, /:\s*(u8|u16|u32|u64|i8|i16|i32|i64|f32|f64|bool)\s*[@=;]/i, /\+\+|--/, /&&|\|\|/]
    const plcasmPatterns = [/^u8\./i, /^u16\./i, /^u32\./i, /^i8\./i, /^i16\./i, /^i32\./i, /^f32\./i, /^f64\./i, /^jmp\s+\w/i, /^jmp_if\s+\w/i, /^jmp_if_not\s+\w/i, /^call\s+\w/i, /^ret$/i, /^exit$/i]
    let stlScore = 0, plcasmScore = 0, plcscriptScore = 0
    for (const line of lines) {
        for (const p of plcscriptPatterns) if (p.test(line)) { plcscriptScore++; break }
        for (const p of stlPatterns) if (p.test(line)) { stlScore++; break }
        for (const p of plcasmPatterns) if (p.test(line)) { plcasmScore++; break }
    }
    if (plcscriptScore > 0 && plcscriptScore >= stlScore && plcscriptScore >= plcasmScore) return 'plcscript'
    if (plcasmScore > 0 && plcasmScore >= stlScore) return 'plcasm'
    if (stlScore > 0) return 'stl'
    return 'plcasm'
}

// Stream code to runtime
function streamCode(runtime, code) {
    for (let i = 0; i < code.length; i++) runtime.wasm_exports.streamIn(code.charCodeAt(i))
    runtime.wasm_exports.streamIn(0)
}

// Capture stdout output
let capturedOutput = ''
function captureStdout(runtime) { capturedOutput = ''; runtime.stdout_callback = msg => { capturedOutput += msg + '\n' } }
function readCapturedOutput() { const o = capturedOutput; capturedOutput = ''; return o }

// CRC8 calculation
function crc8Simple(bytes) {
    let crc = 0
    for (const byte of bytes) {
        crc ^= byte
        for (let k = 0; k < 8; k++) crc = (crc & 0x80) ? ((crc << 1) ^ 0x31) : (crc << 1)
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

    const runtime = new VovkPLC(wasmPath)
    captureStdout(runtime)
    await runtime.initialize(wasmPath, false, false)
    runtime.readStream()
    readCapturedOutput()

    try {
        const input = await readStdin()
        if (!input.trim()) { console.error('Error: Empty input'); process.exit(1) }

        const language = detectLanguage(input)
        const wasm = runtime.wasm_exports
        let analyzeSource = 'compiled'

        if (!jsonOnly && !silent) {
            console.log(`Detected language: ${language.toUpperCase()}`)
        }

        // Step 1: Compile to bytecode depending on language
        if (language === 'bytecode-hex') {
            // Parse hex and load directly as compiled bytecode
            const hexOnly = input.replace(/[\s\r\n]/g, '')
            const bytes = []
            for (let i = 0; i < hexOnly.length; i += 2) bytes.push(parseInt(hexOnly.substr(i, 2), 16))
            if (!silent && !jsonOnly) console.log(`Parsed ${bytes.length} bytes from hex input`)

            // Load into runtime program for wcet_analyze_runtime
            const checksum = crc8Simple(bytes)
            const bufPtr = wasm.getHexDownloadBuffer ? wasm.getHexDownloadBuffer() : 0
            readCapturedOutput()
            if (bufPtr > 0 && wasm.downloadProgramHex) {
                const view = new Uint8Array(wasm.memory.buffer, bufPtr, hexOnly.length + 1)
                for (let i = 0; i < hexOnly.length; i++) view[i] = hexOnly.charCodeAt(i)
                view[hexOnly.length] = 0
                wasm.downloadProgramHex(bufPtr, bytes.length, checksum)
            } else {
                for (const b of bytes) wasm.streamIn(b)
                wasm.downloadProgram(bytes.length, checksum)
            }
            readCapturedOutput()
            analyzeSource = 'runtime'

        } else if (language === 'project') {
            // Compile as project
            readCapturedOutput()
            streamCode(runtime, input)
            wasm.compileProject()
            if (!silent && !jsonOnly) {
                const out = readCapturedOutput()
                if (out.trim()) console.log(out.trimEnd())
            } else { readCapturedOutput() }
            analyzeSource = 'project'

        } else if (language === 'ladder-graph') {
            // Ladder → STL → PLCASM → bytecode
            readCapturedOutput()
            if (wasm.ladder_to_stl_load_from_stream && wasm.ladder_to_stl_compile) {
                streamCode(runtime, input)
                wasm.ladder_to_stl_load_from_stream()
                wasm.ladder_to_stl_compile()
            }
            // Read the STL output and compile as STL
            const stlOutput = runtime.readOutBuffer ? runtime.readOutBuffer() : runtime.readStream()
            if (stlOutput && stlOutput.trim()) {
                runtime.downloadAssembly(stlOutput)
                wasm.compileAssembly(silent ? false : true)
            }
            if (!silent && !jsonOnly) {
                const out = readCapturedOutput()
                if (out.trim()) console.log(out.trimEnd())
            } else { readCapturedOutput() }

        } else if (language === 'plcscript') {
            // PLCScript → PLCASM → bytecode
            readCapturedOutput()
            if (wasm.plcscript_compile_from_stream) {
                streamCode(runtime, input)
                wasm.plcscript_compile_from_stream()
                const plcasm = runtime.readOutBuffer ? runtime.readOutBuffer() : runtime.readStream()
                if (plcasm && plcasm.trim()) {
                    runtime.downloadAssembly(plcasm)
                    wasm.compileAssembly(silent ? false : true)
                }
            } else {
                runtime.downloadAssembly(input)
                wasm.compileAssembly(silent ? false : true)
            }
            if (!silent && !jsonOnly) {
                const out = readCapturedOutput()
                if (out.trim()) console.log(out.trimEnd())
            } else { readCapturedOutput() }

        } else if (language === 'stl') {
            // STL → PLCASM → bytecode
            readCapturedOutput()
            if (wasm.stl_compile_from_stream) {
                streamCode(runtime, input)
                wasm.stl_compile_from_stream()
                const plcasm = runtime.readOutBuffer ? runtime.readOutBuffer() : runtime.readStream()
                if (plcasm && plcasm.trim()) {
                    runtime.downloadAssembly(plcasm)
                    wasm.compileAssembly(silent ? false : true)
                }
            } else {
                runtime.downloadAssembly(input)
                wasm.compileAssembly(silent ? false : true)
            }
            if (!silent && !jsonOnly) {
                const out = readCapturedOutput()
                if (out.trim()) console.log(out.trimEnd())
            } else { readCapturedOutput() }

        } else {
            // PLCASM → bytecode
            readCapturedOutput()
            runtime.downloadAssembly(input)
            wasm.compileAssembly(silent ? false : true)
            if (!silent && !jsonOnly) {
                const out = readCapturedOutput()
                if (out.trim()) console.log(out.trimEnd())
            } else { readCapturedOutput() }
        }

        // Step 2: Run WCET analysis
        const report = runtime.analyzeWCET(analyzeSource, { print: !jsonOnly && !silent })

        // Show the formatted report output if not --json
        if (!jsonOnly && !silent) {
            const printOutput = readCapturedOutput()
            if (printOutput.trim()) console.log(printOutput.trimEnd())
        }

        // Step 3: Output JSON report
        if (jsonOnly) {
            console.log(JSON.stringify(report, null, 2))
        } else {
            console.log()
            console.log('JSON Report:')
            console.log(JSON.stringify(report, null, 2))
        }

    } catch (err) {
        console.error('Analysis error:', err.message || err)
        process.exit(1)
    }
}

run().catch(err => {
    console.error('Fatal error:', err)
    process.exit(1)
})
