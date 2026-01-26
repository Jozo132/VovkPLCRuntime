import VovkPLC from '../dist/VovkPLC.js';

const runtime = new VovkPLC();
let output = '';
runtime.stdout_callback = (s) => output += s;
await runtime.initialize(null, false, false);
runtime.readStream();

const code = `PROJECT Test
VERSION 1.0

SYMBOLS
    start_btn : bit : X0.0
END_SYMBOLS

PROGRAM Main
    BLOCK Init LANG=PLCASM
u8.readBit start_btn
exit
    END_BLOCK
END_PROGRAM
`;

runtime.wasm_exports.project_reset();
runtime.wasm_exports.streamClear();
for (let i = 0; i < code.length; i++) {
    runtime.wasm_exports.streamIn(code.charCodeAt(i));
}
const result = runtime.wasm_exports.project_compile(1); // debug mode
console.log('Result:', result);
console.log('Output:', output);
