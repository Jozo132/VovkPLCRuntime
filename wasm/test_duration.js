import VovkPLC from './dist/VovkPLC.js';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const wasmPath = path.resolve(__dirname, './dist/VovkPLC.wasm');

console.log('Testing duration datatype for timers...\n');

async function testDuration(assembly, expectedMillis, testName) {
    const runtime = await VovkPLC.createWorker(wasmPath);
    try {
        await runtime.downloadAssembly(assembly);
        const compileError = await runtime.callExport('compileAssembly', false);
        
        if (compileError !== 0) {
            const output = await runtime.readStream();
            console.log(`❌ FAIL [${testName}]: Compilation failed`);
            console.log(`   Assembly: ${assembly.trim()}`);
            console.log(`   Error: ${output}`);
            await runtime.terminate();
            return false;
        }
        
        // Extract the compiled bytecode
        const extracted = await runtime.extractProgram();
        if (!extracted.output || extracted.output.length < 14) {
            console.log(`❌ FAIL [${testName}]: Bytecode too short`);
            console.log(`   Extracted output: "${extracted.output}"`);
            console.log(`   Length: ${extracted.output ? extracted.output.length : 0}`);
            await runtime.terminate();
            return false;
        }
        
        // Parse hex string: "3000C0000000C8FF" (pairs of hex digits = bytes)
        // For TON_CONST: opcode(1) + address(2) + preset_time(4) + exit(1) = 8 bytes = 16 hex chars
        // Preset time is at bytes 3-6 (little-endian u32)
        const hexStr = extracted.output.trim();
        const bytes = [];
        for (let i = 0; i < hexStr.length; i += 2) {
            bytes.push(parseInt(hexStr.substr(i, 2), 16));
        }
        
        if (bytes.length < 7) {
            console.log(`❌ FAIL [${testName}]: Bytecode too short after parsing`);
            console.log(`   Raw output: "${extracted.output}"`);
            console.log(`   Bytes parsed: ${bytes.length}`);
            await runtime.terminate();
            return false;
        }
        
        // Read u32 little-endian from bytes 3-6
        // Note: bytecode is displayed with actual byte order, so for little-endian u32,
        // we read bytes[3] as LSB and bytes[6] as MSB
        const actualMillis = (bytes[6] | (bytes[5] << 8) | (bytes[4] << 16) | (bytes[3] << 24)) >>> 0;
        
        if (actualMillis === expectedMillis) {
            console.log(`✓ PASS [${testName}]: ${expectedMillis}ms`);
            console.log(`   Assembly: ${assembly.trim()}`);
            await runtime.terminate();
            return true;
        } else {
            console.log(`❌ FAIL [${testName}]: Value mismatch`);
            console.log(`   Assembly: ${assembly.trim()}`);
            console.log(`   Expected: ${expectedMillis}ms`);
            console.log(`   Got: ${actualMillis}ms`);
            console.log(`   Bytes [3-6]: ${bytes[3].toString(16)} ${bytes[4].toString(16)} ${bytes[5].toString(16)} ${bytes[6].toString(16)}`);
            await runtime.terminate();
            return false;
        }
    } catch (e) {
        console.log(`❌ ERROR [${testName}]: ${e.message}`);
        console.log(e.stack);
        await runtime.terminate();
        return false;
    }
}

async function runTests() {
    let passed = 0;
    let total = 0;
    
    console.log('='.repeat(70));
    console.log('Duration Datatype Tests');
    console.log('='.repeat(70) + '\n');
    
    // Test 1: Simple milliseconds
    total++;
    if (await testDuration('ton S0 T#200ms', 200, 'T#200ms')) passed++;
    console.log();
    
    // Test 2: Simple seconds
    total++;
    if (await testDuration('ton S0 T#5s', 5000, 'T#5s')) passed++;
    console.log();
    
    // Test 3: Simple minutes
    total++;
    if (await testDuration('ton S0 T#2m', 120000, 'T#2m')) passed++;
    console.log();
    
    // Test 4: Simple hours
    total++;
    if (await testDuration('ton S0 T#1h', 3600000, 'T#1h')) passed++;
    console.log();
    
    // Test 5: Complex duration - minutes and seconds
    total++;
    if (await testDuration('ton S0 T#4m10s', 250000, 'T#4m10s')) passed++;
    console.log();
    
    // Test 6: Complex duration - minutes, seconds, and milliseconds
    total++;
    if (await testDuration('ton S0 T#4m10s500ms', 250500, 'T#4m10s500ms')) passed++;
    console.log();
    
    // Test 7: Complex duration - hours, minutes, seconds
    total++;
    if (await testDuration('ton S0 T#1h30m15s', 5415000, 'T#1h30m15s')) passed++;
    console.log();
    
    // Test 8: Old format still works - #500
    total++;
    if (await testDuration('ton S0 #500', 500, 'Legacy #500')) passed++;
    console.log();
    
    // Test 9: TOF with duration
    total++;
    if (await testDuration('tof S1 T#3s', 3000, 'TOF with T#3s')) passed++;
    console.log();
    
    // Test 10: TP with duration
    total++;
    if (await testDuration('tp S2 T#100ms', 100, 'TP with T#100ms')) passed++;
    console.log();
    
    console.log('='.repeat(70));
    console.log(`Results: ${passed}/${total} tests passed`);
    console.log('='.repeat(70));
    
    if (passed === total) {
        console.log('\n✓ All duration datatype tests passed!');
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
