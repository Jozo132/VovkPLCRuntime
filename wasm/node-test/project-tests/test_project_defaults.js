
import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'
import fs from 'fs'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')

if (!fs.existsSync(wasmPath)) {
    console.error('WASM file not found at', wasmPath)
    process.exit(1)
}

const runtime = new VovkPLC()
await runtime.initialize(wasmPath, false, false)

console.log('Testing Project Defaults and Startup Safety Skip')

const projectSource = `
VOVKPLCPROJECT TestDefaults
VERSION 1.0

MEMORY
  M 0-100
END_MEMORY

SYMBOLS
  TestVar : u8 : M10 = 42
  TestBit : bool : M11.0 = 1
  Input1 : bool : M30.0
  EdgeMem : bool : M31.0
  EdgeResult : bool : M32.0
END_SYMBOLS

PROGRAM main
  BLOCK Main LANG=STL
    // Default values check happens automatically via startup block
    
    // Edge detection safety check
    // If Input1 is High on startup, EdgeMem should appear High so FP doesn't trigger
    A Input1
    FP EdgeMem
    = EdgeResult
  END_BLOCK
END_PROGRAM
`

// 1. Compile
console.log('Compiling project...')
const inBufferPtr = runtime.wasm_exports.get_in_buffer_ptr()
const inBufferSize = runtime.wasm_exports.get_in_buffer_size()
const wasmMem = new Uint8Array(runtime.wasm_exports.memory.buffer)

if (projectSource.length >= inBufferSize) {
    console.error('Project source too large for input buffer')
    process.exit(1)
}

runtime.wasm_exports.streamClear()
for (let i = 0; i < projectSource.length; i++) {
    wasmMem[inBufferPtr + i] = projectSource.charCodeAt(i)
}
runtime.wasm_exports.set_in_index(projectSource.length)

const success = runtime.wasm_exports.project_compile(true) // debug=true
if (!success) {
    const errPtr = runtime.wasm_exports.project_getError()
    const memory = new Uint8Array(runtime.wasm_exports.memory.buffer)
    let str = ''
    let p = errPtr
    let limit = 1000 // Safety limit
    while (memory[p] !== 0 && limit-- > 0) {
        str += String.fromCharCode(memory[p++])
    }
    
    // Also get more details
    const line = runtime.wasm_exports.project_getErrorLine()
    const col = runtime.wasm_exports.project_getErrorColumn()
    
    // Get token info
    const tokenPtr = runtime.wasm_exports.project_getErrorToken()
    let tokenStr = ''
    p = tokenPtr
    limit = 100
    while (memory[p] !== 0 && limit-- > 0) {
        tokenStr += String.fromCharCode(memory[p++])
    }

    // Get source line info
    const linePtr = runtime.wasm_exports.project_getErrorSourceLine()
    let lineStr = ''
    p = linePtr
    limit = 500
    while (memory[p] !== 0 && limit-- > 0) {
        lineStr += String.fromCharCode(memory[p++])
    }

    console.error(`\nProject compilation failed!`)
    console.error(`Error: ${str}`)
    console.error(`Location: Line ${line}, Column ${col}`)
    if (tokenStr) console.error(`Token: "${tokenStr}"`)
    
    if (lineStr) {
        console.error(`\nCode:`)
        console.error(`${line}: ${lineStr}`)
        // Create caret pointer
        let pointer = ''
        for (let i = 1; i < col + String(line).length + 2; i++) pointer += ' '
        pointer += '^'
        console.error(pointer)
    }

    process.exit(1)
}
console.log('Project compiled.')

// 2. Load Program to Runtime
console.log('Loading to runtime...')
const loadErr = runtime.wasm_exports.project_loadToRuntime()
if (loadErr !== 0) { // 0 = RUNTIME_OK
    console.error('Load failed with error code:', loadErr)
    process.exit(1)
}
console.log('Loaded successfully.')

// 3. Pre-write Input (Safety Skip Test target)
// We want to simulate M30.0 (Input1) being HIGH on startup.
// Offset = 448 (Marker default).
const MARKER_OFFSET = 448
const MEM_BASE = runtime.wasm_exports.getMemoryLocation()
const memory = new Uint8Array(runtime.wasm_exports.memory.buffer)

const INPUT1_BYTE_OFFSET = MEM_BASE + MARKER_OFFSET + 30
memory[INPUT1_BYTE_OFFSET] = 1
console.log(`Set Input1 (M30.0) at absolute offset ${INPUT1_BYTE_OFFSET} (Relative: ${MARKER_OFFSET + 30}) to 1`)

// 4. Run One Cycle
console.log('Running one cycle...')
runtime.wasm_exports.memoryReset()
runtime.wasm_exports.run()

// 6. Test Output Buffer (Upload)
console.log('Testing Output Buffer (Upload)...')
if (runtime.wasm_exports.flush_out_buffer) runtime.wasm_exports.flush_out_buffer()
const uploadedByteCount = runtime.wasm_exports.project_uploadBytecode()
console.log(`Uploaded ${uploadedByteCount} bytes.`)

const outBufferPtr = runtime.wasm_exports.get_out_buffer_ptr()
const outIndex = runtime.wasm_exports.get_out_index()
// const outBufferSize = runtime.wasm_exports.get_out_buffer_size()
const wasmMemOut = new Uint8Array(runtime.wasm_exports.memory.buffer, outBufferPtr, outIndex)

// project_uploadBytecode outputs hex string (2 chars per byte)
const expectedHexLength = uploadedByteCount * 2
if (outIndex !== expectedHexLength) {
    console.warn(`WARNING: Expected ${expectedHexLength} chars in output buffer, got ${outIndex}`)
} else {
    console.log(`Output buffer size matches expected hex length (${outIndex})`)
}

if (outIndex > 0) {
    const preview = new TextDecoder().decode(wasmMemOut.slice(0, Math.min(outIndex, 40)))
    console.log('Output Buffer Preview (Hex):', preview)
}

// Reset
runtime.wasm_exports.flush_out_buffer()

// 5. Verify Defaults
// TestVar : u8 : M10 = 42
const TESTVAR_BYTE_OFFSET = MEM_BASE + MARKER_OFFSET + 10
const testVarVal = memory[TESTVAR_BYTE_OFFSET]
if (testVarVal !== 42) {
    console.error(`Default Value Fail: Expected 42, got ${testVarVal}`)
    process.exit(1)
} else {
    console.log('Default Value OK (42)')
}

// TestBit : bool : M11.0 = 1
const TESTBIT_BYTE_OFFSET = MEM_BASE + MARKER_OFFSET + 11
const testBitVal = memory[TESTBIT_BYTE_OFFSET]
if ((testBitVal & 1) !== 1) {
    console.error(`Default Bit Fail: Expected 1, got ${testBitVal & 1}`)
    process.exit(1)
} else {
    console.log('Default Bit OK (1)')
}

// 6. Verify Edge Safety Skip
// Logic: A Input1 (1); FP EdgeMem; = EdgeResult (0 if skipped, 1 if triggered)
const EDGERESULT_BYTE_OFFSET = MEM_BASE + MARKER_OFFSET + 32
const edgeResultVal = memory[EDGERESULT_BYTE_OFFSET]

// EdgeMem (M31.0) should be 1 (synced)
const EDGEMEM_BYTE_OFFSET = MEM_BASE + MARKER_OFFSET + 31
const edgeMemVal = memory[EDGEMEM_BYTE_OFFSET]

console.log(`Edge Result: ${edgeResultVal} (Expected 0)`)
console.log(`Edge Memory: ${edgeMemVal} (Expected 1)`)

if ((edgeResultVal & 1) !== 0) {
    console.error(`Edge Safety Fail: Edge Triggered (Result=1) but should have been skipped!`)
    process.exit(1)
} else {
    console.log('Edge Safety OK (Skipped)')
}

if ((edgeMemVal & 1) !== 1) {
    console.error(`Edge Memory Sync Fail: Memory bit not synced to Input!`)
    process.exit(1)
} else {
    console.log('Edge Memory Sync OK')
}

console.log('\nALL TESTS PASSED')
