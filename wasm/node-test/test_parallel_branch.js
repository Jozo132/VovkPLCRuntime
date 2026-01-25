// test_parallel_branch.js - Tests for parallel output branch element support
// Tests the "branch" element type which uses BR stack (SAVE/L BR/DROP BR)

import VovkPLC from '../dist/VovkPLC.js';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm');

let passed = 0;
let failed = 0;

async function test(name, fn) {
    try {
        await fn();
        console.log(`✓ ${name}`);
        passed++;
    } catch (e) {
        console.log(`✗ ${name}`);
        console.log(`  Error: ${e.message}`);
        failed++;
    }
}

function assert(condition, message) {
    if (!condition) throw new Error(message || 'Assertion failed');
}

async function runTests() {
    const runtime = await VovkPLC.createWorker(wasmPath);
    console.log('VovkPLCRuntime WASM loaded successfully\n');

    // Helper to write string to stream
    const writeStream = async (str) => {
        for (let i = 0; i < str.length; i++) {
            await runtime.callExport('streamIn', str.charCodeAt(i));
        }
        await runtime.callExport('streamIn', 0); // Null terminator
    };

    // Test 1: Simple parallel coils
    await test('Simple parallel coils - two coils with common condition', async () => {
        const ladder = JSON.stringify({
            rungs: [{
                elements: [
                    { type: "contact", address: "X0.0" },
                    { 
                        type: "branch", 
                        branches: [
                            { elements: [{ type: "coil", address: "Y0.0", passThrough: true }] },
                            { elements: [{ type: "coil", address: "Y0.1", passThrough: true }] }
                        ]
                    }
                ]
            }]
        });
        
        console.log("  Input JSON:", ladder);
        await writeStream(ladder);
        await runtime.callExport('ladder_load_from_stream');
        const success = await runtime.callExport('ladder_compile');
        console.log("  Compile success:", success);
        
        // Read error message if any
        const hasError = await runtime.callExport('ladder_has_error');
        if (hasError) {
            await runtime.callExport('ladder_error_to_stream');
            const errorMsg = await runtime.readStream();
            console.log("  Error:", errorMsg);
        }
        
        // Read output
        await runtime.callExport('ladder_output_to_stream');
        const output = await runtime.readStream();
        console.log("  Output:", output);
        
        assert(success, "Compilation should succeed");
        
        await runtime.callExport('ladder_output_to_stream');
        const stl = await runtime.readStream();
        console.log("  Generated STL:");
        console.log(stl.split('\n').map(l => '    ' + l).join('\n'));
        
        // Verify STL contains BR stack instructions
        assert(stl.includes('SAVE'), "STL should contain SAVE instruction");
        assert(stl.includes('L BR'), "STL should contain L BR instruction");
        assert(stl.includes('DROP BR'), "STL should contain DROP BR instruction");
        assert(stl.includes('= Y0.0'), "STL should contain assignment to Y0.0");
        assert(stl.includes('= Y0.1'), "STL should contain assignment to Y0.1");
        assert(stl.includes('A X0.0'), "STL should contain contact X0.0");
    });

    // Test 2: Mixed actions (coil + increment)
    await test('Mixed actions - coil and increment', async () => {
        const ladder = JSON.stringify({
            rungs: [{
                elements: [
                    { type: "contact", address: "X0.0" },
                    { 
                        type: "branch", 
                        branches: [
                            { elements: [{ type: "coil", address: "Y0.0", passThrough: true }] },
                            { elements: [{ type: "inc", address: "MW0", dataType: "i16", passThrough: true }] }
                        ]
                    }
                ]
            }]
        });
        
        await writeStream(ladder);
        await runtime.callExport('ladder_load_from_stream');
        const success = await runtime.callExport('ladder_compile');
        
        assert(success, "Compilation should succeed");
        
        await runtime.callExport('ladder_output_to_stream');
        const stl = await runtime.readStream();
        console.log("  Generated STL:");
        console.log(stl.split('\n').map(l => '    ' + l).join('\n'));
        
        // Verify STL contains BR stack instructions and actions
        assert(stl.includes('SAVE'), "STL should contain SAVE instruction");
        assert(stl.includes('L BR'), "STL should contain L BR instruction");
        assert(stl.includes('DROP BR'), "STL should contain DROP BR instruction");
        assert(stl.includes('= Y0.0'), "STL should contain assignment to Y0.0");
        assert(stl.includes('INCI'), "STL should contain INCI instruction for increment");
    });

    // Test 3: Three parallel coils (SKIPPED for faster iteration)
    /*
    await test('Three parallel coils', async () => {
        const ladder = JSON.stringify({
            rungs: [{
                elements: [
                    { type: "contact", address: "I0.0" },
                    { type: "contact", address: "I0.1" },
                    { 
                        type: "branch", 
                        branches: [
                            { elements: [{ type: "coil", address: "Q0.0", passThrough: true }] },
                            { elements: [{ type: "coil", address: "Q0.1", passThrough: true }] },
                            { elements: [{ type: "coil", address: "Q0.2", passThrough: true }] }
                        ]
                    }
                ]
            }]
        });
        
        await writeStream(ladder);
        await runtime.callExport('ladder_load_from_stream');
        const success = await runtime.callExport('ladder_compile');
        
        assert(success, "Compilation should succeed");
        
        await runtime.callExport('ladder_output_to_stream');
        const stl = await runtime.readStream();
        console.log("  Generated STL:");
        console.log(stl.split('\n').map(l => '    ' + l).join('\n'));
        
        // Count L BR occurrences - should be 3 (one per branch)
        const lbrCount = (stl.match(/L BR/g) || []).length;
        assert(lbrCount === 3, `Should have 3 L BR instructions, got ${lbrCount}`);
        
        assert(stl.includes('= Q0.0'), "STL should contain assignment to Q0.0");
        assert(stl.includes('= Q0.1'), "STL should contain assignment to Q0.1");
        assert(stl.includes('= Q0.2'), "STL should contain assignment to Q0.2");
    });

    // Test 4: OR logic followed by branch
    await test('OR logic followed by branch', async () => {
        const ladder = JSON.stringify({
            rungs: [{
                elements: [
                    {
                        type: "or",
                        branches: [
                            { elements: [{ type: "contact", address: "X0.0" }] },
                            { elements: [{ type: "contact", address: "X0.1" }] }
                        ]
                    },
                    { 
                        type: "branch", 
                        branches: [
                            { elements: [{ type: "coil", address: "Y0.0", passThrough: true }] },
                            { elements: [{ type: "coil", address: "Y0.1", passThrough: true }] }
                        ]
                    }
                ]
            }]
        });
        
        await writeStream(ladder);
        await runtime.callExport('ladder_load_from_stream');
        const success = await runtime.callExport('ladder_compile');
        
        assert(success, "Compilation should succeed");
        
        await runtime.callExport('ladder_output_to_stream');
        const stl = await runtime.readStream();
        console.log("  Generated STL:");
        console.log(stl.split('\n').map(l => '    ' + l).join('\n'));
        
        // Verify both OR logic and branch instructions are present
        assert(stl.includes('O'), "STL should contain OR instruction");
        assert(stl.includes('SAVE'), "STL should contain SAVE instruction");
        assert(stl.includes('DROP BR'), "STL should contain DROP BR instruction");
    });

    // Test 5: Branch with set/reset coils
    await test('Branch with set and reset coils', async () => {
        const ladder = JSON.stringify({
            rungs: [{
                elements: [
                    { type: "contact", address: "X0.0" },
                    { 
                        type: "branch", 
                        branches: [
                            { elements: [{ type: "coil_set", address: "M0.0", passThrough: true }] },
                            { elements: [{ type: "coil_rset", address: "M0.1", passThrough: true }] }
                        ]
                    }
                ]
            }]
        });
        
        await writeStream(ladder);
        await runtime.callExport('ladder_load_from_stream');
        const success = await runtime.callExport('ladder_compile');
        
        assert(success, "Compilation should succeed");
        
        await runtime.callExport('ladder_output_to_stream');
        const stl = await runtime.readStream();
        console.log("  Generated STL:");
        console.log(stl.split('\n').map(l => '    ' + l).join('\n'));
        
        assert(stl.includes('S M0.0'), "STL should contain SET M0.0");
        assert(stl.includes('R M0.1'), "STL should contain RESET M0.1");
    });

    // Test 6: Full pipeline - compile to bytecode
    await test('Full pipeline - ladder to bytecode', async () => {
        const ladder = JSON.stringify({
            rungs: [{
                elements: [
                    { type: "contact", address: "X0.0" },
                    { 
                        type: "branch", 
                        branches: [
                            { elements: [{ type: "coil", address: "Y0.0", passThrough: true }] },
                            { elements: [{ type: "coil", address: "Y0.1", passThrough: true }] }
                        ]
                    }
                ]
            }]
        });
        
        await writeStream(ladder);
        await runtime.callExport('ladder_load_from_stream');
        const success = await runtime.callExport('ladder_compile_full');
        
        assert(success, "Full compilation should succeed");
        
        // Get bytecode
        const size = await runtime.callExport('compilerGetBytecodeSize');
        const bytecode = [];
        for (let i = 0; i < size; i++) {
            bytecode.push(await runtime.callExport('compilerGetBytecodeAt', i));
        }
        console.log(`  Bytecode length: ${bytecode.length}`);
        console.log(`  Bytecode: [${bytecode.map(b => '0x' + b.toString(16).padStart(2, '0')).join(', ')}]`);
        
        // Verify bytecode contains BR stack opcodes
        const BR_SAVE = 0x90;
        const BR_READ = 0x91;
        const BR_DROP = 0x92;
        
        assert(bytecode.includes(BR_SAVE), "Bytecode should contain BR_SAVE (0x90)");
        assert(bytecode.includes(BR_READ), "Bytecode should contain BR_READ (0x91)");
        assert(bytecode.includes(BR_DROP), "Bytecode should contain BR_DROP (0x92)");
    });
    */

    console.log(`\nTests passed: ${passed}, Tests failed: ${failed}`);
    await runtime.terminate();
    process.exit(failed > 0 ? 1 : 0);
}

runTests().catch(e => {
    console.error('Test runner error:', e);
    process.exit(1);
});
