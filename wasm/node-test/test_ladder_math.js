// @ts-check
// Test file for ladder diagram math operations in Network IR
// Tests math_add, math_sub, math_mul, math_div, move, compare operations
// Output is proper Siemens STL format:
//   L operand   - Load to accumulator
//   +I/-D/*R/R  - Operations with second operand (I=i16, D=i32, R=f32)
//   T dest      - Transfer accumulator to memory

import VovkPLC from '../dist/VovkPLC.js';
import path from 'path';
import {fileURLToPath} from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm');

const TESTS = [];
let totalTests = 0;
let passedTests = 0;

function test(name, fn) {
    TESTS.push({ name, fn });
}

function expect(actual) {
    return {
        toBe(expected) {
            if (actual !== expected) {
                throw new Error(`Expected ${expected} but got ${actual}`);
            }
        },
        toContain(substring) {
            if (typeof actual !== 'string' || !actual.includes(substring)) {
                throw new Error(`Expected output to contain "${substring}"\nActual output:\n${actual}`);
            }
        },
        toMatch(regex) {
            if (typeof actual !== 'string' || !regex.test(actual)) {
                throw new Error(`Expected "${actual}" to match ${regex}`);
            }
        },
        toBeTruthy() {
            if (!actual) {
                throw new Error(`Expected ${actual} to be truthy`);
            }
        }
    };
}

// Helper to compile ladder JSON to STL
async function compileLadder(runtime, ladderJson) {
    // Write to stream
    for (let i = 0; i < ladderJson.length; i++) {
        await runtime.callExport('streamIn', ladderJson.charCodeAt(i));
    }
    await runtime.callExport('streamIn', 0); // Null terminator
    
    // Load and compile
    await runtime.callExport('ladder_load_from_stream');
    const success = await runtime.callExport('ladder_compile');
    
    if (!success) {
        return { success: false, stl: '' };
    }
    
    // Get output
    await runtime.callExport('ladder_output_to_stream');
    const stl = await runtime.readStream();
    
    return { success: true, stl };
}

async function runTests() {
    const runtime = await VovkPLC.createWorker(wasmPath);

    console.log('\n========================================');
    console.log('  Ladder Math Operations Tests');
    console.log('========================================\n');

    for (const { name, fn } of TESTS) {
        totalTests++;
        try {
            await fn(runtime);
            passedTests++;
            console.log(`  ✓ ${name}`);
        } catch (err) {
            console.log(`  ✗ ${name}`);
            console.log(`    Error: ${err.message}`);
        }
    }

    console.log('\n----------------------------------------');
    console.log(`  Results: ${passedTests}/${totalTests} tests passed`);
    console.log('----------------------------------------\n');

    await runtime.terminate();
    return passedTests === totalTests ? 0 : 1;
}

// ============ Basic Math Operation Tests ============

test('math_add with i16 data type', async (runtime) => {
    const ladder = JSON.stringify({
        rungs: [{
            elements: [{
                type: "math_add",
                dataType: "i16",
                in1: "MW0",
                in2: "MW2",
                out: "MW4"
            }]
        }]
    });
    
    const result = await compileLadder(runtime, ladder);
    expect(result.success).toBeTruthy();
    expect(result.stl).toContain('L MW0');
    expect(result.stl).toContain('+I MW2');
    expect(result.stl).toContain('T MW4');
});

test('math_sub with i32 data type', async (runtime) => {
    const ladder = JSON.stringify({
        rungs: [{
            elements: [{
                type: "math_sub",
                dataType: "i32",
                in1: "MD0",
                in2: "MD4",
                out: "MD8"
            }]
        }]
    });
    
    const result = await compileLadder(runtime, ladder);
    expect(result.success).toBeTruthy();
    expect(result.stl).toContain('L MD0');
    expect(result.stl).toContain('-D MD4');
    expect(result.stl).toContain('T MD8');
});

test('math_mul with f32 data type', async (runtime) => {
    const ladder = JSON.stringify({
        rungs: [{
            elements: [{
                type: "math_mul",
                dataType: "f32",
                in1: "MR0",
                in2: "MR4",
                out: "MR8"
            }]
        }]
    });
    
    const result = await compileLadder(runtime, ladder);
    expect(result.success).toBeTruthy();
    expect(result.stl).toContain('L MR0');
    expect(result.stl).toContain('*R MR4');
    expect(result.stl).toContain('T MR8');
});

test('math_div with i16 data type', async (runtime) => {
    const ladder = JSON.stringify({
        rungs: [{
            elements: [{
                type: "math_div",
                dataType: "i16",
                in1: "MW0",
                in2: "MW2",
                out: "MW4"
            }]
        }]
    });
    
    const result = await compileLadder(runtime, ladder);
    expect(result.success).toBeTruthy();
    expect(result.stl).toContain('L MW0');
    expect(result.stl).toContain('/I MW2');
    expect(result.stl).toContain('T MW4');
});

// ============ Constant Operand Tests ============

test('math_add with constant operand', async (runtime) => {
    const ladder = JSON.stringify({
        rungs: [{
            elements: [{
                type: "math_add",
                dataType: "i16",
                in1: "MW0",
                in2: "#100",
                out: "MW4"
            }]
        }]
    });
    
    const result = await compileLadder(runtime, ladder);
    expect(result.success).toBeTruthy();
    expect(result.stl).toContain('L MW0');
    expect(result.stl).toContain('+I #100');
    expect(result.stl).toContain('T MW4');
});

test('math_mul with constant first operand', async (runtime) => {
    const ladder = JSON.stringify({
        rungs: [{
            elements: [{
                type: "math_mul",
                dataType: "i32",
                in1: "#50",
                in2: "MD0",
                out: "MD4"
            }]
        }]
    });
    
    const result = await compileLadder(runtime, ladder);
    expect(result.success).toBeTruthy();
    expect(result.stl).toContain('L #50');
    expect(result.stl).toContain('*D MD0');
    expect(result.stl).toContain('T MD4');
});

// ============ Move Operation Tests ============

test('move operation with i16', async (runtime) => {
    const ladder = JSON.stringify({
        rungs: [{
            elements: [{
                type: "move",
                dataType: "i16",
                in1: "MW0",
                out: "MW4"
            }]
        }]
    });
    
    const result = await compileLadder(runtime, ladder);
    expect(result.success).toBeTruthy();
    expect(result.stl).toContain('L MW0');
    expect(result.stl).toContain('T MW4');
});

test('move with constant value', async (runtime) => {
    const ladder = JSON.stringify({
        rungs: [{
            elements: [{
                type: "move",
                dataType: "i32",
                in1: "#12345",
                out: "MD0"
            }]
        }]
    });
    
    const result = await compileLadder(runtime, ladder);
    expect(result.success).toBeTruthy();
    expect(result.stl).toContain('L #12345');
    expect(result.stl).toContain('T MD0');
});

// ============ Compare Operation Tests ============

test('compare_eq operation', async (runtime) => {
    const ladder = JSON.stringify({
        rungs: [{
            elements: [{
                type: "compare_eq",
                dataType: "i16",
                in1: "MW0",
                in2: "MW2",
                out: "M0.0"
            }]
        }]
    });
    
    const result = await compileLadder(runtime, ladder);
    expect(result.success).toBeTruthy();
    expect(result.stl).toContain('L MW0');
    expect(result.stl).toContain('==I MW2');
});

test('compare_gt operation', async (runtime) => {
    const ladder = JSON.stringify({
        rungs: [{
            elements: [{
                type: "compare_gt",
                dataType: "i32",
                in1: "MD0",
                in2: "#100",
                out: "M0.0"
            }]
        }]
    });
    
    const result = await compileLadder(runtime, ladder);
    expect(result.success).toBeTruthy();
    expect(result.stl).toContain('L MD0');
    expect(result.stl).toContain('>D #100');
});

test('compare_lt operation', async (runtime) => {
    const ladder = JSON.stringify({
        rungs: [{
            elements: [{
                type: "compare_lt",
                dataType: "f32",
                in1: "MR0",
                in2: "MR4",
                out: "M0.0"
            }]
        }]
    });
    
    const result = await compileLadder(runtime, ladder);
    expect(result.success).toBeTruthy();
    expect(result.stl).toContain('L MR0');
    expect(result.stl).toContain('<R MR4');
});

// ============ Type Inference Tests ============

test('type inference from MW address prefix', async (runtime) => {
    const ladder = JSON.stringify({
        rungs: [{
            elements: [{
                type: "math_add",
                in1: "MW0",
                in2: "MW2",
                out: "MW4"
            }]
        }]
    });
    
    const result = await compileLadder(runtime, ladder);
    expect(result.success).toBeTruthy();
    // Should infer i16 from MW prefix - uses +I
    expect(result.stl).toContain('+I');
    expect(result.stl).toContain('MW');
});

test('type inference from MD address prefix', async (runtime) => {
    const ladder = JSON.stringify({
        rungs: [{
            elements: [{
                type: "math_mul",
                in1: "MD0",
                in2: "MD4",
                out: "MD8"
            }]
        }]
    });
    
    const result = await compileLadder(runtime, ladder);
    expect(result.success).toBeTruthy();
    // Should infer i32 from MD prefix - uses *D
    expect(result.stl).toContain('*D');
    expect(result.stl).toContain('MD');
});

test('type inference from MR address prefix', async (runtime) => {
    const ladder = JSON.stringify({
        rungs: [{
            elements: [{
                type: "math_div",
                in1: "MR0",
                in2: "MR4",
                out: "MR8"
            }]
        }]
    });
    
    const result = await compileLadder(runtime, ladder);
    expect(result.success).toBeTruthy();
    // Should infer f32 from MR prefix - uses /R
    expect(result.stl).toContain('/R');
    expect(result.stl).toContain('MR');
});

// ============ Mixed Logic and Math Tests ============

test('contact + math_add + coil', async (runtime) => {
    const ladder = JSON.stringify({
        rungs: [{
            elements: [
                { type: "contact", address: "I0.0" },
                { type: "math_add", dataType: "i16", in1: "MW0", in2: "MW2", out: "MW4" },
                { type: "coil", address: "Q0.0" }
            ]
        }]
    });
    
    const result = await compileLadder(runtime, ladder);
    expect(result.success).toBeTruthy();
    // Should have contact logic, math operation, and coil
    expect(result.stl).toContain('A I0.0');  // Contact
    expect(result.stl).toContain('L MW0');   // Math load
    expect(result.stl).toContain('+I MW2');  // Math add
    expect(result.stl).toContain('T MW4'); // Math store
    expect(result.stl).toContain('= Q0.0');  // Coil
});

// ============ All Compare Operations ============

test('compare_neq operation', async (runtime) => {
    const ladder = JSON.stringify({
        rungs: [{
            elements: [{
                type: "compare_neq",
                dataType: "i16",
                in1: "MW0",
                in2: "MW2",
                out: "M0.0"
            }]
        }]
    });
    
    const result = await compileLadder(runtime, ladder);
    expect(result.success).toBeTruthy();
    expect(result.stl).toContain('<>I');
});

test('compare_gte operation', async (runtime) => {
    const ladder = JSON.stringify({
        rungs: [{
            elements: [{
                type: "compare_gte",
                dataType: "i32",
                in1: "MD0",
                in2: "MD4",
                out: "M0.0"
            }]
        }]
    });
    
    const result = await compileLadder(runtime, ladder);
    expect(result.success).toBeTruthy();
    expect(result.stl).toContain('>=D');
});

test('compare_lte operation', async (runtime) => {
    const ladder = JSON.stringify({
        rungs: [{
            elements: [{
                type: "compare_lte",
                dataType: "f32",
                in1: "MR0",
                in2: "MR4",
                out: "M0.0"
            }]
        }]
    });
    
    const result = await compileLadder(runtime, ladder);
    expect(result.success).toBeTruthy();
    expect(result.stl).toContain('<=R');
});

// Run the tests
runTests().then(code => process.exit(code));
