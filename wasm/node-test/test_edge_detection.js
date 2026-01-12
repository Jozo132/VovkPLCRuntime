// test_edge_detection.js
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
        console.log('Runtime initialized. Testing Edge Detection...')

        const assembly = `
            # 1. Test Rising Edge (readBitDU)
            
            # Case 1: LL (Low -> Low) [State=0, Input=0] -> Expect 0
            u8.writeBitOff C0.0
            u8.writeBitOff M0.0
            u8.readBitDU C0.0 M0.0
            u8.const 0
            u8.cmp_eq
            jmp_if_not fail

            # Case 2: LH (Low -> High) [State=0, Input=1] -> Expect 1
            u8.writeBitOn C0.0
            u8.writeBitOff M0.0
            u8.readBitDU C0.0 M0.0
            u8.const 1
            u8.cmp_eq
            jmp_if_not fail
            
            # Case 3: HH (High -> High) [State=1, Input=1] -> Expect 0
            u8.writeBitOn C0.0
            u8.writeBitOn M0.0
            u8.readBitDU C0.0 M0.0
            u8.const 0
            u8.cmp_eq
            jmp_if_not fail

            # Case 4: HL (High -> Low) [State=1, Input=0] -> Expect 0
            u8.writeBitOff C0.0
            u8.writeBitOn M0.0
            u8.readBitDU C0.0 M0.0
            u8.const 0
            u8.cmp_eq
            jmp_if_not fail

            # 2. Test Falling Edge (readBitDD)
            
            # Case 1: LL (Low -> Low) [State=0, Input=0] -> Expect 0
            u8.writeBitOff C0.1
            u8.writeBitOff M0.1
            u8.readBitDD C0.1 M0.1
            u8.const 0
            u8.cmp_eq
            jmp_if_not fail

            # Case 2: LH (Low -> High) [State=0, Input=1] -> Expect 0
            u8.writeBitOn C0.1
            u8.writeBitOff M0.1
            u8.readBitDD C0.1 M0.1
            u8.const 0
            u8.cmp_eq
            jmp_if_not fail

            # Case 3: HH (High -> High) [State=1, Input=1] -> Expect 0
            u8.writeBitOn C0.1
            u8.writeBitOn M0.1
            u8.readBitDD C0.1 M0.1
            u8.const 0
            u8.cmp_eq
            jmp_if_not fail

            # Case 4: HL (High -> Low) [State=1, Input=0] -> Expect 1
            u8.writeBitOff C0.1
            u8.writeBitOn M0.1
            u8.readBitDD C0.1 M0.1
            u8.const 1
            u8.cmp_eq
            jmp_if_not fail

            # Success
            exit

            fail:
            u8.const 255
            # Force error state or just exit with stack not empty?
            # jmp to invalid to crash ?
            u8.const 1
            u8.const 0
            u8.div # Div by zero?
            exit
        `
        
        console.log('Downloading assembly...')
        await runtime.downloadAssembly(assembly)

        console.log('Compiling assembly...')
        const compileError = await runtime.callExport('compileAssembly', true)
        const compileOutput = await runtime.readStream()
        if (compileOutput) console.log('Compiler Output:\n' + compileOutput)
        if (compileError) throw new Error('Compilation failed!')

        console.log('Loading program...')
        const loadError = await runtime.callExport('loadCompiledProgram')
        const loadOutput = await runtime.readStream()
        if (loadOutput) console.log('Loader Output:\n' + loadOutput)
        if (loadError) throw new Error('Load failed!')

        console.log('Running program (Explain Mode)...')
        const runStatus = await runtime.callExport('runExplain')
        console.log(`Run status: ${runStatus}`)
        
        if (runStatus !== 0) {
            throw new Error(`Runtime failed with status ${runStatus}`)
        }
        
        console.log('Success!')

    } catch (e) {
        console.error(e)
        // Ensure process exits with error code
        process.exitCode = 1
    } finally {
        await runtime.terminate()
    }
}

run()
