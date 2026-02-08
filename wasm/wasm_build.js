import { execSync } from 'child_process';
import { resolve } from 'path';
import { platform } from 'os';

const isWindows = platform() === 'win32';
const BUILD_PATH = './wasm/';
const BUILD_FILE = isWindows ? 'build.bat' : 'build.sh';
const TARGET_RELATIVE = BUILD_PATH + BUILD_FILE;
const TARGET = resolve(TARGET_RELATIVE);

// Check for --safe flag
const safeMode = process.argv.includes('--safe');

// Check that LLVM tools (clang++ and wasm-ld) are installed
function checkCommand(cmd) {
    try {
        const which = isWindows ? 'where' : 'which';
        execSync(`${which} ${cmd}`, { stdio: 'ignore' });
        return true;
    } catch {
        return false;
    }
}

const hasClang = checkCommand('clang++');
const hasWasmLd = checkCommand('wasm-ld');

if (!hasClang || !hasWasmLd) {
    const missing = [!hasClang && 'clang++', !hasWasmLd && 'wasm-ld'].filter(Boolean).join(', ');
    console.error('----------------------------------------------------------------------');
    console.error('ERROR: LLVM is not installed or not in PATH.');
    console.error(`Missing: ${missing}`);
    console.error('');
    console.error('To compile the WASM executable, you need LLVM with WebAssembly support.');
    console.error('Install LLVM from: https://releases.llvm.org/download.html');
    console.error('----------------------------------------------------------------------');
    process.exit(1);
}

console.log(`Building the WASM executable (${safeMode ? 'SAFE MODE' : 'production'}) ...`);
console.log('Executing: ' + TARGET_RELATIVE + (safeMode ? ' --safe' : ''));

try {
    const command = safeMode ? `"${TARGET}" --safe` : `"${TARGET}"`;
    execSync(command, { stdio: 'inherit', shell: true });
    
    const msg = `The compiled WASM files are located in: ${BUILD_PATH}dist/`;
    console.log('-'.repeat(msg.length));
    console.log(msg);
    if (safeMode) {
        console.log(`  VovkPLC.wasm        - safe mode (with PLCRUNTIME_SAFE_MODE bounds checks)`);
    } else {
        console.log(`  VovkPLC.wasm        - production (no bounds checks)`);
    }
    console.log(`Along with: ${BUILD_PATH}dist/VovkPLC.js, ${BUILD_PATH}dist/VovkPLC.worker.js`);
    console.log('-'.repeat(msg.length));
    process.exit(0);
} catch (error) {
    console.error('Error building the WASM executable.');
    console.error(error.status);
    process.exit(error.status || 1);
}
