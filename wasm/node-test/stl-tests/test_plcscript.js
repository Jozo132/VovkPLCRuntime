#!/usr/bin/env node
// test_plcscript.js - Comprehensive PLCScript unit tests
//
// This script performs FULL end-to-end testing:
//   1. PLCScript → PLCASM transpilation (checks expected tokens)
//   2. PLCASM → Bytecode compilation (checks expected bytecode)
//   3. Bytecode EXECUTION
//   4. Memory value VERIFICATION (optional .expect file)
//   5. Stack leak detection (stack must be empty after execution)
//
// Usage:
//   node wasm/node-test/stl-tests/test_plcscript.js [test_name] [--verbose] [--update]
//   npm run test_plcscript
//
// Examples:
//   node test_plcscript.js                    # Run all tests
//   node test_plcscript.js test_04            # Run only test_04_bitwise
//   node test_plcscript.js bitwise            # Run tests matching "bitwise"
//   node test_plcscript.js test_04 --verbose  # Show full PLCASM and bytecode output
//   node test_plcscript.js --update           # Regenerate all .plcasm expected files
//   node test_plcscript.js test_04 --update   # Regenerate expected output for test_04
//
// Test files (in plcscript-samples/):
//   - test_XX_name.plcscript - PLCScript source input
//   - test_XX_name.plcasm    - Expected PLCASM tokens + bytecode
//   - test_XX_name.expect    - Optional: Expected memory values after execution (JSON)
//
// .plcasm file format:
//   # Comments start with #
//   # Required tokens (one per line, must appear in PLCASM output):
//   bw.and.x8
//   bw.or.x8
//   # BYTECODE: 03 F0 19 03 ...
//
// .expect file format (JSON):
//   {
//     "memory": {
//       "0": { "type": "i32", "value": 5 },
//       "10": { "type": "f32", "value": 3.14 }
//     }
//   }

import VovkPLC from '../../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

// Check if running as main module or imported
const isMainModule = process.argv[1] && (
    process.argv[1].endsWith('test_plcscript.js') ||
    process.argv[1].includes('test_plcscript')
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

// Compile PLCScript to PLCASM
const compilePLCScript = (runtime, source) => {
    const inBufferPtr = runtime.wasm_exports.get_in_buffer_ptr()
    const inBufferSize = runtime.wasm_exports.get_in_buffer_size()
    const memory = new Uint8Array(runtime.wasm.exports.memory.buffer)

    runtime.wasm_exports.streamClear()
    const sourceLen = Math.min(source.length, inBufferSize - 1)
    for (let i = 0; i < sourceLen; i++) {
        memory[inBufferPtr + i] = source.charCodeAt(i)
    }
    memory[inBufferPtr + sourceLen] = 0
    runtime.wasm_exports.set_in_index(sourceLen)

    runtime.wasm_exports.plcscript_load_from_stream()
    const success = runtime.wasm_exports.plcscript_compile()

    if (!success || runtime.wasm_exports.plcscript_hasError()) {
        const errorMsg = getString(runtime, runtime.wasm_exports.plcscript_getError())
        const errorLine = runtime.wasm_exports.plcscript_getErrorLine()
        const errorCol = runtime.wasm_exports.plcscript_getErrorColumn()
        return { error: `${errorMsg} at line ${errorLine}, col ${errorCol}` }
    }

    runtime.wasm_exports.flush_out_buffer()
    runtime.wasm_exports.plcscript_output_to_stream()

    const outBufferPtr = runtime.wasm_exports.get_out_buffer_ptr()
    const outIndex = runtime.wasm_exports.get_out_index()
    const outMem = new Uint8Array(runtime.wasm.exports.memory.buffer)

    let plcasm = ''
    for (let i = 0; i < outIndex; i++) {
        plcasm += String.fromCharCode(outMem[outBufferPtr + i])
    }

    return { output: plcasm }
}

// Compile PLCASM to bytecode and return the bytecode
const compilePLCASM = (runtime, plcasm) => {
    // Use the downloadAssembly helper which properly streams + loads assembly
    runtime.downloadAssembly(plcasm)
    
    // Compile
    const compileError = runtime.wasm_exports.compileAssembly(false)
    
    // Clear any output
    runtime.readStream()
    
    if (compileError) {
        return { error: 'PLCASM compilation failed' }
    }

    // Extract bytecode using uploadProgram (streams to out buffer as hex)
    const extracted = runtime.extractProgram()
    const bytecode = extracted.output || ''
    const bytecodeSize = extracted.size || 0

    // Load compiled program for execution
    const loadError = runtime.wasm_exports.loadCompiledProgram()
    runtime.readStream()
    
    if (loadError) {
        return { error: 'Failed to load compiled program' }
    }

    return { success: true, bytecode, bytecodeSize }
}

// Execute the loaded program and return execution info
const executeProgram = (runtime) => {
    const wasm = runtime.wasm_exports
    
    // Reset memory and stack
    if (wasm.memoryReset) wasm.memoryReset()
    if (wasm.clearStack) wasm.clearStack()
    
    // Get initial free memory for leak detection
    const initialFreeMemory = wasm.get_free_memory ? wasm.get_free_memory() : 0
    
    // Run the program
    if (wasm.run) {
        wasm.run()
    }
    
    // Get execution stats
    const stackSize = wasm.getStackSize ? wasm.getStackSize() : 0
    const instructionCount = wasm.getLastInstructionCount ? wasm.getLastInstructionCount() : 0
    const finalFreeMemory = wasm.get_free_memory ? wasm.get_free_memory() : 0
    
    // Check for memory leak
    const memoryLeak = initialFreeMemory !== finalFreeMemory
    
    return {
        stackSize,
        instructionCount,
        memoryLeak,
        initialFreeMemory,
        finalFreeMemory
    }
}

// Read memory value at address with specified type
const readMemoryValue = (runtime, address, type) => {
    const wasm = runtime.wasm_exports
    const memoryOffset = wasm.getMemoryLocation()
    const buffer = runtime.wasm.exports.memory.buffer
    const view = new DataView(buffer)
    const offset = memoryOffset + address
    
    switch (type) {
        case 'u8': return view.getUint8(offset)
        case 'i8': return view.getInt8(offset)
        case 'u16': return view.getUint16(offset, true)
        case 'i16': return view.getInt16(offset, true)
        case 'u32': return view.getUint32(offset, true)
        case 'i32': return view.getInt32(offset, true)
        case 'f32': return view.getFloat32(offset, true)
        case 'f64': return view.getFloat64(offset, true)
        default: return view.getUint8(offset)
    }
}

// Parse expected output from .plcasm file
const parseExpectedOutput = (content) => {
    const lines = content.split('\n')
    const tokens = []
    let bytecode = null
    
    for (const line of lines) {
        const trimmed = line.trim()
        if (!trimmed || trimmed.startsWith('//')) continue
        if (trimmed.startsWith('#')) {
            // Check for bytecode comment
            if (trimmed.startsWith('# BYTECODE:')) {
                bytecode = trimmed.replace('# BYTECODE:', '').trim()
            }
            continue
        }
        tokens.push(trimmed)
    }
    
    return { tokens, bytecode }
}

// Generate expected output file content
const generateExpectedOutput = (testName, tokens, bytecode) => {
    let content = `# Expected output for ${testName}\n`
    content += `# Auto-generated - edit tokens as needed, bytecode is regenerated on --update\n`
    content += `#\n`
    content += `# Required PLCASM tokens (must appear in transpiled output):\n`
    for (const token of tokens) {
        content += `${token}\n`
    }
    content += `#\n`
    content += `# BYTECODE: ${bytecode}\n`
    return content
}

// Format test name from filename
const formatTestName = (filename) => {
    return filename
        .replace(/^test_\d+_/, '')
        .replace(/_/g, ' ')
        .replace(/\b\w/g, c => c.toUpperCase())
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
 * Run PLCScript Compiler unit tests
 * @param {Object} [options] - Options
 * @param {boolean} [options.silent] - If true, suppress console output
 * @param {boolean} [options.verbose] - If true, show verbose output
 * @returns {Promise<SuiteResult>}
 */
export async function runTests(options = {}) {
    const { silent = false, verbose: optVerbose = false } = options
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
        if (!silent) console.error(`${RED}${error}${RESET}`)
        return {
            name: 'PLCScript Compiler & Runtime',
            passed: 0, failed: 1, total: 1,
            tests: [{ name: 'WASM Load', passed: false, error }],
            failures: [{ name: 'WASM Load', error }]
        }
    }

    const runtime = new VovkPLC(wasmPath)
    await runtime.initialize(wasmPath, false, true) // silent mode

    const samplesDir = path.join(__dirname, 'plcscript-samples')
    
    if (!fs.existsSync(samplesDir)) {
        const error = `Samples directory not found: ${samplesDir}`
        if (!silent) console.error(`${RED}${error}${RESET}`)
        return {
            name: 'PLCScript Compiler & Runtime',
            passed: 0, failed: 1, total: 1,
            tests: [{ name: 'Find Samples', passed: false, error }],
            failures: [{ name: 'Find Samples', error }]
        }
    }

    const files = fs.readdirSync(samplesDir)
    let testCases = files
        .filter(f => f.match(/^test_\d+.*\.plcscript$/))
        .map(f => f.replace('.plcscript', ''))
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
                name: 'PLCScript Compiler & Runtime',
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
            name: 'PLCScript Compiler & Runtime',
            passed: 0, failed: 1, total: 1,
            tests: [{ name: 'Find Tests', passed: false, error }],
            failures: [{ name: 'Find Tests', error }]
        }
    }

    log(`${BOLD}${CYAN}╔══════════════════════════════════════════════════════════════════╗${RESET}`)
    log(`${BOLD}${CYAN}║         PLCScript End-to-End Compiler & Runtime Tests            ║${RESET}`)
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
        const plcscriptPath = path.join(samplesDir, `${testCase}.plcscript`)
        const expectedPath = path.join(samplesDir, `${testCase}.plcasm`)
        const expectPath = path.join(samplesDir, `${testCase}.expect`)

        const testName = formatTestName(testCase)
        const plcscript = fs.readFileSync(plcscriptPath, 'utf8')
        const hasExpectedFile = fs.existsSync(expectedPath)
        const hasExpectFile = fs.existsSync(expectPath)

        const testErrors = []
        let generatedPlcasm = ''
        let generatedBytecode = ''
        let bytecodeSize = 0

        try {
            // STEP 1: Compile PLCScript → PLCASM
            const plcscriptResult = compilePLCScript(runtime, plcscript.trim())

            if (plcscriptResult.error) {
                testErrors.push(`PLCScript compile error: ${plcscriptResult.error}`)
                throw new Error('PLCScript compilation failed')
            }

            generatedPlcasm = plcscriptResult.output

            // STEP 2: Compile PLCASM → Bytecode
            const plcasmResult = compilePLCASM(runtime, generatedPlcasm)

            if (plcasmResult.error) {
                testErrors.push(`PLCASM compile error: ${plcasmResult.error}`)
                throw new Error('PLCASM compilation failed')
            }

            generatedBytecode = plcasmResult.bytecode
            bytecodeSize = plcasmResult.bytecodeSize

            // STEP 3: Validate expected output (if .plcasm file exists and not in update mode)
            if (hasExpectedFile && !updateMode) {
                const expectedData = parseExpectedOutput(fs.readFileSync(expectedPath, 'utf8'))
                
                // Check tokens
                const missingTokens = []
                for (const token of expectedData.tokens) {
                    if (!generatedPlcasm.includes(token)) {
                        missingTokens.push(token)
                    }
                }
                if (missingTokens.length > 0) {
                    testErrors.push(`Missing PLCASM tokens: ${missingTokens.join(', ')}`)
                }
                
                // Check bytecode
                if (expectedData.bytecode && expectedData.bytecode !== generatedBytecode) {
                    testErrors.push(`Bytecode mismatch:`)
                    testErrors.push(`  expected: ${expectedData.bytecode}`)
                    testErrors.push(`  actual:   ${generatedBytecode}`)
                }
            }

            // STEP 4: Execute bytecode
            const execResult = executeProgram(runtime)

            // STEP 5: Check for stack leak (stack should be empty after execution)
            if (execResult.stackSize !== 0) {
                testErrors.push(`Stack leak: ${execResult.stackSize} bytes left on stack`)
            }

            // STEP 6: Check for memory leak (allocator)
            if (execResult.memoryLeak) {
                testErrors.push(`Memory leak: free memory changed from ${execResult.initialFreeMemory} to ${execResult.finalFreeMemory}`)
            }

            // STEP 7: Check execution actually happened
            if (execResult.instructionCount === 0) {
                testErrors.push('No instructions executed (possible skipped code)')
            }

            // STEP 8: Verify expected memory values (if .expect file exists)
            if (hasExpectFile) {
                const expectData = JSON.parse(fs.readFileSync(expectPath, 'utf8'))
                
                if (expectData.memory) {
                    for (const [addrStr, spec] of Object.entries(expectData.memory)) {
                        const address = parseInt(addrStr, 10)
                        const actualValue = readMemoryValue(runtime, address, spec.type)
                        const expectedValue = spec.value
                        
                        // For floats, allow small epsilon difference
                        if (spec.type === 'f32' || spec.type === 'f64') {
                            const epsilon = 0.0001
                            if (Math.abs(actualValue - expectedValue) > epsilon) {
                                testErrors.push(`Memory[${address}] (${spec.type}): expected ${expectedValue}, got ${actualValue}`)
                            }
                        } else {
                            if (actualValue !== expectedValue) {
                                testErrors.push(`Memory[${address}] (${spec.type}): expected ${expectedValue}, got ${actualValue}`)
                            }
                        }
                    }
                }
            }

            // UPDATE MODE: Regenerate expected output file
            if (updateMode) {
                // Preserve existing tokens or use empty
                let tokens = []
                if (hasExpectedFile) {
                    const existingData = parseExpectedOutput(fs.readFileSync(expectedPath, 'utf8'))
                    tokens = existingData.tokens
                }
                
                const newContent = generateExpectedOutput(testCase, tokens, generatedBytecode)
                fs.writeFileSync(expectedPath, newContent)
                
                log(`${YELLOW}~${RESET} ${testName} - Updated (${bytecodeSize} bytes)`)
                testResults.push({ name: testName, passed: true, info: `updated, ${bytecodeSize} bytes` })
                updated++
                continue
            }

            // Report result
            if (testErrors.length === 0) {
                const flags = []
                if (!hasExpectedFile) flags.push('no expected file')
                if (!hasExpectFile) flags.push('no memory check')
                const infoBase = `${bytecodeSize} bytes`
                const info = flags.length > 0 ? `${infoBase}, ${flags.join(', ')}` : infoBase
                const suffix = ` ${DIM}(${info})${RESET}`
                log(`${GREEN}✓${RESET} ${testName}${suffix}`)
                passed++
                testResults.push({ name: testName, passed: true, info })
                
                // Verbose output for passing tests
                if (verboseMode) {
                    log()
                    log(`${DIM}${'─'.repeat(60)}${RESET}`)
                    log(`${CYAN}PLCASM Output:${RESET}`)
                    log(generatedPlcasm.split('\n').map(l => `  ${DIM}${l}${RESET}`).join('\n'))
                    log()
                    log(`${CYAN}Bytecode (${bytecodeSize} bytes):${RESET} ${generatedBytecode}`)
                    log(`${DIM}${'─'.repeat(60)}${RESET}`)
                    log()
                }
            } else {
                log(`${RED}✗${RESET} ${testName}`)
                testResults.push({ name: testName, passed: false, error: testErrors[0] })
                failures.push({ 
                    name: testName, 
                    errors: testErrors, 
                    plcasm: generatedPlcasm, 
                    bytecode: generatedBytecode,
                    bytecodeSize,
                    plcscript 
                })
                failed++
            }

        } catch (error) {
            if (testErrors.length === 0) {
                testErrors.push(error.message)
            }
            log(`${RED}✗${RESET} ${testName}`)
            testResults.push({ name: testName, passed: false, error: testErrors[0] })
            failures.push({ 
                name: testName, 
                errors: testErrors, 
                plcasm: generatedPlcasm,
                bytecode: generatedBytecode,
                bytecodeSize,
                plcscript 
            })
            failed++
        }
    }

    // Summary
    log()
    log('─'.repeat(68))

    // Show detailed failures
    if (failures.length > 0 && !updateMode && !silent) {
        log()
        log(`${BOLD}${RED}Failed Tests:${RESET}`)
        log()

        for (const failure of failures) {
            log(`${BOLD}${failure.name}:${RESET}`)
            for (const err of failure.errors) {
                log(`  ${RED}• ${err}${RESET}`)
            }
            
            // Show verbose output for failed tests
            if (verboseMode && failure.plcscript) {
                log()
                log(`${CYAN}PLCScript Source:${RESET}`)
                log(failure.plcscript.split('\n').map(l => `  ${DIM}${l}${RESET}`).join('\n'))
                log()
                if (failure.plcasm) {
                    log(`${CYAN}Generated PLCASM:${RESET}`)
                    log(failure.plcasm.split('\n').map(l => `  ${DIM}${l}${RESET}`).join('\n'))
                }
                if (failure.bytecode) {
                    log()
                    log(`${CYAN}Generated Bytecode (${failure.bytecodeSize} bytes):${RESET} ${failure.bytecode}`)
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
            log(`${DIM}All tests: compiled, executed, stack verified${RESET}`)
        } else {
            log(`${RED}Results: ${passed}/${total} tests passed, ${failed} failed${RESET}`)
        }
    }

    return {
        name: 'PLCScript Compiler & Runtime',
        passed,
        failed,
        total: passed + failed,
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
