// @ts-check
"use strict"

/** 
 * @typedef {{ 
 *     streamIn: (char: number) => boolean
 *     loadAssembly: () => void
 *     compileAssembly: (debug?: boolean) => boolean
 *     loadCompiledProgram: () => boolean
 *     runFullProgram: () => void
 *     runFullProgramDebug: () => void
 *     uploadProgram: () => number
 *     getMemoryLocation: () => number
 *     getMemoryArea: (address: number, size: number) => number
 *     writeMemoryByte: (address: number, byte: number) => number
 *     get_free_memory: () => number
 *     doNothing: () => void
 *     run: () => number
 *     runDirty: () => number
 *     run_unit_test: () => void
 *     run_custom_test: () => void
 *     downloadAssembly: (assembly: string) => boolean
 *     extractProgram: () => { size: number, output: string }
 *     memory: WebAssembly.Memory
 * }} PLCRUntimeWasmExportTypes
*/

class PLCRuntimeWasm_class {
    /** @type { WebAssembly.Instance | null } */
    wasm = null
    /** @type { PLCRUntimeWasmExportTypes | null } */
    wasm_exports
    running = false
    console_message = ''
    error_message = ''
    stream_message = ''

    /** @type { Uint8Array } */
    crc8_table = new Uint8Array(256)
    crc8_table_loaded = false

    stdout_callback = console.log
    stderr_callback = console.error

    constructor() { }

    initialize = async (wasm_path = '', debug = false) => {
        wasm_path = wasm_path || "/simulator.wasm" // "/wasm_test_cases/string_alloc.wasm"
        if (this.running && this.wasm) return this
        this.running = true
        if (debug) console.log("Starting up...")
        /** @type { ArrayBuffer | Buffer } */
        let wasmBuffer
        /** @type { Performance } */
        let perf
        if (typeof window !== 'undefined') { // Browser
            perf = window.performance
            const wasmFile = await fetch(wasm_path)
            wasmBuffer = await wasmFile.arrayBuffer()
        } else { // Node.js
            // @ts-ignore
            perf = (await import('perf_hooks')).performance
            const fs = await import("fs")
            const path = await import("path")
            const __dirname = path.resolve(path.dirname(''), '../')
            const wasm_path = path.join(__dirname, "simulator.wasm")
            wasmBuffer = fs.readFileSync(wasm_path)
        }
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
        this.wasm = wasmInstance // @ts-ignore
        this.wasm_exports = { ...wasmInstance.exports }
        if (!this.wasm_exports) throw new Error("WebAssembly module exports not found")
        this.wasm_exports.downloadAssembly = (assembly) => this.downloadAssembly(assembly)
        this.wasm_exports.extractProgram = () => this.extractProgram()
        const required_methods = ['run_unit_test', 'run_custom_test', 'get_free_memory', 'doNothing', 'compileAssembly', 'loadCompiledProgram', 'runFullProgramDebug', 'runFullProgram', 'uploadProgram', 'getMemoryArea', 'writeMemoryByte']
        for (let i = 0; i < required_methods.length; i++) {
            const method = required_methods[i]
            if (!this.wasm_exports[method]) throw new Error(`${method} function not found`)
        }
        return this
    }

    /** @param { string } assembly */
    downloadAssembly = (assembly) => {
        // Use 'bool streamIn(char)' to download the assembly character by character
        // Use 'void loadAssembly()' to load the assembly into the PLC from the stream buffer
        if (!this.wasm_exports) throw new Error("WebAssembly module not initialized")
        const { streamIn, loadAssembly } = this.wasm_exports
        const translate = {
            // P_On: "u8.const 1",
            // P_Off: "u8.const 0",
            // P_100ms: "u8.readBit 1.0",
            // P_200ms: "u8.readBit 1.1",
            // P_300ms: "u8.readBit 1.2",
            // P_500ms: "u8.readBit 1.3",
            // P_1s: "u8.readBit 1.4",
            // P_2s: "u8.readBit 1.5",
            // P_5s: "u8.readBit 1.6",
            // P_10s: "u8.readBit 1.7",
            // P_30s: "u8.readBit 2.0",
            // P_1min: "u8.readBit 2.1",
            // P_2min: "u8.readBit 2.2",
            // P_5min: "u8.readBit 2.3",
            // P_10min: "u8.readBit 2.4",
            // P_30min: "u8.readBit 2.5",
            // P_1hr: "u8.readBit 2.6",
            // P_2hr: "u8.readBit 2.7",
            // P_3hr: "u8.readBit 3.0",
            // P_4hr: "u8.readBit 3.1",
            // P_5hr: "u8.readBit 3.2",
            // P_6hr: "u8.readBit 3.3",
            // P_12hr: "u8.readBit 3.4",
            // P_1day: "u8.readBit 3.5",
        }
        for (const key in translate) {
            const value = translate[key]
            assembly = assembly.replace(new RegExp(key, 'g'), value)
        }
        let ok = true
        for (let i = 0; i < assembly.length && ok; i++) {
            const char = assembly[i]
            const c = char.charCodeAt(0)
            ok = streamIn(c)
        }
        if (!ok) throw new Error("Failed to download assembly")
        loadAssembly()
        const error = !ok
        return error
    }
    /** @param { string } assembly */
    compile(assembly, run = false) {
        if (!this.wasm_exports) throw new Error("WebAssembly module not initialized")
        if (assembly) this.downloadAssembly(assembly)
        const { compileAssembly, loadCompiledProgram, runFullProgramDebug } = this.wasm_exports
        if (!compileAssembly) throw new Error("'compileAssembly' function not found")
        if (!loadCompiledProgram) throw new Error("'loadCompiledProgram' function not found")
        if (!runFullProgramDebug) throw new Error("'runFullProgramDebug' function not found")
        if (compileAssembly(false)) throw new Error("Failed to compile assembly")
        if (run) {
            loadCompiledProgram()
            this.runDebug()
        }
        return this.extractProgram()
    }

    run = () => {
        if (!this.wasm_exports) throw new Error("WebAssembly module not initialized")
        const { runFullProgram } = this.wasm_exports
        if (!runFullProgram) throw new Error("'runFullProgram' function not found")
        return runFullProgram()
    }
    runDebug = () => {
        if (!this.wasm_exports) throw new Error("WebAssembly module not initialized")
        const { runFullProgramDebug } = this.wasm_exports
        if (!runFullProgramDebug) throw new Error("'runFullProgramDebug' function not found")
        return runFullProgramDebug()
    }


    extractProgram = () => {
        if (!this.wasm_exports) throw new Error("WebAssembly module not initialized")
        const { uploadProgram } = this.wasm_exports
        this.stream_message = ''
        const size = +uploadProgram()
        const output = this.readStream()
        return { size, output }
    }



    /** @type { (address: number, size?: number) => Uint8Array } */
    readMemoryArea = (address, size = 1) => {
        if (!this.wasm_exports) throw new Error("WebAssembly module not initialized")
        // const { getMemoryArea } = this.wasm_exports
        // if (!getMemoryArea) throw new Error("'getMemoryArea' function not found")
        // getMemoryArea(address, size)
        // const output = this.readStream()
        const { memory, getMemoryLocation } = this.wasm_exports
        const offset = getMemoryLocation()
        const buffer = new Uint8Array(memory.buffer, offset + address, size)
        return buffer
    }

    /** @type { (address: number, data: number[]) => string } */
    writeMemoryArea = (address, data) => {
        if (!this.wasm_exports) throw new Error("WebAssembly module not initialized")
        const { writeMemoryByte } = this.wasm_exports
        if (!writeMemoryByte) throw new Error("'writeMemoryByte' function not found")
        for (let i = 0; i < data.length; i++) {
            const byte = data[i] & 0xFF
            const success = writeMemoryByte(address + i, byte)
            if (!success) throw new Error(`Failed to write byte ${byte} at address ${address + i}`)
        }
        const output = this.readStream()
        return output
    }

    getExports = () => {
        if (!this.wasm_exports) throw new Error("WebAssembly module not initialized")
        return this.wasm_exports
    }

    console_print = c => {
        const char = String.fromCharCode(c)
        if (char === '\n') {
            const callback = this.stdout_callback || console.log
            callback(this.console_message)
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
            // console.error(this.error_message)
            const callback = this.stderr_callback || console.error
            callback(this.error_message)
            this.error_message = ''
        } else this.error_message += char
    }

    /** @param { (message: string) => void } callback */
    onStdout = callback => { this.stdout_callback = callback }
    /** @param { (message: string) => void } callback */
    onStderr = callback => { this.stderr_callback = callback }

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

    /** @param { number | number[] } data * @param { number } [crc] */
    crc8 = (data, crc = 0) => {
        if (!this.crc8_table_loaded) {
            this.crc8_table_loaded = true
            for (let i = 0; i < 256; i++) {
                let crc8 = i
                for (let j = 0; j < 8; j++)
                    crc8 = crc8 & 0x80 ? (crc8 << 1) ^ 0x31 : crc8 << 1;
                this.crc8_table[i] = crc8 & 0xff;
            }
        }
        data = Array.isArray(data) ? data : [data];
        const size = data.length;
        for (let i = 0; i < size; i++)
            if (!(data[i] >= 0 && data[i] <= 255)) throw new Error(`Invalid data byte at index ${i}: ${data[i]}`);
        for (let i = 0; i < size; i++) {
            let index = (crc ^ data[i]) & 0xff;
            crc = this.crc8_table[index] & 0xff;
        }
        return crc;
    }

    /** @param { string } hex_string * @returns { number[] } */
    parseHex = hex_string => {
        // Parse 02x formatted HEX string
        if (typeof hex_string !== 'string') throw new Error(`Invalid HEX string: ${hex_string}`);
        if (hex_string.length % 2 !== 0) throw new Error(`Invalid HEX string length: ${hex_string.length}`);
        const hex_array = hex_string.match(/.{1,2}/g);
        if (!hex_array) throw new Error(`Invalid HEX string: ${hex_string}`);
        const num_array = [];
        for (let i = 0; i < hex_array.length; i++) {
            const num = parseInt(hex_array[i], 16);
            if (num < 0 || num > 255) throw new Error(`Invalid HEX string byte at index ${i}: ${hex_array[i]}`);
            num_array.push(num);
        }
        return num_array;
    }
    /** @param { string } str * @returns { string } */
    stringToHex = str => str.split('').map(c => c.charCodeAt(0).toString(16).padStart(2, '0')).join('')

    //  - PLC reset:        'RS<u8>' (checksum)
    //  - Program download: 'PD<u32><u8[]><u8>' (size, data, checksum)
    //  - Program upload:   'PU<u8>' (checksum)
    //  - Program run:      'PR<u8>' (checksum)
    //  - Program stop:     'PS<u8>' (checksum)
    //  - Memory read:      'MR<u32><u32><u8>' (address, size, checksum)
    //  - Memory write:     'MW<u32><u32><u8[]><u8>' (address, size, data, checksum)
    //  - Memory format:    'MF<u32><u32><u8><u8>' (address, size, value, checksum)
    //  - Source download:  'SD<u32><u8[]><u8>' (size, data, checksum) // Only available if PLCRUNTIME_SOURCE_ENABLED is defined
    //  - Source upload:    'SU<u32><u8>' (size, checksum) // Only available if PLCRUNTIME_SOURCE_ENABLED is defined
    buildCommand = {

        /** @returns { string } */
        plcReset: () => {
            const cmd = "RS"
            const cmd_hex = this.stringToHex(cmd)
            const checksum = this.crc8(this.parseHex(cmd_hex))
            const checksum_hex = checksum.toString(16).padStart(2, '0');
            const command = cmd + checksum_hex;
            return command;
        },

        /** @param { number[] | [number[]] | [string] } input * @returns { string } */
        programDownload: (...input) => {
            const cmd = "PD"
            const cmd_hex = this.stringToHex(cmd)
            let checksum = this.crc8(this.parseHex(cmd_hex))
            input = Array.isArray(input[0]) ? input[0] : input
            const allowedChars = '0123456789abcdefABCDEF'
            if (typeof input[0] === 'string') input = this.parseHex(input[0].split('').filter(c => allowedChars.includes(c)).join('') || '')
            /** @type { number[] } */// @ts-ignore
            const data = input
            const data_hex = data.map(d => d.toString(16).padStart(2, '0'))
            const size = data.length;
            const size_hex_u32 = size.toString(16).padStart(8, '0');
            checksum = this.crc8(this.parseHex(size_hex_u32), checksum)
            checksum = this.crc8(data, checksum)
            const checksum_hex = checksum.toString(16).padStart(2, '0');
            const command = cmd + size_hex_u32 + data_hex.join('') + checksum_hex;
            return command;
        },

        /** @returns { string } */
        programUpload: () => {
            const cmd = "PU"
            const cmd_hex = this.stringToHex(cmd)
            const checksum = this.crc8(this.parseHex(cmd_hex))
            const checksum_hex = checksum.toString(16).padStart(2, '0');
            const command = cmd + checksum_hex;
            return command;
        },

        /** @returns { string } */
        programRun: () => {
            const cmd = "PR"
            const cmd_hex = this.stringToHex(cmd)
            const checksum = this.crc8(this.parseHex(cmd_hex))
            const checksum_hex = checksum.toString(16).padStart(2, '0');
            const command = cmd + checksum_hex;
            return command;
        },

        /** @returns { string } */
        programStop: () => {
            const cmd = "PS"
            const cmd_hex = this.stringToHex(cmd)
            const checksum = this.crc8(this.parseHex(cmd_hex))
            const checksum_hex = checksum.toString(16).padStart(2, '0');
            const command = cmd + checksum_hex;
            return command;
        },

        /** @param { number } address * @param { number } size * @returns { string } */
        memoryRead: (address, size = 1) => {
            const cmd = "MR"
            const cmd_hex = this.stringToHex(cmd)
            const address_hex_u32 = address.toString(16).padStart(8, '0');
            const size_hex_u32 = size.toString(16).padStart(8, '0');
            let checksum = this.crc8(this.parseHex(cmd_hex))
            checksum = this.crc8(this.parseHex(address_hex_u32), checksum)
            checksum = this.crc8(this.parseHex(size_hex_u32), checksum)
            const checksum_hex = checksum.toString(16).padStart(2, '0');
            const command = cmd + address_hex_u32 + size_hex_u32 + checksum_hex;
            return command;
        },

        /** @param { number } address * @param { number[] | [number[]] | [string] } input * @returns { string } */
        memoryWrite: (address, input) => {
            const cmd = "MW"
            const cmd_hex = this.stringToHex(cmd)
            let checksum = this.crc8(this.parseHex(cmd_hex))
            const address_hex_u32 = address.toString(16).padStart(8, '0');
            checksum = this.crc8(this.parseHex(address_hex_u32), checksum)
            input = Array.isArray(input[0]) ? input[0] : input
            const allowedChars = '0123456789abcdefABCDEF'
            if (typeof input[0] === 'string') input = this.parseHex(input[0].split('').filter(c => allowedChars.includes(c)).join('') || '')
            /** @type { number[] } */// @ts-ignore
            const data = input
            const data_hex = data.map(d => d.toString(16).padStart(2, '0')).join('')
            const size = data.length;
            const size_hex_u32 = size.toString(16).padStart(8, '0');
            checksum = this.crc8(this.parseHex(size_hex_u32), checksum)
            checksum = this.crc8(data, checksum)
            const checksum_hex = checksum.toString(16).padStart(2, '0');
            const command = cmd + address_hex_u32 + size_hex_u32 + data_hex + checksum_hex;
            return command;
        },

        /** @param { number } address * @param { number } size * @param { number } value * @returns { string } */
        memoryFormat: (address, size, value) => {
            const cmd = "MF"
            const cmd_hex = this.stringToHex(cmd)
            const address_hex_u32 = address.toString(16).padStart(8, '0');
            const size_hex_u32 = size.toString(16).padStart(8, '0');
            const value_hex = value.toString(16).padStart(2, '0');
            let checksum = this.crc8(this.parseHex(cmd_hex))
            checksum = this.crc8(this.parseHex(address_hex_u32), checksum)
            checksum = this.crc8(this.parseHex(size_hex_u32), checksum)
            checksum = this.crc8(this.parseHex(value_hex), checksum)
            const checksum_hex = checksum.toString(16).padStart(2, '0');
            const command = cmd + address_hex_u32 + size_hex_u32 + value_hex + checksum_hex;
            return command;
        },

    }

}

const PLCRuntimeWasm = new PLCRuntimeWasm_class()

// Export the module if we are in a browser
if (typeof window !== 'undefined') {
    console.log(`WASM exported as window object`)
    Object.assign(window, { PLCRuntimeWasm })
}

// Export for CommonJS modules
if (typeof module !== 'undefined') {
    console.log(`WASM exported as module`)
    module.exports = PLCRuntimeWasm
}
// Export for ES modules
export default PLCRuntimeWasm;