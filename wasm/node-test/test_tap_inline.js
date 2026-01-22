// test_tap_inline.js - Test TAP and coils inside OR branches
// These should be emitted INLINE within the branch blocks, not as separate networks
import VovkPLC from '../dist/VovkPLC.js';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm');

const expectedOutput = `// Generated from Network IR by VovkPLCRuntime

A(
    A(
        A button1
        O Y0.1
    )
    A M0.0
    TON T0, T#10s
    O(
        A(
            A button1
            O Y0.3
        )
        A M0.2
        TON T1, T#3s
    )
    O(
        A Y0.2
        R Y0.1
        TAP
    )
)
AN M0.1
= Y0.1
= Y0.4
`;

const main = async () => {
    const plc = new VovkPLC(wasmPath);
    await plc.initialize(wasmPath, false, true);
    
    // Test input with coil and TAP inside OR branch
    // Branch 3: Y0.2 -> R Y0.1 -> TAP
    // This should emit:  O( A Y0.2 / R Y0.1 / TAP )  NOT separate networks!
    const ladderJson = {
        networks: [{
            segments: [{
                elements: [[
                    {
                        type: 'or',
                        branches: [
                            // Branch 1: (button1 OR Y0.1) AND M0.0 AND TON T0
                            [
                                {
                                    type: 'or',
                                    branches: [
                                        [{type: 'contact_no', address: 'button1'}],
                                        [{type: 'contact_no', address: 'Y0.1'}]
                                    ]
                                },
                                {type: 'contact_no', address: 'M0.0'},
                                {type: 'timer_on', address: 'T0', preset: 'T#10s'}
                            ],
                            // Branch 2: (button1 OR Y0.3) AND M0.2 AND TON T1
                            [
                                {
                                    type: 'or',
                                    branches: [
                                        [{type: 'contact_no', address: 'button1'}],
                                        [{type: 'contact_no', address: 'Y0.3'}]
                                    ]
                                },
                                {type: 'contact_no', address: 'M0.2'},
                                {type: 'timer_on', address: 'T1', preset: 'T#3s'}
                            ],
                            // Branch 3: Y0.2 -> R Y0.1 -> TAP (should be INLINE!)
                            [
                                {type: 'contact_no', address: 'Y0.2'},
                                {type: 'coil_rset', address: 'Y0.1'},
                                {type: 'tap'}
                            ]
                        ]
                    },
                    {type: 'contact_nc', address: 'M0.1'},
                    {type: 'coil', address: 'Y0.1'},
                    {type: 'coil', address: 'Y0.4'}
                ]]
            }]
        }]
    };
    
    console.log("=== Test: TAP and Coil inside OR branches (should be INLINE) ===\n");
    
    const result = plc.compileLadder(ladderJson);
    const output = result.output.trim();
    const expected = expectedOutput.trim();
    
    console.log("Output:");
    console.log(output);
    console.log();
    
    if (output === expected) {
        console.log("✓ TEST PASSED: Output matches expected!");
        console.log("  - Coil (R Y0.1) is INLINE within O( ... ) block");
        console.log("  - TAP is INLINE within O( ... ) block");
    } else {
        console.log("✗ TEST FAILED: Output does not match expected!");
        console.log("\nExpected:");
        console.log(expected);
        process.exit(1);
    }
};

main().catch(err => {
    console.error('Error:', err);
    process.exit(1);
});
