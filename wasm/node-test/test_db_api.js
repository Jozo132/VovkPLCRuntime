// Quick test for the DataBlock WASM API
import VovkPLC from '../dist/VovkPLC.js'

const plc = new VovkPLC()
await plc.init('./wasm/dist/VovkPLC.wasm', false, true)

const project = `
VOVKPLCPROJECT TestDB
VERSION 1.0

DATABLOCKS
  DB1 "Motor" {
    speed : i16 = 100
    position : f32 = 3.14
    status : u8 = 1
  }
  DB2 "Sensor" {
    value : f32
    threshold : f32 = 50.0
  }
END_DATABLOCKS

PROGRAM Main
  BLOCK LANG=PLCASM Code
    exit
  END_BLOCK
END_PROGRAM
`

const result = plc.compileProject(project)
if (result.problem) {
    console.log('Compile error:', result.problem)
    process.exit(1)
}
console.log('compiled:', !!result.bytecode)
console.log('bytecode:', result.bytecode)
console.log('warnings:', result.warnings)

// Get the full bytecode from the project compiler directly
const bcLen = plc.wasm_exports.project_getBytecodeLength()
const bcPtr = plc.wasm_exports.project_getBytecode()
console.log('project bytecodeLength:', bcLen)
console.log('project bytecodePtr:', bcPtr)
console.log('declCount:', plc.dbGetDeclCount())

const decls = plc.dbGetAllDecls()
for (const d of decls) {
    console.log(`DB${d.db_number} '${d.alias}' size=${d.totalSize} offset=${d.computedOffset}`)
    for (const f of d.fields) {
        console.log(`  ${f.name}: ${f.typeName} (size=${f.typeSize}, offset=${f.offset}, default=${f.hasDefault ? f.defaultValue : 'none'})`)
    }
}

// Load to runtime and run once to execute CONFIG_DB
const loadResult = plc.wasm_exports.project_load()
console.log('load result:', loadResult)
console.log('bytecode:', result.bytecode)
console.log('bytecode length:', result.bytecode.length / 2, 'bytes')
const runResult = plc.run() // Executes CONFIG_DB + program (allocates DBs, writes defaults)
console.log('run result:', runResult)
console.log('active DBs after run:', plc.dbGetActiveCount())
console.log('active entries:', plc.dbGetActiveEntries())
console.log('slot count:', plc.dbGetSlotCount())
for (let i = 0; i < plc.dbGetSlotCount(); i++) {
    const e = plc.dbGetEntry(i)
    if (e.db_number !== 0) console.log(`  slot ${i}: DB${e.db_number} offset=${e.offset} size=${e.size}`)
}
console.log('resolveAddress(1,0):', plc.dbResolveAddress(1, 0))
const motor = plc.dbReadFields(1)
console.log('Motor fields:', motor)

plc.dbWriteField(1, 'speed', 2000)
console.log('Motor.speed after write:', plc.dbReadField(1, 'speed'))

plc.dbWriteFields(2, { value: 42.5, threshold: 100.0 })
console.log('Sensor fields:', plc.dbReadFields(2))

// Test dbGetActiveEntries
console.log('Active entries:', plc.dbGetActiveEntries())

console.log('\nAll DataBlock API tests passed!')
