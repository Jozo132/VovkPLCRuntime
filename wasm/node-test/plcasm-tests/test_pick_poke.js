// test_pick_poke.js - Tests for PICK and POKE stack instructions
// PICK: Copy value from stack at byte depth to top
// POKE: Write top value to stack at byte depth

import VovkPLC from '../../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

const run = async () => {
    const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')

    if (!fs.existsSync(wasmPath)) {
        console.error('WASM file not found!')
        process.exit(1)
    }

    const runtime = await VovkPLC.createWorker(wasmPath)

    let passed = 0
    let failed = 0

    async function test(name, assembly, expectedBytecodeIncludes) {
        try {
            await runtime.downloadAssembly(assembly)
            const compileError = await runtime.callExport('compileAssembly', false)
            await runtime.readStream() // Clear output

            if (compileError) {
                throw new Error('Compilation failed')
            }

            // Get bytecode
            await runtime.callExport('uploadProgram')
            const extracted = await runtime.extractProgram()
            const bytecode = extracted.output

            // Verify expected bytecode patterns are present
            for (const expected of expectedBytecodeIncludes) {
                if (!bytecode.includes(expected)) {
                    throw new Error(`Expected bytecode to include "${expected}", got: ${bytecode}`)
                }
            }

            console.log(`✓ ${name}`)
            passed++
        } catch (e) {
            console.log(`✗ ${name}`)
            console.log(`  Error: ${e.message}`)
            failed++
        }
    }

    console.log('╔══════════════════════════════════════════════════════════════════╗')
    console.log('║              PICK and POKE Instruction Tests                     ║')
    console.log('╚══════════════════════════════════════════════════════════════════╝')
    console.log('')

    // Test 1: u32.pick compiles
    // PICK opcode = 0x1C, type_u32 = 0x05, depth = 0x0000
    await test('u32.pick 0 compiles correctly', `
        u32.const 100
        u32.pick 0
        exit
    `, ['1C050000'])

    // Test 2: u32.pick with depth 4
    await test('u32.pick 4 compiles with correct depth', `
        u32.const 100
        u32.const 200
        u32.pick 4
        exit
    `, ['1C050004'])

    // Test 3: u8.pick
    await test('u8.pick 1 compiles correctly', `
        u8.const 10
        u8.const 20
        u8.pick 1
        exit
    `, ['1C030001'])

    // Test 4: u32.poke compiles
    // POKE opcode = 0x1D, type_u32 = 0x05, depth = 0x0000
    await test('u32.poke 0 compiles correctly', `
        u32.const 100
        u32.const 200
        u32.poke 0
        exit
    `, ['1D050000'])

    // Test 5: u8.poke
    await test('u8.poke 1 compiles correctly', `
        u8.const 10
        u8.const 20
        u8.poke 1
        exit
    `, ['1D030001'])

    // Test 6: i16.pick
    await test('i16.pick 0 compiles correctly', `
        i16.const -100
        i16.pick 0
        exit
    `, ['1C080000'])

    console.log('')
    console.log('────────────────────────────────────────────────────────────────────')
    console.log(`Results: ${passed}/${passed + failed} tests passed`)
    console.log('')

    await runtime.terminate()

    if (failed > 0) {
        process.exit(1)
    }
}

run().catch(e => {
    console.error(e)
    process.exit(1)
})
