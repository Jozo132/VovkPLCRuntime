// test_project.js - Tests for VovkPLC.compileProject() method
import VovkPLC from '../dist/VovkPLC.js'
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

const run = async () => {
    // Path to WASM file
    const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')
    console.log(`Loading WASM from: ${wasmPath}`)

    if (!fs.existsSync(wasmPath)) {
        console.error('WASM file not found!')
        process.exit(1)
    }

    const runtime = new VovkPLC()
    await runtime.initialize(wasmPath, false, true) // (path, debug, silent)

    console.log('')
    console.log('╔══════════════════════════════════════════════════════════════════╗')
    console.log('║                VovkPLC.compileProject() Tests                    ║')
    console.log('╚══════════════════════════════════════════════════════════════════╝')
    console.log('')

    let passed = 0
    let failed = 0

    // Test 1: Simple PLCASM project
    {
        const testName = 'Simple PLCASM project'
        const project = `
VOVKPLCPROJECT SimpleTest
VERSION 1.0

PROGRAM Main
    BLOCK Code LANG=PLCASM
        u8.const 42
        exit
    END_BLOCK
END_PROGRAM
`
        const result = runtime.compileProject(project)

        if (result.bytecode && !result.problem) {
            // Expected: 03 2A FF (PUSH u8 42, EXIT)
            if (result.bytecode.includes('03 2A') && result.bytecode.includes('FF')) {
                console.log(`${GREEN}✓${RESET} ${testName}`)
                console.log(`  Bytecode: ${result.bytecode}`)
                passed++
            } else {
                console.log(`${RED}✗${RESET} ${testName} - unexpected bytecode`)
                console.log(`  Got: ${result.bytecode}`)
                failed++
            }
        } else {
            console.log(`${RED}✗${RESET} ${testName} - compilation failed`)
            console.log(`  Error: ${result.problem?.message}`)
            failed++
        }
    }

    // Test 2: STL block
    {
        const testName = 'STL block compilation'
        const project = `
VOVKPLCPROJECT STLTest
VERSION 1.0

PROGRAM Main
    BLOCK Code LANG=STL
        A X0.0
        = Y0.0
    END_BLOCK
END_PROGRAM
`
        const result = runtime.compileProject(project)

        if (result.bytecode && !result.problem) {
            console.log(`${GREEN}✓${RESET} ${testName}`)
            console.log(`  Bytecode: ${result.bytecode}`)
            passed++
        } else {
            console.log(`${RED}✗${RESET} ${testName} - compilation failed`)
            console.log(`  Error: ${result.problem?.message}`)
            failed++
        }
    }

    // Test 3: Ladder block
    {
        const testName = 'Ladder block compilation'
        const project = `
VOVKPLCPROJECT LadderTest
VERSION 1.0

PROGRAM Main
    BLOCK Code LANG=LADDER
        {"nodes":[{"id":"n1","type":"contact","address":"X0.0","x":0,"y":0},{"id":"n2","type":"coil","address":"Y0.0","x":1,"y":0}],"connections":[{"sources":["n1"],"destinations":["n2"]}]}
    END_BLOCK
END_PROGRAM
`
        const result = runtime.compileProject(project)

        if (result.bytecode && !result.problem) {
            console.log(`${GREEN}✓${RESET} ${testName}`)
            console.log(`  Bytecode: ${result.bytecode}`)
            passed++
        } else {
            console.log(`${RED}✗${RESET} ${testName} - compilation failed`)
            console.log(`  Error: ${result.problem?.message}`)
            failed++
        }
    }

    // Test 4: Multi-block project with symbols
    {
        const testName = 'Multi-block project with symbols'
        const project = `
VOVKPLCPROJECT MultiBlock
VERSION 1.0

SYMBOLS
    StartButton : BOOL : X0.0 : Start input
    Motor : BOOL : Y0.0 : Motor output
END_SYMBOLS

PROGRAM Main
    BLOCK Block1 LANG=STL
        A StartButton
        = Motor
    END_BLOCK
    
    BLOCK Block2 LANG=PLCASM
        u8.const 10
        u8.const 20
        u8.add
        u8.drop
    END_BLOCK
END_PROGRAM
`
        const result = runtime.compileProject(project)

        if (result.bytecode && !result.problem) {
            console.log(`${GREEN}✓${RESET} ${testName}`)
            console.log(`  Bytecode: ${result.bytecode}`)
            passed++
        } else {
            console.log(`${RED}✗${RESET} ${testName} - compilation failed`)
            console.log(`  Error: ${result.problem?.message}`)
            failed++
        }
    }

    // Test 5: Syntax error - should return problem
    {
        const testName = 'Syntax error returns problem object'
        const project = `
VOVKPLCPROJECT ErrorTest
VERSION 1.0

PROGRAM Main
    BLOCK Code LANG=PLCASM
        invalid_instruction_here
        exit
    END_BLOCK
END_PROGRAM
`
        const result = runtime.compileProject(project)

        if (!result.bytecode && result.problem) {
            console.log(`${GREEN}✓${RESET} ${testName}`)
            console.log(`  Problem: ${result.problem.message} (line ${result.problem.line})`)
            if (result.problem.block) console.log(`  Block: ${result.problem.block}`)
            if (result.problem.compiler) console.log(`  Compiler: ${result.problem.compiler}`)
            passed++
        } else {
            console.log(`${RED}✗${RESET} ${testName} - should have returned an error`)
            console.log(`  Got bytecode: ${result.bytecode}`)
            failed++
        }
    }

    // Test 6: Invalid language
    {
        const testName = 'Invalid language returns problem'
        const project = `
VOVKPLCPROJECT InvalidLang
VERSION 1.0

PROGRAM Main
    BLOCK Code LANG=INVALID
        something
    END_BLOCK
END_PROGRAM
`
        const result = runtime.compileProject(project)

        if (!result.bytecode && result.problem) {
            console.log(`${GREEN}✓${RESET} ${testName}`)
            console.log(`  Problem: ${result.problem.message} (line ${result.problem.line})`)
            passed++
        } else {
            console.log(`${RED}✗${RESET} ${testName} - should have returned an error`)
            failed++
        }
    }

    // Test 7: Empty project - should return problem
    {
        const testName = 'Empty project returns problem'
        const project = `
VOVKPLCPROJECT Empty
VERSION 1.0
`
        const result = runtime.compileProject(project)

        if (!result.bytecode && result.problem) {
            console.log(`${GREEN}✓${RESET} ${testName}`)
            console.log(`  Problem: ${result.problem.message}`)
            passed++
        } else {
            console.log(`${RED}✗${RESET} ${testName} - should have returned an error for empty project`)
            failed++
        }
    }

    // Test 8: Return type shape validation
    {
        const testName = 'Return type has correct shape'
        const successProject = `
VOVKPLCPROJECT ShapeTest
VERSION 1.0

PROGRAM Main
    BLOCK Code LANG=PLCASM
        exit
    END_BLOCK
END_PROGRAM
`
        const successResult = runtime.compileProject(successProject)
        
        // Check success shape: { bytecode: string, problem: null }
        const hasCorrectSuccessShape = 
            typeof successResult.bytecode === 'string' &&
            successResult.problem === null

        const failProject = `
VOVKPLCPROJECT ShapeFailTest
VERSION 1.0

PROGRAM Main
    BLOCK Code LANG=PLCASM
        bad_instruction
    END_BLOCK
END_PROGRAM
`
        const failResult = runtime.compileProject(failProject)
        
        // Check failure shape: { bytecode: null, problem: { message, line, column } }
        const hasCorrectFailShape = 
            failResult.bytecode === null &&
            typeof failResult.problem === 'object' &&
            typeof failResult.problem.message === 'string' &&
            typeof failResult.problem.line === 'number' &&
            typeof failResult.problem.column === 'number'

        if (hasCorrectSuccessShape && hasCorrectFailShape) {
            console.log(`${GREEN}✓${RESET} ${testName}`)
            passed++
        } else {
            console.log(`${RED}✗${RESET} ${testName}`)
            if (!hasCorrectSuccessShape) console.log('  Success result has wrong shape')
            if (!hasCorrectFailShape) console.log('  Failure result has wrong shape')
            failed++
        }
    }

    // ========================================================================
    // lintProject() Tests
    // ========================================================================
    console.log('')
    console.log('╔══════════════════════════════════════════════════════════════════╗')
    console.log('║                VovkPLC.lintProject() Tests                       ║')
    console.log('╚══════════════════════════════════════════════════════════════════╝')
    console.log('')

    // Test 9: Valid project returns empty array
    {
        const testName = 'Valid project returns empty problems array'
        const project = `
VOVKPLCPROJECT ValidProject
VERSION 1.0

PROGRAM Main
    BLOCK Code LANG=PLCASM
        u8.const 42
        exit
    END_BLOCK
END_PROGRAM
`
        const problems = runtime.lintProject(project)

        if (Array.isArray(problems) && problems.length === 0) {
            console.log(`${GREEN}✓${RESET} ${testName}`)
            passed++
        } else {
            console.log(`${RED}✗${RESET} ${testName} - expected empty array`)
            console.log(`  Got: ${JSON.stringify(problems)}`)
            failed++
        }
    }

    // Test 10: Invalid PLCASM returns problem array
    {
        const testName = 'Invalid PLCASM returns problem in array'
        const project = `
VOVKPLCPROJECT InvalidPLCASM
VERSION 1.0

PROGRAM Main
    BLOCK Code LANG=PLCASM
        bad_instruction_here
        exit
    END_BLOCK
END_PROGRAM
`
        const problems = runtime.lintProject(project)

        if (Array.isArray(problems) && problems.length > 0) {
            const p = problems[0]
            if (p.type === 'error' && p.message && typeof p.line === 'number') {
                console.log(`${GREEN}✓${RESET} ${testName}`)
                console.log(`  Problem: ${p.message} (line ${p.line}, col ${p.column})`)
                if (p.block) console.log(`  Block: ${p.block}`)
                if (p.compiler) console.log(`  Compiler: ${p.compiler}`)
                passed++
            } else {
                console.log(`${RED}✗${RESET} ${testName} - problem has wrong shape`)
                console.log(`  Got: ${JSON.stringify(p)}`)
                failed++
            }
        } else {
            console.log(`${RED}✗${RESET} ${testName} - expected array with problem`)
            console.log(`  Got: ${JSON.stringify(problems)}`)
            failed++
        }
    }

    // Test 11: Invalid STL returns problem
    {
        const testName = 'Invalid STL returns problem in array'
        const project = `
VOVKPLCPROJECT InvalidSTL
VERSION 1.0

PROGRAM Main
    BLOCK Code LANG=STL
        INVALID_OPCODE X0.0
        = Y0.0
    END_BLOCK
END_PROGRAM
`
        const problems = runtime.lintProject(project)

        if (Array.isArray(problems) && problems.length > 0) {
            console.log(`${GREEN}✓${RESET} ${testName}`)
            console.log(`  Problem: ${problems[0].message} (line ${problems[0].line})`)
            passed++
        } else {
            console.log(`${RED}✗${RESET} ${testName} - expected array with problem`)
            failed++
        }
    }

    // Test 12: Empty/invalid content returns problem
    {
        const testName = 'Empty content returns problem'
        const project = ``  // Empty string

        const problems = runtime.lintProject(project)

        if (Array.isArray(problems) && problems.length > 0) {
            console.log(`${GREEN}✓${RESET} ${testName}`)
            console.log(`  Problem: ${problems[0].message}`)
            passed++
        } else {
            console.log(`${RED}✗${RESET} ${testName} - expected error for empty content`)
            failed++
        }
    }

    // Test 13: Problem object has correct type field
    {
        const testName = 'Problem object has type field set to error'
        const project = `
VOVKPLCPROJECT TypeTest
VERSION 1.0

PROGRAM Main
    BLOCK Code LANG=PLCASM
        undefined_op
    END_BLOCK
END_PROGRAM
`
        const problems = runtime.lintProject(project)

        if (Array.isArray(problems) && problems.length > 0 && problems[0].type === 'error') {
            console.log(`${GREEN}✓${RESET} ${testName}`)
            passed++
        } else {
            console.log(`${RED}✗${RESET} ${testName} - expected type to be 'error'`)
            console.log(`  Got: ${JSON.stringify(problems[0])}`)
            failed++
        }
    }

    // Test 14: lintProject returns array (not discriminated union like compileProject)
    {
        const testName = 'lintProject always returns array'
        const validProject = `
VOVKPLCPROJECT ArrayTest
VERSION 1.0

PROGRAM Main
    BLOCK Code LANG=PLCASM
        exit
    END_BLOCK
END_PROGRAM
`
        const invalidProject = `
VOVKPLCPROJECT ArrayTest2
VERSION 1.0

PROGRAM Main
    BLOCK Code LANG=PLCASM
        bad
    END_BLOCK
END_PROGRAM
`
        const validResult = runtime.lintProject(validProject)
        const invalidResult = runtime.lintProject(invalidProject)

        if (Array.isArray(validResult) && Array.isArray(invalidResult)) {
            console.log(`${GREEN}✓${RESET} ${testName}`)
            console.log(`  Valid project: ${validResult.length} problems`)
            console.log(`  Invalid project: ${invalidResult.length} problems`)
            passed++
        } else {
            console.log(`${RED}✗${RESET} ${testName}`)
            failed++
        }
    }

    // Summary
    console.log('')
    console.log('────────────────────────────────────────────────────────────────────')
    console.log('')

    const total = passed + failed
    if (failed === 0) {
        console.log(`${GREEN}${BOLD}Results: ${passed}/${total} tests passed${RESET}`)
    } else {
        console.log(`${RED}${BOLD}Results: ${passed}/${total} tests passed, ${failed} failed${RESET}`)
        process.exit(1)
    }
}

run().catch(err => {
    console.error('Unhandled Error:', err)
    process.exit(1)
})
