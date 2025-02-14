// @ts-check
"use strict"

import PLCRuntimeWasm from "../../simulator.js"

const wasm = await PLCRuntimeWasm.initialize("/simulator.wasm")
const PLC = wasm.getExports()

const {
    run_unit_test,
    run_custom_test,
    get_free_memory,
    doNothing,
    downloadAssembly,
    compileAssembly,
    loadCompiledProgram,
    runFullProgramDebug,
    extractProgram
} = PLC


const unit_test = () => {


    try {
        console.log("Running VovkPLCRuntime WebAssembly simulation unit test...")
        run_unit_test()
        console.log("Done.")
    } catch (error) {
        console.error(error)
    }
}

const run = () => {
    try {
        console.log("Running custom code...")
        run_custom_test()
        console.log("Done.")
    } catch (error) {
        console.error(error)
    }
}

let old_memory = 0
const checkForMemoryLeak = () => {
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
    console.log("Running doNothing()...")
    doNothing()
}

const do_compile_test = async () => {
    const d = document.getElementById('assembly')
    if (!d) throw new Error("Assembly element not found") // @ts-ignore
    const assembly = d.value
    //  const bytecode = compile(assembly) // Compile the assembly code to the final runtime bytecode
    console.log("Running compileAssembly()...")
    if (downloadAssembly(assembly)) return // Push the assembly code to the runtime
    if (compileAssembly()) return // Compile the code and return if there is an error
    if (loadCompiledProgram()) return // Load the compiled program into the runtime
    runFullProgramDebug() // Verify the code by running it in the simulator
    const { size, output } = extractProgram() // Extract the current program from the runtime
    const downloadString = PLCRuntimeWasm.buildCommand.programDownload(output)
    console.log(`Program download string for Serial/UART/RS232: ${downloadString}`)
}


const unit_test_element = document.getElementById("unit-test")
const compile_test_element = document.getElementById("compile-test")
const do_nothing_element = document.getElementById("do-nothing")
const leak_check_element = document.getElementById("leak-check")
const output_element = document.getElementById("output")

if (!unit_test_element) throw new Error("unit_test_element not found")
if (!compile_test_element) throw new Error("compile_test_element not found")
if (!do_nothing_element) throw new Error("do_nothing_element not found")
if (!leak_check_element) throw new Error("leak_check_element not found")
if (!output_element) throw new Error("output_element not found")


unit_test_element.addEventListener("click", unit_test)
compile_test_element.addEventListener("click", do_compile_test)
do_nothing_element.addEventListener("click", do_nothing_job)
leak_check_element.addEventListener("click", checkForMemoryLeak)


