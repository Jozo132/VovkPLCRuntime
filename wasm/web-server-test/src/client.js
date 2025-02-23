// @ts-check
"use strict"

import PLCRuntimeWasm from "../../simulator.js"

const runtime = new PLCRuntimeWasm("/simulator.wasm")
await runtime.initialize()
const PLC = runtime.getExports()

const {
    run,
    runDirty,
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
        print_console("Running VovkPLCRuntime WebAssembly simulation unit test...")
        run_unit_test()
        console.log("Done.")
        print_console("Done.")
    } catch (error) {
        console.error(error)
    }
}

const run_cycle = () => {
    try {
        run()
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
        print_console(`Memory leak detected: ${diff_in_bytes} bytes`)
    } else {
        console.log("No memory leak detected.")
        print_console("No memory leak detected.")
    }
    old_memory = now
}

const do_nothing_job = () => {
    console.log("Running doNothing()...")
    print_console("Running doNothing()...")
    doNothing()
}

const do_compile_test = async () => { // @ts-ignore
    const assembly = assembly_element.value
    //  const bytecode = compile(assembly) // Compile the assembly code to the final runtime bytecode
    const message = "Compiling the assembly code..."
    console.log(message)
    print_console(message)
    if (downloadAssembly(assembly)) return // Push the assembly code to the runtime
    if (compileAssembly(true)) return // Compile the code and return if there is an error
    if (loadCompiledProgram()) return // Load the compiled program into the runtime
    runFullProgramDebug() // Verify the code by running it in the simulator
    const { size, output } = extractProgram() // Extract the current program from the runtime
    const downloadString = runtime.buildCommand.programDownload(output)
    const download_command = `Program download string for Serial/UART/RS232: ${downloadString}`
    console.log(download_command)
    print_console(download_command)
}


const unit_test_element = document.getElementById("unit-test")
const compile_test_element = document.getElementById("compile-test")
const run_cycle_element = document.getElementById("run-cycle")
const do_nothing_element = document.getElementById("do-nothing")
const leak_check_element = document.getElementById("leak-check")
const output_element = document.getElementById("output")
const console_element = document.getElementById("console")

if (!unit_test_element) throw new Error("unit_test_element not found")
if (!compile_test_element) throw new Error("compile_test_element not found")
if (!run_cycle_element) throw new Error("run_cycle_element not found")
if (!do_nothing_element) throw new Error("do_nothing_element not found")
if (!leak_check_element) throw new Error("leak_check_element not found")
if (!output_element) throw new Error("output_element not found")
if (!console_element) throw new Error("console_element not found")


let update_timeout = null

const print_console = (text) => {
    const p = document.createElement("p")
    const replace_map = [
        ['&', '&amp;'],
        ['\t', '    '],
        ['\n', '<br>'],
        [' ', '&nbsp;'],
        ['<', '&lt;'],
        ['>', '&gt;'],
    ]
    for (const [from, to] of replace_map) {
        text = text.replace(new RegExp(from, 'g'), to)
    }
    p.innerHTML = text
    console_element.appendChild(p)
    if (update_timeout) clearTimeout(update_timeout)
    update_timeout = setTimeout(() => {
        console_element.scrollTop = console_element.scrollHeight
        // Keep <p> count to a reasonable number and remove the oldest ones
        while (console_element.children.length > 1000) {
            const child = console_element.children.item(0)
            if (!child) break
            console_element.removeChild(child)
        }
        update_timeout = null
    }, 100)
}

runtime.onStdout(msg => print_console(msg))

unit_test_element.addEventListener("click", unit_test)
compile_test_element.addEventListener("click", do_compile_test)
run_cycle_element.addEventListener("click", run_cycle)
do_nothing_element.addEventListener("click", do_nothing_job)
leak_check_element.addEventListener("click", checkForMemoryLeak)


const assembly_element = document.getElementById('assembly')
if (!assembly_element) throw new Error("Assembly element not found")
const assembly_template = await (await fetch('./samples/blinky.asm')).text() // @ts-ignore
assembly_element.value = assembly_template



const hmi_element = document.getElementById('hmi')
if (!hmi_element) throw new Error("HMI element not found")

const readBit = offset => {
    const index = Math.floor(offset)
    const bit = Math.min(10 * (offset - index), 7)
    // console.log(`Reading bit ${bit} from memory area ${index}`)
    const memory = runtime.readMemoryArea(index, 1)
    return (memory[0] & (1 << bit)) !== 0
}
let hmi_data_state = {}
const update_hmi = () => {
    // const state = readBit(10.0)
    // hmi_element.innerHTML = state ? "ON" : "OFF"
    run_cycle()
    const memory = [...runtime.readMemoryArea(0, 40)].map(x => x.toString(16).padStart(2, '0'))
    const addresses = memory.map((_, i) => i.toString().padStart(2, '0'))
    const columns = 10
    const rows = []

    rows.push(`
        <span style="font-family: monospace; font-size: 2em;">
            &nbsp;&nbsp;
            <span style="color: #888">${addresses.slice(0, columns).join(' ')}</span>
        </span>
    `)

    let row = 0
    for (let i = 0; i < memory.length; i += columns) {
        const memory_str = memory.slice(i, i + columns).join(' ')
        const content = `
            <span style="font-family: monospace; font-size: 2em;">
                <span style="color: #888">${(columns * row).toString().padStart(2, '0')}&nbsp;</span>${memory_str}
            </span>
        `
        rows.push(content)
        row++
    }
    const content = rows.join('<br>')
    if (hmi_data_state.content !== content) {
        hmi_element.innerHTML = content
        hmi_data_state.content = content
    }
}


setInterval(update_hmi, 100)

Object.assign(window, {
    readBit,
    update_hmi
})