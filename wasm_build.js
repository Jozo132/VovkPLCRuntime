import { execSync } from 'child_process';
import { resolve } from 'path';
import { platform } from 'os';

const isWindows = platform() === 'win32';
const BUILD_PATH = './wasm/dist/';
const BUILD_FILE = isWindows ? 'build.bat' : 'build.sh';
const TARGET_RELATIVE = BUILD_PATH + BUILD_FILE;
const TARGET = resolve(TARGET_RELATIVE);

console.log('----------------------------------------------------------------------');
console.log('Note: to compile the WASM executable, you need to have LLVM installed.');
console.log('----------------------------------------------------------------------');
console.log('Building the WASM executable ...');
console.log('Executing: ' + TARGET_RELATIVE);

try {
    execSync(TARGET, { stdio: 'inherit', shell: true });
    
    const msg = `The compiled WASM file is located in: ${BUILD_PATH}VovkPLC.wasm`;
    console.log('-'.repeat(msg.length));
    console.log(msg);
    console.log(`Along with the JS wrapper: ${BUILD_PATH}VovkPLC.js`);
    console.log('-'.repeat(msg.length));
    console.log(`To use it, navigate into the ${BUILD_PATH} directory`);
    console.log('and you can either test it like for example:');
    console.log("   - Node.JS -> cd into 'node-test' and run 'node run.js'");
    console.log("   - Browser -> cd into 'web-server-test' and run 'node server.js'");
    console.log("Or you can copy the 'VovkPLC.wasm' and 'VovkPLC.js' files into your project and use them there as you wish.");
    process.exit(0);
} catch (error) {
    console.error('Error building the WASM executable.');
    console.error(error.status);
    process.exit(error.status || 1);
}
