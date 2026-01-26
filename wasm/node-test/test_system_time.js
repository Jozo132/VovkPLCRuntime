
import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')

const runtime = new VovkPLC()
await runtime.initialize(wasmPath, false, true)

console.log('Testing System Time (Memory Offset 16)')

// 1. Check initial value (should be 0)
// Control offset is 0 usually. So absolute address 16.
let mem = runtime.readMemoryArea(16, 4)
let timeVal = mem[0] | (mem[1] << 8) | (mem[2] << 16) | (mem[3] << 24)
console.log(`Initial System Time: ${timeVal}`)

// 2. Set time manually to 1000
const startTime = 1000
const bytes = [
    startTime & 0xFF,
    (startTime >> 8) & 0xFF,
    (startTime >> 16) & 0xFF,
    (startTime >> 24) & 0xFF
]
// Write byte by byte
for(let i=0; i<4; i++) runtime.writeMemoryByte(16+i, bytes[i])

mem = runtime.readMemoryArea(16, 4)
timeVal = mem[0] | (mem[1] << 8) | (mem[2] << 16) | (mem[3] << 24)
console.log(`Manually Set Time: ${timeVal}`)

if (timeVal !== 1000) {
    console.error('FAILURE: Failed to write time')
    process.exit(1)
}

// 3. Run runtime in a loop for > 1 second
console.log('Running for ~1.5s...')
const delay = ms => new Promise(resolve => setTimeout(resolve, ms))

// We need to loop and call run() repeatedly
// P_1s depends on IntervalGlobalLoopCheck which reads performance.now()
const duration = 1500
const startTs = Date.now()

while (Date.now() - startTs < duration) {
    runtime.run()
    await delay(10) 
}

// 4. Check time
mem = runtime.readMemoryArea(16, 4)
let newTime = mem[0] | (mem[1] << 8) | (mem[2] << 16) | (mem[3] << 24)
console.log(`New System Time: ${newTime}`)

if (newTime > 1000) {
    console.log(`SUCCESS: Time incremented to ${newTime}`)
} else {
    console.error('FAILURE: Time did not increment')
    process.exit(1)
}
process.exit(0)
