// test_stl_compiler.js - Test Siemens STL to PLCASM transpiler
import VovkPLC from '../../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import {fileURLToPath} from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

let passed = 0
let failed = 0

const run = async () => {
    const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')
    console.log(`Loading WASM from: ${wasmPath}`)

    if (!fs.existsSync(wasmPath)) {
        console.error('WASM file not found!')
        process.exit(1)
    }

    const runtime = await VovkPLC.createWorker(wasmPath)
    
    try {
        console.log('Runtime initialized. Testing STL Compiler...\n')

        // Test 1: Simple bit logic
        console.log('=== TEST 1: Simple Bit Logic ===')
        const stl1 = `
// Simple AND-OR logic
A   I0.0
AN  I0.1
O   I0.2
=   Q0.0
`
        await testSTL(runtime, 'Simple Bit Logic', stl1, ['u8.readBit', 'u8.and', 'u8.or', 'u8.writeBit'])

        // Test 2: Set/Reset latch with edge detection
        console.log('\n=== TEST 2: Set/Reset Latch with Edge Detection ===')
        const stl2 = `
// Start/Stop latch
A   I0.0
FP  M0.0
S   M10.0

A   I0.1
R   M10.0

A   M10.0
=   Q0.0
`
        await testSTL(runtime, 'Set/Reset Latch', stl2, ['u8.writeBitOn', 'u8.writeBitOff'])

        // Test 3: Timers
        console.log('\n=== TEST 3: Timers (TON/TOF/TP) ===')
        const stl3 = `
// Timer examples
A   I0.0
TON T0, T#50ms
=   Q0.1

A   I0.1
TOF T1, #100
=   Q0.2

A   I0.2
TP  T2, #200
=   Q0.3
`
        await testSTL(runtime, 'Timers', stl3, ['ton', 'tof', 'tp'])

        // Test 4: Counters
        console.log('\n=== TEST 4: Counters (CTU/CTD) ===')
        const stl4 = `
// Counter examples
// CTU: Count pulses, reset with I0.1
A   I0.0
CTU C0, #10, I0.1
=   Q0.4

// CTD: Count down, load with I0.3
A   I0.2
CTD C1, #5, I0.3
=   Q0.5
`
        await testSTL(runtime, 'Counters', stl4, ['ctu', 'ctd'])

        // Test 5: Math and Compare
        console.log('\n=== TEST 5: Math and Compare ===')
        const stl5 = `
// Load, add, compare
L   M10
L   #10
+I
T   M10

L   M10
L   #100
>I
=   Q0.6
`
        await testSTL(runtime, 'Math and Compare', stl5, ['u8.add', 'u8.cmp_gt'])

        // Test 6: Jumps and Labels
        console.log('\n=== TEST 6: Jumps and Labels ===')
        const stl6 = `
// Conditional jump example
A   I0.0
JC  TrueLabel
JU  EndLabel

TrueLabel:
SET
=   Q0.7

EndLabel:
`
        await testSTL(runtime, 'Jumps and Labels', stl6, ['jmp_if', 'jmp', 'TrueLabel:', 'EndLabel:'])

        // Test 7: Subroutine Call
        console.log('\n=== TEST 7: Subroutine Call ===')
        const stl7 = `
// Main program
CALL MySub
BE

MySub:
A   I0.0
=   Q0.0
BEU
`
        await testSTL(runtime, 'Subroutine Call', stl7, ['call', 'ret', 'MySub:'])

        // Test 8: Nested Parentheses
        console.log('\n=== TEST 8: Nested Parentheses ===')
        const stl8 = `
// Complex nested logic
A(
O   I0.0
O   I0.1
)
A(
O   I0.2
O   I0.3
)
=   Q0.0
`
        await testSTL(runtime, 'Nested Parentheses', stl8, ['u8.and', 'u8.or'])

        // Test 9: IEC IL-style aliases (LD, ST, AND, OR)
        console.log('\n=== TEST 9: IEC IL Aliases ===')
        const stl9 = `
// IEC 61131-3 IL-style syntax
LD   I0.0
AND  I0.1
OR   I0.2
ST   Q0.0
`
        await testSTL(runtime, 'IEC IL Aliases', stl9, ['u8.readBit', 'u8.and', 'u8.or', 'u8.writeBit'])

        // Test 10: Full compile (STL -> PLCASM -> Bytecode) using compile() API
        console.log('\n=== TEST 10: Full Compile via compile() API ===')
        const stl10 = `
A   I0.0
AN  I0.1
=   Q0.0
`
        await testSTLFull(runtime, 'Full Compile', stl10)
        
        // Test 11: Test compile() method with language option
        console.log('\n=== TEST 11: compile() with language option ===')
        await testCompileAPI(runtime, 'compile() API with STL', stl10)

        // Test 12: Test complex STL with various operations
        console.log('\n=== TEST 12: Complex STL Program ===')
        const stl12 = `
        // NETWORK 1: Start/Stop latch with edge detect
CLR
A   X0.0
FP  M0.0
S   M10.0

A   X0.1
R   M10.0

A   M10.0
=   Y0.0

// NETWORK 2: Timers driven by latch
A   M10.0
TON T0, T#50ms
=   Y0.1

A   M10.0
TOF T1, #100
=   Y0.2

A   M10.0
TP  T2, #200
=   Y0.3

// NETWORK 3: Counter up on 1s pulse, reset by stop
A   M2.4
CTU C0, #10, X0.1
=   Y0.4

// NETWORK 4: Counter down on 1s pulse, load when start edge
A   M2.4
CTD C1, #5, M0.2
=   Y0.5

// NETWORK 5: Math + compare + jump
L   M20
L   #10
+I
T   M20

L   M20
L   #100
>I
JC  Over100

JU  EndMath

Over100:
A   M10.0
NOT
=   Y0.6

EndMath:

// NETWORK 6: Call subroutine if latch is on
A   M10.0
JC  DoCall
JU  End

DoCall:
CALL Sub1

End:
// Main program ends - jump over subroutine
JU  ProgramEnd

Sub1:
    // If input is false, return immediately (conditional return)
    JCN SubEnd

    // Toggle a bit
    A   Y0.7
    NOT
    =   Y0.7

SubEnd:
    BE

ProgramEnd:
// Program exit point
`
        await testSTLFull(runtime, 'Complex STL Program', stl12)

        // Summary
        console.log('\n========================================')
        console.log(`Tests passed: ${passed}`)
        console.log(`Tests failed: ${failed}`)
        console.log('========================================')

        process.exitCode = failed > 0 ? 1 : 0
    } catch (e) {
        console.error('Test error:', e)
        process.exitCode = 1
    } finally {
        await runtime.terminate()
        await new Promise(res => setTimeout(res, 200))
        process.exit(process.exitCode || 0)
    }
}

// Helper function: Write STL to stream and compile
async function testSTL(runtime, name, stlCode, expectedPatterns = []) {
    console.log(`\nInput STL:\n${stlCode}`)
    
    try {
        // Stream the STL code character by character
        for (let i = 0; i < stlCode.length; i++) {
            await runtime.callExport('streamIn', stlCode.charCodeAt(i))
        }
        // Add null terminator
        await runtime.callExport('streamIn', 0)
        
        // Load STL from stream buffer
        await runtime.callExport('stl_load_from_stream')
        
        // Compile STL to PLCASM
        const success = await runtime.callExport('stl_compile')
        
        if (!success) {
            // Check for error
            const hasError = await runtime.callExport('stl_has_error')
            if (hasError) {
                const errorLine = await runtime.callExport('stl_get_error_line')
                const errorCol = await runtime.callExport('stl_get_error_column')
                console.error(`Compilation error at line ${errorLine}, col ${errorCol}`)
                failed++
                return
            }
        }
        
        // Write output to stream so we can read it
        await runtime.callExport('stl_output_to_stream')
        
        // Read the output from stream
        const output = await runtime.readStream()
        
        console.log(`Output PLCASM:\n${output}`)
        
        // Check expected patterns
        let allFound = true
        for (const pattern of expectedPatterns) {
            if (!output.includes(pattern)) {
                console.log(`FAIL: Expected pattern "${pattern}" not found`)
                allFound = false
            }
        }
        
        if (allFound) {
            console.log(`[${name}] ✓ Transpiled successfully`)
            passed++
        } else {
            failed++
        }
    } catch (e) {
        console.error(`[${name}] ERROR:`, e.message)
        failed++
    }
}

// Helper function: Full compile STL -> PLCASM -> Bytecode -> Execute
async function testSTLFull(runtime, name, stlCode) {
    console.log(`\nInput STL:\n${stlCode}`)
    
    try {
        // Stream the STL code character by character
        for (let i = 0; i < stlCode.length; i++) {
            await runtime.callExport('streamIn', stlCode.charCodeAt(i))
        }
        // Add null terminator
        await runtime.callExport('streamIn', 0)
        
        // Load STL from stream buffer
        await runtime.callExport('stl_load_from_stream')
        
        // Full compile (STL -> PLCASM -> Bytecode)
        const hasError = await runtime.callExport('stl_compile_full')
        
        if (hasError) {
            const errorLine = await runtime.callExport('stl_get_error_line')
            const errorCol = await runtime.callExport('stl_get_error_column')
            console.error(`Full compilation error at line ${errorLine}, col ${errorCol}`)
            failed++
            return
        }
        
        // Write PLCASM output to stream
        await runtime.callExport('stl_output_to_stream')
        const plcasm = await runtime.readOutBuffer()
        console.log(`Generated PLCASM:\n${plcasm}`)
        
        // Load compiled program into runtime
        const loadError = await runtime.callExport('loadCompiledProgram')
        if (loadError) {
            console.error('Failed to load compiled program')
            failed++
            return
        }
        
        // Execute the program in debug mode
        console.log('Executing bytecode...')
        await runtime.callExport('runFullProgramDebug')
        
        // Read execution output
        const execOutput = await runtime.readStream()
        if (execOutput) {
            console.log(`Execution output:\n${execOutput}`)
        }
        
        // Check for runtime errors in output
        if (execOutput && (execOutput.includes('Error') || execOutput.includes('UNDERFLOW') || execOutput.includes('OVERFLOW'))) {
            console.error(`[${name}] FAIL: Runtime error detected in output`)
            failed++
            return
        }
        
        console.log(`[${name}] ✓ Full compile and execute successful`)
        passed++
    } catch (e) {
        console.error(`[${name}] ERROR:`, e.message)
        failed++
    }
}

// Helper function: Test compile() API with language option
async function testCompileAPI(runtime, name, stlCode) {
    console.log(`\nInput STL:\n${stlCode}`)
    
    try {
        // Use the compile() method with language option
        const result = await runtime.call('compile', stlCode, { language: 'stl', run: true })
        
        console.log(`Bytecode size: ${result.size} bytes`)
        if (result.plcasm) {
            console.log(`Generated PLCASM:\n${result.plcasm}`)
        }
        
        // Read execution output
        const execOutput = await runtime.readStream()
        if (execOutput) {
            console.log(`Execution output:\n${execOutput}`)
        }
        
        console.log(`[${name}] ✓ compile() API successful`)
        passed++
    } catch (e) {
        console.error(`[${name}] ERROR:`, e.message)
        failed++
    }
}

run()
