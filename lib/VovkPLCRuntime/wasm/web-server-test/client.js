// @ts-check
"use strict"

const run_element = document.getElementById("run")
const output_element = document.getElementById("output")

if (!run_element) throw new Error("run_element not found")
if (!output_element) throw new Error("output_element not found")


let message = ''
const console_print = c => {
    const char = String.fromCharCode(c)
    if (char === '\n') {
        console.log(message)
        message = ''
    } else {
        message += char
    }
}

let wasm = null
let running = false
const initWasm = async () => {
    if (running) return wasm
    running = true
    console.log("Starting up...")
    const wasmFile = await fetch("/simulator.wasm")
    const wasmBuffer = await wasmFile.arrayBuffer()

    // Compile the WebAssembly module because the buffer size is too large
    const wasmModule = await WebAssembly.compile(wasmBuffer)

    const wasmImports = {
        env: {
            stdout: console_print,
        }
    }

    // Instantiate the WebAssembly module
    const wasmInstance = await WebAssembly.instantiate(wasmModule, wasmImports)

    wasm = wasmInstance
    return wasmInstance
}

const run = async () => {

    const wasmInstance = await initWasm()

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

run_element.addEventListener("click", run)


