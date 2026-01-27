
import VovkPLC from '../../dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')

const runtime = new VovkPLC()
await runtime.initialize(wasmPath, false, true)

console.log('Testing P_First_Cycle (Memory Offset 20)')

// 1. First cycle
// Before running, memory should be 0 (default format)
// But wait, updateGlobals logic happens inside run()
// So check flag inside program or run one cycle and check if it WAS set?
// The flag is set at the BEGINNING of run() loop, and cleared at the END of run() variable logic?
// No, I implemented:
// updateGlobals sets memory[20] based on is_first_cycle.
// run loop executes program.
// after loop, is_first_cycle = false.

// So:
// Cycle 1: is_first_cycle=true -> memory[20]=1 -> Execute Program
// Runtime finishes -> is_first_cycle=false
// Cycle 2: is_first_cycle=false -> memory[20]=0 -> Execute Program

// Test:
// Run 1 cycle. Check if memory has 1?
// Wait, memory persists. If I check memory *after* run returns, it should still have 1 because updateGlobals wrote it.
// The next updateGlobals call (start of cycle 2) will clear it.

console.log("Running Cycle 1...")
runtime.run() // Executes one scan cycle (program is empty/default)

const mem1 = runtime.readMemoryArea(20, 1)[0]
console.log(`Cycle 1 - Offset 20: ${mem1}`)

if ((mem1 & 1) !== 1) {
    console.error("FAILURE: P_First_Cycle was not set on first cycle")
    process.exit(1)
}

console.log("Running Cycle 2...")
runtime.run() 

const mem2 = runtime.readMemoryArea(20, 1)[0]
console.log(`Cycle 2 - Offset 20: ${mem2}`)

if ((mem2 & 1) !== 0) {
    console.error("FAILURE: P_First_Cycle was not cleared on second cycle")
    process.exit(1)
}

console.log("SUCCESS: P_First_Cycle logic verified")
