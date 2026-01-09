// @ts-check
'use strict'

const isNodeRuntime = typeof process !== 'undefined' && !!(process.versions && process.versions.node)

/**
 * @typedef {{
 *     initialize: () => void
 *     printInfo: () => void
 *     streamIn: (char: number) => boolean
 *     loadAssembly: () => void
 *     downloadProgram: (size: number, crc: number) => number
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
 *     setMillis: (millis: number) => void
 *     setMicros: (millis: number) => void
 *     getMillis: () => number
 *     getMicros: () => number
 *     run: () => number
 *     runDirty: () => number
 *     run_unit_test: () => void
 *     run_custom_test: () => void
 *     downloadAssembly: (assembly: string) => boolean
 *     extractProgram: () => { size: number, output: string }
 *     memory: WebAssembly.Memory
 *     lint_load_assembly: () => void
 *     lint_run: () => void
 *     lint_get_problem_count: () => number
 *     lint_get_problems_pointer: () => number
 * }} VovkPLCExportTypes
 */

/** @typedef */ // @ts-ignore
class VovkPLC_class {
    /** @type { WebAssembly.Instance | null } */
    wasm = null
    /** @type { VovkPLCExportTypes | null } */
    wasm_exports = null
    /** @type { any } */
    wasmImports
    running = false
    silent = false
    console_message = ''
    error_message = ''
    stream_message = ''
    /** @type { Performance | null } */
    perf = null

    /** @type { Uint8Array } */
    crc8_table = new Uint8Array(256)
    crc8_table_loaded = false

    stdout_callback = console.log
    stderr_callback = console.error

    constructor(wasm_path = '') {
        this.wasm_path = wasm_path
    }

    initialize = async (wasm_path = '', debug = false) => {
        wasm_path = wasm_path || this.wasm_path || '/dist/VovkPLC.wasm' // "/wasm_test_cases/string_alloc.wasm"
        this.wasm_path = wasm_path
        if (this.running && this.wasm) return this
        this.running = true
        if (debug) console.log('Starting up...')
        /** @type { BufferSource | null } */
        let wasmBuffer = null
        if (!isNodeRuntime) {
            this.perf = globalThis.performance || null
            const wasmFile = await fetch(wasm_path)
            wasmBuffer = await wasmFile.arrayBuffer()
        } else {
            // @ts-ignore
            this.perf = globalThis.performance || (await import('perf_hooks')).performance
            const fs = await import('fs')
            const path = await import('path')
            const {fileURLToPath} = await import('url')
            let resolvedPath = wasm_path
            if (!resolvedPath || resolvedPath === '/dist/VovkPLC.wasm') {
                resolvedPath = fileURLToPath(new URL('./VovkPLC.wasm', import.meta.url))
            } else if (resolvedPath.startsWith('file:')) {
                resolvedPath = fileURLToPath(resolvedPath)
            } else if (resolvedPath.startsWith('http://') || resolvedPath.startsWith('https://')) {
                const wasmFile = await fetch(resolvedPath)
                wasmBuffer = await wasmFile.arrayBuffer()
            }
            if (!wasmBuffer) {
                wasmBuffer = fs.readFileSync(path.resolve(resolvedPath))
            }
        }
        this.wasmImports = {
            env: {
                stdout: this.console_print,
                stderr: this.console_error,
                streamOut: this.console_stream, // @ts-ignore
                millis: () => Math.round(this.perf.now()), // @ts-ignore
                micros: () => Math.round(this.perf.now() * 1000),
                // memory: new WebAssembly.Memory({ initial: 256, maximum: 512 }),
            },
        }
        const wasmModule = await WebAssembly.compile(wasmBuffer)
        const wasmInstance = await WebAssembly.instantiate(wasmModule, this.wasmImports)
        if (typeof window !== 'undefined') Object.assign(window, wasmInstance.exports) // Assign all exports to the global scope
        if (!wasmInstance) throw new Error('Failed to instantiate WebAssembly module')
        this.wasm = wasmInstance // @ts-ignore
        this.wasm_exports = {...wasmInstance.exports}
        if (!this.wasm_exports) throw new Error('WebAssembly module exports not found')
        this.wasm_exports.initialize()
        this.wasm_exports.downloadAssembly = assembly => this.downloadAssembly(assembly)
        this.wasm_exports.extractProgram = () => this.extractProgram()
        const required_methods = ['printInfo', 'run_unit_test', 'run_custom_test', 'get_free_memory', 'doNothing', 'compileAssembly', 'loadCompiledProgram', 'runFullProgramDebug', 'runFullProgram', 'uploadProgram', 'getMemoryArea', 'writeMemoryByte']
        for (let i = 0; i < required_methods.length; i++) {
            const method = required_methods[i] // @ts-ignore
            if (!this.wasm_exports[method]) throw new Error(`${method} function not found`)
        }
        return this
    }

    /**
     * @typedef {{
     *     type: 'error' | 'warning' | 'info',
     *     line: number,
     *     column: number,
     *     length: number,
     *     message: string,
     *     token_text: string
     * }} LinterProblem
     */

    /** @param { string } assembly * @param { boolean } [debug=false] * @returns { LinterProblem[] } */
    lint = (assembly, debug = false) => {
        if (!this.wasm_exports) throw new Error('WebAssembly module not initialized')
        if (!this.wasm_exports.lint_load_assembly) throw new Error("'lint_load_assembly' function not found")
        if (!this.wasm_exports.lint_run) throw new Error("'lint_run' function not found")
        if (!this.wasm_exports.lint_get_problem_count) throw new Error("'lint_get_problem_count' function not found")
        if (!this.wasm_exports.lint_get_problems_pointer) throw new Error("'lint_get_problems_pointer' function not found")

        const wasSilent = this.silent

        // Temporarily disable stream output unless debug is enabled
        if (!debug) {
            this.setSilent(true)
        }

        try {
            // 1. Download assembly to Linter
            let ok = true
            for (let i = 0; i < assembly.length && ok; i++) {
                const char = assembly[i]
                const c = char.charCodeAt(0)
                ok = this.wasm_exports.streamIn(c)
            }
            if (!ok) throw new Error('Failed to stream assembly')
            this.wasm_exports.lint_load_assembly()

            // 2. Run Linter
            this.wasm_exports.lint_run()

            // 3. Get results
            const count = this.wasm_exports.lint_get_problem_count()
            if (count === 0) return []

            const pointer = this.wasm_exports.lint_get_problems_pointer()
            /** @type { LinterProblem[] } */
            const problems = []

            // Struct size = 84 bytes  (4+4+4+4+64+4)
            const struct_size = 84

            // Access memory directly
            const memoryBuffer = this.wasm_exports.memory.buffer
            const view = new DataView(memoryBuffer)

            for (let i = 0; i < count; i++) {
                const offset = pointer + i * struct_size
                const type_int = view.getUint32(offset + 0, true)
                const line = view.getUint32(offset + 4, true)
                const column = view.getUint32(offset + 8, true)
                const length = view.getUint32(offset + 12, true)

                // token_text is 64 bytes at offset 16
                let message = ''
                for (let j = 0; j < 64; j++) {
                    const charCode = view.getUint8(offset + 16 + j)
                    if (charCode === 0) break
                    message += String.fromCharCode(charCode)
                }

                const token_ptr = view.getUint32(offset + 80, true)
                let token_text = ''
                if (token_ptr !== 0 && length > 0) {
                    const token_buf = new Uint8Array(memoryBuffer, token_ptr, length)
                    token_text = new TextDecoder().decode(token_buf)
                }

                problems.push({
                    type: type_int === 2 ? 'error' : type_int === 1 ? 'warning' : 'info',
                    line,
                    column,
                    length,
                    message,
                    token_text,
                })
            }

            return problems
        } finally {
            // Restore original stream callback
            this.setSilent(wasSilent)
            // Clear any accumulated stream output
            if (!debug) {
                this.readStream()
            }
        }
    }

    setSilent = (value = true) => {
        this.silent = value
    }

    printInfo = () => {
        if (!this.wasm_exports) throw new Error('WebAssembly module not initialized')
        if (!this.wasm_exports.printInfo) throw new Error("'printInfo' function not found")
        this.wasm_exports.printInfo()
        const raw = this.readStream().trim()
        if (raw.length === 0) return 'No info available'
        if (raw.startsWith('[') && raw.endsWith(']')) {
            // '[VovkPLCRuntime,WASM,0,1,0,324,2025-03-16 19:16:44,1024,104857,104857,16,16,32,16,Simulator]'
            const content = raw.substring(1, raw.length - 1)
            const parts = content.split(',')
            const info = {
                header: parts[0],
                arch: parts[1],
                version: `${parts[2]}.${parts[3]}.${parts[4]} Build ${parts[5]}`,
                date: parts[6],
                stack: +parts[7],
                memory: +parts[8],
                program: +parts[9],
                input_offset: +parts[10],
                input_size: +parts[11],
                output_offset: +parts[12],
                output_size: +parts[13],
                device: parts[14],
            }
            return info
        }
        console.error(`Invalid info response:`, raw)
        return raw
        // return raw
    }

    /** @param { string } assembly */
    downloadAssembly = assembly => {
        // Use 'bool streamIn(char)' to download the assembly character by character
        // Use 'void loadAssembly()' to load the assembly into the PLC from the stream buffer
        if (!this.wasm_exports) throw new Error('WebAssembly module not initialized')
        if (!this.wasm_exports.streamIn) throw new Error("'streamIn' function not found")
        if (!this.wasm_exports.loadAssembly) throw new Error("'loadAssembly' function not found")
        let ok = true
        for (let i = 0; i < assembly.length && ok; i++) {
            const char = assembly[i]
            const c = char.charCodeAt(0)
            ok = this.wasm_exports.streamIn(c)
        }
        if (!ok) throw new Error('Failed to download assembly')
        this.wasm_exports.loadAssembly()
        const error = !ok
        return error
    }
    /** @param { string } assembly */
    compile(assembly, run = false) {
        if (!this.wasm_exports) throw new Error('WebAssembly module not initialized')
        if (assembly) this.downloadAssembly(assembly)
        if (!this.wasm_exports.compileAssembly) throw new Error("'compileAssembly' function not found")
        if (!this.wasm_exports.loadCompiledProgram) throw new Error("'loadCompiledProgram' function not found")
        if (this.wasm_exports.compileAssembly(false)) throw new Error('Failed to compile assembly')
        if (run) {
            // this.updateTime()
            this.wasm_exports.loadCompiledProgram()
            this.runDebug()
        }
        return this.extractProgram()
    }

    /** @param { string | number[] } program */
    downloadBytecode = program => {
        if (!this.wasm_exports) throw new Error('WebAssembly module not initialized')
        if (!this.wasm_exports.streamIn) throw new Error("'streamIn' function not found")
        if (!this.wasm_exports.downloadProgram) throw new Error("'downloadProgram' function not found")
        const code = Array.isArray(program) ? program : this.parseHex(program)
        const size = code.length
        const crc = this.crc8(code)
        for (let i = 0; i < size; i++) {
            const c = code[i]
            this.wasm_exports.streamIn(c)
        }
        const error = this.wasm_exports.downloadProgram(size, crc)
        if (error === 1) throw new Error('Failed to download program -> size mismatch')
        if (error === 2) throw new Error('Failed to download program -> checksum mismatch')
        return error
    }

    run = () => {
        if (!this.wasm_exports) throw new Error('WebAssembly module not initialized')
        if (!this.wasm_exports.run) throw new Error("'runFullProgram' function not found")
        // this.updateTime() // Update millis and micros
        return this.wasm_exports.run()
    }
    runDebug = () => {
        if (!this.wasm_exports) throw new Error('WebAssembly module not initialized')
        if (!this.wasm_exports.runFullProgramDebug) throw new Error("'runFullProgramDebug' function not found")
        // this.updateTime() // Update millis and micros
        return this.wasm_exports.runFullProgramDebug()
    }

    extractProgram = () => {
        if (!this.wasm_exports) throw new Error('WebAssembly module not initialized')
        if (!this.wasm_exports.uploadProgram) throw new Error("'uploadProgram' function not found")
        this.stream_message = ''
        const size = +this.wasm_exports.uploadProgram()
        const output = this.readStream()
        return {size, output}
    }

    /** @type { (address: number, size?: number) => Uint8Array } */
    readMemoryArea = (address, size = 1) => {
        if (!this.wasm_exports) throw new Error('WebAssembly module not initialized')
        // const { getMemoryArea } = this.wasm_exports
        // if (!getMemoryArea) throw new Error("'getMemoryArea' function not found")
        // getMemoryArea(address, size)
        // const output = this.readStream()
        // const { memory, getMemoryLocation } = this.wasm_exports
        if (!this.wasm_exports.memory) throw new Error('WebAssembly memory not found')
        if (!this.wasm_exports.getMemoryLocation) throw new Error("'getMemoryLocation' function not found")
        const offset = this.wasm_exports.getMemoryLocation()
        const buffer = new Uint8Array(this.wasm_exports.memory.buffer, offset + address, size)
        return buffer
    }

    /** @type { (address: number, data: number[]) => string } */
    writeMemoryArea = (address, data) => {
        if (!this.wasm_exports) throw new Error('WebAssembly module not initialized')
        if (!this.wasm_exports.writeMemoryByte) throw new Error("'writeMemoryByte' function not found")
        for (let i = 0; i < data.length; i++) {
            const byte = data[i] & 0xff
            const success = this.wasm_exports.writeMemoryByte(address + i, byte)
            if (!success) throw new Error(`Failed to write byte ${byte} at address ${address + i}`)
        }
        const output = this.readStream()
        return output
    }

    getExports = () => {
        if (!this.wasm_exports) throw new Error('WebAssembly module not initialized')
        return this.wasm_exports
    }

    /** @type { (name: string, ...args: any[]) => any } */
    callExport = (name, ...args) => {
        if (!this.wasm_exports) throw new Error('WebAssembly module not initialized') // @ts-ignore
        const fn = this.wasm_exports[name]
        if (typeof fn !== 'function') throw new Error(`'${name}' export not found`)
        return fn(...args)
    }

    /** @type { (charcode: number) => void } */
    console_print = c => {
        const char = String.fromCharCode(c)
        if (char === '\n') {
            const callback = this.stdout_callback || console.log
            if (this.console_message && this.console_message.length > 0 && !this.silent) callback(this.console_message)
            this.console_message = ''
        } else {
            this.console_message += char
        }
    }

    /** @type { (charcode: number | number[] | string) => void } */
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
    onStdout = callback => {
        this.stdout_callback = callback
    }
    /** @param { (message: string) => void } callback */
    onStderr = callback => {
        this.stderr_callback = callback
    }

    /** @type { (charcode: number | number[] | string) => void } */
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
                for (let j = 0; j < 8; j++) crc8 = crc8 & 0x80 ? (crc8 << 1) ^ 0x31 : crc8 << 1
                this.crc8_table[i] = crc8 & 0xff
            }
        }
        data = Array.isArray(data) ? data : [data]
        const size = data.length
        for (let i = 0; i < size; i++) if (!(data[i] >= 0 && data[i] <= 255)) throw new Error(`Invalid data byte at index ${i}: ${data[i]}`)
        for (let i = 0; i < size; i++) {
            /** @type { number } */
            let index = (crc ^ data[i]) & 0xff
            crc = this.crc8_table[index] & 0xff
        }
        return crc
    }

    /** @param { string } hex_string * @returns { number[] } */
    parseHex = hex_string => {
        // Parse 02x formatted HEX string
        if (typeof hex_string !== 'string') throw new Error(`Invalid HEX string: ${hex_string}`)
        hex_string = hex_string.replace(/[^0-9a-fA-F]/g, '')
        if (hex_string.length % 2 !== 0) throw new Error(`Invalid HEX string length: ${hex_string.length}`)
        const hex_array = hex_string.match(/.{1,2}/g)
        if (!hex_array) throw new Error(`Invalid HEX string: ${hex_string}`)
        const num_array = []
        for (let i = 0; i < hex_array.length; i++) {
            const num = parseInt(hex_array[i], 16)
            if (num < 0 || num > 255) throw new Error(`Invalid HEX string byte at index ${i}: ${hex_array[i]}`)
            num_array.push(num)
        }
        return num_array
    }
    /** @param { string } str * @returns { string } */
    stringToHex = str =>
        str
            .split('')
            .map(c => c.charCodeAt(0).toString(16).padStart(2, '0'))
            .join('')

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
            const cmd = 'RS'
            const cmd_hex = this.stringToHex(cmd)
            const checksum = this.crc8(this.parseHex(cmd_hex))
            const checksum_hex = checksum.toString(16).padStart(2, '0')
            const command = cmd + checksum_hex
            return command
        },

        /** @param { number[] | [number[]] | [string] } input * @returns { string } */
        programDownload: (...input) => {
            const cmd = 'PD'
            const cmd_hex = this.stringToHex(cmd)
            let checksum = this.crc8(this.parseHex(cmd_hex))
            input = Array.isArray(input[0]) ? input[0] : input
            const allowedChars = '0123456789abcdefABCDEF'
            if (typeof input[0] === 'string')
                input = this.parseHex(
                    input[0]
                        .split('')
                        .filter(c => allowedChars.includes(c))
                        .join('') || ''
                )
            /** @type { number[] } */ // @ts-ignore
            const data = input
            const data_hex = data.map(d => d.toString(16).padStart(2, '0'))
            const size = data.length
            const size_hex_u32 = size.toString(16).padStart(8, '0')
            checksum = this.crc8(this.parseHex(size_hex_u32), checksum)
            checksum = this.crc8(data, checksum)
            const checksum_hex = checksum.toString(16).padStart(2, '0')
            const command = cmd + size_hex_u32 + data_hex.join('') + checksum_hex
            return command.toUpperCase()
        },

        /** @returns { string } */
        programUpload: () => {
            const cmd = 'PU'
            const cmd_hex = this.stringToHex(cmd)
            const checksum = this.crc8(this.parseHex(cmd_hex))
            const checksum_hex = checksum.toString(16).padStart(2, '0')
            const command = cmd + checksum_hex
            return command
        },

        /** @returns { string } */
        programRun: () => {
            const cmd = 'PR'
            const cmd_hex = this.stringToHex(cmd)
            const checksum = this.crc8(this.parseHex(cmd_hex))
            const checksum_hex = checksum.toString(16).padStart(2, '0')
            const command = cmd + checksum_hex
            return command
        },

        /** @returns { string } */
        programStop: () => {
            const cmd = 'PS'
            const cmd_hex = this.stringToHex(cmd)
            const checksum = this.crc8(this.parseHex(cmd_hex))
            const checksum_hex = checksum.toString(16).padStart(2, '0')
            const command = cmd + checksum_hex
            return command
        },

        /** @param { number } address * @param { number } size * @returns { string } */
        memoryRead: (address, size = 1) => {
            const cmd = 'MR'
            const cmd_hex = this.stringToHex(cmd)
            const address_hex_u32 = address.toString(16).padStart(8, '0')
            const size_hex_u32 = size.toString(16).padStart(8, '0')
            let checksum = this.crc8(this.parseHex(cmd_hex))
            checksum = this.crc8(this.parseHex(address_hex_u32), checksum)
            checksum = this.crc8(this.parseHex(size_hex_u32), checksum)
            const checksum_hex = checksum.toString(16).padStart(2, '0')
            const command = cmd + address_hex_u32 + size_hex_u32 + checksum_hex
            return command
        },

        /** @param { number } address * @param { number[] | [number[]] | [string] } input * @returns { string } */
        memoryWrite: (address, input) => {
            const cmd = 'MW'
            const cmd_hex = this.stringToHex(cmd)
            let checksum = this.crc8(this.parseHex(cmd_hex))
            const address_hex_u32 = address.toString(16).padStart(8, '0')
            checksum = this.crc8(this.parseHex(address_hex_u32), checksum)
            input = Array.isArray(input[0]) ? input[0] : input
            const allowedChars = '0123456789abcdefABCDEF'
            if (typeof input[0] === 'string')
                input = this.parseHex(
                    input[0]
                        .split('')
                        .filter(c => allowedChars.includes(c))
                        .join('') || ''
                )
            /** @type { number[] } */ // @ts-ignore
            const data = input
            const data_hex = data.map(d => d.toString(16).padStart(2, '0')).join('')
            const size = data.length
            const size_hex_u32 = size.toString(16).padStart(8, '0')
            checksum = this.crc8(this.parseHex(size_hex_u32), checksum)
            checksum = this.crc8(data, checksum)
            const checksum_hex = checksum.toString(16).padStart(2, '0')
            const command = cmd + address_hex_u32 + size_hex_u32 + data_hex + checksum_hex
            return command
        },

        /** @param { number } address * @param { number } size * @param { number } value * @returns { string } */
        memoryFormat: (address, size, value) => {
            const cmd = 'MF'
            const cmd_hex = this.stringToHex(cmd)
            const address_hex_u32 = address.toString(16).padStart(8, '0')
            const size_hex_u32 = size.toString(16).padStart(8, '0')
            const value_hex = value.toString(16).padStart(2, '0')
            let checksum = this.crc8(this.parseHex(cmd_hex))
            checksum = this.crc8(this.parseHex(address_hex_u32), checksum)
            checksum = this.crc8(this.parseHex(size_hex_u32), checksum)
            checksum = this.crc8(this.parseHex(value_hex), checksum)
            const checksum_hex = checksum.toString(16).padStart(2, '0')
            const command = cmd + address_hex_u32 + size_hex_u32 + value_hex + checksum_hex
            return command
        },
    }
}

/**
 * @typedef {{
 *   postMessage: (message: any) => void,
 *   terminate?: () => any,
 *   addEventListener?: (type: string, listener: (event: any) => void) => void,
 *   removeEventListener?: (type: string, listener: (event: any) => void) => void,
 *   on?: (type: string, listener: (event: any) => void) => void,
 *   off?: (type: string, listener: (event: any) => void) => void,
 *   removeListener?: (type: string, listener: (event: any) => void) => void,
 * }} VovkPLCWorkerLike
 */

/**
 * @typedef {(url: URL | string) => VovkPLCWorkerLike | Promise<VovkPLCWorkerLike>} VovkPLCWorkerFactory
 */

/**
 * @typedef {{ workerUrl?: URL | string, workerFactory?: VovkPLCWorkerFactory, debug?: boolean }} VovkPLCWorkerOptions
 */

/**
 * @typedef {{ resolve: (value: any) => void, reject: (reason?: any) => void }} VovkPLCPendingRequest
 */

class VovkPLCWorkerInstance {
    /** @type { VovkPLCWorkerClient } */
    client
    /** @type { number } */
    id
    /** @param { VovkPLCWorkerClient } client * @param { number } id */
    constructor(client, id) {
        this.client = client
        this.id = id
    }
    /** @type { (method: string, ...args: any[]) => Promise<any> } */
    call = (method, ...args) => this.client.callInstance(this.id, method, ...args)
    /** @type { (name: string, ...args: any[]) => Promise<any> } */
    callExport = (name, ...args) => this.client.callInstance(this.id, 'callExport', name, ...args)
    /** @type { () => Promise<string[]> } */
    getExports = () => this.client.callInstance(this.id, 'getExports')
    /** @type { (callback: (message: string, instanceId?: number) => void) => Promise<any> } */
    onStdout = callback => this.client.onStdout(callback, this.id)
    /** @type { (callback: (message: string, instanceId?: number) => void) => Promise<any> } */
    onStderr = callback => this.client.onStderr(callback, this.id)
    /** @type { () => Promise<any> } */
    dispose = () => this.client.disposeInstance(this.id)
}

class VovkPLCWorkerClient {
    /** @type { VovkPLCWorkerLike } */
    worker
    /** @type { number } */
    nextId = 1
    /** @type { Map<number, VovkPLCPendingRequest> } */
    pending = new Map()
    /** @type { Map<string | number, (message: string, instanceId?: number) => void> } */
    stdoutHandlers = new Map()
    /** @type { Map<string | number, (message: string, instanceId?: number) => void> } */
    stderrHandlers = new Map()
    /** @type { (message: any) => void } */
    _onMessage
    /** @type { (error: any) => void } */
    _onError
    /** @param { VovkPLCWorkerLike } worker */
    constructor(worker) {
        this.worker = worker
        this._onMessage = this._handleMessage.bind(this)
        this._onError = this._handleError.bind(this)
        if (typeof worker.addEventListener === 'function') {
            worker.addEventListener('message', this._onMessage)
            worker.addEventListener('error', this._onError)
        } else if (typeof worker.on === 'function') {
            worker.on('message', this._onMessage)
            worker.on('error', this._onError)
        }
    }

    /** @type { (message: any) => void } */
    _handleMessage = message => {
        const data = message && message.data !== undefined ? message.data : message
        if (!data || typeof data !== 'object') return
        if (data.type === 'event') {
            this._handleEvent(data)
            return
        }
        if (typeof data.id !== 'number') return
        const pending = this.pending.get(data.id)
        if (!pending) return
        this.pending.delete(data.id)
        if (data.ok) pending.resolve(data.result)
        else pending.reject(new Error(data.error || 'Worker call failed'))
    }

    /** @type { (data: any) => void } */
    _handleEvent = data => {
        if (!data || typeof data.event !== 'string') return
        const key = data.instanceId == null ? 'default' : data.instanceId
        if (data.event === 'stdout') {
            const handler = this.stdoutHandlers.get(key) || this.stdoutHandlers.get('default')
            if (handler) handler(data.message, data.instanceId)
            return
        }
        if (data.event === 'stderr') {
            const handler = this.stderrHandlers.get(key) || this.stderrHandlers.get('default')
            if (handler) handler(data.message, data.instanceId)
        }
    }

    /** @type { (error: any) => void } */
    _handleError = error => {
        const message = error && error.message ? error.message : String(error)
        const err = error instanceof Error ? error : new Error(message)
        for (const pending of this.pending.values()) pending.reject(err)
        this.pending.clear()
    }

    /** @type { (type: string, payload?: Record<string, any>) => Promise<any> } */
    _send = (type, payload = {}) => {
        const id = this.nextId++
        return new Promise((resolve, reject) => {
            this.pending.set(id, {resolve, reject})
            this.worker.postMessage({id, type, ...payload})
        })
    }

    /** @type { (stream: 'stdout' | 'stderr', callback: (message: string, instanceId?: number) => void, instanceId?: number | null) => Promise<any> } */
    _setStreamHandler = (stream, callback, instanceId) => {
        if (typeof callback !== 'function') throw new Error('Stream callback must be a function')
        const key = instanceId == null ? 'default' : instanceId
        if (stream === 'stdout') this.stdoutHandlers.set(key, callback)
        else if (stream === 'stderr') this.stderrHandlers.set(key, callback)
        else throw new Error(`Unknown stream type: ${stream}`)
        return this._send('subscribe', {stream, instanceId})
    }

    /** @type { (wasmPath?: string, debug?: boolean) => Promise<any> } */
    initialize = (wasmPath = '', debug = false) => this._send('init', {wasmPath, debug})
    /** @type { (method: string, ...args: any[]) => Promise<any> } */
    call = (method, ...args) => this._send('call', {method, args})
    /** @type { (instanceId: number | null | undefined, method: string, ...args: any[]) => Promise<any> } */
    callInstance = (instanceId, method, ...args) => this._send('call', {instanceId, method, args})
    /** @type { (wasmPath?: string, debug?: boolean) => Promise<VovkPLCWorkerInstance> } */
    createInstance = (wasmPath = '', debug = false) => this._send('create', {wasmPath, debug}).then(id => new VovkPLCWorkerInstance(this, id))
    /** @type { (instanceId: number | null | undefined) => Promise<any> } */
    disposeInstance = instanceId => this._send('dispose', {instanceId})
    /** @type { (callback: (message: string, instanceId?: number) => void, instanceId?: number | null) => Promise<any> } */
    onStdout = (callback, instanceId = null) => this._setStreamHandler('stdout', callback, instanceId)
    /** @type { (callback: (message: string, instanceId?: number) => void, instanceId?: number | null) => Promise<any> } */
    onStderr = (callback, instanceId = null) => this._setStreamHandler('stderr', callback, instanceId)

    /** @type { () => Promise<any> } */
    terminate = async () => {
        for (const pending of this.pending.values()) pending.reject(new Error('Worker terminated'))
        this.pending.clear()
        this.stdoutHandlers.clear()
        this.stderrHandlers.clear()
        if (typeof this.worker.removeEventListener === 'function') {
            this.worker.removeEventListener('message', this._onMessage)
            this.worker.removeEventListener('error', this._onError)
        } else if (typeof this.worker.off === 'function') {
            this.worker.off('message', this._onMessage)
            this.worker.off('error', this._onError)
        } else if (typeof this.worker.removeListener === 'function') {
            this.worker.removeListener('message', this._onMessage)
            this.worker.removeListener('error', this._onError)
        }
        if (typeof this.worker.terminate === 'function') return this.worker.terminate()
    }
}

class VovkPLCWorker extends VovkPLCWorkerClient {
    /** @type { VovkPLC_class } */
    helper
    /** @type { VovkPLC_class['buildCommand'] } */
    buildCommand
    /** @param { VovkPLCWorkerLike } worker */
    constructor(worker) {
        super(worker)
        this.helper = new VovkPLC_class()
        this.buildCommand = this.helper.buildCommand
    }

    /** @type { (wasmPath?: string, options?: VovkPLCWorkerOptions) => Promise<VovkPLCWorker> } */
    static create = async (wasmPath = '', {workerUrl, workerFactory, debug = false} = {}) => {
        const resolvedUrl = workerUrl || new URL('./VovkPLC.worker.js', import.meta.url)
        const factory = workerFactory || (await getDefaultWorkerFactory())
        const worker = await createWorker(factory, resolvedUrl)
        const client = new VovkPLCWorker(worker)
        await client.initialize(wasmPath, debug)
        return client
    }

    /** @type { (assembly: string, debug?: boolean) => Promise<any> } */
    lint = (assembly, debug = false) => this.call('lint', assembly, debug)
    /** @type { (value?: boolean) => Promise<any> } */
    setSilent = value => this.call('setSilent', value)
    /** @type { () => Promise<any> } */
    printInfo = () => this.call('printInfo')
    /** @type { (assembly: string) => Promise<any> } */
    downloadAssembly = assembly => this.call('downloadAssembly', assembly)
    /** @type { (assembly: string, run?: boolean) => Promise<any> } */
    compile = (assembly, run = false) => this.call('compile', assembly, run)
    /** @type { (program: string | number[]) => Promise<any> } */
    downloadBytecode = program => this.call('downloadBytecode', program)
    /** @type { () => Promise<any> } */
    run = () => this.call('run')
    /** @type { () => Promise<any> } */
    runDebug = () => this.call('runDebug')
    /** @type { () => Promise<{ size: number, output: string }> } */
    extractProgram = () => this.call('extractProgram')
    /** @type { (address: number, size?: number) => Promise<Uint8Array> } */
    readMemoryArea = (address, size = 1) => this.call('readMemoryArea', address, size)
    /** @type { (address: number, data: number[]) => Promise<string> } */
    writeMemoryArea = (address, data) => this.call('writeMemoryArea', address, data)
    /** @type { () => Promise<string> } */
    readStream = () => this.call('readStream')
    /** @type { () => Promise<string[]> } */
    getExports = () => this.call('getExports')
    /** @type { (name: string, ...args: any[]) => Promise<any> } */
    callExport = (name, ...args) => this.call('callExport', name, ...args)

    /** @type { (data: number | number[], crc?: number) => number } */
    crc8 = (data, crc = 0) => this.helper.crc8(data, crc)
    /** @type { (hex: string) => number[] } */
    parseHex = hex => this.helper.parseHex(hex)
    /** @type { (str: string) => string } */
    stringToHex = str => this.helper.stringToHex(str)
}

/** @type { () => Promise<VovkPLCWorkerFactory> } */
const getDefaultWorkerFactory = async () => {
    if (isNodeRuntime) {
        const {Worker} = await import('worker_threads') // @ts-ignore
        return url => new Worker(url, {type: 'module'})
    }
    if (typeof Worker !== 'undefined') return url => new Worker(url, {type: 'module'})
    throw new Error('Workers are not supported in this environment')
}

/** @type { (factory: VovkPLCWorkerFactory, url: URL | string) => Promise<VovkPLCWorkerLike> } */
const createWorker = async (factory, url) => {
    const worker = factory(url) // @ts-ignore
    return worker && typeof worker.then === 'function' ? await worker : worker
}

/** @type { (wasmPath?: string, options?: VovkPLCWorkerOptions) => Promise<VovkPLCWorker> } */// @ts-ignore
VovkPLC_class.createWorker = (wasmPath = '', options = {}) => VovkPLCWorker.create(wasmPath, options)

// Export the module if we are in a browser
if (typeof window !== 'undefined') {
    // console.log(`WASM exported as window object`)
    Object.assign(window, {VovkPLC: VovkPLC_class})
}

// Export for CommonJS modules
if (typeof module !== 'undefined') {
    // console.log(`WASM exported as module`)
    module.exports = VovkPLC_class
}
// Export for ES modules
export default VovkPLC_class
export {VovkPLCWorker, VovkPLCWorkerClient, VovkPLCWorkerInstance}
