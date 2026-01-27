// test_counters.js
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
        console.log('Runtime initialized. Testing Counter Instructions (CTU/CTD)...')

        // Memory Layout:
        // C: 0 (Size 64)
        // X: 64 (Size 64)
        // Y: 128 (Size 64)
        // S: 192 (Size 256) - Counter structs go here
        // M: 448 (Size 256)
        await runtime.callExport('setRuntimeOffsets', 0, 64, 128, 192, 448)

        const testAssembly = `
            // COUNTER TEST PROGRAM
            // Counter struct is 5 bytes: [CV: u32][Flags: u8]
            
            // CTU Test: X0.0=CU (count), X0.1=R (reset)
            // Counter at S0 (bytes 0-4), PV=5
            // Output to Y0.0
            u8.readBit X0.0      // CU - count up input
            u8.readBit X0.1      // R - reset input
            ctu S0 #5            // Counter at S0, preset value 5
            u8.writeBit Y0.0     // Q output - true when CV >= 5
            
            // CTD Test: X0.2=CD (count down), X0.3=LD (load)
            // Counter at S5 (bytes 5-9), PV=3
            // Output to Y0.1
            u8.readBit X0.2      // CD - count down input
            u8.readBit X0.3      // LD - load input (loads PV into CV)
            ctd S5 #3            // Counter at S5, preset value 3
            u8.writeBit Y0.1     // Q output - true when CV == 0
            
            // CTU_MEM Test: X0.4=CU, X0.5=R
            // Counter at S10, PV from M0 (u32)
            // Output to Y0.2
            u8.readBit X0.4      // CU
            u8.readBit X0.5      // R
            ctu S10 M0           // Counter at S10, preset from M0
            u8.writeBit Y0.2
            
            // CTD_MEM Test: X0.6=CD, X0.7=LD
            // Counter at S15, PV from M4 (u32)
            // Output to Y0.3
            u8.readBit X0.6      // CD
            u8.readBit X0.7      // LD
            ctd S15 M4           // Counter at S15, preset from M4
            u8.writeBit Y0.3
            
            exit
        `

        console.log('Downloading counter test assembly...')
        await runtime.downloadAssembly(testAssembly)
        const compileError = await runtime.callExport('compileAssembly', true)
        if (compileError) throw new Error('Compilation failed!')
        const loadError = await runtime.callExport('loadCompiledProgram')
        if (loadError) throw new Error('Load failed!')

        // Addresses
        const X = 64
        const Y = 128
        const S = 192
        const M = 448

        // Helper to read Y byte
        const readY = async () => {
            await runtime.callExport('getMemoryArea', Y, 1)
            const yRes = await runtime.readStream()
            return parseInt(yRes, 16)
        }

        // Helper to read counter CV (u32 at given S offset)
        // The runtime uses union which means native endianness - for WASM that's little-endian
        const readCV = async (offset) => {
            await runtime.callExport('getMemoryArea', S + offset, 5) // Read 5 bytes (CV + flags)
            const cvHex = await runtime.readStream()
            // Remove spaces and get clean hex
            const cleanHex = cvHex.replace(/\s+/g, '')
            const bytes = cleanHex.match(/.{2}/g) || []
            if (bytes.length < 4) return 0
            // Little-endian u32: byte[0] is LSB
            return parseInt(bytes[3] + bytes[2] + bytes[1] + bytes[0], 16)
        }

        // Helper to write u32 to memory (little-endian, matching runtime's union-based read)
        const writeU32 = async (addr, value) => {
            await runtime.callExport('writeMemoryByte', addr, value & 0xFF)
            await runtime.callExport('writeMemoryByte', addr + 1, (value >> 8) & 0xFF)
            await runtime.callExport('writeMemoryByte', addr + 2, (value >> 16) & 0xFF)
            await runtime.callExport('writeMemoryByte', addr + 3, (value >> 24) & 0xFF)
        }

        // Pulse helper - sets bit high, runs, sets low, runs (for edge detection)
        // We track state externally since readMemoryByte may not exist
        let currentX = 0
        const pulse = async (bitMask) => {
            currentX |= bitMask
            await runtime.callExport('writeMemoryByte', X, currentX)
            await runtime.callExport('runFullProgram')
            currentX &= ~bitMask
            await runtime.callExport('writeMemoryByte', X, currentX)
            await runtime.callExport('runFullProgram')
        }

        console.log('\n=== TEST CTU (Counter Up) ===')
        console.log('X0.0=CU, X0.1=R, S0=counter, PV=5, Y0.0=Q')
        
        // Clear all inputs
        await runtime.callExport('writeMemoryByte', X, 0)

        // 1. Initial State: CV=0, Q=0
        await runtime.callExport('runFullProgram')
        let y = await readY()
        let cv = await readCV(0)
        console.log(`Initial: CV=${cv}, Y0.0=${y & 1}`)
        if ((y & 1) !== 0) throw new Error('CTU Init Failed: Expected Q=0')
        if (cv !== 0) throw new Error('CTU Init Failed: Expected CV=0')

        // 2. Pulse CU 3 times - should count to 3
        console.log('Pulsing CU 3 times...')
        for (let i = 0; i < 3; i++) {
            await pulse(0x01) // Pulse X0.0
        }
        cv = await readCV(0)
        y = await readY()
        console.log(`After 3 pulses: CV=${cv}, Y0.0=${y & 1}`)
        if (cv !== 3) throw new Error(`CTU Count Failed: Expected CV=3, got ${cv}`)
        if ((y & 1) !== 0) throw new Error('CTU Q Failed: Expected Q=0 (CV < PV)')

        // 3. Pulse CU 2 more times - should reach 5, Q=1
        console.log('Pulsing CU 2 more times...')
        for (let i = 0; i < 2; i++) {
            await pulse(0x01)
        }
        cv = await readCV(0)
        y = await readY()
        console.log(`After 5 pulses: CV=${cv}, Y0.0=${y & 1}`)
        if (cv !== 5) throw new Error(`CTU Count Failed: Expected CV=5, got ${cv}`)
        if ((y & 1) !== 1) throw new Error('CTU Q Failed: Expected Q=1 (CV >= PV)')

        // 4. Pulse CU 1 more time - should be 6, Q still 1
        console.log('Pulsing CU 1 more time...')
        await pulse(0x01)
        cv = await readCV(0)
        y = await readY()
        console.log(`After 6 pulses: CV=${cv}, Y0.0=${y & 1}`)
        if (cv !== 6) throw new Error(`CTU Overflow Failed: Expected CV=6, got ${cv}`)
        if ((y & 1) !== 1) throw new Error('CTU Q Failed: Expected Q=1')

        // 5. Reset with R (X0.1)
        console.log('Resetting counter with R...')
        await runtime.callExport('writeMemoryByte', X, 0x02) // Set R bit
        await runtime.callExport('runFullProgram')
        await runtime.callExport('writeMemoryByte', X, 0x00) // Clear R
        await runtime.callExport('runFullProgram')
        cv = await readCV(0)
        y = await readY()
        console.log(`After reset: CV=${cv}, Y0.0=${y & 1}`)
        if (cv !== 0) throw new Error(`CTU Reset Failed: Expected CV=0, got ${cv}`)
        if ((y & 1) !== 0) throw new Error('CTU Reset Q Failed: Expected Q=0')

        console.log('CTU Test PASSED!\n')

        // ========================================
        console.log('=== TEST CTD (Counter Down) ===')
        console.log('X0.2=CD, X0.3=LD, S5=counter, PV=3, Y0.1=Q')
        
        // Clear inputs
        await runtime.callExport('writeMemoryByte', X, 0)

        // 1. Load counter with PV (set LD high)
        console.log('Loading counter with PV=3...')
        await runtime.callExport('writeMemoryByte', X, 0x08) // X0.3 = LD
        await runtime.callExport('runFullProgram')
        await runtime.callExport('writeMemoryByte', X, 0x00)
        await runtime.callExport('runFullProgram')
        cv = await readCV(5) // S5
        y = await readY()
        console.log(`After load: CV=${cv}, Y0.1=${(y >> 1) & 1}`)
        if (cv !== 3) throw new Error(`CTD Load Failed: Expected CV=3, got ${cv}`)
        if (((y >> 1) & 1) !== 0) throw new Error('CTD Q Failed: Expected Q=0 (CV > 0)')

        // 2. Count down 2 times
        console.log('Counting down 2 times...')
        for (let i = 0; i < 2; i++) {
            await pulse(0x04) // Pulse X0.2 (CD)
        }
        cv = await readCV(5)
        y = await readY()
        console.log(`After 2 counts: CV=${cv}, Y0.1=${(y >> 1) & 1}`)
        if (cv !== 1) throw new Error(`CTD Count Failed: Expected CV=1, got ${cv}`)
        if (((y >> 1) & 1) !== 0) throw new Error('CTD Q Failed: Expected Q=0')

        // 3. Count down 1 more time - should reach 0, Q=1
        console.log('Counting down 1 more time...')
        await pulse(0x04)
        cv = await readCV(5)
        y = await readY()
        console.log(`After 3 counts: CV=${cv}, Y0.1=${(y >> 1) & 1}`)
        if (cv !== 0) throw new Error(`CTD Count Failed: Expected CV=0, got ${cv}`)
        if (((y >> 1) & 1) !== 1) throw new Error('CTD Q Failed: Expected Q=1 (CV == 0)')

        // 4. Try to count below 0 - should stay at 0
        console.log('Counting down at zero (should stay 0)...')
        await pulse(0x04)
        cv = await readCV(5)
        y = await readY()
        console.log(`After underflow attempt: CV=${cv}, Y0.1=${(y >> 1) & 1}`)
        if (cv !== 0) throw new Error(`CTD Underflow Failed: Expected CV=0, got ${cv}`)

        // 5. Reload with LD
        console.log('Reloading counter...')
        await runtime.callExport('writeMemoryByte', X, 0x08)
        await runtime.callExport('runFullProgram')
        cv = await readCV(5)
        y = await readY()
        console.log(`After reload: CV=${cv}, Y0.1=${(y >> 1) & 1}`)
        if (cv !== 3) throw new Error(`CTD Reload Failed: Expected CV=3, got ${cv}`)
        if (((y >> 1) & 1) !== 0) throw new Error('CTD Reload Q Failed: Expected Q=0')

        console.log('CTD Test PASSED!\n')

        // ========================================
        console.log('=== TEST CTU_MEM (Counter Up with Memory PV) ===')
        console.log('X0.4=CU, X0.5=R, S10=counter, PV from M0=7, Y0.2=Q')

        // Set PV at M0 to 7
        await writeU32(M, 7)
        
        // Clear inputs and counter
        await runtime.callExport('writeMemoryByte', X, 0)
        await runtime.callExport('writeMemoryByte', X, 0x20) // Reset
        await runtime.callExport('runFullProgram')
        await runtime.callExport('writeMemoryByte', X, 0)
        await runtime.callExport('runFullProgram')

        // Count up 7 times
        console.log('Counting up 7 times with PV=7...')
        for (let i = 0; i < 7; i++) {
            await pulse(0x10) // Pulse X0.4
        }
        cv = await readCV(10)
        y = await readY()
        console.log(`After 7 pulses: CV=${cv}, Y0.2=${(y >> 2) & 1}`)
        if (cv !== 7) throw new Error(`CTU_MEM Count Failed: Expected CV=7, got ${cv}`)
        if (((y >> 2) & 1) !== 1) throw new Error('CTU_MEM Q Failed: Expected Q=1')

        console.log('CTU_MEM Test PASSED!\n')

        // ========================================
        console.log('=== TEST CTD_MEM (Counter Down with Memory PV) ===')
        console.log('X0.6=CD, X0.7=LD, S15=counter, PV from M4=4, Y0.3=Q')

        // Set PV at M4 to 4
        await writeU32(M + 4, 4)
        
        // Load counter
        console.log('Loading counter with PV=4 from M4...')
        await runtime.callExport('writeMemoryByte', X, 0x80) // X0.7 = LD
        await runtime.callExport('runFullProgram')
        await runtime.callExport('writeMemoryByte', X, 0)
        await runtime.callExport('runFullProgram')

        cv = await readCV(15)
        y = await readY()
        console.log(`After load: CV=${cv}, Y0.3=${(y >> 3) & 1}`)
        if (cv !== 4) throw new Error(`CTD_MEM Load Failed: Expected CV=4, got ${cv}`)

        // Count down 4 times
        console.log('Counting down 4 times...')
        for (let i = 0; i < 4; i++) {
            await pulse(0x40) // Pulse X0.6
        }
        cv = await readCV(15)
        y = await readY()
        console.log(`After 4 counts: CV=${cv}, Y0.3=${(y >> 3) & 1}`)
        if (cv !== 0) throw new Error(`CTD_MEM Count Failed: Expected CV=0, got ${cv}`)
        if (((y >> 3) & 1) !== 1) throw new Error('CTD_MEM Q Failed: Expected Q=1')

        console.log('CTD_MEM Test PASSED!\n')

        console.log('========================================')
        console.log('All Counter Tests PASSED!')
        console.log('========================================')

    } catch (e) {
        console.error('TEST FAILED:', e)
        process.exitCode = 1
    } finally {
        runtime.terminate()
        await new Promise(res => setTimeout(res, 200))
        process.exit(process.exitCode || 0)
    }
}

run()
