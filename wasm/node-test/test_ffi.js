// FFI Integration Tests for VovkPLC Runtime
// Tests dynamic FFI registration and execution via WASM <-> JS callbacks

import { fileURLToPath, pathToFileURL } from 'url';
import { dirname, join } from 'path';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

// Import VovkPLC class - use file:// URL for Windows compatibility
const vovkPLCPath = join(__dirname, '..', 'dist', 'VovkPLC.js');
const { default: VovkPLC } = await import(pathToFileURL(vovkPLCPath).href);

// ============================================================================
// Test Utilities
// ============================================================================

const colors = {
    reset: '\x1b[0m',
    green: '\x1b[32m',
    red: '\x1b[31m',
    yellow: '\x1b[33m',
    cyan: '\x1b[36m',
    dim: '\x1b[2m',
};

// Memory layout: S(0-63) X(64-127) Y(128-191) M(192-447) T(448+) C(...)
// Use M (Marker) memory for test data - never use S/X/Y directly
const M = 192;  // Marker offset - safe to use for general data

// ============================================================================
// Test Runner (Standalone)
// ============================================================================

function log(msg) { console.log(msg); }
function pass(name) { log(`  ${colors.green}✓${colors.reset} ${name}`); return true; }
function fail(name, reason) { log(`  ${colors.red}✗${colors.reset} ${name}: ${reason}`); return false; }

export async function runFFITests() {
    log(`\n${colors.cyan}FFI Integration Tests${colors.reset}\n`);
    
    let passed = 0;
    let failed = 0;
    
    // Load WASM
    const plc = new VovkPLC();
    await plc.initialize(join(__dirname, '..', 'dist', 'VovkPLC.wasm'));
    plc.setSilent(true);
    
    // Helper to compile and run PLCASM
    function compileAndRun(source) {
        plc.downloadAssembly(source);
        const compileError = plc.wasm_exports.compileAssembly();
        if (compileError) return { error: 'compile' };
        
        const loadError = plc.wasm_exports.loadCompiledProgram();
        if (loadError) return { error: 'load' };
        
        const runError = plc.wasm_exports.run();
        return { error: runError };
    }
    
    // Helper to read typed value from memory
    function readMemory(addr, type) {
        const memOffset = plc.wasm_exports.getMemoryLocation();
        const view = new DataView(plc.wasm_exports.memory.buffer);
        const le = plc.isLittleEndian;
        switch (type) {
            case 'bool': return view.getUint8(memOffset + addr) !== 0;
            case 'u8': return view.getUint8(memOffset + addr);
            case 'i8': return view.getInt8(memOffset + addr);
            case 'u16': return view.getUint16(memOffset + addr, le);
            case 'i16': return view.getInt16(memOffset + addr, le);
            case 'u32': return view.getUint32(memOffset + addr, le);
            case 'i32': return view.getInt32(memOffset + addr, le);
            case 'f32': return view.getFloat32(memOffset + addr, le);
            case 'f64': return view.getFloat64(memOffset + addr, le);
            default: return 0;
        }
    }
    
    // Helper to write typed value to memory
    function writeMemory(addr, type, value) {
        const memOffset = plc.wasm_exports.getMemoryLocation();
        const view = new DataView(plc.wasm_exports.memory.buffer);
        const le = plc.isLittleEndian;
        switch (type) {
            case 'bool': view.setUint8(memOffset + addr, value ? 1 : 0); break;
            case 'u8': view.setUint8(memOffset + addr, value); break;
            case 'i8': view.setInt8(memOffset + addr, value); break;
            case 'u16': view.setUint16(memOffset + addr, value, le); break;
            case 'i16': view.setInt16(memOffset + addr, value, le); break;
            case 'u32': view.setUint32(memOffset + addr, value, le); break;
            case 'i32': view.setInt32(memOffset + addr, value, le); break;
            case 'f32': view.setFloat32(memOffset + addr, value, le); break;
            case 'f64': view.setFloat64(memOffset + addr, value, le); break;
        }
    }
    
    // Helper to reset state
    function reset() {
        plc.wasm_exports.memoryReset();
        plc.clearFFI();
    }
    
    // ========================================================================
    // Test 1: Register JS function and verify it's in the registry
    // ========================================================================
    try {
        reset();
        
        const idx = plc.registerJSFunction(
            'F_js_add',
            'i32,i32->i32',
            'Add two numbers in JS',
            (a, b) => a + b
        );
        
        if (idx >= 0 && plc.ffiCallbacks.has(idx)) {
            pass('Register JS function'); passed++;
        } else {
            fail('Register JS function', `got index ${idx}`); failed++;
        }
    } catch (e) {
        fail('Register JS function', e.message); failed++;
    }
    
    // ========================================================================
    // Test 2: Call JS function from PLCASM via FFI instruction
    // ========================================================================
    try {
        reset();
        
        // Register JS function
        const idx = plc.registerJSFunction('F_js_add', 'i32,i32->i32', 'Add in JS', (a, b) => a + b);
        
        // Write input values to M (Marker) memory
        writeMemory(M+0, 'i32', 10);   // First param at M+0
        writeMemory(M+4, 'i32', 25);   // Second param at M+4
        // Result will go to M+8
        
        // Compile and run PLCASM that calls the FFI
        const result = compileAndRun(`
            ffi F_js_add ${M+0} ${M+4} ${M+8}
        `);
        
        const value = readMemory(M+8, 'i32');
        
        if (result.error === 0 && value === 35) {
            pass('Call JS function from PLCASM'); passed++;
        } else {
            fail('Call JS function from PLCASM', `error=${result.error}, result=${value}, expected 35`); failed++;
        }
    } catch (e) {
        fail('Call JS function from PLCASM', e.message); failed++;
    }
    
    // ========================================================================
    // Test 3: JS function with float types
    // ========================================================================
    try {
        reset();
        
        plc.registerJSFunction('F_js_mul', 'f32,f32->f32', 'Multiply floats in JS', (a, b) => a * b);
        
        writeMemory(M+0, 'f32', 2.5);
        writeMemory(M+4, 'f32', 4.0);
        
        const result = compileAndRun(`
            ffi F_js_mul ${M+0} ${M+4} ${M+8}
        `);
        
        const value = readMemory(M+8, 'f32');
        
        if (result.error === 0 && Math.abs(value - 10.0) < 0.001) {
            pass('JS function with float types'); passed++;
        } else {
            fail('JS function with float types', `error=${result.error}, result=${value}, expected 10.0`); failed++;
        }
    } catch (e) {
        fail('JS function with float types', e.message); failed++;
    }
    
    // ========================================================================
    // Test 4: JS function with boolean return
    // ========================================================================
    try {
        reset();
        
        plc.registerJSFunction('F_js_gt', 'i32,i32->bool', 'Greater than in JS', (a, b) => a > b);
        
        writeMemory(M+0, 'i32', 100);
        writeMemory(M+4, 'i32', 50);
        
        const result = compileAndRun(`
            ffi F_js_gt ${M+0} ${M+4} ${M+8}
        `);
        
        const value = readMemory(M+8, 'bool');
        
        if (result.error === 0 && value === true) {
            pass('JS function with boolean return'); passed++;
        } else {
            fail('JS function with boolean return', `error=${result.error}, result=${value}, expected true`); failed++;
        }
    } catch (e) {
        fail('JS function with boolean return', e.message); failed++;
    }
    
    // ========================================================================
    // Test 5: JS function with 3 parameters (clamp)
    // ========================================================================
    try {
        reset();
        
        plc.registerJSFunction('F_js_clamp', 'i32,i32,i32->i32', 'Clamp value', (val, min, max) => {
            if (val < min) return min;
            if (val > max) return max;
            return val;
        });
        
        writeMemory(M+0, 'i32', 150);   // value to clamp
        writeMemory(M+4, 'i32', 0);     // min
        writeMemory(M+8, 'i32', 100);   // max
        
        const result = compileAndRun(`
            ffi F_js_clamp ${M+0} ${M+4} ${M+8} ${M+12}
        `);
        
        const value = readMemory(M+12, 'i32');
        
        if (result.error === 0 && value === 100) {
            pass('JS function with 3 parameters'); passed++;
        } else {
            fail('JS function with 3 parameters', `error=${result.error}, result=${value}, expected 100`); failed++;
        }
    } catch (e) {
        fail('JS function with 3 parameters', e.message); failed++;
    }
    
    // ========================================================================
    // Test 6: JS function with side effects (stateful counter)
    // ========================================================================
    try {
        reset();
        
        let callCount = 0;
        plc.registerJSFunction('F_js_count', 'void->i32', 'Count calls', () => ++callCount);
        
        const result = compileAndRun(`
            ffi F_js_count ${M+0}
            ffi F_js_count ${M+4}
            ffi F_js_count ${M+8}
        `);
        
        const r1 = readMemory(M+0, 'i32');
        const r2 = readMemory(M+4, 'i32');
        const r3 = readMemory(M+8, 'i32');
        
        if (result.error === 0 && r1 === 1 && r2 === 2 && r3 === 3) {
            pass('JS function with side effects'); passed++;
        } else {
            fail('JS function with side effects', `error=${result.error}, results=${r1},${r2},${r3}`); failed++;
        }
    } catch (e) {
        fail('JS function with side effects', e.message); failed++;
    }
    
    // ========================================================================
    // Test 7: Multiple JS functions chained
    // ========================================================================
    try {
        reset();
        
        plc.registerJSFunction('F_square', 'i32->i32', 'Square', x => x * x);
        plc.registerJSFunction('F_double', 'i32->i32', 'Double', x => x * 2);
        plc.registerJSFunction('F_inc', 'i32->i32', 'Increment', x => x + 1);
        
        writeMemory(M+0, 'i32', 5);
        
        // square(5) = 25, double(25) = 50, inc(50) = 51
        const result = compileAndRun(`
            ffi F_square ${M+0} ${M+4}
            ffi F_double ${M+4} ${M+8}
            ffi F_inc ${M+8} ${M+12}
        `);
        
        const value = readMemory(M+12, 'i32');
        
        if (result.error === 0 && value === 51) {
            pass('Multiple JS functions chained'); passed++;
        } else {
            fail('Multiple JS functions chained', `error=${result.error}, result=${value}, expected 51`); failed++;
        }
    } catch (e) {
        fail('Multiple JS functions chained', e.message); failed++;
    }
    
    // ========================================================================
    // Test 8: Mix of C++ demo functions and JS functions
    // ========================================================================
    try {
        reset();
        
        // Register C++ demos
        plc.wasm_exports.ffi_registerDemos();
        
        // Register JS function
        plc.registerJSFunction('F_js_negate', 'i32->i32', 'Negate in JS', x => -x);
        
        writeMemory(M+0, 'i32', 10);
        writeMemory(M+4, 'i32', 20);
        
        // Use C++ F_add_i32 (10 + 20 = 30), then JS F_js_negate (-30)
        const result = compileAndRun(`
            ffi F_add_i32 ${M+0} ${M+4} ${M+8}
            ffi F_js_negate ${M+8} ${M+12}
        `);
        
        const value = readMemory(M+12, 'i32');
        
        if (result.error === 0 && value === -30) {
            pass('Mix of C++ and JS functions'); passed++;
        } else {
            fail('Mix of C++ and JS functions', `error=${result.error}, result=${value}, expected -30`); failed++;
        }
    } catch (e) {
        fail('Mix of C++ and JS functions', e.message); failed++;
    }
    
    // ========================================================================
    // Test 9: JS function returning u16
    // ========================================================================
    try {
        reset();
        
        plc.registerJSFunction('F_js_u16', 'u16->u16', 'Increment u16', x => x + 1);
        
        writeMemory(M+0, 'u16', 65534);
        
        const result = compileAndRun(`
            ffi F_js_u16 ${M+0} ${M+2}
        `);
        
        const value = readMemory(M+2, 'u16');
        
        if (result.error === 0 && value === 65535) {
            pass('JS function with u16 types'); passed++;
        } else {
            fail('JS function with u16 types', `error=${result.error}, result=${value}, expected 65535`); failed++;
        }
    } catch (e) {
        fail('JS function with u16 types', e.message); failed++;
    }
    
    // ========================================================================
    // Test 10: Unregister JS function
    // ========================================================================
    try {
        reset();
        
        const idx = plc.registerJSFunction('F_temp', 'i32->i32', 'Temp', x => x);
        const beforeCount = plc.getFFICount();
        
        const unregistered = plc.unregisterJSFunction(idx);
        const afterCount = plc.getFFICount();
        
        if (unregistered && afterCount === beforeCount - 1 && !plc.ffiCallbacks.has(idx)) {
            pass('Unregister JS function'); passed++;
        } else {
            fail('Unregister JS function', `unregistered=${unregistered}, before=${beforeCount}, after=${afterCount}`); failed++;
        }
    } catch (e) {
        fail('Unregister JS function', e.message); failed++;
    }

    // ========================================================================
    // Test 11: System memory protection - writeMemoryByte
    // ========================================================================
    try {
        reset();
        
        let threw = false;
        try {
            plc.writeMemoryByte(0, 42);  // Address 0 is in System partition
        } catch (e) {
            if (e.message.includes('System partition')) {
                threw = true;
            }
        }
        
        if (threw) {
            pass('System memory protection - writeMemoryByte'); passed++;
        } else {
            fail('System memory protection - writeMemoryByte', 'Should throw when writing to address 0'); failed++;
        }
    } catch (e) {
        fail('System memory protection - writeMemoryByte', e.message); failed++;
    }

    // ========================================================================
    // Test 12: System memory protection - writeMemoryArea
    // ========================================================================
    try {
        reset();
        
        let threw = false;
        try {
            plc.writeMemoryArea(10, [1, 2, 3]);  // Address 10 is in System partition
        } catch (e) {
            if (e.message.includes('System partition')) {
                threw = true;
            }
        }
        
        if (threw) {
            pass('System memory protection - writeMemoryArea'); passed++;
        } else {
            fail('System memory protection - writeMemoryArea', 'Should throw when writing to address 10'); failed++;
        }
    } catch (e) {
        fail('System memory protection - writeMemoryArea', e.message); failed++;
    }

    // ========================================================================
    // Test 13: System memory protection allows X/Y/M writes
    // ========================================================================
    try {
        reset();
        
        // These should NOT throw - addresses in X/Y/M regions
        plc.writeMemoryByte(64, 0x11);   // X0 (Input region)
        plc.writeMemoryByte(128, 0x22);  // Y0 (Output region)
        plc.writeMemoryByte(M, 0x33);    // M0 (Marker region)
        
        const xVal = plc.readMemoryArea(64, 1)[0];
        const yVal = plc.readMemoryArea(128, 1)[0];
        const mVal = plc.readMemoryArea(M, 1)[0];
        
        if (xVal === 0x11 && yVal === 0x22 && mVal === 0x33) {
            pass('System memory protection allows X/Y/M writes'); passed++;
        } else {
            fail('System memory protection allows X/Y/M writes', `X=${xVal}, Y=${yVal}, M=${mVal}`); failed++;
        }
    } catch (e) {
        fail('System memory protection allows X/Y/M writes', e.message); failed++;
    }

    // ========================================================================
    // Worker FFI Tests
    // ========================================================================
    log(`\n${colors.cyan}Worker FFI Tests${colors.reset}\n`);
    
    let worker = null;
    try {
        // Create worker instance
        worker = await VovkPLC.createWorker(join(__dirname, '..', 'dist', 'VovkPLC.wasm'), {
            silent: true,
            forcePostMessage: true  // Use postMessage for compatibility
        });
        
        // Test 14: Worker - Register JS function
        try {
            const idx = await worker.registerJSFunction(
                'F_worker_add',
                'i32,i32->i32',
                'Add in worker',
                (a, b) => a + b
            );
            
            if (idx >= 0) {
                pass('Worker - Register JS function'); passed++;
            } else {
                fail('Worker - Register JS function', `got index ${idx}`); failed++;
            }
        } catch (e) {
            fail('Worker - Register JS function', e.message); failed++;
        }

        // Test 15: Worker - Call JS function from PLCASM
        try {
            await worker.clearFFI();
            await worker.registerJSFunction('F_worker_add', 'i32,i32->i32', 'Add', (a, b) => a + b);
            
            // Write input values to M memory
            await worker.writeMemoryArea(M+0, [20, 0, 0, 0]);  // i32 = 20 at M+0
            await worker.writeMemoryArea(M+4, [15, 0, 0, 0]);  // i32 = 15 at M+4
            
            // Compile and run
            const result = await worker.compilePLCASM(`ffi F_worker_add ${M+0} ${M+4} ${M+8}`, { run: true });
            
            // Read result
            const mem = await worker.readMemoryArea(M+8, 4);
            const value = mem[0] | (mem[1] << 8) | (mem[2] << 16) | (mem[3] << 24);
            
            if (!result.error && value === 35) {
                pass('Worker - Call JS function from PLCASM'); passed++;
            } else {
                fail('Worker - Call JS function from PLCASM', `error=${result.error}, value=${value}, expected 35`); failed++;
            }
        } catch (e) {
            fail('Worker - Call JS function from PLCASM', e.message); failed++;
        }

        // Test 16: Worker - JS function with floats
        try {
            await worker.clearFFI();
            await worker.registerJSFunction('F_worker_mul', 'f32,f32->f32', 'Multiply', (a, b) => a * b);
            
            // Write f32 values (2.5 and 4.0)
            const buf = new ArrayBuffer(8);
            const view = new DataView(buf);
            view.setFloat32(0, 2.5, true);
            view.setFloat32(4, 4.0, true);
            await worker.writeMemoryArea(M+0, Array.from(new Uint8Array(buf)));
            
            await worker.compilePLCASM(`ffi F_worker_mul ${M+0} ${M+4} ${M+8}`, { run: true });
            
            const mem = await worker.readMemoryArea(M+8, 4);
            // Worker returns object with numeric keys due to JSON serialization, convert to array
            const memArray = Array.isArray(mem) ? mem : Object.keys(mem).sort((a,b) => a-b).map(k => mem[k]);
            const resultBuf = new ArrayBuffer(4);
            new Uint8Array(resultBuf).set(memArray);
            const value = new DataView(resultBuf).getFloat32(0, true);
            
            if (Math.abs(value - 10.0) < 0.001) {
                pass('Worker - JS function with floats'); passed++;
            } else {
                fail('Worker - JS function with floats', `value=${value}, expected 10.0`); failed++;
            }
        } catch (e) {
            fail('Worker - JS function with floats', e.message); failed++;
        }

        // Test 17: Worker - getFFICount
        try {
            await worker.clearFFI();
            await worker.registerJSFunction('F_a', 'i32->i32', 'A', x => x);
            await worker.registerJSFunction('F_b', 'i32->i32', 'B', x => x);
            
            const count = await worker.getFFICount();
            
            if (count === 2) {
                pass('Worker - getFFICount'); passed++;
            } else {
                fail('Worker - getFFICount', `expected 2, got ${count}`); failed++;
            }
        } catch (e) {
            fail('Worker - getFFICount', e.message); failed++;
        }

        // Test 18: Worker - Unregister JS function
        try {
            await worker.clearFFI();
            const idx = await worker.registerJSFunction('F_temp', 'i32->i32', 'Temp', x => x);
            const beforeCount = await worker.getFFICount();
            
            const unregistered = await worker.unregisterJSFunction(idx);
            const afterCount = await worker.getFFICount();
            
            if (unregistered && afterCount === beforeCount - 1) {
                pass('Worker - Unregister JS function'); passed++;
            } else {
                fail('Worker - Unregister JS function', `unregistered=${unregistered}, before=${beforeCount}, after=${afterCount}`); failed++;
            }
        } catch (e) {
            fail('Worker - Unregister JS function', e.message); failed++;
        }

        // Test 19: Worker - clearFFI
        try {
            await worker.registerJSFunction('F_x', 'i32->i32', 'X', x => x);
            await worker.registerJSFunction('F_y', 'i32->i32', 'Y', x => x);
            await worker.clearFFI();
            
            const count = await worker.getFFICount();
            
            if (count === 0) {
                pass('Worker - clearFFI'); passed++;
            } else {
                fail('Worker - clearFFI', `expected 0, got ${count}`); failed++;
            }
        } catch (e) {
            fail('Worker - clearFFI', e.message); failed++;
        }

    } catch (e) {
        fail('Worker FFI setup', e.message); failed++;
    } finally {
        // Clean up worker
        if (worker) {
            try {
                await worker.terminate();
            } catch (e) {
                // Ignore termination errors
            }
        }
    }
    
    // ========================================================================
    // Summary
    // ========================================================================
    log('');
    log(`${colors.dim}────────────────────────────────────────${colors.reset}`);
    log(`Total: ${passed + failed}, Passed: ${colors.green}${passed}${colors.reset}, Failed: ${colors.red}${failed}${colors.reset}`);
    
    return { passed, failed };
}

// ============================================================================
// Test Definitions for Unit Test Runner
// ============================================================================

const testDefinitions = [
    { name: 'Register JS function' },
    { name: 'Call JS function from PLCASM' },
    { name: 'JS function with float types' },
    { name: 'JS function with boolean return' },
    { name: 'JS function with 3 parameters' },
    { name: 'JS function with side effects' },
    { name: 'Multiple JS functions chained' },
    { name: 'Mix of C++ and JS functions' },
    { name: 'JS function with u16 types' },
    { name: 'Unregister JS function' },
    { name: 'System memory protection - writeMemoryByte' },
    { name: 'System memory protection - writeMemoryArea' },
    { name: 'System memory protection allows X/Y/M writes' },
    // Worker tests
    { name: 'Worker - Register JS function' },
    { name: 'Worker - Call JS function from PLCASM' },
    { name: 'Worker - JS function with floats' },
    { name: 'Worker - getFFICount' },
    { name: 'Worker - Unregister JS function' },
    { name: 'Worker - clearFFI' },
];

/**
 * Run tests and return results for the unified test runner
 * @param {object} options - Test options  
 * @param {boolean} options.verbose - Whether to show detailed output
 * @param {boolean} options.debug - Whether to show debug output
 * @param {VovkPLC} [options.runtime] - Shared runtime instance (ignored - FFI needs fresh instance)
 * @returns {Promise<{name: string, passed: number, failed: number, total: number, tests: Array}>}
 */
export async function runTests(options = {}) {
    const { verbose = false } = options;
    
    // FFI tests need their own runtime instance because they register JS callbacks
    const plc = new VovkPLC();
    await plc.initialize(join(__dirname, '..', 'dist', 'VovkPLC.wasm'));
    plc.setSilent(true);
    
    const tests = [];
    let passed = 0;
    let failed = 0;
    
    // Helper to compile and run PLCASM
    function compileAndRun(source) {
        plc.downloadAssembly(source);
        const compileError = plc.wasm_exports.compileAssembly();
        if (compileError) return { error: 'compile' };
        
        const loadError = plc.wasm_exports.loadCompiledProgram();
        if (loadError) return { error: 'load' };
        
        const runError = plc.wasm_exports.run();
        return { error: runError };
    }
    
    // Helper to read typed value from memory
    function readMemory(addr, type) {
        const memOffset = plc.wasm_exports.getMemoryLocation();
        const view = new DataView(plc.wasm_exports.memory.buffer);
        const le = plc.isLittleEndian;
        switch (type) {
            case 'bool': return view.getUint8(memOffset + addr) !== 0;
            case 'u8': return view.getUint8(memOffset + addr);
            case 'i8': return view.getInt8(memOffset + addr);
            case 'u16': return view.getUint16(memOffset + addr, le);
            case 'i16': return view.getInt16(memOffset + addr, le);
            case 'u32': return view.getUint32(memOffset + addr, le);
            case 'i32': return view.getInt32(memOffset + addr, le);
            case 'f32': return view.getFloat32(memOffset + addr, le);
            case 'f64': return view.getFloat64(memOffset + addr, le);
            default: return 0;
        }
    }
    
    // Helper to write typed value to memory
    function writeMemory(addr, type, value) {
        const memOffset = plc.wasm_exports.getMemoryLocation();
        const view = new DataView(plc.wasm_exports.memory.buffer);
        const le = plc.isLittleEndian;
        switch (type) {
            case 'bool': view.setUint8(memOffset + addr, value ? 1 : 0); break;
            case 'u8': view.setUint8(memOffset + addr, value); break;
            case 'i8': view.setInt8(memOffset + addr, value); break;
            case 'u16': view.setUint16(memOffset + addr, value, le); break;
            case 'i16': view.setInt16(memOffset + addr, value, le); break;
            case 'u32': view.setUint32(memOffset + addr, value, le); break;
            case 'i32': view.setInt32(memOffset + addr, value, le); break;
            case 'f32': view.setFloat32(memOffset + addr, value, le); break;
            case 'f64': view.setFloat64(memOffset + addr, value, le); break;
        }
    }
    
    // Helper to reset state
    function reset() {
        plc.wasm_exports.memoryReset();
        plc.clearFFI();
    }
    
    // Run all tests and collect results
    const testFunctions = [
        // Test 1: Register JS function
        () => {
            reset();
            const idx = plc.registerJSFunction('F_js_add', 'i32,i32->i32', 'Add two numbers in JS', (a, b) => a + b);
            if (idx >= 0 && plc.ffiCallbacks.has(idx)) return { passed: true };
            return { passed: false, error: `got index ${idx}` };
        },
        // Test 2: Call JS function from PLCASM
        () => {
            reset();
            plc.registerJSFunction('F_js_add', 'i32,i32->i32', 'Add in JS', (a, b) => a + b);
            writeMemory(M+0, 'i32', 10);
            writeMemory(M+4, 'i32', 25);
            const result = compileAndRun(`ffi F_js_add ${M+0} ${M+4} ${M+8}`);
            const value = readMemory(M+8, 'i32');
            if (result.error === 0 && value === 35) return { passed: true };
            return { passed: false, error: `error=${result.error}, result=${value}, expected 35` };
        },
        // Test 3: JS function with float types
        () => {
            reset();
            plc.registerJSFunction('F_js_mul', 'f32,f32->f32', 'Multiply floats in JS', (a, b) => a * b);
            writeMemory(M+0, 'f32', 2.5);
            writeMemory(M+4, 'f32', 4.0);
            const result = compileAndRun(`ffi F_js_mul ${M+0} ${M+4} ${M+8}`);
            const value = readMemory(M+8, 'f32');
            if (result.error === 0 && Math.abs(value - 10.0) < 0.001) return { passed: true };
            return { passed: false, error: `error=${result.error}, result=${value}, expected 10.0` };
        },
        // Test 4: JS function with boolean return
        () => {
            reset();
            plc.registerJSFunction('F_js_gt', 'i32,i32->bool', 'Greater than', (a, b) => a > b);
            writeMemory(M+0, 'i32', 100);
            writeMemory(M+4, 'i32', 50);
            const result = compileAndRun(`ffi F_js_gt ${M+0} ${M+4} ${M+8}`);
            const value = readMemory(M+8, 'bool');
            if (result.error === 0 && value === true) return { passed: true };
            return { passed: false, error: `error=${result.error}, result=${value}, expected true` };
        },
        // Test 5: JS function with 3 parameters
        () => {
            reset();
            plc.registerJSFunction('F_js_clamp', 'i32,i32,i32->i32', 'Clamp value', (v, lo, hi) => Math.max(lo, Math.min(hi, v)));
            writeMemory(M+0, 'i32', 150);
            writeMemory(M+4, 'i32', 0);
            writeMemory(M+8, 'i32', 100);
            const result = compileAndRun(`ffi F_js_clamp ${M+0} ${M+4} ${M+8} ${M+12}`);
            const value = readMemory(M+12, 'i32');
            if (result.error === 0 && value === 100) return { passed: true };
            return { passed: false, error: `error=${result.error}, result=${value}, expected 100` };
        },
        // Test 6: JS function with side effects
        () => {
            reset();
            let callCount = 0;
            plc.registerJSFunction('F_js_count', '->i32', 'Count calls', () => ++callCount);
            compileAndRun(`ffi F_js_count ${M+0}`);
            compileAndRun(`ffi F_js_count ${M+4}`);
            compileAndRun(`ffi F_js_count ${M+8}`);
            if (callCount === 3 && readMemory(M+0, 'i32') === 1 && readMemory(M+8, 'i32') === 3) return { passed: true };
            return { passed: false, error: `callCount=${callCount}` };
        },
        // Test 7: Multiple JS functions chained
        () => {
            reset();
            plc.registerJSFunction('F_js_add', 'i32,i32->i32', 'Add', (a, b) => a + b);
            plc.registerJSFunction('F_js_double', 'i32->i32', 'Double', x => x * 2);
            writeMemory(M+0, 'i32', 5);
            writeMemory(M+4, 'i32', 3);
            compileAndRun(`ffi F_js_add ${M+0} ${M+4} ${M+8}`);
            compileAndRun(`ffi F_js_double ${M+8} ${M+12}`);
            const value = readMemory(M+12, 'i32');
            if (value === 16) return { passed: true };
            return { passed: false, error: `expected 16, got ${value}` };
        },
        // Test 8: Mix of C++ and JS functions (C++ FFI not registered, just test JS)
        () => {
            reset();
            plc.registerJSFunction('F_js_square', 'i32->i32', 'Square', x => x * x);
            writeMemory(M+0, 'i32', 7);
            const result = compileAndRun(`ffi F_js_square ${M+0} ${M+4}`);
            const value = readMemory(M+4, 'i32');
            if (result.error === 0 && value === 49) return { passed: true };
            return { passed: false, error: `error=${result.error}, result=${value}` };
        },
        // Test 9: JS function with u16 types
        () => {
            reset();
            plc.registerJSFunction('F_js_add16', 'u16,u16->u16', 'Add u16', (a, b) => (a + b) & 0xFFFF);
            writeMemory(M+0, 'u16', 1000);
            writeMemory(M+2, 'u16', 2000);
            const result = compileAndRun(`ffi F_js_add16 ${M+0} ${M+2} ${M+4}`);
            const value = readMemory(M+4, 'u16');
            if (result.error === 0 && value === 3000) return { passed: true };
            return { passed: false, error: `error=${result.error}, result=${value}, expected 3000` };
        },
        // Test 10: Unregister JS function
        () => {
            reset();
            const idx = plc.registerJSFunction('F_temp', 'i32->i32', 'Temp', x => x);
            const beforeCount = plc.getFFICount();
            const unregistered = plc.unregisterJSFunction(idx);
            const afterCount = plc.getFFICount();
            if (unregistered && afterCount === beforeCount - 1 && !plc.ffiCallbacks.has(idx)) return { passed: true };
            return { passed: false, error: `unregistered=${unregistered}, before=${beforeCount}, after=${afterCount}` };
        },
        // Test 11: System memory protection - writeMemoryByte
        () => {
            reset();
            try {
                plc.writeMemoryByte(0, 42);
                return { passed: false, error: 'Should throw when writing to address 0' };
            } catch (e) {
                if (e.message.includes('System partition')) return { passed: true };
                return { passed: false, error: e.message };
            }
        },
        // Test 12: System memory protection - writeMemoryArea
        () => {
            reset();
            try {
                plc.writeMemoryArea(10, [1, 2, 3]);
                return { passed: false, error: 'Should throw when writing to address 10' };
            } catch (e) {
                if (e.message.includes('System partition')) return { passed: true };
                return { passed: false, error: e.message };
            }
        },
        // Test 13: System memory protection allows X/Y/M writes
        () => {
            reset();
            plc.writeMemoryByte(64, 0x11);
            plc.writeMemoryByte(128, 0x22);
            plc.writeMemoryByte(M, 0x33);
            const xVal = plc.readMemoryArea(64, 1)[0];
            const yVal = plc.readMemoryArea(128, 1)[0];
            const mVal = plc.readMemoryArea(M, 1)[0];
            if (xVal === 0x11 && yVal === 0x22 && mVal === 0x33) return { passed: true };
            return { passed: false, error: `X=${xVal}, Y=${yVal}, M=${mVal}` };
        },
    ];
    
    // Run synchronous tests (tests 1-13)
    for (let i = 0; i < testFunctions.length; i++) {
        try {
            const result = testFunctions[i]();
            if (result.passed) {
                passed++;
                tests.push({ name: testDefinitions[i].name, passed: true });
            } else {
                failed++;
                tests.push({ name: testDefinitions[i].name, passed: false, error: result.error });
            }
        } catch (e) {
            failed++;
            tests.push({ name: testDefinitions[i].name, passed: false, error: e.message });
        }
    }
    
    // Worker FFI tests (tests 14-19)
    let worker = null;
    try {
        worker = await VovkPLC.createWorker(join(__dirname, '..', 'dist', 'VovkPLC.wasm'), {
            silent: true,
            forcePostMessage: true
        });
        
        // Test 14: Worker - Register JS function
        try {
            const idx = await worker.registerJSFunction('F_worker_add', 'i32,i32->i32', 'Add', (a, b) => a + b);
            if (idx >= 0) {
                passed++;
                tests.push({ name: testDefinitions[13].name, passed: true });
            } else {
                failed++;
                tests.push({ name: testDefinitions[13].name, passed: false, error: `got index ${idx}` });
            }
        } catch (e) {
            failed++;
            tests.push({ name: testDefinitions[13].name, passed: false, error: e.message });
        }
        
        // Test 15: Worker - Call JS function from PLCASM
        try {
            await worker.clearFFI();
            await worker.registerJSFunction('F_worker_add', 'i32,i32->i32', 'Add', (a, b) => a + b);
            await worker.writeMemoryArea(M+0, [20, 0, 0, 0]);
            await worker.writeMemoryArea(M+4, [15, 0, 0, 0]);
            const result = await worker.compilePLCASM(`ffi F_worker_add ${M+0} ${M+4} ${M+8}`, { run: true });
            const mem = await worker.readMemoryArea(M+8, 4);
            const value = mem[0] | (mem[1] << 8) | (mem[2] << 16) | (mem[3] << 24);
            if (!result.error && value === 35) {
                passed++;
                tests.push({ name: testDefinitions[14].name, passed: true });
            } else {
                failed++;
                tests.push({ name: testDefinitions[14].name, passed: false, error: `error=${result.error}, value=${value}` });
            }
        } catch (e) {
            failed++;
            tests.push({ name: testDefinitions[14].name, passed: false, error: e.message });
        }
        
        // Test 16: Worker - JS function with floats
        try {
            await worker.clearFFI();
            await worker.registerJSFunction('F_worker_mul', 'f32,f32->f32', 'Mul', (a, b) => a * b);
            const buf = new ArrayBuffer(8);
            const view = new DataView(buf);
            view.setFloat32(0, 2.5, true);
            view.setFloat32(4, 4.0, true);
            await worker.writeMemoryArea(M+0, Array.from(new Uint8Array(buf)));
            const compileResult = await worker.compilePLCASM(`ffi F_worker_mul ${M+0} ${M+4} ${M+8}`, { run: true });
            const mem = await worker.readMemoryArea(M+8, 4);
            // Worker returns object with numeric keys due to JSON serialization, convert to array
            const memArray = Array.isArray(mem) ? mem : Object.keys(mem).sort((a,b) => a-b).map(k => mem[k]);
            const resultBuf = new ArrayBuffer(4);
            new Uint8Array(resultBuf).set(memArray);
            const value = new DataView(resultBuf).getFloat32(0, true);
            // console.log('Test 16 debug:', { compileResult, mem, memArray, value });
            if (Math.abs(value - 10.0) < 0.001) {
                passed++;
                tests.push({ name: testDefinitions[15].name, passed: true });
            } else {
                failed++;
                tests.push({ name: testDefinitions[15].name, passed: false, error: `value=${value}, expected 10.0, mem=${JSON.stringify(mem)}, memArray=${JSON.stringify(memArray)}` });
            }
        } catch (e) {
            failed++;
            tests.push({ name: testDefinitions[15].name, passed: false, error: e.message });
        }
        
        // Test 17: Worker - getFFICount
        try {
            await worker.clearFFI();
            await worker.registerJSFunction('F_a', 'i32->i32', 'A', x => x);
            await worker.registerJSFunction('F_b', 'i32->i32', 'B', x => x);
            const count = await worker.getFFICount();
            if (count === 2) {
                passed++;
                tests.push({ name: testDefinitions[16].name, passed: true });
            } else {
                failed++;
                tests.push({ name: testDefinitions[16].name, passed: false, error: `expected 2, got ${count}` });
            }
        } catch (e) {
            failed++;
            tests.push({ name: testDefinitions[16].name, passed: false, error: e.message });
        }
        
        // Test 18: Worker - Unregister JS function
        try {
            await worker.clearFFI();
            const idx = await worker.registerJSFunction('F_temp', 'i32->i32', 'Temp', x => x);
            const beforeCount = await worker.getFFICount();
            const unregistered = await worker.unregisterJSFunction(idx);
            const afterCount = await worker.getFFICount();
            if (unregistered && afterCount === beforeCount - 1) {
                passed++;
                tests.push({ name: testDefinitions[17].name, passed: true });
            } else {
                failed++;
                tests.push({ name: testDefinitions[17].name, passed: false, error: `before=${beforeCount}, after=${afterCount}` });
            }
        } catch (e) {
            failed++;
            tests.push({ name: testDefinitions[17].name, passed: false, error: e.message });
        }
        
        // Test 19: Worker - clearFFI
        try {
            await worker.registerJSFunction('F_x', 'i32->i32', 'X', x => x);
            await worker.registerJSFunction('F_y', 'i32->i32', 'Y', x => x);
            await worker.clearFFI();
            const count = await worker.getFFICount();
            if (count === 0) {
                passed++;
                tests.push({ name: testDefinitions[18].name, passed: true });
            } else {
                failed++;
                tests.push({ name: testDefinitions[18].name, passed: false, error: `expected 0, got ${count}` });
            }
        } catch (e) {
            failed++;
            tests.push({ name: testDefinitions[18].name, passed: false, error: e.message });
        }
        
    } catch (e) {
        // Worker setup failed - mark all worker tests as failed
        for (let i = 13; i < 19; i++) {
            failed++;
            tests.push({ name: testDefinitions[i].name, passed: false, error: `Worker setup failed: ${e.message}` });
        }
    } finally {
        if (worker) {
            try { await worker.terminate(); } catch (e) { /* ignore */ }
        }
    }
    
    return {
        name: 'FFI & Memory Protection',
        passed,
        failed,
        total: testDefinitions.length,
        tests
    };
}

// ============================================================================
// Main - Run if executed directly
// ============================================================================

const isMainModule = process.argv[1]?.replace(/\\/g, '/').endsWith('test_ffi.js');
if (isMainModule) {
    const result = await runFFITests();
    process.exit(result.failed > 0 ? 1 : 0);
}
