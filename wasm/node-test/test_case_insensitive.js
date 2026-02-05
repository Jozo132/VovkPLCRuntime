// Test case-insensitivity and multiple push syntax variants
import VovkPLC from '../dist/VovkPLC.js';
import path from 'path';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm');

// Test cases: [name, plcasm, expectedBytecodeHex]
// Bytecode format: opcode + value + EXIT(FF)
const testCases = [
    // Case-insensitive type.const - all should produce same bytecode
    ['u8.const lowercase', 'u8.const 5', '0305FF'],
    ['U8.CONST uppercase', 'U8.CONST 5', '0305FF'],
    ['U8.Const mixedcase', 'U8.Const 5', '0305FF'],
    
    // type.push variant - same bytecode
    ['u8.push lowercase', 'u8.push 10', '030AFF'],
    ['U8.PUSH uppercase', 'U8.PUSH 10', '030AFF'],
    
    // shorthand (just type name) - same bytecode
    ['u8 shorthand', 'u8 15', '030FFF'],
    ['U8 shorthand uppercase', 'U8 15', '030FFF'],
    
    // Different types - verify bytecode (little-endian: 256 = 0x00 0x01)
    ['u16 shorthand', 'u16 256', '040001FF'], // u16 256 = 0x0001 (little-endian)
    ['U16.CONST uppercase', 'U16.CONST 256', '040001FF'],
    
    // Case-insensitive opcodes - NOP = 0x00
    ['NOP uppercase', 'NOP', '00FF'],
    ['nop lowercase', 'nop', '00FF'],
    ['Nop mixedcase', 'Nop', '00FF'],
    
    // Multiple instructions with mixed case
    // U8 5 = 03 05, U8 10 = 03 0A, U8.ADD = 20 03 (ADD + type_u8), EXIT = FF
    ['mixed case program', 'U8 5\nU8 10\nU8.ADD', '0305030A2003FF'],
];

/**
 * Run case-insensitivity tests and return results
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

    const testResults = [];
    let passed = 0;
    let failed = 0;

    for (const [name, plcasm, expectedBytecode] of testCases) {
        try {
            // Compile PLCASM
            const result = plc.compilePLCASM(plcasm);
            
            // Output is already a hex string like "0305FF"
            const bytecodeHex = result.output.toUpperCase();

            if (bytecodeHex === expectedBytecode) {
                passed++;
                testResults.push({ name, passed: true });
            } else {
                failed++;
                testResults.push({ name, passed: false, error: `Expected [${expectedBytecode}], got [${bytecodeHex}]` });
            }
        } catch (e) {
            failed++;
            testResults.push({ name, passed: false, error: e.message });
        }
    }

    return {
        name: 'Case-Insensitive PLCASM',
        passed,
        failed,
        total: testCases.length,
        tests: testResults
    };
}

// Run standalone if executed directly
const isMainModule = process.argv[1]?.replace(/\\/g, '/').endsWith('test_case_insensitive.js');
if (isMainModule) {
    runTests({ verbose: true }).then(result => {
        console.log('Testing case-insensitivity and push syntax variants...\n');
        for (const t of result.tests) {
            console.log(`${t.passed ? '✅' : '❌'} ${t.name}${t.error ? ': ' + t.error : ''}`);
        }
        console.log(`\n${result.passed}/${result.total} tests passed`);
        process.exit(result.failed > 0 ? 1 : 0);
    });
}
