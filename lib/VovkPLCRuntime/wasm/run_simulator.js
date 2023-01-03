// @ts-check
"use strict"

// This is a Node.JS script that runs the simulator compiled in WebAssembly.
// The compiled WebAssembly module is loaded from the file system.
// It requires memory to be allocated for the module, so it is run asynchronously.
// It has a main() function that takes no arguments and returns a number.
// The main() function runs forever, so this script will also run forever.
// The script can be terminated by pressing Ctrl-C.

const fs = require("fs")
const path = require("path")

// The wasm module was written in C++ and compiled to WebAssembly.
const wasmFile = path.join(__dirname, "simulator.wasm")
const wasmBuffer = fs.readFileSync(wasmFile)

const main = async () => {

    // First, we prepare the import object.
    // The import object is passed to the WebAssembly.instantiate() function.
    // The import object contains the memory we allocated.
    // The import object also contains functions that the wasm module can call.
    // The wasm module can call these functions by name.

    let wasmMalloc, wasmFree;
    const imports = { env: {} }
    imports.env.memory = new WebAssembly.Memory({
        initial: 8,
        maximum: 32,
    })
    imports.env.__memory_base = 0
    // Add malloc(len) and free(addr) functions. 
    imports.malloc = len => wasmMalloc ? wasmMalloc(len) : 0
    imports.free = addr => wasmFree ? wasmFree(addr) : 0

    imports.env._Znam = len => wasmMalloc ? wasmMalloc(len) : 0                // [LinkError: WebAssembly.instantiate(): Import #2 module="env" function="_Znam" error: function import requires a callable]
    imports.env._ZdlPv = addr => wasmFree ? wasmFree(addr) : 0                 // [LinkError: WebAssembly.instantiate(): Import #3 module="env" function="_ZdlPv" error: function import requires a callable]
    imports.env.memset = (addr, value, len) => {                               // [LinkError: WebAssembly.instantiate(): Import #4 module="env" function="memset" error: function import requires a callable]
        const memory = new Uint8Array(imports.env.memory.buffer)
        for (let i = 0; i < len; i++) {
            memory[addr + i] = value
        }
    }
    imports.env._Znwm = len => wasmMalloc ? wasmMalloc(len) : 0                // [LinkError: WebAssembly.instantiate(): Import #5 module="env" function="_Znwm" error: function import requires a callable]
    imports.env._ZdaPv = addr => wasmFree ? wasmFree(addr) : 0                 // [LinkError: WebAssembly.instantiate(): Import #6 module="env" function="_ZdaPv" error: function import requires a callable]

    imports.env.__print = c => process.stdout.write(String.fromCharCode(c))    // Add __print(char c) function.






    // Next, instantiate the module.
    // The module will use the memory we allocated.
    const wasmModule = await WebAssembly.instantiate(wasmBuffer, imports)
    console.log("Instantiated simulator")

    // Add malloc(len) and free(addr) functions.
    // These functions are exported by the wasm module.
    wasmMalloc = wasmModule.instance.exports.malloc
    wasmFree = wasmModule.instance.exports.free

    // Finally, run the main() function.
    // The main() function will run forever, so this script will also run forever.
    // The main() function returns a number, which is the exit code.
    console.log("Running simulator") // @ts-ignore
    const exitCode = wasmModule.instance.exports.main()
    console.log("Simulator exited with code", exitCode)

}

main().catch(console.error).finally(() => process.exit(0))
