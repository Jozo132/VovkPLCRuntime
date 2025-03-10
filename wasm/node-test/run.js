// @ts-check
"use strict"

// This file is used to the WebAssembly module (which was compiled from C++ code) to run in Node.js.
// It is used for testing purposes only.

// const PLCRuntimeWasm = require("../VovkPLC.js").default
import VovkPLC from "../VovkPLC.js"

const runtime = new VovkPLC("/VovkPLC.wasm")

const main = async () => {
    await runtime.initialize()
    // Get the main functions
    const { run_unit_test, run_custom_test } = runtime.getExports()

    if (!run_unit_test) throw new Error("'run_unit_test' function not found")
    if (!run_custom_test) throw new Error("'run_custom_test' function not found")

    let message = ''
    try {
        console.log("Running VovkPLCRuntime WebAssembly simulation unit test...") // @ts-ignore
        run_unit_test() // @ts-ignore
        for (let i = 0; i < 10; i++) run_custom_test()
        message = runtime.readStream()
        console.log("Done.")
    } catch (error) {
        console.error(error)
    }
    if (message) console.log(message)

}

main().catch(console.error).finally(() => process.exit())