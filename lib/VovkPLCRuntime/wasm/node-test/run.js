// @ts-check
"use strict"

// This file is used to the WebAssembly module (which was compiled from C++ code) to run in Node.js.
// It is used for testing purposes only.

const fs = require("fs")
const path = require("path")

const wasmBuffer = fs.readFileSync(path.join(__dirname, "../simulator.wasm"))

let message = ''
const print = charcode => {
    const char = String.fromCharCode(charcode)
    if (char === '\0' || char === '\n') {
        console.log(message)
        message = ''
    } else {
        message += char
    }
}

const wasmImports = {
    env: {
        stdout: print,
    }
}

const main = async () => {
    const wasmModule = await WebAssembly.compile(wasmBuffer)
    const wasmInstance = await WebAssembly.instantiate(wasmModule, wasmImports)

    // Get the main functions
    const { run_unit_test } = wasmInstance.exports
    if (!run_unit_test) throw new Error("'run_unit_test' function not found")

    try {
        console.log("Running VovkPLCRuntime WebAssembly simulation unit test...") // @ts-ignore
        run_unit_test()
        console.log("Done.")
    } catch (error) {
        if (message) console.log(message)
        console.error(error)
    }

}

main().catch(console.error).finally(() => process.exit())