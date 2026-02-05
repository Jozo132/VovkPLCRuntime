#!/usr/bin/env node
// test_ladder_stl.js - Automated Ladder Graph to STL unit tests
//
// This script tests the Ladder Graph → STL compiler by comparing
// the generated STL output against expected .stl files.
//
// Usage:
//   node wasm/node-test/ladder-tests/test_ladder_stl.js
//   npm run test:ladder
//
// Test files:
//   - test_XX.json - Ladder Graph JSON input
//   - test_XX.stl  - Expected STL output

import VovkPLC from '../../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

// Check if running as main module or imported
const isMainModule = process.argv[1] && (
    process.argv[1].endsWith('test_ladder_stl.js') ||
    process.argv[1].includes('test_ladder_stl')
)

// ANSI colors
const RED = '\x1b[31m'
const GREEN = '\x1b[32m'
const YELLOW = '\x1b[33m'
const CYAN = '\x1b[36m'
const DIM = '\x1b[2m'
const RESET = '\x1b[0m'
const BOLD = '\x1b[1m'

// Stream code to runtime (synchronous for direct VovkPLC)
function streamCode(runtime, code) {
    for (let i = 0; i < code.length; i++) {
        runtime.wasm_exports.streamIn(code.charCodeAt(i))
    }
    runtime.wasm_exports.streamIn(0)
}

// Normalize STL for comparison (trim lines, remove empty lines, normalize whitespace)
function normalizeSTL(stl) {
    return stl
        .split('\n')
        .map(line => line.trim())
        .filter(line => line.length > 0)
        .join('\n')
}

// Check STL code for stack leaks by compiling and running it
// Returns { success: boolean, error?: string, stackDepth?: number }
function checkSTLForStackLeak(runtime, stlCode) {
    try {
        // Clear the runtime stack
        if (runtime.wasm_exports.clearStack) {
            runtime.wasm_exports.clearStack()
        }
        
        // Compile STL to PLCASM
        runtime.wasm_exports.streamClear()
        streamCode(runtime, stlCode)
        runtime.wasm_exports.stl_load_from_stream()
        const stlCompileSuccess = runtime.wasm_exports.stl_compile()
        
        if (!stlCompileSuccess) {
            return { success: false, error: 'STL compilation failed' }
        }
        
        runtime.wasm_exports.stl_output_to_stream()
        const plcasm = runtime.readOutBuffer()
        
        // Compile PLCASM to bytecode
        runtime.downloadAssembly(plcasm)
        const compileError = runtime.wasm_exports.compileAssembly(false)
        
        if (compileError) {
            return { success: false, error: 'PLCASM compilation failed' }
        }
        
        // Get bytecode size before loading program
        const bytecodeSize = runtime.wasm_exports.getCompiledBytecodeLength ? 
            runtime.wasm_exports.getCompiledBytecodeLength() : 0
        
        // Load and run the program
        runtime.wasm_exports.loadCompiledProgram()
        runtime.wasm_exports.run()
        
        // Check stack depth
        const stackDepth = runtime.wasm_exports.getStackSize()
        
        if (stackDepth !== 0) {
            return { success: false, error: `Stack leak: ${stackDepth} byte(s) left on stack`, stackDepth, bytecodeSize }
        }
        
        return { success: true, stackDepth: 0, bytecodeSize }
    } catch (e) {
        return { success: false, error: e.message }
    }
}

// Compare two STL strings and show differences using unified diff style
function compareSTL(expected, actual) {
    const expectedLines = expected.split('\n').map(l => l.trim()).filter(l => l)
    const actualLines = actual.split('\n').map(l => l.trim()).filter(l => l)
    
    // Build a simple unified diff
    const diff = []
    const maxLen = Math.max(expectedLines.length, actualLines.length)
    
    // Find lines only in expected (removed)
    for (let i = 0; i < expectedLines.length; i++) {
        if (!actualLines.includes(expectedLines[i])) {
            diff.push({ type: '-', line: expectedLines[i], lineNum: i + 1 })
        }
    }
    
    // Find lines only in actual (added)  
    for (let i = 0; i < actualLines.length; i++) {
        if (!expectedLines.includes(actualLines[i])) {
            diff.push({ type: '+', line: actualLines[i], lineNum: i + 1 })
        }
    }
    
    return diff
}

/**
 * @typedef {Object} TestResult
 * @property {string} name - Test name
 * @property {boolean} passed - Whether the test passed
 * @property {string} [error] - Error message if failed
 * @property {string} [info] - Additional info
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
 * Run Ladder → STL unit tests
 * @param {Object} [options] - Options
 * @param {boolean} [options.silent] - If true, suppress console output
 * @param {boolean} [options.verbose] - If true, show verbose output
 * @param {VovkPLC} [options.runtime] - Shared runtime instance (creates one if not provided)
 * @returns {Promise<SuiteResult>}
 */
export async function runTests(options = {}) {
    const { silent = false, verbose = false, runtime: sharedRuntime = null } = options
    const log = silent ? () => {} : console.log.bind(console)
    const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')
    
    /** @type {TestResult[]} */
    const testResults = []
    /** @type {Object[]} */
    const failures = []
    let passed = 0
    let failed = 0
    
    if (!fs.existsSync(wasmPath)) {
        const error = `WASM file not found at ${wasmPath}`
        if (!silent) {
            console.error(`${RED}Error: ${error}${RESET}`)
            console.error('Run "npm run build" first.')
        }
        return {
            name: 'Ladder Graph → STL Compiler',
            passed: 0, failed: 1, total: 1,
            tests: [{ name: 'WASM Load', passed: false, error }],
            failures: [{ name: 'WASM Load', error }]
        }
    }

    // Use shared runtime if provided, otherwise create one (for standalone execution)
    let runtime = sharedRuntime
    if (!runtime) {
        runtime = new VovkPLC(wasmPath)
        runtime.stdout_callback = () => {}
        await runtime.initialize(wasmPath, false, false)
        runtime.readStream()
    }
    
    // Find all test files
    const testDir = __dirname
    const files = fs.readdirSync(testDir)
    const testCases = files
        .filter(f => f.match(/^test_\d+\.json$/))
        .map(f => f.replace('.json', ''))
        .sort()
    
    if (testCases.length === 0) {
        if (!silent) console.error(`${RED}No test cases found in ${testDir}${RESET}`)
        return {
            name: 'Ladder Graph → STL Compiler',
            passed: 0, failed: 1, total: 1,
            tests: [{ name: 'Find Tests', passed: false, error: 'No test cases found' }],
            failures: [{ name: 'Find Tests', error: 'No test cases found' }]
        }
    }
    
    log(`${BOLD}${CYAN}╔══════════════════════════════════════════════════════════════════╗${RESET}`)
    log(`${BOLD}${CYAN}║           Ladder Graph → STL Compiler Unit Tests                 ║${RESET}`)
    log(`${BOLD}${CYAN}╚══════════════════════════════════════════════════════════════════╝${RESET}`)
    log()
    log(`Found ${testCases.length} test case(s)`)
    log()
    
    for (const testCase of testCases) {
        const jsonPath = path.join(testDir, `${testCase}.json`)
        const stlPath = path.join(testDir, `${testCase}.stl`)
        
        // Clear the runtime stack before each test
        if (runtime.wasm_exports.clearStack) {
            runtime.wasm_exports.clearStack()
        }
        
        const ladderJson = fs.readFileSync(jsonPath, 'utf8')
        
        try {
            // Compile ladder graph to STL
            streamCode(runtime, ladderJson)
            runtime.wasm_exports.ladder_standalone_load_stream()
            
            const success = runtime.wasm_exports.ladder_standalone_compile()
            
            if (!success) {
                log(`${RED}✗${RESET} ${testCase} - Ladder compilation failed`)
                failed++
                testResults.push({ name: testCase, passed: false, error: 'Ladder compilation failed' })
                failures.push({ name: testCase, error: 'Ladder compilation failed' })
                continue
            }
            
            runtime.wasm_exports.ladder_standalone_output_to_stream()
            const actualSTL = runtime.readOutBuffer()
            
            // Check if this is an expected stack leak test (marked in JSON)
            const isExpectedLeakTest = ladderJson.includes('"expect_stack_leak"')
            
            // Check if expected STL file exists - if not, verify and generate it
            if (!fs.existsSync(stlPath)) {
                // Before saving, check for stack leaks (unless it's an expected leak test)
                if (!isExpectedLeakTest) {
                    const leakCheck = checkSTLForStackLeak(runtime, actualSTL)
                    if (!leakCheck.success) {
                        log(`${RED}✗${RESET} ${testCase} - Cannot generate .stl file: ${leakCheck.error}`)
                        log(`  Generated STL:`)
                        actualSTL.split('\n').forEach((line, i) => log(`    ${i+1}| ${line}`))
                        failed++
                        testResults.push({ name: testCase, passed: false, error: `Generated STL has error: ${leakCheck.error}` })
                        failures.push({ name: testCase, error: `Generated STL has error: ${leakCheck.error}`, actual: actualSTL })
                        continue
                    }
                }
                fs.writeFileSync(stlPath, actualSTL)
                log(`${CYAN}+${RESET} ${testCase} - Generated expected .stl file`)
                passed++
                testResults.push({ name: testCase, passed: true, info: '(generated)' })
                continue
            }
            
            const expectedSTL = fs.readFileSync(stlPath, 'utf8')
            
            // Compare normalized STL
            const normalizedExpected = normalizeSTL(expectedSTL)
            const normalizedActual = normalizeSTL(actualSTL)
            
            // Special case: if generated STL is empty/minimal, use the expected STL for stack testing
            // This allows testing raw PLCASM or manually written STL for stack leaks
            const stlToTest = normalizedActual.length <= 50 ? expectedSTL : actualSTL
            const isRawTest = normalizedActual.length <= 50
            
            if (!isRawTest && normalizedExpected !== normalizedActual) {
                log(`${RED}✗${RESET} ${testCase} - STL output mismatch`)
                const diffs = compareSTL(expectedSTL, actualSTL)
                testResults.push({ name: testCase, passed: false, error: 'STL output mismatch' })
                failures.push({ name: testCase, diffs, expected: expectedSTL, actual: actualSTL })
                failed++
                continue
            }
            
            // Check for stack leaks by compiling and running the STL
            const leakCheck = checkSTLForStackLeak(runtime, stlToTest)
            
            if (!leakCheck.success) {
                if (isExpectedLeakTest) {
                    // This test expects a stack leak - it's a negative test
                    const info = `expected leak: ${leakCheck.stackDepth}B` + (leakCheck.bytecodeSize ? `, ${leakCheck.bytecodeSize} bytes` : '')
                    log(`${GREEN}✓${RESET} ${testCase} ${CYAN}(${info})${RESET}`)
                    passed++
                    testResults.push({ name: testCase, passed: true, info })
                    continue
                }
                log(`${RED}✗${RESET} ${testCase} - ${leakCheck.error}`)
                failed++
                testResults.push({ name: testCase, passed: false, error: leakCheck.error })
                failures.push({ name: testCase, error: leakCheck.error })
                continue
            }
            
            const info = leakCheck.bytecodeSize ? `${leakCheck.bytecodeSize} bytes` : undefined
            log(`${GREEN}✓${RESET} ${testCase}` + (info ? ` ${DIM}(${info})${RESET}` : ''))
            passed++
            testResults.push({ name: testCase, passed: true, info })
        } catch (e) {
            log(`${RED}✗${RESET} ${testCase} - Error: ${e.message}`)
            failed++
            testResults.push({ name: testCase, passed: false, error: e.message })
            failures.push({ name: testCase, error: e.message })
        }
    }
    
    log()
    log('─'.repeat(68))
    
    // Show detailed failures
    if (failures.length > 0 && !silent) {
        log()
        log(`${BOLD}${RED}Failed Tests:${RESET}`)
        log()
        
        for (const failure of failures) {
            log(`${BOLD}${failure.name}:${RESET}`)
            if (failure.error) {
                log(`  Error: ${failure.error}`)
            } else if (failure.diffs && failure.diffs.length > 0) {
                log(`  Changes:`)
                // Show removed lines (in expected but not actual)
                const removed = failure.diffs.filter(d => d.type === '-')
                const added = failure.diffs.filter(d => d.type === '+')
                
                if (removed.length > 0) {
                    log(`    ${RED}Missing (expected but not in output):${RESET}`)
                    for (const d of removed.slice(0, 10)) {
                        log(`      ${RED}- ${d.line}${RESET}`)
                    }
                    if (removed.length > 10) log(`      ... and ${removed.length - 10} more`)
                }
                
                if (added.length > 0) {
                    log(`    ${YELLOW}Extra (in output but not expected):${RESET}`)
                    for (const d of added.slice(0, 10)) {
                        log(`      ${YELLOW}+ ${d.line}${RESET}`)
                    }
                    if (added.length > 10) log(`      ... and ${added.length - 10} more`)
                }
            } else if (failure.expected && failure.actual) {
                log(`  Expected:\n${failure.expected}`)
                log(`  Actual:\n${failure.actual}`)
            }
            log()
        }
    }
    
    // Summary
    const total = passed + failed
    const statusColor = failed > 0 ? RED : GREEN
    log()
    log(`${BOLD}Results: ${statusColor}${passed}/${total} tests passed${RESET}`)
    
    return {
        name: 'Ladder Graph → STL Compiler',
        passed,
        failed,
        total,
        tests: testResults,
        failures
    }
}

// Run as main module
if (isMainModule) {
    runTests().then(result => {
        if (result.failed > 0) process.exit(1)
    }).catch(err => {
        console.error(`${RED}Error: ${err.message}${RESET}`)
        process.exit(1)
    })
}
