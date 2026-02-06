// @ts-check
// Test file for PLCScript string support (str8/str16)
// Tests string operations through the project compiler

import VovkPLC from '../../dist/VovkPLC.js';
import path from 'path';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm');

/**
 * @typedef {Object} TestResult
 * @property {string} name - Test name
 * @property {boolean} passed - Whether the test passed
 * @property {string} [error] - Error message if failed
 * @property {string} [info] - Additional info
 */

/**
 * @typedef {Object} SuiteResult
 * @property {string} name - Suite name
 * @property {number} passed - Number of passed tests
 * @property {number} failed - Number of failed tests
 * @property {number} total - Total number of tests
 * @property {TestResult[]} tests - Individual test results
 * @property {Object[]} [failures] - Detailed failure information
 */

/**
 * Run PLCScript String unit tests
 * @param {Object} [options] - Options
 * @param {boolean} [options.silent] - If true, suppress console output
 * @param {VovkPLC} [options.runtime] - Shared runtime instance (creates one if not provided)
 * @returns {Promise<SuiteResult>}
 */
export async function runTests(options = {}) {
    const { silent = false, runtime: sharedRuntime = null } = options;
    const log = silent ? () => {} : console.log.bind(console);
    
    // Use shared runtime if provided, otherwise create one (for standalone execution)
    let plc = sharedRuntime;
    if (!plc) {
        plc = new VovkPLC();
        plc.stdout_callback = () => {}; // Suppress output
        await plc.initialize(wasmPath, false, true);
    }

    /** @type {TestResult[]} */
    const testResults = [];
    /** @type {Object[]} */
    const failures = [];

    /**
     * Wraps PLCScript code in a project format
     * @param {string} code
     * @returns {string}
     */
    function wrapProject(code) {
        return `VOVKPLCPROJECT Test
VERSION 1
PROGRAM main
BLOCK LANG=PLCSCRIPT main
${code}
END_BLOCK
END_PROGRAM
`;
    }

    /**
     * @param {string} name
     * @param {() => void} fn
     */
    function test(name, fn) {
        try {
            fn();
            testResults.push({ name, passed: true });
        } catch (e) {
            const errorMsg = e instanceof Error ? e.message : String(e);
            testResults.push({ name, passed: false, error: errorMsg });
            failures.push({ name, error: errorMsg });
        }
    }

    /**
     * @param {string} code
     * @param {string} expectedError
     */
    function assertCompileError(code, expectedError) {
        const result = plc.compileProject(wrapProject(code));
        if (!result.problem) {
            throw new Error(`Expected compile error containing "${expectedError}", but compilation succeeded`);
        }
        if (!result.problem.message.includes(expectedError)) {
            throw new Error(`Expected error containing "${expectedError}", got: ${result.problem.message}`);
        }
    }

    /**
     * @param {string} code
     * @returns {object}
     */
    function compile(code) {
        const result = plc.compileProject(wrapProject(code));
        if (result.problem) {
            throw new Error(`Compilation failed: ${result.problem.message}`);
        }
        return result;
    }

    // ========================================================================
    // String Declaration Tests
    // ========================================================================
    
    test('String variable declaration with str8', () => {
        compile(`let message: str8[32] @ M0;`);
    });
    
    test('String variable declaration with str16', () => {
        compile(`let message: str16[100] @ M0;`);
    });
    
    test('String declaration requires capacity', () => {
        assertCompileError(`let message: str8 @ M0;`, 'requires capacity');
    });

    // ========================================================================
    // String Literal Assignment Tests
    // ========================================================================
    
    test('String assignment from literal', () => {
        compile(`
            let msg: str8[32] @ M0;
            msg = "Hello";
        `);
    });
    
    test('String assignment with escape sequences', () => {
        compile(`
            let msg: str8[32] @ M0;
            msg = "Hi\\n";
        `);
    });

    // ========================================================================
    // String Copy Tests
    // ========================================================================
    
    test('String copy from another variable', () => {
        compile(`
            let src: str8[32] @ M0;
            let dest: str8[32] @ M34;
            dest = src;
        `);
    });

    // ========================================================================
    // String Concatenation Tests
    // ========================================================================
    
    test('String concatenation with += literal', () => {
        compile(`
            let msg: str8[64] @ M0;
            msg += "World";
        `);
    });
    
    test('String concatenation with += variable', () => {
        compile(`
            let a: str8[32] @ M0;
            let b: str8[32] @ M34;
            a += b;
        `);
    });

    // ========================================================================
    // String Comparison Tests
    // ========================================================================
    
    test('String equality comparison', () => {
        compile(`
            let a: str8[32] @ M0;
            let b: str8[32] @ M34;
            let result: bool @ M68;
            result = a == b;
        `);
    });
    
    test('String inequality comparison', () => {
        compile(`
            let a: str8[32] @ M0;
            let b: str8[32] @ M34;
            let result: bool @ M68;
            result = a != b;
        `);
    });

    // ========================================================================
    // String Property Tests
    // ========================================================================
    
    test('String .length property', () => {
        compile(`
            let msg: str8[32] @ M0;
            let len: u16 @ M34;
            len = msg.length;
        `);
    });
    
    test('String .capacity property', () => {
        compile(`
            let msg: str8[32] @ M0;
            let cap: u16 @ M34;
            cap = msg.capacity;
        `);
    });

    // ========================================================================
    // String Method Tests
    // ========================================================================
    
    test('String .clear() method', () => {
        compile(`
            let msg: str8[32] @ M0;
            msg.clear();
        `);
    });
    
    test('String .indexOf() method', () => {
        compile(`
            let haystack: str8[64] @ M0;
            let needle: str8[32] @ M66;
            let pos: i16 @ M100;
            pos = haystack.indexOf(needle);
        `);
    });
    
    test('String .includes() method', () => {
        compile(`
            let haystack: str8[64] @ M0;
            let needle: str8[32] @ M66;
            let found: bool @ M100;
            found = haystack.includes(needle);
        `);
    });
    
    test('String .charAt() method', () => {
        compile(`
            let msg: str8[32] @ M0;
            let ch: u8 @ M34;
            ch = msg.charAt(0);
        `);
    });
    
    test('String .concat() method', () => {
        compile(`
            let dest: str8[64] @ M0;
            let src: str8[32] @ M66;
            dest.concat(src);
        `);
    });

    // ========================================================================
    // String in Control Flow Tests
    // ========================================================================
    
    test('String equality in if condition', () => {
        compile(`
            let a: str8[32] @ M0;
            let b: str8[32] @ M34;
            let result: u8 @ M68;
            if (a == b) {
                result = 1;
            }
        `);
    });
    
    test('String includes in if condition', () => {
        compile(`
            let msg: str8[64] @ M0;
            let search: str8[16] @ M66;
            let result: u8 @ M84;
            if (msg.includes(search)) {
                result = 1;
            }
        `);
    });

    // Build final result
    const passed = testResults.filter(t => t.passed).length;
    const failed = testResults.filter(t => !t.passed).length;

    return {
        name: 'PLCScript String Support',
        passed,
        failed,
        total: testResults.length,
        tests: testResults,
        failures: failures.length > 0 ? failures : undefined
    };
}

// If run directly
const isMainModule = import.meta.url === `file://${process.argv[1]}`.replace(/\\/g, '/');
if (isMainModule) {
    const result = await runTests({ silent: false });
    console.log(`\nPLCScript string tests: ${result.passed} passed, ${result.failed} failed`);
    process.exit(result.failed > 0 ? 1 : 0);
}
