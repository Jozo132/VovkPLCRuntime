// test_ladder_graph.js - Test the new Ladder Graph to STL compiler
// Tests the graph format: { nodes: [...], connections: [...] }

import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import {fileURLToPath} from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')

async function test() {
    const runtime = await VovkPLC.createWorker(wasmPath)
    console.log('VovkPLCRuntime WASM loaded successfully\n')

    console.log('='.repeat(70))
    console.log('Ladder Graph to STL Compiler Tests')
    console.log('='.repeat(70))

    let passed = 0
    let failed = 0

    // Helper to write string to stream
    const writeToStream = async str => {
        for (let i = 0; i < str.length; i++) {
            await runtime.callExport('streamIn', str.charCodeAt(i))
        }
        await runtime.callExport('streamIn', 0) // Null terminator
    }

    async function runTest(name, graphJson, expectedPatterns) {
        console.log(`\nTest: ${name}`)
        console.log('-'.repeat(50))

        // Clear stream and write graph JSON
        await runtime.callExport('streamClear')
        await writeToStream(JSON.stringify(graphJson))

        // Load from stream and compile
        await runtime.callExport('ladder_graph_load_from_stream')
        const success = await runtime.callExport('ladder_graph_compile')

        if (!success) {
            // Read error - call error_to_stream which writes to streamOut (captured by readStream)
            await runtime.callExport('ladder_graph_error_to_stream')
            const error = await runtime.readStream()
            console.log(`FAILED: Compilation error: ${error}`)
            failed++
            return
        }

        // Get the output - output_to_stream writes to streamOut (captured by readStream)
        await runtime.callExport('ladder_graph_output_to_stream')
        const stl = await runtime.readStream()

        console.log('Generated STL:')
        console.log(stl)

        // Check for expected patterns
        let allFound = true
        for (const pattern of expectedPatterns) {
            if (!stl.includes(pattern)) {
                console.log(`FAILED: Expected pattern not found: "${pattern}"`)
                allFound = false
            }
        }

        if (allFound) {
            console.log('PASSED')
            passed++
        } else {
            failed++
        }
    }

    if (false) { // TEMP SKIP
    // Test 1: Simple series circuit - contact to coil
    await runTest(
        'Simple series (contact -> coil)',
        {
            nodes: [
                {id: 'n1', type: 'contact_no', address: 'X0.0', x: 0, y: 0},
                {id: 'n2', type: 'coil', address: 'Y0.0', x: 100, y: 0},
            ],
            connections: [{sources: ['n1'], destinations: ['n2']}],
        },
        ['A X0.0', '= Y0.0'],
    )

    // Test 2: Two contacts in series
    await runTest(
        'Two contacts in series',
        {
            nodes: [
                {id: 'n1', type: 'contact_no', address: 'X0.0', x: 0, y: 0},
                {id: 'n2', type: 'contact_no', address: 'X0.1', x: 50, y: 0},
                {id: 'n3', type: 'coil', address: 'Y0.0', x: 100, y: 0},
            ],
            connections: [
                {sources: ['n1'], destinations: ['n2']},
                {sources: ['n2'], destinations: ['n3']},
            ],
        },
        ['A X0.0', 'A X0.1', '= Y0.0'],
    )

    // Test 3: OR logic - multiple sources to one destination
    await runTest(
        'OR logic (two sources -> one coil)',
        {
            nodes: [
                {id: 'n1', type: 'contact_no', address: 'X0.0', x: 0, y: 0},
                {id: 'n2', type: 'contact_no', address: 'X0.1', x: 0, y: 50},
                {id: 'n3', type: 'coil', address: 'Y0.0', x: 100, y: 25},
            ],
            connections: [{sources: ['n1', 'n2'], destinations: ['n3']}],
        },
        ['A X0.0', 'O X0.1', '= Y0.0'],
    )

    // Test 4: Parallel outputs - one source to multiple destinations
    await runTest(
        'Parallel outputs (one contact -> two coils)',
        {
            nodes: [
                {id: 'n1', type: 'contact_no', address: 'X0.0', x: 0, y: 0},
                {id: 'n2', type: 'coil', address: 'Y0.0', x: 100, y: 0},
                {id: 'n3', type: 'coil', address: 'Y0.1', x: 100, y: 50},
            ],
            connections: [{sources: ['n1'], destinations: ['n2', 'n3']}],
        },
        ['A X0.0', 'SAVE', 'L BR', '= Y0.0', 'L BR', '= Y0.1', 'DROP BR'],
    )

    // Test 5: Normally closed contact
    await runTest(
        'Normally closed contact',
        {
            nodes: [
                {id: 'n1', type: 'contact_nc', address: 'X0.0', x: 0, y: 0},
                {id: 'n2', type: 'coil', address: 'Y0.0', x: 100, y: 0},
            ],
            connections: [{sources: ['n1'], destinations: ['n2']}],
        },
        ['AN X0.0', '= Y0.0'],
    )

    // Test 6: Set coil
    await runTest(
        'Set coil',
        {
            nodes: [
                {id: 'n1', type: 'contact_no', address: 'X0.0', x: 0, y: 0},
                {id: 'n2', type: 'coil_set', address: 'Y0.0', x: 100, y: 0},
            ],
            connections: [{sources: ['n1'], destinations: ['n2']}],
        },
        ['A X0.0', 'S Y0.0'],
    )

    // Test 7: Reset coil
    await runTest(
        'Reset coil',
        {
            nodes: [
                {id: 'n1', type: 'contact_no', address: 'X0.0', x: 0, y: 0},
                {id: 'n2', type: 'coil_rset', address: 'Y0.0', x: 100, y: 0},
            ],
            connections: [{sources: ['n1'], destinations: ['n2']}],
        },
        ['A X0.0', 'R Y0.0'],
    )

    // Test 8: Timer ON delay
    await runTest(
        'Timer ON delay',
        {
            nodes: [
                {id: 'n1', type: 'contact_no', address: 'X0.0', x: 0, y: 0},
                {id: 'n2', type: 'timer_on', address: 'T0', preset: 1000, x: 50, y: 0},
                {id: 'n3', type: 'coil', address: 'Y0.0', x: 100, y: 0},
            ],
            connections: [
                {sources: ['n1'], destinations: ['n2']},
                {sources: ['n2'], destinations: ['n3']},
            ],
        },
        ['A X0.0', 'TON T0', '= Y0.0'],
    )

    // Test 9: Counter up
    await runTest(
        'Counter up',
        {
            nodes: [
                {id: 'n1', type: 'contact_no', address: 'X0.0', x: 0, y: 0},
                {id: 'n2', type: 'counter_up', address: 'C0', preset: 10, x: 50, y: 0},
                {id: 'n3', type: 'coil', address: 'Y0.0', x: 100, y: 0},
            ],
            connections: [
                {sources: ['n1'], destinations: ['n2']},
                {sources: ['n2'], destinations: ['n3']},
            ],
        },
        ['A X0.0', 'CTU C0', '= Y0.0'],
    )

    // Test 10: TAP instruction (passthrough coil connected to next contact)
    // When a coil has an outgoing connection to a contact, TAP is automatically inserted AFTER the coil
    await runTest(
        'Passthrough coil (auto TAP)',
        {
            nodes: [
                {id: 'n1', type: 'contact_no', address: 'X0.0', x: 0, y: 0},
                {id: 'n2', type: 'coil', address: 'Y0.0', x: 50, y: 0},
                {id: 'n3', type: 'contact_no', address: 'X0.1', x: 100, y: 0},
                {id: 'n4', type: 'coil', address: 'Y0.1', x: 150, y: 0},
            ],
            connections: [
                {sources: ['n1'], destinations: ['n2']},
                {sources: ['n2'], destinations: ['n3']},
                {sources: ['n3'], destinations: ['n4']},
            ],
        },
        ['A X0.0', '= Y0.0', 'TAP', 'A X0.1', '= Y0.1'],
    )

    // Test 11: Edge detection (positive)
    await runTest(
        'Positive edge contact',
        {
            nodes: [
                {id: 'n1', type: 'contact_pos', address: 'X0.0', x: 0, y: 0},
                {id: 'n2', type: 'coil', address: 'Y0.0', x: 100, y: 0},
            ],
            connections: [{sources: ['n1'], destinations: ['n2']}],
        },
        ['A X0.0', 'FP M', '= Y0.0'],
    )

    // Test 12: Edge detection (negative)
    await runTest(
        'Negative edge contact',
        {
            nodes: [
                {id: 'n1', type: 'contact_neg', address: 'X0.0', x: 0, y: 0},
                {id: 'n2', type: 'coil', address: 'Y0.0', x: 100, y: 0},
            ],
            connections: [{sources: ['n1'], destinations: ['n2']}],
        },
        ['A X0.0', 'FN M', '= Y0.0'],
    )

    // Test 13: Complex - OR to parallel outputs
    await runTest(
        'Complex: OR sources to parallel outputs',
        {
            nodes: [
                {id: 'n1', type: 'contact_no', address: 'X0.0', x: 0, y: 0},
                {id: 'n2', type: 'contact_no', address: 'X0.1', x: 0, y: 50},
                {id: 'n3', type: 'coil', address: 'Y0.0', x: 100, y: 0},
                {id: 'n4', type: 'coil', address: 'Y0.1', x: 100, y: 50},
            ],
            connections: [{sources: ['n1', 'n2'], destinations: ['n3', 'n4']}],
        },
        ['A X0.0', 'O X0.1', 'SAVE', 'L BR', '= Y0.0', 'L BR', '= Y0.1', 'DROP BR'],
    )
    } // END TEMP SKIP

    // Test 14: Complex real-world example (from actual ladder JSON)
    const test_14_ladder = {
        id: 'A8uv4BZRuX10w',
        type: 'ladder',
        name: 'Network 4',
        comment: '',
        connections: [
            {id: 'conn_0', sources: ['FGftxzdjGzIdZ', 'ysdkdW5WHrNZj'], destinations: ['OZQBBuLgLyiau']},
            {id: 'conn_1', sources: ['JydGjS4LvzorQ', 'X4WcTremO8j3i'], destinations: ['csSZopeZpDOCu']},
            {id: 'conn_2', sources: ['yq0ydd3f7tt93'], destinations: ['NHX3yKS6ek2zP', 'Wl2ZZ1Iqp5caB']},
            {id: 'conn_3', sources: ['OZQBBuLgLyiau'], destinations: ['QvVMbac1MkaGa']},
            {id: 'conn_4', sources: ['QvVMbac1MkaGa', 'RKyNApYAcuNUU', 'ZzoDAT0mRQaW0'], destinations: ['yq0ydd3f7tt93']},
            {id: 'conn_5', sources: ['csSZopeZpDOCu'], destinations: ['ZzoDAT0mRQaW0']},
            {id: 'conn_6', sources: ['qXA0LSGu23Kd5'], destinations: ['RKyNApYAcuNUU']},
            {id: 'conn_Wl2ZZ1Iqp5caB_i4pbhWAXINbxw', sources: ['Wl2ZZ1Iqp5caB'], destinations: ['i4pbhWAXINbxw']},
        ],
        minimized: false,
        nodes: [
            {id: 'ysdkdW5WHrNZj', x: 0, y: 0, type: 'contact', inverted: false, trigger: 'normal', symbol: 'X0.0'},
            {id: 'OZQBBuLgLyiau', x: 3, y: 0, type: 'contact', inverted: false, trigger: 'normal', symbol: 'M0.0'},
            {id: 'FGftxzdjGzIdZ', x: 0, y: 1, type: 'contact', inverted: false, trigger: 'normal', symbol: 'Y0.1'},
            {id: 'qXA0LSGu23Kd5', x: 0, y: 4, type: 'contact', inverted: false, trigger: 'normal', symbol: 'Y0.2'},
            {id: 'NHX3yKS6ek2zP', x: 9, y: 1, type: 'coil', inverted: false, trigger: 'normal', symbol: 'Y0.4'},
            {id: 'JydGjS4LvzorQ', x: 0, y: 2, type: 'contact', inverted: false, trigger: 'normal', symbol: 'X0.1'},
            {id: 'csSZopeZpDOCu', x: 3, y: 2, type: 'contact', inverted: false, trigger: 'normal', symbol: 'M0.2'},
            {id: 'X4WcTremO8j3i', x: 0, y: 3, type: 'contact', inverted: false, trigger: 'normal', symbol: 'Y0.3'},
            {id: 'yq0ydd3f7tt93', x: 7, y: 0, type: 'contact', inverted: true, trigger: 'normal', symbol: 'M0.1'},
            {id: 'QvVMbac1MkaGa', x: 4, y: 0, type: 'timer_ton', inverted: false, trigger: 'normal', symbol: 'T0', preset: 'T#10s'},
            {id: 'Wl2ZZ1Iqp5caB', x: 9, y: 0, type: 'coil', inverted: false, trigger: 'normal', symbol: 'Y0.1'},
            {id: 'ZzoDAT0mRQaW0', x: 4, y: 2, type: 'timer_ton', inverted: false, trigger: 'normal', symbol: 'T1', preset: 'T#3s'},
            {id: 'RKyNApYAcuNUU', x: 5, y: 4, type: 'coil_rset', inverted: false, trigger: 'normal', symbol: 'Y0.1'},
            {id: 'i4pbhWAXINbxw', x: 10, y: 0, type: 'coil', inverted: false, trigger: 'normal', symbol: 'Y0.2'},
        ],
    }
    await runTest('Complex real-world ladder diagram', test_14_ladder, [
        // Timer T0 network: (X0.0 OR Y0.1) → M0.0 → T0
        'A X0.0',
        'O Y0.1', // OR input
        'A M0.0', // M0.0 contact
        'TON T0, T#10s', // Timer T0

        // Timer T1 network: (X0.1 OR Y0.3) → M0.2 → T1
        'A X0.1',
        'O Y0.3', // OR input
        'A M0.2', // M0.2 contact
        'TON T1, T#3s', // Timer T1

        // Reset rung: Y0.2 → R Y0.1
        'A Y0.2',
        'R Y0.1',

        // Merge point: (T0 OR R_Y0.1 OR T1) → M0.1 NC
        'AN M0.1', // M0.1 NC (inverted contact)

        // Parallel outputs: M0.1 NC → (Y0.4, Y0.1 → Y0.2)
        '= Y0.4', // Output branch 1
        '= Y0.1', // Output branch 2 (with passthrough)
        'TAP', // TAP for passthrough
        '= Y0.2', // Passthrough to Y0.2
    ])

    if (false) {
        // TEMP SKIP PIPELINE TEST
        // Test 15: Full pipeline test - compile to bytecode
        console.log('\nTest: Full pipeline (Graph -> STL -> PLCASM -> Bytecode)')
        console.log('-'.repeat(50))

        const pipelineGraph = {
            nodes: [
                {id: 'n1', type: 'contact_no', address: 'X0.0', x: 0, y: 0},
                {id: 'n2', type: 'coil', address: 'Y0.0', x: 100, y: 0},
            ],
            connections: [{sources: ['n1'], destinations: ['n2']}],
        }

        await runtime.callExport('streamClear')
        await writeToStream(JSON.stringify(pipelineGraph))
        await runtime.callExport('ladder_graph_load_from_stream')

        const fullSuccess = await runtime.callExport('ladder_graph_compile_full')
        if (fullSuccess) {
            console.log('Full pipeline compilation succeeded!')
            // Extract the program to verify bytecode was generated
            const extracted = await runtime.extractProgram()
            if (extracted && extracted.size > 0) {
                console.log(`Bytecode size: ${extracted.size} bytes`)
                console.log(`Bytecode: ${extracted.output.substring(0, 50)}...`)
                passed++
            } else {
                console.log('FAILED: Bytecode extraction returned empty')
                failed++
            }
        } else {
            console.log('FAILED: Full pipeline compilation failed')
            await runtime.callExport('ladder_graph_error_to_stream')
            const stlError = await runtime.readStream()
            if (stlError) console.log(`  Graph error: ${stlError}`)
            failed++
        }
    } // END TEMP SKIP PIPELINE TEST

    // Summary
    console.log('\n' + '='.repeat(70))
    console.log(`Results: ${passed} passed, ${failed} failed`)
    console.log('='.repeat(70))

    await runtime.terminate()
    process.exit(failed > 0 ? 1 : 0)
}

test().catch(err => {
    console.error('Test error:', err)
    process.exit(1)
})
