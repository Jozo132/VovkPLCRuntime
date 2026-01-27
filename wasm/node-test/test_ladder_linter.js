// test_ladder_linter.js - Test the ladder linter validation

import VovkPLC from '../dist/VovkPLC.js';
import path from 'path';
import fs from 'fs';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm');

const run = async () => {
    console.log(`Loading WASM from: ${wasmPath}`);

    if (!fs.existsSync(wasmPath)) {
        console.error('WASM file not found!');
        process.exit(1);
    }

    const runtime = await VovkPLC.createWorker(wasmPath, { silent: true });

    try {
        console.log('='.repeat(70));
        console.log('Ladder Linter Tests');
        console.log('='.repeat(70));

        let passed = 0;
        let failed = 0;

        async function test(name, ladderJson, expectErrors, expectWarnings = 0) {
            console.log(`\nTest: ${name}`);
            console.log('-'.repeat(50));

            try {
                const result = await runtime.lintLadder(ladderJson);

                const errors = result.problems.filter(p => p.type === 'error');
                const warnings = result.problems.filter(p => p.type === 'warning');

                console.log(`  Errors: ${errors.length}, Warnings: ${warnings.length}`);

                for (const p of result.problems) {
                    console.log(`    [${p.type}] (${p.line}:${p.column}) ${p.message}`);
                }

                if (errors.length === expectErrors && warnings.length >= expectWarnings) {
                    console.log('  PASSED');
                    passed++;
                } else {
                    console.log(`  FAILED: Expected ${expectErrors} errors and ${expectWarnings}+ warnings`);
                    failed++;
                }
            } catch (e) {
                console.log(`  FAILED with exception: ${e.message}`);
                failed++;
            }
        }

        // Test 1: Valid ladder diagram
        await test('Valid simple ladder', JSON.stringify({
            nodes: [
                { id: 'n1', type: 'contact_no', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'coil', x: 2, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n2'] }
            ]
        }), 0, 0);

        // Test 2: Duplicate node positions
        await test('Duplicate node positions', JSON.stringify({
            nodes: [
                { id: 'n1', type: 'contact_no', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'contact_nc', x: 0, y: 0, address: 'X0.1' },  // Same position
                { id: 'n3', type: 'coil', x: 2, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1', 'n2'], destinations: ['n3'] }
            ]
        }), 1, 0);  // Expect 1 error for duplicate position

        // Test 3: Backward connection (destination x <= source x)
        await test('Backward connection', JSON.stringify({
            nodes: [
                { id: 'n1', type: 'contact_no', x: 2, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'coil', x: 1, y: 0, address: 'Y0.0' }  // x=1 is left of x=2
            ],
            connections: [
                { sources: ['n1'], destinations: ['n2'] }
            ]
        }), 1, 0);  // Expect 1 error for backward connection

        // Test 4: Connection to non-existing node
        await test('Connection to non-existing node', JSON.stringify({
            nodes: [
                { id: 'n1', type: 'contact_no', x: 0, y: 0, address: 'X0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n99'] }  // n99 doesn't exist
            ]
        }), 1, 0);  // Expect 1 error for missing destination node

        // Test 5: Unknown node type
        await test('Unknown node type', JSON.stringify({
            nodes: [
                { id: 'n1', type: 'unknown_type', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'coil', x: 2, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n2'] }
            ]
        }), 1, 0);  // Expect 1 error for unknown type

        // Test 6: Disconnected node (dead node)
        await test('Disconnected node', JSON.stringify({
            nodes: [
                { id: 'n1', type: 'contact_no', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'contact_no', x: 0, y: 1, address: 'X0.1' },  // No connections
                { id: 'n3', type: 'coil', x: 2, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n3'] }
            ]
        }), 1, 0);  // Expect 1 error for completely disconnected node

        // Test 7: Missing address
        await test('Missing address', JSON.stringify({
            nodes: [
                { id: 'n1', type: 'contact_no', x: 0, y: 0, address: '' },  // No address
                { id: 'n2', type: 'coil', x: 2, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n2'] }
            ]
        }), 1, 0);  // Expect 1 error for missing address

        // Test 8: Timer without preset
        await test('Timer without preset', JSON.stringify({
            nodes: [
                { id: 'n1', type: 'contact_no', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'timer_ton', x: 1, y: 0, address: 'T0' },  // No preset
                { id: 'n3', type: 'coil', x: 2, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n2'] },
                { sources: ['n2'], destinations: ['n3'] }
            ]
        }), 0, 1);  // Expect 1 warning for timer without preset

        // Test 9: Operation block missing inputs
        await test('Operation block missing inputs', JSON.stringify({
            nodes: [
                { id: 'n1', type: 'contact_no', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'fb_add', x: 1, y: 0, address: 'M0', in1: '', in2: '', out: '' },  // Missing all I/O
                { id: 'n3', type: 'coil', x: 2, y: 0, address: 'Y0.0' }
            ],
            connections: [
                { sources: ['n1'], destinations: ['n2'] },
                { sources: ['n2'], destinations: ['n3'] }
            ]
        }), 3, 0);  // Expect 3 errors for missing in1, in2, out

        // Test 10: Contact not connected to output (warning)
        await test('Contact not connected to output', JSON.stringify({
            nodes: [
                { id: 'n1', type: 'contact_no', x: 0, y: 0, address: 'X0.0' },
                { id: 'n2', type: 'coil', x: 2, y: 0, address: 'Y0.0' }
            ],
            connections: []  // n1 has no connections
        }), 2, 2);  // Expect 2 errors for disconnected nodes, 2 warnings for orphaned contact and output without input

        console.log('\n' + '='.repeat(70));
        console.log(`Results: ${passed} passed, ${failed} failed`);
        console.log('='.repeat(70));

        process.exitCode = failed > 0 ? 1 : 0;
    } finally {
        await runtime.terminate();
    }
};

run().catch(err => {
    console.error('Unhandled Error:', err);
    process.exit(1);
});
