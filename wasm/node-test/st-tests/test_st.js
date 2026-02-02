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

// Parse CLI args
const args = process.argv.slice(2)
const updateMode = args.includes('--update')
const verboseMode = args.includes('--verbose')
const testFilter = args.find(a => !a.startsWith('--')) || null

// ANSI colors
const RED = '\x1b[31m'
const GREEN = '\x1b[32m'
const YELLOW = '\x1b[33m'
const CYAN = '\x1b[36m'
const RESET = '\x1b[0m'
const BOLD = '\x1b[1m'
const DIM = '\x1b[2m'

let passed = 0
let failed = 0
let updated = 0
const failures = []

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
const compileST = (runtime, source, verbose = false) => {
    // Clear and stream source
    runtime.wasm_exports.streamClear()
    streamCode(runtime, source)
    
    if (verbose) {
        console.log(`Streamed ${source.length} bytes`)
        console.log(`Stream available: ${runtime.wasm_exports.streamAvailable()}`)
    }
    
    // Load from stream
    runtime.wasm_exports.st_compiler_load_from_stream()
    
    if (verbose) {
        console.log(`After load, stream available: ${runtime.wasm_exports.streamAvailable()}`)
        console.log(`Input length after load: ${runtime.wasm_exports.st_compiler_get_input_length()}`)
        // Debug: check input buffer contents
        const inputPreview = getString(runtime, runtime.wasm_exports.st_compiler_debug_get_input_preview())
        console.log(`Input preview: "${inputPreview.substring(0, 50)}..."`)
    }
    
    // Compile
    const errorCode = runtime.wasm_exports.st_compiler_compile()
    
    if (verbose) {
        console.log(`Compile returned: ${errorCode}`)
        console.log(`Has error: ${runtime.wasm_exports.st_compiler_has_error()}`)
        console.log(`Output length: ${runtime.wasm_exports.st_compiler_get_output_length()}`)
        if (runtime.wasm_exports.st_compiler_has_error()) {
            const errTokenText = getString(runtime, runtime.wasm_exports.st_compiler_get_error_token_text())
            const errTokenType = runtime.wasm_exports.st_compiler_get_error_token_type()
            console.log(`Error token: "${errTokenText}" (type ${errTokenType})`)
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
            console.log(`Output so far:\n${output}`)
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

const run = async () => {
    const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')

    if (!fs.existsSync(wasmPath)) {
        console.error(`${RED}WASM file not found!${RESET}`)
        console.error('Run "npm run build" first.')
        process.exit(1)
    }

    const runtime = new VovkPLC(wasmPath)
    await runtime.initialize(wasmPath, false, true) // silent mode

    const samplesDir = path.join(__dirname, 'st-samples')
    
    if (!fs.existsSync(samplesDir)) {
        console.error(`${RED}Samples directory not found: ${samplesDir}${RESET}`)
        process.exit(1)
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
            console.error(`${RED}No test cases matching '${testFilter}'${RESET}`)
            process.exit(1)
        }
    }

    if (testCases.length === 0) {
        console.error(`${RED}No test cases found in ${samplesDir}${RESET}`)
        process.exit(1)
    }

    console.log(`${BOLD}${CYAN}╔══════════════════════════════════════════════════════════════════╗${RESET}`)
    console.log(`${BOLD}${CYAN}║         Structured Text (ST) → PLCScript Compiler Tests          ║${RESET}`)
    console.log(`${BOLD}${CYAN}╚══════════════════════════════════════════════════════════════════╝${RESET}`)
    console.log()
    
    if (updateMode) {
        console.log(`${YELLOW}Running in UPDATE mode - regenerating expected outputs${RESET}`)
        console.log()
    }
    
    if (testFilter) {
        console.log(`${CYAN}Filter: ${testFilter}${RESET}`)
    }
    console.log(`Found ${testCases.length} test case(s)`)
    console.log()

    for (const testCase of testCases) {
        const stPath = path.join(samplesDir, `${testCase}.st`)
        const expectedPath = path.join(samplesDir, `${testCase}.plcscript`)

        const testName = formatTestName(testCase)
        const stSource = fs.readFileSync(stPath, 'utf8')
        const hasExpectedFile = fs.existsSync(expectedPath)

        const testErrors = []
        let generatedPLCScript = ''
        
        if (verboseMode) {
            console.log(`\nProcessing: ${testCase}`)
            console.log(`ST source length: ${stSource.length}`)
        }

        try {
            // STEP 1: Compile ST → PLCScript
            const stResult = compileST(runtime, stSource.trim(), verboseMode)

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
                
                console.log(`${YELLOW}~${RESET} ${testName} - Updated`)
                updated++
                continue
            }

            // Report result
            if (testErrors.length === 0) {
                const flags = []
                if (!hasExpectedFile) flags.push('no expected file')
                const suffix = flags.length > 0 ? ` ${DIM}(${flags.join(', ')})${RESET}` : ''
                console.log(`${GREEN}✓${RESET} ${testName}${suffix}`)
                passed++
                
                // Verbose output for passing tests
                if (verboseMode) {
                    console.log()
                    console.log(`${DIM}${'─'.repeat(60)}${RESET}`)
                    console.log(`${CYAN}PLCScript Output:${RESET}`)
                    console.log(generatedPLCScript.split('\n').map(l => `  ${DIM}${l}${RESET}`).join('\n'))
                    console.log(`${DIM}${'─'.repeat(60)}${RESET}`)
                    console.log()
                }
            } else {
                console.log(`${RED}✗${RESET} ${testName}`)
                failures.push({ 
                    name: testName, 
                    errors: testErrors, 
                    plcscript: generatedPLCScript,
                    stSource 
                })
                failed++
            }

        } catch (error) {
            if (testErrors.length === 0) {
                testErrors.push(error.message)
            }
            console.log(`${RED}✗${RESET} ${testName}`)
            failures.push({ 
                name: testName, 
                errors: testErrors, 
                plcscript: generatedPLCScript,
                stSource 
            })
            failed++
        }
    }

    // Summary
    console.log()
    console.log('─'.repeat(68))

    // Show detailed failures
    if (failures.length > 0 && !updateMode) {
        console.log()
        console.log(`${BOLD}${RED}Failed Tests:${RESET}`)
        console.log()

        for (const failure of failures) {
            console.log(`${BOLD}${failure.name}:${RESET}`)
            for (const err of failure.errors) {
                console.log(`  ${RED}• ${err}${RESET}`)
            }
            
            // Show verbose output for failed tests
            if (verboseMode && failure.stSource) {
                console.log()
                console.log(`${CYAN}ST Source:${RESET}`)
                console.log(failure.stSource.split('\n').map(l => `  ${DIM}${l}${RESET}`).join('\n'))
                console.log()
                if (failure.plcscript) {
                    console.log(`${CYAN}Generated PLCScript:${RESET}`)
                    console.log(failure.plcscript.split('\n').map(l => `  ${DIM}${l}${RESET}`).join('\n'))
                }
            }
            console.log()
        }
    }

    console.log()
    if (updateMode) {
        console.log(`${YELLOW}Updated ${updated} expected output file(s)${RESET}`)
    } else {
        const total = passed + failed
        if (failed === 0) {
            console.log(`${GREEN}Results: ${passed}/${total} tests passed${RESET}`)
        } else {
            console.log(`${RED}Results: ${passed}/${total} tests passed, ${failed} failed${RESET}`)
        }
    }

    process.exit(failed > 0 && !updateMode ? 1 : 0)
}

run()
