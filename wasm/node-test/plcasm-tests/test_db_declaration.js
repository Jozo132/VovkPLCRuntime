// test_db_declaration.js - Tests for .db<N> DataBlock declaration syntax in PLCASM
import VovkPLC from '../../dist/VovkPLC.js'
import path from 'path'
import { fileURLToPath } from 'url'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')

export async function runTests(options = {}) {
    const { silent = false, verbose = false, runtime: sharedRuntime = null } = options

    let plc = sharedRuntime
    if (!plc) {
        plc = new VovkPLC()
        plc.stdout_callback = () => {}
        await plc.initialize(wasmPath, false, true)
    }

    let passed = 0
    let failed = 0
    const testResults = []
    const failures = []

    function reset() {
        plc.wasm_exports.memoryReset()
    }

    function test(name, fn) {
        reset()
        let testPassed = false
        let errorMsg = null
        try {
            const err = fn()
            if (err) errorMsg = err
            else testPassed = true
        } catch (e) {
            errorMsg = e.message || String(e)
        }
        if (testPassed) {
            passed++
            testResults.push({ name, passed: true })
        } else {
            failed++
            if (!silent) console.log(`  FAIL: ${name} — ${errorMsg}`)
            testResults.push({ name, passed: false, error: errorMsg })
            failures.push({ name, error: errorMsg })
        }
    }

    // Helper: compile PLCASM (no run)
    function compile(source) {
        plc.downloadAssembly(source)
        const ce = plc.wasm_exports.compileAssembly(verbose ? 1 : 0)
        if (ce) return 'compileAssembly failed'
        return null
    }

    // Helper: compile + load + run PLCASM
    function compileAndRun(source) {
        plc.downloadAssembly(source)
        const ce = plc.wasm_exports.compileAssembly(verbose ? 1 : 0)
        if (ce) return 'compileAssembly failed'
        const le = plc.wasm_exports.loadCompiledProgram()
        if (le) return 'loadCompiledProgram failed'
        plc.run()
        return null
    }

    // Helper: read memory as typed values
    function readMemI16(addr) {
        const memBase = plc.wasm_exports.getMemoryLocation()
        const view = new DataView(plc.wasm_exports.memory.buffer, memBase + addr, 2)
        return view.getInt16(0, true) // little-endian
    }
    function readMemU8(addr) {
        const memBase = plc.wasm_exports.getMemoryLocation()
        const mem = new Uint8Array(plc.wasm_exports.memory.buffer, memBase + addr, 1)
        return mem[0]
    }
    function readMemF32(addr) {
        const memBase = plc.wasm_exports.getMemoryLocation()
        const view = new DataView(plc.wasm_exports.memory.buffer, memBase + addr, 4)
        return view.getFloat32(0, true) // little-endian
    }
    function readMemU32(addr) {
        const memBase = plc.wasm_exports.getMemoryLocation()
        const view = new DataView(plc.wasm_exports.memory.buffer, memBase + addr, 4)
        return view.getUint32(0, true)
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Basic .db<N> declaration parsing
    // ═══════════════════════════════════════════════════════════════════════

    test('.db1 declaration compiles without error', () => {
        const err = compile(`.db1 "Motor" = {
            speed: i16 = 100
        }
        exit`)
        if (err) return err
    })

    test('.db1 declaration with multiple fields compiles', () => {
        const err = compile(`.db1 "Motor" = {
            speed: i16 = 100
            position: f32 = 3.14
            enabled: u8 = 1
        }
        exit`)
        if (err) return err
    })

    test('.db declaration without alias compiles', () => {
        const err = compile(`.db1 = {
            value: i32 = 42
        }
        exit`)
        if (err) return err
    })

    test('.db declaration with no defaults compiles', () => {
        const err = compile(`.db1 "Registers" = {
            reg0: u16
            reg1: u16
            reg2: u16
        }
        exit`)
        if (err) return err
    })

    test('Multiple .db declarations compile', () => {
        const err = compile(`.db1 "Motor1" = {
            speed: i16 = 10
        }
        .db2 "Motor2" = {
            speed: i16 = 20
        }
        exit`)
        if (err) return err
    })

    // ═══════════════════════════════════════════════════════════════════════
    // CONFIG_DB emission and default value initialization
    // ═══════════════════════════════════════════════════════════════════════

    test('.db1 emits CONFIG_DB and declares datablock at runtime', () => {
        const err = compileAndRun(`.db1 "Motor" = {
            speed: i16 = 100
        }
        exit`)
        if (err) return err
        const active = plc.wasm_exports.db_getActiveCount()
        if (active !== 1) return `expected 1 active DB, got ${active}`
    })

    test('.db1 default i16 value is written to memory', () => {
        const err = compileAndRun(`.db1 "Motor" = {
            speed: i16 = 100
        }
        exit`)
        if (err) return err
        const addr = plc.wasm_exports.db_resolveAddress(1, 0)
        if (addr === 0xFFFF) return 'resolveAddress failed'
        const val = readMemI16(addr)
        if (val !== 100) return `expected speed=100, got ${val}`
    })

    test('.db1 default f32 value is written to memory', () => {
        const err = compileAndRun(`.db1 "Sensor" = {
            temperature: f32 = 25.5
        }
        exit`)
        if (err) return err
        const addr = plc.wasm_exports.db_resolveAddress(1, 0)
        if (addr === 0xFFFF) return 'resolveAddress failed'
        const val = readMemF32(addr)
        if (Math.abs(val - 25.5) > 0.001) return `expected temperature=25.5, got ${val}`
    })

    test('.db1 default u8 value is written to memory', () => {
        const err = compileAndRun(`.db1 "Flags" = {
            enabled: u8 = 1
        }
        exit`)
        if (err) return err
        const addr = plc.wasm_exports.db_resolveAddress(1, 0)
        if (addr === 0xFFFF) return 'resolveAddress failed'
        const val = readMemU8(addr)
        if (val !== 1) return `expected enabled=1, got ${val}`
    })

    test('.db1 multiple fields with defaults are all initialized', () => {
        const err = compileAndRun(`.db1 "Motor" = {
            speed: i16 = 200
            temperature: f32 = 72.5
            enabled: u8 = 1
        }
        exit`)
        if (err) return err
        const addr_speed = plc.wasm_exports.db_resolveAddress(1, 0)
        const addr_temp = plc.wasm_exports.db_resolveAddress(1, 2)
        const addr_en = plc.wasm_exports.db_resolveAddress(1, 6)
        if (addr_speed === 0xFFFF) return 'resolveAddress failed for speed'
        const speed = readMemI16(addr_speed)
        if (speed !== 200) return `expected speed=200, got ${speed}`
        const temp = readMemF32(addr_temp)
        if (Math.abs(temp - 72.5) > 0.001) return `expected temp=72.5, got ${temp}`
        const en = readMemU8(addr_en)
        if (en !== 1) return `expected enabled=1, got ${en}`
    })

    test('.db with negative default value', () => {
        const err = compileAndRun(`.db1 "PID" = {
            setpoint: i16 = -50
        }
        exit`)
        if (err) return err
        const addr = plc.wasm_exports.db_resolveAddress(1, 0)
        if (addr === 0xFFFF) return 'resolveAddress failed'
        const val = readMemI16(addr)
        if (val !== -50) return `expected setpoint=-50, got ${val}`
    })

    test('Multiple .db declarations initialize all datablocks', () => {
        const err = compileAndRun(`.db1 "Motor1" = {
            speed: i16 = 10
        }
        .db2 "Motor2" = {
            speed: i16 = 20
        }
        exit`)
        if (err) return err
        const active = plc.wasm_exports.db_getActiveCount()
        if (active !== 2) return `expected 2 active, got ${active}`
        const addr1 = plc.wasm_exports.db_resolveAddress(1, 0)
        const addr2 = plc.wasm_exports.db_resolveAddress(2, 0)
        if (addr1 === 0xFFFF) return 'resolveAddress failed for DB1'
        if (addr2 === 0xFFFF) return 'resolveAddress failed for DB2'
        const speed1 = readMemI16(addr1)
        const speed2 = readMemI16(addr2)
        if (speed1 !== 10) return `expected DB1.speed=10, got ${speed1}`
        if (speed2 !== 20) return `expected DB2.speed=20, got ${speed2}`
    })

    // ═══════════════════════════════════════════════════════════════════════
    // Field access via DB number (DB1.field) and alias (Motor.field)
    // ═══════════════════════════════════════════════════════════════════════

    test('Access field via DB1.fieldName pushes resolved address', () => {
        const err = compileAndRun(`.db1 "Motor" = {
            speed: i16 = 100
        }
        i16.const 555
        i16.move_to DB1.speed
        exit`)
        if (err) return err
        const addr = plc.wasm_exports.db_resolveAddress(1, 0)
        if (addr === 0xFFFF) return 'resolveAddress failed'
        const val = readMemI16(addr)
        if (val !== 555) return `expected 555 at DB1.speed, got ${val}`
    })

    test('Access field via alias.fieldName pushes resolved address', () => {
        const err = compileAndRun(`.db1 "Motor" = {
            speed: i16 = 100
        }
        i16.const 777
        i16.move_to Motor.speed
        exit`)
        if (err) return err
        const addr = plc.wasm_exports.db_resolveAddress(1, 0)
        if (addr === 0xFFFF) return 'resolveAddress failed'
        const val = readMemI16(addr)
        if (val !== 777) return `expected 777 at Motor.speed, got ${val}`
    })

    test('DB1.field and alias.field resolve to the same address', () => {
        const err = compileAndRun(`.db1 "Sensor" = {
            value: f32 = 0
        }
        f32.const 99.5
        f32.move_to DB1.value
        f32.load_from Sensor.value
        exit`)
        if (err) return err
        // The push should have read the value written by move_to
        const addr = plc.wasm_exports.db_resolveAddress(1, 0)
        if (addr === 0xFFFF) return 'resolveAddress failed'
        const val = readMemF32(addr)
        if (Math.abs(val - 99.5) > 0.001) return `expected 99.5, got ${val}`
    })

    test('Access second field in multi-field DB by name', () => {
        const err = compileAndRun(`.db1 "Motor" = {
            speed: i16 = 0
            position: f32 = 0
        }
        f32.const 42.0
        f32.move_to DB1.position
        exit`)
        if (err) return err
        // position is at offset 2 (after i16 speed)
        const addr = plc.wasm_exports.db_resolveAddress(1, 2)
        if (addr === 0xFFFF) return 'resolveAddress failed'
        const val = readMemF32(addr)
        if (Math.abs(val - 42.0) > 0.001) return `expected 42.0, got ${val}`
    })

    test('Read field with type.push DB1.field', () => {
        const err = compileAndRun(`.db1 "Config" = {
            threshold: i16 = 500
        }
        // Read the default value that was written during init
        i16.load_from DB1.threshold
        // Write it doubled to marker area
        i16.const 2
        i16.mul
        i16.move_to 192
        exit`)
        if (err) return err
        const val = readMemI16(192)
        if (val !== 1000) return `expected 1000 (500*2), got ${val}`
    })

    test('Read and write across two DBs using aliases', () => {
        const err = compileAndRun(`.db1 "Input" = {
            raw: i16 = 123
        }
        .db2 "Output" = {
            result: i16 = 0
        }
        // Read from Input.raw and write to Output.result
        i16.load_from Input.raw
        i16.move_to Output.result
        exit`)
        if (err) return err
        const addr = plc.wasm_exports.db_resolveAddress(2, 0)
        if (addr === 0xFFFF) return 'resolveAddress failed for DB2'
        const val = readMemI16(addr)
        if (val !== 123) return `expected Output.result=123, got ${val}`
    })

    // ═══════════════════════════════════════════════════════════════════════
    // db_setSlotCount WASM API
    // ═══════════════════════════════════════════════════════════════════════

    test('db_setSlotCount changes the number of available slots', () => {
        plc.wasm_exports.db_setSlotCount(4)
        const slots = plc.wasm_exports.db_getSlotCount()
        if (slots !== 4) return `expected 4 slots, got ${slots}`
        // Restore default
        plc.wasm_exports.db_setSlotCount(16)
    })

    test('db_setSlotCount allows fewer slots for embedded targets', () => {
        plc.wasm_exports.db_setSlotCount(2)
        plc.wasm_exports.db_declare(1, 32)
        plc.wasm_exports.db_declare(2, 32)
        const slot3 = plc.wasm_exports.db_declare(3, 32)
        // Third declare should fail — only 2 slots
        if (slot3 !== 65535 && slot3 !== -1) return `expected failure for 3rd DB with 2 slots, got ${slot3}`
        // Restore default
        plc.wasm_exports.db_setSlotCount(16)
    })

    // ═══════════════════════════════════════════════════════════════════════
    // Edge cases and error handling
    // ═══════════════════════════════════════════════════════════════════════

    test('.db with u32 default value', () => {
        const err = compileAndRun(`.db1 "Data" = {
            value: u32 = 0x12345678
        }
        exit`)
        if (err) return err
        const addr = plc.wasm_exports.db_resolveAddress(1, 0)
        if (addr === 0xFFFF) return 'resolveAddress failed'
        const val = readMemU32(addr)
        if (val !== 0x12345678) return `expected 0x12345678, got 0x${val.toString(16)}`
    })

    test('.db fields without defaults are zero-initialized', () => {
        const err = compileAndRun(`.db1 "Registers" = {
            reg0: i16
            reg1: i16
        }
        exit`)
        if (err) return err
        const addr0 = plc.wasm_exports.db_resolveAddress(1, 0)
        const addr1 = plc.wasm_exports.db_resolveAddress(1, 2)
        if (addr0 === 0xFFFF) return 'resolveAddress failed for reg0'
        const val0 = readMemI16(addr0)
        const val1 = readMemI16(addr1)
        if (val0 !== 0) return `expected reg0=0, got ${val0}`
        if (val1 !== 0) return `expected reg1=0, got ${val1}`
    })

    test('.db mixed defaults and no-defaults', () => {
        const err = compileAndRun(`.db1 "Mixed" = {
            a: i16 = 42
            b: i16
            c: i16 = 99
        }
        exit`)
        if (err) return err
        const addr_a = plc.wasm_exports.db_resolveAddress(1, 0)
        const addr_b = plc.wasm_exports.db_resolveAddress(1, 2)
        const addr_c = plc.wasm_exports.db_resolveAddress(1, 4)
        if (addr_a === 0xFFFF) return 'resolveAddress failed for a'
        const a = readMemI16(addr_a)
        const b = readMemI16(addr_b)
        const c = readMemI16(addr_c)
        if (a !== 42) return `expected a=42, got ${a}`
        if (b !== 0) return `expected b=0, got ${b}`
        if (c !== 99) return `expected c=99, got ${c}`
    })

    test('Case-insensitive DB access (db1 vs DB1)', () => {
        const err = compileAndRun(`.db1 "Motor" = {
            speed: i16 = 100
        }
        i16.load_from db1.speed
        i16.move_to 192
        exit`)
        if (err) return err
        const val = readMemI16(192)
        if (val !== 100) return `expected 100, got ${val}`
    })

    return {
        name: 'DB Declaration Tests',
        passed,
        failed,
        total: passed + failed,
        tests: testResults,
        failures
    }
}

// Standalone execution
if (process.argv[1] && (process.argv[1].includes('test_db_decl') || process.argv[1].endsWith('test_db_declaration.js'))) {
    runTests({ silent: false, verbose: true }).then(r => {
        console.log(`\n${r.name}: ${r.passed}/${r.total} passed`)
        if (r.failures.length > 0) {
            console.log('Failures:')
            for (const f of r.failures) console.log(`  - ${f.name}: ${f.error}`)
        }
        process.exit(r.failed > 0 ? 1 : 0)
    })
}
