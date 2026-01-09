// @ts-check
"use strict"

import PLCRuntimeWasm from "../../dist/VovkPLC.js"

const main = async () => {
    const runtime = await PLCRuntimeWasm.createWorker("/dist/VovkPLC.wasm")
    const PLC = {
        callExport: runtime.callExport,
        exports: await runtime.getExports(),
    }
    const perf = globalThis.performance || {now: () => Date.now()}

    const run = () => runtime.run()
    const runDirty = () => runtime.callExport('runDirty')
    const run_unit_test = () => runtime.callExport('run_unit_test')
    const run_custom_test = () => runtime.callExport('run_custom_test')
    const get_free_memory = () => runtime.callExport('get_free_memory')
    const doNothing = () => runtime.callExport('doNothing')
    const downloadAssembly = assembly => runtime.downloadAssembly(assembly)
    const compileAssembly = debug => runtime.callExport('compileAssembly', debug)
    const loadCompiledProgram = () => runtime.callExport('loadCompiledProgram')
    const runFullProgramDebug = () => runtime.callExport('runFullProgramDebug')
    const extractProgram = () => runtime.extractProgram()


    const unit_test = async () => {
        try {
            console.log("Running VovkPLCRuntime WebAssembly simulation unit test...")
            print_console("Running VovkPLCRuntime WebAssembly simulation unit test...")
            await run_unit_test()
            console.log("Done.")
            print_console("Done.")
        } catch (error) {
            console.error(error)
        }
    }
    let cycle_time = 0
    let cycle_time_max = 0
    const run_cycle = async () => {
        try {
            const t = perf.now()
            await run()
            const dt = perf.now() - t
            cycle_time = dt
            if (dt > cycle_time_max) cycle_time_max = dt
        } catch (error) {
            console.error(error)
        }
    }

    const toggle_run = () => {
        simulation_active = !simulation_active
        const text = simulation_active ? "Stop" : "Start"
        if (toggle_run_element) toggle_run_element.innerText = text
    }

    let old_memory = 0
    const checkForMemoryLeak = async () => {
        const now = await get_free_memory()
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

    const do_nothing_job = async () => {
        console.log("Running doNothing()...")
        print_console("Running doNothing()...")
        await doNothing()
    }

    const do_compile_test = async () => { // @ts-ignore
        const assembly = assembly_element.value
        //  const bytecode = compile(assembly) // Compile the assembly code to the final runtime bytecode
        const message = "Compiling the assembly code..."
        console.log(message)
        print_console(message)
        if (await downloadAssembly(assembly)) return // Push the assembly code to the runtime
        if (await compileAssembly(true)) return // Compile the code and return if there is an error
        if (await loadCompiledProgram()) return // Load the compiled program into the runtime
        await runFullProgramDebug() // Verify the code by running it in the simulator
        const { size, output } = await extractProgram() // Extract the current program from the runtime
        const downloadString = runtime.buildCommand.programDownload(output)
        const download_command = `Program download string for Serial/UART/RS232: ${downloadString}`
        console.log(download_command)
        print_console(download_command)
    }

    const cycle_time_element = document.getElementById("cycle-time")
    const unit_test_element = document.getElementById("unit-test")
    const compile_test_element = document.getElementById("compile-test")
    const run_cycle_element = document.getElementById("run-cycle")
    const toggle_run_element = document.getElementById("toggle-run")
    const do_nothing_element = document.getElementById("do-nothing")
    const leak_check_element = document.getElementById("leak-check")
    const output_element = document.getElementById("output")
    const console_element = document.getElementById("console")

    if (!cycle_time_element) throw new Error("cycle_time_element not found")
    if (!unit_test_element) throw new Error("unit_test_element not found")
    if (!compile_test_element) throw new Error("compile_test_element not found")
    if (!run_cycle_element) throw new Error("run_cycle_element not found")
    if (!toggle_run_element) throw new Error("toggle_run_element not found")
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

    runtime.onStdout(msg => print_console(msg)).catch(console.error)

    unit_test_element.addEventListener("click", () => unit_test().catch(console.error))
    compile_test_element.addEventListener("click", () => do_compile_test().catch(console.error))
    run_cycle_element.addEventListener("click", () => run_cycle().catch(console.error))
    toggle_run_element.addEventListener("click", toggle_run)
    do_nothing_element.addEventListener("click", () => do_nothing_job().catch(console.error))
    leak_check_element.addEventListener("click", () => checkForMemoryLeak().catch(console.error))


    const assembly_element = document.getElementById('assembly')
    if (!assembly_element) throw new Error("Assembly element not found")
    const assembly_template = await (await fetch('./samples/blinky.asm')).text() // @ts-ignore
    assembly_element.value = assembly_template



    const hmi_element = document.getElementById('hmi')
    if (!hmi_element) throw new Error("HMI element not found")

    let memory_snapshot = new Uint8Array(0)
    const readBit = offset => {
        const index = Math.floor(offset)
        const bit = Math.min(10 * (offset - index), 7)
        // console.log(`Reading bit ${bit} from memory area ${index}`)
        const byte = memory_snapshot[index] || 0
        return (byte & (1 << bit)) !== 0
    }

    let simulation_active = true
    let hmi_data_state = {}
    let hmi_updating = false
    const update_hmi = async () => {
        if (hmi_updating) return
        hmi_updating = true
        // const state = readBit(10.0)
        // hmi_element.innerHTML = state ? "ON" : "OFF"
        try {
            if (simulation_active) await run_cycle()
            const columns = 16
            const rows = 4
            const memory_bytes = await runtime.readMemoryArea(0, columns * rows)
            memory_snapshot = memory_bytes
            const memory = Array.from(memory_bytes)
                .map(x => [x, x.toString(16).padStart(2, '0').toUpperCase()])
                .map(([x, hex]) => `<span style="color: #${+x > 0 ? 'DDD' : '000'}">${hex}</span>`)
            const addresses = memory.map((_, i) => i.toString().padStart(2, '0').toUpperCase())
            const output = []

            output.push(`
        <span>
            &nbsp;&nbsp;
            <span style="color: #888">${addresses.slice(0, columns).join(' ')}</span>
        </span>
    `)

            let row = 0
            for (let i = 0; i < memory.length; i += columns) {
                const memory_str = memory.slice(i, i + columns).join(' ')
                const content = `
            <span>
                <span style="color: #888">${(columns * row).toString().padStart(2, '0').toUpperCase()}&nbsp;</span>${memory_str}
            </span>
        `
                output.push(content)
                row++
            }
            const content = output.join('<br>')
            if (hmi_data_state.content !== content) {
                hmi_element.innerHTML = content
                hmi_data_state.content = content
            }
            cycle_time_element.innerText = `${cycle_time.toFixed(1)} (max ${cycle_time_max.toFixed(1)})`
        } finally {
            hmi_updating = false
        }
    }


    setInterval(() => update_hmi().catch(console.error), 100)

    Object.assign(window, {
        readBit,
        update_hmi
    })


    Object.assign(window, {
        runtime,
        PLC,
    })
}

main().catch(console.error)
