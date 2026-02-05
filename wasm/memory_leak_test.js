#!/usr/bin/env node
// memory_leak_test.js - Memory leak detection unit tests for VovkPLC WASM
//
// This script tests for memory leaks by repeatedly calling WASM functions
// and monitoring memory usage. Memory should stabilize after initial allocations.
//
// Usage:
//   node wasm/memory_leak_test.js [--verbose]
//   npm run memory_leak_test
//
// Each test is repeated 10 times. Memory should be stable after the 2nd iteration.
// @ts-check
'use strict'

import VovkPLC from './dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)
const wasmPath = path.resolve(__dirname, 'dist/VovkPLC.wasm')

// ANSI colors
const RED = '\x1b[31m'
const GREEN = '\x1b[32m'
const YELLOW = '\x1b[33m'
const CYAN = '\x1b[36m'
const DIM = '\x1b[2m'
const RESET = '\x1b[0m'
const BOLD = '\x1b[1m'
const BG_RED = '\x1b[41m'
const BG_GREEN = '\x1b[42m'

// Will be set by runTests options or CLI args
let verboseMode = false
let silentMode = false
let log = console.log.bind(console)

const ITERATIONS = 10
const STABLE_AFTER = 2 // Memory should be stable after this many iterations

// Test data
const SIMPLE_PLCASM = `
i16.const 42
i16.const 10
i16.add
i16.drop
`

const STL_SOURCE = `
LD X0.0
A X0.1
= Y0.0
`

const LADDER_JSON = JSON.stringify({
    nodes: [
        { id: 'n1', type: 'contact', x: 0, y: 0, address: 'X0.0' },
        { id: 'n2', type: 'contact', x: 1, y: 0, address: 'X0.1' },
        { id: 'n3', type: 'coil', x: 2, y: 0, address: 'Y0.0' }
    ],
    connections: [
        { from: 'n1', to: 'n2' },
        { from: 'n2', to: 'n3' }
    ]
})

const PLCSCRIPT_SOURCE = `
let counter: i16 @ M0 = 0;
counter = counter + 1;
`

const ST_SOURCE = `
VAR
    counter : INT := 0;
END_VAR
counter := counter + 1;
`

const PROJECT_ALL_LANGUAGES = `
VOVKPLCPROJECT MemoryLeakTest
VERSION 1.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 32
    X 32
    Y 32
    M 128
    T 16
    C 16
END_MEMORY

FLASH
    SIZE 8192
END_FLASH

SYMBOLS
    input_a : bit : X0.0
    input_b : bit : X0.1
    output_y : bit : Y0.0
    counter : i16 : M0
    flag : bit : M2.0
END_SYMBOLS

PROGRAM main
    BLOCK LANG=PLCASM block_plcasm
u8.readBit input_a
u8.readBit input_b
u8.and
u8.writeBit output_y
    END_BLOCK

    BLOCK LANG=STL block_stl
LD X0.0
O X0.1
= Y0.0
    END_BLOCK

    BLOCK LANG=LADDER block_ladder
{
    "nodes": [
        {"id": "n1", "type": "contact", "x": 0, "y": 0, "address": "X0.0"},
        {"id": "n2", "type": "coil", "x": 1, "y": 0, "address": "Y0.0"}
    ],
    "connections": [
        {"from": "n1", "to": "n2"}
    ]
}
    END_BLOCK

    BLOCK LANG=PLCSCRIPT block_plcscript
let temp: i16 @ M4 = 0;
temp = temp + 1;
    END_BLOCK

    BLOCK LANG=ST block_st
VAR
    local_var : INT := 0;
END_VAR
local_var := local_var + 1;
    END_BLOCK
END_PROGRAM
`

/**
 * @typedef {Object} MemorySnapshot
 * @property {number} free
 * @property {number} used
 * @property {number} total
 */

/**
 * @typedef {Object} TestResult
 * @property {string} name
 * @property {boolean} passed
 * @property {MemorySnapshot[]} snapshots
 * @property {string} [note]
 */

/**
 * Get current memory state from WASM
 * @param {any} wasm 
 * @returns {MemorySnapshot}
 */
function getMemorySnapshot(wasm) {
    return {
        free: wasm.get_free_memory(),
        used: wasm.get_used_memory(),
        total: wasm.get_total_memory()
    }
}

/**
 * Check if memory is stable (no increase after stabilization point)
 * @param {MemorySnapshot[]} snapshots 
 * @returns {{ stable: boolean, note: string }}
 */
function analyzeMemoryStability(snapshots) {
    if (snapshots.length < STABLE_AFTER + 1) {
        return { stable: true, note: 'Not enough data' }
    }

    const baseline = snapshots[STABLE_AFTER - 1].used
    let maxIncrease = 0
    let increaseCount = 0

    for (let i = STABLE_AFTER; i < snapshots.length; i++) {
        const increase = snapshots[i].used - baseline
        if (increase > 0) {
            increaseCount++
            maxIncrease = Math.max(maxIncrease, increase)
        }
    }

    if (maxIncrease === 0) {
        return { stable: true, note: 'Memory stable' }
    } else if (increaseCount === 1) {
        return { stable: true, note: `Single +${maxIncrease}b spike` }
    } else {
        return { stable: false, note: `LEAK: +${maxIncrease}b over ${increaseCount} iterations` }
    }
}

/**
 * Format memory snapshots as a compact string
 * @param {MemorySnapshot[]} snapshots 
 * @returns {string}
 */
function formatSnapshots(snapshots) {
    return snapshots.map((s, i) => {
        const prev = i > 0 ? snapshots[i - 1] : null
        const diff = prev ? s.used - prev.used : 0
        const diffStr = diff > 0 ? `${BG_RED}+${diff}${RESET}` : diff < 0 ? `${BG_GREEN}${diff}${RESET}` : `${DIM}0${RESET}`
        return `${diffStr}(${s.used}/${s.total})`
    }).join(' ')
}

/**
 * Run a test multiple times and track memory
 * @param {string} name
 * @param {any} wasm
 * @param {() => void} testFn
 * @returns {TestResult}
 */
function runMemoryTest(name, wasm, testFn) {
    const snapshots = []

    // Initial snapshot
    snapshots.push(getMemorySnapshot(wasm))

    for (let i = 0; i < ITERATIONS; i++) {
        try {
            testFn()
        } catch (e) {
            log(`  ${RED}Error in iteration ${i}: ${e.message}${RESET}`)
        }
        snapshots.push(getMemorySnapshot(wasm))
    }

    const { stable, note } = analyzeMemoryStability(snapshots)

    return {
        name,
        passed: stable,
        snapshots,
        note
    }
}

/**
 * Run memory leak tests
 * @param {Object} options
 * @param {boolean} [options.silent] - Suppress output
 * @param {boolean} [options.verbose] - Show detailed memory snapshots
 * @param {VovkPLC} [options.runtime] - Shared runtime instance (creates one if not provided)
 * @returns {Promise<{name: string, passed: number, failed: number, total: number, tests: Array, failures: Array}>}
 */
export async function runTests(options = {}) {
    const { silent = false, verbose = false, runtime: sharedRuntime = null } = options
    silentMode = silent
    verboseMode = verbose
    log = silent ? () => {} : console.log.bind(console)

    // Use shared runtime if provided, otherwise create one (for standalone execution)
    let runtime = sharedRuntime
    if (!runtime) {
        runtime = new VovkPLC(wasmPath)
        await runtime.initialize(wasmPath, false, true) // silent mode
    }
    const wasm = runtime.wasm_exports

    if (!wasm.get_free_memory || !wasm.get_used_memory || !wasm.get_total_memory) {
        const error = 'Memory tracking functions not available in WASM'
        if (!silent) console.error(`${RED}Error: ${error}${RESET}`)
        return {
            name: 'Memory Leak Detection',
            passed: 0, failed: 1, total: 1,
            tests: [{ name: 'WASM Load', passed: false, error }],
            failures: [{ name: 'WASM Load', error }]
        }
    }

    log(`${BOLD}${CYAN}╔══════════════════════════════════════════════════════════════════╗${RESET}`)
    log(`${BOLD}${CYAN}║           VovkPLC Runtime - Memory Leak Test Suite               ║${RESET}`)
    log(`${BOLD}${CYAN}╚══════════════════════════════════════════════════════════════════╝${RESET}`)
    log()
    log(`${DIM}Testing with ${ITERATIONS} iterations per test, expecting stability after iteration ${STABLE_AFTER}${RESET}`)
    log(`${DIM}Format: diff(used/allocated) - Red=increase, Green=decrease${RESET}`)
    log()

    /** @type {TestResult[]} */
    const results = []

    // Helper to stream a string to WASM
    const streamString = (str) => {
        if (wasm.streamClear) wasm.streamClear()
        for (let i = 0; i < str.length; i++) {
            wasm.streamIn(str.charCodeAt(i))
        }
        wasm.streamIn(0) // Null terminator
    }

    // ═══════════════════════════════════════════════════════════════════
    // Test 1: Routine calls (baseline)
    // ═══════════════════════════════════════════════════════════════════
    log(`${CYAN}Testing routine calls...${RESET}`)
    results.push(runMemoryTest('Routine Calls', wasm, () => {
        wasm.doNothing()
    }))

    // ═══════════════════════════════════════════════════════════════════
    // Test 2: Runtime bytecode execution
    // ═══════════════════════════════════════════════════════════════════
    log(`${CYAN}Testing runtime execution...${RESET}`)
    // First compile a program
    streamString(SIMPLE_PLCASM)
    wasm.loadAssembly()
    wasm.compileAssembly(0)
    wasm.loadCompiledProgram()
    results.push(runMemoryTest('Runtime Execution', wasm, () => {
        wasm.run()
    }))

    // ═══════════════════════════════════════════════════════════════════
    // PLCASM (Lowest level - Assembly)
    // ═══════════════════════════════════════════════════════════════════
    log(`${CYAN}Testing PLCASM compilation...${RESET}`)
    results.push(runMemoryTest('PLCASM Compile', wasm, () => {
        streamString(SIMPLE_PLCASM)
        wasm.loadAssembly()
        wasm.compileAssembly(0)
    }))

    log(`${CYAN}Testing PLCASM linter...${RESET}`)
    results.push(runMemoryTest('PLCASM Lint', wasm, () => {
        streamString(SIMPLE_PLCASM)
        wasm.lint_load_assembly()
        wasm.lint_run()
    }))

    // ═══════════════════════════════════════════════════════════════════
    // STL (Statement List)
    // ═══════════════════════════════════════════════════════════════════
    log(`${CYAN}Testing STL compilation...${RESET}`)
    results.push(runMemoryTest('STL Compile', wasm, () => {
        streamString(STL_SOURCE)
        wasm.stl_load_from_stream()
        wasm.stl_compile_full()
    }))

    log(`${CYAN}Testing STL linter...${RESET}`)
    results.push(runMemoryTest('STL Lint', wasm, () => {
        streamString(STL_SOURCE)
        wasm.stl_lint_load_from_stream()
        wasm.stl_lint_run()
        wasm.stl_lint_clear()
    }))

    // ═══════════════════════════════════════════════════════════════════
    // Ladder
    // ═══════════════════════════════════════════════════════════════════
    log(`${CYAN}Testing Ladder compilation...${RESET}`)
    results.push(runMemoryTest('Ladder Compile', wasm, () => {
        wasm.ladder_standalone_clear()
        streamString(LADDER_JSON)
        wasm.ladder_standalone_load_stream()
        wasm.ladder_standalone_compile_full()
    }))

    log(`${CYAN}Testing Ladder linter...${RESET}`)
    results.push(runMemoryTest('Ladder Lint', wasm, () => {
        wasm.ladder_lint_clear()
        streamString(LADDER_JSON)
        wasm.ladder_lint_load_from_stream()
        wasm.ladder_lint_run()
    }))

    // ═══════════════════════════════════════════════════════════════════
    // PLCScript
    // ═══════════════════════════════════════════════════════════════════
    log(`${CYAN}Testing PLCScript compilation...${RESET}`)
    results.push(runMemoryTest('PLCScript Compile', wasm, () => {
        streamString(PLCSCRIPT_SOURCE)
        wasm.plcscript_load_from_stream()
        wasm.plcscript_compile()
    }))

    log(`${CYAN}Testing PLCScript linter...${RESET}`)
    results.push(runMemoryTest('PLCScript Lint', wasm, () => {
        streamString(PLCSCRIPT_SOURCE)
        wasm.plcscript_linter_load_from_stream()
        wasm.plcscript_linter_lint()
        wasm.plcscript_linter_reset()
    }))

    // ═══════════════════════════════════════════════════════════════════
    // ST (Structured Text - Highest level)
    // ═══════════════════════════════════════════════════════════════════
    log(`${CYAN}Testing ST compilation...${RESET}`)
    results.push(runMemoryTest('ST Compile', wasm, () => {
        streamString(ST_SOURCE)
        wasm.st_compiler_load_from_stream()
        wasm.st_compiler_compile()
    }))

    log(`${CYAN}Testing ST linter...${RESET}`)
    results.push(runMemoryTest('ST Lint', wasm, () => {
        streamString(ST_SOURCE)
        wasm.st_lint_load_from_stream()
        wasm.st_lint_run()
        wasm.st_lint_clear()
    }))

    // ═══════════════════════════════════════════════════════════════════
    // Project (All languages combined)
    // ═══════════════════════════════════════════════════════════════════
    log(`${CYAN}Testing Project compilation (all languages)...${RESET}`)
    results.push(runMemoryTest('Project Compile (All)', wasm, () => {
        wasm.project_reset()
        streamString(PROJECT_ALL_LANGUAGES)
        wasm.project_compile(0)
    }))

    // Print results
    log()
    log(`${BOLD}Test Results:${RESET}`)
    log()

    const nameWidth = Math.max(...results.map(r => r.name.length)) + 2

    for (const result of results) {
        const status = result.passed ? `${GREEN}✓${RESET}` : `${RED}✗${RESET}`
        const name = result.name.padEnd(nameWidth)
        const finalSnapshot = result.snapshots[result.snapshots.length - 1]
        const memUsage = `${DIM}${finalSnapshot.used}/${finalSnapshot.total}b${RESET}`
        const note = result.note ? `${DIM}─${RESET} ${result.passed ? CYAN : RED}${result.note}${RESET}` : ''
        log(`  ${status} ${name} ${memUsage} ${note}`)
        
        if (verboseMode) {
            log(`    ${DIM}Memory:${RESET} ${formatSnapshots(result.snapshots)}`)
        }
    }

    // Summary
    log()
    log(`${'─'.repeat(70)}`)
    log()

    const passed = results.filter(r => r.passed).length
    const failed = results.filter(r => !r.passed).length

    if (failed === 0) {
        log(`${BOLD}Summary:${RESET}`)
        log(`  ${GREEN}✓${RESET} ${passed}/${results.length} tests passed - No memory leaks detected`)
    } else {
        log(`${BOLD}Summary:${RESET}`)
        log(`  ${RED}✗${RESET} ${failed}/${results.length} tests failed - Memory leaks detected!`)
        log()
        log(`${YELLOW}Failing tests:${RESET}`)
        for (const result of results.filter(r => !r.passed)) {
            log(`  ${RED}•${RESET} ${result.name}: ${result.note}`)
            log(`    ${DIM}Memory:${RESET} ${formatSnapshots(result.snapshots)}`)
        }
    }

    log()

    // Final memory state
    const finalMem = getMemorySnapshot(wasm)
    log(`${DIM}Final memory state: ${finalMem.used}/${finalMem.total} bytes used${RESET}`)

    // Convert results to standard test format
    const testResults = results.map(r => {
        const finalSnapshot = r.snapshots[r.snapshots.length - 1]
        return {
            name: r.name,
            passed: r.passed,
            error: r.passed ? undefined : r.note,
            info: `${finalSnapshot.used}/${finalSnapshot.total}b ${r.note || ''}`
        }
    })

    const failures = results.filter(r => !r.passed).map(r => ({
        name: r.name,
        error: r.note
    }))

    return {
        name: 'Memory Leak Detection',
        passed,
        failed,
        total: results.length,
        tests: testResults,
        failures
    }
}

// Main entry point when run directly
import { pathToFileURL } from 'url'
const isMainModule = import.meta.url === pathToFileURL(process.argv[1]).href

if (isMainModule) {
    const verbose = process.argv.includes('--verbose') || process.argv.includes('-v')
    runTests({ verbose }).then(result => {
        process.exit(result.failed > 0 ? 1 : 0)
    }).catch(err => {
        console.error(`${RED}Fatal error: ${err.message}${RESET}`)
        console.error(err.stack)
        process.exit(1)
    })
}