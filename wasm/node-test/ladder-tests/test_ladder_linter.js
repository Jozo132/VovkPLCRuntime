#!/usr/bin/env node
// test_ladder_linter.js - Automated Ladder Linter unit tests
//
// This script tests the Ladder Linter's ability to detect various
// validation errors and warnings in ladder diagrams.
//
// Usage:
//   node wasm/node-test/test_ladder_linter.js

import VovkPLC from '../../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)
const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')

// ANSI colors
const RED = '\x1b[31m'
const GREEN = '\x1b[32m'
const YELLOW = '\x1b[33m'
const CYAN = '\x1b[36m'
const RESET = '\x1b[0m'
const BOLD = '\x1b[1m'

// Test case definitions
const testCases = [
    {
        name: 'valid_simple_ladder',
        ladder: {
            nodes: [
                { id: 'n1', type: 'contact_no', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'coil', x: 2, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n2'] }
            ]
        },
        expectErrors: 0,
        expectWarnings: 0
    },
    {
        name: 'duplicate_node_positions',
        ladder: {
            nodes: [
                { id: 'n1', type: 'contact_no', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'contact_nc', x: 0, y: 0, address: 'X0.1' },
                { id: 'n3', type: 'coil', x: 2, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1', 'n2'], destinations: ['n3'] }
            ]
        },
        expectErrors: 1,
        expectWarnings: 0
    },
    {
        name: 'backward_connection',
        ladder: {
            nodes: [
                { id: 'n1', type: 'contact_no', x: 2, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'coil', x: 1, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n2'] }
            ]
        },
        expectErrors: 1,
        expectWarnings: 0
    },
    {
        name: 'connection_to_nonexistent_node',
        ladder: {
            nodes: [
                { id: 'n1', type: 'contact_no', x: 0, y: 0, address: 'X0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n99'] }
            ]
        },
        expectErrors: 1,
        expectWarnings: 0
    },
    {
        name: 'unknown_node_type',
        ladder: {
            nodes: [
                { id: 'n1', type: 'unknown_type', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'coil', x: 2, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n2'] }
            ]
        },
        expectErrors: 1,
        expectWarnings: 0
    },
    {
        name: 'disconnected_node',
        ladder: {
            nodes: [
                { id: 'n1', type: 'contact_no', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'contact_no', x: 0, y: 1, address: 'X0.1' },
                { id: 'n3', type: 'coil', x: 2, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n3'] }
            ]
        },
        expectErrors: 1,
        expectWarnings: 1
    },
    {
        name: 'missing_address',
        ladder: {
            nodes: [
                { id: 'n1', type: 'contact_no', x: 0, y: 0, address: '' },
                { id: 'n2', type: 'coil', x: 2, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n2'] }
            ]
        },
        expectErrors: 1,
        expectWarnings: 0
    },
    {
        name: 'timer_without_preset',
        ladder: {
            nodes: [
                { id: 'n1', type: 'contact_no', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'timer_ton', x: 1, y: 0, address: 'T0' },
                { id: 'n3', type: 'coil', x: 2, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n2'] },
                { sources: ['n2'], destinations: ['n3'] }
            ]
        },
        expectErrors: 0,
        expectWarnings: 1
    },
    {
        name: 'operation_block_missing_inputs',
        ladder: {
            nodes: [
                { id: 'n1', type: 'contact_no', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'fb_add', x: 1, y: 0, address: 'M0', in1: '', in2: '', out: '' },
                { id: 'n3', type: 'coil', x: 2, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n2'] },
                { sources: ['n2'], destinations: ['n3'] }
            ]
        },
        expectErrors: 3,
        expectWarnings: 0
    },
    {
        name: 'contact_not_connected_to_output',
        ladder: {
            nodes: [
                { id: 'n1', type: 'contact_no', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'coil', x: 2, y: 0, address: 'Y0.0' }
            ],
            connections: []
        },
        expectErrors: 2,
        expectWarnings: 2
    }
]

async function runTests() {
    if (!fs.existsSync(wasmPath)) {
        console.error(`${RED}Error: WASM file not found at ${wasmPath}${RESET}`)
        console.error('Run "npm run build" first.')
        process.exit(1)
    }

    // Initialize runtime
    const runtime = await VovkPLC.createWorker(wasmPath, { silent: true })

    console.log(`${BOLD}${CYAN}╔══════════════════════════════════════════════════════════════════╗${RESET}`)
    console.log(`${BOLD}${CYAN}║              Ladder Linter Unit Tests                            ║${RESET}`)
    console.log(`${BOLD}${CYAN}╚══════════════════════════════════════════════════════════════════╝${RESET}`)
    console.log()
    console.log(`Found ${testCases.length} test case(s)`)
    console.log()

    let passed = 0
    let failed = 0
    const failures = []

    try {
        for (const testCase of testCases) {
            try {
                const result = await runtime.lintLadder(testCase.ladder)

                const errors = result.problems.filter(p => p.type === 'error')
                const warnings = result.problems.filter(p => p.type === 'warning')

                const errorsMatch = errors.length === testCase.expectErrors
                const warningsMatch = warnings.length >= testCase.expectWarnings

                if (errorsMatch && warningsMatch) {
                    // Build status suffix
                    let suffix = ''
                    if (errors.length > 0 || warnings.length > 0) {
                        const parts = []
                        if (errors.length > 0) parts.push(`${errors.length} error${errors.length > 1 ? 's' : ''}`)
                        if (warnings.length > 0) parts.push(`${warnings.length} warning${warnings.length > 1 ? 's' : ''}`)
                        suffix = ` ${CYAN}(${parts.join(', ')})${RESET}`
                    }
                    console.log(`${GREEN}✓${RESET} ${testCase.name}${suffix}`)
                    passed++
                } else {
                    console.log(`${RED}✗${RESET} ${testCase.name}`)
                    failed++
                    failures.push({
                        name: testCase.name,
                        expected: { errors: testCase.expectErrors, warnings: testCase.expectWarnings },
                        actual: { errors: errors.length, warnings: warnings.length },
                        problems: result.problems
                    })
                }
            } catch (e) {
                console.log(`${RED}✗${RESET} ${testCase.name} - Exception: ${e.message}`)
                failed++
                failures.push({
                    name: testCase.name,
                    error: e.message
                })
            }
        }

        console.log()
        console.log('─'.repeat(68))

        // Show detailed failures
        if (failures.length > 0) {
            console.log()
            console.log(`${BOLD}${RED}Failed Tests:${RESET}`)
            console.log()

            for (const failure of failures) {
                console.log(`${BOLD}${failure.name}:${RESET}`)
                if (failure.error) {
                    console.log(`  ${RED}Exception: ${failure.error}${RESET}`)
                } else {
                    console.log(`  Expected: ${failure.expected.errors} errors, ${failure.expected.warnings}+ warnings`)
                    console.log(`  Actual:   ${failure.actual.errors} errors, ${failure.actual.warnings} warnings`)
                    if (failure.problems && failure.problems.length > 0) {
                        console.log(`  Problems:`)
                        for (const p of failure.problems) {
                            const color = p.type === 'error' ? RED : p.type === 'warning' ? YELLOW : CYAN
                            console.log(`    ${color}[${p.type}]${RESET} (${p.line}:${p.column}) ${p.message}`)
                        }
                    }
                }
                console.log()
            }
        }

        // Summary
        const total = passed + failed
        const statusColor = failed > 0 ? RED : GREEN
        console.log()
        console.log(`${BOLD}Results: ${statusColor}${passed}/${total} tests passed${RESET}`)

        process.exitCode = failed > 0 ? 1 : 0
    } finally {
        await runtime.terminate()
    }
}

runTests().catch(err => {
    console.error(`${RED}Error: ${err.message}${RESET}`)
    process.exit(1)
})
