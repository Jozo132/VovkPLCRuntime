#!/usr/bin/env node
// test_plcscript_type_inference.js - PLCScript Type Inference Tests
//
// Tests the type inference system for PLCScript variable declarations:
//   - Type inferred from address prefix (M→u8, MW→i16, MD→i32, bit→bool)
//   - Type inferred from initializer expression
//   - Fewer cvt instructions with correct inference
//   - Symbol table export with inferred flag
//   - lintPLCScript API with symbols and problems
//
// Usage:
//   node wasm/node-test/plcscript-tests/test_plcscript_type_inference.js
//
// @ts-check
'use strict'

import VovkPLC from '../../dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)
const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')

/**
 * @typedef {Object} TestResult
 * @property {string} name
 * @property {boolean} passed
 * @property {string} [error]
 * @property {string} [info]
 */

/**
 * @typedef {Object} SuiteResult
 * @property {string} name
 * @property {number} passed
 * @property {number} failed
 * @property {number} total
 * @property {TestResult[]} tests
 * @property {string[]} [failures]
 */

/**
 * @param {{ silent?: boolean, runtime?: VovkPLC | null }} [options]
 * @returns {Promise<SuiteResult>}
 */
export async function runTests(options = {}) {
    const { silent = false, runtime: sharedRuntime = null } = options

    let plc = sharedRuntime
    if (!plc) {
        plc = new VovkPLC(wasmPath)
        plc.stdout_callback = () => {}
        await plc.initialize(wasmPath, false, true)
    }

    /** @type {TestResult[]} */
    const testResults = []
    /** @type {string[]} */
    const failures = []
    let passed = 0
    let failed = 0

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

    function assert(condition, message) {
        if (!condition) throw new Error(message || 'Assertion failed')
    }

    function streamCode(code) {
        plc.wasm_exports.streamClear()
        for (let i = 0; i < code.length; i++) {
            plc.wasm_exports.streamIn(code.charCodeAt(i))
        }
    }

    function getString(ptr) {
        if (!ptr) return ''
        const memory = new Uint8Array(plc.wasm.exports.memory.buffer)
        let str = ''
        let i = ptr
        while (memory[i] !== 0 && i < memory.length) {
            str += String.fromCharCode(memory[i])
            i++
        }
        return str
    }

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

        if (!success) {
            const errorMsg = getString(wasm.project_getError())
            return { plcasm: '', bytecode: '', error: errorMsg }
        }

        const plcasm = getString(wasm.project_getCombinedPLCASM())
        const bytecodePtr = wasm.project_getBytecode()
        const bytecodeLen = wasm.project_getBytecodeLength()
        let bytecode = ''
        if (bytecodePtr && bytecodeLen > 0) {
            const memory = new Uint8Array(plc.wasm.exports.memory.buffer)
            for (let i = 0; i < bytecodeLen; i++) {
                bytecode += memory[bytecodePtr + i].toString(16).padStart(2, '0').toUpperCase()
            }
        }

        return { plcasm, bytecode, error: null }
    }

    function compileSuccess(script) {
        const result = compilePLCScript(script)
        if (result.error) {
            const err = new Error(`Compilation failed: ${result.error}`)
            // @ts-ignore
            err.plcasm = result.plcasm
            throw err
        }
        return result
    }

    function countOccurrences(str, sub) {
        let count = 0
        let pos = 0
        while ((pos = str.indexOf(sub, pos)) !== -1) {
            count++
            pos += sub.length
        }
        return count
    }

    // === Type Inference from Address Prefix ===

    test('Infer u8 from byte address M0', () => {
        const result = compileSuccess(`
let x @ M0 = 5
`)
        assert(result.plcasm.includes('u8.const 5'), 'Should emit u8.const (inferred u8 from M0)')
        assert(result.plcasm.includes('u8.move_to'), 'Should emit u8.move_to')
        assert(!result.plcasm.includes('cvt'), 'Should have no cvt instruction')
        assert(result.plcasm.includes('[inferred]'), 'Comment should mark as inferred')
    })

    test('Infer i16 from word address MW0', () => {
        const result = compileSuccess(`
let x @ MW0 = 100
`)
        assert(result.plcasm.includes('i16.const 100'), 'Should emit i16.const (inferred i16 from MW)')
        assert(!result.plcasm.includes('cvt'), 'Should have no cvt instruction')
        assert(result.plcasm.includes('[inferred]'), 'Comment should mark as inferred')
    })

    test('Infer i32 from dword address MD0', () => {
        const result = compileSuccess(`
let x @ MD0 = 42
`)
        assert(result.plcasm.includes('i32.const 42'), 'Should emit i32.const (inferred i32 from MD)')
        assert(!result.plcasm.includes('cvt'), 'Should have no cvt instruction')
        assert(result.plcasm.includes('[inferred]'), 'Comment should mark as inferred')
    })

    test('Infer bool from bit address M0.0', () => {
        const result = compileSuccess(`
let flag @ M100.0 = true
`)
        assert(result.plcasm.includes('bool'), 'Should use bool type')
        assert(result.plcasm.includes('[inferred]'), 'Comment should mark as inferred')
    })

    test('Infer u8 from Y address', () => {
        const result = compileSuccess(`
let out @ Y0 = 1
`)
        assert(result.plcasm.includes('u8.const 1'), 'Should emit u8.const (inferred u8 from Y0)')
        assert(result.plcasm.includes('[inferred]'), 'Comment should mark as inferred')
    })

    test('Infer bool from bit address Y0.0', () => {
        const result = compileSuccess(`
let out @ Y0.0 = true
`)
        assert(result.plcasm.includes('bool'), 'Should use bool type')
        assert(result.plcasm.includes('[inferred]'), 'Comment should mark as inferred')
    })

    test('Explicit type overrides address inference', () => {
        const result = compileSuccess(`
let x: i32 @ MW0 = 42
`)
        assert(result.plcasm.includes('i32.const 42'), 'Should use explicit i32, not inferred i16')
        assert(!result.plcasm.includes('[inferred]'), 'Should NOT be marked as inferred')
    })

    // === Type Inference from Initializer Expression ===

    test('Infer i32 from integer literal (local variable)', () => {
        const result = compileSuccess(`
let x = 42
`)
        assert(result.plcasm.includes('i32.const 42'), 'Should emit i32.const 42')
        assert(!result.plcasm.includes('cvt'), 'Should have no cvt instruction')
        assert(result.plcasm.includes('[inferred]'), 'Comment should mark as inferred')
    })

    test('Infer f32 from float literal', () => {
        const result = compileSuccess(`
let pi = 3.14
`)
        assert(result.plcasm.includes('f32.const'), 'Should emit f32.const')
        assert(!result.plcasm.includes('cvt'), 'Should have no cvt instruction')
        assert(result.plcasm.includes('[inferred]'), 'Comment should mark as inferred')
    })

    test('Infer bool from boolean literal', () => {
        const result = compileSuccess(`
let flag = true
`)
        assert(result.plcasm.includes('bool'), 'Should use bool type')
        assert(!result.plcasm.includes('cvt'), 'Should have no cvt instruction')
        assert(result.plcasm.includes('[inferred]'), 'Comment should mark as inferred')
    })

    test('Infer type from typed variable reference', () => {
        const result = compileSuccess(`
let a: u16 @ MW0 = 100
let b = a
`)
        assert(result.plcasm.includes('// let b: u16'), 'Should infer u16 from variable a')
        assert(result.plcasm.includes('[inferred]'), 'b should be marked as inferred')
    })

    // === Fewer cvt Instructions ===

    test('No cvt when address matches literal type', () => {
        const result = compileSuccess(`
let a @ M0 = 10
let b @ MW2 = 200
let c @ MD4 = 50000
`)
        const cvtCount = countOccurrences(result.plcasm, 'cvt ')
        assert(cvtCount === 0, `Expected 0 cvt instructions, got ${cvtCount}.\n${result.plcasm}`)
    })

    test('cvt still emitted when types genuinely differ', () => {
        const result = compileSuccess(`
let a: f32 @ MD0 = 100
`)
        assert(result.plcasm.includes('cvt'), 'Should have cvt for int→float conversion')
    })

    test('Inferred type from address eliminates cvt in compound expression', () => {
        const result = compileSuccess(`
let x @ MW0 = 10
let y @ MW2 = 20
let result @ MW4
result = x + y
`)
        const cvtCount = countOccurrences(result.plcasm, 'cvt ')
        assert(cvtCount === 0, `Expected 0 cvt instructions, got ${cvtCount}.\n${result.plcasm}`)
    })

    // === Edge Cases ===

    test('No type, no initializer defaults to i32', () => {
        const result = compileSuccess(`
let x @ auto
`)
        assert(result.plcasm.includes('i32'), 'Should default to i32 when no type or initializer')
        assert(result.plcasm.includes('[inferred]'), 'Should be marked as inferred')
    })

    test('String variable requires explicit type', () => {
        const result = compilePLCScript(`
let msg = "hello"
`)
        assert(result.error !== null, 'Should fail: string variables need explicit type with capacity')
    })

    test('Explicit type with auto address still works', () => {
        const result = compileSuccess(`
let x: u8 @ auto = 42
`)
        assert(result.plcasm.includes('u8.const 42'), 'Should use explicit u8 type')
        assert(!result.plcasm.includes('[inferred]'), 'Should NOT be marked as inferred')
    })

    test('Const with inferred type', () => {
        const result = compileSuccess(`
const PI = 3.14159
`)
        assert(result.plcasm.includes('f32.const'), 'Should infer f32 for float constant')
        assert(result.plcasm.includes('[inferred]'), 'Should be marked as inferred')
    })

    test('All existing explicit-type declarations still work', () => {
        const result = compileSuccess(`
let a: i32 @ M0 = 10
let b: u8 @ M4 = 20
let c: f32 @ M8 = 3.14
let d: bool @ M20.0 = true
let e: i16 @ MW12 = 100
`)
        assert(result.error === undefined || result.error === null, 'Should compile without errors')
        assert(!result.plcasm.includes('[inferred]'), 'None should be inferred (all explicit)')
    })

    // === lintPLCScript API and Symbol Table Export ===

    test('lintPLCScript returns problems and symbols', () => {
        const result = plc.lintPLCScript(`
let x: i32 @ auto = 42
let y @ MW0 = 100
let z = true
`)
        assert(result !== null && result !== undefined, 'Result should not be null')
        assert(Array.isArray(result.problems), 'Should have problems array')
        assert(Array.isArray(result.symbols), 'Should have symbols array')
        assert(typeof result.output === 'string', 'Should have output string')
    })

    test('Symbol table contains declared variables', () => {
        const result = plc.lintPLCScript(`
let a: i32 @ auto = 10
let b @ MW0 = 20
let c = true
`)
        const names = result.symbols.map(s => s.name)
        assert(names.includes('a'), 'Should contain symbol a')
        assert(names.includes('b'), 'Should contain symbol b')
        assert(names.includes('c'), 'Should contain symbol c')
    })

    test('Symbol table shows correct types', () => {
        const result = plc.lintPLCScript(`
let a: i32 @ auto = 10
let b @ MW0 = 20
let c = true
`)
        const findSym = name => result.symbols.find(s => s.name === name)
        assert(findSym('a').type === 'i32', 'a should be i32 (explicit)')
        assert(findSym('b').type === 'i16', 'b should be i16 (inferred from MW)')
        assert(findSym('c').type === 'bool', 'c should be bool (inferred from literal)')
    })

    test('Symbol table shows inferred flag', () => {
        const result = plc.lintPLCScript(`
let a: i32 @ auto = 10
let b @ MW0 = 20
let c = true
`)
        const findSym = name => result.symbols.find(s => s.name === name)
        assert(findSym('a').isInferred === false, 'a should NOT be inferred (explicit type)')
        assert(findSym('b').isInferred === true, 'b should be inferred (from address)')
        assert(findSym('c').isInferred === true, 'c should be inferred (from literal)')
    })

    test('Symbol table shows address and local info', () => {
        const result = plc.lintPLCScript(`
let a: i32 @ MD0 = 10
let b = 42
`)
        const findSym = name => result.symbols.find(s => s.name === name)
        // MD0 is stored as byte address M0 (dword at byte 0 in marker area)
        assert(findSym('a').address === 'M0', 'a should have address M0 (byte address for MD0)')
        assert(findSym('a').isLocal === false, 'a should not be local (has explicit address)')
        assert(findSym('b').isLocal === true, 'b should be local (no address)')
    })

    test('Lint error detection with symbol table', () => {
        const result = plc.lintPLCScript(`
let x: i32 @ M0 = 10
let y = unknownVar
`)
        assert(result.problems.length > 0, 'Should have errors for undefined variable')
        const xSym = result.symbols.find(s => s.name === 'x')
        assert(xSym !== undefined, 'Should still have symbol x from before the error')
    })

    return {
        name: 'PLCScript Type Inference',
        passed,
        failed,
        total: passed + failed,
        tests: testResults,
        failures,
    }
}

// Standalone execution
const isMainModule = process.argv[1] && (
    process.argv[1].includes('test_plcscript_type_inference') ||
    fileURLToPath(import.meta.url) === process.argv[1]
)

if (isMainModule) {
    const RED = '\x1b[31m'
    const GREEN = '\x1b[32m'
    const RESET = '\x1b[0m'

    console.log('PLCScript Type Inference Tests')
    console.log('══════════════════════════════')

    const result = await runTests()

    for (const t of result.tests) {
        console.log(`  ${t.passed ? GREEN + '✓' : RED + '✗'} ${t.name}${RESET}`)
        if (!t.passed && t.error) console.log(`    ${RED}${t.error}${RESET}`)
    }

    console.log()
    console.log(`${result.passed} passed, ${result.failed} failed`)

    if (result.failed > 0) process.exit(1)
}
