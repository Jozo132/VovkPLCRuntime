// test_branch_stack.js - Tests for BR (Branch Stack) instructions
// Tests: br.save, br.read, br.drop, br.clr

import path from 'path';
import { fileURLToPath } from 'url';
import VovkPLC from '../dist/VovkPLC.js';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm');

console.log(`Loading WASM from: ${wasmPath}`);

const plc = await VovkPLC.createWorker(wasmPath);

console.log('::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::');
console.log('Runtime initialized. Testing Branch Stack (BR) instructions...');
console.log('');

let testsPassed = 0;
let testsFailed = 0;

async function test(name, fn) {
    try {
        await fn();
        console.log(`✓ ${name}`);
        testsPassed++;
    } catch (e) {
        console.log(`✗ ${name}`);
        console.log(`  Error: ${e.message}`);
        testsFailed++;
    }
}

async function compileAndVerify(code, expectedBytecodeIncludes = []) {
    // Download assembly
    await plc.downloadAssembly(code);
    
    // Compile
    const compileError = await plc.callExport('compileAssembly', false);
    if (compileError) {
        throw new Error(`Compilation failed`);
    }
    
    // Extract program to verify bytecode
    const result = await plc.extractProgram();
    const bytecode = result.output;
    
    // Verify expected bytecode patterns are present
    for (const expected of expectedBytecodeIncludes) {
        if (!bytecode.includes(expected)) {
            throw new Error(`Expected bytecode to include ${expected}, got: ${bytecode}`);
        }
    }
    
    // Load and run
    const loadError = await plc.callExport('loadCompiledProgram');
    if (loadError) {
        throw new Error('Load failed');
    }
    
    const runError = await plc.callExport('run');
    if (runError) {
        throw new Error(`Run failed with error code: ${runError}`);
    }
    
    return bytecode;
}

// Run all tests
async function runTests() {
    // Test 1: br.save compiles to 0x90
    await test('br.save compiles to opcode 0x90', async () => {
        const bytecode = await compileAndVerify(`
            u8.const 1
            br.save
            exit
        `, ['90']); // BR_SAVE = 0x90
    });

    // Test 2: br.read compiles to 0x91
    await test('br.read compiles to opcode 0x91', async () => {
        const bytecode = await compileAndVerify(`
            br.read
            exit
        `, ['91']); // BR_READ = 0x91
    });

    // Test 3: br.drop compiles to 0x92
    await test('br.drop compiles to opcode 0x92', async () => {
        const bytecode = await compileAndVerify(`
            br.drop
            exit
        `, ['92']); // BR_DROP = 0x92
    });

    // Test 4: br.clr compiles to 0x93
    await test('br.clr compiles to opcode 0x93', async () => {
        const bytecode = await compileAndVerify(`
            br.clr
            exit
        `, ['93']); // BR_CLR = 0x93
    });

    // Test 5: Full sequence compiles and runs
    await test('Full BR instruction sequence compiles and executes', async () => {
        const bytecode = await compileAndVerify(`
            u8.const 1
            br.save
            br.read
            br.drop
            br.clr
            exit
        `, ['90', '91', '92', '93']); // All four opcodes present
    });

    // Test 6: Nested saves/reads pattern
    await test('Nested branch save/read pattern executes', async () => {
        await compileAndVerify(`
            u8.const 1
            br.save
            u8.const 1
            br.save
            u8.const 0
            br.save
            br.read
            br.drop
            br.read
            br.drop
            br.read
            br.drop
            exit
        `, ['90', '91', '92']);
    });

    // Test 7: Multiple drops (underflow safety test)
    await test('Multiple br.drop operations execute safely', async () => {
        await compileAndVerify(`
            u8.const 1
            br.save
            br.drop
            br.drop
            br.drop
            br.read
            exit
        `, ['92', '91']);
    });

    // Test 8: br.clr clears stack properly
    await test('br.clr after multiple saves executes', async () => {
        await compileAndVerify(`
            u8.const 1
            br.save
            u8.const 1
            br.save
            br.clr
            br.read
            exit
        `, ['90', '93', '91']);
    });

    console.log('');
    console.log('========================================');
    console.log(`Tests passed: ${testsPassed}`);
    console.log(`Tests failed: ${testsFailed}`);
    console.log('========================================');

    await plc.terminate();
    
    if (testsFailed > 0) {
        process.exit(1);
    }
}

runTests();