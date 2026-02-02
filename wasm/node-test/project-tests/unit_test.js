#!/usr/bin/env node
// unit_test.js - Automated Project Compiler unit tests
//
// This script tests the Project Compiler by comparing
// the generated output against expected .output JSON files.
//
// Usage:
//   node wasm/node-test/project-tests/unit_test.js [test_name] [--update] [--verbose] [--run]
//   npm run test:project
//
// Examples:
//   node unit_test.js           # Run all tests
//   node unit_test.js test_03   # Run only test_03
//   node unit_test.js test_03 --update  # Update expected output for test_03
//   node unit_test.js test_03 --verbose # Run with debug output from compiler
//   node unit_test.js test_01 --run     # Explain bytecode step-by-step
//
// Test files (in samples/ subdirectory):
//   - test_XX.project - Project definition input
//   - test_XX.output  - Expected output JSON with compilation results
// @ts-check
'use strict'

import VovkPLC from '../../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import {fileURLToPath} from 'url'
import {execSync} from 'child_process'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

// Check if running as main module or imported
const isMainModule = process.argv[1] && (
    process.argv[1].endsWith('unit_test.js') &&
    process.argv[1].includes('project-tests')
)

// ANSI colors
const RED = '\x1b[31m'
const GREEN = '\x1b[32m'
const YELLOW = '\x1b[33m'
const CYAN = '\x1b[36m'
const RESET = '\x1b[0m'
const BOLD = '\x1b[1m'

// Custom JSON formatter that collapses simple objects into one-liners

/** @type { (obj: any, indent?: number) => string } */
const formatJSON = (obj, indent = 0) => {
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
/** @type { (runtime: VovkPLC, code: string) => void } */
const streamCode = (runtime, code) => {
    runtime.wasm_exports.streamClear()
    for (let i = 0; i < code.length; i++) {
        runtime.wasm_exports.streamIn(code.charCodeAt(i))
    }
}

// Read a null-terminated string from WASM memory
/** @type { (runtime: VovkPLC, ptr: number) => string } */
const getString = (runtime, ptr) => {
    if (!ptr) return '' // @ts-ignore
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
/** @type { (runtime: VovkPLC) => string } */
const getBytecodeHex = runtime => {
    const wasm = runtime.wasm_exports
    const bytecodePtr = wasm.project_getBytecode()
    const bytecodeLen = wasm.project_getBytecodeLength()

    if (!bytecodePtr || bytecodeLen === 0) return '' // @ts-ignore

    const memory = new Uint8Array(runtime.wasm.exports.memory.buffer)
    let hex = ''
    for (let i = 0; i < bytecodeLen; i++) {
        hex += memory[bytecodePtr + i].toString(16).padStart(2, '0').toUpperCase()
        if (i < bytecodeLen - 1) hex += ' '
    }
    return hex
}

// Get symbols from project
/** @type { (runtime: VovkPLC) => any[] } */
const getSymbols = runtime => {
    const wasm = runtime.wasm_exports
    const count = wasm.project_getSymbolCount()
    const symbols = []

    for (let i = 0; i < count; i++) {
        const name = getString(runtime, wasm.project_getSymbolName(i))
        const type = getString(runtime, wasm.project_getSymbolType(i))
        const address = getString(runtime, wasm.project_getSymbolAddress(i))
        symbols.push({name, type, address})
    }

    return symbols
}

// Get program files info
/** @type { (runtime: VovkPLC) => any[] } */
const getFiles = runtime => {
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
            executionOrder,
        })
    }

    return files
}

// Get program blocks info
/** @type { (runtime: VovkPLC) => any[] } */
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
            size,
        })
    }

    return blocks
}

// Get memory areas
/** @type { (runtime: VovkPLC) => any[] } */
function getMemoryAreas(runtime) {
    const wasm = runtime.wasm_exports
    const count = wasm.project_getMemoryAreaCount()
    const areas = []

    for (let i = 0; i < count; i++) {
        const name = getString(runtime, wasm.project_getMemoryAreaName(i))
        const start = wasm.project_getMemoryAreaStart(i)
        const end = wasm.project_getMemoryAreaEnd(i)
        areas.push({name, start, end})
    }

    return areas
}

// Get memory usage info
/** @type { (runtime: VovkPLC) => {available: number, used: number} } */
function getMemoryUsage(runtime) {
    const wasm = runtime.wasm_exports
    const available = wasm.project_getMemoryAvailable ? wasm.project_getMemoryAvailable() : 0
    const used = wasm.project_getMemoryUsed ? wasm.project_getMemoryUsed() : 0
    return {available, used}
}

// Get flash usage info
/** @type { (runtime: VovkPLC) => {size: number, used: number} } */
function getFlashUsage(runtime) {
    const wasm = runtime.wasm_exports
    const size = wasm.project_getFlashSize ? wasm.project_getFlashSize() : 32768
    const used = wasm.project_getFlashUsed ? wasm.project_getFlashUsed() : 0
    return {size, used}
}

// Run the compiled program and get execution info
/** @type { (runtime: VovkPLC) => {success: boolean, steps?: number, stackSize: number, error?: string} } */
function runProgram(runtime) {
    const wasm = runtime.wasm_exports

    // Load the program
    const loaded = wasm.project_load()
    if (!loaded) {
        return {success: false, error: 'Failed to load program', stackSize: 0}
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
    const stackSize = wasm.getStackSize ? wasm.getStackSize() || 0 : 0

    return {
        success: true,
        steps,
        stackSize,
    }
}

// Generate the full output object for a project compilation
/** @type { (runtime: VovkPLC, success: boolean, error?: string | null) => any } */
function generateOutput(runtime, success, error = null) {
    const wasm = runtime.wasm_exports

    /** @typedef { {severity: string, message: string, line: number, column: number, file?: string, block?: string, language?: string, token?: string, sourceLine?: string} } Problem */
    /**
     * @type {{
     *    success: boolean,
     *    project: { name: string, version: string },
     *    bytecode: string,
     *    bytecodeLength: number,
     *    checksum: number,
     *    files: any[],
     *    blocks: any[],
     *    symbols: any[],
     *    memoryAreas: any[],
     *    memory: { available: number, used: number },
     *    flash: { size: number, used: number },
     *    execution: { steps: number, stackSize: number },
     *    problems: Problem[]
     * }} 
    **/
    const output = {
        success,
        project: {
            name: getString(runtime, wasm.project_getName()),
            version: getString(runtime, wasm.project_getVersion()),
        },
        bytecode: '',
        bytecodeLength: 0,
        checksum: 0,
        files: [],
        blocks: [],
        symbols: [],
        memoryAreas: [],
        memory: { available: 0, used: 0 },
        flash: { size: 0, used: 0 },
        execution: { steps: 0, stackSize: 0, },
        problems: [],
    }

    // Get all accumulated problems from the compiler
    const problemCount = wasm.project_getProblemCount ? wasm.project_getProblemCount() : 0
    if (problemCount > 0 && wasm.project_getProblems) {
        const pointer = wasm.project_getProblems()
        // Struct size = 344 bytes (4+4+4+4+128+64+64+4+64+4)
        const struct_size = 344
        const memory = new Uint8Array(wasm.memory.buffer)
        const view = new DataView(wasm.memory.buffer)
        const langNames = ['UNKNOWN', 'PLCASM', 'STL', 'LADDER', 'FBD', 'SFC', 'ST', 'IL']

        for (let i = 0; i < problemCount; i++) {
            const offset = pointer + i * struct_size
            const type_int = view.getUint32(offset + 0, true)
            const line = view.getUint32(offset + 4, true)
            const column = view.getUint32(offset + 8, true)
            const length = view.getUint32(offset + 12, true)

            // message is 128 bytes at offset 16
            let message = ''
            for (let j = 0; j < 128; j++) {
                const charCode = view.getUint8(offset + 16 + j)
                if (charCode === 0) break
                message += String.fromCharCode(charCode)
            }

            // block is 64 bytes at offset 144
            let block = ''
            for (let j = 0; j < 64; j++) {
                const charCode = view.getUint8(offset + 144 + j)
                if (charCode === 0) break
                block += String.fromCharCode(charCode)
            }

            // program is 64 bytes at offset 208
            let program = ''
            for (let j = 0; j < 64; j++) {
                const charCode = view.getUint8(offset + 208 + j)
                if (charCode === 0) break
                program += String.fromCharCode(charCode)
            }

            // lang is 4 bytes at offset 272
            const lang = view.getUint32(offset + 272, true)

            // token_buf is 64 bytes at offset 276
            let token = ''
            for (let j = 0; j < 64; j++) {
                const charCode = view.getUint8(offset + 276 + j)
                if (charCode === 0) break
                token += String.fromCharCode(charCode)
            }

            /** @type { Problem } */
            const problem = {
                severity: type_int === 2 ? 'error' : type_int === 1 ? 'warning' : 'info',
                message: message,
                line: line,
                column: column,
            }
            if (program) problem.file = program
            if (block) {
                problem.block = block
                problem.language = langNames[lang] || 'UNKNOWN'
            }
            if (token) problem.token = token
            output.problems.push(problem)
        }
    }

    if (!success) {
        // If no accumulated problems, fall back to single error message
        if (output.problems.length === 0) {
            const errorMsg = error || getString(runtime, wasm.project_getError())
            const errorLine = wasm.project_getErrorLine()
            const errorCol = wasm.project_getErrorColumn()
            const errorFile = getString(runtime, wasm.project_getErrorFile())
            const errorBlock = getString(runtime, wasm.project_getErrorBlock())
            const errorBlockLang = wasm.project_getErrorBlockLanguage()
            const errorToken = getString(runtime, wasm.project_getErrorToken())
            const errorSourceLine = getString(runtime, wasm.project_getErrorSourceLine())
            const langNames = ['UNKNOWN', 'PLCASM', 'STL', 'LADDER', 'FBD', 'SFC', 'ST', 'IL']
            /** @type { Problem } */
            const problem = {
                severity: 'error',
                message: errorMsg,
                line: errorLine,
                column: errorCol,
            }
            // Add file and block context if available
            if (errorFile) problem.file = errorFile
            if (errorBlock) {
                problem.block = errorBlock
                problem.language = langNames[errorBlockLang] || 'UNKNOWN'
            }
            if (errorToken) problem.token = errorToken
            if (errorSourceLine) problem.sourceLine = errorSourceLine
            output.problems.push(problem)
        }
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
    output.memory = getMemoryUsage(runtime)
    output.flash = getFlashUsage(runtime)

    // Run the program to get execution info
    const execResult = runProgram(runtime)
    if (execResult.success) {
        output.execution.steps = execResult.steps || 0
        output.execution.stackSize = execResult.stackSize || 0

        // Add warning if stack leak detected
        if (execResult.stackSize > 0) {
            output.problems.push({
                severity: 'warning',
                message: `Stack leak: ${execResult.stackSize} byte(s) remaining after execution`,
                line: 0,
                column: 0,
            })
        }
    } else {
        output.problems.push({
            severity: 'warning',
            message: execResult.error || '?',
            line: 0,
            column: 0,
        })
    }

    return output
}

// Compare two output objects and return differences
/** @type { (expected: any, actual: any) => Array<{field: string, expected: any, actual: any}> } */
function compareOutputs(expected, actual) {
    const diffs = []

    // Compare success
    if (expected.success !== actual.success) {
        diffs.push({field: 'success', expected: expected.success, actual: actual.success})
    }

    // Compare bytecode
    if (expected.bytecode !== actual.bytecode) {
        diffs.push({field: 'bytecode', expected: expected.bytecode, actual: actual.bytecode})
    }

    // Compare bytecode length
    if (expected.bytecodeLength !== actual.bytecodeLength) {
        diffs.push({field: 'bytecodeLength', expected: expected.bytecodeLength, actual: actual.bytecodeLength})
    }

    // Compare checksum
    if (expected.checksum !== actual.checksum) {
        diffs.push({field: 'checksum', expected: expected.checksum, actual: actual.checksum})
    }

    // Compare execution
    if (expected.execution.steps !== actual.execution.steps) {
        diffs.push({field: 'execution.steps', expected: expected.execution.steps, actual: actual.execution.steps})
    }
    if (expected.execution.stackSize !== actual.execution.stackSize) {
        diffs.push({field: 'execution.stackSize', expected: expected.execution.stackSize, actual: actual.execution.stackSize})
    }

    // Compare files count
    const expectedFiles = expected.files || []
    const actualFiles = actual.files || []
    if (expectedFiles.length !== actualFiles.length) {
        diffs.push({field: 'files.length', expected: expectedFiles.length, actual: actualFiles.length})
    }

    // Compare blocks count
    if (expected.blocks.length !== actual.blocks.length) {
        diffs.push({field: 'blocks.length', expected: expected.blocks.length, actual: actual.blocks.length})
    }

    // Compare individual block properties
    const minBlocks = Math.min(expected.blocks.length, actual.blocks.length)
    for (let i = 0; i < minBlocks; i++) {
        const eb = expected.blocks[i]
        const ab = actual.blocks[i]
        if (eb.file !== ab.file) {
            diffs.push({field: `blocks[${i}].file`, expected: eb.file, actual: ab.file})
        }
        if (eb.name !== ab.name) {
            diffs.push({field: `blocks[${i}].name`, expected: eb.name, actual: ab.name})
        }
        if (eb.language !== ab.language) {
            diffs.push({field: `blocks[${i}].language`, expected: eb.language, actual: ab.language})
        }
        if (eb.offset !== ab.offset) {
            diffs.push({field: `blocks[${i}].offset`, expected: eb.offset, actual: ab.offset})
        }
        if (eb.size !== ab.size) {
            diffs.push({field: `blocks[${i}].size`, expected: eb.size, actual: ab.size})
        }
    }

    // Compare symbols count
    if (expected.symbols.length !== actual.symbols.length) {
        diffs.push({field: 'symbols.length', expected: expected.symbols.length, actual: actual.symbols.length})
    }

    // Compare error messages (for error tests)
    if (!expected.success && expected.problems.length > 0) {
        const expectedError = expected.problems[0]?.message || ''
        const actualError = actual.problems[0]?.message || ''
        // Check if error messages are similar (contains check)
        if (!actualError.includes(expectedError) && !expectedError.includes(actualError)) {
            diffs.push({field: 'problems[0].message', expected: expectedError, actual: actualError})
        }
    }

    return diffs
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
 * Run Project Compiler unit tests
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
    
    const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')

    // Check for --update flag to force regeneration of expected outputs
    const updateMode = !silent && (process.argv.includes('--update') || process.argv.includes('-u'))

    // Check for --verbose flag to show debug output from compiler
    const verboseMode = optVerbose || (!silent && (process.argv.includes('--verbose') || process.argv.includes('-v')))

    // Check for --run flag to pipe bytecode to npm run explain
    const runMode = !silent && (process.argv.includes('--run') || process.argv.includes('-r'))

    // Check for specific test name argument (not a flag)
    const testFilter = !silent ? process.argv.slice(2).find(arg => !arg.startsWith('-')) : null

    if (!fs.existsSync(wasmPath)) {
        const error = `WASM file not found at ${wasmPath}`
        if (!silent) {
            console.error(`${RED}Error: ${error}${RESET}`)
            console.error('Run "npm run build" first.')
        }
        return {
            name: 'Project Compiler',
            passed: 0, failed: 1, total: 1,
            tests: [{ name: 'WASM Load', passed: false, error }],
            failures: [{ name: 'WASM Load', error }]
        }
    }

    // Initialize runtime
    const runtime = new VovkPLC(wasmPath)

    // Suppress stdout during initialization (unless verbose)
    if (!verboseMode) {
        runtime.stdout_callback = () => {}
    }
    await runtime.initialize(wasmPath, false, false)
    runtime.readStream()

    // Find all test files in samples directory
    const samplesDir = path.join(__dirname, 'samples')
    const files = fs.readdirSync(samplesDir)
    let testCases = files
        .filter(f => f.match(/^test_\d+[a-zA-Z_]*\.project$/))
        .map(f => f.replace('.project', ''))
        .sort()

    // Filter to specific test if provided
    if (testFilter) {
        const filterName = testFilter.replace('.project', '').replace('.output', '')
        // Try exact match first, then prefix match (e.g., "test_01" matches "test_01_base")
        let filtered = testCases.filter(t => t === filterName)
        if (filtered.length === 0) {
            filtered = testCases.filter(t => t.startsWith(filterName + '_') || t.startsWith(filterName))
        }
        testCases = filtered
        if (testCases.length === 0) {
            if (!silent) {
                console.error(`${RED}No test found matching '${testFilter}'${RESET}`)
                console.error(
                    `Available tests: ${files
                        .filter(f => f.endsWith('.project'))
                        .map(f => f.replace('.project', ''))
                        .join(', ')}`,
                )
            }
            return {
                name: 'Project Compiler',
                passed: 0, failed: 1, total: 1,
                tests: [{ name: 'Find Tests', passed: false, error: `No test found matching '${testFilter}'` }],
                failures: [{ name: 'Find Tests', error: `No test found matching '${testFilter}'` }]
            }
        }
    }

    if (testCases.length === 0) {
        if (!silent) console.error(`${RED}No test cases found in ${samplesDir}${RESET}`)
        return {
            name: 'Project Compiler',
            passed: 0, failed: 1, total: 1,
            tests: [{ name: 'Find Tests', passed: false, error: 'No test cases found' }],
            failures: [{ name: 'Find Tests', error: 'No test cases found' }]
        }
    }

    log(`${BOLD}${CYAN}╔══════════════════════════════════════════════════════════════════╗${RESET}`)
    log(`${BOLD}${CYAN}║              Project Compiler Unit Tests                         ║${RESET}`)
    log(`${BOLD}${CYAN}╚══════════════════════════════════════════════════════════════════╝${RESET}`)
    log()
    if (updateMode) {
        log(`${YELLOW}Running in UPDATE mode - regenerating expected outputs${RESET}`)
        log()
    }
    if (verboseMode) {
        log(`${YELLOW}Running in VERBOSE mode - showing compiler debug output${RESET}`)
        log()
    }
    if (runMode) {
        log(`${YELLOW}Running in RUN mode - will explain bytecode after compilation${RESET}`)
        log()
    }
    if (testFilter) {
        log(`Running test: ${testFilter}`)
    } else {
        log(`Found ${testCases.length} test case(s)`)
    }
    log()

    const wasm = runtime.wasm_exports

    // Calculate max test name length for padding
    const maxNameLen = Math.max(...testCases.map(t => t.length))

    for (const testCase of testCases) {
        const projectPath = path.join(samplesDir, `${testCase}.project`)
        const outputPath = path.join(samplesDir, `${testCase}.output`)

        const projectCode = fs.readFileSync(projectPath, 'utf8')

        try {
            // Reset and compile
            wasm.project_reset()
            streamCode(runtime, projectCode)

            if (verboseMode) {
                log(`\n${CYAN}─── Compiling ${testCase} ───${RESET}`)
                log(`${YELLOW}Project source:${RESET}`)
                log(projectCode)
                log(`${YELLOW}Compilation output:${RESET}`)
            }

            const result = wasm.project_compile(verboseMode ? 1 : 0) // 1 = debug output

            if (verboseMode) {
                // Read any output from the stream
                const output = runtime.readStream()
                if (output) log(output)
            }

            // Generate actual output
            const actualOutput = generateOutput(runtime, result)

            // Check if expected output file exists - if not (or in update mode), generate it
            if (!fs.existsSync(outputPath) || updateMode) {
                fs.writeFileSync(outputPath, formatJSON(actualOutput) + '\n')
                const status = actualOutput.success ? `${actualOutput.bytecodeLength} bytes, ${actualOutput.blocks.length} block(s)` : `error: ${actualOutput.problems[0]?.message || 'unknown'}`
                const symbol = updateMode && fs.existsSync(outputPath) ? '~' : '+'
                log(`${CYAN}${symbol}${RESET} ${testCase} - ${updateMode ? 'Updated' : 'Generated'} expected .output file (${status})`)
                passed++
                testResults.push({ name: testCase, passed: true, info: updateMode ? '(updated)' : '(generated)' })
                continue
            }

            // Load expected output
            const expectedOutput = JSON.parse(fs.readFileSync(outputPath, 'utf8'))

            // Compare outputs
            const diffs = compareOutputs(expectedOutput, actualOutput)

            // Collect bytecode for explain at the end
            let bytecodeToExplain = null
            if (runMode && actualOutput.bytecodeLength > 0 && actualOutput.bytecode) {
                bytecodeToExplain = { name: testCase, bytecode: actualOutput.bytecode }
            }

            if (diffs.length > 0) {
                log(`${RED}✗${RESET} ${testCase} - Output mismatch`)
                // Get combined PLCASM for error display (not saved to file)
                const combinedPLCASM = getString(runtime, wasm.project_getCombinedPLCASM())
                testResults.push({ name: testCase, passed: false, error: 'Output mismatch' })
                failures.push({name: testCase, diffs, expected: expectedOutput, actual: actualOutput, combinedPLCASM, bytecodeToExplain})
                failed++
                continue
            }

            // Build status message with aligned columns (only for success cases)
            if (actualOutput.success) {
                const memUsed = actualOutput.memory?.used || 0
                const memAvail = actualOutput.memory?.available || 0
                const flashUsed = actualOutput.flash?.used || actualOutput.bytecodeLength || 0
                const flashSize = actualOutput.flash?.size || 32768

                const paddedName = testCase.padEnd(maxNameLen)

                // Pad numeric values for alignment
                const bytesStr = String(actualOutput.bytecodeLength).padStart(3)
                const stepsStr = String(actualOutput.execution.steps).padStart(2)
                const memUsedStr = String(memUsed).padStart(4)
                const flashUsedStr = String(flashUsed).padStart(5)

                let status = `${bytesStr} bytes  |  ${stepsStr} steps  |  mem: ${memUsedStr}/${memAvail}  |  flash: ${flashUsedStr}/${flashSize}`
                let info = `${actualOutput.bytecodeLength} bytes`
                if (actualOutput.execution.stackSize > 0) {
                    status += `   ${YELLOW}stack: ${actualOutput.execution.stackSize}${RESET}`
                    info += `, stack: ${actualOutput.execution.stackSize}`
                }
                log(`${GREEN}✓${RESET} ${paddedName} ${status}`)
                testResults.push({ name: testCase, passed: true, info })
            } else {
                const status = `expected error: line ${actualOutput.problems[0]?.line || '?'}`
                log(`${GREEN}✓${RESET} ${testCase} (${status})`)
                testResults.push({ name: testCase, passed: true, info: `(${status})` })
            }

            // Store bytecode to explain for passing tests too
            if (bytecodeToExplain) {
                failures.push({name: testCase, bytecodeToExplain, passed: true})
            }

            passed++
        } catch (e) { // @ts-ignore
            log(`${RED}✗${RESET} ${testCase} - Error: ${e.message}`)
            failed++ // @ts-ignore
            testResults.push({ name: testCase, passed: false, error: e.message })
            failures.push({name: testCase, error: e.message})
        }
    }

    log()
    log('─'.repeat(68))

    // Show detailed failures (excluding passed tests that are only there for bytecode explain)
    const actualFailures = failures.filter(f => !f.passed)
    if (actualFailures.length > 0 && !silent) {
        log()
        log(`${BOLD}${RED}Failed Tests:${RESET}`)
        log()

        for (const failure of actualFailures) {
            log(`${BOLD}${failure.name}:${RESET}`)
            if (failure.error) {
                log(`  Error: ${failure.error}`)
            } else if (failure.diffs && failure.diffs.length > 0) {
                // Check if this is a success mismatch where actual failed
                const successDiff = failure.diffs.find(d => d.field === 'success')
                if (successDiff && !failure.actual?.success && failure.actual?.problems?.length > 0) {
                    // Show the actual compilation error prominently
                    const prob = failure.actual.problems[0]
                    log(`  ${RED}Compilation Error:${RESET}`)
                    log(`    ${prob.message}`)
                    if (prob.file || prob.block) {
                        let location = '    at'
                        if (prob.file) location += ` ${prob.file}`
                        if (prob.block) location += `/${prob.block}`
                        if (prob.language) location += ` (${prob.language})`
                        location += ` line ${prob.line}:${prob.column}`
                        log(`${CYAN}${location}${RESET}`)
                    }
                    
                    // Show combined PLCASM with line numbers if available
                    if (failure.combinedPLCASM) {
                        log()
                        log(`  ${CYAN}Combined PLCASM (error at line ${prob.line}):${RESET}`)
                        const lines = failure.combinedPLCASM.split('\n')
                        const errorLineNum = prob.line || 0
                        // Show context: 3 lines before and after the error
                        const startLine = Math.max(0, errorLineNum - 4)
                        const endLine = Math.min(lines.length, errorLineNum + 3)
                        
                        for (let i = startLine; i < endLine; i++) {
                            const lineNum = i + 1  // 1-based
                            const lineNumStr = String(lineNum).padStart(4)
                            const isErrorLine = lineNum === errorLineNum
                            const prefix = isErrorLine ? `${RED}>>` : '  '
                            const suffix = isErrorLine ? RESET : ''
                            log(`  ${prefix}${lineNumStr}│ ${lines[i]}${suffix}`)
                            
                            // Show caret under the error position
                            if (isErrorLine && prob.column) {
                                const col = Math.max(0, prob.column - 1)
                                const tokenLen = prob.token ? prob.token.length : 1
                                const padding = ' '.repeat(8 + col)  // 8 = "  >>XXXX│ " prefix length
                                const caret = '^'.repeat(tokenLen)
                                log(`${padding}${YELLOW}${caret}${RESET}`)
                            }
                        }
                    }
                    log()
                }

                log(`  Differences:`)
                for (const d of failure.diffs) {
                    log(`    ${d.field}:`)
                    log(`      expected: ${RED}${JSON.stringify(d.expected)}${RESET}`)
                    log(`      actual:   ${YELLOW}${JSON.stringify(d.actual)}${RESET}`)
                }
            }
            log()
        }
    }

    // Summary
    const total = passed + failed
    const statusColor = failed > 0 ? RED : GREEN

    // Run explain for all collected bytecodes just before showing results
    if (runMode) {
        const toExplain = failures.filter(f => f.bytecodeToExplain)
        if (toExplain.length > 0) {
            log()
            for (const item of toExplain) { // @ts-ignore
                const { name, bytecode } = item.bytecodeToExplain
                log(`${CYAN}─── Bytecode Explanation for ${name} ───${RESET}`)
                log()
                try {
                    const rootDir = path.resolve(__dirname, '../../..')
                    const result = execSync(`node -e "process.stdout.write('${bytecode}')" | npm run explain --silent`, {
                        cwd: rootDir,
                        encoding: 'utf8',
                        stdio: ['pipe', 'pipe', 'pipe']
                    })
                    log(result)
                } catch (err) {
                    // @ts-ignore
                    log(`${RED}Error running explain: ${err.message}${RESET}`)
                    // @ts-ignore
                    if (err.stdout) log(err.stdout)
                    // @ts-ignore
                    if (err.stderr) log(`${RED}${err.stderr}${RESET}`)
                }
            }
        }
        // Remove the passed entries from failures (they were only there for bytecode)
        for (let i = failures.length - 1; i >= 0; i--) {
            if (failures[i].passed) failures.splice(i, 1)
        }
    }

    log()
    log(`${BOLD}Results: ${statusColor}${passed}/${total} tests passed${RESET}`)

    return {
        name: 'Project Compiler',
        passed,
        failed,
        total,
        tests: testResults,
        failures: actualFailures
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
