#!/usr/bin/env node
// test_wcet.js - WCET (Worst-Case Execution Time) Analysis unit test
//
// Tests the WCET analysis on various PLCASM programs to verify:
// - CFG construction (blocks, edges, loops)
// - Cycle cost estimation (BCET/WCET)
// - Stack depth analysis
// - Dead code detection
// - Loop detection
// - Formatted report output

import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

/**
 * @typedef {Object} TestResult
 * @property {string} name
 * @property {boolean} passed
 * @property {string} [error]
 * @property {string} [info]
 */

export async function runTests(options = {}) {
    const { silent = false, runtime: sharedRuntime = null } = options
    const log = silent ? () => {} : console.log.bind(console)

    let plc = sharedRuntime
    if (!plc) {
        const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')
        plc = new VovkPLC(wasmPath)
        plc.stdout_callback = () => {}
        await plc.initialize(wasmPath, false, true)
    }

    let passed = 0
    let failed = 0
    /** @type {TestResult[]} */
    const testResults = []
    const failures = []

    function assert(condition, testName, message) {
        if (condition) {
            passed++
            testResults.push({ name: testName, passed: true })
        } else {
            failed++
            testResults.push({ name: testName, passed: false, error: message })
            failures.push({ name: testName, error: message })
        }
    }

    // ================================================================
    // Test 1: Simple linear program
    // ================================================================
    {
        plc.compilePLCASM(`
            u8.const 10
            u8.const 20
            u8.add
            exit
        `)
        const r = plc.analyzeWCET('compiled')

        assert(r.bytecode_size > 0, 'Linear: bytecode_size > 0', `bytecode_size was ${r.bytecode_size}`)
        assert(r.instruction_count >= 4, 'Linear: >= 4 instructions', `instruction_count was ${r.instruction_count}`)
        assert(r.block_count >= 1, 'Linear: >= 1 basic block', `block_count was ${r.block_count}`)
        assert(r.loop_count === 0, 'Linear: no loops', `loop_count was ${r.loop_count}`)
        assert(r.bcet_cycles > 0, 'Linear: BCET > 0', `bcet_cycles was ${r.bcet_cycles}`)
        assert(r.wcet_cycles >= r.bcet_cycles, 'Linear: WCET >= BCET', `wcet=${r.wcet_cycles} < bcet=${r.bcet_cycles}`)
        assert(r.unreachable_blocks === 0, 'Linear: no unreachable blocks', `unreachable_blocks was ${r.unreachable_blocks}`)
        assert(!r.warnings.dead_code, 'Linear: no dead_code warning', 'dead_code warning was set')
        assert(!r.warnings.infinite_loop, 'Linear: no infinite_loop warning', 'infinite_loop warning was set')
    }

    // ================================================================
    // Test 2: Conditional branch
    // ================================================================
    {
        plc.compilePLCASM(`
            u8.const 1
            jmp_if 7
            u8.const 0
            exit
            u8.const 1
            exit
        `)
        const r = plc.analyzeWCET('compiled')

        assert(r.block_count >= 2, 'Branch: >= 2 blocks', `block_count was ${r.block_count}`)
        assert(r.edge_count >= 2, 'Branch: >= 2 edges', `edge_count was ${r.edge_count}`)
        assert(r.loop_count === 0, 'Branch: no loops', `loop_count was ${r.loop_count}`)
    }

    // ================================================================
    // Test 3: Backward jump (loop)
    // ================================================================
    {
        plc.compilePLCASM(`
            u8.const 0
            u8.const 1
            u8.add
            jmp 2
        `)
        const r = plc.analyzeWCET('compiled')

        assert(r.loop_count >= 1, 'Loop: detected', `loop_count was ${r.loop_count}`)
        assert(r.warnings.infinite_loop, 'Loop: infinite_loop warning', 'infinite_loop warning not set')
    }

    // ================================================================
    // Test 4: Dead code after unconditional jump
    // ================================================================
    {
        plc.compilePLCASM(`
            jmp 6
            u8.const 99
            exit
        `)
        const r = plc.analyzeWCET('compiled')

        assert(r.unreachable_blocks >= 1, 'DeadCode: unreachable blocks', `unreachable_blocks was ${r.unreachable_blocks}`)
        assert(r.dead_code_bytes > 0, 'DeadCode: dead bytes > 0', `dead_code_bytes was ${r.dead_code_bytes}`)
        assert(r.warnings.dead_code, 'DeadCode: warning set', 'dead_code warning not set')
    }

    // ================================================================
    // Test 5: Opcode frequency table
    // ================================================================
    {
        plc.compilePLCASM(`
            u8.const 1
            u8.const 2
            u8.const 3
            u8.add
            u8.add
            exit
        `)
        const r = plc.analyzeWCET('compiled')

        assert(r.unique_opcodes >= 2, 'OpcodeFreq: >= 2 unique opcodes', `unique_opcodes was ${r.unique_opcodes}`)
        assert(r.opcode_frequency.length > 0, 'OpcodeFreq: table not empty', 'opcode_frequency was empty')
        const u8Entry = r.opcode_frequency.find(e => e.name === 'U8')
        assert(u8Entry && u8Entry.count === 3, 'OpcodeFreq: 3x U8', `U8 count was ${u8Entry ? u8Entry.count : 'not found'}`)
    }

    // ================================================================
    // Test 6: CFG structure consistency
    // ================================================================
    {
        plc.compilePLCASM(`
            u8.const 1
            jmp_if 7
            u8.const 0
            exit
            u8.const 1
            exit
        `)
        const r = plc.analyzeWCET('compiled')

        assert(r.cfg.blocks.length === r.block_count, 'CFG: blocks array matches count', `blocks.length=${r.cfg.blocks.length} != block_count=${r.block_count}`)
        assert(r.cfg.edges.length === r.edge_count, 'CFG: edges array matches count', `edges.length=${r.cfg.edges.length} != edge_count=${r.edge_count}`)
    }

    // ================================================================
    // Test 7: Print report (stdout capture)
    // ================================================================
    {
        plc.compilePLCASM(`
            u8.const 10
            u8.const 20
            u8.add
            exit
        `)
        let captured = ''
        const origCallback = plc.stdout_callback
        const wasSilent = plc.silent
        plc.silent = false
        plc.stdout_callback = (msg) => { captured += msg + '\n' }
        plc.analyzeWCET('compiled', { print: true })
        plc.stdout_callback = origCallback
        plc.silent = wasSilent

        assert(captured.length > 0, 'PrintReport: produces stdout', `output length was ${captured.length}`)
    }

    // ================================================================
    // Test 8: Empty (exit-only) program
    // ================================================================
    {
        plc.compilePLCASM('exit')
        const r = plc.analyzeWCET('compiled')

        assert(r.bytecode_size > 0, 'ExitOnly: bytecode_size > 0', `bytecode_size was ${r.bytecode_size}`)
        assert(r.instruction_count >= 1, 'ExitOnly: >= 1 instruction', `instruction_count was ${r.instruction_count}`)
    }

    return {
        name: 'WCET Static Analysis',
        passed,
        failed,
        total: passed + failed,
        tests: testResults,
        failures
    }
}

// Run directly if executed as main
if (process.argv[1]?.includes('test_wcet')) {
    runTests({ silent: false }).then(result => {
        console.log(`\n${'='.repeat(50)}`)
        console.log(`${result.name}: ${result.passed}/${result.total} passed, ${result.failed} failed`)
        console.log('='.repeat(50))
        if (result.failures.length > 0) {
            console.log('\nFailures:')
            for (const f of result.failures) console.log(`  ✗ ${f.name}: ${f.error}`)
        }
        process.exit(result.failed > 0 ? 1 : 0)
    })
}
