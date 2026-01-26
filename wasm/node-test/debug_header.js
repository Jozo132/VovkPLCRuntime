import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')

const runtime = new VovkPLC()
await runtime.initialize(wasmPath, false, true)

const project = `
VERSION 1.0

PROGRAM Main
    BLOCK Code LANG=PLCASM
        exit
    END_BLOCK
END_PROGRAM
`

console.log('Testing project without VOVKPLCPROJECT header:')
const problems = runtime.lintProject(project)
console.log('lintProject problems:', JSON.stringify(problems, null, 2))

const result = runtime.compileProject(project)
console.log('compileProject result:', JSON.stringify(result, null, 2))
