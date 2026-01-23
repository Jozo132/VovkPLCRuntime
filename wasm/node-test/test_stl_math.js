// Test STL typed math operations
import VovkPLC from '../dist/VovkPLC.js'

const plc = new VovkPLC()
await plc.initialize('../dist/VovkPLC.wasm', false, true)

let passed = 0
let failed = 0

function test(name, stl, expected) {
    const result = plc.compileSTL(stl)
    if (result.error) {
        console.log(`❌ ${name}: Compile error: ${result.error}`)
        failed++
        return
    }
    
    // Normalize whitespace for comparison
    const normalizedOutput = result.output.trim().split('\n').map(l => l.trim()).filter(l => l && !l.startsWith('//')).join('\n')
    const normalizedExpected = expected.trim().split('\n').map(l => l.trim()).filter(l => l && !l.startsWith('//')).join('\n')
    
    if (normalizedOutput === normalizedExpected) {
        console.log(`✅ ${name}`)
        passed++
    } else {
        console.log(`❌ ${name}`)
        console.log(`  Expected:\n    ${normalizedExpected.replace(/\n/g, '\n    ')}`)
        console.log(`  Got:\n    ${normalizedOutput.replace(/\n/g, '\n    ')}`)
        failed++
    }
}

console.log('\n=== STL Typed Math Operations Tests ===\n')

// Siemens STL 16-bit integer operations (+I, -I, *I, /I)
console.log('--- Siemens STL 16-bit Integer Operations ---')
test('Addition +I', '+I', 'i16.add')
test('Subtraction -I', '-I', 'i16.sub')
test('Multiplication *I', '*I', 'i16.mul')
test('Division /I', '/I', 'i16.div')

// Siemens STL 32-bit integer operations (+D, -D, *D, /D)
console.log('\n--- Siemens STL 32-bit Integer Operations ---')
test('Addition +D', '+D', 'i32.add')
test('Subtraction -D', '-D', 'i32.sub')
test('Multiplication *D', '*D', 'i32.mul')
test('Division /D', '/D', 'i32.div')

// Siemens STL Real/Float operations (+R, -R, *R, /R)
console.log('\n--- Siemens STL Float Operations ---')
test('Addition +R', '+R', 'f32.add')
test('Subtraction -R', '-R', 'f32.sub')
test('Multiplication *R', '*R', 'f32.mul')
test('Division /R', '/R', 'f32.div')

// Extended typed ADD operations
console.log('\n--- Extended Typed ADD Operations ---')
test('ADD_U8', 'ADD_U8', 'u8.add')
test('ADD_U16', 'ADD_U16', 'u16.add')
test('ADD_U32', 'ADD_U32', 'u32.add')
test('ADD_I16', 'ADD_I16', 'i16.add')
test('ADD_I32', 'ADD_I32', 'i32.add')
test('ADD_F32', 'ADD_F32', 'f32.add')
test('ADD_F64', 'ADD_F64', 'f64.add')

// Extended typed SUB operations
console.log('\n--- Extended Typed SUB Operations ---')
test('SUB_U8', 'SUB_U8', 'u8.sub')
test('SUB_I32', 'SUB_I32', 'i32.sub')
test('SUB_F32', 'SUB_F32', 'f32.sub')

// Extended typed MUL operations
console.log('\n--- Extended Typed MUL Operations ---')
test('MUL_U16', 'MUL_U16', 'u16.mul')
test('MUL_I64', 'MUL_I64', 'i64.mul')
test('MUL_F64', 'MUL_F64', 'f64.mul')

// Extended typed DIV operations
console.log('\n--- Extended Typed DIV Operations ---')
test('DIV_U32', 'DIV_U32', 'u32.div')
test('DIV_I16', 'DIV_I16', 'i16.div')
test('DIV_F32', 'DIV_F32', 'f32.div')

// MOD operations
console.log('\n--- MOD Operations ---')
test('MOD (default i16)', 'MOD', 'i16.mod')
test('MOD_U8', 'MOD_U8', 'u8.mod')
test('MOD_I32', 'MOD_I32', 'i32.mod')

// NEG operations
console.log('\n--- NEG Operations ---')
test('NEG (default i16)', 'NEG', 'i16.neg')
test('NEG_I8', 'NEG_I8', 'i8.neg')
test('NEG_I32', 'NEG_I32', 'i32.neg')
test('NEG_F32', 'NEG_F32', 'f32.neg')

// ABS operations
console.log('\n--- ABS Operations ---')
test('ABS (default i16)', 'ABS', 'i16.abs')
test('ABS_I8', 'ABS_I8', 'i8.abs')
test('ABS_I32', 'ABS_I32', 'i32.abs')
test('ABS_F64', 'ABS_F64', 'f64.abs')

// Comparison operations
console.log('\n--- Comparison Operations (16-bit) ---')
test('==I', '==I', 'i16.cmp_eq')
test('<>I', '<>I', 'i16.cmp_neq')
test('>I', '>I', 'i16.cmp_gt')
test('>=I', '>=I', 'i16.cmp_gte')
test('<I', '<I', 'i16.cmp_lt')
test('<=I', '<=I', 'i16.cmp_lte')

console.log('\n--- Comparison Operations (32-bit) ---')
test('==D', '==D', 'i32.cmp_eq')
test('<>D', '<>D', 'i32.cmp_neq')
test('>D', '>D', 'i32.cmp_gt')
test('>=D', '>=D', 'i32.cmp_gte')
test('<D', '<D', 'i32.cmp_lt')
test('<=D', '<=D', 'i32.cmp_lte')

console.log('\n--- Comparison Operations (Float) ---')
test('==R', '==R', 'f32.cmp_eq')
test('<>R', '<>R', 'f32.cmp_neq')
test('>R', '>R', 'f32.cmp_gt')
test('>=R', '>=R', 'f32.cmp_gte')
test('<R', '<R', 'f32.cmp_lt')
test('<=R', '<=R', 'f32.cmp_lte')

// Complex expression test - now with lookahead type inference
console.log('\n--- Complex Expression Tests ---')

// L values should get the type of the next math operation
const complexSTL = `
L #10
L #20
+I
L #5
*I
`
const complexExpected = `
i16.const 10
i16.const 20
i16.add
i16.const 5
i16.mul
`
test('Complex: L #10; L #20; +I; L #5; *I (i16 lookahead)', complexSTL, complexExpected)

// 32-bit test
const complex32STL = `
L #100
L #200
+D
`
const complex32Expected = `
i32.const 100
i32.const 200
i32.add
`
test('Complex: L #100; L #200; +D (i32 lookahead)', complex32STL, complex32Expected)

// Float test
const complexFloatSTL = `
L #3
L #4
*R
`
const complexFloatExpected = `
f32.const 3
f32.const 4
f32.mul
`
test('Complex: L #3; L #4; *R (f32 lookahead)', complexFloatSTL, complexFloatExpected)

// Extended type test
const extendedSTL = `
L #50
L #25
ADD_U32
`
const extendedExpected = `
u32.const 50
u32.const 25
u32.add
`
test('Complex: L #50; L #25; ADD_U32 (u32 lookahead)', extendedSTL, extendedExpected)

// Mixed types - first operation determines type for first load set
const mixedSTL = `
L #100
L #200
+D
NEG_I32
ABS_I32
`
const mixedExpected = `
i32.const 100
i32.const 200
i32.add
i32.neg
i32.abs
`
test('Mixed: +D then NEG_I32 then ABS_I32 (i32 lookahead)', mixedSTL, mixedExpected)

// No math op after load - type inferred from Transfer destination
const defaultSTL = `
L #42
T MW10
`
const defaultExpected = `
i16.const 42
i16.move_to M10
`
test('Transfer MW: L #42; T MW10 (i16 from MW)', defaultSTL, defaultExpected)

// Transfer to double word
const transferDSTL = `
L #1000
T MD20
`
const transferDExpected = `
i32.const 1000
i32.move_to M20
`
test('Transfer MD: L #1000; T MD20 (i32 from MD)', transferDSTL, transferDExpected)

// Transfer to byte (no type suffix)
const transferBSTL = `
L #255
T MB5
`
const transferBExpected = `
u8.const 255
u8.move_to M5
`
test('Transfer MB: L #255; T MB5 (u8 from MB)', transferBSTL, transferBExpected)

// Mixed type operations with multiple stages
console.log('\n--- Mixed Type Operations ---')

// This test shows CVT being emitted when types change
const mixedOpsSTL = `
L #10
L #20
+I
T MW0
L MW0
L #100
*I
T MD10
L MD10
L #2
/D
T MW20
`
const mixedOpsExpected = `
i16.const 10
i16.const 20
i16.add
i16.move_to M0
i16.load_from M0
i16.const 100
i16.mul
cvt i16 i32
i32.move_to M10
i32.load_from M10
i32.const 2
i32.div
cvt i32 i16
i16.move_to M20
`
test('Mixed ops: add→store→load→mul→store(cvt i16→i32)→load→div→store(cvt i32→i16)', mixedOpsSTL, mixedOpsExpected)

// Float operations chain
const floatChainSTL = `
L #3
L #4
+R
L #2
*R
T MR0
`
const floatChainExpected = `
f32.const 3
f32.const 4
f32.add
f32.const 2
f32.mul
f32.move_to M0
`
test('Float chain: L #3; L #4; +R; L #2; *R; T MR0', floatChainSTL, floatChainExpected)

// Reading from typed memory
const readTypedSTL = `
L MW10
L MW20
+I
T MW30
`
const readTypedExpected = `
i16.load_from M10
i16.load_from M20
i16.add
i16.move_to M30
`
test('Read typed: L MW10; L MW20; +I; T MW30', readTypedSTL, readTypedExpected)

console.log(`\n=== Results: ${passed} passed, ${failed} failed ===`)
process.exit(failed > 0 ? 1 : 0)
