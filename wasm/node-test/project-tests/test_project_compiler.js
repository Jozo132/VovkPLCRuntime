#!/usr/bin/env node
// test_project_compiler.js - Automated Project Compiler unit tests
//
// This script tests the Project Compiler by comparing
// the generated output against expected .output JSON files.
//
// Usage:
//   node wasm/node-test/project-tests/test_project_compiler.js
//   npm run test:project
//
// Test files:
//   - test_XX.project - Project definition input
//   - test_XX.output  - Expected output JSON with compilation results

import VovkPLC from '../../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

// ANSI colors
const RED = '\x1b[31m'
const GREEN = '\x1b[32m'
const YELLOW = '\x1b[33m'
const CYAN = '\x1b[36m'
const RESET = '\x1b[0m'
const BOLD = '\x1b[1m'

// Custom JSON formatter that collapses simple objects into one-liners
function formatJSON(obj, indent = 0) {
    const spaces = '  '.repeat(indent)
    const nextSpaces = '  '.repeat(indent + 1)
    
    if (obj === null) return 'null'
    if (typeof obj === 'boolean') return obj ? 'true' : 'false'
    if (typeof obj === 'number') return String(obj)
    if (typeof obj === 'string') return JSON.stringify(obj)
    
    if (Array.isArray(obj)) {
        if (obj.length === 0) return '[]'
        
        // Check if all items are simple objects (no nested arrays/objects with nested content)
        const allSimple = obj.every(item => {
            if (typeof item !== 'object' || item === null) return true
            if (Array.isArray(item)) return false
            // Check if object has no nested objects/arrays
            return Object.values(item).every(v => typeof v !== 'object' || v === null)
        })
        
        if (allSimple && obj.length > 0 && typeof obj[0] === 'object' && obj[0] !== null) {
            // Format as array of one-liner objects
            const items = obj.map(item => {
                if (typeof item !== 'object' || item === null) {
                    return nextSpaces + formatJSON(item)
                }
                const pairs = Object.entries(item).map(([k, v]) => `${JSON.stringify(k)}: ${formatJSON(v)}`)
                return nextSpaces + '{ ' + pairs.join(', ') + ' }'
            })
            return '[\n' + items.join(',\n') + '\n' + spaces + ']'
        }
        
        // Normal array formatting
        const items = obj.map(item => nextSpaces + formatJSON(item, indent + 1))
        return '[\n' + items.join(',\n') + '\n' + spaces + ']'
    }
    
    if (typeof obj === 'object') {
        const keys = Object.keys(obj)
        if (keys.length === 0) return '{}'
        
        // Check if this object has no nested objects/arrays (except empty ones)
        const isSimple = Object.values(obj).every(v => {
            if (typeof v !== 'object' || v === null) return true
            if (Array.isArray(v)) return v.length === 0
            return Object.keys(v).length === 0
        })
        
        if (isSimple && indent > 0) {
            // One-liner format
            const pairs = Object.entries(obj).map(([k, v]) => `${JSON.stringify(k)}: ${formatJSON(v)}`)
            return '{ ' + pairs.join(', ') + ' }'
        }
        
        // Normal object formatting with newlines
        const pairs = Object.entries(obj).map(([k, v]) => {
            return nextSpaces + JSON.stringify(k) + ': ' + formatJSON(v, indent + 1)
        })
        return '{\n' + pairs.join(',\n') + '\n' + spaces + '}'
    }
    
    return String(obj)
}

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

// Get bytecode as hex string (space-separated)
function getBytecodeHex(runtime) {
    const wasm = runtime.wasm_exports
    const bytecodePtr = wasm.project_getBytecode()
    const bytecodeLen = wasm.project_getBytecodeLength()
    
    if (!bytecodePtr || bytecodeLen === 0) return ''
    
    const memory = new Uint8Array(runtime.wasm.exports.memory.buffer)
    let hex = ''
    for (let i = 0; i < bytecodeLen; i++) {
        hex += memory[bytecodePtr + i].toString(16).padStart(2, '0').toUpperCase()
        if (i < bytecodeLen - 1) hex += ' '
    }
    return hex
}

// Get symbols from project
function getSymbols(runtime) {
    const wasm = runtime.wasm_exports
    const count = wasm.project_getSymbolCount()
    const symbols = []
    
    for (let i = 0; i < count; i++) {
        const name = getString(runtime, wasm.project_getSymbolName(i))
        const type = getString(runtime, wasm.project_getSymbolType(i))
        const address = getString(runtime, wasm.project_getSymbolAddress(i))
        symbols.push({ name, type, address })
    }
    
    return symbols
}

// Get program files info
function getFiles(runtime) {
    const wasm = runtime.wasm_exports
    const count = wasm.project_getFileCount()
    const files = []
    
    for (let i = 0; i < count; i++) {
        const path = getString(runtime, wasm.project_getFilePath(i))
        const firstBlockIndex = wasm.project_getFileFirstBlockIndex(i)
        const blockCount = wasm.project_getFileBlockCount(i)
        const executionOrder = wasm.project_getFileExecutionOrder(i)
        files.push({
            path,
            firstBlockIndex,
            blockCount,
            executionOrder
        })
    }
    
    return files
}

// Get program blocks info
function getBlocks(runtime) {
    const wasm = runtime.wasm_exports
    const count = wasm.project_getBlockCount()
    const blocks = []
    const langNames = ['UNKNOWN', 'PLCASM', 'STL', 'LADDER', 'FBD', 'SFC', 'ST', 'IL']
    
    for (let i = 0; i < count; i++) {
        const filePath = getString(runtime, wasm.project_getBlockFilePath(i))
        const name = getString(runtime, wasm.project_getBlockName(i))
        const lang = wasm.project_getBlockLanguage(i)
        const offset = wasm.project_getBlockOffset(i)
        const size = wasm.project_getBlockSize(i)
        blocks.push({
            file: filePath,
            name,
            language: langNames[lang] || 'UNKNOWN',
            offset,
            size
        })
    }
    
    return blocks
}

// Get memory areas
function getMemoryAreas(runtime) {
    const wasm = runtime.wasm_exports
    const count = wasm.project_getMemoryAreaCount()
    const areas = []
    
    for (let i = 0; i < count; i++) {
        const name = getString(runtime, wasm.project_getMemoryAreaName(i))
        const start = wasm.project_getMemoryAreaStart(i)
        const end = wasm.project_getMemoryAreaEnd(i)
        areas.push({ name, start, end })
    }
    
    return areas
}

// Run the compiled program and get execution info
function runProgram(runtime) {
    const wasm = runtime.wasm_exports
    
    // Load the program
    const loaded = wasm.project_load()
    if (!loaded) {
        return { success: false, error: 'Failed to load program' }
    }
    
    // Clear stack before running
    if (wasm.clearStack) {
        wasm.clearStack()
    }
    
    // Run one cycle
    if (wasm.run) {
        wasm.run()
    }
    
    // Get instruction count from the runtime
    const steps = wasm.getLastInstructionCount ? wasm.getLastInstructionCount() : 0
    
    // Get stack size after execution
    const stackSize = wasm.getStackSize ? wasm.getStackSize() : 0
    
    return {
        success: true,
        steps,
        stackSize
    }
}

// Generate the full output object for a project compilation
function generateOutput(runtime, success, error = null) {
    const wasm = runtime.wasm_exports
    const output = {
        success,
        project: {
            name: getString(runtime, wasm.project_getName()),
            version: getString(runtime, wasm.project_getVersion())
        },
        bytecode: '',
        bytecodeLength: 0,
        checksum: 0,
        files: [],
        blocks: [],
        symbols: [],
        memoryAreas: [],
        execution: {
            steps: 0,
            stackSize: 0
        },
        problems: []
    }
    
    if (!success) {
        const errorMsg = error || getString(runtime, wasm.project_getError())
        const errorLine = wasm.project_getErrorLine()
        const errorCol = wasm.project_getErrorColumn()
        const errorFile = getString(runtime, wasm.project_getErrorFile())
        const errorBlock = getString(runtime, wasm.project_getErrorBlock())
        const errorBlockLang = wasm.project_getErrorBlockLanguage()
        const langNames = ['UNKNOWN', 'PLCASM', 'STL', 'LADDER', 'FBD', 'SFC', 'ST', 'IL']
        const problem = {
            severity: 'error',
            message: errorMsg,
            line: errorLine,
            column: errorCol
        }
        // Add file and block context if available
        if (errorFile) problem.file = errorFile
        if (errorBlock) {
            problem.block = errorBlock
            problem.language = langNames[errorBlockLang] || 'UNKNOWN'
        }
        output.problems.push(problem)
        return output
    }
    
    // Successful compilation - populate all fields
    output.bytecode = getBytecodeHex(runtime)
    output.bytecodeLength = wasm.project_getBytecodeLength()
    output.checksum = wasm.project_getChecksum()
    output.files = getFiles(runtime)
    output.blocks = getBlocks(runtime)
    output.symbols = getSymbols(runtime)
    output.memoryAreas = getMemoryAreas(runtime)
    
    // Run the program to get execution info
    const execResult = runProgram(runtime)
    if (execResult.success) {
        output.execution.steps = execResult.steps
        output.execution.stackSize = execResult.stackSize
        
        // Add warning if stack leak detected
        if (execResult.stackSize > 0) {
            output.problems.push({
                severity: 'warning',
                message: `Stack leak: ${execResult.stackSize} byte(s) remaining after execution`,
                line: 0,
                column: 0
            })
        }
    } else {
        output.problems.push({
            severity: 'warning',
            message: execResult.error,
            line: 0,
            column: 0
        })
    }
    
    return output
}

// Compare two output objects and return differences
function compareOutputs(expected, actual) {
    const diffs = []
    
    // Compare success
    if (expected.success !== actual.success) {
        diffs.push({ field: 'success', expected: expected.success, actual: actual.success })
    }
    
    // Compare bytecode
    if (expected.bytecode !== actual.bytecode) {
        diffs.push({ field: 'bytecode', expected: expected.bytecode, actual: actual.bytecode })
    }
    
    // Compare bytecode length
    if (expected.bytecodeLength !== actual.bytecodeLength) {
        diffs.push({ field: 'bytecodeLength', expected: expected.bytecodeLength, actual: actual.bytecodeLength })
    }
    
    // Compare checksum
    if (expected.checksum !== actual.checksum) {
        diffs.push({ field: 'checksum', expected: expected.checksum, actual: actual.checksum })
    }
    
    // Compare execution
    if (expected.execution.steps !== actual.execution.steps) {
        diffs.push({ field: 'execution.steps', expected: expected.execution.steps, actual: actual.execution.steps })
    }
    if (expected.execution.stackSize !== actual.execution.stackSize) {
        diffs.push({ field: 'execution.stackSize', expected: expected.execution.stackSize, actual: actual.execution.stackSize })
    }
    
    // Compare files count
    const expectedFiles = expected.files || []
    const actualFiles = actual.files || []
    if (expectedFiles.length !== actualFiles.length) {
        diffs.push({ field: 'files.length', expected: expectedFiles.length, actual: actualFiles.length })
    }
    
    // Compare blocks count
    if (expected.blocks.length !== actual.blocks.length) {
        diffs.push({ field: 'blocks.length', expected: expected.blocks.length, actual: actual.blocks.length })
    }
    
    // Compare symbols count
    if (expected.symbols.length !== actual.symbols.length) {
        diffs.push({ field: 'symbols.length', expected: expected.symbols.length, actual: actual.symbols.length })
    }
    
    // Compare error messages (for error tests)
    if (!expected.success && expected.problems.length > 0) {
        const expectedError = expected.problems[0]?.message || ''
        const actualError = actual.problems[0]?.message || ''
        // Check if error messages are similar (contains check)
        if (!actualError.includes(expectedError) && !expectedError.includes(actualError)) {
            diffs.push({ field: 'problems[0].message', expected: expectedError, actual: actualError })
        }
    }
    
    return diffs
}

async function runTests() {
    const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')
    
    // Check for --update flag to force regeneration of expected outputs
    const updateMode = process.argv.includes('--update') || process.argv.includes('-u')
    
    if (!fs.existsSync(wasmPath)) {
        console.error(`${RED}Error: WASM file not found at ${wasmPath}${RESET}`)
        console.error('Run "npm run build" first.')
        process.exit(1)
    }

    // Initialize runtime
    const runtime = new VovkPLC(wasmPath)
    
    // Suppress stdout during initialization
    runtime.stdout_callback = () => {}
    await runtime.initialize(wasmPath, false, false)
    runtime.readStream()
    
    // Find all test files
    const testDir = __dirname
    const files = fs.readdirSync(testDir)
    const testCases = files
        .filter(f => f.match(/^test_\d+\.project$/))
        .map(f => f.replace('.project', ''))
        .sort()
    
    if (testCases.length === 0) {
        console.error(`${RED}No test cases found in ${testDir}${RESET}`)
        process.exit(1)
    }
    
    console.log(`${BOLD}${CYAN}╔══════════════════════════════════════════════════════════════════╗${RESET}`)
    console.log(`${BOLD}${CYAN}║              Project Compiler Unit Tests                         ║${RESET}`)
    console.log(`${BOLD}${CYAN}╚══════════════════════════════════════════════════════════════════╝${RESET}`)
    console.log()
    if (updateMode) {
        console.log(`${YELLOW}Running in UPDATE mode - regenerating expected outputs${RESET}`)
        console.log()
    }
    console.log(`Found ${testCases.length} test case(s)`)
    console.log()
    
    let passed = 0
    let failed = 0
    const failures = []
    const wasm = runtime.wasm_exports
    
    for (const testCase of testCases) {
        const projectPath = path.join(testDir, `${testCase}.project`)
        const outputPath = path.join(testDir, `${testCase}.output`)
        
        const projectCode = fs.readFileSync(projectPath, 'utf8')
        
        try {
            // Reset and compile
            wasm.project_reset()
            streamCode(runtime, projectCode)
            
            const result = wasm.project_compile(0) // 0 = no debug output
            
            // Generate actual output
            const actualOutput = generateOutput(runtime, result)
            
            // Check if expected output file exists - if not (or in update mode), generate it
            if (!fs.existsSync(outputPath) || updateMode) {
                fs.writeFileSync(outputPath, formatJSON(actualOutput) + '\n')
                const status = actualOutput.success ? 
                    `${actualOutput.bytecodeLength} bytes, ${actualOutput.blocks.length} block(s)` :
                    `error: ${actualOutput.problems[0]?.message || 'unknown'}`
                const symbol = updateMode && fs.existsSync(outputPath) ? '~' : '+'
                console.log(`${CYAN}${symbol}${RESET} ${testCase} - ${updateMode ? 'Updated' : 'Generated'} expected .output file (${status})`)
                passed++
                continue
            }
            
            // Load expected output
            const expectedOutput = JSON.parse(fs.readFileSync(outputPath, 'utf8'))
            
            // Compare outputs
            const diffs = compareOutputs(expectedOutput, actualOutput)
            
            if (diffs.length > 0) {
                console.log(`${RED}✗${RESET} ${testCase} - Output mismatch`)
                failures.push({ name: testCase, diffs, expected: expectedOutput, actual: actualOutput })
                failed++
                continue
            }
            
            // Build status message
            let status = ''
            if (actualOutput.success) {
                status = `${actualOutput.bytecodeLength} bytes, ${actualOutput.execution.steps} steps`
                if (actualOutput.execution.stackSize > 0) {
                    status += `, ${YELLOW}stack: ${actualOutput.execution.stackSize}${RESET}`
                }
            } else {
                status = `expected error: line ${actualOutput.problems[0]?.line || '?'}`
            }
            
            console.log(`${GREEN}✓${RESET} ${testCase} (${status})`)
            passed++
            
        } catch (e) {
            console.log(`${RED}✗${RESET} ${testCase} - Error: ${e.message}`)
            failed++
            failures.push({ name: testCase, error: e.message })
        }
    }
    
    console.log()
    console.log('─'.repeat(68))
    
    // Show detailed failures
    if (failures.length > 0) {
        console.log()
        console.log(`${BOLD}${RED}Failed Tests:${RESET}`)
        console.log()
        
        for (const failure of failures) {
            console.log(`${BOLD}${failure.name}:${RESET}`)
            if (failure.error) {
                console.log(`  Error: ${failure.error}`)
            } else if (failure.diffs && failure.diffs.length > 0) {
                console.log(`  Differences:`)
                for (const d of failure.diffs) {
                    console.log(`    ${d.field}:`)
                    console.log(`      expected: ${RED}${JSON.stringify(d.expected)}${RESET}`)
                    console.log(`      actual:   ${YELLOW}${JSON.stringify(d.actual)}${RESET}`)
                }
            }
            console.log()
        }
    }
    
    // Summary
    const total = passed + failed
    const statusColor = failed > 0 ? RED : GREEN
    console.log()
    console.log(`${BOLD}Results: ${statusColor}${passed}/${total} tests passed${RESET}`)
    
    if (failed > 0) {
        process.exit(1)
    }
}

runTests().catch(err => {
    console.error(`${RED}Error: ${err.message}${RESET}`)
    process.exit(1)
})
