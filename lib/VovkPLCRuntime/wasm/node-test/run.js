// @ts-check
"use strict"

// This file is used to the WebAssembly module (which was compiled from C++ code) to run in Node.js.
// It is used for testing purposes only.

const fs = require("fs")
const path = require("path")

const wasmBuffer = fs.readFileSync(path.join(__dirname, "../simulator.wasm"))

let wasmMalloc, wasmFree;

const wasmImports = {
    env: {
        abort: () => {
            throw new Error("abort")
        },
        memory: new WebAssembly.Memory({ initial: 25600 }),
        table: new WebAssembly.Table({ initial: 12800, element: "anyfunc" }),
        malloc: size => wasmMalloc ? wasmMalloc(size) : 0,
        free: ptr => wasmFree ? wasmFree(ptr) : 0,
        __print: c => process.stdout.write(String.fromCharCode(c)),
        // LinkError: WebAssembly.instantiate(): Import #3 module="env" function="memset" error: function import requires a callable
        memset: (ptr, value, size) => {
            const buffer = new Uint8Array(wasmImports.env.memory.buffer, ptr, size)
            buffer.fill(value)
        },
        _Znam: size => wasmMalloc ? wasmMalloc(size) : 0,
        _Znwm: size => wasmMalloc ? wasmMalloc(size) : 0,
        _ZdaPv: ptr => wasmFree ? wasmFree(ptr) : 0,
        _ZdlPv: ptr => wasmFree ? wasmFree(ptr) : 0,
    }
}

const main = async () => {
    const wasmModule = await WebAssembly.compile(wasmBuffer)
    const wasmInstance = await WebAssembly.instantiate(wasmModule, wasmImports)

    // Get the wasmMalloc and wasmFree functions
    wasmMalloc = wasmInstance.exports.malloc
    wasmFree = wasmInstance.exports.free

    // Get the main functions
    const { main, setup, loop } = wasmInstance.exports
    if (!main) throw new Error("main function not found")
    if (!setup) throw new Error("setup function not found")
    if (!loop) throw new Error("loop function not found")

    let result

    console.log("Running setup...")
    // @ts-ignore
    result = setup()
    console.log("Result:", result)

    console.log("Running loop once...")
    // @ts-ignore
    result = loop()
    console.log("Result:", result)

    // // @ts-ignore
    // result = main()
    console.log("Result:", result)

}

main().catch(console.error).finally(() => process.exit())