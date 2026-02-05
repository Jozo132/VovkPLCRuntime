#!/usr/bin/env node
/**
 * test_reserved_words.js - Tests for reserved word validation in symbol/type names
 * 
 * Tests that symbol names and type names cannot be:
 * 1. Type names (bool, i32, timer, etc.)
 * 2. Instruction names (LD, ST, ADD, etc.)
 * 3. Language keywords (if, while, for, etc.)
 */

import VovkPLC from '../../dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')

/**
 * @typedef {Object} TestResult
 * @property {string} name
 * @property {boolean} passed
 * @property {string} [error]
 */

/**
 * @typedef {Object} SuiteResult
 * @property {number} passed
 * @property {number} failed
 * @property {number} total
 * @property {TestResult[]} tests
 * @property {Object[]} failures
 */

/**
 * Run all reserved word tests
 * @param {Object} [options]
 * @param {boolean} [options.silent]
 * @param {VovkPLC} [options.runtime] - Shared runtime instance (creates one if not provided)
 * @returns {Promise<SuiteResult>}
 */
export async function runTests(options = {}) {
    const { silent = false, runtime: sharedRuntime = null } = options
    const log = silent ? () => {} : console.log.bind(console)
    
    // Use shared runtime if provided, otherwise create one (for standalone execution)
    let plc = sharedRuntime
    if (!plc) {
        plc = new VovkPLC()
        plc.stdout_callback = () => {}
        await plc.initialize(wasmPath, false, true)
    }
    
    let passed = 0, failed = 0
    /** @type {TestResult[]} */
    const testResults = []
    const failures = []
    
    // Project template for symbol tests
    const symbolProject = (name, type = 'i32', addr = 'M0') => `
VOVKPLCPROJECT ReservedWordTest
VERSION 1.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    M 256
    T 16
    C 8
END_MEMORY

FLASH
    SIZE 32768
END_FLASH

SYMBOLS
    ${name} : ${type} : ${addr}
END_SYMBOLS

PROGRAM main
    BLOCK LANG=PLCASM Main
nop
    END_BLOCK
END_PROGRAM
`
    
    // Project template for TYPES section
    const typeProject = (typeName) => `
VOVKPLCPROJECT TypesTest
VERSION 1.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    M 256
    T 16
    C 8
END_MEMORY

FLASH
    SIZE 32768
END_FLASH

TYPES
    ${typeName} = struct { value: i32; };
END_TYPES

SYMBOLS
END_SYMBOLS

PROGRAM main
    BLOCK LANG=PLCASM Main
nop
    END_BLOCK
END_PROGRAM
`
    
    // Project template for PLCScript variable
    const plcscriptProject = (varName) => `
VOVKPLCPROJECT PLCScriptTest
VERSION 1.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    M 256
    T 16
    C 8
END_MEMORY

FLASH
    SIZE 32768
END_FLASH

SYMBOLS
END_SYMBOLS

PROGRAM main
    BLOCK LANG=PLCSCRIPT Main
let ${varName}: i32 @ M0;
    END_BLOCK
END_PROGRAM
`
    
    // Project template for ST variable  
    const stProject = (varName) => `
VOVKPLCPROJECT STTest
VERSION 1.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    M 256
    T 16
    C 8
END_MEMORY

FLASH
    SIZE 32768
END_FLASH

SYMBOLS
END_SYMBOLS

PROGRAM main
    BLOCK LANG=ST Main
VAR
    ${varName} : INT;
END_VAR
    END_BLOCK
END_PROGRAM
`
    
    // Test runner
    function test(name, project, shouldFail, expectedError = null) {
        const result = plc.compileProject(project)
        const hasError = result.problem?.type === 'error'
        const error = result.problem?.message || ''
        
        let ok = true, errors = []
        
        if (shouldFail) {
            if (!hasError) {
                errors.push('Expected failure but succeeded')
                ok = false
            } else if (expectedError && !error.toLowerCase().includes(expectedError.toLowerCase())) {
                errors.push(`Expected error "${expectedError}", got: "${error}"`)
                ok = false
            }
        } else if (hasError) {
            errors.push(`Unexpected error: ${error}`)
            ok = false
        }
        
        if (ok) {
            passed++
            testResults.push({ name, passed: true })
        } else {
            failed++
            testResults.push({ name, passed: false, error: errors[0] })
            failures.push({ name, errors })
        }
    }
    
    // ========== SYMBOL RESERVED WORD TESTS ==========
    
    // Type names as symbols (excluding timer/counter which are allowed as variable names)
    for (const t of ['bool', 'i32', 'f64', 'byte', 'int', 'real', 'ton', 'ctu']) {
        test(`Symbol '${t}' (type) rejected`, symbolProject(t), true, 'reserved')
    }
    
    // Timer/counter are allowed as variable names (common usage)
    for (const t of ['timer', 'counter', 'Timer', 'Counter']) {
        test(`Symbol '${t}' allowed (common name)`, symbolProject(t), false)
    }
    
    // Instructions as symbols (multi-char only)
    for (const i of ['LD', 'ST', 'ADD', 'SUB', 'AND', 'OR', 'JMP', 'RET']) {
        test(`Symbol '${i}' (instruction) rejected`, symbolProject(i), true, 'reserved')
        test(`Symbol '${i.toLowerCase()}' rejected`, symbolProject(i.toLowerCase()), true, 'reserved')
    }
    
    // Single-letter variables are allowed (x, y, a, etc.)
    for (const v of ['x', 'y', 'z', 'a', 'i', 'j', 'k', 'n']) {
        test(`Symbol '${v}' (single letter) allowed`, symbolProject(v), false)
    }
    
    // Keywords as symbols
    for (const k of ['if', 'else', 'while', 'for', 'let', 'const', 'true', 'false', 'return']) {
        test(`Symbol '${k}' (keyword) rejected`, symbolProject(k), true, 'reserved')
    }
    
    // Project keywords as symbols
    for (const k of ['MEMORY', 'FLASH', 'SYMBOLS', 'FILE', 'auto']) {
        test(`Symbol '${k}' (project kw) rejected`, symbolProject(k), true, 'reserved')
    }
    
    // ========== TYPES SECTION RESERVED WORD TESTS ==========
    
    // Can't use type names for custom types
    for (const t of ['bool', 'int', 'ton', 'ctu']) {
        test(`Type name '${t}' rejected`, typeProject(t), true, 'reserved')
    }
    
    // Timer/counter are allowed as custom type names
    for (const t of ['Timer', 'Counter', 'MyTimer', 'MyCounter']) {
        test(`Type name '${t}' allowed`, typeProject(t), false)
    }
    
    // ========== PLCSCRIPT KEYWORD TESTS ==========
    
    // PLCScript keywords as variables
    for (const k of ['let', 'const', 'if', 'else', 'while', 'for', 'function', 'return', 'type', 'struct']) {
        test(`PLCScript var '${k}' rejected`, plcscriptProject(k), true, null)
    }
    
    // ========== ST KEYWORD TESTS ==========
    
    // ST keywords as variables (PROGRAM is not caught since it's after the variable section)
    for (const k of ['VAR', 'IF', 'THEN', 'ELSE', 'FOR', 'WHILE']) {
        test(`ST var '${k}' rejected`, stProject(k), true, null)
    }
    
    // ========== VALID NAMES (should pass) ==========
    
    for (const n of ['motor1', 'sensor_temp', 'valve_A', 'data', 'value', 'myTimer', 'result']) {
        test(`Valid symbol '${n}'`, symbolProject(n), false)
    }
    
    // Valid custom type names
    test(`Valid type 'Motor'`, typeProject('Motor'), false)
    test(`Valid type 'SensorData'`, typeProject('SensorData'), false)
    
    // ========== CROSS-LANGUAGE TESTS ==========
    
    // Symbol with reserved name fails regardless of code language
    test(`Reserved 'ADD' fails with PLCASM`, `
VOVKPLCPROJECT CrossLangTest
VERSION 1.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    M 256
    T 16
    C 8
END_MEMORY

FLASH
    SIZE 32768
END_FLASH

SYMBOLS
    ADD : i32 : M0
END_SYMBOLS

PROGRAM main
    BLOCK LANG=PLCASM Main
nop
    END_BLOCK
END_PROGRAM
`, true, 'reserved')
    
    test(`Reserved 'WHILE' fails with STL`, `
VOVKPLCPROJECT CrossLangTest2
VERSION 1.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    M 256
    T 16
    C 8
END_MEMORY

FLASH
    SIZE 32768
END_FLASH

SYMBOLS
    WHILE : i32 : M0
END_SYMBOLS

PROGRAM main
    BLOCK LANG=STL Main
nop
    END_BLOCK
END_PROGRAM
`, true, 'reserved')
    
    // Case insensitivity
    test(`Case insensitive: 'Bool' rejected`, symbolProject('Bool'), true, 'reserved')
    test(`Case insensitive: 'ADD' = 'add'`, symbolProject('add'), true, 'reserved')
    
    // ========== VARIABLE/SYMBOL NAME VS CUSTOM TYPE NAME ==========
    
    // Symbol with same name as custom type in TYPES section
    test(`Symbol name conflicts with TYPES-defined type`, `
VOVKPLCPROJECT TypeConflictTest
VERSION 1.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    M 256
    T 16
    C 8
END_MEMORY

FLASH
    SIZE 32768
END_FLASH

TYPES
    Motor = struct { speed: i32; };
END_TYPES

SYMBOLS
    Motor : i32 : M0
END_SYMBOLS

PROGRAM main
    BLOCK LANG=PLCASM Main
nop
    END_BLOCK
END_PROGRAM
`, true, 'conflicts with a type')
    
    // PLCScript variable with same name as TYPES-defined type
    test(`PLCScript var name conflicts with TYPES type`, `
VOVKPLCPROJECT TypeConflictTest2
VERSION 1.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    M 256
    T 16
    C 8
END_MEMORY

FLASH
    SIZE 32768
END_FLASH

TYPES
    Sensor = struct { value: f32; };
END_TYPES

SYMBOLS
END_SYMBOLS

PROGRAM main
    BLOCK LANG=PLCSCRIPT Main
let Sensor: i32 @ M0;
    END_BLOCK
END_PROGRAM
`, true, 'conflicts with a type')
    
    // PLCScript variable with same name as PLCScript-defined struct
    test(`PLCScript var name conflicts with local struct`, `
VOVKPLCPROJECT TypeConflictTest3
VERSION 1.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    M 256
    T 16
    C 8
END_MEMORY

FLASH
    SIZE 32768
END_FLASH

SYMBOLS
END_SYMBOLS

PROGRAM main
    BLOCK LANG=PLCSCRIPT Main
type Pump = struct { running: bool; };
let Pump: i32 @ M0;
    END_BLOCK
END_PROGRAM
`, true, 'conflicts with a type')
    
    // ST variable with same name as TYPES-defined type
    test(`ST var name conflicts with TYPES type`, `
VOVKPLCPROJECT TypeConflictTest4
VERSION 1.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    M 256
    T 16
    C 8
END_MEMORY

FLASH
    SIZE 32768
END_FLASH

TYPES
    Valve = struct { open: bool; };
END_TYPES

SYMBOLS
END_SYMBOLS

PROGRAM main
    BLOCK LANG=ST Main
VAR
    Valve : INT;
END_VAR
    END_BLOCK
END_PROGRAM
`, true, 'conflicts with a type')
    
    // Valid: Variable of a custom type (not same name)
    test(`Variable of custom type (different name) is valid`, `
VOVKPLCPROJECT ValidTypeUsage
VERSION 1.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    M 256
    T 16
    C 8
END_MEMORY

FLASH
    SIZE 32768
END_FLASH

TYPES
    Motor = struct { speed: i32; };
END_TYPES

SYMBOLS
    myMotor : Motor : M0
END_SYMBOLS

PROGRAM main
    BLOCK LANG=PLCASM Main
nop
    END_BLOCK
END_PROGRAM
`, false)
    
    log(`\nReserved Word Tests: ${passed} passed, ${failed} failed`)
    
    return { 
        name: 'Reserved Word Validation',
        passed, 
        failed, 
        total: passed + failed, 
        tests: testResults, 
        failures 
    }
}

// Direct execution
const expectedUrl = `file:///${process.argv[1].replace(/\\/g, '/')}`
const isMain = process.argv[1] && import.meta.url.toLowerCase() === expectedUrl.toLowerCase()

if (isMain) {
    runTests().then(r => {
        console.log(`\nReserved Word Tests Summary: ${r.passed} passed, ${r.failed} failed`)
        if (r.failures.length) {
            console.log('\nFailures:')
            r.failures.forEach(f => {
                console.log(`  ${f.name}`)
                f.errors.forEach(e => console.log(`    - ${e}`))
            })
        }
        process.exit(r.failed ? 1 : 0)
    }).catch(err => {
        console.error('Error running tests:', err)
        process.exit(1)
    })
}
