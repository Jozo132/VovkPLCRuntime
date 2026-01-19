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
    
    // Test 9: Symbol definitions are parsed
    {
        const testName = 'Symbol definitions are parsed'
        const stl = `$$ START_BTN | bit | I0.0
$$ MOTOR_OUT | bit | Q0.0
A START_BTN
= MOTOR_OUT
`
        try {
            const result = await runtime.lintSTL(stl)
            // Should compile successfully and resolve symbols
            if (result.problems.length === 0 && result.output.includes('X0.0') && result.output.includes('Y0.0')) {
                console.log(`✓ ${testName}`)
                passed++
            } else {
                console.log(`✗ ${testName}: Symbol resolution failed`)
                console.log(`  Output: ${result.output}`)
                if (result.problems.length > 0) {
                    result.problems.forEach(p => console.log(`  - [${p.type}] Line ${p.line}: ${p.message}`))
                }
                failed++
            }
        } catch (e) {
            console.log(`✗ ${testName}: ${e.message}`)
            failed++
        }
    }
    
    // Test 10: Duplicate symbol name error
    {
        const testName = 'Duplicate symbol name error'
        const stl = `$$ MOTOR | bit | Q0.0
$$ MOTOR | bit | Q0.1
A I0.0
= MOTOR
`
        try {
            const result = await runtime.lintSTL(stl)
            const hasDuplicateError = result.problems.some(p => p.type === 'error' && p.message.includes('Duplicate'))
            if (hasDuplicateError) {
                console.log(`✓ ${testName}`)
                passed++
            } else {
                console.log(`✗ ${testName}: Expected duplicate symbol error`)
                result.problems.forEach(p => console.log(`  - [${p.type}] Line ${p.line}: ${p.message}`))
                failed++
            }
        } catch (e) {
            console.log(`✗ ${testName}: ${e.message}`)
            failed++
        }
    }
    
    // Test 11: Invalid symbol type error
    {
        const testName = 'Invalid symbol type error'
        const stl = `$$ VAR1 | invalid_type | M0
A I0.0
= Q0.0
`
        try {
            const result = await runtime.lintSTL(stl)
            const hasTypeError = result.problems.some(p => p.type === 'error' && p.message.includes('type'))
            if (hasTypeError) {
                console.log(`✓ ${testName}`)
                passed++
            } else {
                console.log(`✗ ${testName}: Expected invalid type error`)
                result.problems.forEach(p => console.log(`  - [${p.type}] Line ${p.line}: ${p.message}`))
                failed++
            }
        } catch (e) {
            console.log(`✗ ${testName}: ${e.message}`)
            failed++
        }
    }
    
    // Test 12: Symbol with byte type
    {
        const testName = 'Symbol with byte type'
        const stl = `$$ COUNTER_VAL | u16 | M10
L COUNTER_VAL
`
        try {
            const result = await runtime.lintSTL(stl)
            // Should not have errors
            if (result.problems.filter(p => p.type === 'error').length === 0) {
                console.log(`✓ ${testName}`)
                passed++
            } else {
                console.log(`✗ ${testName}: Unexpected error`)
                result.problems.forEach(p => console.log(`  - [${p.type}] Line ${p.line}: ${p.message}`))
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
