#!/usr/bin/env node
// unit_test.js - Unified test runner for all VovkPLC unit tests
//
// This script runs all sub-tests and displays results as a tree structure.
// All test suites share the same WASM instance for efficiency and to catch
// any state leakage between tests.
//
// Usage:
//   node wasm/unit_test.js [--short] [--debug] [--filter=<pattern>]
//   npm test           # Detailed test tree (default)
//   npm run test:short # Overview only
//   npm run test:debug # Full compiler debug output
//
// Options:
//   --short      Show only suite overview (pass/fail counts)
//   --debug      Show full compiler debug output (implies verbose)
//   --filter=X   Only run test suites matching pattern X (e.g., --filter=ladder)

import VovkPLC from './dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'

import { runTests as runLadderSTLTests } from './node-test/ladder-tests/test_ladder_stl.js'
import { runTests as runLadderLinterTests } from './node-test/ladder-tests/test_ladder_linter.js'
import { runTests as runProjectTests } from './node-test/project-tests/unit_test.js'
import { runTests as runPLCScriptTests } from './node-test/stl-tests/test_plcscript.js'
import { runTests as runStructTests } from './node-test/plcscript-tests/test_structs.js'
import { runTests as runPLCScriptStringTests } from './node-test/plcscript-tests/test_strings.js'
import { runTests as runSTTests } from './node-test/st-tests/test_st.js'
import { runTests as runReservedWordTests } from './node-test/project-tests/test_reserved_words.js'
import { runTests as runArrayTests } from './node-test/test_arrays.js'
import { runTests as runSafeModeTests } from './node-test/test_safe_mode.js'
import { runTests as runFFITests } from './node-test/test_ffi.js'
import { runTests as runMemoryLeakTests } from './memory_leak_test.js'
import { runTests as runStringTests } from './node-test/test_string.js'
import { runTests as runCaseInsensitiveTests } from './node-test/test_case_insensitive.js'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

// ANSI colors
const RED = '\x1b[31m'
const GREEN = '\x1b[32m'
const YELLOW = '\x1b[33m'
const CYAN = '\x1b[36m'
const DIM = '\x1b[2m'
const RESET = '\x1b[0m'
const BOLD = '\x1b[1m'

// Parse CLI args
const args = process.argv.slice(2)
const debugMode = args.includes('--debug') || args.includes('-d')
const shortMode = args.includes('--short') || args.includes('-s')
const verboseMode = !shortMode // verbose is default, --short disables it
const filterArg = args.find(a => a.startsWith('--filter='))
const filter = filterArg ? filterArg.split('=')[1].toLowerCase() : null

// ═══════════════════════════════════════════════════════════════════════════
// Shared WASM Runtime Instance
// ═══════════════════════════════════════════════════════════════════════════
// All test suites share the same WASM instance to:
// 1. Improve performance by avoiding repeated WASM compilation
// 2. Catch any state leakage between tests (memory corruption, etc.)
// 3. Simulate real-world usage where a single runtime handles many programs

let sharedRuntime = null

/**
 * Get or create the shared VovkPLC runtime instance.
 * This is passed to all test suites so they share the same WASM instance.
 * When tests run standalone, they create their own instance as fallback.
 * @returns {Promise<VovkPLC>}
 */
async function getSharedRuntime() {
    if (sharedRuntime) return sharedRuntime
    
    const wasmPath = path.resolve(__dirname, 'dist/VovkPLC.wasm')
    sharedRuntime = new VovkPLC(wasmPath)
    sharedRuntime.stdout_callback = () => {} // Suppress output
    await sharedRuntime.initialize(wasmPath, false, true)
    return sharedRuntime
}

// Test suite definitions
const testSuites = [
    {
        name: 'Ladder Graph → STL Compiler',
        shortName: 'ladder-stl',
        run: runLadderSTLTests
    },
    {
        name: 'Ladder Linter',
        shortName: 'ladder-linter',
        run: runLadderLinterTests
    },
    {
        name: 'Project Compiler',
        shortName: 'project',
        run: runProjectTests
    },
    {
        name: 'PLCScript Compiler & Runtime',
        shortName: 'plcscript',
        run: runPLCScriptTests
    },
    {
        name: 'Structured Text (ST) Compiler',
        shortName: 'st',
        run: runSTTests
    },
    {
        name: 'Reserved Word Validation',
        shortName: 'reserved',
        run: runReservedWordTests
    },
    {
        name: 'Typed Structs Integration',
        shortName: 'structs',
        run: runStructTests
    },
    {
        name: 'PLCScript String Support',
        shortName: 'plcscript-strings',
        run: runPLCScriptStringTests
    },
    {
        name: 'Array Integration Tests',
        shortName: 'arrays',
        run: runArrayTests
    },
    {
        name: 'Safe Mode Bounds Checking',
        shortName: 'safe-mode',
        run: runSafeModeTests
    },
    {
        name: 'FFI & Memory Protection',
        shortName: 'ffi',
        run: runFFITests
    },
    {
        name: 'String Operations (str8/str16)',
        shortName: 'string',
        run: runStringTests
    },
    {
        name: 'Case-Insensitive PLCASM',
        shortName: 'case-insensitive',
        run: runCaseInsensitiveTests
    },
    {
        name: 'Memory Leak Detection',
        shortName: 'memory',
        run: runMemoryLeakTests
    },
]

// Tree drawing characters
const TREE = {
    BRANCH: '├──',
    LAST_BRANCH: '└──',
    VERTICAL: '│  ',
    SPACE: '   '
}

/**
 * @typedef {Object} TestResult
 * @property {string} name - Test name
 * @property {boolean} passed - Whether the test passed
 * @property {string} [error] - Error message if failed
 * @property {string} [info] - Additional info (e.g., expected errors count)
 */

/**
 * @typedef {Object} SuiteResult
 * @property {string} name - Suite name
 * @property {number} passed - Number of passed tests
 * @property {number} failed - Number of failed tests
 * @property {number} total - Total number of tests
 * @property {TestResult[]} tests - Individual test results
 * @property {Object[]} [failures] - Detailed failure information
 */

/**
 * Format a duration in milliseconds to a human-readable string
 * @param {number} ms 
 * @returns {string}
 */
function formatDuration(ms) {
    if (ms < 1000) return `${ms}ms`
    return `${(ms / 1000).toFixed(2)}s`
}

/**
 * Print a test suite result as a tree
 * @param {SuiteResult} result 
 * @param {boolean} isLast 
 * @param {boolean} verbose 
 */
function printSuiteTree(result, isLast, verbose) {
    const branch = isLast ? TREE.LAST_BRANCH : TREE.BRANCH
    const statusColor = result.failed > 0 ? RED : GREEN
    const statusIcon = result.failed > 0 ? '✗' : '✓'
    
    console.log(`${branch} ${statusColor}${statusIcon}${RESET} ${BOLD}${result.name}${RESET} ${DIM}(${result.passed}/${result.total})${RESET}`)
    
    const prefix = isLast ? TREE.SPACE : TREE.VERTICAL
    
    if (verbose || result.failed > 0) {
        // Print individual test results
        for (let i = 0; i < result.tests.length; i++) {
            const test = result.tests[i]
            const testBranch = i === result.tests.length - 1 ? TREE.LAST_BRANCH : TREE.BRANCH
            const testColor = test.passed ? GREEN : RED
            const testIcon = test.passed ? '✓' : '✗'
            
            let testLine = `${prefix}${testBranch} ${testColor}${testIcon}${RESET} ${test.name}`
            
            // Always show info/details to the right of the test name
            if (test.info) {
                testLine += ` ${DIM}─${RESET} ${CYAN}${test.info}${RESET}`
            }
            
            // In non-verbose mode, only show failed tests
            if (verbose || !test.passed) {
                console.log(testLine)
                
                // Show error details for failed tests
                if (!test.passed && test.error) {
                    const errorPrefix = i === result.tests.length - 1 ? TREE.SPACE : TREE.VERTICAL
                    console.log(`${prefix}${errorPrefix}   ${RED}${test.error}${RESET}`)
                }
            }
        }
    }
}

/**
 * Print detailed failures for a suite
 * @param {SuiteResult} result 
 */
function printDetailedFailures(result) {
    if (!result.failures || result.failures.length === 0) return
    
    console.log()
    console.log(`${BOLD}${RED}Failures in ${result.name}:${RESET}`)
    
    for (const failure of result.failures) {
        console.log()
        console.log(`  ${BOLD}${failure.name}:${RESET}`)
        
        if (failure.error) {
            console.log(`    ${RED}Error: ${failure.error}${RESET}`)
        }
        
        if (failure.errors && failure.errors.length > 0) {
            for (const err of failure.errors) {
                console.log(`    ${RED}• ${err}${RESET}`)
            }
        }
        
        if (failure.diffs && failure.diffs.length > 0) {
            console.log(`    Differences:`)
            for (const d of failure.diffs) {
                if (d.field) {
                    console.log(`      ${d.field}:`)
                    console.log(`        expected: ${RED}${JSON.stringify(d.expected)}${RESET}`)
                    console.log(`        actual:   ${YELLOW}${JSON.stringify(d.actual)}${RESET}`)
                } else if (d.type === '-') {
                    console.log(`      ${RED}- ${d.line}${RESET}`)
                } else if (d.type === '+') {
                    console.log(`      ${YELLOW}+ ${d.line}${RESET}`)
                }
            }
        }
        
        if (failure.expected !== undefined && failure.actual !== undefined && !failure.diffs) {
            console.log(`    Expected: ${failure.expected.errors} errors, ${failure.expected.warnings}+ warnings`)
            console.log(`    Actual:   ${failure.actual.errors} errors, ${failure.actual.warnings} warnings`)
            if (failure.problems && failure.problems.length > 0) {
                console.log(`    Problems:`)
                for (const p of failure.problems) {
                    const color = p.type === 'error' ? RED : p.type === 'warning' ? YELLOW : CYAN
                    console.log(`      ${color}[${p.type}]${RESET} (${p.line}:${p.column}) ${p.message}`)
                }
            }
        }
    }
}

async function main() {
    const startTime = Date.now()
    
    console.log()
    console.log(`${BOLD}${CYAN}╔══════════════════════════════════════════════════════════════════╗${RESET}`)
    console.log(`${BOLD}${CYAN}║           VovkPLC Runtime - Unit Test Suite                      ║${RESET}`)
    console.log(`${BOLD}${CYAN}╚══════════════════════════════════════════════════════════════════╝${RESET}`)
    console.log()
    
    // Initialize shared runtime once for all test suites
    const runtime = await getSharedRuntime()
    
    // Filter test suites if requested
    const suitesToRun = filter 
        ? testSuites.filter(s => s.shortName.includes(filter) || s.name.toLowerCase().includes(filter))
        : testSuites
    
    if (suitesToRun.length === 0) {
        console.log(`${YELLOW}No test suites match filter: ${filter}${RESET}`)
        process.exit(0)
    }
    
    if (filter) {
        console.log(`${DIM}Running ${suitesToRun.length} suite(s) matching: ${filter}${RESET}`)
        console.log()
    }
    
    /** @type {SuiteResult[]} */
    const results = []
    let totalPassed = 0
    let totalFailed = 0
    let totalTests = 0
    
    // Run each test suite with the shared runtime
    for (const suite of suitesToRun) {
        process.stdout.write(`${DIM}Running ${suite.name}...${RESET}\r`)
        
        try {
            // Pass the shared runtime to each test suite
            // verbose=true shows compiler debug output, only enabled with --debug
            const result = await suite.run({ silent: true, verbose: debugMode, runtime })
            results.push(result)
            totalPassed += result.passed
            totalFailed += result.failed
            totalTests += result.total
            
            // Clear the "Running..." line
            process.stdout.write(' '.repeat(60) + '\r')
        } catch (err) {
            // Clear the "Running..." line
            process.stdout.write(' '.repeat(60) + '\r')
            
            results.push({
                name: suite.name,
                passed: 0,
                failed: 1,
                total: 1,
                tests: [{ name: 'Suite Error', passed: false, error: err.message }],
                failures: [{ name: 'Suite Error', error: err.message }]
            })
            totalFailed++
            totalTests++
        }
    }
    
    const duration = Date.now() - startTime
    
    // Print results tree
    console.log(`${BOLD}Test Results:${RESET}`)
    console.log()
    
    for (let i = 0; i < results.length; i++) {
        printSuiteTree(results[i], i === results.length - 1, verboseMode)
    }
    
    // Print detailed failures if any
    const failedSuites = results.filter(r => r.failed > 0)
    if (failedSuites.length > 0 && !verboseMode) {
        console.log()
        console.log('─'.repeat(68))
        for (const suite of failedSuites) {
            printDetailedFailures(suite)
        }
    }
    
    // Summary
    console.log()
    console.log('─'.repeat(68))
    console.log()
    
    const statusColor = totalFailed > 0 ? RED : GREEN
    const statusIcon = totalFailed > 0 ? '✗' : '✓'
    
    console.log(`${BOLD}Summary:${RESET}`)
    console.log(`  ${statusColor}${statusIcon}${RESET} ${totalPassed}/${totalTests} tests passed across ${results.length} suite(s)`)
    console.log(`  ${DIM}Duration: ${formatDuration(duration)}${RESET}`)
    
    if (totalFailed > 0) {
        console.log()
        console.log(`  ${RED}${totalFailed} test(s) failed${RESET}`)
    }
    
    console.log()
    
    process.exit(totalFailed > 0 ? 1 : 0)
}

main().catch(err => {
    console.error(`${RED}Fatal error: ${err.message}${RESET}`)
    console.error(err.stack)
    process.exit(1)
})
