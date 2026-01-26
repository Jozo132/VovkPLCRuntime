import VovkPLC from '../../dist/VovkPLC.js';
import path from 'path';
import fs from 'fs';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const wasmPath = path.join(__dirname, '../../dist/VovkPLC.wasm');

async function test() {
    const runtime = new VovkPLC(wasmPath);
    await runtime.initialize(wasmPath, false, false);
    const wasm = runtime.wasm_exports;
    
    wasm.project_reset();
    
    // Use test_03.project
    const code = fs.readFileSync(path.join(__dirname, 'test_03.project'), 'utf8');
    
    // Stream code
    for (let i = 0; i < code.length; i++) {
        wasm.streamIn(code.charCodeAt(i));
    }
    
    // Compile
    const success = wasm.project_compile(1);
    
    // Get string helper
    const getString = (ptr) => {
        if (!ptr) return '';
        const memory = new Uint8Array(runtime.wasm.exports.memory.buffer);
        let str = '';
        let i = ptr;
        while (memory[i] !== 0 && i < memory.length) {
            str += String.fromCharCode(memory[i]);
            i++;
        }
        return str;
    };
    
    console.log('=== Compilation Result ===');
    console.log('Success:', success);
    console.log('Bytecode len:', wasm.project_getBytecodeLength());
    console.log('Has error:', wasm.project_hasError());
    
    if (wasm.project_hasError()) {
        console.log('\n=== Error Details ===');
        console.log('Compiler:', getString(wasm.project_getErrorCompiler()));
        console.log('File:', getString(wasm.project_getErrorFile()));
        console.log('Block:', getString(wasm.project_getErrorBlock()));
        console.log('Line:', wasm.project_getErrorLine());
        console.log('Column:', wasm.project_getErrorColumn());
        console.log('Error:', getString(wasm.project_getError()));
        
        const token = getString(wasm.project_getErrorToken());
        if (token) {
            console.log('Token:', `"${token}"`);
            console.log('Token Length:', wasm.project_getErrorTokenLength());
        }
        
        const sourceLine = getString(wasm.project_getErrorSourceLine());
        if (sourceLine) {
            console.log('\n=== Source Line ===');
            console.log(sourceLine);
            // Print pointer to error column
            const col = wasm.project_getErrorColumn();
            if (col > 0) {
                const pointer = ' '.repeat(col - 1) + '^';
                console.log(pointer);
            }
        }
    } else {
        console.log('\nCompilation succeeded!');
    }
}

test().catch(console.error);
