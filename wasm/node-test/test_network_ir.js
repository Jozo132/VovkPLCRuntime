// Test Network IR and STL round-trip conversion
import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')

// Helper to stream code to runtime
async function streamCode(runtime, code) {
    for (let i = 0; i < code.length; i++) {
        runtime.wasm_exports.streamIn(code.charCodeAt(i))
    }
}

async function runTests() {
    console.log('Loading WASM module via VovkPLC.js...')
    
    // Use direct instance mode (not worker) to access raw exports
    const runtime = new VovkPLC(wasmPath)
    await runtime.initialize(wasmPath, false, true) // silent mode
    
    // Get direct access to exports for NIR functions
    const exports = runtime.wasm_exports
    
    // Helper to get string from WASM memory
    const getString = (ptr) => {
        const mem = new Uint8Array(exports.memory.buffer)
        let str = ''
        while (mem[ptr] !== 0) {
            str += String.fromCharCode(mem[ptr++])
        }
        return str
    }
    
    try {
        console.log('\n=== Testing Network IR Functions ===\n')
        
        // Test 1: Check NIR exports exist
        console.log('Test 1: Verify NIR exports exist')
        const nirFunctions = [
            'nir_reset',
            'nir_get_symbol_count',
            'nir_get_expr_count',
            'nir_get_network_count',
            'nir_get_action_count',
            'nir_sizeof_symbol',
            'nir_sizeof_expr',
            'nir_sizeof_network',
        ]
        
        for (const fn of nirFunctions) {
            if (typeof exports[fn] === 'function') {
                console.log(`  ✓ ${fn} exists`)
            } else {
                console.log(`  ✗ ${fn} missing`)
            }
        }
        
        // Test 2: Reset and verify initial state
        console.log('\nTest 2: Reset and verify initial state')
        exports.nir_reset()
        console.log(`  Symbol count: ${exports.nir_get_symbol_count()}`)
        console.log(`  Expr count: ${exports.nir_get_expr_count()}`)
        console.log(`  Network count: ${exports.nir_get_network_count()}`)
        
        // Test 3: Check STL to IR parser exports
        console.log('\nTest 3: Verify STL<->IR exports')
        const stlIrFunctions = [
            'stl_to_ir_load_from_stream',
            'stl_to_ir_parse',
            'stl_to_ir_has_error',
            'stl_to_ir_get_error',
            'ir_to_stl_emit',
            'ir_to_stl_get_output',
            'ir_to_stl_get_output_length',
            'stl_roundtrip',
        ]
        
        for (const fn of stlIrFunctions) {
            if (typeof exports[fn] === 'function') {
                console.log(`  ✓ ${fn} exists`)
            } else {
                console.log(`  ✗ ${fn} missing`)
            }
        }
        
        // Test 4: Simple STL round-trip
        console.log('\nTest 4: Simple STL round-trip')
        const simpleSTL = `A X0.0
= Y0.0
`
        
        await streamCode(runtime, simpleSTL)
        exports.stl_to_ir_load_from_stream()
        
        const parseResult = exports.stl_to_ir_parse()
        console.log(`  Parse result: ${parseResult ? 'success' : 'failed'}`)
        
        if (exports.stl_to_ir_has_error()) {
            const errPtr = exports.stl_to_ir_get_error()
            console.log(`  Error: ${getString(errPtr)}`)
        } else {
            console.log(`  Symbol count: ${exports.nir_get_symbol_count()}`)
            console.log(`  Expr count: ${exports.nir_get_expr_count()}`)
            console.log(`  Action count: ${exports.nir_get_action_count()}`)
            console.log(`  Network count: ${exports.nir_get_network_count()}`)
            
            // Emit back to STL
            const emitResult = exports.ir_to_stl_emit()
            console.log(`  Emit result: ${emitResult ? 'success' : 'failed'}`)
            
            if (emitResult) {
                const outLen = exports.ir_to_stl_get_output_length()
                const outPtr = exports.ir_to_stl_get_output()
                const outStr = getString(outPtr)
                console.log(`  Output length: ${outLen}`)
                console.log(`  Output:\n${outStr}`)
            }
        }
        
        // Test 5: Complex STL with OR
        console.log('\nTest 5: Complex STL with OR branches')
        exports.nir_reset()
        
        const complexSTL = `A X0.0
O X0.1
= Y0.0
`
        
        await streamCode(runtime, complexSTL)
        exports.stl_to_ir_load_from_stream()
        
        if (exports.stl_to_ir_parse()) {
            console.log(`  Parse: success`)
            console.log(`  Symbols: ${exports.nir_get_symbol_count()}`)
            console.log(`  Expressions: ${exports.nir_get_expr_count()}`)
            console.log(`  Networks: ${exports.nir_get_network_count()}`)
            
            if (exports.ir_to_stl_emit()) {
                const outStr = getString(exports.ir_to_stl_get_output())
                console.log(`  Emitted:\n${outStr}`)
            }
        } else {
            console.log(`  Parse failed: ${getString(exports.stl_to_ir_get_error())}`)
        }
        
        // Test 6: STL with AND series
        console.log('\nTest 6: STL with AND series')
        exports.nir_reset()
        
        const andSTL = `A X0.0
A X0.1
A X0.2
= Y0.0
`
        
        await streamCode(runtime, andSTL)
        exports.stl_to_ir_load_from_stream()
        
        if (exports.stl_to_ir_parse()) {
            console.log(`  Parse: success`)
            console.log(`  Symbols: ${exports.nir_get_symbol_count()}`)
            console.log(`  Expressions: ${exports.nir_get_expr_count()}`)
            console.log(`  Networks: ${exports.nir_get_network_count()}`)
            
            if (exports.ir_to_stl_emit()) {
                const outStr = getString(exports.ir_to_stl_get_output())
                console.log(`  Emitted:\n${outStr}`)
            }
        } else {
            console.log(`  Parse failed: ${getString(exports.stl_to_ir_get_error())}`)
        }
        
        // Test 7: Control flow detection
        console.log('\nTest 7: Control flow detection (should fail)')
        exports.nir_reset()
        
        const controlFlowSTL = `A X0.0
JC label1
= Y0.0
label1:
= Y0.1
`
        
        await streamCode(runtime, controlFlowSTL)
        exports.stl_to_ir_load_from_stream()
        
        if (exports.stl_to_ir_parse()) {
            console.log(`  Parse: unexpectedly succeeded (should have failed)`)
        } else {
            console.log(`  Parse failed as expected`)
            console.log(`  Error: ${getString(exports.stl_to_ir_get_error())}`)
        }
        
        // Test 8: Full pipeline with simple STL
        console.log('\nTest 8: Full pipeline STL -> IR -> STL -> PLCASM -> Bytecode')
        exports.nir_reset()
        
        const pipelineSTL = `A X0.0
AN X0.1
= Y0.0
`
        
        await streamCode(runtime, pipelineSTL)
        exports.ladder_load_from_stream()
        
        if (exports.ladder_compile_full && exports.ladder_compile_full()) {
            console.log('  Full pipeline: success')
            // Check bytecode output
            const bytecodeLen = exports.compiler_get_bytecode_length ? exports.compiler_get_bytecode_length() : -1
            console.log(`  Bytecode length: ${bytecodeLen}`)
        } else {
            console.log('  Full pipeline: failed')
            if (exports.ladder_has_error && exports.ladder_has_error()) {
                console.log(`  Error: ${getString(exports.ladder_get_error())}`)
            }
        }
        
        console.log('\n=== Tests Complete ===\n')
        
    } catch (err) {
        console.error('Test error:', err)
    }
}

runTests()
    .catch(console.error)
    .finally(() => process.exit())
