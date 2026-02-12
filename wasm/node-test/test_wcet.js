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

let passed = 0
let failed = 0

function assert(condition, message) {
    if (condition) {
        passed++
    } else {
        failed++
        console.error(`  FAIL: ${message}`)
    }
}

function section(name) {
    console.log(`\n--- ${name} ---`)
}

const run = async () => {
    const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')
    const plc = new VovkPLC(wasmPath, { silent: true })
    await plc.init()

    // ================================================================
    section('Test 1: Simple linear program')
    // ================================================================
    {
        plc.compilePLCASM(`
            u8.const 10
            u8.const 20
            u8.add
            exit
        `)
        const report = plc.analyzeWCET('compiled')
        console.log(`  bytecode_size=${report.bytecode_size}, instructions=${report.instruction_count}`)
        console.log(`  blocks=${report.block_count}, edges=${report.edge_count}, loops=${report.loop_count}`)
        console.log(`  BCET=${report.bcet_cycles} CU, WCET=${report.wcet_cycles} CU`)
        console.log(`  stack: max=${report.max_stack_depth}, exit=${report.stack_at_exit}`)

        assert(report.bytecode_size > 0, 'bytecode_size should be > 0')
        assert(report.instruction_count >= 4, 'should have at least 4 instructions')
        assert(report.block_count >= 1, 'should have at least 1 basic block')
        assert(report.loop_count === 0, 'linear program should have no loops')
        assert(report.bcet_cycles > 0, 'BCET should be > 0')
        assert(report.wcet_cycles >= report.bcet_cycles, 'WCET should be >= BCET')
        assert(report.unreachable_blocks === 0, 'no unreachable blocks expected')
        assert(!report.warnings.dead_code, 'no dead code expected')
        assert(!report.warnings.infinite_loop, 'no infinite loop expected')
    }

    // ================================================================
    section('Test 2: Program with conditional branch')
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
        const report = plc.analyzeWCET('compiled')
        console.log(`  bytecode_size=${report.bytecode_size}, instructions=${report.instruction_count}`)
        console.log(`  blocks=${report.block_count}, edges=${report.edge_count}, loops=${report.loop_count}`)
        console.log(`  BCET=${report.bcet_cycles} CU, WCET=${report.wcet_cycles} CU`)
        console.log(`  paths=${report.total_paths}`)

        assert(report.block_count >= 2, 'branching should produce at least 2 blocks')
        assert(report.edge_count >= 2, 'branching should produce at least 2 edges')
        assert(report.loop_count === 0, 'no loops expected in branching')
    }

    // ================================================================
    section('Test 3: Program with a backward jump (loop)')
    // ================================================================
    {
        plc.compilePLCASM(`
            u8.const 0
            u8.const 1
            u8.add
            jmp 2
        `)
        const report = plc.analyzeWCET('compiled')
        console.log(`  bytecode_size=${report.bytecode_size}, instructions=${report.instruction_count}`)
        console.log(`  blocks=${report.block_count}, edges=${report.edge_count}, loops=${report.loop_count}`)
        console.log(`  warnings: infinite_loop=${report.warnings.infinite_loop}`)

        assert(report.loop_count >= 1, 'backward jump should create a loop')
        assert(report.warnings.infinite_loop, 'unconditional backward jump should warn of infinite loop')
    }

    // ================================================================
    section('Test 4: Dead code after unconditional jump')
    // ================================================================
    {
        plc.compilePLCASM(`
            jmp 6
            u8.const 99
            exit
        `)
        const report = plc.analyzeWCET('compiled')
        console.log(`  bytecode_size=${report.bytecode_size}, instructions=${report.instruction_count}`)
        console.log(`  blocks=${report.block_count}, unreachable=${report.unreachable_blocks}`)
        console.log(`  dead_code_bytes=${report.dead_code_bytes}`)
        console.log(`  warnings: dead_code=${report.warnings.dead_code}`)

        assert(report.unreachable_blocks >= 1, 'should detect unreachable blocks after unconditional jump')
        assert(report.dead_code_bytes > 0, 'should have dead code bytes')
        assert(report.warnings.dead_code, 'should warn about dead code')
    }

    // ================================================================
    section('Test 5: Opcode frequency table')
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
        const report = plc.analyzeWCET('compiled')
        console.log(`  unique_opcodes=${report.unique_opcodes}`)
        console.log(`  opcode_frequency (top entries):`)
        for (const entry of report.opcode_frequency.slice(0, 5)) {
            console.log(`    ${entry.name}: count=${entry.count}, bcet=${entry.total_bcet}, wcet=${entry.total_wcet}`)
        }

        assert(report.unique_opcodes >= 2, 'should have at least 2 unique opcodes')
        assert(report.opcode_frequency.length > 0, 'opcode frequency table should not be empty')

        const u8Entry = report.opcode_frequency.find(e => e.name === 'U8')
        assert(u8Entry && u8Entry.count === 3, 'should find 3 U8 opcodes')
    }

    // ================================================================
    section('Test 6: CFG structure details')
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
        const report = plc.analyzeWCET('compiled')
        console.log(`  CFG blocks:`)
        for (const block of report.cfg.blocks) {
            console.log(`    [${block.start}-${block.end}] bcet=${block.bcet} wcet=${block.wcet} unreachable=${block.unreachable} stack_delta=${block.stack_delta}`)
        }
        console.log(`  CFG edges:`)
        for (const edge of report.cfg.edges) {
            console.log(`    ${edge.from} -> ${edge.to} type=${edge.type} back=${edge.is_back_edge}`)
        }
        if (report.cfg.loops.length > 0) {
            console.log(`  Loops:`)
            for (const loop of report.cfg.loops) {
                console.log(`    header=${loop.header} back_edge=${loop.back_edge} max_iter=${loop.max_iterations}`)
            }
        }

        assert(report.cfg.blocks.length === report.block_count, 'cfg.blocks length should match block_count')
        assert(report.cfg.edges.length === report.edge_count, 'cfg.edges length should match edge_count')
    }

    // ================================================================
    section('Test 7: Print report (stdout capture)')
    // ================================================================
    {
        plc.compilePLCASM(`
            u8.const 10
            u8.const 20
            u8.add
            exit
        `)
        // Capture stdout output (printf goes through stdout_callback, not streamOut)
        let captured = ''
        const origCallback = plc.stdout_callback
        plc.stdout_callback = (msg) => { captured += msg + '\n' }
        const report = plc.analyzeWCET('compiled', { print: true })
        plc.stdout_callback = origCallback
        console.log(`  Report output length: ${captured.length} chars`)
        if (captured.length > 0) {
            // Show first few lines
            const lines = captured.split('\n').slice(0, 5)
            for (const line of lines) console.log(`  | ${line}`)
            if (captured.split('\n').length > 5) console.log(`  | ... (${captured.split('\n').length} lines total)`)
        }
        assert(captured.length > 0, 'printed report should produce stdout output')
    }

    // ================================================================
    section('Test 8: Empty program')
    // ================================================================
    {
        plc.compilePLCASM('exit')
        const report = plc.analyzeWCET('compiled')
        console.log(`  bytecode_size=${report.bytecode_size}, instructions=${report.instruction_count}`)
        assert(report.bytecode_size > 0, 'even "exit" should have bytecode')
        assert(report.instruction_count >= 1, 'should have at least 1 instruction')
    }

    // ================================================================
    // Summary
    // ================================================================
    console.log(`\n${'='.repeat(50)}`)
    console.log(`WCET Analysis Tests: ${passed} passed, ${failed} failed`)
    console.log('='.repeat(50))

    if (failed > 0) process.exitCode = 1
}

run().catch(err => {
    console.error('Fatal error:', err)
    process.exit(1)
})
