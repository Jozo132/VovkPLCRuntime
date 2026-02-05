// test_string.js - String and Char datatype unit tests
// Copyright (c) 2024 J.Vovk - GPL-3.0-or-later

import VovkPLC from '../dist/VovkPLC.js';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm');

const COLOR = { reset: '\x1b[0m', green: '\x1b[32m', red: '\x1b[31m', cyan: '\x1b[36m', yellow: '\x1b[33m' };

// Memory addresses for string tests (M area at 256+)
const M = 256;

/**
 * Run string tests and return results
 * @param {object} options - Test options
 * @param {boolean} options.verbose - Show detailed output
 * @param {VovkPLC} [options.runtime] - Shared runtime instance
 * @returns {Promise<{name: string, passed: number, failed: number, total: number, tests: Array}>}
 */
export async function runTests(options = {}) {
    const { verbose = false, runtime: sharedRuntime = null } = options;
    
    // Use shared runtime or create own
    let plc = sharedRuntime;
    if (!plc) {
        plc = new VovkPLC();
        plc.stdout_callback = () => {};
        await plc.initialize(wasmPath, false, true);
    }

    // Helper to write bytes to memory
    function writeMemory(addr, data) {
        plc.writeMemoryArea(addr, data);
    }

    // Helper to read bytes from memory as array
    function readMemory(addr, size) {
        return Array.from(plc.readMemoryArea(addr, size));
    }

    // Helper to setup a str8 at address: [u8 capacity, u8 length, char...]
    function setupStr8(addr, capacity, content) {
        const len = content.length;
        const data = [capacity, len];
        for (let i = 0; i < len; i++) {
            data.push(content.charCodeAt(i));
        }
        // Pad to full capacity
        while (data.length < capacity + 2) {
            data.push(0);
        }
        writeMemory(addr, data);
    }

    // Helper to read str8 from address
    function readStr8(addr) {
        const mem = readMemory(addr, 2);
        const capacity = mem[0];
        const len = mem[1];
        let str = '';
        if (len > 0) {
            const chars = readMemory(addr + 2, len);
            for (let i = 0; i < len; i++) {
                str += String.fromCharCode(chars[i]);
            }
        }
        return { capacity, length: len, content: str };
    }

    // Helper to setup a str16 at address: [u16 capacity, u16 length, char...]
    function setupStr16(addr, capacity, content) {
        const len = content.length;
        const data = [
            capacity & 0xFF, (capacity >> 8) & 0xFF,
            len & 0xFF, (len >> 8) & 0xFF
        ];
        for (let i = 0; i < len; i++) {
            data.push(content.charCodeAt(i));
        }
        // Pad a bit
        while (data.length < Math.min(capacity + 4, 300)) {
            data.push(0);
        }
        writeMemory(addr, data);
    }

    // Helper to read str16 from address
    function readStr16(addr) {
        const header = readMemory(addr, 4);
        const capacity = header[0] | (header[1] << 8);
        const len = header[2] | (header[3] << 8);
        let str = '';
        if (len > 0) {
            const chars = readMemory(addr + 4, len);
            for (let i = 0; i < len; i++) {
                str += String.fromCharCode(chars[i]);
            }
        }
        return { capacity, length: len, content: str };
    }

    // Helper to compile and run PLCASM
    function compileAndRun(source) {
        plc.downloadAssembly(source);
        const compileError = plc.wasm_exports.compileAssembly();
        if (compileError) throw new Error(`Compile error: ${compileError}`);

        const loadError = plc.wasm_exports.loadCompiledProgram();
        if (loadError) throw new Error(`Load error: ${loadError}`);

        plc.wasm_exports.clearStack();
        const runError = plc.wasm_exports.run();
        return runError;
    }

    // Helper to read a u16 value from memory
    function readU16(addr) {
        const mem = readMemory(addr, 2);
        return mem[0] | (mem[1] << 8);
    }

    // Helper to read an i8 from memory
    function readI8(addr) {
        const mem = readMemory(addr, 1);
        return mem[0] > 127 ? mem[0] - 256 : mem[0];
    }

    // Helper to read a u8 value from memory
    function readU8(addr) {
        return readMemory(addr, 1)[0];
    }

    // Expect helper
    function expect(actual) {
        return {
            toBe(expected) {
                if (actual !== expected) {
                    throw new Error(`Expected ${expected}, got ${actual}`);
                }
            }
        };
    }

    // Test results collection
    const testResults = [];
    let passed = 0;
    let failed = 0;

    function test(name, fn) {
        try {
            fn();
            passed++;
            testResults.push({ name, passed: true });
        } catch (e) {
            failed++;
            testResults.push({ name, passed: false, error: e.message });
        }
    }

    const RESULT = M + 800;  // Result storage area

    // STR_LEN tests
    test('str.len returns correct length', () => {
        setupStr8(M, 20, 'Hello');
        compileAndRun(`str.len ${M}  u16.move_to ${RESULT}`);
        expect(readU16(RESULT)).toBe(5);
    });

    test('str.len returns 0 for empty string', () => {
        setupStr8(M, 20, '');
        compileAndRun(`str.len ${M}  u16.move_to ${RESULT}`);
        expect(readU16(RESULT)).toBe(0);
    });

    test('str16.len returns correct length', () => {
        setupStr16(M, 500, 'Hello World');
        compileAndRun(`str16.len ${M}  u16.move_to ${RESULT}`);
        expect(readU16(RESULT)).toBe(11);
    });

    // STR_CAP tests
    test('str.cap returns correct capacity', () => {
        setupStr8(M, 50, 'Hello');
        compileAndRun(`str.cap ${M}  u16.move_to ${RESULT}`);
        expect(readU16(RESULT)).toBe(50);
    });

    test('str16.cap returns correct capacity', () => {
        setupStr16(M, 1000, 'Test');
        compileAndRun(`str16.cap ${M}  u16.move_to ${RESULT}`);
        expect(readU16(RESULT)).toBe(1000);
    });

    // STR_GET tests
    test('str.get returns correct character', () => {
        setupStr8(M, 20, 'Hello');
        // Push index 0 to get 'H'
        compileAndRun(`u16 0  str.get ${M}  u8.move_to ${RESULT}`);
        expect(readU8(RESULT)).toBe('H'.charCodeAt(0)); // 72
    });

    test('str.get returns 0 for out of bounds', () => {
        setupStr8(M, 20, 'Hi');
        // Push index 5 (out of bounds for "Hi")
        compileAndRun(`u16 5  str.get ${M}  u8.move_to ${RESULT}`);
        expect(readU8(RESULT)).toBe(0);
    });

    // STR_SET tests
    test('str.set sets character at index', () => {
        setupStr8(M, 20, 'Hello');
        // Push index 0, then char 'J' to replace 'H'
        compileAndRun(`u16 0  u8 74  str.set ${M}`);
        const str = readStr8(M);
        expect(str.content).toBe('Jello');
    });

    // STR_CLEAR tests
    test('str.clear sets length to 0', () => {
        setupStr8(M, 20, 'Hello');
        compileAndRun(`str.clear ${M}`);
        const str = readStr8(M);
        expect(str.length).toBe(0);
        expect(str.capacity).toBe(20);
    });

    // STR_CMP tests
    test('str.cmp returns 0 for equal strings', () => {
        setupStr8(M, 20, 'Hello');
        setupStr8(M + 100, 20, 'Hello');
        compileAndRun(`str.cmp ${M} ${M + 100}  i8.move_to ${RESULT}`);
        expect(readI8(RESULT)).toBe(0);
    });

    test('str.cmp returns -1 for first < second', () => {
        setupStr8(M, 20, 'Apple');
        setupStr8(M + 100, 20, 'Banana');
        compileAndRun(`str.cmp ${M} ${M + 100}  i8.move_to ${RESULT}`);
        expect(readI8(RESULT)).toBe(-1);
    });

    test('str.cmp returns 1 for first > second', () => {
        setupStr8(M, 20, 'Banana');
        setupStr8(M + 100, 20, 'Apple');
        compileAndRun(`str.cmp ${M} ${M + 100}  i8.move_to ${RESULT}`);
        expect(readI8(RESULT)).toBe(1);
    });

    // STR_EQ tests
    test('str.eq returns 1 for equal strings', () => {
        setupStr8(M, 20, 'Hello');
        setupStr8(M + 100, 20, 'Hello');
        compileAndRun(`str.eq ${M} ${M + 100}  u8.move_to ${RESULT}`);
        expect(readU8(RESULT)).toBe(1);
    });

    test('str.eq returns 0 for different strings', () => {
        setupStr8(M, 20, 'Hello');
        setupStr8(M + 100, 20, 'World');
        compileAndRun(`str.eq ${M} ${M + 100}  u8.move_to ${RESULT}`);
        expect(readU8(RESULT)).toBe(0);
    });

    // STR_CONCAT tests
    test('str.concat appends source to destination', () => {
        setupStr8(M, 30, 'Hello');
        setupStr8(M + 100, 20, ' World');
        compileAndRun(`str.concat ${M} ${M + 100}`);
        const str = readStr8(M);
        expect(str.content).toBe('Hello World');
        expect(str.length).toBe(11);
    });

    test('str.concat truncates when exceeding capacity', () => {
        setupStr8(M, 8, 'Hello'); // Capacity 8, length 5
        setupStr8(M + 100, 20, '!!!!!!'); // 6 chars
        compileAndRun(`str.concat ${M} ${M + 100}`);
        const str = readStr8(M);
        expect(str.length).toBe(8);
        expect(str.content).toBe('Hello!!!');
    });

    // STR_COPY tests
    test('str.copy copies source to destination', () => {
        setupStr8(M, 30, '');
        setupStr8(M + 100, 20, 'Test Copy');
        compileAndRun(`str.copy ${M} ${M + 100}`);
        const str = readStr8(M);
        expect(str.content).toBe('Test Copy');
        expect(str.length).toBe(9);
    });

    // STR_SUBSTR tests
    test('str.substr extracts substring', () => {
        setupStr8(M, 30, '');
        setupStr8(M + 100, 30, 'Hello World');
        // Push start=6, len=5 to get "World"
        compileAndRun(`u16 6  u16 5  str.substr ${M} ${M + 100}`);
        const str = readStr8(M);
        expect(str.content).toBe('World');
    });

    // STR_FIND tests
    test('str.find returns position when found', () => {
        setupStr8(M, 30, 'Hello World');
        setupStr8(M + 100, 10, 'World');
        compileAndRun(`str.find ${M} ${M + 100}  i16.move_to ${RESULT}`);
        const result = readMemory(RESULT, 2);
        const pos = result[0] | (result[1] << 8);
        expect(pos).toBe(6);
    });

    test('str.find returns -1 when not found', () => {
        setupStr8(M, 30, 'Hello World');
        setupStr8(M + 100, 10, 'xyz');
        compileAndRun(`str.find ${M} ${M + 100}  i16.move_to ${RESULT}`);
        const result = readMemory(RESULT, 2);
        const pos = (result[0] | (result[1] << 8)) << 16 >> 16; // Sign extend
        expect(pos).toBe(-1);
    });

    // STR_CHAR tests
    test('str.char appends character', () => {
        setupStr8(M, 20, 'Hell');
        compileAndRun(`u8 111  str.char ${M}`); // 'o'
        const str = readStr8(M);
        expect(str.content).toBe('Hello');
        expect(str.length).toBe(5);
    });

    test('str.char does not append when at capacity', () => {
        setupStr8(M, 3, 'abc');
        compileAndRun(`u8 100  str.char ${M}`); // 'd'
        const str = readStr8(M);
        expect(str.content).toBe('abc');
        expect(str.length).toBe(3);
    });

    // str16 tests
    test('str16.concat works correctly', () => {
        setupStr16(M, 300, 'Hello');
        setupStr16(M + 350, 300, ' World');
        compileAndRun(`str16.concat ${M} ${M + 350}`);
        const str = readStr16(M);
        expect(str.content).toBe('Hello World');
        expect(str.length).toBe(11);
        expect(str.capacity).toBe(300);
    });

    // =========================================================================
    // Cross-type string operations (str8 <-> str16)
    // =========================================================================

    // str.concat16 - str8 dest, str16 src
    test('str.concat16 appends str16 source to str8 destination', () => {
        setupStr8(M, 30, 'Hello');
        setupStr16(M + 100, 300, ' World'); // str16 source
        compileAndRun(`str.concat16 ${M} ${M + 100}`);
        const str = readStr8(M);
        expect(str.content).toBe('Hello World');
        expect(str.length).toBe(11);
    });

    // str16.concat8 - str16 dest, str8 src
    test('str16.concat8 appends str8 source to str16 destination', () => {
        setupStr16(M, 300, 'Hello');
        setupStr8(M + 350, 20, ' World'); // str8 source
        compileAndRun(`str16.concat8 ${M} ${M + 350}`);
        const str = readStr16(M);
        expect(str.content).toBe('Hello World');
        expect(str.length).toBe(11);
    });

    // str.copy16 - str8 dest, str16 src
    test('str.copy16 copies str16 source to str8 destination', () => {
        setupStr8(M, 30, '');
        setupStr16(M + 100, 300, 'Test Cross');
        compileAndRun(`str.copy16 ${M} ${M + 100}`);
        const str = readStr8(M);
        expect(str.content).toBe('Test Cross');
        expect(str.length).toBe(10);
    });

    // str16.copy8 - str16 dest, str8 src
    test('str16.copy8 copies str8 source to str16 destination', () => {
        setupStr16(M, 300, '');
        setupStr8(M + 350, 20, 'Cross Copy');
        compileAndRun(`str16.copy8 ${M} ${M + 350}`);
        const str = readStr16(M);
        expect(str.content).toBe('Cross Copy');
        expect(str.length).toBe(10);
    });

    // str.cmp16 - str8 vs str16
    test('str.cmp16 compares str8 with str16', () => {
        setupStr8(M, 20, 'Hello');
        setupStr16(M + 100, 300, 'Hello');
        compileAndRun(`str.cmp16 ${M} ${M + 100}  i8.move_to ${RESULT}`);
        expect(readI8(RESULT)).toBe(0); // Equal
    });

    // str16.cmp8 - str16 vs str8
    test('str16.cmp8 compares str16 with str8', () => {
        setupStr16(M, 300, 'Apple');
        setupStr8(M + 350, 20, 'Banana');
        compileAndRun(`str16.cmp8 ${M} ${M + 350}  i8.move_to ${RESULT}`);
        expect(readI8(RESULT)).toBe(-1); // Apple < Banana
    });

    // str.eq16 - str8 vs str16
    test('str.eq16 checks equality between str8 and str16', () => {
        setupStr8(M, 20, 'Hello');
        setupStr16(M + 100, 300, 'Hello');
        compileAndRun(`str.eq16 ${M} ${M + 100}  u8.move_to ${RESULT}`);
        expect(readU8(RESULT)).toBe(1); // Equal
    });

    test('str.eq16 returns 0 for different strings', () => {
        setupStr8(M, 20, 'Hello');
        setupStr16(M + 100, 300, 'World');
        compileAndRun(`str.eq16 ${M} ${M + 100}  u8.move_to ${RESULT}`);
        expect(readU8(RESULT)).toBe(0); // Not equal
    });

    // str16.eq8 - str16 vs str8
    test('str16.eq8 checks equality between str16 and str8', () => {
        setupStr16(M, 300, 'Test');
        setupStr8(M + 350, 20, 'Test');
        compileAndRun(`str16.eq8 ${M} ${M + 350}  u8.move_to ${RESULT}`);
        expect(readU8(RESULT)).toBe(1); // Equal
    });

    // str.find16 - search str16 in str8
    test('str.find16 finds str16 needle in str8 haystack', () => {
        setupStr8(M, 30, 'Hello World');
        setupStr16(M + 100, 300, 'World');
        compileAndRun(`str.find16 ${M} ${M + 100}  i16.move_to ${RESULT}`);
        const result = readMemory(RESULT, 2);
        const pos = result[0] | (result[1] << 8);
        expect(pos).toBe(6);
    });

    // str16.find8 - search str8 in str16
    test('str16.find8 finds str8 needle in str16 haystack', () => {
        setupStr16(M, 300, 'Hello World');
        setupStr8(M + 350, 20, 'World');
        compileAndRun(`str16.find8 ${M} ${M + 350}  i16.move_to ${RESULT}`);
        const result = readMemory(RESULT, 2);
        const pos = result[0] | (result[1] << 8);
        expect(pos).toBe(6);
    });

    // str.substr16 - extract from str16 to str8
    test('str.substr16 extracts from str16 to str8', () => {
        setupStr8(M, 30, '');
        setupStr16(M + 100, 300, 'Hello World');
        compileAndRun(`u16 6  u16 5  str.substr16 ${M} ${M + 100}`);
        const str = readStr8(M);
        expect(str.content).toBe('World');
    });

    // str16.substr8 - extract from str8 to str16
    test('str16.substr8 extracts from str8 to str16', () => {
        setupStr16(M, 300, '');
        setupStr8(M + 350, 30, 'Hello World');
        compileAndRun(`u16 0  u16 5  str16.substr8 ${M} ${M + 350}`);
        const str = readStr16(M);
        expect(str.content).toBe('Hello');
    });

    return {
        name: 'String Operations (str8/str16)',
        passed,
        failed,
        total: testResults.length,
        tests: testResults
    };
}

// Run standalone if executed directly
const isMainModule = process.argv[1]?.replace(/\\/g, '/').endsWith('test_string.js');
if (isMainModule) {
    runTests({ verbose: true }).then(result => {
        console.log(`\n${COLOR.cyan}${result.name}${COLOR.reset}: ${COLOR.green}${result.passed} passed${COLOR.reset}, ${result.failed > 0 ? COLOR.red : ''}${result.failed} failed${COLOR.reset}`);
        if (result.failed > 0) {
            console.log('\nFailures:');
            for (const t of result.tests.filter(t => !t.passed)) {
                console.log(`  ${COLOR.red}✗${COLOR.reset} ${t.name}: ${t.error}`);
            }
            process.exit(1);
        }
    }).catch(e => {
        console.error(e);
        process.exit(1);
    });
}
