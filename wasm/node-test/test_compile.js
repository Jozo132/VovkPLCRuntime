// test_compile.js
import VovkPLC from '../dist/VovkPLC.js';
import path from 'path';
import fs from 'fs';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Override console.log for clean output
const originalLog = console.log;

const run = async () => {
    // Path to WASM file
    const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm');
    console.log(`Loading WASM from: ${wasmPath}`);

    if (!fs.existsSync(wasmPath)) {
        console.error("WASM file not found!");
        process.exit(1);
    }
    
    // In node environment, VovkPLC might handle file loading differently.
    // However, if VovkPLC.js expects a URL/path for fetch, and we are in node, 
    // we might need to rely on it using 'fs' if it detects node, OR we pass the buffer.
    // Looking at VovkPLC.js, it seems to take a path string.
    
    const runtime = new VovkPLC(wasmPath);
    await runtime.initialize();
    
    const exports = runtime.getExports();
    
    console.log("Runtime initialized. Testing compilation...");
    
    // Simple Assembly Test
    const assembly = `
    u8.const 10
    u8.const 20
    u8.add
    exit
    `;
    
    console.log("----------------------------------------");
    console.log("Assembly Code to Compile:");
    console.log(assembly);
    console.log("----------------------------------------");
    
    // 1. Download Assembly
    console.log("Downloading assembly to runtime...");
    // We can use runtime.downloadAssembly(string) or manually streamIn
    runtime.downloadAssembly(assembly);
    
    // 2. Compile
    console.log("Compiling assembly (debug=true)...");
    const compileError = exports.compileAssembly(true);
    
    // Read any stream output from compilation
    const compileOutput = runtime.readStream();
    if (compileOutput) {
        console.log("Compiler Output:\n" + compileOutput);
    }
    
    if (compileError) {
        console.error("Compilation failed!");
        process.exit(1);
    }
    console.log("Compilation success (returned false).");
    
    // 3. Load Compiled Program
    console.log("Loading compiled program...");
    const loadError = exports.loadCompiledProgram();
    
    // Check output
    const loadOutput = runtime.readStream();
    if (loadOutput) {
        console.log("Loader Output:\n" + loadOutput);
    }
    
    if (loadError) {
        console.error("Load failed (returned true)!");
    } else {
        console.log("Load success (returned false).");
    }

    // 4. Extract/Check Program Size
    console.log("Checking loaded program size...");
    const uploadedSize = exports.uploadProgram();
    console.log(`uploadProgram() returned size: ${uploadedSize}`);
    
    // 5. Extract content verify
    const extracted = runtime.extractProgram();
    console.log(`extractedProgram() size: ${extracted.size}`);
    console.log(`extractedProgram() output length: ${extracted.output.length}`);
    if (extracted.output.length > 0) {
        console.log("First few bytes (HEX): " + extracted.output.substring(0, 20) + "...");
    } else {
        console.log("Extracted output is empty.");
    }

    // 6. Run it?
    if (uploadedSize > 0) {
        console.log("Running program...");
        exports.runFullProgramDebug();
        const runOutput = runtime.readStream();
        console.log("Run Output:\n" + runOutput);
    } else {
        console.error("Cannot run empty program.");
    }
    
}

run().catch(err => {
    console.error("Unhandled Error:", err);
    process.exit(1);
});
