#!/usr/bin/env node
// test_plcscript_features.js - Comprehensive PLCScript Feature Tests
//
// Tests various PLCScript language features:
//   - Nested math expressions
//   - For loops
//   - While loops
//   - If/else if/else
//   - Ternary operator
//   - Break/continue
//   - Increment/decrement operators
//   - Bitwise operations
//   - Direct PLC address assignment
//   - Optional semicolons
//
// Usage:
//   node wasm/node-test/plcscript-tests/test_plcscript_features.js
//
// @ts-check
'use strict'

import VovkPLC from '../../dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

// ANSI colors
const RED = '\x1b[31m'
const GREEN = '\x1b[32m'
const YELLOW = '\x1b[33m'
const RESET = '\x1b[0m'

console.log('╔══════════════════════════════════════════════════════════════════╗')
console.log('║          PLCScript Feature Unit Tests                            ║')
console.log('╚══════════════════════════════════════════════════════════════════╝')
console.log()

const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')
const plc = new VovkPLC(wasmPath)
plc.stdout_callback = () => {} // Suppress output
await plc.initialize(wasmPath, false, false)
plc.readStream() // Clear any startup output

let passed = 0
let failed = 0
let skipped = 0

/**
 * @param {string} name 
 * @param {() => void} fn 
 * @param {boolean} skip
 */
function test(name, fn, skip = false) {
    if (skip) {
        console.log(`${YELLOW}○ ${name} (skipped)${RESET}`)
        skipped++
        return
    }
    try {
        fn()
        console.log(`${GREEN}✓ ${name}${RESET}`)
        passed++
    } catch (e) {
        console.log(`${RED}✗ ${name}${RESET}`)
        console.log(`  ${RED}Error: ${e.message}${RESET}`)
        if (e.plcasm) {
            console.log(`  Generated PLCASM:`)
            console.log(e.plcasm.split('\n').map(l => `    ${l}`).join('\n'))
        }
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
 * Helper to stream code to runtime
 * @param {string} code 
 */
function streamCode(code) {
    plc.wasm_exports.streamClear()
    for (let i = 0; i < code.length; i++) {
        plc.wasm_exports.streamIn(code.charCodeAt(i))
    }
}

/**
 * Helper to read a null-terminated string from WASM memory
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
 * Helper to wrap PLCScript code in a project and compile it
 * @param {string} script 
 * @returns {{plcasm: string, bytecode: string, error: string | null}}
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
END_MEMORY

FILE main
    BLOCK LANG=PLCSCRIPT Main
${script}
    END_BLOCK
END_FILE
`
    const wasm = plc.wasm_exports
    
    // Reset and compile
    wasm.project_reset()
    streamCode(project)
    
    const success = wasm.project_compile(0) // 0 = no debug output
    
    if (!success) {
        // Get error message
        const errorMsg = getString(wasm.project_getError())
        return { plcasm: '', bytecode: '', error: errorMsg }
    }
    
    // Get combined PLCASM
    const plcasm = getString(wasm.project_getCombinedPLCASM())
    
    // Get bytecode
    const bytecodePtr = wasm.project_getBytecode()
    const bytecodeLen = wasm.project_getBytecodeLength()
    let bytecode = ''
    if (bytecodePtr && bytecodeLen > 0) {
        // @ts-ignore
        const memory = new Uint8Array(plc.wasm.exports.memory.buffer)
        for (let i = 0; i < bytecodeLen; i++) {
            bytecode += memory[bytecodePtr + i].toString(16).padStart(2, '0').toUpperCase()
        }
    }
    
    return { plcasm, bytecode, error: null }
}

/**
 * Helper to compile and assert no errors
 * @param {string} script 
 * @returns {{plcasm: string, bytecode: string}}
 */
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

// ============================================================================
// Test Category: Nested Math Expressions
// ============================================================================
console.log('\n--- Nested Math Expressions ---\n')

test('Simple arithmetic: a + b * c', async () => {
    const result = compileSuccess(`
let a: i32 @ M0 = 10
let b: i32 @ M4 = 20
let c: i32 @ M8 = 3
let result: i32 @ M12

result = a + b * c  // Should be 10 + (20 * 3) = 70
`)
    assert(result.plcasm.includes('.add'), 'Should generate add')
    assert(result.plcasm.includes('.mul'), 'Should generate mul')
})

test('Parenthesized expression: (a + b) * c', async () => {
    const result = compileSuccess(`
let a: i32 @ M0 = 10
let b: i32 @ M4 = 20
let c: i32 @ M8 = 3
let result: i32 @ M12

result = (a + b) * c  // Should be (10 + 20) * 3 = 90
`)
    assert(result.plcasm.includes('.add'), 'Should generate add')
    assert(result.plcasm.includes('.mul'), 'Should generate mul')
})

test('Deeply nested: ((a + b) * (c - d)) / e', async () => {
    const result = compileSuccess(`
let a: i32 @ M0 = 100
let b: i32 @ M4 = 50
let c: i32 @ M8 = 30
let d: i32 @ M12 = 10
let e: i32 @ M16 = 5
let result: i32 @ M20

result = ((a + b) * (c - d)) / e  // ((100+50) * (30-10)) / 5 = (150 * 20) / 5 = 600
`)
    assert(result.plcasm.includes('.add'), 'Should generate add')
    assert(result.plcasm.includes('.sub'), 'Should generate sub')
    assert(result.plcasm.includes('.mul'), 'Should generate mul')
    assert(result.plcasm.includes('.div'), 'Should generate div')
})

test('Modulo operation: a % b', async () => {
    const result = compileSuccess(`
let a: i32 @ M0 = 17
let b: i32 @ M4 = 5
let result: i32 @ M8

result = a % b  // 17 % 5 = 2
`)
    assert(result.plcasm.includes('.mod'), 'Should generate mod')
})

test('Unary minus: -a + b', async () => {
    const result = compileSuccess(`
let a: i32 @ M0 = 10
let b: i32 @ M4 = 30
let result: i32 @ M8

result = -a + b  // -10 + 30 = 20
`)
    assert(result.plcasm.includes('.neg'), 'Should generate negation')
})

test('Mixed float and int expressions', async () => {
    const result = compileSuccess(`
let x: f32 @ M0 = 3.14
let y: f32 @ M4 = 2.0
let result: f32 @ M8

result = x * y + 1.5
`)
    assert(result.plcasm.includes('f32.'), 'Should use f32 operations')
})

// ============================================================================
// Test Category: For Loops
// ============================================================================
console.log('\n--- For Loops ---\n')

test('Basic for loop', async () => {
    const result = compileSuccess(`
let sum: i32 @ M0 = 0
for (let i: i32 @ M4 = 0; i < 10; i++) {
    sum = sum + i
}
`)
    assert(result.plcasm.includes('__forcond_'), 'Should generate for condition label')
    assert(result.plcasm.includes('__endfor_'), 'Should generate endfor label')
    assert(result.plcasm.includes('jmp.if.not'), 'Should generate conditional jump')
})

test('For loop with decrement', async () => {
    const result = compileSuccess(`
let sum: i32 @ M0 = 0
for (let i: i32 @ M4 = 10; i > 0; i--) {
    sum = sum + i
}
`)
    assert(result.plcasm.includes('__forcond_'), 'Should generate for condition label')
})

test('Nested for loops', async () => {
    const result = compileSuccess(`
let total: i32 @ M0 = 0
for (let i: i32 @ M4 = 0; i < 3; i++) {
    for (let j: i32 @ M8 = 0; j < 3; j++) {
        total = total + 1
    }
}
`)
    // Should have two different for condition labels
    const forcondMatches = result.plcasm.match(/__forcond_/g)
    assert(forcondMatches && forcondMatches.length >= 2, 'Should have nested for loops')
})

test('For loop without braces', async () => {
    const result = compileSuccess(`
let sum: i32 @ M0 = 0
for (let i: i32 @ M4 = 0; i < 5; i++)
    sum = sum + i
`)
    assert(result.plcasm.includes('__forcond_'), 'Should compile without braces')
})

// ============================================================================
// Test Category: While Loops
// ============================================================================
console.log('\n--- While Loops ---\n')

test('Basic while loop', async () => {
    const result = compileSuccess(`
let count: i32 @ M0 = 0
let max: i32 @ M4 = 10
while (count < max) {
    count = count + 1
}
`)
    assert(result.plcasm.includes('__while_'), 'Should generate while label')
    assert(result.plcasm.includes('__endwhile_'), 'Should generate endwhile label')
})

test('While loop with complex condition', async () => {
    const result = compileSuccess(`
let x: i32 @ M0 = 0
let y: i32 @ M4 = 100
while (x < 50 && y > 0) {
    x = x + 1
    y = y - 2
}
`)
    assert(result.plcasm.includes('logic.and'), 'Should generate logical AND')
})

test('While loop without braces', async () => {
    const result = compileSuccess(`
let x: i32 @ M0 = 0
while (x < 10)
    x = x + 1
`)
    assert(result.plcasm.includes('__while_'), 'Should compile without braces')
})

test('Nested while loops', async () => {
    const result = compileSuccess(`
let i: i32 @ M0 = 0
let j: i32 @ M4 = 0
while (i < 3) {
    j = 0
    while (j < 3) {
        j = j + 1
    }
    i = i + 1
}
`)
    const whileMatches = result.plcasm.match(/__while_/g)
    assert(whileMatches && whileMatches.length >= 2, 'Should have nested while loops')
})

// ============================================================================
// Test Category: If/Else If/Else
// ============================================================================
console.log('\n--- If/Else If/Else ---\n')

test('Simple if statement', async () => {
    const result = compileSuccess(`
let x: i32 @ M0 = 10
let result: i32 @ M4 = 0
if (x > 5) {
    result = 1
}
`)
    assert(result.plcasm.includes('jmp.if.not'), 'Should generate conditional jump')
})

test('If-else statement', async () => {
    const result = compileSuccess(`
let x: i32 @ M0 = 3
let result: i32 @ M4 = 0
if (x > 5) {
    result = 1
} else {
    result = 2
}
`)
    assert(result.plcasm.includes('__else_'), 'Should generate else label')
    assert(result.plcasm.includes('__endif_'), 'Should generate endif label')
})

test('If-else if-else chain', async () => {
    const result = compileSuccess(`
let x: i32 @ M0 = 50
let result: i32 @ M4 = 0

if (x < 25) {
    result = 1
} else if (x < 50) {
    result = 2
} else if (x < 75) {
    result = 3
} else {
    result = 4
}
`)
    // Should have multiple condition checks
    const jmpMatches = result.plcasm.match(/jmp\.if\.not/g)
    assert(jmpMatches && jmpMatches.length >= 3, 'Should have multiple conditional jumps')
})

test('If without braces', async () => {
    const result = compileSuccess(`
let x: i32 @ M0 = 10
let result: i32 @ M4 = 0
if (x > 5)
    result = 1
`)
    assert(result.plcasm.includes('jmp.if.not'), 'Should compile without braces')
})

test('If-else without braces', async () => {
    const result = compileSuccess(`
let x: i32 @ M0 = 3
let result: i32 @ M4 = 0
if (x > 5)
    result = 1
else
    result = 2
`)
    assert(result.plcasm.includes('__else_'), 'Should compile without braces')
})

test('Nested if statements', async () => {
    const result = compileSuccess(`
let a: i32 @ M0 = 10
let b: i32 @ M4 = 20
let result: i32 @ M8 = 0

if (a > 5) {
    if (b > 15) {
        result = 1
    } else {
        result = 2
    }
} else {
    result = 3
}
`)
    const elseMatches = result.plcasm.match(/__else_/g)
    assert(elseMatches && elseMatches.length >= 2, 'Should have nested else labels')
})

// ============================================================================
// Test Category: Ternary Operator
// ============================================================================
console.log('\n--- Ternary Operator ---\n')

test('Simple ternary', async () => {
    const result = compileSuccess(`
let x: i32 @ M0 = 10
let result: i32 @ M4 = x > 5 ? 100 : 200
`)
    assert(result.plcasm.includes('__ternary_false_'), 'Should generate ternary false label')
    assert(result.plcasm.includes('__ternary_end_'), 'Should generate ternary end label')
})

test('Ternary in expression', async () => {
    const result = compileSuccess(`
let a: i32 @ M0 = 10
let b: i32 @ M4 = 20
let result: i32 @ M8 = (a > b ? a : b) + 5
`)
    assert(result.plcasm.includes('__ternary_'), 'Should use ternary operator')
})

test('Nested ternary', async () => {
    const result = compileSuccess(`
let x: i32 @ M0 = 50
let result: i32 @ M4 = x < 25 ? 1 : x < 75 ? 2 : 3
`)
    const ternaryMatches = result.plcasm.match(/__ternary_false_/g)
    assert(ternaryMatches && ternaryMatches.length >= 2, 'Should have nested ternaries')
})

test('Ternary with bool result', async () => {
    const result = compileSuccess(`
let a: bool @ M0.0 = true
let b: bool @ M0.1 = false
let result: bool @ M0.2 = a ? true : b
`)
    assert(result.plcasm.includes('__ternary_'), 'Should handle bool ternary')
})

// ============================================================================
// Test Category: Break and Continue
// ============================================================================
console.log('\n--- Break and Continue ---\n')

test('Break in while loop', async () => {
    const result = compileSuccess(`
let i: i32 @ M0 = 0
while (i < 100) {
    if (i > 10)
        break
    i = i + 1
}
`)
    // Break should jump to endwhile
    assert(result.plcasm.includes('__endwhile_'), 'Should have endwhile label for break')
})

test('Continue in while loop', async () => {
    const result = compileSuccess(`
let i: i32 @ M0 = 0
let sum: i32 @ M4 = 0
while (i < 10) {
    i = i + 1
    if (i == 5)
        continue
    sum = sum + i
}
`)
    // Continue should jump back to while start
    assert(result.plcasm.includes('__while_'), 'Should have while label for continue')
})

test('Break in for loop', async () => {
    const result = compileSuccess(`
let sum: i32 @ M0 = 0
for (let i: i32 @ M4 = 0; i < 100; i++) {
    if (i > 5)
        break
    sum = sum + i
}
`)
    assert(result.plcasm.includes('__endfor_'), 'Should have endfor label for break')
})

test('Continue in for loop', async () => {
    const result = compileSuccess(`
let sum: i32 @ M0 = 0
for (let i: i32 @ M4 = 0; i < 10; i++) {
    if (i == 3)
        continue
    sum = sum + i
}
`)
    assert(result.plcasm.includes('__forupd_'), 'Should have update label for continue')
})

// ============================================================================
// Test Category: Increment/Decrement Operators
// ============================================================================
console.log('\n--- Increment/Decrement Operators ---\n')

test('Post-increment: x++', async () => {
    const result = compileSuccess(`
let x: i32 @ M0 = 5
let y: i32 @ M4 = x++
`)
    assert(result.plcasm.includes('.add'), 'Should generate add for increment')
})

test('Post-decrement: x--', async () => {
    const result = compileSuccess(`
let x: i32 @ M0 = 5
let y: i32 @ M4 = x--
`)
    assert(result.plcasm.includes('.sub'), 'Should generate sub for decrement')
})

test('Pre-increment: ++x', async () => {
    const result = compileSuccess(`
let x: i32 @ M0 = 5
let y: i32 @ M4 = ++x
`)
    assert(result.plcasm.includes('.add'), 'Should generate add for pre-increment')
})

test('Pre-decrement: --x', async () => {
    const result = compileSuccess(`
let x: i32 @ M0 = 5
let y: i32 @ M4 = --x
`)
    assert(result.plcasm.includes('.sub'), 'Should generate sub for pre-decrement')
})

test('Compound assignment: +=', async () => {
    const result = compileSuccess(`
let x: i32 @ M0 = 10
x += 5
`)
    assert(result.plcasm.includes('.add'), 'Should generate add for +=')
})

test('Compound assignment: -=, *=, /=', async () => {
    const result = compileSuccess(`
let a: i32 @ M0 = 100
let b: i32 @ M4 = 10
let c: i32 @ M8 = 5
a -= 10
b *= 2
c /= 2
`)
    assert(result.plcasm.includes('.sub'), 'Should generate sub')
    assert(result.plcasm.includes('.mul'), 'Should generate mul')
    assert(result.plcasm.includes('.div'), 'Should generate div')
})

// ============================================================================
// Test Category: Bitwise Operations
// ============================================================================
console.log('\n--- Bitwise Operations ---\n')

test('Bitwise AND: a & b', async () => {
    const result = compileSuccess(`
let a: u32 @ M0 = 0xFF00
let b: u32 @ M4 = 0x0F0F
let result: u32 @ M8 = a & b
`)
    assert(result.plcasm.includes('bw.and'), 'Should generate bitwise AND')
})

test('Bitwise OR: a | b', async () => {
    const result = compileSuccess(`
let a: u32 @ M0 = 0xFF00
let b: u32 @ M4 = 0x00FF
let result: u32 @ M8 = a | b
`)
    assert(result.plcasm.includes('bw.or'), 'Should generate bitwise OR')
})

test('Bitwise XOR: a ^ b', async () => {
    const result = compileSuccess(`
let a: u32 @ M0 = 0xAAAA
let b: u32 @ M4 = 0x5555
let result: u32 @ M8 = a ^ b
`)
    assert(result.plcasm.includes('bw.xor'), 'Should generate bitwise XOR')
})

test('Bitwise NOT: ~a', async () => {
    const result = compileSuccess(`
let a: u32 @ M0 = 0x0000FFFF
let result: u32 @ M4 = ~a
`)
    assert(result.plcasm.includes('bw.not'), 'Should generate bitwise NOT')
})

test('Left shift: a << b', async () => {
    const result = compileSuccess(`
let a: u32 @ M0 = 1
let result: u32 @ M4 = a << 4
`)
    assert(result.plcasm.includes('bw.lshift'), 'Should generate left shift')
})

test('Right shift: a >> b', async () => {
    const result = compileSuccess(`
let a: u32 @ M0 = 256
let result: u32 @ M4 = a >> 4
`)
    assert(result.plcasm.includes('bw.rshift'), 'Should generate right shift')
})

// ============================================================================
// Test Category: Direct PLC Address Assignment
// ============================================================================
console.log('\n--- Direct PLC Address Assignment ---\n')

test('Direct output bit assignment', async () => {
    const result = compileSuccess(`
let input: bool @ X0.0
Y0.0 = input
`)
    assert(result.plcasm.includes('u8.writeBit Y0.0'), 'Should write directly to Y0.0')
})

test('Direct output with expression', async () => {
    const result = compileSuccess(`
Y0.0 = X0.0 && X0.1
`)
    assert(result.plcasm.includes('u8.readBit X0.0'), 'Should read X0.0')
    assert(result.plcasm.includes('u8.readBit X0.1'), 'Should read X0.1')
    assert(result.plcasm.includes('u8.writeBit Y0.0'), 'Should write to Y0.0')
})

test('Direct marker assignment', async () => {
    const result = compileSuccess(`
let value: i16 @ M10 = 100
M20 = M10
`)
    // Note: M20 without explicit type should still work
    assert(!result.error, 'Should compile successfully')
})

// ============================================================================
// Test Category: Optional Semicolons
// ============================================================================
console.log('\n--- Optional Semicolons ---\n')

test('No semicolons at all', async () => {
    const result = compileSuccess(`
let a: i32 @ M0 = 10
let b: i32 @ M4 = 20
let c: i32 @ M8 = a + b
if (c > 25)
    c = c * 2
`)
    assert(!result.error, 'Should compile without semicolons')
})

test('Mixed semicolons', async () => {
    const result = compileSuccess(`
let a: i32 @ M0 = 10;
let b: i32 @ M4 = 20
let c: i32 @ M8 = a + b;
`)
    assert(!result.error, 'Should compile with mixed semicolons')
})

test('Semicolons inside for loop (required)', async () => {
    const result = compileSuccess(`
let sum: i32 @ M0 = 0
for (let i: i32 @ M4 = 0; i < 5; i++)
    sum = sum + i
`)
    assert(!result.error, 'For loop semicolons should work')
})

// ============================================================================
// Test Category: Logical Operators
// ============================================================================
console.log('\n--- Logical Operators ---\n')

test('Logical AND: a && b', async () => {
    const result = compileSuccess(`
let a: bool @ M0.0 = true
let b: bool @ M0.1 = true
let result: bool @ M0.2 = a && b
`)
    assert(result.plcasm.includes('logic.and'), 'Should generate logical AND')
})

test('Logical OR: a || b', async () => {
    const result = compileSuccess(`
let a: bool @ M0.0 = true
let b: bool @ M0.1 = false
let result: bool @ M0.2 = a || b
`)
    assert(result.plcasm.includes('logic.or'), 'Should generate logical OR')
})

test('Logical NOT: !a', async () => {
    const result = compileSuccess(`
let a: bool @ M0.0 = true
let result: bool @ M0.1 = !a
`)
    assert(result.plcasm.includes('logic.not'), 'Should generate logical NOT')
})

test('Complex logical expression', async () => {
    const result = compileSuccess(`
let a: bool @ M0.0 = true
let b: bool @ M0.1 = false
let c: bool @ M0.2 = true
let result: bool @ M0.3 = (a && b) || (!a && c)
`)
    assert(result.plcasm.includes('logic.and'), 'Should have AND')
    assert(result.plcasm.includes('logic.or'), 'Should have OR')
    assert(result.plcasm.includes('logic.not'), 'Should have NOT')
})

// ============================================================================
// Test Category: Comparison Operators
// ============================================================================
console.log('\n--- Comparison Operators ---\n')

test('All comparison operators', async () => {
    const result = compileSuccess(`
let a: i32 @ M0 = 50
let b: i32 @ M4 = 50

let eq: bool @ M8.0 = a == b
let neq: bool @ M8.1 = a != b
let lt: bool @ M8.2 = a < b
let gt: bool @ M8.3 = a > b
let lte: bool @ M8.4 = a <= b
let gte: bool @ M8.5 = a >= b
`)
    assert(result.plcasm.includes('.eq'), 'Should have equality')
    assert(result.plcasm.includes('.neq'), 'Should have not-equal')
    assert(result.plcasm.includes('.lt'), 'Should have less-than')
    assert(result.plcasm.includes('.gt'), 'Should have greater-than')
    assert(result.plcasm.includes('.lte'), 'Should have less-than-or-equal')
    assert(result.plcasm.includes('.gte'), 'Should have greater-than-or-equal')
})

// ============================================================================
// Test Category: Comments
// ============================================================================
console.log('\n--- Comments ---\n')

test('Single-line comments', async () => {
    const result = compileSuccess(`
// This is a comment
let x: i32 @ M0 = 10  // inline comment
// Another comment
let y: i32 @ M4 = 20
`)
    assert(!result.error, 'Should handle single-line comments')
})

test('Multi-line comments', async () => {
    const result = compileSuccess(`
/* Multi-line
   comment
   here */
let x: i32 @ M0 = 10
let y: i32 @ M4 = /* inline */ 20
`)
    assert(!result.error, 'Should handle multi-line comments')
})

// ============================================================================
// Test Category: Built-in Functions
// ============================================================================
console.log('\n--- Built-in Functions ---\n')

test('TON timer', async () => {
    const result = compileSuccess(`
let input: bool @ X0.0
let timerDone: bool @ M0.0

timerDone = TON(T0, input, 1000)
`)
    assert(result.plcasm.includes('ton.const T0'), 'Should generate TON instruction')
})

test('Rising edge detection', async () => {
    const result = compileSuccess(`
let input: bool @ X0.0
let pulse: bool @ M0.0

pulse = risingEdge(input, M1.0)
`)
    assert(result.plcasm.includes('stack.du'), 'Should generate edge detection')
})

test('Falling edge detection', async () => {
    const result = compileSuccess(`
let input: bool @ X0.0
let pulse: bool @ M0.0

pulse = fallingEdge(input, M1.0)
`)
    assert(result.plcasm.includes('stack.dd'), 'Should generate falling edge detection')
})

// ============================================================================
// Test Category: Type Conversions
// ============================================================================
console.log('\n--- Type Conversions ---\n')

test('Integer to float assignment', async () => {
    const result = compileSuccess(`
let intVal: i32 @ M0 = 100
let floatVal: f32 @ M4 = intVal
`)
    assert(result.plcasm.includes('cvt'), 'Should generate type conversion')
})

// ============================================================================
// Test Category: Edge Cases
// ============================================================================
console.log('\n--- Edge Cases ---\n')

test('Empty if block', async () => {
    const result = compileSuccess(`
let x: i32 @ M0 = 10
if (x > 5) {
}
`)
    assert(!result.error, 'Should handle empty if block')
})

test('Multiple statements on conceptual "line"', async () => {
    const result = compileSuccess(`
let a: i32 @ M0 = 1
let b: i32 @ M4 = 2
let c: i32 @ M8 = 3
a = b; b = c; c = a
`)
    assert(!result.error, 'Should handle multiple statements')
})

test('Hex number literals', async () => {
    const result = compileSuccess(`
let a: u32 @ M0 = 0xFF
let b: u32 @ M4 = 0xABCD
let c: u32 @ M8 = 0x12345678
`)
    assert(!result.error, 'Should parse hex literals')
})

test('Negative number literals', async () => {
    const result = compileSuccess(`
let a: i32 @ M0 = -100
let b: i32 @ M4 = -1
`)
    assert(!result.error, 'Should parse negative literals')
})

test('Float literals with exponent', async () => {
    const result = compileSuccess(`
let a: f32 @ M0 = 1.5e10
let b: f32 @ M4 = 3.14e-5
`)
    assert(!result.error, 'Should parse float exponent notation')
})

// ============================================================================
// Test Category: Not Yet Implemented (Skipped)
// ============================================================================
console.log('\n--- Not Yet Implemented (Skipped) ---\n')

test('User-defined functions', () => {
    // Functions are not yet implemented
}, true)

test('Switch/case statement', () => {
    // Switch/case is not yet implemented
}, true)

test('Arrays', () => {
    // Arrays are not yet implemented
}, true)

// ============================================================================
// Summary
// ============================================================================
console.log()
console.log('════════════════════════════════════════════════════════════════════')
console.log()
console.log(`Results: ${GREEN}${passed} passed${RESET}, ${failed > 0 ? RED : ''}${failed} failed${RESET}, ${YELLOW}${skipped} skipped${RESET}`)
console.log()

process.exit(failed > 0 ? 1 : 0)
