// test_project_multi_error.js
import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')

const runtime = new VovkPLC()
await runtime.initialize(wasmPath, false, true)

console.log('Testing Multiple Error Reporting in lintProject()')

// Note: VOVKPLCPROJECT header is mandatory now
const project = `
VOVKPLCPROJECT MultiErrorTest
VERSION 1.0

PROGRAM Main
    BLOCK Code LANG=PLCASM
        u8.const 10
        invalid_one
        u8.drop
        invalid_two
        exit
    END_BLOCK
END_PROGRAM
`

const problems = runtime.lintProject(project)

console.log(`Found ${problems.length} problems (expected 2)`)
problems.forEach((p, i) => {
    console.log(`Problem ${i+1}: ${p.message}`)
    console.log(`  Location: Line ${p.line}, Col ${p.column}`)
    console.log(`  Type: ${p.type}`)
    console.log(`  Token: "${p.token || ''}"`)
    console.log(`  Block: "${p.block || ''}"`)
    console.log(`  Compiler: "${p.compiler || ''}" (Lang ID: ${p.lang})`)
})

if (problems.length === 2 && 
    problems[0].message === 'unknown token' &&
    problems[1].message === 'unknown token') {
    
    // Check extra fields
    const p1 = problems[0]
    // Note: C++ uses "Code" (case sensitive), JS should read it correctly
    if (p1.block === 'Code' && p1.compiler === 'PLCASM') {
        console.log('SUCCESS: Multiple errors detected correctly with context info')
    } else {
        console.log('FAILURE: Missing block/language info')
        console.log(`Got Block: '${p1.block}', Compiler: '${p1.compiler}'`)
        process.exit(1)
    }

} else {
    console.log('FAILURE: Expected 2 errors')
    process.exit(1)
}
