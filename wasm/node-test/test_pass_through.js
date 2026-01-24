// Test file for Pass-Through Element Grouping
// Tests that consecutive pass-through elements share the same JCN skip block

import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')

async function runTests() {
    const runtime = await VovkPLC.createWorker(wasmPath)
    console.log('VovkPLCRuntime WASM loaded successfully\n')

    let passed = 0
    let failed = 0

    try {
        // Helper to write string to stream
        const writeToStream = async (str) => {
            for (let i = 0; i < str.length; i++) {
                await runtime.callExport('streamIn', str.charCodeAt(i))
            }
            await runtime.callExport('streamIn', 0) // Null terminator
        }

        // Helper to compile and get output
        const compileAndGetOutput = async (json) => {
            await writeToStream(json)
            await runtime.callExport('ladder_load_from_stream')
            const success = await runtime.callExport('ladder_compile')
            if (!success) {
                const errPtr = await runtime.callExport('ladder_get_error')
                throw new Error(`Compilation failed`)
            }
            await runtime.callExport('ladder_output_to_stream')
            return await runtime.readStream()
        }

        // =====================================================
        // Test 1: Sequential pass-through (inc then coil)
        // =====================================================
        console.log('='.repeat(60))
        console.log('Test 1: Sequential pass-through (inc then coil)')
        console.log('='.repeat(60))
        console.log('Input: contact X0 -> inc MW0 -> coil Y0')
        console.log('Expected: A X0, JCN skip, INCI MW0, = Y0, skip:')
        
        const test1 = JSON.stringify({
            rungs: [{
                elements: [
                    { type: 'contact', address: 'X0' },
                    { type: 'inc', address: 'MW0', passThrough: true },
                    { type: 'coil', address: 'Y0', passThrough: true }
                ]
            }]
        })
        
        const output1 = await compileAndGetOutput(test1)
        console.log('\nGenerated STL:')
        console.log(output1)
        
        // Check for single skip block
        const skip1Count = (output1.match(/skip_/g) || []).length
        if (skip1Count <= 2 && output1.includes('INCI') && output1.includes('= Y0')) {
            console.log('✓ Test 1 PASSED: Single JCN skip block for grouped pass-through elements')
            passed++
        } else {
            console.log('✗ Test 1 FAILED: Expected single skip block')
            failed++
        }

        // =====================================================
        // Test 2: Multiple parallel pass-through (same RLO source)
        // =====================================================
        console.log('\n' + '='.repeat(60))
        console.log('Test 2: Multiple parallel pass-through (same RLO source)')
        console.log('='.repeat(60))
        console.log('Input: contact X0 -> inc MW0, inc MW2, inc MW4')
        console.log('Expected: A X0, JCN skip, INCI MW0, INCI MW2, INCI MW4, skip:')
        
        const test2 = JSON.stringify({
            rungs: [{
                elements: [
                    { type: 'contact', address: 'X0' },
                    { type: 'inc', address: 'MW0', passThrough: true },
                    { type: 'inc', address: 'MW2', passThrough: true },
                    { type: 'inc', address: 'MW4', passThrough: true }
                ]
            }]
        })
        
        const output2 = await compileAndGetOutput(test2)
        console.log('\nGenerated STL:')
        console.log(output2)
        
        // Check for single skip block with 3 INCI instructions
        const skip2Count = (output2.match(/skip_/g) || []).length
        const inci2Count = (output2.match(/INCI/g) || []).length
        if (skip2Count <= 2 && inci2Count === 3) {
            console.log('✓ Test 2 PASSED: All 3 INCI instructions in single skip block')
            passed++
        } else {
            console.log(`✗ Test 2 FAILED: Expected 3 INCI in single skip block (got ${inci2Count} INCI, ${skip2Count/2} skip blocks)`)
            failed++
        }

        // =====================================================
        // Test 3: Pass-through then logic branch
        // =====================================================
        console.log('\n' + '='.repeat(60))
        console.log('Test 3: Pass-through then logic branch')
        console.log('='.repeat(60))
        console.log('Input: contact X0 -> coil Y0 -> contact X1 -> coil Y1')
        console.log('Expected: Two networks - Y0 under X0, Y1 under X0 AND X1')
        
        const test3 = JSON.stringify({
            rungs: [{
                elements: [
                    { type: 'contact', address: 'X0' },
                    { type: 'coil', address: 'Y0', passThrough: true },
                    { type: 'contact', address: 'X1' },
                    { type: 'coil', address: 'Y1', passThrough: true }
                ]
            }]
        })
        
        const output3 = await compileAndGetOutput(test3)
        console.log('\nGenerated STL:')
        console.log(output3)
        
        // Should have separate conditions: Y0 under X0, Y1 under X0 AND X1
        if (output3.includes('A X0') && output3.includes('= Y0') && 
            output3.includes('A X1') && output3.includes('= Y1')) {
            console.log('✓ Test 3 PASSED: Logic branch after pass-through creates new condition')
            passed++
        } else {
            console.log('✗ Test 3 FAILED: Expected proper logic branching')
            failed++
        }

        // =====================================================
        // Test 4: Mixed pass-through types (coil + inc + math)
        // =====================================================
        console.log('\n' + '='.repeat(60))
        console.log('Test 4: Mixed pass-through types')
        console.log('='.repeat(60))
        console.log('Input: contact button1 -> inc MW14 -> inc MW14 -> inc MW22')
        console.log('Expected: A button1, JCN skip, INCI MW14, INCI MW14, INCI MW22, skip:')
        
        const test4 = JSON.stringify({
            rungs: [{
                elements: [
                    { type: 'contact', address: 'button1' },
                    { type: 'inc', address: 'MW14', passThrough: true },
                    { type: 'inc', address: 'MW14', passThrough: true },
                    { type: 'inc', address: 'MW22', passThrough: true }
                ]
            }]
        })
        
        const output4 = await compileAndGetOutput(test4)
        console.log('\nGenerated STL:')
        console.log(output4)
        
        // Check for single skip block
        const skip4Count = (output4.match(/skip_/g) || []).length
        const inci4Count = (output4.match(/INCI/g) || []).length
        if (skip4Count <= 2 && inci4Count === 3) {
            console.log('✓ Test 4 PASSED: All pass-through elements grouped in single skip block')
            passed++
        } else {
            console.log(`✗ Test 4 FAILED: Expected single skip block with 3 INCI (got ${inci4Count} INCI, ${skip4Count/2} skip blocks)`)
            failed++
        }

        // =====================================================
        // Test 5: Coil-only network (should not need JCN)
        // =====================================================
        console.log('\n' + '='.repeat(60))
        console.log('Test 5: Coil-only network')
        console.log('='.repeat(60))
        console.log('Input: contact X0 -> coil Y0')
        console.log('Expected: A X0, = Y0 (no JCN needed for coil-only)')
        
        const test5 = JSON.stringify({
            rungs: [{
                elements: [
                    { type: 'contact', address: 'X0' },
                    { type: 'coil', address: 'Y0' }
                ]
            }]
        })
        
        const output5 = await compileAndGetOutput(test5)
        console.log('\nGenerated STL:')
        console.log(output5)
        
        // Coil-only should not have JCN
        if (!output5.includes('JCN')) {
            console.log('✓ Test 5 PASSED: Coil-only network emits without JCN')
            passed++
        } else {
            console.log('✗ Test 5 FAILED: Coil-only network should not need JCN')
            failed++
        }

        // =====================================================
        // Test 6: Auto-detection of pass-through types
        // =====================================================
        console.log('\n' + '='.repeat(60))
        console.log('Test 6: Auto-detection of pass-through types (no explicit flag)')
        console.log('='.repeat(60))
        console.log('Input: contact X0 -> inc MW0 -> dec MW2 (no passThrough flag)')
        console.log('Expected: Auto-detect inc/dec as pass-through, group together')
        
        const test6 = JSON.stringify({
            rungs: [{
                elements: [
                    { type: 'contact', address: 'X0' },
                    { type: 'inc', address: 'MW0' },  // No explicit passThrough flag
                    { type: 'dec', address: 'MW2' }   // Should auto-detect
                ]
            }]
        })
        
        const output6 = await compileAndGetOutput(test6)
        console.log('\nGenerated STL:')
        console.log(output6)
        
        const skip6Count = (output6.match(/skip_/g) || []).length
        if (skip6Count <= 2 && output6.includes('INCI') && output6.includes('DECI')) {
            console.log('✓ Test 6 PASSED: Auto-detected pass-through types grouped correctly')
            passed++
        } else {
            console.log('✗ Test 6 FAILED: Pass-through type auto-detection failed')
            failed++
        }

        // =====================================================
        // Test 7: TAP instruction for RLO continuation
        // =====================================================
        console.log('\n' + '='.repeat(60))
        console.log('Test 7: TAP instruction for RLO continuation')
        console.log('='.repeat(60))
        console.log('Input: contact X0 -> coil Y0 -> TAP -> contact X1 -> coil Y1')
        console.log('Expected: First network with TAP, second network uses continued RLO')
        
        const test7 = JSON.stringify({
            rungs: [{
                elements: [
                    { type: 'contact', address: 'X0' },
                    { type: 'coil', address: 'Y0' },
                    { type: 'tap' },
                    { type: 'contact', address: 'X1' },
                    { type: 'coil', address: 'Y1' }
                ]
            }]
        })
        
        const output7 = await compileAndGetOutput(test7)
        console.log('\nGenerated STL:')
        console.log(output7)
        
        // Should have TAP instruction and two separate network sections
        if (output7.includes('TAP') && output7.includes('A X0') && 
            output7.includes('= Y0') && output7.includes('A X1') && output7.includes('= Y1')) {
            console.log('✓ Test 7 PASSED: TAP instruction correctly splits networks')
            passed++
        } else {
            console.log('✗ Test 7 FAILED: TAP instruction not working correctly')
            failed++
        }

        // =====================================================
        // Test 8: TAP with pass-through grouping
        // =====================================================
        console.log('\n' + '='.repeat(60))
        console.log('Test 8: TAP with pass-through grouping')
        console.log('='.repeat(60))
        console.log('Input: contact X0 -> inc MW0 -> inc MW2 -> TAP -> inc MW4 -> coil Y0')
        console.log('Expected: First group (MW0, MW2) with TAP, second group (MW4, Y0)')
        
        const test8 = JSON.stringify({
            rungs: [{
                elements: [
                    { type: 'contact', address: 'X0' },
                    { type: 'inc', address: 'MW0', passThrough: true },
                    { type: 'inc', address: 'MW2', passThrough: true },
                    { type: 'tap' },
                    { type: 'inc', address: 'MW4', passThrough: true },
                    { type: 'coil', address: 'Y0', passThrough: true }
                ]
            }]
        })
        
        const output8 = await compileAndGetOutput(test8)
        console.log('\nGenerated STL:')
        console.log(output8)
        
        // Should have TAP and proper grouping
        const inci8Count = (output8.match(/INCI/g) || []).length
        if (output8.includes('TAP') && inci8Count === 3 && output8.includes('= Y0')) {
            console.log('✓ Test 8 PASSED: TAP works correctly with pass-through grouping')
            passed++
        } else {
            console.log('✗ Test 8 FAILED: TAP with pass-through grouping not working')
            failed++
        }

        // =====================================================
        // Summary
        // =====================================================
        console.log('\n' + '='.repeat(60))
        console.log(`Test Summary: ${passed} passed, ${failed} failed`)
        console.log('='.repeat(60))

    } catch (error) {
        console.error('Error during tests:', error)
        failed++
    } finally {
        await runtime.terminate()
    }

    process.exit(failed > 0 ? 1 : 0)
}

runTests()
