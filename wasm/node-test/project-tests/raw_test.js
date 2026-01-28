#!/usr/bin/env node
// raw_test.js - Raw output viewer for project test samples
//
// This script outputs the raw compilation or linting result
// for a project test sample without any formatting or comparison.
//
// Usage:
//   node raw_test.js <test_name> --compile   # Show raw compilation output
//   node raw_test.js <test_name> --lint      # Show raw linter output
//   node raw_test.js <test_name> --ir        # Show intermediate representation (symbols, memory, blocks)
//   node raw_test.js <test_name> --modifiers # Show modifiable values for hot-patching
//
// Examples:
//   node raw_test.js test_01 --compile
//   node raw_test.js test_01 --lint
//   node raw_test.js test_01 --ir
//   node raw_test.js test_01 --modifiers
//
// @ts-check
'use strict'

import VovkPLC from '../../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import {fileURLToPath} from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

// ANSI colors
const RED = '\x1b[31m'
const GREEN = '\x1b[32m'
const YELLOW = '\x1b[33m'
const CYAN = '\x1b[36m'
const RESET = '\x1b[0m'
const BOLD = '\x1b[1m'

function printUsage() {
    console.log(`
${BOLD}Usage:${RESET}
  node raw_test.js <test_name> --compile   Show raw compilation output
  node raw_test.js <test_name> --lint      Show raw linter output
  node raw_test.js <test_name> --ir        Show intermediate representation (symbols, memory, blocks)
  node raw_test.js <test_name> --modifiers Show modifiable values for hot-patching

${BOLD}Examples:${RESET}
  node raw_test.js test_01 --compile
  node raw_test.js test_01 --lint
  node raw_test.js test_01 --ir
  node raw_test.js test_01 --modifiers
  node raw_test.js test_01_base --compile
`)
}

async function main() {
    const args = process.argv.slice(2)
    
    // Parse arguments
    const compileMode = args.includes('--compile') || args.includes('-c')
    const lintMode = args.includes('--lint') || args.includes('-l')
    const irMode = args.includes('--ir') || args.includes('-i')
    const modifiersMode = args.includes('--modifiers') || args.includes('-m')
    const testName = args.find(arg => !arg.startsWith('-'))
    
    if (!testName) {
        console.error(`${RED}Error: No test name provided${RESET}`)
        printUsage()
        process.exit(1)
    }
    
    const modeCount = [compileMode, lintMode, irMode, modifiersMode].filter(Boolean).length
    if (modeCount === 0) {
        console.error(`${RED}Error: Must specify --compile, --lint, --ir, or --modifiers${RESET}`)
        printUsage()
        process.exit(1)
    }
    
    if (modeCount > 1) {
        console.error(`${RED}Error: Cannot specify multiple modes${RESET}`)
        printUsage()
        process.exit(1)
    }
    
    // Find the project file
    const samplesDir = path.join(__dirname, 'samples')
    const cleanName = testName.replace('.project', '').replace('.output', '')
    let projectPath = path.join(samplesDir, `${cleanName}.project`)
    
    if (!fs.existsSync(projectPath)) {
        // Try without _base suffix or similar variations
        const files = fs.readdirSync(samplesDir)
        const match = files.find(f => f.startsWith(cleanName) && f.endsWith('.project'))
        if (match) {
            projectPath = path.join(samplesDir, match)
        } else {
            console.error(`${RED}Error: Project file not found: ${projectPath}${RESET}`)
            console.error(`Available tests: ${files.filter(f => f.endsWith('.project')).map(f => f.replace('.project', '')).join(', ')}`)
            process.exit(1)
        }
    }
    
    // Read project code
    const projectCode = fs.readFileSync(projectPath, 'utf8')
    
    // Initialize runtime
    const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')
    if (!fs.existsSync(wasmPath)) {
        console.error(`${RED}Error: WASM file not found at ${wasmPath}${RESET}`)
        console.error('Run "npm run build" first.')
        process.exit(1)
    }
    
    const runtime = new VovkPLC(wasmPath)
    runtime.stdout_callback = () => {}  // Suppress runtime output
    await runtime.initialize(wasmPath, false, false)
    
    const modeName = compileMode ? 'Compilation' : lintMode ? 'Linter' : irMode ? 'IR' : 'Modifiers'
    console.log(`${CYAN}─── ${modeName} Output for ${cleanName} ───${RESET}`)
    console.log()
    
    if (compileMode) {
        // Compile the project
        const result = runtime.compileProject(projectCode)
        console.log(JSON.stringify(result, null, 2))
    } else if (lintMode) {
        // Lint the project
        const problems = runtime.lintProject(projectCode)
        console.log(JSON.stringify(problems, null, 2))
    } else if (irMode) {
        // Show intermediate representation
        const ir = runtime.getProjectIR(projectCode)
        console.log(JSON.stringify(ir, null, 2))
    } else if (modifiersMode) {
        // Show modifiable values
        const result = runtime.getProjectIRModifiers(projectCode)
        console.log(JSON.stringify(result, null, 2))
    }
}

main().catch(err => {
    console.error(`${RED}Error: ${err.message}${RESET}`)
    process.exit(1)
})
