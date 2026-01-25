#!/usr/bin/env node
// test_ladder_stl.js - Automated Ladder Graph to STL unit tests
//
// This script tests the Ladder Graph → STL compiler by comparing
// the generated STL output against expected .stl files.
//
// Usage:
//   node wasm/node-test/ladder-tests/test_ladder_stl.js
//   npm run test:ladder
//
// Test files:
//   - test_XX.json - Ladder Graph JSON input
//   - test_XX.stl  - Expected STL output

import VovkPLC from '../../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

// ANSI colors
const RED = '\x1b[31m'
const GREEN = '\x1b[32m'
const YELLOW = '\x1b[33m'
const CYAN = '\x1b[36m'
const RESET = '\x1b[0m'
const BOLD = '\x1b[1m'

// Stream code to runtime (synchronous for direct VovkPLC)
function streamCode(runtime, code) {
    for (let i = 0; i < code.length; i++) {
        runtime.wasm_exports.streamIn(code.charCodeAt(i))
    }
    runtime.wasm_exports.streamIn(0)
}

// Normalize STL for comparison (trim lines, remove empty lines, normalize whitespace)
function normalizeSTL(stl) {
    return stl
        .split('\n')
        .map(line => line.trim())
        .filter(line => line.length > 0)
        .join('\n')
}

// Compare two STL strings and show differences using unified diff style
function compareSTL(expected, actual) {
    const expectedLines = expected.split('\n').map(l => l.trim()).filter(l => l)
    const actualLines = actual.split('\n').map(l => l.trim()).filter(l => l)
    
    // Build a simple unified diff
    const diff = []
    const maxLen = Math.max(expectedLines.length, actualLines.length)
    
    // Find lines only in expected (removed)
    for (let i = 0; i < expectedLines.length; i++) {
        if (!actualLines.includes(expectedLines[i])) {
            diff.push({ type: '-', line: expectedLines[i], lineNum: i + 1 })
        }
    }
    
    // Find lines only in actual (added)  
    for (let i = 0; i < actualLines.length; i++) {
        if (!expectedLines.includes(actualLines[i])) {
            diff.push({ type: '+', line: actualLines[i], lineNum: i + 1 })
        }
    }
    
    return diff
}

async function runTests() {
    const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')
    
    if (!fs.existsSync(wasmPath)) {
        console.error(`${RED}Error: WASM file not found at ${wasmPath}${RESET}`)
        console.error('Run "npm run build" first.')
        process.exit(1)
    }

    // Initialize runtime
    const runtime = new VovkPLC(wasmPath)
    
    // Suppress stdout during initialization
    runtime.stdout_callback = () => {}
    await runtime.initialize(wasmPath, false, false)
    runtime.readStream()
    
    // Find all test files
    const testDir = __dirname
    const files = fs.readdirSync(testDir)
    const testCases = files
        .filter(f => f.match(/^test_\d+\.json$/))
        .map(f => f.replace('.json', ''))
        .sort()
    
    if (testCases.length === 0) {
        console.error(`${RED}No test cases found in ${testDir}${RESET}`)
        process.exit(1)
    }
    
    console.log(`${BOLD}${CYAN}╔══════════════════════════════════════════════════════════════════╗${RESET}`)
    console.log(`${BOLD}${CYAN}║          Ladder Graph → STL Compiler Unit Tests                 ║${RESET}`)
    console.log(`${BOLD}${CYAN}╚══════════════════════════════════════════════════════════════════╝${RESET}`)
    console.log()
    console.log(`Found ${testCases.length} test case(s)`)
    console.log()
    
    let passed = 0
    let failed = 0
    const failures = []
    
    for (const testCase of testCases) {
        const jsonPath = path.join(testDir, `${testCase}.json`)
        const stlPath = path.join(testDir, `${testCase}.stl`)
        
        const ladderJson = fs.readFileSync(jsonPath, 'utf8')
        
        try {
            // Compile ladder graph to STL
            streamCode(runtime, ladderJson)
            runtime.wasm_exports.ladder_graph_load_from_stream()
            
            const success = runtime.wasm_exports.ladder_graph_compile()
            
            if (!success) {
                console.log(`${RED}✗${RESET} ${testCase} - Ladder compilation failed`)
                failed++
                failures.push({ name: testCase, error: 'Ladder compilation failed' })
                continue
            }
            
            runtime.wasm_exports.ladder_graph_output_to_stream()
            const actualSTL = runtime.readStream()
            
            // Check if expected STL file exists - if not, generate it
            if (!fs.existsSync(stlPath)) {
                fs.writeFileSync(stlPath, actualSTL)
                console.log(`${CYAN}+${RESET} ${testCase} - Generated expected .stl file`)
                passed++
                continue
            }
            
            const expectedSTL = fs.readFileSync(stlPath, 'utf8')
            
            // Compare normalized STL
            const normalizedExpected = normalizeSTL(expectedSTL)
            const normalizedActual = normalizeSTL(actualSTL)
            
            if (normalizedExpected === normalizedActual) {
                console.log(`${GREEN}✓${RESET} ${testCase}`)
                passed++
            } else {
                console.log(`${RED}✗${RESET} ${testCase} - STL output mismatch`)
                const diffs = compareSTL(expectedSTL, actualSTL)
                failures.push({ name: testCase, diffs, expected: expectedSTL, actual: actualSTL })
                failed++
            }
        } catch (e) {
            console.log(`${RED}✗${RESET} ${testCase} - Error: ${e.message}`)
            failed++
            failures.push({ name: testCase, error: e.message })
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
                console.log(`  Error: ${failure.error}`)
            } else if (failure.diffs && failure.diffs.length > 0) {
                console.log(`  Changes:`)
                // Show removed lines (in expected but not actual)
                const removed = failure.diffs.filter(d => d.type === '-')
                const added = failure.diffs.filter(d => d.type === '+')
                
                if (removed.length > 0) {
                    console.log(`    ${RED}Missing (expected but not in output):${RESET}`)
                    for (const d of removed.slice(0, 10)) {
                        console.log(`      ${RED}- ${d.line}${RESET}`)
                    }
                    if (removed.length > 10) console.log(`      ... and ${removed.length - 10} more`)
                }
                
                if (added.length > 0) {
                    console.log(`    ${YELLOW}Extra (in output but not expected):${RESET}`)
                    for (const d of added.slice(0, 10)) {
                        console.log(`      ${YELLOW}+ ${d.line}${RESET}`)
                    }
                    if (added.length > 10) console.log(`      ... and ${added.length - 10} more`)
                }
            } else if (failure.expected && failure.actual) {
                console.log(`  Expected:\n${failure.expected}`)
                console.log(`  Actual:\n${failure.actual}`)
            }
            console.log()
        }
    }
    
    // Summary
    const total = passed + failed
    const statusColor = failed > 0 ? RED : GREEN
    console.log()
    console.log(`${BOLD}Results: ${statusColor}${passed}/${total} tests passed${RESET}`)
    
    if (failed > 0) {
        process.exit(1)
    }
}

runTests().catch(err => {
    console.error(`${RED}Error: ${err.message}${RESET}`)
    process.exit(1)
})
