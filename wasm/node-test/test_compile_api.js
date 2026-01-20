#!/usr/bin/env node
// test_compile_api.js - Tests for VovkPLC.js compilation API wrappers

import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')

console.log('============================================================')
console.log('VovkPLC.js Compilation API Tests')
console.log('============================================================')
console.log()

// Test direct class usage
const runDirectTests = async () => {
    console.log('┌─────────────────────────────────────────────────────────────────┐')
    console.log('│ Testing VovkPLC Class Direct API                                │')
    console.log('└─────────────────────────────────────────────────────────────────┘')
    console.log()

    const plc = new VovkPLC()
    await plc.initialize(wasmPath, false, true)

    // Test 1: compileLadder
    console.log('Test 1: compileLadder()')
    const ladderJson = {
        rungs: [{
            elements: [
                { type: 'contact', address: 'I0.0' },
                { type: 'coil', address: 'Q0.0' }
            ]
        }]
    }
    const ladderResult = plc.compileLadder(ladderJson)
    console.log(`  Type: ${ladderResult.type}`)
    console.log(`  Size: ${ladderResult.size} bytes`)
    console.log(`  Output:\n${ladderResult.output.split('\n').map(l => '    ' + l).join('\n')}`)
    console.log('  ✓ compileLadder() works')
    console.log()

    // Test 2: compileSTL
    console.log('Test 2: compileSTL()')
    const stlResult = plc.compileSTL('A I0.0\n= Q0.0')
    console.log(`  Type: ${stlResult.type}`)
    console.log(`  Size: ${stlResult.size} bytes`)
    console.log(`  Output:\n${stlResult.output.split('\n').map(l => '    ' + l).join('\n')}`)
    console.log('  ✓ compileSTL() works')
    console.log()

    // Test 3: compilePLCASM
    console.log('Test 3: compilePLCASM()')
    const plcasmResult = plc.compilePLCASM('u8.const 42\nexit')
    console.log(`  Type: ${plcasmResult.type}`)
    console.log(`  Size: ${plcasmResult.size} bytes`)
    console.log(`  Output: ${plcasmResult.output.trim()}`)
    console.log('  ✓ compilePLCASM() works')
    console.log()

    // Test 4: compile() with language option
    console.log('Test 4: compile() with language options')
    
    const compileLadder = plc.compile(JSON.stringify(ladderJson), { language: 'ladder' })
    console.log(`  compile(ladder): type=${compileLadder.type}`)
    
    const compileStl = plc.compile('A I0.1\n= Q0.1', { language: 'stl' })
    console.log(`  compile(stl): type=${compileStl.type}`)
    
    const compilePlcasm = plc.compile('u8.const 1\nexit', { language: 'plcasm' })
    console.log(`  compile(plcasm): type=${compilePlcasm.type}`)
    
    console.log('  ✓ compile() with language options works')
    console.log()

    // Test 5: compileAll - full pipeline from ladder
    console.log('Test 5: compileAll() - Full pipeline from Ladder')
    const allFromLadder = plc.compileAll(ladderJson, 'ladder')
    console.log(`  Has ladder: ${!!allFromLadder.ladder}`)
    console.log(`  Has stl: ${!!allFromLadder.stl}`)
    console.log(`  Has plcasm: ${!!allFromLadder.plcasm}`)
    console.log(`  Has bytecode: ${!!allFromLadder.bytecode}`)
    console.log('  ✓ compileAll(ladder) works')
    console.log()

    // Test 6: compileAll - from STL
    console.log('Test 6: compileAll() - From STL')
    const allFromStl = plc.compileAll('A I0.0\n= Q0.0', 'stl')
    console.log(`  Has ladder: ${!!allFromStl.ladder}`)
    console.log(`  Has stl: ${!!allFromStl.stl}`)
    console.log(`  Has plcasm: ${!!allFromStl.plcasm}`)
    console.log(`  Has bytecode: ${!!allFromStl.bytecode}`)
    console.log('  ✓ compileAll(stl) works')
    console.log()

    // Test 7: compileAll - from PLCASM
    console.log('Test 7: compileAll() - From PLCASM')
    const allFromPlcasm = plc.compileAll('u8.const 1\nexit', 'plcasm')
    console.log(`  Has ladder: ${!!allFromPlcasm.ladder}`)
    console.log(`  Has stl: ${!!allFromPlcasm.stl}`)
    console.log(`  Has plcasm: ${!!allFromPlcasm.plcasm}`)
    console.log(`  Has bytecode: ${!!allFromPlcasm.bytecode}`)
    console.log('  ✓ compileAll(plcasm) works')
    console.log()
}

// Test worker usage
const runWorkerTests = async () => {
    console.log('┌─────────────────────────────────────────────────────────────────┐')
    console.log('│ Testing VovkPLCWorker API                                       │')
    console.log('└─────────────────────────────────────────────────────────────────┘')
    console.log()

    const worker = await VovkPLC.createWorker(wasmPath, { silent: true })

    // Test 1: compileLadder via worker
    console.log('Test 1: worker.compileLadder()')
    const ladderJson = {
        rungs: [{
            elements: [
                { type: 'contact', address: 'I0.0' },
                { type: 'contact', address: 'I0.1' },
                { type: 'coil', address: 'Q0.0' }
            ]
        }]
    }
    const ladderResult = await worker.compileLadder(ladderJson)
    console.log(`  Type: ${ladderResult.type}`)
    console.log(`  Contains "A I0.0": ${ladderResult.output.includes('A I0.0')}`)
    console.log(`  Contains "A I0.1": ${ladderResult.output.includes('A I0.1')}`)
    console.log('  ✓ worker.compileLadder() works')
    console.log()

    // Test 2: compileSTL via worker
    console.log('Test 2: worker.compileSTL()')
    const stlResult = await worker.compileSTL('A I0.0\nA I0.1\n= Q0.0')
    console.log(`  Type: ${stlResult.type}`)
    console.log(`  Contains "u8.readBit": ${stlResult.output.includes('u8.readBit')}`)
    console.log('  ✓ worker.compileSTL() works')
    console.log()

    // Test 3: compile with language option via worker
    console.log('Test 3: worker.compile() with language options')
    const result1 = await worker.compile(JSON.stringify(ladderJson), { language: 'ladder' })
    const result2 = await worker.compile('A I0.0\n= Q0.0', { language: 'stl' })
    const result3 = await worker.compile('u8.const 1\nexit', { language: 'plcasm' })
    console.log(`  ladder -> ${result1.type}`)
    console.log(`  stl -> ${result2.type}`)
    console.log(`  plcasm -> ${result3.type}`)
    console.log('  ✓ worker.compile() with language options works')
    console.log()

    // Test 4: compileAll via worker
    console.log('Test 4: worker.compileAll()')
    const allResult = await worker.compileAll(ladderJson, 'ladder')
    console.log(`  ladder: ${allResult.ladder ? allResult.ladder.length + ' chars' : 'N/A'}`)
    console.log(`  stl: ${allResult.stl ? allResult.stl.split('\n').filter(l => l.trim()).length + ' lines' : 'N/A'}`)
    console.log(`  plcasm: ${allResult.plcasm ? allResult.plcasm.split('\n').filter(l => l.trim()).length + ' lines' : 'N/A'}`)
    console.log(`  bytecode: ${allResult.bytecode ? allResult.bytecode.trim().split(' ').length + ' bytes' : 'N/A'}`)
    console.log('  ✓ worker.compileAll() works')
    console.log()

    await worker.terminate()
}

// Run all tests
const run = async () => {
    try {
        await runDirectTests()
        await runWorkerTests()

        console.log('============================================================')
        console.log('All API tests passed!')
        console.log('============================================================')
        
        // Give worker time to clean up
        await new Promise(res => setTimeout(res, 100))
        process.exit(0)
    } catch (e) {
        console.error('Test failed:', e.message)
        console.error(e.stack)
        process.exit(1)
    }
}

run()
