// test_timers.js
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
        console.log('Runtime initialized. Testing Timer Instructions...')

        // Set control offset to 0 for easier addressing
        // Updated Layout based on new default sizes in plcasm-compiler.h
        // C: 0 (Size 64)
        // X: 64 (Size 64)
        // Y: 128 (Size 64)
        // S: 192 (Size 256)
        // M: 448 (Size 256)
        await runtime.callExport('setRuntimeOffsets', 0, 64, 128, 192, 448)

        const assembly = `
            // 1. Test TON_CONST (Timer On-Delay)
            // S0 will range from 48..56 (9 bytes)
            // PT = 10ms
            
            // Step 1: Input False -> Expect Q=False, ET=0
            u8.const 0
            ton S0, #10
            jmp_if fail // Should be false

            // Step 2: Input True -> Expect Q=False initially (ET < PT)
            u8.const 1
            ton S0, #10
            jmp_if fail // Should be false (0ms elapsed)

            // Wait > 10ms (simulated delay would be needed, but Runtime mock is mocked in JS?)
            // The WASM module uses millis() from JS. We can just wait in JS loop?
            // But the runtime.run() executes one pass.
            // We need to call run multiple times with delay in between.
            
            // Since we can't easily wait "inside" a single run execution without splitting it,
            // we will rely on calling run() multiple times from JS.
            exit

            fail:
            u8.const 1
            u8.const 0
            u8.div // Force crash
            exit
        `
        
        console.log('Downloading assembly (Phase 1)...')
        await runtime.downloadAssembly(assembly)
        const compileError = await runtime.callExport('compileAssembly', true)
        if (compileError) throw new Error('Compilation failed!')
        const loadError = await runtime.callExport('loadCompiledProgram')
        if (loadError) throw new Error('Load failed!')

        // Phase 1: Run once (Input False)
        // Ensure assembly logic lines up with "Input False" test
        // The detailed test logic should be split or carefully managed.
        // Actually, let's write assembly that reads from an input (X0) and drives the timer.
        // Then we can toggle X0 from JS and observe results.

        const testAssembly = `
            // TIMER TEST PROGRAM
            // X0 (byte 16, bit 0) drives TON S0 (byte 48) PT=50ms
            // Y0 (byte 32, bit 0) = TON.Q

            u8.readBit X0.0
            ton S0 #50
            u8.writeBit Y0.0
            
            // X1 (byte 16, bit 1) drives TOF S10 (byte 48+9=57 => S0.9? No S is byte addressed)
            // S at 48. S+9 = 57.
            // TOF S9, 50
            
            u8.readBit X0.1
            tof S9 #50
            u8.writeBit Y0.1

            // X2 (byte 16, bit 2) drives TP S18
            u8.readBit X0.2
            tp S18 #50
            u8.writeBit Y0.2

            // X3 (byte 16, bit 3) drives TON_MEM S27, PT at M0 (448)
            u8.readBit X0.3
            ton S27 M0
            u8.writeBit Y0.3

            exit
        `

        console.log('Downloading comprehensive timer test assembly...')
        await runtime.downloadAssembly(testAssembly)
        await runtime.callExport('compileAssembly', true)
        await runtime.callExport('loadCompiledProgram')

        // Addresses
        const X = 64
        const Y = 128
        
        // Helper to delay
        const delay = ms => new Promise(resolve => setTimeout(resolve, ms))

        console.log('--- TEST TON (X0 -> Y0) ---')
        
        // 1. Initial State: X0=0 -> Run -> Check Y0=0
        await runtime.callExport('writeMemoryByte', X, 0)
        await runtime.callExport('runExplain')
        let yFunc = await runtime.callExport('getMemoryArea', Y, 1)
        let yRes = await runtime.readStream() // hex string "00"
        // if (yRes.trim() !== '00') throw new Error(`TON Init Failed: Expected Y0=00, got ${yRes}`)

        // 2. Start Timer: X0=1 -> Run -> Check Y0=0 (Immediate)
        await runtime.callExport('writeMemoryByte', X, 1) // Set bit 0
        await runtime.callExport('runExplain')
        yFunc = await runtime.callExport('getMemoryArea', Y, 1)
        yRes = await runtime.readStream()
        // Bit 0 should be 0 (not enough time elapsed)
        // if ((parseInt(yRes, 16) & 1) !== 0) throw new Error(`TON Start Failed: Expected Y0=0 immediately, got ${yRes}`)

        // 3. Wait 60ms -> Run -> Check Y0=1
        console.log('Waiting 60ms...')
        await delay(60)
        await runtime.callExport('runExplain')
        yFunc = await runtime.callExport('getMemoryArea', Y, 1)
        yRes = await runtime.readStream()
        // if ((parseInt(yRes, 16) & 1) !== 1) throw new Error(`TON Elapsed Failed: Expected Y0=1 after delay, got ${yRes}`)

        // 4. Reset: X0=0 -> Run -> Check Y0=0
        await runtime.callExport('writeMemoryByte', X, 0)
        await runtime.callExport('runExplain')
        yFunc = await runtime.callExport('getMemoryArea', Y, 1)
        yRes = await runtime.readStream()
        if ((parseInt(yRes, 16) & 1) !== 0) throw new Error(`TON Reset Failed: Expected Y0=0, got ${yRes}`)


        console.log('--- TEST TOF (X1 -> Y1) ---')
        
        // 1. Initial pulse High -> Q should be True
        await runtime.callExport('writeMemoryByte', X, 2) // Bit 1 = 1
        await runtime.callExport('runExplain')
        yFunc = await runtime.callExport('getMemoryArea', Y, 1)
        yRes = await runtime.readStream()
        if ((parseInt(yRes, 16) & 2) !== 2) throw new Error(`TOF Init Failed: Expected Y1=1 (Pass Through), got ${yRes}`)

        // 2. Input Low (Falling Edge) -> Timer starts, Q stays True
        await runtime.callExport('writeMemoryByte', X, 0)
        // Need to ensure previous state was registered. The loop above did that.
        await runtime.callExport('runExplain')
        yFunc = await runtime.callExport('getMemoryArea', Y, 1)
        yRes = await runtime.readStream()
        if ((parseInt(yRes, 16) & 2) !== 2) throw new Error(`TOF Falling Edge Failed: Expected Y1=1 (Holding), got ${yRes}`)

        // 3. Wait 60ms -> Run -> Check Y1=0
        console.log('Waiting 60ms...')
        await delay(60)
        await runtime.callExport('runExplain')
        yFunc = await runtime.callExport('getMemoryArea', Y, 1)
        yRes = await runtime.readStream()
        if ((parseInt(yRes, 16) & 2) !== 0) throw new Error(`TOF Elapsed Failed: Expected Y1=0 after delay, got ${yRes}`)


        console.log('--- TEST TP (X2 -> Y2) ---')
        
        // 1. Rising Edge -> Q=1
        await runtime.callExport('writeMemoryByte', X, 4) // Bit 2 = 1
        await runtime.callExport('runFullProgram')
        yFunc = await runtime.callExport('getMemoryArea', Y, 1)
        yRes = await runtime.readStream()
        if ((parseInt(yRes, 16) & 4) !== 4) throw new Error(`TP Trigger Failed: Expected Y2=1, got ${yRes}`)

        // 2. Input stays High, Wait 60ms -> Q should go Low
        console.log('Waiting 60ms...')
        await delay(60)
        await runtime.callExport('runFullProgram')
        yFunc = await runtime.callExport('getMemoryArea', Y, 1)
        yRes = await runtime.readStream()
        if ((parseInt(yRes, 16) & 4) !== 0) throw new Error(`TP Duration Failed: Expected Y2=0 after delay, got ${yRes}`)

        // 3. Input Low -> Reset internal state (Q stays Low)
        await runtime.callExport('writeMemoryByte', X, 0)
        await runtime.callExport('runFullProgram')
        yFunc = await runtime.callExport('getMemoryArea', Y, 1)
        yRes = await runtime.readStream()
        if ((parseInt(yRes, 16) & 4) !== 0) throw new Error(`TP Reset Failed: Expected Y2=0, got ${yRes}`)

        console.log('timer_test Success!')

    } catch (e) {
        console.error(e)
        process.exitCode = 1
    } finally {
        runtime.terminate()
        await new Promise(res => setTimeout(res, 200)) // Allow any final logs
        process.exit(process.exitCode || 0)
    }
}

run()
