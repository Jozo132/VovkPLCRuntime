// @ts-check
"use strict"

class PLCRuntimeWasm_class {
    /** @type { WebAssembly.Instance | null } */
    wasm = null
    running = false
    console_message = ''
    error_message = ''
    stream_message = ''

    /** @type { Uint8Array } */
    crc8_table = new Uint8Array(256)
    crc8_table_loaded = false

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
        return this
    }

    /** @param { string } assembly */
    downloadAssembly = (assembly) => {
        // Use 'bool streamIn(char)' to download the assembly character by character
        // Use 'void loadAssembly()' to load the assembly into the PLC from the stream buffer
        if (!this.wasm) throw new Error("WebAssembly module not initialized")
        const { streamIn, loadAssembly } = this.wasm.exports
        const translate = {
            P_On: "u8.const 1",
            P_Off: "u8.const 0",
            P_100ms: "u8.readBit.0 1",
            P_200ms: "u8.readBit.1 1",
            P_300ms: "u8.readBit.2 1",
            P_500ms: "u8.readBit.3 1",
            P_1s: "u8.readBit.4 1",
            P_2s: "u8.readBit.5 1",
            P_5s: "u8.readBit.6 1",
            P_10s: "u8.readBit.7 1",
            P_30s: "u8.readBit.0 2",
            P_1min: "u8.readBit.1 2",
            P_2min: "u8.readBit.2 2",
            P_5min: "u8.readBit.3 2",
            P_10min: "u8.readBit.4 2",
            P_30min: "u8.readBit.5 2",
            P_1hr: "u8.readBit.6 2",
            P_2hr: "u8.readBit.7 2",
            P_3hr: "u8.readBit.0 3",
            P_4hr: "u8.readBit.1 3",
            P_5hr: "u8.readBit.2 3",
            P_6hr: "u8.readBit.3 3",
            P_12hr: "u8.readBit.4 3",
            P_1day: "u8.readBit.5 3",
        }
        for (const key in translate) {
            const value = translate[key]
            assembly = assembly.replace(new RegExp(key, 'g'), value)
        }
        let ok = true
        for (let i = 0; i < assembly.length && ok; i++) {
            const char = assembly[i]
            const c = char.charCodeAt(0) // @ts-ignore
            ok = streamIn(c)
        }
        if (!ok) throw new Error("Failed to download assembly") // @ts-ignore
        loadAssembly()
    }

    extractProgram = () => {
        if (!this.wasm) throw new Error("WebAssembly module not initialized")
        const { uploadProgram } = this.wasm.exports
        if (!uploadProgram) throw new Error("'uploadProgram' function not found")
        this.stream_message = '' // @ts-ignore
        const size = uploadProgram()
        const output = this.readStream()
        return { size, output }
    }

    getExports = () => {
        if (!this.wasm) throw new Error("WebAssembly module not initialized")
        return Object.assign({}, this.wasm.exports, {
            downloadAssembly: this.downloadAssembly,
            extractProgram: this.extractProgram,
        })
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

    //  - PLC reset:        'RS<uint8_t>' (checksum)
    //  - Program download: 'PD<uint32_t><uint8_t[]><uint8_t>' (size, data, checksum)
    //  - Program upload:   'PU<uint8_t>' (checksum)
    //  - Program run:      'PR<uint8_t>' (checksum)
    //  - Program stop:     'PS<uint8_t>' (checksum)
    //  - Memory read:      'MR<uint32_t><uint32_t><uint8_t>' (address, size, checksum)
    //  - Memory write:     'MW<uint32_t><uint32_t><uint8_t[]><uint8_t>' (address, size, data, checksum)
    //  - Memory format:    'MF<uint32_t><uint32_t><uint8_t><uint8_t>' (address, size, value, checksum)
    //  - Source download:  'SD<uint32_t><uint8_t[]><uint8_t>' (size, data, checksum) // Only available if PLCRUNTIME_SOURCE_ENABLED is defined
    //  - Source upload:    'SU<uint32_t><uint8_t>' (size, checksum) // Only available if PLCRUNTIME_SOURCE_ENABLED is defined
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

const ________PLCRuntimeWasm_______ = new PLCRuntimeWasm_class()

/** @typedef { PLCRuntimeWasm_class } PLCRuntimeWasm */

// Export the module if we are in Node.js
if (typeof module !== 'undefined') module.exports = ________PLCRuntimeWasm_______

// Export the module if we are in a browser
if (typeof window !== 'undefined') Object.assign(window, { PLCRuntimeWasm: ________PLCRuntimeWasm_______ })