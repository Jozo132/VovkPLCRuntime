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
    const { main, setup, loop } = wasmInstance.exports
    if (!main) throw new Error("main function not found")
    if (!setup) throw new Error("setup function not found")
    if (!loop) throw new Error("loop function not found")

    let result
    try {

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
        // console.log("Result:", result)
    } catch (error) {
        if (message) console.log(message)
        throw error
    }

}

main().catch(console.error).finally(() => process.exit())