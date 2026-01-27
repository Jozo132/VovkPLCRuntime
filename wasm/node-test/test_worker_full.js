// @ts-check
'use strict'

import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import {fileURLToPath} from 'url'
import fs from 'fs'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')

const assert = (condition, message) => {
    if (!condition) {
        throw new Error(message || "Assertion failed")
    }
}

const main = async () => {
    if (!fs.existsSync(wasmPath)) {
        console.error('WASM file not found at:', wasmPath)
        process.exit(1)
    }

    console.log('::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::')
    console.log(':: VovkPLC Shared Memory Worker Verification Test')
    console.log('::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::')

    const worker = await VovkPLC.createWorker(wasmPath, {silent: false, debug: false})
    worker.onStdout(msg => console.log('    [WORKER]', msg)) // Log all output
    
    try {
        console.log('\n[1] Setup Shared IO...')
        const ok = await worker.enableSharedMemory()
        assert(ok, 'enableSharedMemory failed')
        console.log('    Shared Memory enabled.')
        
        await worker.printInfo()
        const exportedWasmFunctions = await worker.getExports()
        console.log('    GetExports result:', exportedWasmFunctions)
        assert(exportedWasmFunctions && exportedWasmFunctions.length > 0, 'No exports found')
        console.log(`    Worker initialized. Found ${exportedWasmFunctions.length} exports.`)

        // ---------------------------------------------------------
        // 1b. Memory Access Test
        // ---------------------------------------------------------
        console.log('\n[1b] Memory Access Test...')
        await worker.writeMemoryArea(0, [123])
        const checkMem = await worker.readMemoryArea(0, 1)
        console.log(`    Written 123, Read ${checkMem[0]}`)
        assert(checkMem[0] === 123, 'Memory Read/Write failed')
        console.log('    Memory Read/Write verified.')

        // ---------------------------------------------------------
        // 2. Compilation Test
        // ---------------------------------------------------------
        console.log('\n[2] Compilation Test...')
        const assembly = `
            // Calculate 10 + 20 = 30
            // Try pushing Pointer FIRST.
            ptr.const 0
            
            u8.const 10
            u8.const 20
            u8.add
            // Stack: [Ptr, 30]
            
            // Execute move
            u8.move
            
            // Read back to verify inside ASM
            ptr.const 0
            u8.load
            
            exit
        `
        
        console.log('    Compiling simple addition...')
        // compilePLCASM(assembly, {run: true}) attempts to load and runDebug immediately
        const compilationResult = await worker.compilePLCASM(assembly, {run: true}) 
        console.log('    Compilation successful & Debug Run executed.')
        // Verify bytecode extraction
        assert(compilationResult && compilationResult.size > 0 && compilationResult.output, "Bytecode extraction failed")
        // console.log('    Compilation Result:', JSON.stringify(compilationResult))

        // Remove manual loading as compile(..., true) does it
        // console.log('    Loading compiled program...')
        // await worker.callExport('loadCompiledProgram')
        // console.log('    Program loaded.')

        // ---------------------------------------------------------
        // 3. Execution (Verification)
        // ---------------------------------------------------------
        console.log('\n[3] Execution (Run Single Cycle)...')
        
        // compile(..., true) already ran once!
        // So memory should be updated.
        
        // Setup Shared Mode just in case it influences run
        // worker.setSharedMode('run') // Not needed for run() calls usually?
        
        // Verify Memory
        // Value 30 should be at address 0
        const mem = await worker.readMemoryArea(0, 1)
        const result = mem[0]
        console.log(`    Memory[0] = ${result}`)
        assert(result === 30, `Expected Memory[0] to be 30, got ${result}`)
        
        // ---------------------------------------------------------
        // 3b. Bytecode Upload & Performance Test
        // ---------------------------------------------------------
        console.log('\n[3b] Bytecode Upload & Performance Test...')
        if (compilationResult && compilationResult.output) {
            // Restore program via bytecode
            console.log('    Uploading extracted bytecode to WASM...')
            await worker.downloadBytecode(compilationResult.output)
            
            console.log('    Benchmarking 1000 cycles...')
            const start = performance.now()
            for (let i = 0; i < 1000; i++) {
                await worker.run() // Run a single cycle
            }
            const end = performance.now()
            const total = end - start
            const perCycle = total / 1000
            console.log(`    1000 cycles completed in ${total.toFixed(2)} ms`)
            console.log(`    Avg time per cycle: ${perCycle.toFixed(4)} ms`)
            
            // Verify memory is still correct
            const memBench = await worker.readMemoryArea(0, 1)
            assert(memBench[0] === 30, 'Memory verify failed after benchmark')
        } else {
            console.warn('    Skipping benchmark - no bytecode extracted.')
        }

        // ---------------------------------------------------------
        // 4. Integrated Unit Test
        // ---------------------------------------------------------
        /*
        console.log('\n[4] Integrated Unit Test (run_unit_test)...')
        // Capturing output to check for "Done" or errors
        let output = ''
        await worker.onStdout(msg => {
            output += msg
            // console.log('[STDOUT]', msg) // Optional verbose
        })
        
        console.log('    Running Internal Unit Tests...')
        try {
            await worker.callExport('run_unit_test')
            // run_custom_test is also used in unit_test.js
            for(let i=0; i<5; i++) await worker.callExport('run_custom_test')
        } catch(e) {
            console.error('    Unit Test Execution Failed:', e)
            throw e
        }
        
        // Wait a bit for stream flush if async?
        // readStream is manual in non-worker, but here we use callback
        // The callback might be batched.
        
        console.log('    Internal Unit Tests Completed.')
        if (output.includes('FAIL') || output.includes('Error')) {
            console.log('    Output contains potential errors:')
            console.log(output)
             // Not throwing strictly unless we know formats, but warning
        } else {
             console.log('    Log looks clean.')
        }
        */

        // ---------------------------------------------------------
        // 5. Bytecode Debugging / Stepping
        // ---------------------------------------------------------
        console.log('\n[5] Bytecode Debugger Test...')
        // We use shared control for stepping
        // 1. Reset
        await worker.compilePLCASM(assembly) // Re-compile/Reset memory
        // Clear memory 0 manually to be sure
        await worker.writeMemoryArea(0, [0])
        
        // DEBUG: Check offsets
        const info = await worker.printInfo()
        console.log('    Info:', JSON.stringify(info))
        try {
           const memLoc = await worker.callExport('getMemoryLocation')
           console.log(`    getMemoryLocation: ${memLoc}`)
        } catch(e) { console.log('    Could not getMemoryLocation via export: ' + e.message) }

        // Reload the program for the debugger test (since unit tests overwrote it)
        console.log('    Reloading simple program...');
        await worker.compilePLCASM(`
            ptr.const 0
            u8.const 10
            u8.const 20
            u8.add
            u8.move
            exit
        `);
        
        // 2. Set to STEP mode via Shared Memory
        worker.setSharedMode('step')
        
        // 3. Step loop
        console.log('    Stepping through instructions...')
        let cycles = 0
        const maxCycles = 10
        let val = 0
        
        while(cycles < maxCycles) {
            // Check status (requires polling the shared struct)
            const status = worker.getSharedStatus()
            
            // Trigger a step
            worker.setSharedMode('step')
            
            // Simplified: wait for memory change
            await new Promise(r => setTimeout(r, 50))
            
            const memNew = await worker.readMemoryArea(0, 1)
            console.log(`    Cycle ${cycles}: Status=${status?.status} Cycle=${status?.cycle} Mem[0]=${memNew[0]}`)
            
            if (memNew[0] === 30) {
                val = 30
                console.log(`    Target value reached at cycle attempt ${cycles+1}`)
                break
            }
            cycles++
        }
        
        assert(val === 30, 'Stepping did not produce result')
        console.log('    Stepping verified.')

        // ---------------------------------------------------------
        // 6. Runtime Benchmark (Virtual Machine Mode)
        // ---------------------------------------------------------
        console.log('\n[6] Runtime Benchmark (Virtual Machine Mode)...')
        console.log('    Starting PLC Runtime for 5 seconds...')
        
        await worker.resetStats()
        await worker.startRuntime()

        // Wait 5 seconds
        await new Promise(r => setTimeout(r, 5000))

        await worker.stopRuntime()
        console.log('    Runtime stopped. Waiting for status update...')
        
        // Wait for status to settle (Worker loop poll is ~50ms)
        let stats = await worker.getSharedStatus()
        for(let i=0; i<20; i++) {
             stats = await worker.getSharedStatus()
             if (stats.status === 0 || stats.status === 2) break
             await new Promise(r => setTimeout(r, 50))
        }

        console.log('    PLC Runtime Statistics:')
        console.log('      Status:   ', stats.status)
        console.log('      Cycles:   ', stats.cycles)
        if (stats.cycles > 0) {
            console.log('      Last (us):', stats.last_time)
            console.log('      Min (us): ', stats.min_time)
            console.log('      Max (us): ', stats.max_time)
        } else {
            console.warn('      No cycles executed!')
        }
        
        // Assertions
        assert(stats.cycles > 500, `Expected > 500 cycles in 5s, got ${stats.cycles}`)
        assert(stats.status === 2 || stats.status === 0, 'Status should be stopped/idle') 

        console.log('\n[SUCCESS] All worker verification tests passed.')

    } catch (e) {
        console.error('\n[FAILED] Test failed with error:')
        console.error(e)
        process.exit(1)
    } finally {
        worker.setSharedMode('stop')
        worker.terminate()
        await new Promise(res => setTimeout(res, 100)) // Allow any final logs
        process.exit(0)
    }
}

main()
