// @ts-check
"use strict"

// This file is used to the WebAssembly module (which was compiled from C++ code) to run in Node.js.
// It is used for testing purposes only.

const fs = require("fs")
const path = require("path")


let message = ''
const console_print = charcode => {
    const char = String.fromCharCode(charcode)
    if (char === '\0' || char === '\n') {
        console.log(message)
        message = ''
    } else {
        message += char
    }
}

const main = async () => {
    const wasmImports = {
        env: {
            stdout: console_print,
        }
    }
    const wasmBuffer = fs.readFileSync(path.join(__dirname, "../simulator.wasm"))
    const wasmModule = await WebAssembly.compile(wasmBuffer)
    const wasmInstance = await WebAssembly.instantiate(wasmModule, wasmImports)

    // Get the main functions
    const { run_unit_test, run_custom_test } = wasmInstance.exports
    if (!run_unit_test) throw new Error("'run_unit_test' function not found")
    if (!run_custom_test) throw new Error("'run_custom_test' function not found")

    try {
        console.log("Running VovkPLCRuntime WebAssembly simulation unit test...")
        // @ts-ignore
        run_unit_test()
        // @ts-ignore
        for (let i = 0; i < 10; i++) run_custom_test()
        console.log("Done.")
    } catch (error) {
        if (message) console.log(message)
        console.error(error)
    }

}

main().catch(console.error).finally(() => process.exit())