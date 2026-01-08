import { execSync } from 'child_process';
import { resolve } from 'path';
import { platform } from 'os';

const isWindows = platform() === 'win32';
const BUILD_PATH = './wasm/';
const BUILD_FILE = isWindows ? 'test.bat' : 'test.sh';
const TARGET_RELATIVE = BUILD_PATH + BUILD_FILE;
const args = process.argv.slice(2).join(' ');
const TARGET = resolve(TARGET_RELATIVE) + ' ' + args;

console.log('-----------------------------------------------------------------');
console.log('Note: to compile the WASM tests, you need to have LLVM installed.');
console.log('-----------------------------------------------------------------');
console.log('Executing: ' + TARGET_RELATIVE);

try {
    execSync(TARGET, { stdio: 'inherit', shell: true });
    process.exit(0);
} catch (error) {
    console.error('Error building the WASM executable.');
    console.error(error.status);
    process.exit(error.status || 1);
}
