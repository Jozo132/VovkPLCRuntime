// Quick test for ladder compiler debugging
import VovkPLC from '../../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

async function run() {
    const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')
    const plc = new VovkPLC(wasmPath);
    plc.stdout_callback = (msg) => console.log('[WASM]', msg);
    await plc.initialize(wasmPath, false, false);
    plc.readStream(); // Clear any init messages

    // Ladder Graph JSON with nodes and connections (sources/destinations arrays)
    const testInput = JSON.stringify({
        "nodes": [
            { "id": "n1", "type": "contact", "symbol": "I0.0", "x": 0, "y": 0 },
            { "id": "n2", "type": "coil", "symbol": "Q0.0", "x": 1, "y": 0 }
        ],
        "connections": [
            { "sources": ["n1"], "destinations": ["n2"] }
        ]
    });

    console.log('Input length:', testInput.length);

    // Clear state before compilation
    plc.wasm_exports.ladder_standalone_clear();

    // Stream the input character by character
    for (let i = 0; i < testInput.length; i++) {
        plc.wasm_exports.streamIn(testInput.charCodeAt(i));
    }
    plc.wasm_exports.streamIn(0); // Null terminator
    console.log('Input stream index after streaming:', plc.wasm_exports.get_in_index());
    
    // Load from stream
    plc.wasm_exports.ladder_standalone_load_stream();
    console.log('Input stream index after load:', plc.wasm_exports.get_in_index());
    
    // Compile - now returns output_len on success, -1 on failure
    console.log('Calling ladder_standalone_compile...');
    const compileResult = plc.wasm_exports.ladder_standalone_compile();
    const success = compileResult >= 0;
    console.log('Compile result:', compileResult, '(success:', success, ')');
    
    // Check for errors first
    const hasError = plc.wasm_exports.ladder_standalone_has_error();
    console.log('Has error:', hasError);
    
    // Get output length - also compare with compile result
    const outputLen = plc.wasm_exports.ladder_standalone_get_output_len();
    console.log('Output length from getter:', outputLen, '(from compile:', compileResult, ')');
    
    // Use compileResult as outputLen if getter returns 0
    const effectiveLen = outputLen > 0 ? outputLen : compileResult;
    
    if (effectiveLen > 0) {
        plc.wasm_exports.ladder_standalone_output_to_stream();
        const output = plc.readOutBuffer();
        console.log('Output:', output);
    } else {
        // Check for errors
        if (hasError) {
            const errorPtr = plc.wasm_exports.ladder_standalone_get_error();
            const mem = new Uint8Array(plc.wasm_exports.memory.buffer);
            let error = '';
            for (let i = 0; mem[errorPtr + i] !== 0 && i < 256; i++) {
                error += String.fromCharCode(mem[errorPtr + i]);
            }
            console.log('Error:', error);
        } else {
            console.log('No output and no error - something went wrong');
        }
    }
}

run().catch(console.error);
