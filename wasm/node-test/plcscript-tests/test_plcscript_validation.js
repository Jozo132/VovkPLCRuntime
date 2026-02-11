#!/usr/bin/env node
// test_plcscript_validation.js - PLCScript Compiler Validation Tests
//
// Tests error detection and validation in the PLCScript compiler:
//   - Duplicate parameter names
//   - Const without initializer
//   - Built-in function name conflict
//   - Duplicate struct field names
//   - Variable shadowing (warning via project problems)
//   - Division by zero (warning via project problems)
//   - Unreachable code (warning via project problems)
//   - Empty block (warning via project problems)
//   - Missing return path (warning via project problems)
//
// Usage:
//   node wasm/node-test/plcscript-tests/test_plcscript_validation.js
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
 * Run all PLCScript validation unit tests
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
     * @param {string} script 
     * @returns {{plcasm: string, error: string | null, problems: Problem[]}}
     */
    function compilePLCScript(script) {
        const project = `
VOVKPLCPROJECT TestProject
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

PROGRAM main
    BLOCK LANG=PLCSCRIPT Main
${script}
    END_BLOCK
END_PROGRAM
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
     * @param {string} script 
     * @param {string} expectedError 
     */
    function assertCompileError(script, expectedError) {
        const result = compilePLCScript(script)
        assert(result.error !== null, `Expected compilation error but succeeded.\nPLCASM:\n${result.plcasm}`)
        assert(
            result.error.toLowerCase().includes(expectedError.toLowerCase()),
            `Expected error "${expectedError}" but got: "${result.error}"`
        )
    }

    /**
     * @param {string} script 
     * @returns {{plcasm: string, problems: Problem[]}}
     */
    function assertCompileSuccess(script) {
        const result = compilePLCScript(script)
        assert(result.error === null, `Expected success but got error: "${result.error}"`)
        return { plcasm: result.plcasm, problems: result.problems }
    }

    /**
     * @param {string} script 
     * @param {string} expectedWarning 
     */
    function assertCompileWarning(script, expectedWarning) {
        const result = assertCompileSuccess(script)
        const warnings = result.problems.filter(p => p.severity === 'warning')
        const found = warnings.find(p => p.message.toLowerCase().includes(expectedWarning.toLowerCase()))
        assert(found !== undefined,
            `Expected warning "${expectedWarning}" but got: ${JSON.stringify(warnings.map(w => w.message))}`
        )
    }

    /**
     * @param {string} script 
     */
    function assertCompileClean(script) {
        const result = assertCompileSuccess(script)
        const warnings = result.problems.filter(p => p.severity === 'warning')
        assert(warnings.length === 0,
            `Expected no warnings but got ${warnings.length}: ${JSON.stringify(warnings.map(w => w.message))}`
        )
    }

    // === Error Detection Tests ===

    test('Const without initializer', () => {
        assertCompileError(`const LIMIT: i32;`, 'Constant declaration requires an initializer')
    })

    test('Const without initializer (with address)', () => {
        assertCompileError(`const LIMIT: i32 @ M0;`, 'Constant declaration requires an initializer')
    })

    test('Duplicate parameter names', () => {
        assertCompileError(
            `function add(a: i32, a: i32): i32 {\n    return a + a;\n}\nlet r: i32 @ M0 = add(1, 2);`,
            'Duplicate parameter name'
        )
    })

    test('Built-in function name conflict (TON)', () => {
        assertCompileError(`function TON(): i32 { return 0; }`, 'Function name conflicts with built-in PLC function')
    })

    test('Built-in function name conflict (CTU)', () => {
        assertCompileError(`function CTU(): i32 { return 0; }`, 'Function name conflicts with built-in PLC function')
    })

    test('Built-in function name conflict (risingEdge)', () => {
        assertCompileError(`function risingEdge(): i32 { return 0; }`, 'Function name conflicts with built-in PLC function')
    })

    test('Duplicate struct field names', () => {
        assertCompileError(`type Motor = struct {\n    speed: i32;\n    speed: f32;\n}`, 'Duplicate field name in struct')
    })

    test('Duplicate symbol in same scope', () => {
        assertCompileError(`let x: i32 @ M0 = 0;\nlet x: i32 @ M4 = 1;`, 'Duplicate symbol')
    })

    test('Undefined variable', () => {
        assertCompileError(`let x: i32 @ M0 = y;`, 'Undefined variable')
    })

    test('Cannot assign to const', () => {
        assertCompileError(`const PI: f32 = 3.14;\nlet dummy: f32 @ M0 = 0;\nPI = 2.71;`, 'Cannot assign to const')
    })

    test('Break outside of loop', () => {
        assertCompileError(`break;`, 'break')
    })

    test('Continue outside of loop', () => {
        assertCompileError(`continue;`, 'continue')
    })

    test('Nested functions not supported', () => {
        assertCompileError(
            `function outer(): i32 {\n    function inner(): i32 { return 0; }\n    return 0;\n}\nlet x: i32 @ M0 = outer();`,
            'Nested functions'
        )
    })

    test('Function already defined', () => {
        assertCompileError(
            `function foo(): i32 { return 0; }\nfunction foo(): i32 { return 0; }\nlet x: i32 @ M0 = 0;`,
            'already defined'
        )
    })

    test('Too many function parameters', () => {
        const params = Array.from({ length: 17 }, (_, i) => `p${i}: i32`).join(', ')
        assertCompileError(`function f(${params}): i32 { return 0; }\nlet x: i32 @ M0 = 0;`, 'Too many function parameters')
    })

    test('Non-void function returning void', () => {
        assertCompileError(
            `function compute(): i32 {\n    return;\n}\nlet r: i32 @ M0 = compute();`,
            'Non-void function must return a value'
        )
    })

    test('Variable name conflicts with type name', () => {
        assertCompileError(`type Motor = struct { speed: i32; }\nlet Motor: i32 @ M0 = 0;`, 'Variable name conflicts with a type name')
    })

    test('Too few arguments', () => {
        assertCompileError(
            `function add(a: i32, b: i32): i32 { return a; }\nlet r: i32 @ M0 = add(1);`,
            'Too few arguments'
        )
    })

    test('String without capacity', () => {
        assertCompileError(`let s: str8 @ M0;`, 'String type requires capacity')
    })

    test('Arrays of bool not supported', () => {
        assertCompileError(`let flags: bool[10] @ M0;`, 'Arrays of bool')
    })

    // === Valid Code Tests ===

    test('Const with initializer compiles', () => {
        assertCompileClean(`const LIMIT: i32 = 100;`)
    })

    test('Unique parameter names compile', () => {
        assertCompileClean(`function add(a: i32, b: i32): i32 {\n    return a + b;\n}\nlet r: i32 @ M0 = add(1, 2);`)
    })

    test('Unique struct fields compile', () => {
        assertCompileClean(`type Motor = struct {\n    speed: i32;\n    direction: bool;\n}`)
    })

    test('User function with non-builtin name compiles', () => {
        assertCompileClean(`function calculate(): i32 { return 42; }\nlet r: i32 @ M0 = calculate();`)
    })

    test('Variable shadowing in inner scope compiles (warning only)', () => {
        assertCompileSuccess(`let x: i32 @ M0 = 10;\nif (x > 5) {\n    let x: i32 @ M4 = 20;\n    x = x + 1;\n}`)
    })

    test('Clean code with no warnings', () => {
        assertCompileClean(`let counter: i32 @ M0 = 0;\ncounter = counter + 1;\nif (counter > 10) {\n    counter = 0;\n}`)
    })

    test('Function with return has no warnings', () => {
        assertCompileClean(`function double(x: i32): i32 {\n    return x * 2;\n}\nlet r: i32 @ M0 = double(21);`)
    })

    // === Warning Detection Tests ===

    test('Warning: variable shadowing', () => {
        assertCompileWarning(
            `let x: i32 @ M0 = 10;\nif (x > 5) {\n    let x: i32 @ M4 = 20;\n    x = x + 1;\n}`,
            'shadow'
        )
    })

    test('Warning: division by constant zero', () => {
        assertCompileWarning(`let x: i32 @ M0 = 10;\nlet y: i32 @ M4 = x / 0;`, 'division by constant zero')
    })

    test('Warning: empty block', () => {
        assertCompileWarning(`let x: i32 @ M0 = 10;\nif (x > 5) {\n}`, 'empty block')
    })

    test('Warning: unreachable code after return', () => {
        assertCompileWarning(
            `function compute(): i32 {\n    return 42;\n    let x: i32 @ M0 = 10;\n}\nlet r: i32 @ M0 = compute();`,
            'unreachable'
        )
    })

    test('Warning: unreachable code after break', () => {
        assertCompileWarning(
            `let x: i32 @ M0 = 0;\nwhile (x < 10) {\n    break;\n    x = x + 1;\n}`,
            'unreachable'
        )
    })

    test('Warning: missing return path', () => {
        assertCompileWarning(
            `function maybe(x: i32): i32 {\n    if (x > 0) {\n        return x;\n    }\n}\nlet r: i32 @ M0 = maybe(1);`,
            'not all code paths return'
        )
    })

    // === Illegal Symbol Name Tests ===

    test('PLC address as variable name (M0)', () => {
        assertCompileError(`let M0: i32 = 0;`, 'conflicts with a PLC address')
    })

    test('PLC address as variable name (X0)', () => {
        assertCompileError(`let X0: i32 = 0;`, 'conflicts with a PLC address')
    })

    test('PLC address as variable name (Y0)', () => {
        assertCompileError(`let Y0: i32 = 0;`, 'conflicts with a PLC address')
    })

    test('PLC address as variable name (I0)', () => {
        assertCompileError(`let I0: i32 = 0;`, 'conflicts with a PLC address')
    })

    test('PLC address as variable name (Q0)', () => {
        assertCompileError(`let Q0: i32 = 0;`, 'conflicts with a PLC address')
    })

    test('PLC address as variable name (T0)', () => {
        assertCompileError(`let T0: i32 = 0;`, 'conflicts with a PLC address')
    })

    test('PLC address as variable name (C0)', () => {
        assertCompileError(`let C0: i32 = 0;`, 'conflicts with a PLC address')
    })

    test('PLC address as variable name (S0)', () => {
        assertCompileError(`let S0: i32 = 0;`, 'conflicts with a PLC address')
    })

    test('PLC address case insensitive (m0)', () => {
        assertCompileError(`let m0: i32 = 0;`, 'conflicts with a PLC address')
    })

    test('PLC address with W suffix (MW10)', () => {
        assertCompileError(`let MW10: i32 = 0;`, 'conflicts with a PLC address')
    })

    test('PLC address with D suffix (MD20)', () => {
        assertCompileError(`let MD20: i32 = 0;`, 'conflicts with a PLC address')
    })

    test('Reserved keyword as variable name (TON)', () => {
        assertCompileError(`let TON: i32 = 0;`, 'reserved keyword')
    })

    test('Reserved keyword as variable name (AND)', () => {
        assertCompileError(`let AND: i32 = 0;`, 'reserved keyword')
    })

    test('Reserved keyword as variable name (ctud)', () => {
        assertCompileError(`let ctud: i32 = 0;`, 'reserved keyword')
    })

    test('Reserved keyword as variable name (var)', () => {
        assertCompileError(`let var: i32 = 0;`, 'reserved keyword')
    })

    test('Reserved keyword as variable name (MOD)', () => {
        assertCompileError(`let MOD: i32 = 0;`, 'reserved keyword')
    })

    test('Project symbol with PLC address name (M0)', () => {
        const project = `
VOVKPLCPROJECT TestProject
VERSION 1.0.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    M 512
END_MEMORY

SYMBOLS
    M0 : i32 : M0
END_SYMBOLS

PROGRAM main
    BLOCK LANG=PLCSCRIPT Main
let x: i32 @ M4 = 0;
    END_BLOCK
END_PROGRAM
`
        const wasm = plc.wasm_exports
        wasm.project_reset()
        streamCode(project)
        const success = wasm.project_compile(0)
        assert(!success, 'Expected compilation to fail')
        const error = getString(wasm.project_getError())
        assert(error.toLowerCase().includes('conflicts with a plc address'),
            `Expected PLC address error but got: "${error}"`)
    })

    test('Project symbol with reserved keyword name (ADD)', () => {
        const project = `
VOVKPLCPROJECT TestProject
VERSION 1.0.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    M 512
END_MEMORY

SYMBOLS
    ADD : i32 : M0
END_SYMBOLS

PROGRAM main
    BLOCK LANG=PLCSCRIPT Main
let x: i32 @ M4 = 0;
    END_BLOCK
END_PROGRAM
`
        const wasm = plc.wasm_exports
        wasm.project_reset()
        streamCode(project)
        const success = wasm.project_compile(0)
        assert(!success, 'Expected compilation to fail')
        const error = getString(wasm.project_getError())
        assert(error.toLowerCase().includes('reserved word'),
            `Expected reserved word error but got: "${error}"`)
    })

    return { name: 'PLCScript Validation', passed, failed, total: passed + failed, tests: testResults, failures }
}

// Standalone execution
const isMainModule = process.argv[1] && (
    process.argv[1].includes('test_plcscript_validation') ||
    fileURLToPath(import.meta.url) === process.argv[1]
)

if (isMainModule) {
    const RED = '\x1b[31m'
    const GREEN = '\x1b[32m'
    const RESET = '\x1b[0m'

    console.log('PLCScript Validation Tests')
    console.log('══════════════════════════')

    const result = await runTests()

    for (const t of result.tests) {
        console.log(`  ${t.passed ? GREEN + '✓' : RED + '✗'} ${t.name}${RESET}`)
        if (!t.passed && t.error) console.log(`    ${RED}${t.error}${RESET}`)
    }

    console.log()
    console.log(`${result.passed} passed, ${result.failed} failed`)

    if (result.failed > 0) process.exit(1)
}
