// test_stl_linter.js - Test STL Linter functionality
import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')

async function runTests() {
    console.log('=== STL Linter Tests ===\n')
    
    const runtime = await VovkPLC.createWorker(wasmPath)
    
    let passed = 0
    let failed = 0
    
    // Test 1: Valid STL code should have no problems
    {
        const testName = 'Valid STL code - no errors'
        const stl = `// Simple valid STL program
A I0.0
= Q0.0
`
        try {
            const result = await runtime.lintSTL(stl)
            if (result.problems.length === 0) {
                console.log(`✓ ${testName}`)
                passed++
            } else {
                console.log(`✗ ${testName}: Expected 0 problems, got ${result.problems.length}`)
                result.problems.forEach(p => console.log(`  - Line ${p.line}: ${p.message}`))
                failed++
            }
        } catch (e) {
            console.log(`✗ ${testName}: ${e.message}`)
            failed++
        }
    }
    
    // Test 2: Multiple instructions on valid code
    {
        const testName = 'Complex valid STL code'
        const stl = `// Network 1: AND logic
A I0.0
A I0.1
= Q0.0

// Network 2: OR logic
A I0.2
O I0.3
= Q0.1

// Timer
A I1.0
TON T0, T#5s
`
        try {
            const result = await runtime.lintSTL(stl)
            if (result.problems.length === 0) {
                console.log(`✓ ${testName}`)
                passed++
            } else {
                console.log(`✗ ${testName}: Expected 0 problems, got ${result.problems.length}`)
                result.problems.forEach(p => console.log(`  - Line ${p.line}: ${p.message}`))
                failed++
            }
        } catch (e) {
            console.log(`✗ ${testName}: ${e.message}`)
            failed++
        }
    }
    
    // Test 3: Unclosed parenthesis should generate warning
    {
        const testName = 'Unclosed parenthesis warning'
        const stl = `A I0.0
A( I0.1
O I0.2
= Q0.0
`
        try {
            const result = await runtime.lintSTL(stl)
            const hasWarning = result.problems.some(p => p.type === 'warning' && p.message.includes('parenthesis'))
            if (hasWarning) {
                console.log(`✓ ${testName}`)
                passed++
            } else {
                console.log(`✗ ${testName}: Expected unclosed parenthesis warning`)
                result.problems.forEach(p => console.log(`  - [${p.type}] Line ${p.line}: ${p.message}`))
                failed++
            }
        } catch (e) {
            console.log(`✗ ${testName}: ${e.message}`)
            failed++
        }
    }
    
    // Test 4: Invalid math operator
    {
        const testName = 'Invalid math operator error'
        const stl = `L #10
+X
= MW0
`
        try {
            const result = await runtime.lintSTL(stl)
            const hasError = result.problems.some(p => p.type === 'error')
            if (hasError) {
                console.log(`✓ ${testName}`)
                passed++
            } else {
                console.log(`✗ ${testName}: Expected error for invalid operator`)
                failed++
            }
        } catch (e) {
            console.log(`✗ ${testName}: ${e.message}`)
            failed++
        }
    }
    
    // Test 5: Missing operand after assignment
    {
        const testName = 'Missing operand after assignment'
        const stl = `A I0.0
=
`
        try {
            const result = await runtime.lintSTL(stl)
            const hasError = result.problems.some(p => p.type === 'error' && p.message.includes('operand'))
            if (hasError) {
                console.log(`✓ ${testName}`)
                passed++
            } else {
                console.log(`✗ ${testName}: Expected missing operand error`)
                result.problems.forEach(p => console.log(`  - [${p.type}] Line ${p.line}: ${p.message}`))
                failed++
            }
        } catch (e) {
            console.log(`✗ ${testName}: ${e.message}`)
            failed++
        }
    }
    
    // Test 6: Get output PLCASM even with warnings
    {
        const testName = 'PLCASM output generated'
        const stl = `A I0.0
= Q0.0
`
        try {
            const result = await runtime.lintSTL(stl)
            if (result.output && result.output.length > 0) {
                console.log(`✓ ${testName}`)
                passed++
            } else {
                console.log(`✗ ${testName}: No PLCASM output generated`)
                failed++
            }
        } catch (e) {
            console.log(`✗ ${testName}: ${e.message}`)
            failed++
        }
    }
    
    // Test 7: Problem positions are correct
    {
        const testName = 'Problem line numbers are accurate'
        const stl = `A I0.0
= Q0.0
+X
A I0.1
`
        try {
            const result = await runtime.lintSTL(stl)
            const errorOnLine3 = result.problems.some(p => p.line === 3)
            if (errorOnLine3) {
                console.log(`✓ ${testName}`)
                passed++
            } else {
                console.log(`✗ ${testName}: Expected error on line 3`)
                result.problems.forEach(p => console.log(`  - Line ${p.line}: ${p.message}`))
                failed++
            }
        } catch (e) {
            console.log(`✗ ${testName}: ${e.message}`)
            failed++
        }
    }
    
    // Test 8: Counters use correct prefix
    {
        const testName = 'Counter instructions lint correctly'
        const stl = `A I0.0
CTU C0, #10, I0.1
`
        try {
            const result = await runtime.lintSTL(stl)
            if (result.problems.length === 0 && result.output.includes('ctu C0')) {
                console.log(`✓ ${testName}`)
                passed++
            } else {
                console.log(`✗ ${testName}: Counter instruction failed`)
                if (result.problems.length > 0) {
                    result.problems.forEach(p => console.log(`  - Line ${p.line}: ${p.message}`))
                }
                failed++
            }
        } catch (e) {
            console.log(`✗ ${testName}: ${e.message}`)
            failed++
        }
    }
    
    await runtime.terminate()
    
    console.log(`\n=== Results: ${passed} passed, ${failed} failed ===`)
    process.exit(failed > 0 ? 1 : 0)
}

runTests().catch(e => {
    console.error('Test error:', e)
    process.exit(1)
})
