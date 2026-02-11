import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'
const __dirname = path.dirname(fileURLToPath(import.meta.url))
const wasmPath = path.resolve(__dirname, '..', 'dist', 'VovkPLC.wasm')
const plc = new VovkPLC(wasmPath)
plc.stdout_callback = () => {}
await plc.initialize(wasmPath, false, false)
plc.readStream()

// Test 1: lintProjectMetadata with duplicate field
const source1 = `VOVKPLCPROJECT T
VERSION 1.0
MEMORY
  OFFSET 0
  AVAILABLE 1024
  S 64
  X 64
  Y 64
  M 256
END_MEMORY
DATABLOCKS
DB1 {
  value: u16
  value: u32
}
END_DATABLOCKS
PROGRAM main
  BLOCK LANG=LADDER Net1
{"comment":"","nodes":[],"connections":[]}
  END_BLOCK
END_PROGRAM`
const problems1 = plc.lintProjectMetadata(source1)
console.log('Test 1 - Duplicate field:')
console.log(JSON.stringify(problems1, null, 2))
console.assert(problems1.length > 0, 'Should find problems')
console.assert(problems1[0].block === 'DATABLOCKS', `Expected block=DATABLOCKS, got ${problems1[0].block}`)
console.assert(problems1[0].message.includes('Duplicate field'), `Expected duplicate message, got ${problems1[0].message}`)

// Test 2: lintProjectMetadata with no errors
const source2 = `VOVKPLCPROJECT T
VERSION 1.0
MEMORY
  OFFSET 0
  AVAILABLE 1024
  S 64
  X 64
  Y 64
  M 256
END_MEMORY
DATABLOCKS
DB1 {
  x: u16
  y: u32
}
END_DATABLOCKS
PROGRAM main
  BLOCK LANG=LADDER Net1
{"comment":"","nodes":[],"connections":[]}
  END_BLOCK
END_PROGRAM`
const problems2 = plc.lintProjectMetadata(source2)
console.log('\nTest 2 - No errors:')
console.log(JSON.stringify(problems2, null, 2))
console.assert(problems2.length === 0, `Expected no problems, got ${problems2.length}`)

console.log('\n✓ All lintProjectMetadata tests passed')
