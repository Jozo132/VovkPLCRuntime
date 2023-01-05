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
            abort: () => { throw new Error("abort") },
            __print: console_print,
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
    } catch (error) {
        if (message) console.log(message)
        console.error(error)
    }

    // // @ts-ignore
    // result = main()
    // console.log("Result:", result)
}

run_element.addEventListener("click", run)


