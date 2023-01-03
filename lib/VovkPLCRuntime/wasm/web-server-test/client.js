// @ts-check
"use strict"

const run_element = document.getElementById("run")
const output_element = document.getElementById("output")

if (!run_element) throw new Error("run_element not found")
if (!output_element) throw new Error("output_element not found")


let running = false
const run = async () => {
    if (running) return console.warn("Already running")
    running = true

    console.log("Starting up...")

    const wasmBuffer = await fetch("/simulator.wasm").then(response => response.arrayBuffer())
    // const wasmModule = new WebAssembly.Module(wasmBuffer)
    // Compile the WebAssembly module because the buffer size is too large
    const wasmModule = await WebAssembly.compile(wasmBuffer)

    let wasmMalloc, wasmFree;

    let message = ''
    const print = c => {
        const char = String.fromCharCode(c)
        if (char === '\n') {
            console.log(message)
            message = ''
        } else {
            message += char
        }
    }
    const memory = new WebAssembly.Memory({ initial: 2560 }) // 64k * 256 = 16MB
    const wasmImports = {
        js: {
            mem: memory,
        },
        env: {
            abort: () => {
                throw new Error("abort")
            },
            memory,
            table: new WebAssembly.Table({ initial: 1280, element: "anyfunc" }),
            __print: print,   // Add __print(char c) function.

            // malloc: len => wasmMalloc ? wasmMalloc(len) : 0,
            // free: addr => wasmFree ? wasmFree(addr) : 0,

            _Znam: len => wasmMalloc ? wasmMalloc(len) : 0,               // [LinkError: WebAssembly.instantiate(): Import #2 module="env" function="_Znam" error: function import requires a callable]
            _ZdlPv: addr => wasmFree ? wasmFree(addr) : 0,                // [LinkError: WebAssembly.instantiate(): Import #3 module="env" function="_ZdlPv" error: function import requires a callable]
            memset: (addr, value, len) => {                               // [LinkError: WebAssembly.instantiate(): Import #4 module="env" function="memset" error: function import requires a callable]
                const memory = new Uint8Array(wasmImports.env.buffer)
                for (let i = 0; i < len; i++) {
                    memory[addr + i] = value
                }
            },
            _Znwm: len => wasmMalloc ? wasmMalloc(len) : 0,               // [LinkError: WebAssembly.instantiate(): Import #5 module="env" function="_Znwm" error: function import requires a callable]
            _ZdaPv: addr => wasmFree ? wasmFree(addr) : 0,                // [LinkError: WebAssembly.instantiate(): Import #6 module="env" function="_ZdaPv" error: function import requires a callable]

        }
    }

    // Instantiate the WebAssembly module
    const wasmInstance = await WebAssembly.instantiate(wasmModule, wasmImports)

    // Get the wasmMalloc and wasmFree functions
    wasmMalloc = wasmInstance.exports.malloc
    wasmFree = wasmInstance.exports.free

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

    // // // @ts-ignore
    // result = main()
    // console.log("Result:", result)
}

run_element.addEventListener("click", run)


