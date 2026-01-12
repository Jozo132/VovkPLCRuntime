// test_flow_rel.js
import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import {fileURLToPath} from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

const run = async () => {
    const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')
    console.log(`Loading WASM from: ${wasmPath}`)

    if (!fs.existsSync(wasmPath)) {
        console.error('WASM file not found!')
        process.exit(1)
    }

    const runtime = await VovkPLC.createWorker(wasmPath)
    try {
        console.log("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
        console.log("Runtime initialized. Testing Relative Flow Instructions...");

        const assembly_with_labels = `
            u8.const 0
        loop:
            u8.copy
            u8.const 3
            u8.cmp_eq
            jmp_if_rel end_loop
            
            u8.const 1
            u8.add
            
            call_rel my_func
            
            jmp_rel loop

        end_loop:
            u8.drop
            exit

        my_func:
            ret
        `;

        console.log("Downloading assembly...");
        await runtime.downloadAssembly(assembly_with_labels);

        console.log("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
        console.log("Compiling assembly...");
        const compileStatus = await runtime.callExport('compileAssembly', true);
        const compileOutput = await runtime.readStream();
        if (compileOutput) console.log('Compiler Output:\n' + compileOutput);

        if (compileStatus !== 0) {
             console.error("Compilation failed with status: " + compileStatus);
             process.exit(1);
        }

        console.log("Loading compiled program...");
        await runtime.callExport('loadCompiledProgram');
        
        console.log("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
        console.log("Running program (Explain Mode)...");
        
        await runtime.callExport('runExplain');
        
        console.log("Success!");

    } catch (e) {
        console.error(e)
        process.exit(1)
    } finally {
        if (runtime) runtime.terminate()
    }
}

run()
