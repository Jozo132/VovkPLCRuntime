// test_compile.js
import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import {fileURLToPath} from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

// Override console.log for clean output
const originalLog = console.log

const run = async () => {
    // Path to WASM file
    const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')
    console.log(`Loading WASM from: ${wasmPath}`)

    if (!fs.existsSync(wasmPath)) {
        console.error('WASM file not found!')
        process.exit(1)
    }

    // In node environment, VovkPLC might handle file loading differently.
    // However, if VovkPLC.js expects a URL/path for fetch, and we are in node,
    // we might need to rely on it using 'fs' if it detects node, OR we pass the buffer.
    // Looking at VovkPLC.js, it seems to take a path string.

    const runtime = await VovkPLC.createWorker(wasmPath)
    try {
        console.log('Runtime initialized. Testing compilation...')

        // Simple Assembly Test
        const assembly = `
            u8.readBit C0.0
            u8.readBit X0.1
            u8.readBit Y0.2
            u8.readBit S0.3
            u8.readBit M0.4
            u8.readBit 0.5
            u32.drop
            u16.drop
            u8.const 10
            u8.const 20
            u8.add
            exit
        `

        console.log('----------------------------------------')
        console.log('Assembly Code to Compile:')
        console.log(assembly)
        console.log('----------------------------------------')

        // 1. Download Assembly
        console.log('Downloading assembly to runtime...')
        await runtime.downloadAssembly(assembly)

        // 2. Compile
        console.log('Compiling assembly (debug=true)...')
        const compileError = await runtime.callExport('compileAssembly', true)

        // Read any stream output from compilation
        const compileOutput = await runtime.readStream()
        if (compileOutput) {
            console.log('Compiler Output:\n' + compileOutput)
        }

        if (compileError) {
            throw new Error('Compilation failed!')
        }
        console.log('Compilation success (returned false).')

        // 3. Load Compiled Program
        console.log('Loading compiled program...')
        const loadError = await runtime.callExport('loadCompiledProgram')

        // Check output
        const loadOutput = await runtime.readStream()
        if (loadOutput) {
            console.log('Loader Output:\n' + loadOutput)
        }

        if (loadError) {
            console.error('Load failed (returned true)!')
        } else {
            console.log('Load success (returned false).')
        }

        // 4. Extract/Check Program Size
        console.log('Checking loaded program size...')
        const uploadedSize = await runtime.callExport('uploadProgram')
        console.log(`uploadProgram() returned size: ${uploadedSize}`)

        // 5. Extract content verify
        const extracted = await runtime.extractProgram()
        console.log(`extractedProgram() size: ${extracted.size}`)
        console.log(`extractedProgram() output length: ${extracted.output.length}`)
        if (extracted.output.length > 0) {
            console.log('First few bytes (HEX): ' + extracted.output.substring(0, 20) + '...')
        } else {
            console.log('Extracted output is empty.')
        }

        // 6. Run it?
        if (uploadedSize > 0) {
            console.log('Running program...')
            await runtime.callExport('runFullProgramDebug')
            const runOutput = await runtime.readStream()
            console.log('Run Output:\n' + runOutput)
        } else {
            console.error('Cannot run empty program.')
        }
    } finally {
        await runtime.terminate()
    }
}

run().catch(err => {
    console.error('Unhandled Error:', err)
    process.exit(1)
})
