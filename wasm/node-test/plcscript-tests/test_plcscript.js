// test_plcscript.js - PLCScript Compiler Unit Tests
// Tests the TypeScript/ES7 subset language that compiles to PLCASM

import { fileURLToPath } from 'url';
import { pathToFileURL } from 'url';
import path from 'path';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Load VovkPLC using proper file URL
const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm');
const VovkPLCPath = path.resolve(__dirname, '../../dist/VovkPLC.js');
const VovkPLC = (await import(pathToFileURL(VovkPLCPath).href)).default;

console.log('╔══════════════════════════════════════════════════════════════════╗');
console.log('║              PLCScript Compiler Unit Tests                       ║');
console.log('╚══════════════════════════════════════════════════════════════════╝');
console.log();

// Create PLC instance
const plc = await VovkPLC.createWorker(wasmPath);

let passed = 0;
let failed = 0;

async function test(name, fn) {
    try {
        await fn();
        console.log(`✓ ${name}`);
        passed++;
    } catch (e) {
        console.log(`✗ ${name}`);
        console.log(`  Error: ${e.message}`);
        failed++;
    }
}

function assert(condition, message) {
    if (!condition) {
        throw new Error(message || 'Assertion failed');
    }
}

// Test 1: Simple variable declaration and assignment
await test('Simple variable declaration with address', async () => {
    const script = `
// Simple variable with PLC address
let counter: i16 @ MW10 = 0;
counter = counter + 1;
`;
    
    const project = `
VOVKPLCPROJECT Test1
VERSION 1.0.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    S 64
    M 256
    T 16
    C 8
END_MEMORY

FILE main
    BLOCK LANG=PLCSCRIPT Main
${script}
    END_BLOCK
END_FILE
`;
    
    await plc.downloadAssembly(project);
    const error = await plc.callExport('compileProject', false);
    
    if (error) {
        const msg = await plc.readStream();
        throw new Error(`Compilation failed: ${msg}`);
    }
    
    // Get generated PLCASM
    const combined = await plc.callExport('getProjectCombinedPLCASM');
    const plcasm = await plc.readStream();
    
    assert(plcasm.includes('i16.const 0'), 'Should generate constant push');
    assert(plcasm.includes('i16.store MW10') || plcasm.includes('MW10'), 'Should reference MW10 address');
});

// Test 2: If statement
await test('If statement compilation', async () => {
    const script = `
let flag: bool @ M0.0;
let output: bool @ Y0.0;

if (flag) {
    output = true;
}
`;
    
    const project = `
VOVKPLCPROJECT Test2
VERSION 1.0.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    S 64
    M 256
    T 16
    C 8
END_MEMORY

FILE main
    BLOCK LANG=PLCSCRIPT Main
${script}
    END_BLOCK
END_FILE
`;
    
    await plc.downloadAssembly(project);
    const error = await plc.callExport('compileProject', false);
    
    if (error) {
        const msg = await plc.readStream();
        throw new Error(`Compilation failed: ${msg}`);
    }
    
    const combined = await plc.callExport('getProjectCombinedPLCASM');
    const plcasm = await plc.readStream();
    
    assert(plcasm.includes('jmp.if.not'), 'Should generate conditional jump');
});

// Test 3: While loop
await test('While loop compilation', async () => {
    const script = `
let i: i16 @ MW20 = 0;
while (i < 10) {
    i = i + 1;
}
`;
    
    const project = `
VOVKPLCPROJECT Test3
VERSION 1.0.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    S 64
    M 256
    T 16
    C 8
END_MEMORY

FILE main
    BLOCK LANG=PLCSCRIPT Main
${script}
    END_BLOCK
END_FILE
`;
    
    await plc.downloadAssembly(project);
    const error = await plc.callExport('compileProject', false);
    
    if (error) {
        const msg = await plc.readStream();
        throw new Error(`Compilation failed: ${msg}`);
    }
    
    const combined = await plc.callExport('getProjectCombinedPLCASM');
    const plcasm = await plc.readStream();
    
    assert(plcasm.includes('__while_'), 'Should generate while label');
    assert(plcasm.includes('jmp'), 'Should generate jump instruction');
});

// Test 4: Arithmetic expressions
await test('Arithmetic expressions', async () => {
    const script = `
let a: i32 @ MD0 = 10;
let b: i32 @ MD4 = 20;
let result: i32 @ MD8;

result = (a + b) * 2 - 5;
`;
    
    const project = `
VOVKPLCPROJECT Test4
VERSION 1.0.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    S 64
    M 256
    T 16
    C 8
END_MEMORY

FILE main
    BLOCK LANG=PLCSCRIPT Main
${script}
    END_BLOCK
END_FILE
`;
    
    await plc.downloadAssembly(project);
    const error = await plc.callExport('compileProject', false);
    
    if (error) {
        const msg = await plc.readStream();
        throw new Error(`Compilation failed: ${msg}`);
    }
    
    const combined = await plc.callExport('getProjectCombinedPLCASM');
    const plcasm = await plc.readStream();
    
    assert(plcasm.includes('.add'), 'Should generate add instruction');
    assert(plcasm.includes('.mul'), 'Should generate multiply instruction');
    assert(plcasm.includes('.sub'), 'Should generate subtract instruction');
});

// Test 5: Comparison operators
await test('Comparison operators', async () => {
    const script = `
let x: i16 @ MW30 = 50;
let isGreater: bool @ M1.0;
let isEqual: bool @ M1.1;
let isLess: bool @ M1.2;

isGreater = x > 25;
isEqual = x == 50;
isLess = x < 100;
`;
    
    const project = `
VOVKPLCPROJECT Test5
VERSION 1.0.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    S 64
    M 256
    T 16
    C 8
END_MEMORY

FILE main
    BLOCK LANG=PLCSCRIPT Main
${script}
    END_BLOCK
END_FILE
`;
    
    await plc.downloadAssembly(project);
    const error = await plc.callExport('compileProject', false);
    
    if (error) {
        const msg = await plc.readStream();
        throw new Error(`Compilation failed: ${msg}`);
    }
    
    const combined = await plc.callExport('getProjectCombinedPLCASM');
    const plcasm = await plc.readStream();
    
    assert(plcasm.includes('.gt'), 'Should generate greater-than comparison');
    assert(plcasm.includes('.eq'), 'Should generate equality comparison');
    assert(plcasm.includes('.lt'), 'Should generate less-than comparison');
});

// Test 6: Logical operators
await test('Logical operators', async () => {
    const script = `
let a: bool @ M2.0 = true;
let b: bool @ M2.1 = false;
let andResult: bool @ M2.2;
let orResult: bool @ M2.3;
let notResult: bool @ M2.4;

andResult = a && b;
orResult = a || b;
notResult = !a;
`;
    
    const project = `
VOVKPLCPROJECT Test6
VERSION 1.0.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    S 64
    M 256
    T 16
    C 8
END_MEMORY

FILE main
    BLOCK LANG=PLCSCRIPT Main
${script}
    END_BLOCK
END_FILE
`;
    
    await plc.downloadAssembly(project);
    const error = await plc.callExport('compileProject', false);
    
    if (error) {
        const msg = await plc.readStream();
        throw new Error(`Compilation failed: ${msg}`);
    }
    
    const combined = await plc.callExport('getProjectCombinedPLCASM');
    const plcasm = await plc.readStream();
    
    assert(plcasm.includes('logic.and'), 'Should generate logical AND');
    assert(plcasm.includes('logic.or'), 'Should generate logical OR');
    assert(plcasm.includes('logic.not'), 'Should generate logical NOT');
});

// Test 7: TON timer function
await test('TON timer built-in function', async () => {
    const script = `
let startBtn: bool @ X0.0;
let motorOn: bool @ Y0.0;

// Start motor after 2 second delay
motorOn = TON(T0, startBtn, 2000);
`;
    
    const project = `
VOVKPLCPROJECT Test7
VERSION 1.0.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    S 64
    M 256
    T 16
    C 8
END_MEMORY

FILE main
    BLOCK LANG=PLCSCRIPT Main
${script}
    END_BLOCK
END_FILE
`;
    
    await plc.downloadAssembly(project);
    const error = await plc.callExport('compileProject', false);
    
    if (error) {
        const msg = await plc.readStream();
        throw new Error(`Compilation failed: ${msg}`);
    }
    
    const combined = await plc.callExport('getProjectCombinedPLCASM');
    const plcasm = await plc.readStream();
    
    assert(plcasm.includes('ton.const T0'), 'Should generate TON timer instruction');
});

// Test 8: Increment/decrement operators
await test('Increment and decrement operators', async () => {
    const script = `
let count: i16 @ MW40 = 0;

count++;
count++;
count--;
`;
    
    const project = `
VOVKPLCPROJECT Test8
VERSION 1.0.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    S 64
    M 256
    T 16
    C 8
END_MEMORY

FILE main
    BLOCK LANG=PLCSCRIPT Main
${script}
    END_BLOCK
END_FILE
`;
    
    await plc.downloadAssembly(project);
    const error = await plc.callExport('compileProject', false);
    
    if (error) {
        const msg = await plc.readStream();
        throw new Error(`Compilation failed: ${msg}`);
    }
    
    const combined = await plc.callExport('getProjectCombinedPLCASM');
    const plcasm = await plc.readStream();
    
    // Should see add and sub operations for increment/decrement
    assert(plcasm.includes('.add'), 'Should generate add for increment');
    assert(plcasm.includes('.sub'), 'Should generate sub for decrement');
});

// Test 9: For loop
await test('For loop compilation', async () => {
    const script = `
let sum: i16 @ MW50 = 0;

for (let i: i16 @ MW52 = 0; i < 5; i++) {
    sum = sum + i;
}
`;
    
    const project = `
VOVKPLCPROJECT Test9
VERSION 1.0.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    S 64
    M 256
    T 16
    C 8
END_MEMORY

FILE main
    BLOCK LANG=PLCSCRIPT Main
${script}
    END_BLOCK
END_FILE
`;
    
    await plc.downloadAssembly(project);
    const error = await plc.callExport('compileProject', false);
    
    if (error) {
        const msg = await plc.readStream();
        throw new Error(`Compilation failed: ${msg}`);
    }
    
    const combined = await plc.callExport('getProjectCombinedPLCASM');
    const plcasm = await plc.readStream();
    
    assert(plcasm.includes('__forcond_'), 'Should generate for condition label');
    assert(plcasm.includes('__endfor_'), 'Should generate endfor label');
});

// Test 10: Direct PLC address access
await test('Direct PLC address access in expressions', async () => {
    const script = `
let result: bool @ Y0.0;

// Direct access to input bit
result = X0.0 && X0.1;
`;
    
    const project = `
VOVKPLCPROJECT Test10
VERSION 1.0.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    S 64
    M 256
    T 16
    C 8
END_MEMORY

FILE main
    BLOCK LANG=PLCSCRIPT Main
${script}
    END_BLOCK
END_FILE
`;
    
    await plc.downloadAssembly(project);
    const error = await plc.callExport('compileProject', false);
    
    if (error) {
        const msg = await plc.readStream();
        throw new Error(`Compilation failed: ${msg}`);
    }
    
    const combined = await plc.callExport('getProjectCombinedPLCASM');
    const plcasm = await plc.readStream();
    
    assert(plcasm.includes('u8.readBit X0.0'), 'Should read X0.0 directly');
    assert(plcasm.includes('u8.readBit X0.1'), 'Should read X0.1 directly');
});

// Test 11: Comments
await test('Single-line and multi-line comments', async () => {
    const script = `
// This is a single-line comment
let x: i16 @ MW60 = 10;

/* This is a
   multi-line comment */
let y: i16 @ MW62 = 20;

x = y; // inline comment
`;
    
    const project = `
VOVKPLCPROJECT Test11
VERSION 1.0.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    S 64
    M 256
    T 16
    C 8
END_MEMORY

FILE main
    BLOCK LANG=PLCSCRIPT Main
${script}
    END_BLOCK
END_FILE
`;
    
    await plc.downloadAssembly(project);
    const error = await plc.callExport('compileProject', false);
    
    if (error) {
        const msg = await plc.readStream();
        throw new Error(`Compilation failed: ${msg}`);
    }
    
    // Should compile without errors - comments are ignored
});

// Test 12: Float arithmetic
await test('Float type arithmetic', async () => {
    const script = `
let temp: f32 @ MD100 = 25.5;
let offset: f32 @ MD104 = 1.5;
let result: f32 @ MD108;

result = temp + offset;
`;
    
    const project = `
VOVKPLCPROJECT Test12
VERSION 1.0.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    S 64
    M 256
    T 16
    C 8
END_MEMORY

FILE main
    BLOCK LANG=PLCSCRIPT Main
${script}
    END_BLOCK
END_FILE
`;
    
    await plc.downloadAssembly(project);
    const error = await plc.callExport('compileProject', false);
    
    if (error) {
        const msg = await plc.readStream();
        throw new Error(`Compilation failed: ${msg}`);
    }
    
    const combined = await plc.callExport('getProjectCombinedPLCASM');
    const plcasm = await plc.readStream();
    
    assert(plcasm.includes('f32.'), 'Should use f32 type operations');
});

// Summary
console.log();
console.log('────────────────────────────────────────────────────────────────────');
console.log();
console.log(`Results: ${passed}/${passed + failed} tests passed`);

await plc.terminate();

process.exit(failed > 0 ? 1 : 0);
