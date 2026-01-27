// test_lint.js
import VovkPLC from '../../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import {fileURLToPath} from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

const run = async () => {
    // Path to WASM file
    const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')
    console.log(`Loading WASM from: ${wasmPath}`)

    if (!fs.existsSync(wasmPath)) {
        console.error('WASM file not found!')
        process.exit(1)
    }

    const runtime = await VovkPLC.createWorker(wasmPath, { silent: true })
    try {
        console.log('Runtime initialized. Testing linting with buggy assembly...')

        // Buggy Assembly Test (from test_9_simulator_linter.cpp)
        const assembly = `
################# Linter Test Assembly

# Error 1: Typo in instruction
u8.const 10
u8.typomistake 

# Error 2: Missing operand
u8.const 

# Error 3: Jump to undefined label
jmp undefined_label

`

        console.log('----------------------------------------')
        console.log('Assembly Code to Lint (with intentional errors):')
        console.log(assembly)
        console.log('----------------------------------------')

        // Lint the assembly
        console.log('Linting assembly...')
        const t_start = performance.now()
        const problems = await runtime.lintPLCASM(assembly)
        const t_end = performance.now()
        console.log(`Linting completed in ${(t_end - t_start).toFixed(2)} ms`)

        if (problems.length > 0) {
            console.log('Linting detected errors:')
            problems.forEach((problem, index) => {
                console.log(`  ${index + 1}. [${problem.type.toUpperCase()}] Line ${problem.line}, Column ${problem.column}: ${problem.message} (${problem.token_text})`)
            })
            console.log('SUCCESS: Linter detected errors in buggy assembly.')
        } else {
            throw new Error('Linting unexpectedly found no errors!')
        }
    } finally {
        await runtime.terminate()
    }
}

run().catch(err => {
    console.error('Unhandled Error:', err)
    process.exit(1)
})
