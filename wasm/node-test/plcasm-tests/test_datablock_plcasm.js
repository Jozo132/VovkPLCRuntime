// test_datablock_plcasm.js - DataBlock WASM API and PLCASM integration tests
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

    // ═══════════════════════════════════════════════════════════════════════
    // Direct WASM API tests
    // ═══════════════════════════════════════════════════════════════════════

    test('DB table has 16 slots by default', () => {
        const slots = plc.wasm_exports.db_getSlotCount()
        if (slots !== 16) return `expected 16 slots, got ${slots}`
    })

    test('DB active count is 0 after reset', () => {
        const count = plc.wasm_exports.db_getActiveCount()
        if (count !== 0) return `expected 0 active, got ${count}`
    })

    test('db_declare allocates a datablock', () => {
        const slot = plc.wasm_exports.db_declare(1, 64)
        if (slot < 0) return `db_declare returned ${slot}`
        const active = plc.wasm_exports.db_getActiveCount()
        if (active !== 1) return `expected 1 active, got ${active}`
        const db = plc.wasm_exports.db_getEntryDB(slot)
        if (db !== 1) return `slot db should be 1, got ${db}`
        const size = plc.wasm_exports.db_getEntrySize(slot)
        if (size !== 64) return `slot size should be 64, got ${size}`
    })

    test('db_declare multiple datablocks', () => {
        plc.wasm_exports.db_declare(1, 32)
        plc.wasm_exports.db_declare(2, 64)
        plc.wasm_exports.db_declare(3, 16)
        const active = plc.wasm_exports.db_getActiveCount()
        if (active !== 3) return `expected 3 active, got ${active}`
    })

    test('db_declare rejects duplicate DB number', () => {
        plc.wasm_exports.db_declare(5, 32)
        const slot2 = plc.wasm_exports.db_declare(5, 64)
        // Should return -1 for duplicate
        // Treat as signed: slot2 is u16 from WASM, but db_declare returns i16
        // In WASM, i16 -1 comes through as 65535 (unsigned interpretation)
        if (slot2 !== 65535 && slot2 !== -1) return `expected -1 for duplicate, got ${slot2}`
    })

    test('db_resolveAddress returns correct absolute address', () => {
        const slot = plc.wasm_exports.db_declare(10, 100)
        if (slot < 0) return `declare failed: ${slot}`
        const offset = plc.wasm_exports.db_getEntryOffset(slot)
        const addr = plc.wasm_exports.db_resolveAddress(10, 0)
        if (addr !== offset) return `expected ${offset}, got ${addr}`
        const addr4 = plc.wasm_exports.db_resolveAddress(10, 4)
        if (addr4 !== offset + 4) return `expected ${offset + 4}, got ${addr4}`
    })

    test('db_resolveAddress returns 0xFFFF for invalid DB', () => {
        const addr = plc.wasm_exports.db_resolveAddress(99, 0)
        if (addr !== 0xFFFF) return `expected 0xFFFF, got ${addr}`
    })

    test('db_resolveAddress returns 0xFFFF for out-of-range offset', () => {
        plc.wasm_exports.db_declare(1, 10)
        const addr = plc.wasm_exports.db_resolveAddress(1, 10) // offset 10 in a 10-byte block is out of range
        if (addr !== 0xFFFF) return `expected 0xFFFF, got ${addr}`
    })

    test('db_remove deletes a datablock', () => {
        plc.wasm_exports.db_declare(1, 32)
        plc.wasm_exports.db_declare(2, 64)
        const ok = plc.wasm_exports.db_remove(1)
        if (ok !== 1) return `db_remove returned ${ok}`
        const active = plc.wasm_exports.db_getActiveCount()
        if (active !== 1) return `expected 1 active after remove, got ${active}`
        // DB 2 should still resolve
        const addr = plc.wasm_exports.db_resolveAddress(2, 0)
        if (addr === 0xFFFF) return `DB 2 should still exist`
    })

    test('db_remove returns 0 for non-existent DB', () => {
        const ok = plc.wasm_exports.db_remove(99)
        if (ok !== 0) return `expected 0, got ${ok}`
    })

    test('db_format clears all entries', () => {
        plc.wasm_exports.db_declare(1, 16)
        plc.wasm_exports.db_declare(2, 16)
        plc.wasm_exports.db_format()
        const active = plc.wasm_exports.db_getActiveCount()
        if (active !== 0) return `expected 0 active after format, got ${active}`
    })

    test('db_compact packs datablocks tightly', () => {
        plc.wasm_exports.db_declare(1, 32)
        plc.wasm_exports.db_declare(2, 64)
        plc.wasm_exports.db_declare(3, 16)
        // Remove middle one to create a gap
        plc.wasm_exports.db_remove(2)
        // Compact should pack remaining DBs
        const lowest = plc.wasm_exports.db_compact()
        const tableOffset = plc.wasm_exports.db_getTableOffset()
        // After compact, total data is 32 + 16 = 48 bytes, should pack right against table
        const expectedLowest = tableOffset - 48
        if (lowest !== expectedLowest) return `expected lowest=${expectedLowest}, got ${lowest}`
    })

    test('db_migrate moves datablock data', () => {
        plc.wasm_exports.db_declare(1, 16)
        // Write some data to DB1 via memory
        const origOffset = plc.wasm_exports.db_getEntryOffset(0)
        const memBase = plc.wasm_exports.getMemoryLocation()
        const mem = new Uint8Array(plc.wasm_exports.memory.buffer, memBase + origOffset, 16)
        for (let i = 0; i < 16; i++) mem[i] = i + 0xA0

        // Migrate to a different address
        const targetAddr = origOffset - 100
        const ok = plc.wasm_exports.db_migrate(1, targetAddr)
        if (ok !== 1) return `db_migrate returned ${ok}`

        // Verify new offset
        const newOffset = plc.wasm_exports.db_getEntryOffset(0)
        if (newOffset !== targetAddr) return `expected offset=${targetAddr}, got ${newOffset}`

        // Verify data was copied
        const newMem = new Uint8Array(plc.wasm_exports.memory.buffer, memBase + targetAddr, 16)
        for (let i = 0; i < 16; i++) {
            if (newMem[i] !== (i + 0xA0)) return `byte ${i} mismatch: expected ${i + 0xA0}, got ${newMem[i]}`
        }
    })

    test('db_getFreeSpace decreases after allocation', () => {
        const free0 = plc.wasm_exports.db_getFreeSpace()
        plc.wasm_exports.db_declare(1, 100)
        const free1 = plc.wasm_exports.db_getFreeSpace()
        if (free1 >= free0) return `free space should decrease: was ${free0}, now ${free1}`
        if (free0 - free1 !== 100) return `expected 100 byte decrease, got ${free0 - free1}`
    })

    // ═══════════════════════════════════════════════════════════════════════
    // PLCASM .runtime_config_db directive tests
    // ═══════════════════════════════════════════════════════════════════════

    test('.runtime_config_db declares one DB via bytecode', () => {
        const err = compileAndRun('.runtime_config_db 1 5 32\nexit')
        if (err) return err
        const active = plc.wasm_exports.db_getActiveCount()
        if (active !== 1) return `expected 1 active DB, got ${active}`
        const addr = plc.wasm_exports.db_resolveAddress(5, 0)
        if (addr === 0xFFFF) return 'resolveAddress failed for DB5'
    })

    test('.runtime_config_db declares multiple DBs via bytecode', () => {
        const err = compileAndRun('.runtime_config_db 3 1 16 2 32 3 64\nexit')
        if (err) return err
        const active = plc.wasm_exports.db_getActiveCount()
        if (active !== 3) return `expected 3 active, got ${active}`
        // Verify each DB is resolvable
        for (const [db, size] of [[1, 16], [2, 32], [3, 64]]) {
            const addr = plc.wasm_exports.db_resolveAddress(db, 0)
            if (addr === 0xFFFF) return `resolveAddress failed for DB${db}`
            const addrEnd = plc.wasm_exports.db_resolveAddress(db, size - 1)
            if (addrEnd === 0xFFFF) return `resolveAddress failed for DB${db} last byte`
        }
    })

    test('.runtime_config_db is idempotent on repeated runs', () => {
        const source = '.runtime_config_db 1 7 48\nexit'
        const err1 = compileAndRun(source)
        if (err1) return err1
        // Run again (simulating second scan cycle) — should not declare again
        plc.run()
        const active = plc.wasm_exports.db_getActiveCount()
        if (active !== 1) return `expected 1 active after re-run, got ${active}`
    })

    test('DB memory is read/writable after .runtime_config_db', () => {
        const err = compileAndRun('.runtime_config_db 1 1 8\nexit')
        if (err) return err
        const addr = plc.wasm_exports.db_resolveAddress(1, 0)
        if (addr === 0xFFFF) return 'resolveAddress failed'
        // Write via direct memory access
        const memBase = plc.wasm_exports.getMemoryLocation()
        const mem = new Uint8Array(plc.wasm_exports.memory.buffer, memBase + addr, 8)
        mem[0] = 0xDE
        mem[1] = 0xAD
        mem[7] = 0xFF
        // Read back
        if (mem[0] !== 0xDE) return `byte 0 mismatch`
        if (mem[1] !== 0xAD) return `byte 1 mismatch`
        if (mem[7] !== 0xFF) return `byte 7 mismatch`
    })

    test('PLCASM can write to DB-resolved addresses', () => {
        // Declare DB1 with 8 bytes, then write a u32 constant to the DB area
        // We do this by first declaring through the directive, exiting,
        // then compile a second program that writes to the resolved address
        const declErr = compileAndRun('.runtime_config_db 1 1 8\nexit')
        if (declErr) return declErr
        const addr = plc.wasm_exports.db_resolveAddress(1, 0)
        if (addr === 0xFFFF) return 'resolveAddress failed'
        // Now compile a program that writes 0x12345678 to the absolute address
        const writeAsm = `u32.const 0x12345678\nu32.move_to ${addr}\nexit`
        const err = compileAndRun(writeAsm)
        if (err) return err
        // Verify via memory read
        const memBase = plc.wasm_exports.getMemoryLocation()
        const mem = new Uint8Array(plc.wasm_exports.memory.buffer, memBase + addr, 4)
        // Little-endian u32: 0x12345678 → [0x78, 0x56, 0x34, 0x12]
        if (mem[0] !== 0x78) return `byte 0: expected 0x78, got 0x${mem[0].toString(16)}`
        if (mem[1] !== 0x56) return `byte 1: expected 0x56, got 0x${mem[1].toString(16)}`
        if (mem[2] !== 0x34) return `byte 2: expected 0x34, got 0x${mem[2].toString(16)}`
        if (mem[3] !== 0x12) return `byte 3: expected 0x12, got 0x${mem[3].toString(16)}`
    })

    return {
        name: 'DataBlock PLCASM Tests',
        passed,
        failed,
        total: passed + failed,
        tests: testResults,
        failures
    }
}

// Standalone execution
if (process.argv[1] && (process.argv[1].includes('test_datablock') || process.argv[1].endsWith('test_datablock_plcasm.js'))) {
    runTests({ silent: false, verbose: true }).then(r => {
        console.log(`\n${r.name}: ${r.passed}/${r.total} passed`)
        process.exit(r.failed > 0 ? 1 : 0)
    })
}
