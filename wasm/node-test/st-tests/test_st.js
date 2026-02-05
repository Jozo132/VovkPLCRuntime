#!/usr/bin/env node
// test_st.js - IEC 61131-3 Structured Text (ST) to PLCScript Compiler Unit Tests
//
// This script tests the ST compiler by:
//   1. ST → PLCScript transpilation (checks expected tokens)
//   2. Optional full pipeline: PLCScript → PLCASM → Bytecode execution
//
// Usage:
//   node wasm/node-test/st-tests/test_st.js [test_name] [--verbose] [--update]
//   npm run test_st
//
// Examples:
//   node test_st.js                    # Run all tests
//   node test_st.js test_04            # Run only test_04_all_types
//   node test_st.js arithmetic         # Run tests matching "arithmetic"
//   node test_st.js test_04 --verbose  # Show full PLCScript output
//   node test_st.js --update           # Regenerate all .plcscript expected files
//   node test_st.js test_04 --update   # Regenerate expected output for test_04
//
// Test files (in st-samples/):
//   - test_XX_name.st         - Structured Text source input
//   - test_XX_name.plcscript  - Expected PLCScript tokens (one per line)
//
// .plcscript file format:
//   # Comments start with #
//   # Required tokens (one per line, must appear in PLCScript output):
//   counter: i16
//   flag: bool
//   if (flag)

import VovkPLC from '../../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

// Check if running as main module or imported
const isMainModule = process.argv[1] && (
    process.argv[1].endsWith('test_st.js') ||
    process.argv[1].includes('test_st')
)

// ANSI colors
const RED = '\x1b[31m'
const GREEN = '\x1b[32m'
const YELLOW = '\x1b[33m'
const CYAN = '\x1b[36m'
const RESET = '\x1b[0m'
const BOLD = '\x1b[1m'
const DIM = '\x1b[2m'

// Helper to read string from WASM memory
const getString = (runtime, ptr) => {
    if (!ptr) return ''
    const mem = new Uint8Array(runtime.wasm.exports.memory.buffer)
    let str = ''
    let i = ptr
    while (mem[i] !== 0 && i < mem.length) {
        str += String.fromCharCode(mem[i])
        i++
    }
    return str
}

// Stream code to runtime (synchronous for direct VovkPLC)
const streamCode = (runtime, code) => {
    for (let i = 0; i < code.length; i++) {
        runtime.wasm_exports.streamIn(code.charCodeAt(i))
    }
    // Note: Do NOT stream null terminator - streamRead will add it
}

// Compile ST to PLCScript
const compileST = (runtime, source, verbose = false, log = console.log.bind(console)) => {
    // Clear and stream source
    runtime.wasm_exports.streamClear()
    streamCode(runtime, source)
    
    if (verbose) {
        log(`Streamed ${source.length} bytes`)
        log(`Stream available: ${runtime.wasm_exports.streamAvailable()}`)
    }
    
    // Load from stream
    runtime.wasm_exports.st_compiler_load_from_stream()
    
    if (verbose) {
        log(`After load, stream available: ${runtime.wasm_exports.streamAvailable()}`)
        log(`Input length after load: ${runtime.wasm_exports.st_compiler_get_input_length()}`)
        // Debug: check input buffer contents
        const inputPreview = getString(runtime, runtime.wasm_exports.st_compiler_debug_get_input_preview())
        log(`Input preview: "${inputPreview.substring(0, 50)}..."`)
    }
    
    // Compile
    const errorCode = runtime.wasm_exports.st_compiler_compile()
    
    if (verbose) {
        log(`Compile returned: ${errorCode}`)
        log(`Has error: ${runtime.wasm_exports.st_compiler_has_error()}`)
        log(`Output length: ${runtime.wasm_exports.st_compiler_get_output_length()}`)
        if (runtime.wasm_exports.st_compiler_has_error()) {
            const errTokenText = getString(runtime, runtime.wasm_exports.st_compiler_get_error_token_text())
            const errTokenType = runtime.wasm_exports.st_compiler_get_error_token_type()
            log(`Error token: "${errTokenText}" (type ${errTokenType})`)
        }
    }
    
    if (errorCode !== 0) {
        const hasError = runtime.wasm_exports.st_compiler_has_error()
        
        if (verbose) {
            // Get the output to see what was emitted
            runtime.wasm_exports.flush_out_buffer()
            runtime.wasm_exports.st_compiler_output_to_stream()
            const outBufferPtr = runtime.wasm_exports.get_out_buffer_ptr()
            const outIndex = runtime.wasm_exports.get_out_index()
            const outMem = new Uint8Array(runtime.wasm.exports.memory.buffer)
            let output = ''
            for (let i = 0; i < outIndex; i++) {
                output += String.fromCharCode(outMem[outBufferPtr + i])
            }
            log(`Output so far:\n${output}`)
        }
        
        if (hasError) {
            const errorMsg = getString(runtime, runtime.wasm_exports.st_compiler_get_error())
            const errorLine = runtime.wasm_exports.st_compiler_get_error_line()
            const errorCol = runtime.wasm_exports.st_compiler_get_error_column()
            return { error: `${errorMsg} at line ${errorLine}, col ${errorCol}` }
        }
        return { error: 'ST compilation failed (unknown error)' }
    }
    
    // Get output via stream
    runtime.wasm_exports.flush_out_buffer()
    runtime.wasm_exports.st_compiler_output_to_stream()
    
    const outBufferPtr = runtime.wasm_exports.get_out_buffer_ptr()
    const outIndex = runtime.wasm_exports.get_out_index()
    const outMem = new Uint8Array(runtime.wasm.exports.memory.buffer)
    
    let plcscript = ''
    for (let i = 0; i < outIndex; i++) {
        plcscript += String.fromCharCode(outMem[outBufferPtr + i])
    }
    
    return { output: plcscript }
}

// Parse expected output from .plcscript file
const parseExpectedOutput = (content) => {
    const lines = content.split('\n')
    const tokens = []
    
    for (const line of lines) {
        const trimmed = line.trim()
        if (!trimmed || trimmed.startsWith('#')) continue
        tokens.push(trimmed)
    }
    
    return { tokens }
}

// Generate expected output file content
const generateExpectedOutput = (testName, tokens) => {
    let content = `# Expected PLCScript output for ${testName}\n`
    content += `# Auto-generated - edit tokens as needed\n`
    content += `#\n`
    content += `# Required PLCScript tokens (must appear in transpiled output):\n`
    for (const token of tokens) {
        content += `${token}\n`
    }
    return content
}

// Format test name from filename
const formatTestName = (filename) => {
    return filename
        .replace(/^test_\d+_/, '')
        .replace(/_/g, ' ')
        .replace(/\b\w/g, c => c.toUpperCase())
}

// Extract representative tokens from PLCScript output (for auto-generation)
const extractTokens = (plcscript) => {
    const tokens = []
    const lines = plcscript.split('\n')
    
    for (const line of lines) {
        const trimmed = line.trim()
        if (!trimmed) continue
        
        // Extract variable declarations (let x: type)
        const varMatch = trimmed.match(/let\s+(\w+):\s*(\w+)/)
        if (varMatch) {
            tokens.push(`${varMatch[1]}: ${varMatch[2]}`)
            continue
        }
        
        // Extract assignments
        if (trimmed.includes(' = ') && !trimmed.startsWith('let ')) {
            tokens.push(trimmed)
            continue
        }
        
        // Extract control flow keywords
        if (trimmed.startsWith('if (') || trimmed.startsWith('while (') || 
            trimmed.startsWith('for (') || trimmed.startsWith('do {') ||
            trimmed === '} else {' || trimmed.startsWith('else if (') ||
            trimmed === 'break;' || trimmed === 'return;') {
            tokens.push(trimmed)
            continue
        }
        
        // Extract do-while conditions
        if (trimmed.startsWith('} while (')) {
            tokens.push(trimmed)
        }
    }
    
    return tokens
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
 * Run Structured Text (ST) Compiler unit tests
 * @param {Object} [options] - Options
 * @param {boolean} [options.silent] - If true, suppress console output
 * @param {boolean} [options.verbose] - If true, show verbose output
 * @param {VovkPLC} [options.runtime] - Shared runtime instance (creates one if not provided)
 * @returns {Promise<SuiteResult>}
 */
export async function runTests(options = {}) {
    const { silent = false, verbose: optVerbose = false, runtime: sharedRuntime = null } = options
    const log = silent ? () => {} : console.log.bind(console)
    
    /** @type {TestResult[]} */
    const testResults = []
    /** @type {Object[]} */
    const failures = []
    let passed = 0
    let failed = 0
    let updated = 0
    
    // Parse CLI args only when not in silent mode
    const args = !silent ? process.argv.slice(2) : []
    const updateMode = !silent && args.includes('--update')
    const verboseMode = optVerbose || (!silent && args.includes('--verbose'))
    const testFilter = !silent ? args.find(a => !a.startsWith('--')) || null : null
    
    const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')

    if (!fs.existsSync(wasmPath)) {
        const error = `WASM file not found at ${wasmPath}`
        if (!silent) {
            console.error(`${RED}${error}${RESET}`)
            console.error('Run "npm run build" first.')
        }
        return {
            name: 'Structured Text (ST) Compiler',
            passed: 0, failed: 1, total: 1,
            tests: [{ name: 'WASM Load', passed: false, error }],
            failures: [{ name: 'WASM Load', error }]
        }
    }

    // Use shared runtime if provided, otherwise create one (for standalone execution)
    let runtime = sharedRuntime
    if (!runtime) {
        runtime = new VovkPLC(wasmPath)
        await runtime.initialize(wasmPath, false, true) // silent mode
    }

    const samplesDir = path.join(__dirname, 'st-samples')
    
    if (!fs.existsSync(samplesDir)) {
        const error = `Samples directory not found: ${samplesDir}`
        if (!silent) console.error(`${RED}${error}${RESET}`)
        return {
            name: 'Structured Text (ST) Compiler',
            passed: 0, failed: 1, total: 1,
            tests: [{ name: 'Find Samples', passed: false, error }],
            failures: [{ name: 'Find Samples', error }]
        }
    }

    const files = fs.readdirSync(samplesDir)
    let testCases = files
        .filter(f => f.match(/^test_\d+.*\.st$/))
        .map(f => f.replace('.st', ''))
        .sort()

    // Filter tests if specified
    if (testFilter) {
        testCases = testCases.filter(tc => 
            tc.includes(testFilter) || 
            tc.toLowerCase().includes(testFilter.toLowerCase())
        )
        if (testCases.length === 0) {
            const error = `No test cases matching '${testFilter}'`
            if (!silent) console.error(`${RED}${error}${RESET}`)
            return {
                name: 'Structured Text (ST) Compiler',
                passed: 0, failed: 1, total: 1,
                tests: [{ name: 'Find Tests', passed: false, error }],
                failures: [{ name: 'Find Tests', error }]
            }
        }
    }

    if (testCases.length === 0) {
        const error = `No test cases found in ${samplesDir}`
        if (!silent) console.error(`${RED}${error}${RESET}`)
        return {
            name: 'Structured Text (ST) Compiler',
            passed: 0, failed: 1, total: 1,
            tests: [{ name: 'Find Tests', passed: false, error }],
            failures: [{ name: 'Find Tests', error }]
        }
    }

    log(`${BOLD}${CYAN}╔══════════════════════════════════════════════════════════════════╗${RESET}`)
    log(`${BOLD}${CYAN}║         Structured Text (ST) → PLCScript Compiler Tests          ║${RESET}`)
    log(`${BOLD}${CYAN}╚══════════════════════════════════════════════════════════════════╝${RESET}`)
    log()
    
    if (updateMode) {
        log(`${YELLOW}Running in UPDATE mode - regenerating expected outputs${RESET}`)
        log()
    }
    
    if (testFilter) {
        log(`${CYAN}Filter: ${testFilter}${RESET}`)
    }
    log(`Found ${testCases.length} test case(s)`)
    log()

    for (const testCase of testCases) {
        const stPath = path.join(samplesDir, `${testCase}.st`)
        const expectedPath = path.join(samplesDir, `${testCase}.plcscript`)

        const testName = formatTestName(testCase)
        const stSource = fs.readFileSync(stPath, 'utf8')
        const hasExpectedFile = fs.existsSync(expectedPath)

        const testErrors = []
        let generatedPLCScript = ''
        
        if (verboseMode) {
            log(`\nProcessing: ${testCase}`)
            log(`ST source length: ${stSource.length}`)
        }

        try {
            // STEP 1: Compile ST → PLCScript
            const stResult = compileST(runtime, stSource.trim(), verboseMode, log)

            if (stResult.error) {
                testErrors.push(`ST compile error: ${stResult.error}`)
                throw new Error('ST compilation failed')
            }

            generatedPLCScript = stResult.output

            // STEP 2: Validate expected output (if .plcscript file exists and not in update mode)
            if (hasExpectedFile && !updateMode) {
                const expectedData = parseExpectedOutput(fs.readFileSync(expectedPath, 'utf8'))
                
                // Check tokens
                const missingTokens = []
                for (const token of expectedData.tokens) {
                    if (!generatedPLCScript.includes(token)) {
                        missingTokens.push(token)
                    }
                }
                if (missingTokens.length > 0) {
                    testErrors.push(`Missing PLCScript tokens: ${missingTokens.join(', ')}`)
                }
            }

            // UPDATE MODE: Regenerate expected output file
            if (updateMode) {
                // Preserve existing tokens or auto-extract from output
                let tokens = []
                if (hasExpectedFile) {
                    const existingData = parseExpectedOutput(fs.readFileSync(expectedPath, 'utf8'))
                    tokens = existingData.tokens
                } else {
                    // Auto-extract tokens from output
                    tokens = extractTokens(generatedPLCScript)
                }
                
                const newContent = generateExpectedOutput(testCase, tokens)
                fs.writeFileSync(expectedPath, newContent)
                
                log(`${YELLOW}~${RESET} ${testName} - Updated`)
                updated++
                testResults.push({ name: testName, passed: true, info: 'updated' })
                continue
            }

            // Report result
            if (testErrors.length === 0) {
                const flags = []
                if (!hasExpectedFile) flags.push('no expected file')
                const lineCount = generatedPLCScript.split('\n').filter(l => l.trim()).length
                const info = `${lineCount} lines` + (flags.length > 0 ? `, ${flags.join(', ')}` : '')
                const suffix = ` ${DIM}(${info})${RESET}`
                log(`${GREEN}✓${RESET} ${testName}${suffix}`)
                passed++
                testResults.push({ name: testName, passed: true, info })
                
                // Verbose output for passing tests
                if (verboseMode) {
                    log()
                    log(`${DIM}${'─'.repeat(60)}${RESET}`)
                    log(`${CYAN}PLCScript Output:${RESET}`)
                    log(generatedPLCScript.split('\n').map(l => `  ${DIM}${l}${RESET}`).join('\n'))
                    log(`${DIM}${'─'.repeat(60)}${RESET}`)
                    log()
                }
            } else {
                log(`${RED}✗${RESET} ${testName}`)
                const failureInfo = { 
                    name: testName, 
                    errors: testErrors, 
                    plcscript: generatedPLCScript,
                    stSource 
                }
                failures.push(failureInfo)
                testResults.push({ name: testName, passed: false, error: testErrors.join('; ') })
                failed++
            }

        } catch (error) {
            if (testErrors.length === 0) {
                testErrors.push(error.message)
            }
            log(`${RED}✗${RESET} ${testName}`)
            const failureInfo = { 
                name: testName, 
                errors: testErrors, 
                plcscript: generatedPLCScript,
                stSource 
            }
            failures.push(failureInfo)
            testResults.push({ name: testName, passed: false, error: testErrors.join('; ') })
            failed++
        }
    }

    // Summary
    log()
    log('─'.repeat(68))

    // Show detailed failures
    if (failures.length > 0 && !updateMode) {
        log()
        log(`${BOLD}${RED}Failed Tests:${RESET}`)
        log()

        for (const failure of failures) {
            log(`${BOLD}${failure.name}:${RESET}`)
            for (const err of failure.errors) {
                log(`  ${RED}• ${err}${RESET}`)
            }
            
            // Show verbose output for failed tests
            if (verboseMode && failure.stSource) {
                log()
                log(`${CYAN}ST Source:${RESET}`)
                log(failure.stSource.split('\n').map(l => `  ${DIM}${l}${RESET}`).join('\n'))
                log()
                if (failure.plcscript) {
                    log(`${CYAN}Generated PLCScript:${RESET}`)
                    log(failure.plcscript.split('\n').map(l => `  ${DIM}${l}${RESET}`).join('\n'))
                }
            }
            log()
        }
    }

    log()
    if (updateMode) {
        log(`${YELLOW}Updated ${updated} expected output file(s)${RESET}`)
    } else {
        const total = passed + failed
        if (failed === 0) {
            log(`${GREEN}Results: ${passed}/${total} tests passed${RESET}`)
        } else {
            log(`${RED}Results: ${passed}/${total} tests passed, ${failed} failed${RESET}`)
        }
    }

    return {
        name: 'Structured Text (ST) Compiler',
        passed,
        failed,
        total: passed + failed,
        tests: testResults,
        failures
    }
}

// Run if executed directly
if (isMainModule) {
    runTests().then(result => {
        process.exit(result.failed > 0 ? 1 : 0)
    })
}
