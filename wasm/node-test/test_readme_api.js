// test_readme_api.js - Verify README API example is up-to-date and works
// This test mirrors the JavaScript/WASM usage example from README.md

import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')

const GREEN = '\x1b[32m'
const RED = '\x1b[31m'
const RESET = '\x1b[0m'

let passed = 0
let failed = 0

const test = (name, condition, details = '') => {
    if (condition) {
        console.log(`${GREEN}✓${RESET} ${name}`)
        passed++
    } else {
        console.log(`${RED}✗${RESET} ${name}`)
        if (details) console.log(`  ${details}`)
        failed++
    }
}

const main = async () => {
    console.log('='.repeat(70))
    console.log('README API Example Verification Test')
    console.log('='.repeat(70))
    console.log('')

    // =========================================================================
    // Test the exact API usage shown in README.md
    // =========================================================================

    // Simple assembly for testing
    const assembly = `
        // Simple test: push 10, push 20, add = 30
        u8.const 10
        u8.const 20
        u8.add
        exit
    `

    let stdoutMessages = []

    try {
        // -----------------------------------------------------------------
        // README Example Line: const worker = await VovkPLC.createWorker(...)
        // -----------------------------------------------------------------
        console.log('[1] Testing VovkPLC.createWorker()...')
        const worker = await VovkPLC.createWorker(wasmPath)
        test('VovkPLC.createWorker() returns worker instance', worker !== null && worker !== undefined)
        test('worker has expected methods', 
            typeof worker.onStdout === 'function' &&
            typeof worker.lintPLCASM === 'function' &&
            typeof worker.downloadAssembly === 'function' &&
            typeof worker.run === 'function' &&
            typeof worker.callExport === 'function' &&
            typeof worker.getExports === 'function' &&
            typeof worker.terminate === 'function'
        )

        // -----------------------------------------------------------------
        // README Example Line: worker.onStdout(msg => console.log(msg))
        // -----------------------------------------------------------------
        console.log('\n[2] Testing worker.onStdout()...')
        worker.onStdout(msg => {
            stdoutMessages.push(msg)
        })
        test('worker.onStdout() accepts callback', true) // If it didn't throw, it worked

        // -----------------------------------------------------------------
        // README Example Line: const problems = await worker.lintPLCASM(assembly)
        // -----------------------------------------------------------------
        console.log('\n[3] Testing worker.lintPLCASM()...')
        const problems = await worker.lintPLCASM(assembly)
        test('worker.lintPLCASM() returns array', Array.isArray(problems))
        test('worker.lintPLCASM() finds no errors in valid assembly', problems.length === 0, 
            problems.length > 0 ? `Found ${problems.length} problems` : '')

        // Test with intentionally buggy assembly
        const buggyAssembly = `
            u8.const 10
            u8.invalid_instruction
        `
        const buggyProblems = await worker.lintPLCASM(buggyAssembly)
        test('worker.lintPLCASM() detects errors in invalid assembly', buggyProblems.length > 0)

        // -----------------------------------------------------------------
        // README Example Line: await worker.downloadAssembly(assembly)
        // -----------------------------------------------------------------
        console.log('\n[4] Testing worker.downloadAssembly()...')
        const downloadResult = await worker.downloadAssembly(assembly)
        // downloadAssembly returns false on success (no error), true on error
        test('worker.downloadAssembly() succeeds', downloadResult === false)

        // -----------------------------------------------------------------
        // README Example Line: await worker.run()
        // -----------------------------------------------------------------
        console.log('\n[5] Testing worker.run()...')
        const runResult = await worker.run()
        test('worker.run() executes without error', runResult !== undefined)

        // -----------------------------------------------------------------
        // README Example Line: await worker.callExport('run_unit_test')
        // -----------------------------------------------------------------
        console.log('\n[6] Testing worker.callExport()...')
        try {
            // Call run_unit_test exactly as shown in README
            const unitTestResult = await worker.callExport('run_unit_test')
            test('worker.callExport(\'run_unit_test\') works', unitTestResult !== undefined)
        } catch (e) {
            test('worker.callExport(\'run_unit_test\') works', false, e.message)
        }

        // -----------------------------------------------------------------
        // README Example Line: const exports = await worker.getExports()
        // -----------------------------------------------------------------
        console.log('\n[7] Testing worker.getExports()...')
        const exports = await worker.getExports()
        test('worker.getExports() returns array', Array.isArray(exports))
        test('worker.getExports() contains expected functions', 
            exports.includes('run') && 
            exports.includes('run_unit_test') && 
            exports.includes('initialize'),
            `Found ${exports.length} exports`
        )

        // -----------------------------------------------------------------
        // README Example Line: await worker.terminate()
        // -----------------------------------------------------------------
        console.log('\n[8] Testing worker.terminate()...')
        await worker.terminate()
        test('worker.terminate() completes without error', true)

    } catch (error) {
        console.error(`${RED}Test failed with error:${RESET}`, error.message)
        failed++
    }

    // =========================================================================
    // Summary
    // =========================================================================
    console.log('')
    console.log('='.repeat(70))
    console.log(`Results: ${GREEN}${passed} passed${RESET}, ${failed > 0 ? RED : ''}${failed} failed${RESET}`)
    console.log('='.repeat(70))

    if (failed > 0) {
        console.log(`\n${RED}WARNING: README API example may be out of date!${RESET}`)
        console.log('Please update README.md to match the current API.')
        process.exit(1)
    } else {
        console.log(`\n${GREEN}README API example is up-to-date and working.${RESET}`)
    }
}

main().catch(err => {
    console.error('Test error:', err)
    process.exit(1)
})
