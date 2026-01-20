// Test file for Ladder Compiler
// Tests LADDER JSON to STL conversion and STL to LADDER JSON conversion

import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')

async function runTests() {
    const runtime = await VovkPLC.createWorker(wasmPath)
    console.log('VovkPLCRuntime WASM loaded successfully\n')

    try {
        // Helper to write string to stream
        const writeToStream = async (str) => {
            for (let i = 0; i < str.length; i++) {
                await runtime.callExport('streamIn', str.charCodeAt(i))
            }
            await runtime.callExport('streamIn', 0) // Null terminator
        }

        // =====================================================
        // Test 1: Simple rung with one contact and one coil
        // =====================================================
        console.log('='.repeat(60))
        console.log('Test 1: Simple rung (one contact, one coil)')
        console.log('='.repeat(60))
        
        const ladder1 = JSON.stringify({
            rungs: [
                {
                    elements: [
                        { type: 'contact', address: 'I0.0', inverted: false, trigger: 'normal' },
                        { type: 'coil', address: 'Q0.0', inverted: false }
                    ]
                }
            ]
        })
        
        await writeToStream(ladder1)
        await runtime.callExport('ladder_load_from_stream')
        
        const success1 = await runtime.callExport('ladder_compile')
        if (!success1) {
            console.log('ERROR compiling ladder')
        } else {
            await runtime.callExport('ladder_output_to_stream')
            const output1 = await runtime.readStream()
            console.log('Input LADDER JSON:')
            console.log(ladder1)
            console.log('\nOutput STL:')
            console.log(output1)
        }

        // =====================================================
        // Test 2: Series contacts (AND logic)
        // =====================================================
        console.log('\n' + '='.repeat(60))
        console.log('Test 2: Series contacts (AND logic)')
        console.log('='.repeat(60))
        
        const ladder2 = JSON.stringify({
            rungs: [
                {
                    elements: [
                        { type: 'contact', address: 'I0.0', inverted: false, trigger: 'normal' },
                        { type: 'contact', address: 'I0.1', inverted: false, trigger: 'normal' },
                        { type: 'contact', address: 'I0.2', inverted: true, trigger: 'normal' },
                        { type: 'coil', address: 'Q0.0', inverted: false }
                    ]
                }
            ]
        })
        
        await writeToStream(ladder2)
        await runtime.callExport('ladder_load_from_stream')
        
        const success2 = await runtime.callExport('ladder_compile')
        if (!success2) {
            console.log('ERROR compiling ladder')
        } else {
            await runtime.callExport('ladder_output_to_stream')
            const output2 = await runtime.readStream()
            console.log('Input LADDER JSON:')
            console.log(ladder2)
            console.log('\nOutput STL:')
            console.log(output2)
            console.log('Expected: A I0.0, A I0.1, AN I0.2, = Q0.0')
        }

        // =====================================================
        // Test 3: Multiple coils (same RLO for multiple outputs)
        // =====================================================
        console.log('\n' + '='.repeat(60))
        console.log('Test 3: Multiple coils')
        console.log('='.repeat(60))
        
        const ladder3 = JSON.stringify({
            rungs: [
                {
                    elements: [
                        { type: 'contact', address: 'I0.0', inverted: false, trigger: 'normal' },
                        { type: 'coil', address: 'Q0.0', inverted: false },
                        { type: 'coil', address: 'Q0.1', inverted: false },
                        { type: 'coil_set', address: 'Q0.2', inverted: false },
                        { type: 'coil_rset', address: 'Q0.3', inverted: false }
                    ]
                }
            ]
        })
        
        await writeToStream(ladder3)
        await runtime.callExport('ladder_load_from_stream')
        
        const success3 = await runtime.callExport('ladder_compile')
        if (!success3) {
            console.log('ERROR compiling ladder')
        } else {
            await runtime.callExport('ladder_output_to_stream')
            const output3 = await runtime.readStream()
            console.log('Input LADDER JSON:')
            console.log(ladder3)
            console.log('\nOutput STL:')
            console.log(output3)
            console.log('Expected: A I0.0, = Q0.0, = Q0.1, S Q0.2, R Q0.3')
        }

        // =====================================================
        // Test 4: Edge detection (rising edge)
        // =====================================================
        console.log('\n' + '='.repeat(60))
        console.log('Test 4: Edge detection (rising edge)')
        console.log('='.repeat(60))
        
        const ladder4 = JSON.stringify({
            rungs: [
                {
                    elements: [
                        { type: 'contact', address: 'I0.0', inverted: false, trigger: 'rising' },
                        { type: 'coil', address: 'Q0.0', inverted: false }
                    ]
                }
            ]
        })
        
        await writeToStream(ladder4)
        await runtime.callExport('ladder_load_from_stream')
        
        const success4 = await runtime.callExport('ladder_compile')
        if (!success4) {
            console.log('ERROR compiling ladder')
        } else {
            await runtime.callExport('ladder_output_to_stream')
            const output4 = await runtime.readStream()
            console.log('Input LADDER JSON:')
            console.log(ladder4)
            console.log('\nOutput STL:')
            console.log(output4)
            console.log('Expected: A I0.0, FP M100.0, = Q0.0')
        }

        // =====================================================
        // Test 5: Complex ladder with OR branches (parallel contacts)
        // =====================================================
        console.log('\n' + '='.repeat(60))
        console.log('Test 5: Complex ladder with OR branches')
        console.log('='.repeat(60))
        
        // Ladder diagram:
        //     +--[ I0.0 ]--[ I0.1 ]--+
        //     |                      |
        // ----+--[ I0.2 ]--[ I0.3 ]--+--[ M0.0 ]--( Q0.0 )
        //     |                      |
        //     +--[ I0.4 ]------------+
        //
        // This represents: ((I0.0 AND I0.1) OR (I0.2 AND I0.3) OR I0.4) AND M0.0 = Q0.0
        
        const ladder5 = JSON.stringify({
            rungs: [
                {
                    comment: 'Complex OR logic with series contacts',
                    elements: [
                        // Main branch: I0.0 AND I0.1
                        { type: 'contact', address: 'I0.0', inverted: false, trigger: 'normal' },
                        { type: 'contact', address: 'I0.1', inverted: false, trigger: 'normal' },
                        { type: 'coil', address: 'Q0.0', inverted: false }
                    ],
                    branches: [
                        {
                            // Second branch: I0.2 AND I0.3
                            elements: [
                                { type: 'contact', address: 'I0.2', inverted: false, trigger: 'normal' },
                                { type: 'contact', address: 'I0.3', inverted: false, trigger: 'normal' }
                            ]
                        },
                        {
                            // Third branch: just I0.4
                            elements: [
                                { type: 'contact', address: 'I0.4', inverted: false, trigger: 'normal' }
                            ]
                        }
                    ]
                }
            ]
        })
        
        await writeToStream(ladder5)
        await runtime.callExport('ladder_load_from_stream')
        
        const success5 = await runtime.callExport('ladder_compile')
        if (!success5) {
            console.log('ERROR compiling ladder')
        } else {
            await runtime.callExport('ladder_output_to_stream')
            const output5 = await runtime.readStream()
            console.log('Input LADDER JSON:')
            console.log(JSON.stringify(JSON.parse(ladder5), null, 2))
            console.log('\n  Ladder Diagram:')
            console.log('      +──┤ I0.0 ├──┤ I0.1 ├──+')
            console.log('      │                      │')
            console.log('  ────+──┤ I0.2 ├──┤ I0.3 ├──+──( Q0.0 )──')
            console.log('      │                      │')
            console.log('      +──┤ I0.4 ├────────────+')
            console.log('\nOutput STL:')
            console.log(output5)
            console.log('Expected: (I0.0 AND I0.1) OR (I0.2 AND I0.3) OR I0.4 = Q0.0')
            console.log('Expected STL: A I0.0, A I0.1, O( A I0.2 A I0.3 ), O( A I0.4 ), = Q0.0')
        }

        // =====================================================
        // Test 6: Multiple rungs
        // =====================================================
        console.log('\n' + '='.repeat(60))
        console.log('Test 6: Multiple rungs')
        console.log('='.repeat(60))
        
        const ladder6 = JSON.stringify({
            rungs: [
                {
                    comment: 'Rung 1 - Start button',
                    elements: [
                        { type: 'contact', address: 'I0.0', inverted: false, trigger: 'normal' },
                        { type: 'coil_set', address: 'M0.0', inverted: false }
                    ]
                },
                {
                    comment: 'Rung 2 - Stop button',
                    elements: [
                        { type: 'contact', address: 'I0.1', inverted: false, trigger: 'normal' },
                        { type: 'coil_rset', address: 'M0.0', inverted: false }
                    ]
                },
                {
                    comment: 'Rung 3 - Output',
                    elements: [
                        { type: 'contact', address: 'M0.0', inverted: false, trigger: 'normal' },
                        { type: 'coil', address: 'Q0.0', inverted: false }
                    ]
                }
            ]
        })
        
        await writeToStream(ladder6)
        await runtime.callExport('ladder_load_from_stream')
        
        const success6 = await runtime.callExport('ladder_compile')
        if (!success6) {
            console.log('ERROR compiling ladder')
        } else {
            await runtime.callExport('ladder_output_to_stream')
            const output6 = await runtime.readStream()
            console.log('Input LADDER JSON:')
            console.log(JSON.stringify(JSON.parse(ladder6), null, 2))
            console.log('\nOutput STL:')
            console.log(output6)
        }

        // =====================================================
        // Test 7: RS Flip-Flop - Full pipeline with logging
        // =====================================================
        console.log('\n' + '='.repeat(60))
        console.log('Test 7: RS Flip-Flop - LADDER → STL → PLCASM')
        console.log('='.repeat(60))
        
        const ladderRS = JSON.stringify({
            rungs: [
                {
                    comment: 'Set input - pressing I0.0 sets the latch',
                    elements: [
                        { type: 'contact', address: 'I0.0', inverted: false, trigger: 'normal' },
                        { type: 'coil_set', address: 'M0.0', inverted: false }
                    ]
                },
                {
                    comment: 'Reset input - pressing I0.1 resets the latch',
                    elements: [
                        { type: 'contact', address: 'I0.1', inverted: false, trigger: 'normal' },
                        { type: 'coil_rset', address: 'M0.0', inverted: false }
                    ]
                },
                {
                    comment: 'Output - latch state drives output Q0.0',
                    elements: [
                        { type: 'contact', address: 'M0.0', inverted: false, trigger: 'normal' },
                        { type: 'coil', address: 'Q0.0', inverted: false }
                    ]
                }
            ]
        })
        
        console.log('\n┌─────────────────────────────────────────────────────────┐')
        console.log('│ STEP 1: LADDER Logic (JSON)                             │')
        console.log('└─────────────────────────────────────────────────────────┘')
        console.log(JSON.stringify(JSON.parse(ladderRS), null, 2))
        
        console.log('\n  Ladder Diagram:')
        console.log('  ┌───────────────────────────────────────────┐')
        console.log('  │  Rung 1: SET                              │')
        console.log('  │    ──┤ I0.0 ├──────────────────(S M0.0)── │')
        console.log('  │                                           │')
        console.log('  │  Rung 2: RESET                            │')
        console.log('  │    ──┤ I0.1 ├──────────────────(R M0.0)── │')
        console.log('  │                                           │')
        console.log('  │  Rung 3: OUTPUT                           │')
        console.log('  │    ──┤ M0.0 ├──────────────────( Q0.0 )── │')
        console.log('  └───────────────────────────────────────────┘')
        
        await writeToStream(ladderRS)
        await runtime.callExport('ladder_load_from_stream')
        
        const successRS = await runtime.callExport('ladder_compile')
        if (!successRS) {
            console.log('ERROR compiling ladder')
        } else {
            await runtime.callExport('ladder_output_to_stream')
            const stlRS = await runtime.readStream()
            
            console.log('\n┌─────────────────────────────────────────────────────────┐')
            console.log('│ STEP 2: STL (Statement List)                            │')
            console.log('└─────────────────────────────────────────────────────────┘')
            console.log(stlRS)
            
            // Now compile STL to PLCASM
            await writeToStream(stlRS)
            await runtime.callExport('stl_load_from_stream')
            await runtime.callExport('stl_compile')
            await runtime.callExport('stl_output_to_stream')
            const plcasmRS = await runtime.readStream()
            
            console.log('┌─────────────────────────────────────────────────────────┐')
            console.log('│ STEP 3: PLCASM (Low-level Assembly)                     │')
            console.log('└─────────────────────────────────────────────────────────┘')
            console.log(plcasmRS)
            
            console.log('┌─────────────────────────────────────────────────────────┐')
            console.log('│ Summary: RS Flip-Flop                                   │')
            console.log('├─────────────────────────────────────────────────────────┤')
            console.log('│ I0.0 = SET button    → Sets M0.0 latch                  │')
            console.log('│ I0.1 = RESET button  → Resets M0.0 latch                │')
            console.log('│ M0.0 = Latch memory  → Internal state                   │')
            console.log('│ Q0.0 = Output        → Follows latch state              │')
            console.log('└─────────────────────────────────────────────────────────┘')
        }

        // =====================================================
        // Test 8: STL to Ladder - Simple conversion
        // =====================================================
        console.log('\n' + '='.repeat(60))
        console.log('Test 8: STL to Ladder conversion')
        console.log('='.repeat(60))
        
        const stl8 = `
// Simple rung
A I0.0
A I0.1
= Q0.0

// Second rung
AN I0.2
S M0.0
R M0.1
`
        
        await writeToStream(stl8)
        await runtime.callExport('stl_load_from_stream')
        
        const error8 = await runtime.callExport('stl_to_ladder_convert')
        const hasError8 = await runtime.callExport('stl_to_ladder_has_error')
        if (hasError8) {
            console.log('ERROR in STL to Ladder conversion')
        } else {
            await runtime.callExport('stl_to_ladder_output_to_stream')
            const output8 = await runtime.readStream()
            console.log('Input STL:')
            console.log(stl8)
            console.log('\nOutput LADDER JSON:')
            console.log(output8)
            try {
                console.log('\nParsed:')
                console.log(JSON.stringify(JSON.parse(output7), null, 2))
            } catch (e) {
                console.log('(Could not parse JSON)')
            }
        }

        // =====================================================
        // Test 9: STL to Ladder - Control flow detection
        // =====================================================
        console.log('\n' + '='.repeat(60))
        console.log('Test 9: STL to Ladder - Control flow should error')
        console.log('='.repeat(60))
        
        const stl9 = `
A I0.0
JC LABEL1
= Q0.0
LABEL1:
= Q0.1
`
        
        await writeToStream(stl9)
        await runtime.callExport('stl_load_from_stream')
        
        await runtime.callExport('stl_to_ladder_convert')
        const hasError9 = await runtime.callExport('stl_to_ladder_has_error')
        if (hasError9) {
            console.log('Input STL (with control flow):')
            console.log(stl9)
            console.log('\n✓ Correctly detected control flow (stl_to_ladder_has_error = true)')
        } else {
            await runtime.callExport('stl_to_ladder_output_to_stream')
            const output9 = await runtime.readStream()
            console.log('WARNING: Should have errored for control flow!')
            console.log('Output:', output9)
        }

        // =====================================================
        // Test 10: Full pipeline - Ladder to bytecode
        // =====================================================
        console.log('\n' + '='.repeat(60))
        console.log('Test 10: Full pipeline - Ladder to bytecode')
        console.log('='.repeat(60))
        
        const ladder10 = JSON.stringify({
            rungs: [
                {
                    elements: [
                        { type: 'contact', address: 'I0.0', inverted: false, trigger: 'normal' },
                        { type: 'coil', address: 'Q0.0', inverted: false }
                    ]
                }
            ]
        })
        
        await writeToStream(ladder10)
        await runtime.callExport('ladder_load_from_stream')
        
        const success10 = await runtime.callExport('ladder_compile_full')
        if (!success10) {
            console.log('Error in full pipeline')
            // Try to see where the error was
            const ladderErr = await runtime.callExport('ladder_has_error')
            const stlErr = await runtime.callExport('stl_has_error')
            console.log('ladder_has_error:', ladderErr, ', stl_has_error:', stlErr)
        } else {
            console.log('Input LADDER JSON:')
            console.log(ladder10)
            
            // Get generated STL
            await runtime.callExport('ladder_output_to_stream')
            const stl10 = await runtime.readStream()
            console.log('\nGenerated STL:')
            console.log(stl10)
            
            // Get generated PLCASM
            await runtime.callExport('stl_output_to_stream')
            const plcasm10 = await runtime.readStream()
            console.log('Generated PLCASM:')
            console.log(plcasm10)
            
            console.log('✓ Full pipeline completed successfully!')
        }

        console.log('\n' + '='.repeat(60))
        console.log('All tests completed!')
        console.log('='.repeat(60))

    } finally {
         await runtime.terminate()
    }
}

runTests()
    .catch(console.error)
    .finally(() => process.exit())
