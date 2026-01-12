// test_edge_extensions.js
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
        console.log('Runtime initialized. Testing Edge Detection Extensions...')

        const assembly = `
            # 1. Test STACK_DU (Rising Edge of Stack Value)
            # Use C0.0 (Input) and M0.0 (State)

            # Case: LL (Input 0, State 0) -> Expect 0
            u8.writeBitOff C0.0
            u8.writeBitOff M0.0
            u8.readBit C0.0 # Push 0
            u8.du M0.0      # Check DU using state M0.0
            u8.const 0
            u8.cmp_eq
            jmp_if_not fail

            # Case: LH (Input 1, State 0) -> Expect 1
            u8.writeBitOn C0.0
            u8.writeBitOff M0.0
            u8.readBit C0.0 # Push 1
            u8.du M0.0
            u8.const 1
            u8.cmp_eq
            jmp_if_not fail

            # Case: HH (Input 1, State 1) -> Expect 0
            u8.writeBitOn C0.0
            u8.writeBitOn M0.0
            u8.readBit C0.0 # Push 1
            u8.du M0.0
            u8.const 0
            u8.cmp_eq
            jmp_if_not fail
            
            # Case: HL (Input 0, State 1) -> Expect 0
            u8.writeBitOff C0.0
            u8.writeBitOn M0.0
            u8.readBit C0.0 # Push 0
            u8.du M0.0
            u8.const 0
            u8.cmp_eq
            jmp_if_not fail

            # 2. Test WRITE_BIT_DU (Pulse Coil)
            # Target C0.1, State M0.1

            # Case: Rising Edge Input (1) -> Target Should be 1
            u8.writeBitOff M0.1 # Clear State
            u8.const 1          # Input 1 (Rising from state 0)
            u8.writeBitDU C0.1 M0.1
            
            # Check Target C0.1 is 1
            u8.readBit C0.1
            u8.const 1
            u8.cmp_eq
            jmp_if_not fail

            # Case: Stable High Input (1) -> Target Should be 0
            u8.writeBitOn M0.1 # State 1
            u8.const 1         # Input 1
            u8.writeBitDU C0.1 M0.1
            
            # Check Target C0.1 is 0
            u8.readBit C0.1
            u8.const 0
            u8.cmp_eq
            jmp_if_not fail

            # 3. Test WRITE_SET_DU (Set on Rising)
            # Target C0.2, State M0.2

            # Init Target 0, State 0
            u8.writeBitOff C0.2
            u8.writeBitOff M0.2
            
            # Rising Edge -> Target Set to 1
            u8.const 1
            u8.writeBitOnDU C0.2 M0.2
            u8.readBit C0.2
            u8.const 1
            u8.cmp_eq
            jmp_if_not fail

            # Falling Edge -> Target stays 1
            u8.writeBitOff C0.2 # Wait, reset target manually to check if it STAYS 0 on fall? 
                                # No, SET coil sets to 1. 
            u8.writeBitOn C0.2  # Ensure it is 1
            u8.writeBitOn M0.2  # State 1
            
            # Provide 0 (Falling Edge 1->0)
            u8.const 0
            u8.writeBitOnDU C0.2 M0.2 
            
            # Target should still be 1 (No action on falling)
            u8.readBit C0.2
            u8.const 1
            u8.cmp_eq
            jmp_if_not fail
            
            # Provide 1 (Rising Edge 0->1) but target is already 1.
            # Reset State manually to 0
            u8.writeBitOff M0.2
            u8.const 1
            u8.writeBitOnDU C0.2 M0.2 
            u8.readBit C0.2
            u8.const 1
            u8.cmp_eq
            jmp_if_not fail

            # Success
            exit

            fail:
            u8.const 255
            u8.const 1
            u8.const 0
            u8.div
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
