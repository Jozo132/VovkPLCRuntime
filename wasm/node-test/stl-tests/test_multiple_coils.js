// test_multiple_coils.js - Test multiple coil outputs on same rung
import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import {fileURLToPath} from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

// Helper to transpile STL
async function transpileSTL(runtime, stlCode) {
    // Stream the STL code
    for (let i = 0; i < stlCode.length; i++) {
        await runtime.callExport('streamIn', stlCode.charCodeAt(i))
    }
    await runtime.callExport('streamIn', 0) // null terminator
    
    await runtime.callExport('stl_load_from_stream')
    await runtime.callExport('stl_compile')
    await runtime.callExport('stl_output_to_stream')
    
    return await runtime.readStream()
}

const run = async () => {
    const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')
    const runtime = await VovkPLC.createWorker(wasmPath)
    
    console.log('=== Test: Multiple Coil Outputs ===\n')
    
    // Test 1: Simple multiple outputs
    const stl1 = `A I0.0
= Q0.0
= Q0.1
= Q0.2`
    
    console.log('Input STL:')
    console.log(stl1)
    console.log('\nExpected pattern:')
    console.log('u8.readBit X0.0')
    console.log('u8.copy       // Before Q0.0 (Q0.1 follows)')
    console.log('u8.writeBit Y0.0')
    console.log('u8.copy       // Before Q0.1 (Q0.2 follows)')
    console.log('u8.writeBit Y0.1')
    console.log('u8.writeBit Y0.2  // Last output, no copy')
    console.log('\nActual PLCASM:')
    const result1 = await transpileSTL(runtime, stl1)
    console.log(result1)
    
    // Count u8.copy occurrences - should be 2 (one before each output except last)
    const dupCount = (result1.match(/u8\.copy/g) || []).length
    console.log(`u8.copy count: ${dupCount} (expected: 2)`)
    console.log(dupCount === 2 ? '✓ PASS' : '✗ FAIL')
    
    // Test 2: Mix of =, S, R outputs
    console.log('\n=== Test: Mixed output instructions ===')
    const stl2 = `A I0.0
= Q0.0
S M0.0
R M0.1`
    
    console.log('Input STL:')
    console.log(stl2)
    console.log('\nActual PLCASM:')
    const result2 = await transpileSTL(runtime, stl2)
    console.log(result2)
    
    const dupCount2 = (result2.match(/u8\.copy/g) || []).length
    console.log(`u8.copy count: ${dupCount2} (expected: 2)`)
    console.log(dupCount2 === 2 ? '✓ PASS' : '✗ FAIL')
    
    // Test 3: Single output (no copy needed)
    console.log('\n=== Test: Single output (no copy) ===')
    const stl3 = `A I0.0
= Q0.0`
    
    console.log('Input STL:')
    console.log(stl3)
    console.log('\nActual PLCASM:')
    const result3 = await transpileSTL(runtime, stl3)
    console.log(result3)
    
    const dupCount3 = (result3.match(/u8\.copy/g) || []).length
    console.log(`u8.copy count: ${dupCount3} (expected: 0)`)
    console.log(dupCount3 === 0 ? '✓ PASS' : '✗ FAIL')
    
    await runtime.terminate()
    await new Promise(res => setTimeout(res, 100))
    process.exit(0)
}

run().catch(err => {
    console.error(err)
    process.exit(1)
})
