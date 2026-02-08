#!/usr/bin/env node
/**
 * test_timers.js - Timer and counter functional tests
 *
 * Tests deterministic timer/counter behavior using setMillis override:
 * - TON (On-delay timer) with constant preset
 * - TOF (Off-delay timer)
 * - TP  (Pulse timer)
 * - CTU (Count-up counter)
 * - CTD (Count-down counter)
 * - setMillis override and revert behavior
 *
 * All tests use PLCASM assembly compiled at runtime so they are
 * resilient to opcode or bytecode encoding changes.
 */

import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')

/**
 * @typedef {Object} TestResult
 * @property {string} name
 * @property {boolean} passed
 * @property {string} [error]
 */

// Memory layout constants (matching default PLCASM memory map)
// S=0..63  X=64..127  Y=128..191  M=192..447  T=448+  C=592+
const X = 64   // Input byte address (X0)
const Y = 128  // Output byte address (Y0)
const T = 448  // Timer T0 struct address

export async function runTests(options = {}) {
    const { silent = false, runtime: sharedRuntime = null } = options

    // Use shared runtime if provided, otherwise create one
    let plc = sharedRuntime
    if (!plc) {
        plc = new VovkPLC()
        plc.stdout_callback = () => {}
        await plc.initialize(wasmPath, false, true)
    }

    const readByte = addr => plc.readMemoryArea(addr, 1)[0]
    const readU32 = addr => {
        const bytes = plc.readMemoryArea(addr, 4)
        return (bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24)) >>> 0
    }

    /**
     * Compile PLCASM source, load the program, but do NOT run it.
     * Returns null on success or an error string.
     */
    function compileAndLoad(plcasm) {
        plc.downloadAssembly(plcasm)
        const compileErr = plc.wasm_exports.compileAssembly(false)
        if (compileErr) return `compileAssembly failed (code ${compileErr})`
        const loadErr = plc.wasm_exports.loadCompiledProgram()
        if (loadErr) return `loadCompiledProgram failed (code ${loadErr})`
        return null
    }

    let passed = 0, failed = 0
    /** @type {TestResult[]} */
    const testResults = []
    const failures = []

    function test(name, fn) {
        // Reset memory before each test
        if (plc.wasm_exports?.memoryReset) plc.wasm_exports.memoryReset()
        // Clear millis/micros override
        plc.setMillis(null)
        plc.setMicros(null)

        let testPassed = false
        let errorMsg = null
        try {
            const err = fn()
            if (err) {
                errorMsg = err
            } else {
                testPassed = true
            }
        } catch (e) {
            errorMsg = e.message
        } finally {
            // Always revert overrides after each test
            plc.setMillis(null)
            plc.setMicros(null)
        }

        if (testPassed) {
            passed++
            testResults.push({ name, passed: true })
        } else {
            failed++
            if (!silent) console.log(`  FAIL: ${name} - ${errorMsg}`)
            testResults.push({ name, passed: false, error: errorMsg })
            failures.push({ name, error: errorMsg })
        }
    }

    // ========================================================================
    // PLCASM sources (compiled at runtime — no hardcoded bytecode)
    // ========================================================================

    // TON: Read input bit X0.0, run TON with timer T0, preset 1000ms, write result to Y0.0
    const TON_ASM = `
        u8.readBit X0.0
        ton T0 T#1s
        u8.writeBit Y0.0
    `.trim()

    // TOF: Same layout but TOF instruction
    const TOF_ASM = `
        u8.readBit X0.0
        tof T0 T#1s
        u8.writeBit Y0.0
    `.trim()

    // TP: Pulse timer
    const TP_ASM = `
        u8.readBit X0.0
        tp T0 T#1s
        u8.writeBit Y0.0
    `.trim()

    // CTU: Count-up counter — needs TWO stack inputs: CU (below) and R (top)
    // Push CU from X0.0, then push R=0 (never reset), then CTU
    const CTU_ASM = `
        u8.readBit X0.0
        u8.const 0
        ctu C0 #5
        u8.writeBit Y0.0
    `.trim()

    // ========================================================================
    // TON Timer Tests (On-Delay)
    // ========================================================================

    test('TON ─ basic 1s on-delay timer', () => {
        const err = compileAndLoad(TON_ASM)
        if (err) return err

        // Input ON at t=0
        plc.writeMemoryByte(X, 1)
        plc.setMillis(0)
        plc.run()
        if ((readByte(Y) & 1) !== 0) return 't=0: Q should be 0'

        // t=500 — halfway, still OFF
        plc.setMillis(500)
        plc.run()
        if ((readByte(Y) & 1) !== 0) return 't=500: Q should be 0'

        // t=1000 — preset reached, ON
        plc.setMillis(1000)
        plc.run()
        if ((readByte(Y) & 1) !== 1) return 't=1000: Q should be 1'

        // t=2000 — still ON while input held
        plc.setMillis(2000)
        plc.run()
        if ((readByte(Y) & 1) !== 1) return 't=2000: Q should be 1'

        // Input OFF — Q goes OFF, timer resets
        plc.writeMemoryByte(X, 0)
        plc.setMillis(2500)
        plc.run()
        if ((readByte(Y) & 1) !== 0) return 't=2500 (input OFF): Q should be 0'

        return null
    })

    test('TON ─ timer resets on input toggle', () => {
        const err = compileAndLoad(TON_ASM)
        if (err) return err

        // Start timer
        plc.writeMemoryByte(X, 1)
        plc.setMillis(0)
        plc.run()

        // Progress to 800ms (not yet elapsed)
        plc.setMillis(800)
        plc.run()
        if ((readByte(Y) & 1) !== 0) return 't=800: Q should be 0'

        // Toggle input OFF then ON — resets the timer
        plc.writeMemoryByte(X, 0)
        plc.setMillis(900)
        plc.run()

        plc.writeMemoryByte(X, 1)
        plc.setMillis(1000)
        plc.run()
        // Timer restarted at t=1000, so at t=1000 it should NOT be done
        if ((readByte(Y) & 1) !== 0) return 't=1000 (after reset): Q should be 0'

        // Now wait full 1000ms from restart
        plc.setMillis(2000)
        plc.run()
        if ((readByte(Y) & 1) !== 1) return 't=2000 (1000ms after restart): Q should be 1'

        return null
    })

    test('TON ─ ET (elapsed time) tracks correctly', () => {
        const err = compileAndLoad(TON_ASM)
        if (err) return err

        // Timer struct at T: [ET(u32) StartTime(u32) Flags(u8)]
        plc.writeMemoryByte(X, 1)
        plc.setMillis(100)
        plc.run()

        plc.setMillis(600)
        plc.run()
        const et = readU32(T)
        if (et < 400 || et > 600) return `ET should be ~500, got ${et}`

        return null
    })

    // ========================================================================
    // TOF Timer Tests (Off-Delay)
    // ========================================================================

    test('TOF ─ basic 1s off-delay timer', () => {
        const err = compileAndLoad(TOF_ASM)
        if (err) return err

        // TOF: Q follows input while ON. When input goes OFF, Q stays ON for PT ms.
        // Input ON — Q immediately ON
        plc.writeMemoryByte(X, 1)
        plc.setMillis(0)
        plc.run()
        if ((readByte(Y) & 1) !== 1) return 't=0 (input ON): Q should be 1'

        // Keep input ON for a bit, then turn OFF
        plc.setMillis(100)
        plc.run()

        plc.writeMemoryByte(X, 0)
        plc.setMillis(200)
        plc.run()
        // Q should still be ON (off-delay running)
        if ((readByte(Y) & 1) !== 1) return 't=200 (just turned off): Q should still be 1'

        plc.setMillis(700)
        plc.run()
        if ((readByte(Y) & 1) !== 1) return 't=700 (500ms after off): Q should still be 1'

        // t=1200 — 1000ms after input went OFF at t=200
        plc.setMillis(1200)
        plc.run()
        if ((readByte(Y) & 1) !== 0) return 't=1200 (1000ms after off): Q should be 0'

        return null
    })

    // ========================================================================
    // TP Timer Tests (Pulse)
    // ========================================================================

    test('TP ─ basic 1s pulse timer', () => {
        const err = compileAndLoad(TP_ASM)
        if (err) return err

        // TP: On rising edge, Q goes ON for exactly PT ms regardless of input
        plc.writeMemoryByte(X, 1)
        plc.setMillis(0)
        plc.run()
        if ((readByte(Y) & 1) !== 1) return 't=0 (rising edge): Q should be 1'

        // Mid-pulse
        plc.setMillis(500)
        plc.run()
        if ((readByte(Y) & 1) !== 1) return 't=500: Q should still be 1'

        // After pulse — need to go past 1000ms
        plc.setMillis(1001)
        plc.run()
        if ((readByte(Y) & 1) !== 0) return 't=1001: Q should be 0 (pulse done)'

        return null
    })

    // ========================================================================
    // CTU Counter Tests (Count Up)
    // ========================================================================

    test('CTU ─ count up to 5', () => {
        const err = compileAndLoad(CTU_ASM)
        if (err) return err

        plc.setMillis(0)
        // Pulse the input 5 times (CU rising edges)
        for (let i = 0; i < 5; i++) {
            plc.writeMemoryByte(X, 1)
            plc.setMillis(i * 100)
            plc.run()
            plc.writeMemoryByte(X, 0)
            plc.setMillis(i * 100 + 50)
            plc.run()
        }

        // After 5 pulses, Q should be ON (CV >= PV)
        if ((readByte(Y) & 1) !== 1) return 'After 5 pulses: Q should be 1'

        return null
    })

    test('CTU ─ not reached before PV', () => {
        const err = compileAndLoad(CTU_ASM)
        if (err) return err

        plc.setMillis(0)
        // Only 3 pulses (PV=5)
        for (let i = 0; i < 3; i++) {
            plc.writeMemoryByte(X, 1)
            plc.setMillis(i * 100)
            plc.run()
            plc.writeMemoryByte(X, 0)
            plc.setMillis(i * 100 + 50)
            plc.run()
        }

        if ((readByte(Y) & 1) !== 0) return 'After 3 pulses (PV=5): Q should be 0'

        return null
    })

    // ========================================================================
    // setMillis override revert test
    // ========================================================================

    test('setMillis ─ override reverts to real time', () => {
        plc.setMillis(999999)
        if (plc.getMillis() !== 999999) return `getMillis should return 999999, got ${plc.getMillis()}`

        plc.setMillis(null)
        const t = plc.getMillis()
        // After revert, getMillis should return a reasonable performance.now() value (not 999999)
        if (t === 999999) return 'getMillis still returning override after null revert'

        return null
    })

    test('setMicros ─ override reverts to real time', () => {
        plc.setMicros(123456789)
        if (plc.getMicros() !== 123456789) return `getMicros should return 123456789, got ${plc.getMicros()}`

        plc.setMicros(null)
        const t = plc.getMicros()
        if (t === 123456789) return 'getMicros still returning override after null revert'

        return null
    })

    return {
        name: 'Timer & Counter Functional Tests',
        passed,
        failed,
        total: passed + failed,
        tests: testResults,
        failures
    }
}

// Run directly if executed as main
if (process.argv[1]?.includes('test_timers')) {
    runTests({ silent: false }).then(result => {
        console.log(`\n${result.name}: ${result.passed}/${result.total} passed`)
        if (result.failures.length > 0) {
            console.log('\nFailures:')
            for (const f of result.failures) {
                console.log(`  ✗ ${f.name}: ${f.error}`)
            }
        }
        process.exit(result.failed > 0 ? 1 : 0)
    })
}
