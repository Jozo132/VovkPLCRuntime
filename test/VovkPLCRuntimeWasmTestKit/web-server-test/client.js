// @ts-check
"use strict"

const getWasm = () => PLCRuntimeWasm.initialize("/simulator.wasm")


const run = async () => {
    const wasm = await getWasm()

    // Get the main functions
    const { run_unit_test, run_custom_test } = wasm.exports
    if (!run_unit_test) throw new Error("'run_unit_test' function not found")
    if (!run_custom_test) throw new Error("'run_custom_test' function not found")

    try {
        console.log("Running VovkPLCRuntime WebAssembly simulation unit test...") // @ts-ignore
        run_unit_test()
        console.log("Done. Now running custom code...") // @ts-ignore
        run_custom_test()
        console.log("Done.")
    } catch (error) {
        console.error(error)
    }
}

let old_memory = 0
const checkForMemoryLeak = async () => {
    const wasm = await getWasm()

    const { get_free_memory } = wasm.exports
    if (!get_free_memory) throw new Error("'get_free_memory' function not found") // @ts-ignore

    const now = get_free_memory()
    const diff_in_bytes = old_memory - now
    if (diff_in_bytes > 0) {
        console.error(`Memory leak detected: ${diff_in_bytes} bytes`)
    } else {
        console.log("No memory leak detected.")
    }
    old_memory = now
}

const do_nothing_job = async () => {
    const wasm = await getWasm()

    const { doNothing } = wasm.exports
    if (!doNothing) throw new Error("'doNothing' function not found")
    console.log("Running doNothing()...") // @ts-ignore
    doNothing()
}

const do_compile_test = async () => {
    const wasm = await getWasm()

    const { compileTest } = wasm.exports
    if (!compileTest) throw new Error("'compileTest' function not found")
    console.log("Running compileTest()...") // @ts-ignore
    compileTest()
}


const init_element = document.getElementById("init")
const run_element = document.getElementById("run")
const compile_test_element = document.getElementById("compile-test")
const do_nothing_element = document.getElementById("do-nothing")
const leak_check_element = document.getElementById("leak-check")
const output_element = document.getElementById("output")

if (!init_element) throw new Error("init_element not found")
if (!run_element) throw new Error("run_element not found")
if (!compile_test_element) throw new Error("compile_test_element not found")
if (!do_nothing_element) throw new Error("do_nothing_element not found")
if (!leak_check_element) throw new Error("leak_check_element not found")
if (!output_element) throw new Error("output_element not found")


init_element.addEventListener("click", getWasm)
run_element.addEventListener("click", run)
compile_test_element.addEventListener("click", do_compile_test)
do_nothing_element.addEventListener("click", do_nothing_job)
leak_check_element.addEventListener("click", checkForMemoryLeak)


