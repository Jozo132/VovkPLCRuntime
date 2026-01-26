// Test for Project Compiler
// Tests the full project compilation with multiple program blocks in different languages

import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

// Stream code to runtime
function streamCode(runtime, code) {
    runtime.wasm_exports.streamClear()
    for (let i = 0; i < code.length; i++) {
        runtime.wasm_exports.streamIn(code.charCodeAt(i))
    }
}

// Read a null-terminated string from WASM memory
function getString(runtime, ptr) {
    if (!ptr) return ''
    const memory = new Uint8Array(runtime.wasm.exports.memory.buffer)
    let str = ''
    let i = ptr
    while (memory[i] !== 0 && i < memory.length) {
        str += String.fromCharCode(memory[i])
        i++
    }
    return str
}

// Capture stdout output
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
    
    // Clear initial banner
    runtime.readStream()
    readCapturedOutput()

    const wasm = runtime.wasm_exports

    console.log('╔══════════════════════════════════════════════════════════════════╗')
    console.log('║              Project Compiler Tests                              ║')
    console.log('╚══════════════════════════════════════════════════════════════════╝')
    console.log()

    let passed = 0
    let failed = 0

    // Test 1: Simple PLCASM project
    const test1_project = `
PROJECT TestProject
VERSION 1.0

SYMBOLS
    input1 : bit : X0.0
    output1 : bit : Y0.0
END_SYMBOLS

PROGRAM Main
    BLOCK Code LANG=PLCASM
u8.const 1
u8.const 2
u8.add
exit
    END_BLOCK
END_PROGRAM
`

    console.log('Test 1: Simple PLCASM project')
    console.log('─'.repeat(60))

    streamCode(runtime, test1_project)
    readCapturedOutput() // Clear

    let result = wasm.project_compile(1)  // 1 = debug mode
    let output = readCapturedOutput()
    if (output) console.log(output)

    if (!result) {
        const errorPtr = wasm.project_getError()
        const error = getString(runtime, errorPtr)
        const errorLine = wasm.project_getErrorLine()
        const errorCol = wasm.project_getErrorColumn()
        console.log(`✗ FAILED - Error at ${errorLine}:${errorCol}: ${error}`)
        failed++
    } else {
        const bytecodeLen = wasm.project_getBytecodeLength()
        const checksum = wasm.project_getChecksum()
        const projectName = getString(runtime, wasm.project_getName())
        const projectVersion = getString(runtime, wasm.project_getVersion())
        const blockCount = wasm.project_getBlockCount()
        
        console.log(`Project: ${projectName} v${projectVersion}`)
        console.log(`Bytecode length: ${bytecodeLen} bytes`)
        console.log(`Checksum: 0x${checksum.toString(16).padStart(2, '0').toUpperCase()}`)
        console.log(`Block count: ${blockCount}`)
        
        for (let i = 0; i < blockCount; i++) {
            const blockName = getString(runtime, wasm.project_getBlockName(i))
            const blockLang = wasm.project_getBlockLanguage(i)
            const blockOffset = wasm.project_getBlockOffset(i)
            const blockSize = wasm.project_getBlockSize(i)
            console.log(`  Block ${i}: ${blockName} (lang=${blockLang}) offset=${blockOffset} size=${blockSize}`)
        }
        
        if (bytecodeLen > 0 && blockCount === 1) {
            console.log('✓ PASSED')
            passed++
        } else {
            console.log('✗ FAILED - unexpected results')
            failed++
        }
    }

    console.log()

    // Test 2: STL project
    const test2_project = `
PROJECT STLTest
VERSION 2.0

PROGRAM Network1
    BLOCK Logic LANG=STL
A X0.0
= Y0.0
    END_BLOCK
END_PROGRAM
`

    console.log('Test 2: STL project')
    console.log('─'.repeat(60))

    wasm.project_reset()
    streamCode(runtime, test2_project)
    readCapturedOutput()

    result = wasm.project_compile(1)
    output = readCapturedOutput()
    if (output) console.log(output)

    if (!result) {
        const errorPtr = wasm.project_getError()
        const error = getString(runtime, errorPtr)
        console.log(`✗ FAILED - Error: ${error}`)
        failed++
    } else {
        const bytecodeLen = wasm.project_getBytecodeLength()
        console.log(`Bytecode length: ${bytecodeLen} bytes`)
        
        wasm.project_printInfo()
        output = readCapturedOutput()
        if (output) console.log(output)
        
        if (bytecodeLen > 0) {
            console.log('✓ PASSED')
            passed++
        } else {
            console.log('✗ FAILED - no bytecode generated')
            failed++
        }
    }

    console.log()

    // Test 3: Multi-block project with different languages
    const test3_project = `
PROJECT MultiLang
VERSION 3.0

MEMORY
M 448-703
I 64-127
Q 128-191
T 704-1023
END_MEMORY

SYMBOLS
    start_btn : bit : X0.0
    stop_btn : bit : X0.1
    motor : bit : Y0.0
END_SYMBOLS

PROGRAM Main
    BLOCK Init LANG=PLCASM
u8.const 0
exit
    END_BLOCK

    BLOCK Logic LANG=STL
A X0.0
AN X0.1
= Y0.0
    END_BLOCK
END_PROGRAM
`

    console.log('Test 3: Multi-block project (PLCASM + STL)')
    console.log('─'.repeat(60))

    wasm.project_reset()
    streamCode(runtime, test3_project)
    readCapturedOutput()

    result = wasm.project_compile(1)
    output = readCapturedOutput()
    if (output) console.log(output)

    if (!result) {
        const errorPtr = wasm.project_getError()
        const error = getString(runtime, errorPtr)
        const errorLine = wasm.project_getErrorLine()
        console.log(`✗ FAILED - Error at line ${errorLine}: ${error}`)
        failed++
    } else {
        const bytecodeLen = wasm.project_getBytecodeLength()
        const blockCount = wasm.project_getBlockCount()
        const memAreaCount = wasm.project_getMemoryAreaCount()
        
        console.log(`Bytecode length: ${bytecodeLen} bytes`)
        console.log(`Block count: ${blockCount}`)
        console.log(`Memory areas: ${memAreaCount}`)
        
        for (let i = 0; i < memAreaCount; i++) {
            const areaName = getString(runtime, wasm.project_getMemoryAreaName(i))
            const areaStart = wasm.project_getMemoryAreaStart(i)
            const areaEnd = wasm.project_getMemoryAreaEnd(i)
            console.log(`  Memory ${areaName}: ${areaStart}-${areaEnd}`)
        }
        
        const langNames = ['UNKNOWN', 'PLCASM', 'STL', 'LADDER', 'FBD', 'SFC', 'ST', 'IL']
        for (let i = 0; i < blockCount; i++) {
            const blockName = getString(runtime, wasm.project_getBlockName(i))
            const blockLang = wasm.project_getBlockLanguage(i)
            const blockOffset = wasm.project_getBlockOffset(i)
            const blockSize = wasm.project_getBlockSize(i)
            console.log(`  Block ${i}: ${blockName} (${langNames[blockLang] || 'UNKNOWN'}) offset=${blockOffset} size=${blockSize}`)
        }
        
        // Print first 80 bytes of bytecode
        const bytecodePtr3 = wasm.project_getBytecode()
        if (bytecodePtr3 && bytecodeLen > 0) {
            const memory = new Uint8Array(runtime.wasm.exports.memory.buffer)
            const numBytes = Math.min(80, bytecodeLen)
            let hexStr = ''
            for (let i = 0; i < numBytes; i++) {
                hexStr += memory[bytecodePtr3 + i].toString(16).padStart(2, '0').toUpperCase()
                if ((i + 1) % 16 === 0) hexStr += '\n'
                else hexStr += ' '
            }
            console.log(`\nBytecode (first ${numBytes} bytes):`)
            console.log(hexStr.trim())
        }
        
        // Load and run
        console.log('Loading program into runtime...')
        const loaded = wasm.project_load()
        console.log('Load result:', loaded ? 'SUCCESS' : 'FAILED')
        
        if (bytecodeLen > 0 && blockCount === 2) {
            console.log('✓ PASSED')
            passed++
        } else {
            console.log('✗ FAILED - unexpected results')
            failed++
        }
    }

    console.log()

    // Test 4: Ladder JSON project
    const test4_ladder = {
        nodes: [
            { id: "n1", type: "contact_no", address: "X0.0", x: 0, y: 0 },
            { id: "n2", type: "coil", address: "Y0.0", x: 100, y: 0 }
        ],
        connections: [
            { sources: ["n1"], destinations: ["n2"] }
        ]
    }

    const test4_project = `
PROJECT LadderTest
VERSION 4.0

PROGRAM Network1
    BLOCK Ladder LANG=LADDER
${JSON.stringify(test4_ladder)}
    END_BLOCK
END_PROGRAM
`

    console.log('Test 4: Ladder JSON project')
    console.log('─'.repeat(60))

    wasm.project_reset()
    streamCode(runtime, test4_project)
    readCapturedOutput()

    result = wasm.project_compile(1)
    output = readCapturedOutput()
    if (output) console.log(output)

    if (!result) {
        const errorPtr = wasm.project_getError()
        const error = getString(runtime, errorPtr)
        const errorLine = wasm.project_getErrorLine()
        console.log(`✗ FAILED - Error at line ${errorLine}: ${error}`)
        failed++
    } else {
        const bytecodeLen = wasm.project_getBytecodeLength()
        console.log(`Bytecode length: ${bytecodeLen} bytes`)
        
        wasm.project_printInfo()
        output = readCapturedOutput()
        if (output) console.log(output)
        
        // Print first 80 bytes of bytecode
        const bytecodePtr = wasm.project_getBytecode()
        if (bytecodePtr && bytecodeLen > 0) {
            const memory = new Uint8Array(runtime.wasm.exports.memory.buffer)
            const numBytes = Math.min(80, bytecodeLen)
            let hexStr = ''
            for (let i = 0; i < numBytes; i++) {
                hexStr += memory[bytecodePtr + i].toString(16).padStart(2, '0').toUpperCase()
                if ((i + 1) % 16 === 0) hexStr += '\n'
                else hexStr += ' '
            }
            console.log(`\nBytecode (first ${numBytes} bytes):`)
            console.log(hexStr.trim())
        }
        
        if (bytecodeLen > 0) {
            console.log('✓ PASSED')
            passed++
        } else {
            console.log('✗ FAILED - no bytecode generated')
            failed++
        }
    }

    console.log()
    console.log('─'.repeat(60))
    console.log(`Results: ${passed}/${passed + failed} tests passed`)
    
    if (failed > 0) {
        process.exit(1)
    }
}

run().catch(err => {
    console.error('Test failed:', err)
    process.exit(1)
})
