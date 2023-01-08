// @ts-check
"use strict"

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
    // const wasmFile = await fetch("/wasm_test_cases/string_alloc.wasm")
    const wasmFile = await fetch("/simulator.wasm")
    const wasmBuffer = await wasmFile.arrayBuffer()
    const wasmImports = {
        env: {
            stdout: console_print,
        }
    }
    const wasmModule = await WebAssembly.compile(wasmBuffer)
    const wasmInstance = await WebAssembly.instantiate(wasmModule, wasmImports)
    Object.assign(window, wasmInstance.exports)
    wasm = wasmInstance
    return wasmInstance
}

const run = async () => {

    const wasmInstance = await initWasm()

    // Get the main functions
    const { run_unit_test, run_custom_test } = wasmInstance.exports
    if (!run_unit_test) throw new Error("'run_unit_test' function not found")
    if (!run_custom_test) throw new Error("'run_custom_test' function not found")

    try {
        console.log("Running VovkPLCRuntime WebAssembly simulation unit test...") // @ts-ignore
        run_unit_test()
        console.log("Done. Now running custom code...")
        run_custom_test()
        console.log("Done.")
    } catch (error) {
        if (message) console.log(message)
        console.error(error)
    }
}


const init_element = document.getElementById("init")
const run_element = document.getElementById("run")
const do_nothing_element = document.getElementById("do-nothing")
const leak_check_element = document.getElementById("leak-check")
const output_element = document.getElementById("output")

if (!init_element) throw new Error("init_element not found")
if (!run_element) throw new Error("run_element not found")
if (!do_nothing_element) throw new Error("do_nothing_element not found")
if (!leak_check_element) throw new Error("leak_check_element not found")
if (!output_element) throw new Error("output_element not found")


let old_memory = 0
const checkForMemoryLeak = () => {
    const { get_free_memory } = wasm.exports
    if (!get_free_memory) throw new Error("'get_free_memory' function not found")

    const now = get_free_memory()
    const diff_in_bytes = old_memory - now
    if (diff_in_bytes > 0) {
        console.error(`Memory leak detected: ${diff_in_bytes} bytes`)
    } else {
        console.log("No memory leak detected.")
    }
    old_memory = now
}

const do_nothing_job = () => {
    const { doNothing } = wasm.exports
    if (!doNothing) throw new Error("'doNothing' function not found")
    console.log("Running doNothing()...")
    doNothing()
}

init_element.addEventListener("click", initWasm)
run_element.addEventListener("click", run)
do_nothing_element.addEventListener("click", do_nothing_job)
leak_check_element.addEventListener("click", checkForMemoryLeak)


