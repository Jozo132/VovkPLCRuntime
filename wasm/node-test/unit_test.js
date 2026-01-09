// @ts-check
'use strict'

// This file is used to the WebAssembly module (which was compiled from C++ code) to run in Node.js.
// It is used for testing purposes only.

// const PLCRuntimeWasm = require("../dist/VovkPLC.js").default
import VovkPLC from '../dist/VovkPLC.js'
import path from 'path'
import {fileURLToPath} from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)
const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')

const main = async () => {
    const runtime = await VovkPLC.createWorker(wasmPath)
    try {
        let message = ''
        try {
            console.log('Running VovkPLCRuntime WebAssembly simulation unit test...')
            await runtime.callExport('run_unit_test')
            for (let i = 0; i < 10; i++) await runtime.callExport('run_custom_test')
            message = await runtime.readStream()
            console.log('Done.')
        } catch (error) {
            console.error(error)
        }
        if (message) console.log(message)
    } finally {
        await runtime.terminate()
    }
}

main()
    .catch(console.error)
    .finally(() => process.exit())
