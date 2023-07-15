// @ts-check
"use strict"

// This file is used to the WebAssembly module (which was compiled from C++ code) to run in Node.js.
// It is used for testing purposes only.

const simulator = require("../simulator")

const main = async () => {
    await simulator.initialize()
    // Get the main functions
    const { run_unit_test, run_custom_test } = simulator.getExports()
    
    if (!run_unit_test) throw new Error("'run_unit_test' function not found")
    if (!run_custom_test) throw new Error("'run_custom_test' function not found")

    let message = ''
    try {
        console.log("Running VovkPLCRuntime WebAssembly simulation unit test...")
        // @ts-ignore
        run_unit_test()
        // @ts-ignore
        for (let i = 0; i < 10; i++) run_custom_test()
        message = simulator.readStream()
        console.log("Done.")
    } catch (error) {
        if (message) console.log(message)
        console.error(error)
    }

}

main().catch(console.error).finally(() => process.exit())