#!/usr/bin/env node
// test_project_compilation_api.js - Integration test for Project Compiler API
//
// This script tests both direct VovkPLC and VovkPLC.worker APIs to verify:
//   1. API parity between direct and worker modes
//   2. Compilation correctness (same bytecode output)
//   3. Performance comparison between direct and worker compilation
//
// Usage:
//   node test_project_compilation_api.js [--iterations N]
//
// Examples:
//   node test_project_compilation_api.js              # Run with default 100 iterations
//   node test_project_compilation_api.js --iterations 1000  # Run with 1000 iterations

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
const DIM = '\x1b[2m'

// Parse command line arguments
function parseArgs() {
    const args = process.argv.slice(2)
    let iterations = 100
    
    for (let i = 0; i < args.length; i++) {
        if (args[i] === '--iterations' && args[i + 1]) {
            iterations = parseInt(args[i + 1], 10)
            if (isNaN(iterations) || iterations < 1) iterations = 100
        }
    }
    
    return { iterations }
}

// Assert helper
function assert(condition, message) {
    if (!condition) {
        throw new Error(message || 'Assertion failed')
    }
}


// Compare two results for equality
function compareResults(direct, worker) {
    const diffs = []
    
    // Compare success/error state
    const directSuccess = !direct.problem
    const workerSuccess = !worker.problem
    
    if (directSuccess !== workerSuccess) {
        diffs.push({
            field: 'success',
            direct: directSuccess,
            worker: workerSuccess
        })
    }
    
    // Compare bytecode
    if (direct.bytecode && worker.bytecode) {
        const directHex = direct.bytecode
        const workerHex = worker.bytecode
        if (directHex !== workerHex) {
            diffs.push({
                field: 'bytecode',
                direct: directHex,
                worker: workerHex
            })
        }
    } else if (direct.bytecode || worker.bytecode) {
        diffs.push({
            field: 'bytecode',
            direct: direct.bytecode || null,
            worker: worker.bytecode || null
        })
    }
    
    // Compare error messages if both failed
    if (direct.problem && worker.problem) {
        if (direct.problem.message !== worker.problem.message) {
            diffs.push({
                field: 'error.message',
                direct: direct.problem.message,
                worker: worker.problem.message
            })
        }
        if (direct.problem.line !== worker.problem.line) {
            diffs.push({
                field: 'error.line',
                direct: direct.problem.line,
                worker: worker.problem.line
            })
        }
    }
    
    return diffs
}

async function main() {
    const { iterations } = parseArgs()
    const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')
    const projectPath = path.join(__dirname, 'test_01_base.project')
    
    console.log(`${BOLD}${CYAN}╔══════════════════════════════════════════════════════════════════╗${RESET}`)
    console.log(`${BOLD}${CYAN}║          Project Compiler API Integration Test                   ║${RESET}`)
    console.log(`${BOLD}${CYAN}╚══════════════════════════════════════════════════════════════════╝${RESET}`)
    console.log()
    
    // Check prerequisites
    if (!fs.existsSync(wasmPath)) {
        console.error(`${RED}Error: WASM file not found at ${wasmPath}${RESET}`)
        console.error('Run "npm run build" first.')
        process.exit(1)
    }
    
    if (!fs.existsSync(projectPath)) {
        console.error(`${RED}Error: Test project file not found at ${projectPath}${RESET}`)
        process.exit(1)
    }
    
    const projectSource = fs.readFileSync(projectPath, 'utf8')
    console.log(`${DIM}Test project: test_01_base.project (${projectSource.length} chars)${RESET}`)
    console.log(`${DIM}Iterations: ${iterations}${RESET}`)
    console.log()
    
    // =========================================================================
    // Initialize Direct Runtime
    // =========================================================================
    console.log(`${BOLD}[1] Initializing Direct Runtime...${RESET}`)
    const direct = new VovkPLC(wasmPath)
    direct.stdout_callback = () => {} // Suppress output
    await direct.initialize(wasmPath, false, true)
    console.log(`    ${GREEN}✓${RESET} Direct runtime initialized`)
    
    // =========================================================================
    // Initialize Worker Runtime
    // =========================================================================
    console.log(`${BOLD}[2] Initializing Worker Runtime...${RESET}`)
    const worker = await VovkPLC.createWorker(wasmPath, { silent: true, debug: false })
    console.log(`    ${GREEN}✓${RESET} Worker runtime initialized`)
    
    // =========================================================================
    // API Parity Test - Single Compilation
    // =========================================================================
    console.log()
    console.log(`${BOLD}[3] API Parity Test - Single Compilation...${RESET}`)
    
    const directResult = direct.compileProject(projectSource)
    const workerResult = await worker.compileProject(projectSource)

    const directBytecode = directResult.bytecode.split(' ')
    const workerBytecode = workerResult.bytecode.split(' ')
    
    console.log(`    Direct: ${directResult.problem ? RED + 'ERROR' + RESET : GREEN + 'OK' + RESET}` +
        (directBytecode ? ` (${directBytecode.length} bytes)` : ''))
    console.log(`    Worker: ${workerResult.problem ? RED + 'ERROR' + RESET : GREEN + 'OK' + RESET}` +
        (workerBytecode ? ` (${workerBytecode.length} bytes)` : ''))
    
    const diffs = compareResults(directResult, workerResult)
    
    if (diffs.length > 0) {
        console.log(`    ${RED}✗ API mismatch detected:${RESET}`)
        for (const d of diffs) {
            console.log(`      ${d.field}:`)
            console.log(`        direct: ${JSON.stringify(d.direct)}`)
            console.log(`        worker: ${JSON.stringify(d.worker)}`)
        }
        process.exit(1)
    } else {
        console.log(`    ${GREEN}✓${RESET} Results match - API parity verified`)
    }
    
    // Show compilation output summary
    if (directResult.bytecode) {
        console.log()
        console.log(`    ${DIM}Bytecode: ${directBytecode.slice(0, 20).join(' ')}${directBytecode.length > 20 ? '...' : ''}${RESET}`)
    }
    
    // =========================================================================
    // Performance Comparison - Direct vs Worker
    // =========================================================================
    console.log()
    console.log(`${BOLD}[4] Performance Comparison (${iterations} iterations)...${RESET}`)
    
    // Warm up
    console.log(`    ${DIM}Warming up...${RESET}`)
    for (let i = 0; i < 10; i++) {
        direct.compileProject(projectSource)
        await worker.compileProject(projectSource)
    }
    
    // Benchmark Direct
    console.log(`    Benchmarking Direct runtime...`)
    const directTimes = []
    const directStart = performance.now()
    for (let i = 0; i < iterations; i++) {
        const start = performance.now()
        direct.compileProject(projectSource)
        directTimes.push(performance.now() - start)
    }
    const directTotal = performance.now() - directStart
    
    // Benchmark Worker
    console.log(`    Benchmarking Worker runtime...`)
    const workerTimes = []
    const workerStart = performance.now()
    for (let i = 0; i < iterations; i++) {
        const start = performance.now()
        await worker.compileProject(projectSource)
        workerTimes.push(performance.now() - start)
    }
    const workerTotal = performance.now() - workerStart
    
    // Calculate statistics
    const calcStats = (times) => {
        const sorted = [...times].sort((a, b) => a - b)
        const sum = times.reduce((a, b) => a + b, 0)
        return {
            min: sorted[0],
            max: sorted[sorted.length - 1],
            avg: sum / times.length,
            median: sorted[Math.floor(sorted.length / 2)],
            p95: sorted[Math.floor(sorted.length * 0.95)],
            p99: sorted[Math.floor(sorted.length * 0.99)]
        }
    }
    
    const directStats = calcStats(directTimes)
    const workerStats = calcStats(workerTimes)
    
    console.log()
    console.log(`    ${BOLD}Results:${RESET}`)
    console.log()
    console.log(`    ${''.padEnd(8)}    ${CYAN}Direct${RESET}       ${YELLOW}Worker${RESET}      ${DIM}Ratio${RESET}`)
    console.log(`    ${'─'.repeat(45)}`)
    console.log(`    ${'Total:'.padEnd(8)} ${directTotal.toFixed(2).padStart(8)} ms   ${workerTotal.toFixed(2).padStart(8)} ms   ${(workerTotal / directTotal).toFixed(2)}x`)
    console.log(`    ${'Average:'.padEnd(8)} ${directStats.avg.toFixed(3).padStart(8)} ms   ${workerStats.avg.toFixed(3).padStart(8)} ms   ${(workerStats.avg / directStats.avg).toFixed(2)}x`)
    console.log(`    ${'Median:'.padEnd(8)} ${directStats.median.toFixed(3).padStart(8)} ms   ${workerStats.median.toFixed(3).padStart(8)} ms   ${(workerStats.median / directStats.median).toFixed(2)}x`)
    console.log(`    ${'Min:'.padEnd(8)} ${directStats.min.toFixed(3).padStart(8)} ms   ${workerStats.min.toFixed(3).padStart(8)} ms`)
    console.log(`    ${'Max:'.padEnd(8)} ${directStats.max.toFixed(3).padStart(8)} ms   ${workerStats.max.toFixed(3).padStart(8)} ms`)
    console.log(`    ${'P95:'.padEnd(8)} ${directStats.p95.toFixed(3).padStart(8)} ms   ${workerStats.p95.toFixed(3).padStart(8)} ms`)
    console.log(`    ${'P99:'.padEnd(8)} ${directStats.p99.toFixed(3).padStart(8)} ms   ${workerStats.p99.toFixed(3).padStart(8)} ms`)
    console.log()
    console.log(`    ${DIM}Compilations/sec: Direct=${(1000 / directStats.avg).toFixed(0)}, Worker=${(1000 / workerStats.avg).toFixed(0)}${RESET}`)
    
    // =========================================================================
    // API Method Coverage Test
    // =========================================================================
    console.log()
    console.log(`${BOLD}[5] API Method Coverage Test...${RESET}`)
    
    const apiMethods = [
        { name: 'compileProject', async: true },
        { name: 'lintProject', async: true },
        { name: 'compile', async: true },
        { name: 'compilePLCASM', async: true },
        { name: 'compileSTL', async: true },
        { name: 'compileLadder', async: true },
        { name: 'lint', async: true },
        { name: 'lintSTL', async: true },
    ]
    
    let methodsAvailable = 0
    let methodsMissing = 0
    
    for (const method of apiMethods) {
        const directHas = typeof direct[method.name] === 'function'
        const workerHas = typeof worker[method.name] === 'function'
        
        if (directHas && workerHas) {
            console.log(`    ${GREEN}✓${RESET} ${method.name}`)
            methodsAvailable++
        } else if (directHas && !workerHas) {
            console.log(`    ${YELLOW}!${RESET} ${method.name} - ${YELLOW}missing in Worker${RESET}`)
            methodsMissing++
        } else if (!directHas && workerHas) {
            console.log(`    ${YELLOW}!${RESET} ${method.name} - ${YELLOW}missing in Direct${RESET}`)
            methodsMissing++
        } else {
            console.log(`    ${RED}✗${RESET} ${method.name} - ${RED}missing in both${RESET}`)
            methodsMissing++
        }
    }
    
    console.log()
    console.log(`    ${methodsAvailable}/${apiMethods.length} methods available in both APIs`)
    
    // =========================================================================
    // Cleanup
    // =========================================================================
    console.log()
    console.log(`${BOLD}[6] Cleanup...${RESET}`)
    worker.terminate()
    console.log(`    ${GREEN}✓${RESET} Worker terminated`)
    
    // =========================================================================
    // Summary
    // =========================================================================
    console.log()
    console.log(`${'─'.repeat(68)}`)
    console.log()
    console.log(`${BOLD}${GREEN}All tests passed!${RESET}`)
    console.log()
    console.log(`${DIM}Summary:${RESET}`)
    console.log(`  • API parity: ${GREEN}verified${RESET}`)
    console.log(`  • Bytecode match: ${GREEN}identical${RESET}`)
    console.log(`  • Worker overhead: ~${(workerStats.avg / directStats.avg).toFixed(1)}x slower (message passing)`)
    console.log(`  • Direct throughput: ${(1000 / directStats.avg).toFixed(0)} compilations/sec`)
    console.log(`  • Worker throughput: ${(1000 / workerStats.avg).toFixed(0)} compilations/sec`)
    console.log()
    
    process.exit(0)
}

main().catch(err => {
    console.error(`${RED}Error: ${err.message}${RESET}`)
    console.error(err.stack)
    process.exit(1)
})
