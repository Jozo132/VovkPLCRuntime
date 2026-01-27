import VovkPLC from '../../dist/VovkPLC.js';
import path from 'path';
import fs from 'fs';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

(async () => {
    // Path to WASM file
    const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm');
    if (!fs.existsSync(wasmPath)) {
        console.error('WASM file not found!');
        process.exit(1);
    }

    const runtime = await VovkPLC.createWorker(wasmPath);
    
    // Test 1 - Symbol resolution in bit operations
    const code1 = `
$$ button1 | bit | X0.0
$$ led1 | bit | Y0.0

u8.readBit button1
u8.writeBit led1
`;
    
    console.log('Test 1 - Symbol resolution in bit operations:');
    await runtime.downloadAssembly(code1);
    const error1 = await runtime.callExport('compileAssembly', true);
    const output1 = await runtime.readStream();
    console.log('  Compile Error:', error1);
    if (output1) console.log('  Output:', output1);
    console.log('');
    
    // Test 2: Mix of symbols and direct addresses
    const code2 = `
$$ sensor | bit | M10.3
$$ output | bit | Y1.5

u8.readBit sensor
u8.readBit X0.0
u8.writeBit output
u8.writeBit Y0.7
`;
    
    console.log('Test 2 - Mixed symbols and direct addresses:');
    await runtime.downloadAssembly(code2);
    const error2 = await runtime.callExport('compileAssembly', true);
    const output2 = await runtime.readStream();
    console.log('  Compile Error:', error2);
    if (output2) console.log('  Output:', output2);
    console.log('');
    
    // Test 3: Invalid symbol should still error
    const code3 = `
u8.readBit undefined_symbol
`;
    
    console.log('Test 3 - Undefined symbol should error:');
    await runtime.downloadAssembly(code3);
    const error3 = await runtime.callExport('compileAssembly', true);
    const output3 = await runtime.readStream();
    console.log('  Compile Error:', error3);
    if (output3) console.log('  Output:', output3);
    console.log('');
    
    await runtime.terminate({ exit: true });
})();
