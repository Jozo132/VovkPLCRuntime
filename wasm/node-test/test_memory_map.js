// test_memory_map.js - Test Timer and Counter memory mapping with struct size multipliers
import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import {fileURLToPath} from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')

const run = async () => {
    const runtime = await VovkPLC.createWorker(wasmPath)
    try {
        console.log('Testing Timer/Counter memory mapping with struct size multipliers...\n')

        // Set up runtime offsets - memory layout:
        // Controls: 0-63 (64 bytes)
        // Inputs:   64-127 (64 bytes)  
        // Outputs:  128-191 (64 bytes)
        // Systems:  192-447 (256 bytes)
        // Markers:  448-703 (256 bytes)
        // Timers:   704+ (16 timers * 9 bytes = 144 bytes)
        // Counters: 848+ (16 counters * 5 bytes = 80 bytes)
        
        await runtime.callExport('setRuntimeOffsets', 0, 64, 128, 192, 448)

        // Test timer address parsing
        // T0 should be at offset 704, T1 at 713, T2 at 722
        console.log('Testing Timer addresses with struct size multiplier (9 bytes):')
        
        const timerTest = `
            // Timer T0 with preset 1000ms
            u8.const 1
            ton T0, 1000
            
            // Timer T1 with preset 2000ms  
            u8.const 1
            ton T1, 2000
            
            // Timer T2 with preset 3000ms
            u8.const 1
            ton T2, 3000
        `

        let result = await runtime.compile(timerTest)
        console.log('Timer compile result: SUCCESS')
        console.log('Bytecode size:', result.size, 'bytes')
        console.log('Bytecode:', result.output)
        
        // Execute the bytecode
        await runtime.callExport('loadCompiledProgram')
        const timerRunResult = await runtime.callExport('runFullProgramDebug')
        console.log('Timer execution result:', timerRunResult)
        const timerOutput = await runtime.readStream()
        if (timerOutput) console.log('Timer output:', timerOutput)

        // Test counter address parsing
        // K0 should be at counter offset, K1 at +5, K2 at +10
        console.log('\nTesting Counter addresses with struct size multiplier (5 bytes):')
        
        const counterTest = `
            // Counter C0 at base counter offset
            u32.const 100
            u32.move_to C0
            
            // Counter C1 should be at counter_offset + (1 * 5)
            u32.const 200
            u32.move_to C1
        `

        result = await runtime.compile(counterTest)
        console.log('Counter compile result: SUCCESS')
        console.log('Bytecode size:', result.size, 'bytes')
        console.log('Bytecode:', result.output)
        
        // Execute the bytecode
        await runtime.callExport('loadCompiledProgram')
        const counterRunResult = await runtime.callExport('runFullProgramDebug')
        console.log('Counter execution result:', counterRunResult)
        const counterOutput = await runtime.readStream()
        if (counterOutput) console.log('Counter output:', counterOutput)

        console.log('\nTimer/Counter memory mapping test complete!')
    } finally {
        await runtime.terminate()
    }
}

run()
    .catch(console.error)
    .finally(() => process.exit())
