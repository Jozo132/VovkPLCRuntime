// Test adjacency and power rail warnings in ladder linter
import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')

const RED = '\x1b[31m'
const GREEN = '\x1b[32m'
const YELLOW = '\x1b[33m'
const CYAN = '\x1b[36m'
const RESET = '\x1b[0m'

async function runTests() {
    const runtime = await VovkPLC.createWorker(wasmPath, { silent: true })

    try {
        // Test 1: Two adjacent nodes not connected (warning expected)
        console.log('Test 1: Adjacent nodes not connected')
        const result1 = await runtime.lintLadder({
            nodes: [
                { id: 'n1', type: 'contact', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'contact', x: 1, y: 0, address: 'X0.1' },  // Adjacent to n1 but no connection
                { id: 'n3', type: 'coil', x: 3, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n3'] },
                { sources: ['n2'], destinations: ['n3'] }
            ]
        })
        console.log('Problems:', result1.problems.length)
        result1.problems.forEach(p => {
            const color = p.type === 'error' ? RED : p.type === 'warning' ? YELLOW : CYAN
            console.log(`  ${color}[${p.type}]${RESET} (${p.line}:${p.column}) ${p.message}`)
        })

        // Test 2: Input node not at power rail and no input connection (warning expected)
        console.log('\nTest 2: Input node not at power rail')
        const result2 = await runtime.lintLadder({
            nodes: [
                { id: 'n1', type: 'contact', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'contact', x: 2, y: 0, address: 'X0.1' },  // Not at x=0 and no input
                { id: 'n3', type: 'coil', x: 4, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n3'] },
                { sources: ['n2'], destinations: ['n3'] }
            ]
        })
        console.log('Problems:', result2.problems.length)
        result2.problems.forEach(p => {
            const color = p.type === 'error' ? RED : p.type === 'warning' ? YELLOW : CYAN
            console.log(`  ${color}[${p.type}]${RESET} (${p.line}:${p.column}) ${p.message}`)
        })

        // Test 3: Operation block not at power rail without input (warning expected)
        console.log('\nTest 3: Operation block not at power rail')
        const result3 = await runtime.lintLadder({
            nodes: [
                { id: 'n1', type: 'contact', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'fb_add', x: 2, y: 0, in1: 'M0', in2: 'M1', out: 'M2' },
                { id: 'n3', type: 'coil', x: 4, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n3'] }
            ]
        })
        console.log('Problems:', result3.problems.length)
        result3.problems.forEach(p => {
            const color = p.type === 'error' ? RED : p.type === 'warning' ? YELLOW : CYAN
            console.log(`  ${color}[${p.type}]${RESET} (${p.line}:${p.column}) ${p.message}`)
        })

        // Test 4: Valid case - adjacent AND connected (no adjacency warning)
        console.log('\nTest 4: Adjacent and connected (should have no adjacency warnings)')
        const result4 = await runtime.lintLadder({
            nodes: [
                { id: 'n1', type: 'contact', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'contact', x: 1, y: 0, address: 'X0.1' },
                { id: 'n3', type: 'coil', x: 2, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n2'] },
                { sources: ['n2'], destinations: ['n3'] }
            ]
        })
        console.log('Problems:', result4.problems.length)
        result4.problems.forEach(p => {
            const color = p.type === 'error' ? RED : p.type === 'warning' ? YELLOW : CYAN
            console.log(`  ${color}[${p.type}]${RESET} (${p.line}:${p.column}) ${p.message}`)
        })

        // Test 5: Comparator not at power rail without input (warning expected)
        console.log('\nTest 5: Comparator not at power rail')
        const result5 = await runtime.lintLadder({
            nodes: [
                { id: 'n1', type: 'contact', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'cmp_eq', x: 2, y: 0, in1: 'M0', in2: 'M1' },
                { id: 'n3', type: 'coil', x: 4, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n3'] },
                { sources: ['n2'], destinations: ['n3'] }
            ]
        })
        console.log('Problems:', result5.problems.length)
        result5.problems.forEach(p => {
            const color = p.type === 'error' ? RED : p.type === 'warning' ? YELLOW : CYAN
            console.log(`  ${color}[${p.type}]${RESET} (${p.line}:${p.column}) ${p.message}`)
        })

        // Test 6: Vertically adjacent but not connected
        console.log('\nTest 6: Vertically adjacent nodes not connected')
        const result6 = await runtime.lintLadder({
            nodes: [
                { id: 'n1', type: 'contact', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'contact', x: 0, y: 1, address: 'X0.1' },
                { id: 'n3', type: 'coil', x: 2, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n3'] },
                { sources: ['n2'], destinations: ['n3'] }
            ]
        })
        console.log('Problems:', result6.problems.length)
        result6.problems.forEach(p => {
            const color = p.type === 'error' ? RED : p.type === 'warning' ? YELLOW : CYAN
            console.log(`  ${color}[${p.type}]${RESET} (${p.line}:${p.column}) ${p.message}`)
        })

        console.log(`\n${GREEN}=== All tests completed ===${RESET}`)

    } finally {
        await runtime.terminate()
    }
}

runTests().catch(err => {
    console.error(`${RED}Error: ${err.message}${RESET}`)
    process.exit(1)
})
