import VovkPLC from '../dist/VovkPLC.js';
import path from 'path';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));

const run = async () => {
    const runtime = await VovkPLC.createWorker(path.join(__dirname, '../dist/VovkPLC.wasm'), {silent: true});
    
    // Test 1: Timer in nested OR block
    console.log('=== Test 1: Timer with contact in nested elements ===');
    const timerLadder = JSON.stringify({
        networks: [{
            elements: [
                { type: 'contact', address: 'I0.0' },
                { type: 'timer_ton', address: 'T0', preset: 'T#1s' },
                { type: 'coil', address: 'Q0.0' }
            ]
        }]
    });
    
    let result = await runtime.compileLadder(timerLadder);
    console.log('Output:\n' + result.output);
    
    // Test 2: Timer with literal preset in nested OR
    console.log('=== Test 2: Timer in OR branch ===');
    const timerOrLadder = JSON.stringify({
        networks: [{
            elements: [
                { type: 'or', branches: [
                    { elements: [
                        { type: 'contact', address: 'I0.0' },
                        { type: 'timer_ton', address: 'T0', preset: 'T#500ms' }
                    ]},
                    { elements: [
                        { type: 'contact', address: 'I0.1' }
                    ]}
                ]},
                { type: 'coil', address: 'Q0.0' }
            ]
        }]
    });
    
    result = await runtime.compileLadder(timerOrLadder);
    console.log('Output:\n' + result.output);
    
    // Test 3: Counter
    console.log('=== Test 3: Counter ===');
    const counterLadder = JSON.stringify({
        networks: [{
            elements: [
                { type: 'contact', address: 'I0.0' },
                { type: 'counter_u', address: 'C0', preset: 10 },
                { type: 'coil', address: 'Q0.0' }
            ]
        }]
    });
    
    result = await runtime.compileLadder(counterLadder);
    console.log('Output:\n' + result.output);
    
    // Test 4: Multiple timers
    console.log('=== Test 4: TOF Timer ===');
    const tofLadder = JSON.stringify({
        networks: [{
            elements: [
                { type: 'contact', address: 'I0.0' },
                { type: 'timer_tof', address: 'T1', preset: 'T#2s' },
                { type: 'coil', address: 'Q0.0' }
            ]
        }]
    });
    
    result = await runtime.compileLadder(tofLadder);
    console.log('Output:\n' + result.output);
    
    await runtime.terminate();
    console.log('All tests completed!');
    process.exit(0);
};

run().catch(e => { console.error(e); process.exit(1); });
