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


let error_message = ''
/** @param { number | number[] | string } charcode */
const console_error = charcode => {
    if (typeof charcode === 'string') {
        if (charcode.length === 0) return
        if (charcode.length === 1) return console_error(charcode.charCodeAt(0))
        return charcode.split('').forEach(console_error)
    }
    if (Array.isArray(charcode)) return charcode.forEach(console_error)
    const char = String.fromCharCode(charcode)
    if (char === '\0' || char === '\n') {
        console.error(error_message)
        error_message = ''
    } else error_message += char
}

let stream_message = ''
/** @param { number | number[] | string } charcode */
const console_stream = charcode => {
    if (typeof charcode === 'string') {
        if (charcode.length === 0) return
        if (charcode.length === 1) return console_stream(charcode.charCodeAt(0))
        return charcode.split('').forEach(console_stream)
    }
    if (Array.isArray(charcode)) return charcode.forEach(console_stream)
    const char = String.fromCharCode(charcode)
    stream_message += char
    if (stream_message.length > 100000) stream_message = stream_message.substring(stream_message.length - 100000 + 1)
}

const readStream = () => {
    const output = stream_message
    stream_message = ''
    return output
}
const main = async () => {
    const wasmImports = {
        env: {
            stdout: console_print,
            stderr: console_error,
            streamOut: console_stream,
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