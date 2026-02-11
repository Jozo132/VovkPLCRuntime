// Test file for PLCScript struct support
// Tests PLCScript through the project compiler since that's how PLCScript blocks are exposed
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
 * Run PLCScript Struct unit tests
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
    let passed = 0;
    let failed = 0;

    /**
     * Test helper that compiles PLCScript code through the project compiler
     */
    function test(name, code, expectedSubstrings = [], shouldFail = false, expectedError = '') {
        const project = `VOVKPLCPROJECT StructTest
VERSION 1.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    M 512
    T 16
    C 8
END_MEMORY

FLASH
    SIZE 32768
END_FLASH

SYMBOLS
END_SYMBOLS

PROGRAM main
    BLOCK LANG=PLCSCRIPT Main
${code}
    END_BLOCK
END_PROGRAM
`;
        
        const result = plc.compileProject(project);
        
        let plcasm = '';
        if (plc.wasm_exports && plc.wasm_exports.project_getCombinedPLCASM) {
            const ptr = plc.wasm_exports.project_getCombinedPLCASM();
            if (ptr) {
                const memory = new Uint8Array(plc.wasm_exports.memory.buffer);
                let i = ptr;
                while (memory[i] !== 0 && i < memory.length && plcasm.length < 10000) {
                    plcasm += String.fromCharCode(memory[i]);
                    i++;
                }
            }
        }
        
        const hasError = result.problem !== null;
        const error = hasError ? result.problem.message : '';
        
        let testPassed = true;
        let errors = [];
        
        if (shouldFail) {
            if (!hasError) {
                errors.push('Expected compilation to fail but it succeeded');
                testPassed = false;
            } else if (expectedError && !error.includes(expectedError)) {
                errors.push(`Expected error to contain: "${expectedError}"`);
                errors.push(`Got: "${error}"`);
                testPassed = false;
            }
        } else {
            if (hasError) {
                errors.push(`Compilation error: ${error}`);
                testPassed = false;
            } else {
                for (const sub of expectedSubstrings) {
                    if (!plcasm.includes(sub)) {
                        errors.push(`Expected to find: "${sub}"`);
                        testPassed = false;
                    }
                }
            }
        }
        
        if (testPassed) {
            passed++;
            testResults.push({ name, passed: true });
        } else {
            failed++;
            testResults.push({ name, passed: false, error: errors[0] });
            failures.push({ name, errors });
        }
    }

    /**
     * Test helper for mixed PLCScript+PLCASM projects
     */
    function testMixed(name, blocks, expectedAddresses = [], shouldFail = false, expectedError = '') {
        let blocksStr = '';
        for (let i = 0; i < blocks.length; i++) {
            const block = blocks[i];
            blocksStr += `    BLOCK LANG=${block.lang} Block${i + 1}
${block.code}
    END_BLOCK
`;
        }
        
        const project = `VOVKPLCPROJECT StructTest
VERSION 1.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    M 512
    T 16
    C 8
END_MEMORY

FLASH
    SIZE 32768
END_FLASH

SYMBOLS
END_SYMBOLS

PROGRAM main
${blocksStr}END_PROGRAM
`;
        
        const result = plc.compileProject(project);
        
        const hasError = result.problem !== null;
        const error = hasError ? result.problem.message : '';
        
        let testPassed = true;
        let errors = [];
        
        if (shouldFail) {
            if (!hasError) {
                errors.push('Expected compilation to fail but it succeeded');
                testPassed = false;
            } else if (expectedError && !error.includes(expectedError)) {
                errors.push(`Expected error to contain: "${expectedError}"`);
                errors.push(`Got: "${error}"`);
                testPassed = false;
            }
        } else {
            if (hasError) {
                errors.push(`Compilation error: ${error}`);
                testPassed = false;
            } else if (expectedAddresses.length > 0) {
                const bytecode = result.bytecode;
                if (typeof bytecode === 'string') {
                    const bytes = bytecode.split(' ').filter(s => s.length > 0).map(b => parseInt(b, 16));
                    const foundAddresses = [];
                    
                    for (let i = 0; i < bytes.length - 3; i++) {
                        if ((bytes[i] === 0x18 || bytes[i] === 0x19) && 
                            (bytes[i+1] >= 0x06 && bytes[i+1] <= 0x0B)) {
                            // Native endianness (little-endian): low byte first
                            const addr = bytes[i+2] + bytes[i+3] * 256;
                            foundAddresses.push(addr);
                        }
                    }
                    
                    for (const expected of expectedAddresses) {
                        if (!foundAddresses.includes(expected)) {
                            errors.push(`Expected address ${expected} in bytecode, found: [${foundAddresses.join(', ')}]`);
                            testPassed = false;
                        }
                    }
                }
            }
        }
        
        if (testPassed) {
            passed++;
            testResults.push({ name, passed: true });
        } else {
            failed++;
            testResults.push({ name, passed: false, error: errors[0] });
            failures.push({ name, errors });
        }
    }

    // =========================================================================
    // PLCScript Struct Tests (Tests 1-15)
    // =========================================================================

    test('Basic struct declaration', `
type MotorData = struct {
    speed: i32;
    enabled: bool;
};
let motor: MotorData @ M100;
`, ['M100']);

    test('Struct field read (i32)', `
type MotorData = struct {
    speed: i32;
    enabled: bool;
};
let motor: MotorData @ M100;
let x: i32 @ MD200;
x = motor.speed;
`, ['i32.load_from M100', 'i32.move_to M200']);

    test('Struct field write (i32)', `
type MotorData = struct {
    speed: i32;
    enabled: bool;
};
let motor: MotorData @ M100;
motor.speed = 1500;
`, ['i32.move_to M100']);

    test('Bool field at offset', `
type MotorData = struct {
    speed: i32;
    enabled: bool;
};
let motor: MotorData @ M100;
motor.enabled = true;
`, ['M104']);

    test('Multiple struct instances', `
type Point = struct {
    x: i16;
    y: i16;
};
let p1: Point @ M0;
let p2: Point @ M10;
p1.x = 100;
p2.x = 200;
`, ['M0', 'M10']);

    test('Struct with various types', `
type MixedData = struct {
    byteVal: u8;
    wordVal: i16;
    dwordVal: i32;
    flag: bool;
};
let data: MixedData @ M50;
data.byteVal = 255;
data.wordVal = 1000;
data.dwordVal = 100000;
`, ['M50', 'M51', 'M53']);

    test('Read bool field', `
type Flags = struct {
    active: bool;
    error: bool;
};
let flags: Flags @ M0;
let result: bool @ M10.0;
result = flags.active;
`, ['u8.readBit M0', 'M10.0']);

    test('Compound assignment (+=)', `
type Counter = struct {
    count: i32;
};
let ctr: Counter @ M100;
ctr.count += 10;
`, ['i32.load_from M100', 'i32.add', 'i32.move_to M100']);

    test('Struct field in expression', `
type Point = struct {
    x: i32;
    y: i32;
};
let p: Point @ M0;
let sum: i32 @ MD100;
sum = p.x + p.y;
`, ['i32.load_from M0', 'i32.load_from M4', 'i32.add']);

    test('Error on unknown field', `
type Data = struct {
    value: i32;
};
let d: Data @ M0;
d.unknown = 123;
`, [], true, 'Unknown struct field');

    test('Error on unknown struct type', `
let d: UnknownType @ M0;
`, [], true, 'Unknown type');

    test('Post-increment on struct field', `
type Counter = struct {
    value: i32;
};
let c: Counter @ M0;
c.value++;
`, ['i32.load_from M0', 'i32.add', 'i32.move_to M0']);

    test('Second field at correct offset', `
type Pair = struct {
    first: i16;
    second: i16;
};
let p: Pair @ M0;
p.second = 42;
`, ['i16.move_to M2']);

    test('Third field with mixed sizes', `
type Mixed = struct {
    a: u8;
    b: i32;
    c: i16;
};
let m: Mixed @ M10;
m.c = 1000;
`, ['i16.move_to M15']);

    test('Read and write different fields', `
type Data = struct {
    input: i32;
    output: i32;
};
let d: Data @ M0;
d.output = d.input + 1;
`, ['i32.load_from M0', 'i32.move_to M4']);

    // =========================================================================
    // PLCASM Struct Property Access Tests (Tests 16-20)
    // =========================================================================
    // Memory layout: S: 0-63, X: 64-127, Y: 128-191, M: 192+
    // So M100 = 192 + 100 = 292, M200 = 392

    testMixed('PLCASM uses struct from PLCScript', [
        { lang: 'PLCSCRIPT', code: `
type Motor = struct {
    speed: i32;
    enabled: bool;
};
let motor: Motor @ M100;
` },
        { lang: 'PLCASM', code: `
i32.load_from motor.speed
i32.move_to M200
` }
    ], [292, 392]);

    testMixed('PLCASM multi-field access', [
        { lang: 'PLCSCRIPT', code: `
type Position = struct {
    x: i32;
    y: i32;
    z: i32;
};
let pos: Position @ M0;
` },
        { lang: 'PLCASM', code: `
i32.load_from pos.x
i32.load_from pos.y
i32.load_from pos.z
` }
    ], [192, 196, 200]);

    testMixed('PLCASM write to struct field', [
        { lang: 'PLCSCRIPT', code: `
type Counter = struct {
    value: i32;
};
let cnt: Counter @ M50;
` },
        { lang: 'PLCASM', code: `
i32.const 999
i32.move_to cnt.value
` }
    ], [242]);

    testMixed('PLCASM error on unknown field', [
        { lang: 'PLCSCRIPT', code: `
type Data = struct {
    value: i32;
};
let d: Data @ M0;
` },
        { lang: 'PLCASM', code: `
i32.load_from d.nonexistent
` }
    ], [], true, '');

    testMixed('PLCASM multiple struct instances', [
        { lang: 'PLCSCRIPT', code: `
type Sensor = struct {
    reading: i32;
};
let sensor1: Sensor @ M0;
let sensor2: Sensor @ M10;
` },
        { lang: 'PLCASM', code: `
i32.load_from sensor1.reading
i32.load_from sensor2.reading
` }
    ], [192, 202]);

    // ========================================================================
    // Cross-Language Struct Tests - ST (Structured Text)
    // ========================================================================

    testMixed('ST uses struct from PLCScript', [
        { lang: 'PLCSCRIPT', code: `
type Motor = struct {
    speed: i32;
    enabled: bool;
};
let motor: Motor @ M100;
` },
        { lang: 'ST', code: `motor.speed := 1500;` }
    ], []);

    testMixed('ST writes to struct field', [
        { lang: 'PLCSCRIPT', code: `
type Sensor = struct {
    value: i32;
    status: bool;
};
let sensor: Sensor @ M50;
` },
        { lang: 'ST', code: `sensor.value := 1000;` }
    ], []);

    testMixed('ST reads struct bool field', [
        { lang: 'PLCSCRIPT', code: `
type Flags = struct {
    active: bool;
    error: bool;
};
let flags: Flags @ M0;
` },
        { lang: 'ST', code: `flags.active := TRUE;` }
    ], []);

    // ========================================================================
    // Cross-Language Struct Tests - STL (Statement List)
    // ========================================================================

    testMixed('STL uses struct from PLCScript', [
        { lang: 'PLCSCRIPT', code: `
type Counter = struct {
    value: i32;
    preset: i32;
};
let cnt: Counter @ M100;
` },
        { lang: 'STL', code: `L cnt.value
T MD200` }
    ], []);

    testMixed('STL writes to struct field', [
        { lang: 'PLCSCRIPT', code: `
type Data = struct {
    input: i32;
    output: i32;
};
let data: Data @ M50;
` },
        { lang: 'STL', code: `L 999
T data.output` }
    ], []);

    testMixed('STL reads struct bool field', [
        { lang: 'PLCSCRIPT', code: `
type Status = struct {
    running: bool;
    fault: bool;
};
let status: Status @ M0;
` },
        { lang: 'STL', code: `A status.running
= M10.0` }
    ], []);

    // ========================================================================
    // Cross-Language Struct Tests - LADDER
    // ========================================================================

    testMixed('LADDER uses struct contact', [
        { lang: 'PLCSCRIPT', code: `
type Button = struct {
    pressed: bool;
    latched: bool;
};
let btn: Button @ M0;
` },
        { lang: 'LADDER', code: `{
    "nodes": [
        {"id": "1", "type": "contact", "x": 0, "y": 0, "symbol": "btn.pressed"},
        {"id": "2", "type": "coil", "x": 1, "y": 0, "symbol": "M10.0"}
    ],
    "connections": [
        {"id": "c1", "sources": ["1"], "destinations": ["2"]}
    ]
}` }
    ], []);

    testMixed('LADDER uses struct coil', [
        { lang: 'PLCSCRIPT', code: `
type Output = struct {
    lamp: bool;
    alarm: bool;
};
let out: Output @ M100;
` },
        { lang: 'LADDER', code: `{
    "nodes": [
        {"id": "1", "type": "contact", "x": 0, "y": 0, "symbol": "M0.0"},
        {"id": "2", "type": "coil", "x": 1, "y": 0, "symbol": "out.lamp"}
    ],
    "connections": [
        {"id": "c1", "sources": ["1"], "destinations": ["2"]}
    ]
}` }
    ], []);

    // ========================================================================
    // Project-Level TYPES Section Tests
    // ========================================================================

    /**
     * Test helper for projects with TYPES section
     */
    function testWithTypes(name, typesSection, blocks, shouldFail = false, expectedError = '') {
        let blocksStr = '';
        for (let i = 0; i < blocks.length; i++) {
            const block = blocks[i];
            blocksStr += `    BLOCK LANG=${block.lang} Block${i + 1}
${block.code}
    END_BLOCK
`;
        }
        
        const project = `VOVKPLCPROJECT StructTest
VERSION 1.0

MEMORY
    OFFSET 0
    AVAILABLE 1024
    S 64
    X 64
    Y 64
    M 512
    T 16
    C 8
END_MEMORY

FLASH
    SIZE 32768
END_FLASH

TYPES
${typesSection}
END_TYPES

SYMBOLS
END_SYMBOLS

PROGRAM main
${blocksStr}END_PROGRAM
`;
        
        const result = plc.compileProject(project);
        const hasError = result.problem !== null;
        const error = hasError ? result.problem.message : '';
        
        let testPassed = true;
        let errors = [];
        
        if (shouldFail) {
            if (!hasError) {
                errors.push('Expected compilation to fail but it succeeded');
                testPassed = false;
            } else if (expectedError && !error.toLowerCase().includes(expectedError.toLowerCase())) {
                errors.push(`Expected error to contain: "${expectedError}"`);
                errors.push(`Got: "${error}"`);
                testPassed = false;
            }
        } else {
            if (hasError) {
                errors.push(`Compilation error: ${error}`);
                testPassed = false;
            }
        }
        
        if (testPassed) {
            passed++;
            testResults.push({ name, passed: true });
        } else {
            failed++;
            testResults.push({ name, passed: false, error: errors[0] });
            failures.push({ name, errors });
        }
    }

    // Test: Struct declared in TYPES section, used in PLCScript
    testWithTypes('TYPES section struct used in PLCScript', `
    Motor = struct {
        speed: i32;
        enabled: bool;
    };
`, [
        { lang: 'PLCSCRIPT', code: `let motor: Motor @ M100;
motor.speed = 1500;` }
    ]);

    // Test: Struct declared in TYPES section, used in PLCASM
    testWithTypes('TYPES section struct used in PLCASM', `
    Counter = struct {
        value: i32;
        preset: i32;
    };
`, [
        { lang: 'PLCSCRIPT', code: `let cnt: Counter @ M0;` },
        { lang: 'PLCASM', code: `i32.load_from cnt.value
i32.move_to M100` }
    ]);

    // Test: Struct declared in TYPES section, used in STL
    testWithTypes('TYPES section struct used in STL', `
    Data = struct {
        input: i32;
        output: i32;
    };
`, [
        { lang: 'PLCSCRIPT', code: `let data: Data @ M50;` },
        { lang: 'STL', code: `L data.input
T data.output` }
    ]);

    // Test: Struct declared in TYPES section, used in LADDER
    testWithTypes('TYPES section struct used in LADDER', `
    Button = struct {
        pressed: bool;
        latched: bool;
    };
`, [
        { lang: 'PLCSCRIPT', code: `let btn: Button @ M0;` },
        { lang: 'LADDER', code: `{
    "nodes": [
        {"id": "1", "type": "contact", "x": 0, "y": 0, "symbol": "btn.pressed"},
        {"id": "2", "type": "coil", "x": 1, "y": 0, "symbol": "M10.0"}
    ],
    "connections": [
        {"id": "c1", "sources": ["1"], "destinations": ["2"]}
    ]
}` }
    ]);

    // ========================================================================
    // Collision Detection Tests
    // ========================================================================

    // Test: Duplicate identical struct in TYPES (should compile with warning)
    testWithTypes('Duplicate identical struct in TYPES (warning)', `
    MyStruct = struct {
        value: i32;
    };
    MyStruct = struct {
        value: i32;
    };
`, [
        { lang: 'PLCSCRIPT', code: `let x: MyStruct @ M0;` }
    ]);

    // Test: Conflicting struct in TYPES (different field count)
    testWithTypes('Conflicting struct in TYPES (field count differs)', `
    Conflict1 = struct {
        value: i32;
    };
    Conflict1 = struct {
        value: i32;
        extra: bool;
    };
`, [
        { lang: 'PLCSCRIPT', code: `let x: Conflict1 @ M0;` }
    ], true, 'Conflicting');

    // Test: Conflicting struct in TYPES (field type differs)
    testWithTypes('Conflicting struct in TYPES (field type differs)', `
    Conflict2 = struct {
        value: i32;
    };
    Conflict2 = struct {
        value: i16;
    };
`, [
        { lang: 'PLCSCRIPT', code: `let x: Conflict2 @ M0;` }
    ], true, 'Conflicting');

    // Test: Conflicting struct in TYPES (field name differs)
    testWithTypes('Conflicting struct in TYPES (field name differs)', `
    Conflict3 = struct {
        value: i32;
    };
    Conflict3 = struct {
        data: i32;
    };
`, [
        { lang: 'PLCSCRIPT', code: `let x: Conflict3 @ M0;` }
    ], true, 'Conflicting');

    // Test: TYPES defines struct, PLCScript redefines identical (warning)
    testWithTypes('TYPES + PLCScript duplicate identical (warning)', `
    SharedType = struct {
        count: i32;
    };
`, [
        { lang: 'PLCSCRIPT', code: `
type SharedType = struct {
    count: i32;
};
let x: SharedType @ M0;` }
    ]);

    // Test: TYPES defines struct, PLCScript redefines conflicting (error)
    testWithTypes('TYPES + PLCScript conflicting (error)', `
    ConflictType = struct {
        value: i32;
    };
`, [
        { lang: 'PLCSCRIPT', code: `
type ConflictType = struct {
    value: bool;
};
let x: ConflictType @ M0;` }
    ], true, 'Conflicting');

    // Test: Multiple PLCScript blocks with identical struct (warning)
    testMixed('Multiple PLCScript blocks duplicate identical', [
        { lang: 'PLCSCRIPT', code: `
type MultiBlock = struct {
    value: i32;
};
let a: MultiBlock @ M0;` },
        { lang: 'PLCSCRIPT', code: `
type MultiBlock = struct {
    value: i32;
};
let b: MultiBlock @ M10;` }
    ], []);

    // Test: Multiple PLCScript blocks with conflicting struct (error)
    testMixed('Multiple PLCScript blocks conflicting struct', [
        { lang: 'PLCSCRIPT', code: `
type ConflictMulti = struct {
    value: i32;
};
let a: ConflictMulti @ M0;` },
        { lang: 'PLCSCRIPT', code: `
type ConflictMulti = struct {
    data: i32;
};
let b: ConflictMulti @ M10;` }
    ], [], true, 'Conflicting');

    return {
        name: 'Typed Structs Integration',
        passed,
        failed,
        total: passed + failed,
        tests: testResults,
        failures
    };
}

// Run directly if called from command line
const isMain = process.argv[1] && (
    process.argv[1].endsWith('test_structs.js') || 
    process.argv[1].includes('test_structs')
);

if (isMain) {
    const result = await runTests({ silent: false });
    
    console.log('\nTyped Struct Integration Tests:\n');
    for (const test of result.tests) {
        if (test.passed) {
            console.log(`✓ ${test.name}`);
        } else {
            console.log(`✗ ${test.name}`);
            if (test.error) console.log(`  ${test.error}`);
        }
    }
    
    console.log(`\n${result.passed} passed, ${result.failed} failed`);
    process.exit(result.failed > 0 ? 1 : 0);
}
