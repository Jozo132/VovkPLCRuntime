// test_target_flags.js - Test target flags for project compilation
import VovkPLC from '../dist/VovkPLC.js';

const plc = new VovkPLC();
await plc.initialize('./wasm/dist/VovkPLC.wasm', false, true);

// Project using timers
const projectWithTimer = `
VOVKPLCPROJECT TestTimer
VERSION 1.0

PROGRAM Main
    BLOCK LANG=STL Code
        LD X0.0
        TON T0 #1000
        ST Y0.0
    END_BLOCK
END_PROGRAM
`;

// Test with all features enabled (default)
console.log('Test 1: Compile with all features enabled (default)');
const result1 = plc.compileProject(projectWithTimer);
console.log('  Result:', result1.problem ? 'ERROR: ' + result1.problem.message : 'SUCCESS');

// Test with timers disabled
console.log('\nTest 2: Compile with timers disabled');
const flags = VovkPLC.RUNTIME_FLAGS;
const noTimersFlags = 0xFFFF & ~flags.TIMERS;
console.log('  Flags: 0x' + noTimersFlags.toString(16).padStart(4, '0'));
const result2 = plc.compileProject(projectWithTimer, { targetFlags: noTimersFlags });
console.log('  Result:', result2.problem ? 'ERROR: ' + result2.problem.message : 'SUCCESS');

// Decode flags
console.log('\nTest 3: Decode runtime flags');
const decoded = VovkPLC.decodeRuntimeFlags(noTimersFlags);
console.log('  Decoded:', JSON.stringify(decoded, null, 2));

// Verify expected behavior
const passed = result1.bytecode !== null && result2.problem !== null && result2.problem.message.includes('timer');
console.log('\n' + (passed ? '✓ All tests passed!' : '✗ Tests failed!'));
process.exit(passed ? 0 : 1);
