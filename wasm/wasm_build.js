import { execSync } from 'child_process';
import { resolve } from 'path';
import { platform } from 'os';

const isWindows = platform() === 'win32';
const BUILD_PATH = './wasm/';
const BUILD_FILE = isWindows ? 'build.bat' : 'build.sh';
const TARGET_RELATIVE = BUILD_PATH + BUILD_FILE;
const TARGET = resolve(TARGET_RELATIVE);

console.log('----------------------------------------------------------------------');
console.log('Note: to compile the WASM executable, you need to have LLVM installed.');
console.log('----------------------------------------------------------------------');
console.log('Building the WASM executable ...');
console.log('Executing: ' + TARGET_RELATIVE);

try {
    execSync(`"${TARGET}"`, { stdio: 'inherit', shell: true });
    
    const msg = `The compiled WASM files are located in: ${BUILD_PATH}dist/`;
    console.log('-'.repeat(msg.length));
    console.log(msg);
    console.log(`  VovkPLC.wasm        - optimized (no bounds checks)`);
    console.log(`  VovkPLC-debug.wasm  - debug (with PLCRUNTIME_SAFE_MODE)`);
    console.log(`Along with: ${BUILD_PATH}dist/VovkPLC.js, ${BUILD_PATH}dist/VovkPLC.worker.js`);
    console.log('-'.repeat(msg.length));
    process.exit(0);
} catch (error) {
    console.error('Error building the WASM executable.');
    console.error(error.status);
    process.exit(error.status || 1);
}
