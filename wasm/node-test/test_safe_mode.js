#!/usr/bin/env node
// test_safe_mode.js - Tests for PLCRUNTIME_SAFE_MODE bounds checking
//
// This test runs malicious/untrusted bytecode against both WASM variants:
// - VovkPLC.wasm (optimized, no bounds checks) - may produce undefined behavior
// - VovkPLC-debug.wasm (safe mode, with bounds checks) - should catch errors gracefully
//
// Usage: npm run test_safe_mode

import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'
import fs from 'fs'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

// RuntimeError enum values (from runtime-instructions.h)
const RuntimeError = {
    STATUS_SUCCESS: 0,
    UNKNOWN_INSTRUCTION: 1,
    INVALID_INSTRUCTION: 2,
    INVALID_DATA_TYPE: 3,
    STACK_OVERFLOW: 4,
    STACK_UNDERFLOW: 5,
    INVALID_STACK_SIZE: 6,
    EMPTY_STACK: 7,
    CALL_STACK_OVERFLOW: 8,
    CALL_STACK_UNDERFLOW: 9,
    INVALID_MEMORY_ADDRESS: 10,
    INVALID_MEMORY_SIZE: 11,
    UNDEFINED_STATE: 12,
    INVALID_PROGRAM_INDEX: 13,
    PROGRAM_SIZE_EXCEEDED: 14,
    PROGRAM_POINTER_OUT_OF_BOUNDS: 15,
    PROGRAM_EXITED: 16,
    EMPTY_PROGRAM: 17,
    NO_PROGRAM: 18,
    INVALID_CHECKSUM: 19,
    EXECUTION_ERROR: 20,
    EXECUTION_TIMEOUT: 21,
    MEMORY_ACCESS_ERROR: 22,
    PROGRAM_CYCLE_LIMIT_EXCEEDED: 23,
}

const RuntimeErrorName = Object.fromEntries(
    Object.entries(RuntimeError).map(([k, v]) => [v, k])
)

// Instruction opcodes (from runtime-instructions.h)
const Opcode = {
    NOP: 0x00,
    type_pointer: 0x01,
    type_bool: 0x02,
    type_u8: 0x03,
    type_u16: 0x04,
    type_u32: 0x05,
    type_u64: 0x06,
    type_i8: 0x07,
    type_i16: 0x08,
    type_i32: 0x09,
    type_i64: 0x0A,
    type_f32: 0x0B,
    type_f64: 0x0C,
    CVT: 0x10,
    LOAD: 0x11,
    MOVE: 0x12,
    MOVE_COPY: 0x13,
    COPY: 0x14,
    SWAP: 0x15,
    DROP: 0x16,
    CLEAR: 0x17,
    LOAD_FROM: 0x18,
    MOVE_TO: 0x19,
    INC_MEM: 0x1A,
    DEC_MEM: 0x1B,
    PICK: 0x1C,
    POKE: 0x1D,
    ADD: 0x20,
    SUB: 0x21,
    MUL: 0x22,
    DIV: 0x23,
    MOD: 0x24,
    POW: 0x25,
    SQRT: 0x26,
    NEG: 0x27,
    ABS: 0x28,
    SIN: 0x29,
    COS: 0x2A,
    JMP: 0x40,
    JMP_IF: 0x41,
    JMP_IF_NOT: 0x42,
    CALL: 0x43,
    CALL_IF: 0x44,
    CALL_IF_NOT: 0x45,
    RET: 0x46,
    EXIT: 0xFF,
}

// CRC8 checksum (polynomial 0x31)
function crc8(data) {
    let crc = 0
    for (const byte of data) {
        crc ^= byte
        for (let i = 0; i < 8; i++) {
            if (crc & 0x80) {
                crc = ((crc << 1) ^ 0x31) & 0xFF
            } else {
                crc = (crc << 1) & 0xFF
            }
        }
    }
    return crc
}

// Test cases: malicious bytecode that should fail with bounds checking
const testCases = [
    // ===== PROGRAM BOUNDS TESTS (require SAFE_MODE) =====
    {
        name: 'Program size exceeded - truncated ADD instruction',
        description: 'ADD instruction without type operand',
        bytecode: [Opcode.ADD],
        expectedSafe: RuntimeError.PROGRAM_POINTER_OUT_OF_BOUNDS,
        expectCrashUnsafe: true,
        safeModeOnly: true,
    },
    {
        name: 'Program size exceeded - truncated PUSH u32',
        description: 'PUSH u32 but only 2 bytes of data provided',
        bytecode: [Opcode.type_u32, 0x12, 0x34],
        expectedSafe: RuntimeError.PROGRAM_SIZE_EXCEEDED,
        expectCrashUnsafe: true,
        safeModeOnly: true,
    },
    {
        name: 'Program size exceeded - truncated PUSH u16',
        description: 'PUSH u16 but only 1 byte of data provided',
        bytecode: [Opcode.type_u16, 0x12],
        expectedSafe: RuntimeError.PROGRAM_SIZE_EXCEEDED,
        expectCrashUnsafe: true,
        safeModeOnly: true,
    },
    {
        name: 'Program size exceeded - truncated SUB instruction',
        description: 'SUB instruction without type operand',
        bytecode: [Opcode.type_u8, 0x05, Opcode.type_u8, 0x03, Opcode.SUB],
        expectedSafe: RuntimeError.PROGRAM_POINTER_OUT_OF_BOUNDS,
        expectCrashUnsafe: true,
        safeModeOnly: true,
    },
    {
        name: 'Program size exceeded - truncated CVT instruction',
        description: 'CVT with only source type, missing dest type',
        bytecode: [Opcode.type_u8, 0x05, Opcode.CVT, Opcode.type_u8],
        expectedSafe: RuntimeError.PROGRAM_POINTER_OUT_OF_BOUNDS,
        expectCrashUnsafe: true,
        safeModeOnly: true,
    },
    {
        name: 'Program size exceeded - truncated DROP instruction',
        description: 'DROP without type operand',
        bytecode: [Opcode.type_u8, 0x05, Opcode.DROP],
        expectedSafe: RuntimeError.PROGRAM_POINTER_OUT_OF_BOUNDS,
        expectCrashUnsafe: true,
        safeModeOnly: true,
    },
    {
        name: 'Program size exceeded - truncated COPY instruction',
        description: 'COPY without type operand',
        bytecode: [Opcode.type_u8, 0x05, Opcode.COPY],
        expectedSafe: RuntimeError.PROGRAM_POINTER_OUT_OF_BOUNDS,
        expectCrashUnsafe: true,
        safeModeOnly: true,
    },
    {
        name: 'Program size exceeded - truncated SWAP instruction',
        description: 'SWAP with only one type operand',
        bytecode: [Opcode.type_u8, 0x05, Opcode.type_u8, 0x03, Opcode.SWAP, Opcode.type_u8],
        expectedSafe: RuntimeError.PROGRAM_POINTER_OUT_OF_BOUNDS,
        expectCrashUnsafe: true,
        safeModeOnly: true,
    },
    
    // ===== INVALID DATA TYPE TESTS (work on both builds) =====
    {
        name: 'Invalid data type - CVT with invalid source type',
        description: 'Conversion with invalid type specifier 0xFF',
        bytecode: [Opcode.type_u8, 0x05, Opcode.CVT, 0xFF, Opcode.type_u8, Opcode.EXIT],
        expectedSafe: RuntimeError.INVALID_DATA_TYPE,
        expectCrashUnsafe: false,
    },
    {
        name: 'Invalid data type - CVT with invalid dest type',
        description: 'Conversion to invalid type specifier 0xFF',
        bytecode: [Opcode.type_u8, 0x05, Opcode.CVT, Opcode.type_u8, 0xFF, Opcode.EXIT],
        expectedSafe: RuntimeError.INVALID_DATA_TYPE,
        expectCrashUnsafe: false,
    },
    {
        name: 'Invalid data type - ADD with invalid type',
        description: 'ADD with type 0x7F which is not defined',
        bytecode: [Opcode.type_u8, 0x05, Opcode.type_u8, 0x03, Opcode.ADD, 0x7F, Opcode.EXIT],
        expectedSafe: RuntimeError.INVALID_DATA_TYPE,
        expectCrashUnsafe: false,
    },
    
    // ===== STACK SAFETY TESTS (require SAFE_MODE) =====
    {
        name: 'Stack Underflow - DROP from empty stack',
        description: 'DROP (u8) on empty stack',
        bytecode: [Opcode.DROP, Opcode.type_u8, Opcode.EXIT],
        expectedSafe: RuntimeError.STACK_UNDERFLOW,
        expectCrashUnsafe: false,
        safeModeOnly: true,
    },
    {
        name: 'Stack Underflow - COPY from empty stack',
        description: 'COPY (u8) on empty stack',
        bytecode: [Opcode.COPY, Opcode.type_u8, Opcode.EXIT],
        expectedSafe: RuntimeError.STACK_UNDERFLOW,
        expectCrashUnsafe: false,
        safeModeOnly: true,
    },
    {
        name: 'Stack Overflow - push beyond stack limit',
        description: 'Push many u64 values to overflow the 1024-byte stack',
        // Generate bytecode that pushes 130 u64 values (130 * 8 = 1040 bytes > 1024 stack limit)
        // Each u64 push is 9 bytes: [type_u64, 8 bytes of data]
        bytecode: (() => {
            const code = []
            for (let i = 0; i < 130; i++) {
                code.push(Opcode.type_u64, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88)
            }
            code.push(Opcode.EXIT)
            return code
        })(),
        expectedSafe: RuntimeError.STACK_OVERFLOW,
        expectCrashUnsafe: false,
        safeModeOnly: true,
    },

    // ===== VALID EXECUTION TESTS (should succeed on both) =====
    {
        name: 'Valid program - simple add',
        description: 'Push two values, add them',
        bytecode: [Opcode.type_u8, 0x05, Opcode.type_u8, 0x03, Opcode.ADD, Opcode.type_u8, Opcode.EXIT],
        expectedSafe: RuntimeError.STATUS_SUCCESS,
        expectCrashUnsafe: false,
    },
]

// Check if debug WASM exists
function debugWasmExists() {
    const wasmDebug = path.resolve(__dirname, '../dist/VovkPLC-debug.wasm')
    return fs.existsSync(wasmDebug)
}

async function loadBytecodeToRuntime(runtime, bytecode) {
    // Clear stream
    runtime.wasm_exports.streamClear()
    
    // Stream bytes
    for (const byte of bytecode) {
        runtime.wasm_exports.streamIn(byte)
    }
    
    // Calculate checksum
    const checksum = crc8(bytecode)
    
    // Download program
    const loadError = runtime.wasm_exports.downloadProgram(bytecode.length, checksum)
    return loadError
}

async function runTest(testCase, runtime, variant) {
    const { name, bytecode, expectedSafe, expectCrashUnsafe } = testCase
    
    // Reset runtime
    runtime.wasm_exports.initialize()
    runtime.wasm_exports.clearStack()
    
    // Load bytecode
    const loadError = await loadBytecodeToRuntime(runtime, bytecode)
    if (loadError) {
        return {
            name,
            variant,
            loaded: false,
            loadError,
            status: null,
            expected: expectedSafe,
        }
    }
    
    // Run program
    let status
    try {
        status = runtime.wasm_exports.run()
    } catch (e) {
        return {
            name,
            variant,
            loaded: true,
            crashed: true,
            error: e.message,
            expected: expectedSafe,
        }
    }
    
    return {
        name,
        variant,
        loaded: true,
        status,
        statusName: RuntimeErrorName[status] || `UNKNOWN(${status})`,
        expected: expectedSafe,
        expectedName: RuntimeErrorName[expectedSafe],
        expectCrashUnsafe,
    }
}

async function main() {
    console.log()
    console.log('╔══════════════════════════════════════════════════════════════════╗')
    console.log('║       PLCRUNTIME_SAFE_MODE Bounds Checking Tests                 ║')
    console.log('╚══════════════════════════════════════════════════════════════════╝')
    console.log()
    
    // Get shared runtime instances
    const { runtimeOptimized, runtimeDebug } = await getSharedRuntimes()
    
    console.log('Testing malicious bytecode against WASM variants:')
    console.log('  • VovkPLC.wasm       - production (no bounds checks)')
    console.log('  • VovkPLC-debug.wasm - safe mode (with bounds checks)')
    console.log()
    console.log('Most tests use the production build to verify normal operation.')
    console.log('Tests marked [SAFE MODE ONLY] use the debug build for overflow protection.')
    console.log()
    
    const results = {
        passed: 0,
        failed: 0,
        total: testCases.length,
    }
    
    console.log('─'.repeat(70))
    
    for (const testCase of testCases) {
        const isSafeModeOnly = testCase.safeModeOnly === true
        const runtime = isSafeModeOnly ? runtimeDebug : runtimeOptimized
        const variantName = isSafeModeOnly ? 'Safe Mode' : 'Production'
        
        console.log()
        console.log(`┌─ ${testCase.name}${isSafeModeOnly ? ' [SAFE MODE ONLY]' : ''}`)
        console.log(`│  ${testCase.description}`)
        console.log(`│  Bytecode: [${testCase.bytecode.slice(0, 20).map(b => '0x' + b.toString(16).toUpperCase().padStart(2, '0')).join(', ')}${testCase.bytecode.length > 20 ? `, ... (${testCase.bytecode.length} bytes total)` : ''}]`)
        
        // Run on selected runtime
        const result = await runTest(testCase, runtime, variantName.toLowerCase())
        
        // Check if it returned the expected status
        const caughtExpected = result.status === testCase.expectedSafe
        
        // Display results
        console.log(`│`)
        console.log(`│  Runtime: ${variantName}`)
        if (result.crashed) {
            console.log(`│    Status: CRASHED - ${result.error}`)
        } else if (!result.loaded) {
            console.log(`│    Status: Load failed (error ${result.loadError})`)
        } else {
            console.log(`│    Status: ${result.statusName} (${result.status})`)
            console.log(`│    Expected: ${RuntimeErrorName[testCase.expectedSafe]} (${testCase.expectedSafe})`)
        }
        
        // Determine pass/fail
        if (caughtExpected) {
            console.log(`│`)
            console.log(`└─ ✓ PASS: ${isSafeModeOnly ? 'Safe mode' : 'Runtime'} correctly ${testCase.expectedSafe === RuntimeError.STATUS_SUCCESS ? 'succeeded' : 'caught the error'}`)
            results.passed++
        } else {
            console.log(`│`)
            console.log(`└─ ✗ FAIL: Expected ${RuntimeErrorName[testCase.expectedSafe]} but got ${result.statusName}`)
            results.failed++
        }
    }
    
    console.log()
    console.log('─'.repeat(70))
    console.log()
    
    // Summary
    console.log('Summary:')
    console.log(`  Total tests: ${results.total}`)
    console.log(`  Passed: ${results.passed}`)
    console.log(`  Failed: ${results.failed}`)
    console.log()
    
    if (results.failed > 0) {
        console.log('⚠ Some tests failed')
        process.exit(1)
    } else {
        console.log('✓ All tests passed - shared instances work correctly across test suite')
        process.exit(0)
    }
}

// Shared WASM instances for test suite (singleton pattern)
let sharedRuntimeOptimized = null
let sharedRuntimeDebug = null

/**
 * Initialize or return shared WASM runtime instances.
 * This ensures all tests in the suite share the same instances,
 * catching any state leakage between test runs.
 * @param {VovkPLC|null} externalRuntime - Optional external runtime for the optimized build
 */
async function getSharedRuntimes(externalRuntime = null) {
    const wasmOptimized = path.resolve(__dirname, '../dist/VovkPLC.wasm')
    const wasmDebug = path.resolve(__dirname, '../dist/VovkPLC-debug.wasm')
    
    // Use external runtime if provided (from unit test runner), otherwise create one
    if (externalRuntime) {
        sharedRuntimeOptimized = externalRuntime
    } else if (!sharedRuntimeOptimized) {
        sharedRuntimeOptimized = new VovkPLC(wasmOptimized)
        await sharedRuntimeOptimized.initialize(wasmOptimized, false, true)
    }
    
    // Debug runtime is always created separately (different WASM)
    if (!sharedRuntimeDebug) {
        sharedRuntimeDebug = new VovkPLC(wasmDebug)
        await sharedRuntimeDebug.initialize(wasmDebug, false, true)
    }
    
    return { runtimeOptimized: sharedRuntimeOptimized, runtimeDebug: sharedRuntimeDebug }
}

/**
 * Run tests and return results for the unified test runner
 * @param {object} options - Test options
 * @param {boolean} options.verbose - Whether to show detailed output
 * @param {boolean} options.debug - Whether to show debug output
 * @param {VovkPLC} [options.runtime] - Shared runtime instance for optimized build
 * @returns {Promise<{name: string, passed: number, failed: number, total: number, tests: Array}>}
 */
export async function runTests(options = {}) {
    const { verbose = false, debug = false, runtime: sharedRuntime = null } = options
    
    // Check if debug WASM exists
    if (!debugWasmExists()) {
        return {
            name: 'Safe Mode Bounds Checking',
            passed: 0,
            failed: 0,
            total: 1,
            tests: [{
                name: 'Safe Mode Tests',
                passed: false,
                error: 'VovkPLC-debug.wasm not found. Run `npm run build` to generate it.'
            }]
        }
    }
    
    // Get shared runtime instances (use external runtime for optimized if provided)
    const { runtimeOptimized, runtimeDebug } = await getSharedRuntimes(sharedRuntime)
    
    const tests = []
    let passed = 0
    let failed = 0
    
    for (const testCase of testCases) {
        // Choose which runtime to use:
        // - safeModeOnly tests use the debug (safe mode) runtime
        // - All other tests use the optimized (production) runtime to verify they work correctly
        const runtime = testCase.safeModeOnly ? runtimeDebug : runtimeOptimized
        const variant = testCase.safeModeOnly ? 'safe' : 'production'
        
        // Run the test
        const result = await runTest(testCase, runtime, variant)
        
        // Check if the runtime returned the expected error/status
        const caughtExpected = result.status === testCase.expectedSafe
        
        if (caughtExpected) {
            passed++
            tests.push({
                name: testCase.name,
                passed: true,
                info: verbose ? `${RuntimeErrorName[testCase.expectedSafe]}` : undefined
            })
        } else {
            failed++
            tests.push({
                name: testCase.name,
                passed: false,
                error: `Expected ${RuntimeErrorName[testCase.expectedSafe]} but got ${result.loaded ? result.statusName : `load error ${result.loadError}`}`
            })
        }
    }
    
    return {
        name: 'Safe Mode Bounds Checking',
        passed,
        failed,
        total: testCases.length,
        tests
    }
}

// Only run main() when executed directly, not when imported
const isMainModule = process.argv[1]?.endsWith('test_safe_mode.js')
if (isMainModule) {
    main().catch(err => {
        console.error('Error:', err)
        process.exit(1)
    })
}
