// @ts-check
"use strict"

// This file is used to execute the WebAssembly module (which was compiled from C++ code) to run in Node.js.
// It is used for testing purposes only.

const fs = require("fs")
const path = require("path")
const { performance } = require('perf_hooks');

const delay = ms => new Promise(resolve => setTimeout(resolve, ms))

const timestamp = () => { // "YYYY-MM-DD HH:mm:ss.SSS"
    const date = new Date()
    const year = date.getFullYear().toString().padStart(4, '0')
    const month = (date.getMonth() + 1).toString().padStart(2, '0')
    const day = date.getDate().toString().padStart(2, '0')
    const hour = date.getHours().toString().padStart(2, '0')
    const minute = date.getMinutes().toString().padStart(2, '0')
    const second = date.getSeconds().toString().padStart(2, '0')
    const millisecond = date.getMilliseconds().toString().padStart(3, '0')
    return `${year}-${month}-${day} ${hour}:${minute}:${second}.${millisecond}`
}


let message = ''
const console_print = charcode => {
    const char = String.fromCharCode(charcode)
    if (char === '\0' || char === '\n') {
        console.log(`[${timestamp()}]:`, message)
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
        console.error(`[${timestamp()}]:`, error_message)
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

class Simulator {
    constructor() {
        this.wasmInstance = null
        this.initialized = false
        this.initializing = false
    }

    async init() {
        if (this.initialized) return
        if (this.initializing) {
            let i = 100
            while (!this.initialized && i--) await delay(100)
            if (this.initialized) return
            throw new Error("Simulator initialization timeout")
        }
        this.initializing = true
        // const memory = new WebAssembly.Memory({
        //     initial: 256,
        //     maximum: 512,
        // })
        const wasmImports = {
            env: {
                // memory,
                // memset: (offset, value, size) => {
                //     const memoryBuffer = new Uint8Array(memory.buffer)
                //     for (let i = 0; i < size; i++) {
                //         memoryBuffer[offset + i] = value
                //     }
                // },
                stdout: console_print,
                stderr: console_error,
                streamOut: console_stream,
                millis: () => +performance.now().toFixed(0),
                micros: () => +(performance.now() * 1000).toFixed(0),
            }
        }
        const wasmBuffer = fs.readFileSync(path.join(__dirname, "./simulator.wasm"))
        const wasmModule = await WebAssembly.compile(wasmBuffer)
        this.wasmInstance = await WebAssembly.instantiate(wasmModule, wasmImports)
        console.log("Simulator initialized.")
        this.initialized = true
        this.initializing = false
    }

    getExports() {
        if (!this.wasmInstance) throw new Error("Simulator not initialized")
        return this.wasmInstance.exports
    }

    execute(function_name, ...args) {
        if (!this.wasmInstance) throw new Error("Simulator not initialized")
        const { exports } = this.wasmInstance
        const func = exports[function_name]
        if (!func) throw new Error(`Function '${function_name}' not found in WASM module exports`)
        if (typeof func !== 'function') throw new Error(`'${function_name}' is not a valid function ... it is a ${typeof func}`)
        return func(...args)
    }

    readStream() { return readStream() }
}

const simulator = new Simulator()



module.exports = simulator