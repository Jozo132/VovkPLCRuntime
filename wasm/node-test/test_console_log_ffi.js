/**
 * Test console_log FFI implemented in JavaScript
 * This demonstrates how to define string-handling FFI functions in JS
 */

import VovkPLC from '../dist/VovkPLC.js';
import { join, dirname } from 'path';
import { fileURLToPath } from 'url';

const __dirname = dirname(fileURLToPath(import.meta.url));
const wasmPath = join(__dirname, '..', 'dist', 'VovkPLC.wasm');

const plc = new VovkPLC();
await plc.initialize(wasmPath, false, true);  // silent mode for clean output

// Clear any existing FFIs and register our JS-based console_log
plc.clearFFI();

// Register console_log_str8 as a JS FFI function
// Signature: "u16->void" - takes the address of a str8 string
// The JS callback receives the u16 value directly (the FFI system reads it from memory)
plc.registerFFI('console_log_str8', 'u16->void', 'Output str8 to console', (strAddr) => {
    // strAddr is the u16 address value (already read from memory)
    // str8 layout: [u8 capacity, u8 length, data...]
    const mem = plc.readMemoryArea(strAddr, 68);
    const len = mem[1];
    const data = mem.slice(2, 2 + len);
    const str = String.fromCharCode(...data);
    console.log(str);
    // void return - no return needed
});

const M = 4000;  // Marker area
const STR = M;           // String at M+0 (capacity 64)
const STR2 = M + 68;     // Second string at M+68
const PARAM = M + 140;   // FFI param address
const RET = M + 142;     // FFI return address (unused for void)

function initStr8(addr, capacity = 64) {
    // str8 layout: [u8 capacity][u8 length][data...]
    plc.writeMemoryByte(addr, capacity);
    plc.writeMemoryByte(addr + 1, 0);
}

function setParam(addr) {
    // Write address as u16 to PARAM location
    plc.writeMemoryByte(PARAM, addr & 0xFF);
    plc.writeMemoryByte(PARAM + 1, (addr >> 8) & 0xFF);
}

function compileAndRun(source) {
    plc.downloadAssembly(source);
    if (plc.wasm_exports.compileAssembly()) return { error: 'compile' };
    if (plc.wasm_exports.loadCompiledProgram()) return { error: 'load' };
    plc.setSilent(false);  // enable output for run
    const result = plc.wasm_exports.run();
    plc.setSilent(true);   // back to silent
    return { error: result };
}

function readStr8(addr) {
    const mem = plc.readMemoryArea(addr, 68);
    const len = mem[1];
    return String.fromCharCode(...mem.slice(2, 2 + len));
}

function reset() {
    plc.wasm_exports.memoryReset();
    initStr8(STR, 64);
    initStr8(STR2, 64);
    setParam(STR);
}

console.log('Testing console_log FFI with various string operations\n');

// Test 1: Simple string literal
reset();
console.log('Test 1: cstr.lit with simple string');
compileAndRun(`
    cstr.lit ${STR} "Hello World!"
    ffi console_log_str8 ${PARAM} ${RET}
`);
console.log(`Memory: "${readStr8(STR)}"\n`);

// Test 2: String with escape sequences  
reset();
console.log('Test 2: cstr.lit with escape sequences');
compileAndRun(`
    cstr.lit ${STR} "Line1\\nLine2\\tTabbed"
    ffi console_log_str8 ${PARAM} ${RET}
`);
console.log(`Memory: "${readStr8(STR)}"\n`);

// Test 3: String concatenation
reset();
console.log('Test 3: cstr.lit + cstr.cat concatenation');
compileAndRun(`
    cstr.lit ${STR} "Hello "
    cstr.cat ${STR} "World!"
    ffi console_log_str8 ${PARAM} ${RET}
`);
console.log(`Memory: "${readStr8(STR)}"\n`);

// Test 4: Multiple concatenations
reset();
console.log('Test 4: Multiple cstr.cat operations');
compileAndRun(`
    cstr.lit ${STR} "A"
    cstr.cat ${STR} "B"
    cstr.cat ${STR} "C"
    cstr.cat ${STR} "D"
    cstr.cat ${STR} "E"
    ffi console_log_str8 ${PARAM} ${RET}
`);
console.log(`Memory: "${readStr8(STR)}"\n`);

// Test 5: Clear then concatenate
reset();
console.log('Test 5: str.clear then concatenate');
compileAndRun(`
    cstr.lit ${STR} "Initial"
    str.clear ${STR}
    cstr.cat ${STR} "Fresh Start"
    ffi console_log_str8 ${PARAM} ${RET}
`);
console.log(`Memory: "${readStr8(STR)}"\n`);

// Test 6: Copy between strings
reset();
setParam(STR2);
console.log('Test 6: str.copy between two strings');
compileAndRun(`
    cstr.lit ${STR} "Source String"
    cstr.lit ${STR2} "Will be overwritten"
    str.copy ${STR2} ${STR}
    ffi console_log_str8 ${PARAM} ${RET}
`);
console.log(`STR1: "${readStr8(STR)}", STR2: "${readStr8(STR2)}"\n`);

// Test 7: Long string
reset();
console.log('Test 7: Long string near capacity');
compileAndRun(`
    cstr.lit ${STR} "This is a longer string testing capacity"
    ffi console_log_str8 ${PARAM} ${RET}
`);
const len = plc.readMemoryArea(STR + 1, 1)[0];
console.log(`Memory (len=${len}): "${readStr8(STR)}"\n`);

// Test 8: Special characters
reset();
console.log('Test 8: Special characters');
compileAndRun(`
    cstr.lit ${STR} "Symbols: !@#$%^&*()[]{}|"
    ffi console_log_str8 ${PARAM} ${RET}
`);
console.log(`Memory: "${readStr8(STR)}"\n`);

// Test 9: Build string with char (push char to stack, then str.char pops and appends)
reset();
console.log('Test 9: Build string char by char');
compileAndRun(`
    str.clear ${STR}
    u8 72
    str.char ${STR}
    u8 105
    str.char ${STR}
    u8 33
    str.char ${STR}
    ffi console_log_str8 ${PARAM} ${RET}
`);
console.log(`Memory: "${readStr8(STR)}"\n`);

console.log('✓ All console_log tests completed!');
