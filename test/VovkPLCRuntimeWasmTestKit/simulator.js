// @ts-check
"use strict"

class PLCRuntimeWasm_class {
    /** @type { WebAssembly.Instance | null } */
    wasm = null
    running = false
    console_message = ''
    error_message = ''
    stream_message = ''

    constructor() { }

    initialize = async (wasm_path = '', debug = false) => {
        wasm_path = wasm_path || "/simulator.wasm" // "/wasm_test_cases/string_alloc.wasm"
        if (this.running && this.wasm) return this.wasm
        this.running = true
        if (debug) console.log("Starting up...")
        /** @type { ArrayBuffer | Buffer } */
        let wasmBuffer
        /** @type { Performance } */
        let perf
        if (typeof module !== 'undefined') { // @ts-ignore
            perf = require('perf_hooks').performance
            const fs = require("fs")
            const path = require("path")
            wasmBuffer = fs.readFileSync(path.join(__dirname, wasm_path))
        } else if (typeof window !== 'undefined') {
            perf = window.performance
            const wasmFile = await fetch(wasm_path)
            wasmBuffer = await wasmFile.arrayBuffer()
        } else throw new Error("Unknown environment")
        const wasmImports = {
            env: {
                stdout: this.console_print,
                stderr: this.console_error,
                streamOut: this.console_stream,
                millis: () => +perf.now().toFixed(0),
                micros: () => +(perf.now() * 1000).toFixed(0),
            }
        }
        const wasmModule = await WebAssembly.compile(wasmBuffer)
        const wasmInstance = await WebAssembly.instantiate(wasmModule, wasmImports)
        if (typeof window !== 'undefined') Object.assign(window, wasmInstance.exports) // Assign all exports to the global scope
        if (!wasmInstance) throw new Error("Failed to instantiate WebAssembly module")
        this.wasm = wasmInstance
        return wasmInstance
    }

    /** @param { string } assembly */
    uploadAssembly = (assembly) => {
        // Use 'bool streamIn(char)' to upload the assembly character by character
        // Use 'void loadAssembly()' to load the assembly into the PLC from the stream buffer
        if (!this.wasm) throw new Error("WebAssembly module not initialized")
        const { streamIn, loadAssembly } = this.wasm.exports
        let ok = true
        for (let i = 0; i < assembly.length && ok; i++) {
            const char = assembly[i]
            const c = char.charCodeAt(0) // @ts-ignore
            ok = streamIn(c)
        }
        if (!ok) throw new Error("Failed to upload assembly") // @ts-ignore
        loadAssembly()
    }

    getExports = () => {
        if (!this.wasm) throw new Error("WebAssembly module not initialized")
        return Object.assign({}, this.wasm.exports, { uploadAssembly: this.uploadAssembly })
    }

    console_print = c => {
        const char = String.fromCharCode(c)
        if (char === '\n') {
            console.log(this.console_message)
            this.console_message = ''
        } else {
            this.console_message += char
        }
    }

    /** @param { number | number[] | string } charcode */
    console_error = charcode => {
        if (typeof charcode === 'string') {
            if (charcode.length === 0) return
            if (charcode.length === 1) return this.console_error(charcode.charCodeAt(0))
            return charcode.split('').forEach(this.console_error)
        }
        if (Array.isArray(charcode)) return charcode.forEach(this.console_error)
        const char = String.fromCharCode(charcode)
        if (char === '\0' || char === '\n') {
            console.error(this.error_message)
            this.error_message = ''
        } else this.error_message += char
    }

    /** @param { number | number[] | string } charcode */
    console_stream = charcode => {
        if (typeof charcode === 'string') {
            if (charcode.length === 0) return
            if (charcode.length === 1) return this.console_stream(charcode.charCodeAt(0))
            return charcode.split('').forEach(this.console_stream)
        }
        if (Array.isArray(charcode)) return charcode.forEach(this.console_stream)
        const char = String.fromCharCode(charcode)
        this.stream_message += char
        if (this.stream_message.length > 100000) this.stream_message = this.stream_message.substring(this.stream_message.length - 100000 + 1)
    }

    readStream = () => {
        const output = this.stream_message
        this.stream_message = ''
        return output
    }
}

const ________PLCRuntimeWasm_______ = new PLCRuntimeWasm_class()

/** @typedef { PLCRuntimeWasm_class } PLCRuntimeWasm */

// Export the module if we are in Node.js
if (typeof module !== 'undefined') module.exports = ________PLCRuntimeWasm_______

// Export the module if we are in a browser
if (typeof window !== 'undefined') Object.assign(window, { PLCRuntimeWasm: ________PLCRuntimeWasm_______ })