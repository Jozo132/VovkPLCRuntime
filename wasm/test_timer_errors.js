import VovkPLC from './dist/VovkPLC.js';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const wasmPath = path.resolve(__dirname, './dist/VovkPLC.wasm');

console.log('Testing timer instruction error messages...\n');

async function testTimerError(assembly, expectedErrorSnippet, testName) {
    const runtime = await VovkPLC.createWorker(wasmPath);
    
    // Capture stdout
    let capturedOutput = '';
    const originalWrite = process.stdout.write.bind(process.stdout);
    process.stdout.write = (chunk, encoding, callback) => {
        capturedOutput += chunk.toString();
        return originalWrite(chunk, encoding, callback);
    };
    
    try {
        await runtime.downloadAssembly(assembly);
        const compileError = await runtime.callExport('compileAssembly', false);
        const output = await runtime.readStream();
        
        // Restore stdout
        process.stdout.write = originalWrite;
        
        if (compileError === 0) {
            console.log(`❌ FAIL [${testName}]: Expected error but compilation succeeded`);
            console.log(`   Assembly: ${assembly.trim()}`);
            await runtime.terminate();
            return false;
        }
        
        const fullOutput = (output || '') + capturedOutput;
        if (fullOutput.includes(expectedErrorSnippet)) {
            console.log(`✓ PASS [${testName}]: Got expected error`);
            console.log(`   Assembly: ${assembly.trim()}`);
            const errorLine = fullOutput.split('\n').find(line => line.includes('Error'));
            if (errorLine) console.log(`   Error: ${errorLine.trim()}`);
            await runtime.terminate();
            return true;
        } else {
            console.log(`❌ FAIL [${testName}]: Error message doesn't contain expected snippet`);
            console.log(`   Assembly: ${assembly.trim()}`);
            console.log(`   Expected snippet: "${expectedErrorSnippet}"`);
            console.log(`   Full output: ${fullOutput}`);
            await runtime.terminate();
            return false;
        }
    } catch (e) {
        process.stdout.write = originalWrite;
        console.log(`❌ ERROR [${testName}]: ${e.message}`);
        await runtime.terminate();
        return false;
    }
}

async function testValidTimer(assembly, testName) {
    const runtime = await VovkPLC.createWorker(wasmPath);
    try {
        await runtime.downloadAssembly(assembly);
        const compileError = await runtime.callExport('compileAssembly', false);
        await runtime.readStream(); // Clear stream
        
        if (compileError === 0) {
            console.log(`✓ PASS [${testName}]: Compiled successfully`);
            console.log(`   Assembly: ${assembly.trim()}`);
            await runtime.terminate();
            return true;
        } else {
            const output = await runtime.readStream();
            console.log(`❌ FAIL [${testName}]: Should compile successfully`);
            console.log(`   Assembly: ${assembly.trim()}`);
            console.log(`   Error: ${output}`);
            await runtime.terminate();
            return false;
        }
    } catch (e) {
        console.log(`❌ ERROR [${testName}]: ${e.message}`);
        await runtime.terminate();
        return false;
    }
}

async function runTests() {
    let passed = 0;
    let total = 0;
    
    console.log('='.repeat(70));
    console.log('Timer Instruction Error Message Tests');
    console.log('='.repeat(70) + '\n');
    
    // Test 1: Just "ton" with no arguments
    total++;
    if (await testTimerError('ton', 'requires a second argument', 'No arguments')) passed++;
    console.log();
    
    // Test 2: "ton" with only one argument
    total++;
    if (await testTimerError('ton S0', 'expected preset time value', 'Missing second argument')) passed++;
    console.log();
    
    // Test 3: "ton" with invalid first argument
    total++;
    if (await testTimerError('ton invalid #100', 'expected timer address', 'Invalid first argument')) passed++;
    console.log();
    
    // Test 4: "ton" with invalid second argument (no # and not an address)
    total++;
    if (await testTimerError('ton S0 invalid', 'expected preset time value', 'Invalid second argument')) passed++;
    console.log();
    
    // Test 5: "ton" with # but invalid value
    total++;
    if (await testTimerError('ton S0 #abc', 'expected integer value after #', 'Invalid value after #')) passed++;
    console.log();
    
    // Test 6: Valid ton with constant - should succeed
    total++;
    if (await testValidTimer('ton S0 #100', 'Valid TON constant')) passed++;
    console.log();
    
    // Test 7: Valid ton with memory - should succeed  
    total++;
    if (await testValidTimer('ton S0 M0', 'Valid TON memory')) passed++;
    console.log();
    
    // Test 8: Valid tof with constant
    total++;
    if (await testValidTimer('tof S1 #200', 'Valid TOF constant')) passed++;
    console.log();
    
    // Test 9: Valid tp with constant
    total++;
    if (await testValidTimer('tp S2 #50', 'Valid TP constant')) passed++;
    console.log();
    
    console.log('='.repeat(70));
    console.log(`Results: ${passed}/${total} tests passed`);
    console.log('='.repeat(70));
    
    if (passed === total) {
        console.log('\n✓ All timer error message tests passed!');
        process.exit(0);
    } else {
        console.log(`\n❌ ${total - passed} test(s) failed`);
        process.exit(1);
    }
}

runTests().catch(e => {
    console.error('Fatal error:', e);
    process.exit(1);
});
