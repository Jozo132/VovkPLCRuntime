#!/usr/bin/env node
// test_st_validation.js - ST Compiler Validation Tests
//
// Tests error detection and validation in the ST (Structured Text) compiler:
//   - Duplicate variable names
//   - Constant without initializer
//   - Cannot assign to CONSTANT
//   - EXIT outside of loop
//   - Variable name conflicts with type name
//   - Division by constant zero (warning via project problems)
//   - Unreachable code after EXIT/RETURN (warning via project problems)
//   - Empty IF block (warning via project problems)
//   - Unused variable (warning via project problems)
//
// Usage:
//   node wasm/node-test/st-tests/test_st_validation.js
//
// @ts-check
'use strict'

import VovkPLC from '../../dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

/**
 * @typedef {{ name: string, passed: boolean, error?: string }} TestResult
 * @typedef {{ passed: number, failed: number, results: TestResult[], failures: string[] }} SuiteResult
 * @typedef {{ severity: string, message: string, line: number, column: number, block?: string, language?: string, token?: string }} Problem
 */

const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')

/**
 * Run all ST validation unit tests
 * @param {Object} [options]
 * @param {boolean} [options.silent] - Suppress console output
 * @param {VovkPLC} [options.runtime] - Shared WASM runtime instance
 * @returns {Promise<SuiteResult>}
 */
export async function runTests(options = {}) {
    const { silent = false, runtime: sharedRuntime = null } = options
    const log = silent ? () => {} : console.log.bind(console)

    // Use shared runtime if provided, otherwise create one
    let plc = sharedRuntime
    if (!plc) {
        plc = new VovkPLC()
        plc.stdout_callback = () => {}
        await plc.initialize(wasmPath, false, true)
    }

    /** @type {TestResult[]} */
    const testResults = []
    /** @type {string[]} */
    const failures = []
    let passed = 0
    let failed = 0

    /**
     * @param {string} name 
     * @param {() => void} fn 
     */
    function test(name, fn) {
        try {
            fn()
            testResults.push({ name, passed: true })
            passed++
        } catch (e) {
            testResults.push({ name, passed: false, error: e.message })
            failures.push(`${name}: ${e.message}`)
            failed++
        }
    }

    /**
     * @param {boolean} condition 
     * @param {string} message 
     */
    function assert(condition, message) {
        if (!condition) {
            throw new Error(message || 'Assertion failed')
        }
    }

    /**
     * @param {string} code 
     */
    function streamCode(code) {
        plc.wasm_exports.streamClear()
        for (let i = 0; i < code.length; i++) {
            plc.wasm_exports.streamIn(code.charCodeAt(i))
        }
    }

    /**
     * @param {number} ptr 
     * @returns {string}
     */
    function getString(ptr) {
        if (!ptr) return ''
        // @ts-ignore
        const memory = new Uint8Array(plc.wasm.exports.memory.buffer)
        let str = ''
        let i = ptr
        while (memory[i] !== 0 && i < memory.length) {
            str += String.fromCharCode(memory[i])
            i++
        }
        return str
    }

    /**
     * Read accumulated problems from the project compiler
     * @returns {Problem[]}
     */
    function readProjectProblems() {
        const wasm = plc.wasm_exports
        // @ts-ignore
        const wasmMemory = plc.wasm.exports.memory.buffer

        const problemCount = wasm.project_getProblemCount ? wasm.project_getProblemCount() : 0
        if (problemCount === 0 || !wasm.project_getProblems) return []

        const pointer = wasm.project_getProblems()
        const struct_size = 344
        const view = new DataView(wasmMemory)

        /** @type {Problem[]} */
        const problems = []
        for (let i = 0; i < problemCount; i++) {
            const offset = pointer + i * struct_size
            const type_int = view.getUint32(offset + 0, true)
            const line = view.getUint32(offset + 4, true)
            const column = view.getUint32(offset + 8, true)

            let message = ''
            for (let j = 0; j < 128; j++) {
                const charCode = view.getUint8(offset + 16 + j)
                if (charCode === 0) break
                message += String.fromCharCode(charCode)
            }

            let block = ''
            for (let j = 0; j < 64; j++) {
                const charCode = view.getUint8(offset + 144 + j)
                if (charCode === 0) break
                block += String.fromCharCode(charCode)
            }

            let token = ''
            for (let j = 0; j < 64; j++) {
                const charCode = view.getUint8(offset + 276 + j)
                if (charCode === 0) break
                token += String.fromCharCode(charCode)
            }

            /** @type {Problem} */
            const problem = {
                severity: type_int === 2 ? 'error' : type_int === 1 ? 'warning' : 'info',
                message, line, column,
            }
            if (block) problem.block = block
            if (token) problem.token = token
            problems.push(problem)
        }
        return problems
    }

    /**
     * Compile an ST block through the project compiler
     * @param {string} stCode
     * @returns {{plcasm: string, error: string | null, problems: Problem[]}}
     */
    function compileST(stCode) {
        const project = `
VOVKPLCPROJECT TestSTProject
VERSION 1.0.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    M 512
    T 16
    C 8
END_MEMORY

FILE main
    BLOCK LANG=ST Main
${stCode}
    END_BLOCK
END_FILE
`
        const wasm = plc.wasm_exports
        wasm.project_reset()
        streamCode(project)
        const success = wasm.project_compile(0)
        const problems = readProjectProblems()

        if (!success) {
            const errorMsg = getString(wasm.project_getError())
            return { plcasm: '', error: errorMsg, problems }
        }
        const plcasm = getString(wasm.project_getCombinedPLCASM())
        return { plcasm, error: null, problems }
    }

    /**
     * Compile ST code using the direct ST compiler (not project compiler)
     * Used for tests that need ST compiler errors directly
     * @param {string} stCode
     * @returns {{ output: string, error: string | null, errorLine: number, errorColumn: number }}
     */
    function compileSTDirect(stCode) {
        const wasm = plc.wasm_exports
        wasm.streamClear()
        streamCode(stCode)
        wasm.st_compiler_load_from_stream()
        const errorCode = wasm.st_compiler_compile()

        if (errorCode !== 0) {
            const error = getString(wasm.st_compiler_get_error())
            return {
                output: '',
                error,
                errorLine: wasm.st_compiler_get_error_line(),
                errorColumn: wasm.st_compiler_get_error_column()
            }
        }

        // Read output from stream
        wasm.st_compiler_output_to_stream()
        let output = ''
        let available = wasm.streamAvailable()
        for (let i = 0; i < available; i++) {
            output += String.fromCharCode(wasm.streamOut())
        }
        return { output, error: null, errorLine: 0, errorColumn: 0 }
    }

    /**
     * Assert that ST code produces a compile error containing the expected message
     * @param {string} stCode 
     * @param {string} expectedError 
     */
    function assertSTError(stCode, expectedError) {
        const result = compileSTDirect(stCode)
        assert(result.error !== null, `Expected compilation error but succeeded.\nOutput:\n${result.output}`)
        assert(
            result.error.toLowerCase().includes(expectedError.toLowerCase()),
            `Expected error "${expectedError}" but got: "${result.error}"`
        )
    }

    /**
     * Assert that ST code compiles successfully via the project compiler
     * @param {string} stCode 
     * @returns {{plcasm: string, problems: Problem[]}}
     */
    function assertCompileSuccess(stCode) {
        const result = compileST(stCode)
        assert(result.error === null, `Expected success but got error: "${result.error}"`)
        return { plcasm: result.plcasm, problems: result.problems }
    }

    /**
     * Assert that ST code produces a warning containing the expected message
     * @param {string} stCode 
     * @param {string} expectedWarning 
     */
    function assertCompileWarning(stCode, expectedWarning) {
        const result = assertCompileSuccess(stCode)
        const warnings = result.problems.filter(p => p.severity === 'warning')
        const found = warnings.find(p => p.message.toLowerCase().includes(expectedWarning.toLowerCase()))
        assert(found !== undefined,
            `Expected warning "${expectedWarning}" but got: ${JSON.stringify(warnings.map(w => w.message))}`
        )
    }

    /**
     * Assert that ST code compiles cleanly with no warnings
     * @param {string} stCode 
     */
    function assertCompileClean(stCode) {
        const result = assertCompileSuccess(stCode)
        const warnings = result.problems.filter(p => p.severity === 'warning')
        assert(warnings.length === 0,
            `Expected no warnings but got ${warnings.length}: ${JSON.stringify(warnings.map(w => w.message))}`
        )
    }

    // === Error Detection Tests ===

    test('Duplicate variable name', () => {
        assertSTError(`
VAR
    counter : INT;
    counter : INT;
END_VAR
counter := 0;
`, 'Duplicate variable name')
    })

    test('Constant without initializer', () => {
        assertSTError(`
VAR CONSTANT
    LIMIT : INT;
END_VAR
`, 'Constant declaration requires an initializer')
    })

    test('Cannot assign to CONSTANT', () => {
        assertSTError(`
VAR CONSTANT
    PI : REAL := 3.14;
END_VAR
VAR
    dummy : REAL;
END_VAR
PI := 2.71;
dummy := PI;
`, 'Cannot assign to CONSTANT')
    })

    test('EXIT outside of loop', () => {
        assertSTError(`
VAR
    x : INT;
END_VAR
x := 0;
EXIT;
`, 'EXIT statement outside of loop')
    })

    test('Expected variable name in VAR', () => {
        assertSTError(`
VAR
    123bad : INT;
END_VAR
`, 'Expected variable name')
    })

    test('Expected type after colon', () => {
        assertSTError(`
VAR
    x : ;
END_VAR
`, 'Expected type')
    })

    test('Expected END_VAR', () => {
        assertSTError(`
VAR
    x : INT;
`, 'Expected END_VAR')
    })

    test('Expected THEN after IF condition', () => {
        assertSTError(`
VAR
    x : INT;
END_VAR
x := 0;
IF x > 0
    x := 1;
END_IF;
`, 'Expected THEN')
    })

    test('Expected END_IF', () => {
        assertSTError(`
VAR
    x : INT;
END_VAR
x := 0;
IF x > 0 THEN
    x := 1;
`, 'Expected END_IF')
    })

    test('Expected DO in FOR loop', () => {
        assertSTError(`
VAR
    i : INT;
END_VAR
FOR i := 0 TO 10
    i := i;
END_FOR;
`, 'Expected DO')
    })

    test('Expected DO in WHILE loop', () => {
        assertSTError(`
VAR
    x : INT;
END_VAR
x := 0;
WHILE x < 10
    x := x + 1;
END_WHILE;
`, 'Expected DO')
    })

    test('No source code', () => {
        assertSTError('', 'No source code')
    })

    // === Valid Code Tests ===

    test('Constant with initializer compiles', () => {
        assertCompileClean(`
VAR CONSTANT
    LIMIT : INT := 100;
END_VAR
VAR
    x : INT;
END_VAR
x := LIMIT;
`)
    })

    test('Unique variable names compile', () => {
        assertCompileClean(`
VAR
    counter : INT;
    limit : INT;
END_VAR
counter := 0;
limit := 10;
`)
    })

    test('Variable in FOR loop not unused', () => {
        assertCompileClean(`
VAR
    i : INT;
    sum : INT;
END_VAR
sum := 0;
FOR i := 0 TO 10 DO
    sum := sum + i;
END_FOR;
`)
    })

    test('Variable in expression not unused', () => {
        assertCompileClean(`
VAR
    a : INT;
    b : INT;
END_VAR
a := 10;
b := a + 5;
`)
    })

    test('EXIT inside loop is valid', () => {
        assertCompileClean(`
VAR
    x : INT;
END_VAR
x := 0;
WHILE x < 10 DO
    x := x + 1;
    IF x > 5 THEN
        EXIT;
    END_IF;
END_WHILE;
`)
    })

    test('RETURN compiles cleanly', () => {
        assertCompileClean(`
VAR
    x : INT;
END_VAR
x := 0;
IF x > 10 THEN
    RETURN;
END_IF;
x := x + 1;
`)
    })

    test('Nested loops compile', () => {
        assertCompileClean(`
VAR
    i : INT;
    j : INT;
    result : INT;
END_VAR
result := 0;
FOR i := 0 TO 5 DO
    FOR j := 0 TO 3 DO
        result := result + 1;
    END_FOR;
END_FOR;
`)
    })

    test('CASE statement compiles', () => {
        assertCompileClean(`
VAR
    mode : INT;
    output : INT;
END_VAR
mode := 1;
CASE mode OF
    0: output := 0;
    1: output := 10;
    2: output := 20;
ELSE
    output := -1;
END_CASE;
`)
    })

    test('REPEAT UNTIL compiles', () => {
        assertCompileClean(`
VAR
    x : INT;
END_VAR
x := 0;
REPEAT
    x := x + 1;
UNTIL x >= 10
END_REPEAT;
`)
    })

    // === Warning Detection Tests ===

    test('Warning: unused variable', () => {
        assertCompileWarning(`
VAR
    x : INT;
    unused_var : INT;
END_VAR
x := 42;
`, 'Unused variable')
    })

    test('Warning: division by constant zero', () => {
        assertCompileWarning(`
VAR
    x : INT;
    y : INT;
END_VAR
x := 10;
y := x / 0;
`, 'Division by constant zero')
    })

    test('Warning: unreachable code after RETURN', () => {
        assertCompileWarning(`
VAR
    x : INT;
END_VAR
x := 0;
RETURN;
x := 1;
`, 'Unreachable code')
    })

    test('Warning: unreachable code after EXIT', () => {
        assertCompileWarning(`
VAR
    x : INT;
END_VAR
x := 0;
WHILE x < 10 DO
    EXIT;
    x := x + 1;
END_WHILE;
`, 'Unreachable code')
    })

    test('Warning: empty IF block', () => {
        assertCompileWarning(`
VAR
    x : INT;
END_VAR
x := 0;
IF x > 0 THEN
END_IF;
x := 1;
`, 'Empty IF block')
    })

    test('No warning when all variables are used', () => {
        assertCompileClean(`
VAR
    a : INT;
    b : INT;
    c : INT;
END_VAR
a := 1;
b := a + 2;
c := a + b;
`)
    })

    return { name: 'ST Validation', passed, failed, total: passed + failed, tests: testResults, failures }
}

// Standalone execution
const isMainModule = process.argv[1] && (
    process.argv[1].includes('test_st_validation') ||
    fileURLToPath(import.meta.url) === process.argv[1]
)

if (isMainModule) {
    const RED = '\x1b[31m'
    const GREEN = '\x1b[32m'
    const RESET = '\x1b[0m'

    console.log('ST Validation Tests')
    console.log('═══════════════════')

    const result = await runTests()

    for (const t of result.tests) {
        console.log(`  ${t.passed ? GREEN + '✓' : RED + '✗'} ${t.name}${RESET}`)
        if (!t.passed && t.error) console.log(`    ${RED}${t.error}${RESET}`)
    }

    console.log()
    console.log(`${result.passed} passed, ${result.failed} failed`)

    if (result.failed > 0) process.exit(1)
}
