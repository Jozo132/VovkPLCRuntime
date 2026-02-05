// @ts-check
'use strict'

import VovkPLC from './VovkPLC.js'

const isNodeRuntime = typeof process !== 'undefined' && !!(process.versions && process.versions.node)

/**
 * Robust detection for shared memory and decoding capabilities.
 */
const checkSupport = () => {
    const support = {
        sab: typeof SharedArrayBuffer !== 'undefined',
        atomics: typeof Atomics !== 'undefined',
        decodeShared: false
    }
    
    if (support.sab) {
        try {
            const sab = new SharedArrayBuffer(1)
            const view = new Uint8Array(sab)
            new TextDecoder().decode(view)
            support.decodeShared = true
        } catch (e) {
            // Keep defaults
        }
    }
    return support
}

const SUPPORT = checkSupport()

// Control flags for worker loops
let queueLoopActive = false
let sharedLoopActive = new Map() // key -> active

const SHARED_BUFFER_SIZE = 64 * 1024 * 1024 // 64MB
const RING_SIZE = 1024 * 1024 // 1MB for each ring
const OFFSETS = {
    M2S_WRITE: 0, // Master Write Index
    M2S_READ: 4,  // Slave Read Index
    S2M_WRITE: 8, // Slave Write Index
    S2M_READ: 12, // Master Read Index
    LOCK: 16,
    IO_OFFSET: 20,
    IO_IN_SIZE: 24,
    IO_OUT_SIZE: 28,
    M2S_START: 1024,
    S2M_START: 1024 + RING_SIZE,
    DATA_START: 1024 + RING_SIZE * 2
}

/**
 * @typedef {{ post: (message: any, transfer?: Transferable[]) => void, on: (handler: (message: any) => void) => void }} VovkPLCWorkerPort
 * @typedef {Map<number, VovkPLC>} VovkPLCInstanceMap
 * @typedef {Map<string | number, Promise<any>>} VovkPLCQueueMap
 * @typedef {{ id: number, type: string, instanceId?: number, method?: string, args?: any[], stream?: string, wasmPath?: string, debug?: boolean, silent?: boolean }} VovkPLCWorkerRequest
 * @typedef {{ id: number, ok: boolean, result?: any, error?: string }} VovkPLCWorkerResponse
 * @typedef {{ type: 'event', event: 'stdout' | 'stderr', instanceId?: number, message: string }} VovkPLCWorkerEvent
 */

/** @type { () => Promise<VovkPLCWorkerPort> } */
const getPort = async () => {
    if (isNodeRuntime) {
        const {parentPort} = await import('worker_threads')
        if (!parentPort) throw new Error('worker_threads parentPort not available')
        return {
            post: (message, transfer) => parentPort.postMessage(message, transfer),
            on: handler => parentPort.on('message', handler),
        }
    }
    if (typeof self === 'undefined') throw new Error('Worker global scope not available')
    return {
        // @ts-ignore - postMessage with transfer list
        post: (message, transfer) => transfer ? self.postMessage(message, transfer) : self.postMessage(message),
        on: handler => {
            self.onmessage = event => handler(event.data)
        },
    }
}

/** @type { VovkPLCInstanceMap } */
const instances = new Map()
/** @type { VovkPLCQueueMap } */
const queues = new Map()
/** @type { VovkPLC | null } */
let defaultInstance = null
/** @type { number } */
let nextId = 1

/** @type { (key: string | number, fn: () => any) => Promise<any> } */
const enqueue = (key, fn) => {
    const previous = queues.get(key) || Promise.resolve()
    const next = previous.then(fn, fn)
    queues.set(
        key,
        next.catch(() => {})
    )
    return next
}

/** @type { (instanceId?: number | null) => string | number } */
const getInstanceKey = instanceId => (instanceId == null ? 'default' : instanceId)

/** @type { (wasmPath?: string, debug?: boolean, silent?: boolean) => Promise<boolean> } */
const ensureDefault = async (wasmPath, debug, silent = false) => {
    if (!defaultInstance) defaultInstance = new VovkPLC(wasmPath)
    await defaultInstance.initialize(wasmPath, debug, silent)
    return true
}

/** @type { (wasmPath?: string, debug?: boolean, silent?: boolean) => Promise<number> } */
const createInstance = async (wasmPath, debug, silent = false) => {
    const instanceId = nextId++
    const instance = new VovkPLC(wasmPath)
    instances.set(instanceId, instance)
    await instance.initialize(wasmPath, debug, silent)
    return instanceId
}

/** @type { (instanceId?: number | null) => boolean } */
const disposeInstance = instanceId => {
    if (instanceId == null) {
        defaultInstance = null
        queues.delete('default')
        return true
    }
    instances.delete(instanceId)
    queues.delete(instanceId)
    return true
}

/** @type { (instanceId: number | null | undefined, method: string, args?: any[]) => any } */
const callInstance = (instanceId, method, args) => {
    const instance = instanceId == null ? defaultInstance : instances.get(instanceId)
    if (!instance) throw new Error('WebAssembly module not initialized')
    if (method === 'getExports') {
        const exportsObject = instance.getExports()
        return Object.keys(exportsObject)
    }
    const fn = /** @type { any } */ (instance)[method]
    if (typeof fn !== 'function') throw new Error(`'${method}' function not found`)
    return fn.apply(instance, args || [])
}

/** @type { (post: (message: VovkPLCWorkerEvent) => void, instanceId: number | undefined, stream: string) => boolean } */
const subscribeInstance = (post, instanceId, stream) => {
    const instance = instanceId == null ? defaultInstance : instances.get(instanceId)
    if (!instance) throw new Error('WebAssembly module not initialized')
    if (stream === 'stdout') {
        instance.onStdout(message => post({type: 'event', event: 'stdout', instanceId, message}))
        return true
    }
    if (stream === 'stderr') {
        instance.onStderr(message => post({type: 'event', event: 'stderr', instanceId, message}))
        return true
    }
    throw new Error(`Unknown stream type: ${stream}`)
}


const encoder = new TextEncoder()
const decoder = new TextDecoder()

/** @type { SharedArrayBuffer | null } */
let sab = null
/** @type { Int32Array | null } */
let sabI32 = null
/** @type { Uint8Array | null } */
let sabU8 = null

/**
 * Handles a batched ArrayBuffer request from the main thread.
 * Format: [u32 count][...ops]
 * Each op: [u32 id][u8 typeLen][type bytes][u32 payloadLen][payload bytes]
 * @param { (message: any, transfer?: Transferable[]) => void } post
 * @param { ArrayBuffer } buffer
 */
const handleBatchRequest = async (post, buffer) => {
    const dv = new DataView(buffer)
    const u8 = new Uint8Array(buffer)
    let off = 0
    
    const count = dv.getUint32(off, true); off += 4
    
    // Parse all requests
    /** @type { Array<{id: number, type: string, payload: any}> } */
    const requests = []
    for (let i = 0; i < count; i++) {
        const id = dv.getUint32(off, true); off += 4
        const typeLen = dv.getUint8(off); off += 1
        const type = decoder.decode(u8.subarray(off, off + typeLen)); off += typeLen
        const payloadLen = dv.getUint32(off, true); off += 4
        let payload = {}
        if (payloadLen > 0) {
            const payloadStr = decoder.decode(u8.subarray(off, off + payloadLen))
            try { payload = JSON.parse(payloadStr) } catch (e) { /* ignore */ }
        }
        off += payloadLen
        requests.push({ id, type, payload })
    }
    
    // Process all requests and collect responses
    /** @type { Array<{id: number, ok: boolean, result?: any, error?: string}> } */
    const responses = []
    
    for (const req of requests) {
        try {
            const result = await processRequest(post, req.id, req.type, req.payload)
            responses.push({ id: req.id, ok: true, result })
        } catch (e) {
            responses.push({ id: req.id, ok: false, error: String(e instanceof Error ? e.message : e) })
        }
    }
    
    // Build response buffer
    // Format: [u32 count][...responses]
    // Each response: [u32 id][u8 ok][u32 payloadLen][payload bytes]
    let totalSize = 4
    const encodedResponses = responses.map(r => {
        const payloadBytes = encoder.encode(JSON.stringify(r.ok ? { result: r.result } : { error: r.error }))
        totalSize += 4 + 1 + 4 + payloadBytes.length
        return { id: r.id, ok: r.ok, payloadBytes }
    })
    
    const respBuf = new ArrayBuffer(totalSize)
    const respDv = new DataView(respBuf)
    const respU8 = new Uint8Array(respBuf)
    let respOff = 0
    
    respDv.setUint32(respOff, encodedResponses.length, true); respOff += 4
    
    for (const r of encodedResponses) {
        respDv.setUint32(respOff, r.id, true); respOff += 4
        respDv.setUint8(respOff, r.ok ? 1 : 0); respOff += 1
        respDv.setUint32(respOff, r.payloadBytes.length, true); respOff += 4
        respU8.set(r.payloadBytes, respOff); respOff += r.payloadBytes.length
    }
    
    // Transfer response buffer back (zero-copy)
    post(respBuf, [respBuf])
}

/**
 * Process a single request (shared logic for both batched and individual messages).
 * @param { (message: any, transfer?: Transferable[]) => void } post
 * @param { number } id
 * @param { string } type
 * @param { any } payload
 * @returns { Promise<any> }
 */
const processRequest = async (post, id, type, payload) => {
    if (type === 'init') {
        const { wasmPath = '', debug = false, silent = false, sharedBuffer } = payload
        
        // Handle Shared Buffer Setup
        if (SUPPORT.sab && SUPPORT.atomics && sharedBuffer instanceof SharedArrayBuffer) {
            sab = sharedBuffer
            sabI32 = new Int32Array(sab)
            sabU8 = new Uint8Array(sab)
        }
        
        const ok = await enqueue('default', () => ensureDefault(wasmPath, debug, silent))
        return sab ? 'init_ack' : ok
    } else if (type === 'create') {
        const { wasmPath = '', debug = false, silent = false } = payload
        return await createInstance(wasmPath, debug, silent)
    } else if (type === 'dispose') {
        return disposeInstance(payload.instanceId)
    } else if (type === 'call') {
        const { instanceId, method, args = [] } = payload
        if (!method) throw new Error('Method name is required')
        const key = getInstanceKey(instanceId)
        return await enqueue(key, () => callInstance(instanceId, method, args))
    } else if (type === 'subscribe') {
        const { instanceId, stream } = payload
        if (!stream) throw new Error('Stream name is required')
        return subscribeInstance(post, instanceId, stream)
    } else if (type === 'setup_shared_io') {
        return setupSharedIO()
    } else if (type === 'startRuntime') {
        if (sabI32) {
            const offset = sabI32[OFFSETS.IO_OFFSET / 4]
            const i32 = new Int32Array(sab, offset)
            Atomics.store(i32, 0, 1)
            return true
        }
        return false
    } else if (type === 'stopRuntime') {
        if (sabI32) {
            const offset = sabI32[OFFSETS.IO_OFFSET / 4]
            const i32 = new Int32Array(sab, offset)
            Atomics.store(i32, 0, 0)
            return true
        }
        return false
    } else if (type === 'resetStats') {
        if (sabI32) {
            const offset = sabI32[OFFSETS.IO_OFFSET / 4]
            const i32 = new Int32Array(sab, offset)
            Atomics.store(i32, 6, 0)
            Atomics.store(i32, 8, 2147483647)
            Atomics.store(i32, 9, 0)
            return true
        }
        return false
    } else if (type === 'setup_shared_memory') {
        const { instanceId, sharedBuffer } = payload
        return setupSharedInstance(instanceId, sharedBuffer)
    } else if (type === 'shutdown') {
        // Stop all loops to allow graceful termination
        stopAllLoops()
        // Give loops time to see the flag, then exit
        if (isNodeRuntime) {
            setTimeout(() => {
                if (typeof process !== 'undefined' && typeof process.exit === 'function') {
                    process.exit(0)
                }
            }, 50)
        }
        return true
    } else if (type === 'ffi_register') {
        // Register a JS function for FFI - function is passed as string and recreated
        const { instanceId, name, signature, description, fnString } = payload
        const instance = instanceId == null ? defaultInstance : instances.get(instanceId)
        if (!instance) throw new Error('WebAssembly module not initialized')
        
        // Recreate the function from string
        // The fnString should be an arrow function or function expression
        // e.g., "(a, b) => a + b" or "function(a, b) { return a + b; }"
        let fn
        try {
            // Wrap in parentheses to make it an expression
            fn = eval(`(${fnString})`)
        } catch (e) {
            throw new Error(`Failed to parse FFI function: ${e.message}`)
        }
        
        if (typeof fn !== 'function') {
            throw new Error('FFI function string did not evaluate to a function')
        }
        
        return instance.registerJSFunction(name, signature, description, fn)
    } else if (type === 'ffi_unregister') {
        const { instanceId, index } = payload
        const instance = instanceId == null ? defaultInstance : instances.get(instanceId)
        if (!instance) throw new Error('WebAssembly module not initialized')
        return instance.unregisterJSFunction(index)
    } else if (type === 'ffi_count') {
        const { instanceId } = payload
        const instance = instanceId == null ? defaultInstance : instances.get(instanceId)
        if (!instance) throw new Error('WebAssembly module not initialized')
        return instance.getFFICount()
    } else if (type === 'ffi_clear') {
        const { instanceId } = payload
        const instance = instanceId == null ? defaultInstance : instances.get(instanceId)
        if (!instance) throw new Error('WebAssembly module not initialized')
        instance.clearFFI()
        return true
    } else {
        throw new Error(`Unknown worker request: ${type}`)
    }
}

/** @type { (post: (message: VovkPLCWorkerResponse | VovkPLCWorkerEvent) => void, message: VovkPLCWorkerRequest | ArrayBuffer) => Promise<void> } */
const handleMessage = async (post, message) => {
    // Handle batched ArrayBuffer requests
    if (message instanceof ArrayBuffer) {
        await handleBatchRequest(post, message)
        return
    }
    
    if (!message || typeof message !== 'object') return
    const { id, type } = message
    if (typeof id !== 'number') return
    
    try {
        const result = await processRequest(post, id, type, message)
        
        // If using Shared Ring, write response there (Except for init which uses postMessage handshake)
        if (sab && type !== 'init') {
            writeResponseToRing({ id, ok: true, result })
        } else {
            post({ id, ok: true, result, type: (type === 'init' && sab) ? 'init_ack' : undefined })
        }
        
        // Start polling loop AFTER sending response if this was init
        if (type === 'init' && sab) {
            startQueueLoop(post)
        }
    } catch (error) {
        if (sab && type !== 'init') {
            writeResponseToRing({ id, ok: false, error: String(error instanceof Error ? error.message || error : error) })
        } else {
            post({ id, ok: false, error: String(error instanceof Error ? error.message || error : error) })
        }
    }
}

const writeResponseToRing = (msg) => {
    if (!sabU8 || !sabI32) return
    // Frame: [TotalSize(4)][ID(4)][Ok(1)][Payload(json)]
    const { id, ok, ...rest } = msg
    const payloadStr = JSON.stringify(rest)
    const payloadBytes = encoder.encode(payloadStr)
    const totalSize = 9 + payloadBytes.length
    
    let writeIdx = Atomics.load(sabI32, OFFSETS.S2M_WRITE / 4)
    const readIdx = Atomics.load(sabI32, OFFSETS.S2M_READ / 4)
    
    if (writeIdx - readIdx + totalSize > RING_SIZE) {
        // Drop? Or spin wait? Spin wait a bit.
        // If full, client isn't reading.
        console.error('Worker Response Queue Full - Dropping Message', id)
        return
    }
    
    const start = OFFSETS.S2M_START
    const current = writeIdx
     
    // Write Helper
    const writeByte = (idx, val) => { if (sabU8) sabU8[start + (idx % RING_SIZE)] = val }
    const writeU32 = (idx, val) => {
        writeByte(idx, val & 0xff)
        writeByte(idx + 1, (val >> 8) & 0xff)
        writeByte(idx + 2, (val >> 16) & 0xff)
        writeByte(idx + 3, (val >> 24) & 0xff)
    }
    
    writeU32(current, totalSize)
    writeU32(current + 4, id)
    writeByte(current + 8, ok ? 1 : 0)
    
    // Payload
    for (let j = 0; j < payloadBytes.length; j++) writeByte(current + 9 + j, payloadBytes[j])
    
    const nextWrite = writeIdx + totalSize
    Atomics.store(sabI32, OFFSETS.S2M_WRITE / 4, nextWrite)
    // Client polls, no notify needed, or notify is useless for browser main thread
}

const stopAllLoops = () => {
    queueLoopActive = false
    sharedLoopActive.clear()
}

const startQueueLoop = async (/** @type {(msg: any) => void} */ post) => {
    queueLoopActive = true
    // Convert to non-blocking loop to allow runSharedLoop (setTimeout) to function
    const checkQueue = async () => {
        if (!queueLoopActive || !sabI32 || !sabU8) return

        try {
            const readIdxAddr = OFFSETS.M2S_READ / 4
            const writeIdxAddr = OFFSETS.M2S_WRITE / 4
            
            // Check availability
            let writeIdx = Atomics.load(sabI32, writeIdxAddr)
            let readIdx = Atomics.load(sabI32, readIdxAddr)
            
            if (readIdx !== writeIdx) {
                // Process available commands
                while (readIdx !== writeIdx) {
                    const start = OFFSETS.M2S_START
                    const current = readIdx
                    
                    const readByte = (idx) => sabU8 ? sabU8[start + (idx % RING_SIZE)] : 0
                    const readU32 = (idx) => {
                        let val = 0
                        val |= readByte(idx)
                        val |= readByte(idx + 1) << 8
                        val |= readByte(idx + 2) << 16
                        val |= readByte(idx + 3) << 24
                        return val
                    }
                    
                    const frameSize = readU32(current)
                    const id = readU32(current + 4)
                    const typeLen = readByte(current + 8)
                    
                    // Type
                    const typeBytes = new Uint8Array(typeLen)
                    for (let i = 0; i < typeLen; i++) typeBytes[i] = readByte(current + 9 + i)
                    const type = decoder.decode(typeBytes)
                    
                    // Payload
                    const payloadLen = frameSize - 9 - typeLen
                    let payload = {}
                        if (payloadLen > 0) {
                        const payloadStart = (current + 9 + typeLen) % RING_SIZE
                        const payloadEnd = (current + 9 + typeLen + payloadLen) % RING_SIZE
                        
                        if (payloadEnd > payloadStart) {
                            const view = new Uint8Array(sab, start + payloadStart, payloadLen)
                            const str = decoder.decode(SUPPORT.decodeShared ? view : new Uint8Array(view))
                            if (str) payload = JSON.parse(str)
                        } else {
                            // Wrapping
                            const part1Len = RING_SIZE - payloadStart
                            const buf = new Uint8Array(payloadLen)
                            buf.set(new Uint8Array(sab, start + payloadStart, part1Len))
                            buf.set(new Uint8Array(sab, start, payloadLen - part1Len), part1Len)
                            const str = decoder.decode(buf)
                            if (str) payload = JSON.parse(str)
                        }
                    }
                    
                    const message = { id, type, ...payload }
                    await handleMessage(post, message)

                    readIdx += frameSize
                    Atomics.store(sabI32, readIdxAddr, readIdx)
                    
                    // Refresh Limit
                    writeIdx = Atomics.load(sabI32, writeIdxAddr)
                }
            }
        } catch(e) {
            console.error('[WORKER] Queue Error:', e)
        }
        
        // Schedule next check (5ms polling)
        if (queueLoopActive) setTimeout(checkQueue, 5)
    }
    checkQueue()
}

/** @type { (instanceId: number | undefined, buffer: SharedArrayBuffer) => boolean } */

/** @type { Map<number | string, { buffer: SharedArrayBuffer, i32: Int32Array, inputs: Uint8Array, outputs: Uint8Array, instance: VovkPLC, wasm_input_offset: number, wasm_output_offset: number }> } */
const sharedContexts = new Map()

/** @type { (instanceId: number | undefined, buffer: SharedArrayBuffer) => boolean } */
const setupSharedInstance = (instanceId, buffer) => {
    const key = getInstanceKey(instanceId)
    const instance = instanceId == null ? defaultInstance : instances.get(instanceId)
    if (!instance) throw new Error('WebAssembly module not initialized')

    if (!SUPPORT.sab || !SUPPORT.atomics || !(buffer instanceof SharedArrayBuffer)) {
        throw new Error('SharedArrayBuffer/Atomics not supported or provided buffer is not shared')
    }

    const i32 = new Int32Array(buffer)
    // Structure: [0: CMD, 1: STATUS, 2: IN_OFF, 3: IN_SZ, 4: OUT_OFF, 5: OUT_SZ, 6: CYCLES, 7: TIME]
    const input_offset_sab = i32[2]
    const input_size = i32[3]
    const output_offset_sab = i32[4]
    const output_size = i32[5]

    const info = instance.printInfo()
    // @ts-ignore
    const wasm_input_offset = info.input_offset
    // @ts-ignore
    const wasm_output_offset = info.output_offset

    if (input_size !== info.input_size || output_size !== info.output_size) {
        // Warning: Size mismatch? We can proceed using the smaller size or error out.
        // Assuming client used printInfo from same instance, it should match.
    }

    const context = {
        buffer,
        i32,
        inputs: new Uint8Array(buffer, input_offset_sab, input_size),
        outputs: new Uint8Array(buffer, output_offset_sab, output_size),
        instance,
        wasm_input_offset,
        wasm_output_offset,
        input_size,
        output_size
    }
    
    sharedContexts.set(key, context)

    // Save Pointers to Header for Client
    // Structure: [0: CMD, 1: STATUS, 2: IN_OFF, 3: IN_SZ, 4: OUT_OFF, 5: OUT_SZ, 6: CYCLES, 7: TIME]
    // The previous implementation used a small buffer or mapped to a specific area.
    // Now we are in the BIG buffer.
    // But setupSharedInstance is called with a buffer. If it's the BIG buffer, we need to locate where to put this struct.
    
    // WARNING: "setupSharedInstance" was designed for the previous task (client provides buffer).
    // Now the buffer is GLOBAL (sab).
    // So we should ignore the buffer passed in "setup_shared_memory" if we have global SAB, or ensure it matches.
    // But 'setup_shared_io' calls 'setupSharedIO' which we need to implement.
    
    // Start Loop
    runSharedLoop(key)
    return true
}

const setupSharedIO = () => {
    // Map the default instance to the shared buffer structure
    // We use the IO_OFFSET area
    if (!defaultInstance || !sabI32 || !sabU8) throw new Error('Not ready for Shared IO')
    
    const info = defaultInstance.printInfo()
    // @ts-ignore
    const input_size = info.input_size
    // @ts-ignore
    const output_size = info.output_size
    
    // Calculate offsets
    // We place the Control Struct at DATA_START (after rings)
    // Structure: [0..15 ints]
    const structStart = OFFSETS.DATA_START
    // Just AFTER the struct (64 bytes), we place Inputs, then Outputs
    const inputOffset = structStart + 64
    const outputOffset = inputOffset + input_size
    
    // Write Header Info
    sabI32[OFFSETS.IO_OFFSET / 4] = structStart
    sabI32[OFFSETS.IO_IN_SIZE / 4] = input_size
    sabI32[OFFSETS.IO_OUT_SIZE / 4] = output_size
    
    // Init Struct
    const i32 = new Int32Array(sab, structStart)
    i32[2] = inputOffset // relative to SAB? No, absolute in SAB.
    // Wait, check runSharedLoop. It uses inputs = new Uint8Array(buffer, input_offset_sab...)
    // So these must be ABSOLUTE offsets.
    i32[2] = inputOffset 
    i32[3] = input_size
    i32[4] = outputOffset
    i32[5] = output_size
    
    // Create Context
    const context = {
        buffer: sab,
        i32,
        inputs: new Uint8Array(sab, inputOffset, input_size),
        outputs: new Uint8Array(sab, outputOffset, output_size),
        instance: defaultInstance,
        // @ts-ignore
        wasm_input_offset: info.input_offset,
        // @ts-ignore
        wasm_output_offset: info.output_offset,
        input_size,
        output_size
    }
    
    sharedContexts.set('default', context)
    sharedLoopActive.set('default', true)
    runSharedLoop('default')
}

const runSharedLoop = (key) => {
    if (!sharedLoopActive.get(key)) return
    
    try {
        const context = sharedContexts.get(key)
        if (!context) return
    
        const { i32, instance, inputs, outputs, wasm_input_offset, wasm_output_offset, input_size, output_size } = context
        
        // 0: CMD [0=IDLE, 1=RUN, 2=PAUSE/STOP, 3=STEP]
        const cmd = Atomics.load(i32, 0)
        
        if (cmd === 3) console.log('[WORKER] Loop sees CMD=3 (STEP)')
    
        if (cmd === 1) { // RUN
            // Store STATUS as RUNNING (1)
            Atomics.store(i32, 1, 1)
            
            const startChunk = Date.now()
            // Run in 15ms bursts to allow message processing
            const memory = /** @type {WebAssembly.Memory} */ (instance.wasm_exports.memory)
            const memBuffer = memory.buffer

            // Local stat tracking for this burst
            let min = Atomics.load(i32, 8) || 2147483647
            let max = Atomics.load(i32, 9) || 0
            
            // Use performance.now() if available for microsecond precision
            const now = (typeof performance !== 'undefined') ? () => performance.now() : () => Date.now()
    
            while (Date.now() - startChunk < 15) {
                const t0 = now()

                // 1. Sync Inputs (SAB -> WASM)
                const mem8 = new Uint8Array(memBuffer) // Re-create view in case of grow (though VovkPLC shouldn't grow memory in loop)
                // Optimization: if input_size is small, loop copy is faster than .set? .set is usually native fast.
                mem8.set(inputs, wasm_input_offset)
    
                // 2. Run
                instance.wasm_exports.run()
    
                // 3. Sync Outputs (WASM -> SAB) and Stats
                const outData = new Uint8Array(memBuffer, wasm_output_offset, output_size)
                outputs.set(outData)
                
                const t1 = now()
                // Time in MICROSECONDS
                const dt = Math.floor((t1 - t0) * 1000)

                // Stats
                const cycle = Atomics.load(i32, 6) + 1
                Atomics.store(i32, 6, cycle)
                Atomics.store(i32, 7, dt) // Last Time
                
                if (dt < min) { min = dt; Atomics.store(i32, 8, min) }
                if (dt > max) { max = dt; Atomics.store(i32, 9, max) }
    
                // Check if Command changed
                if (Atomics.load(i32, 0) !== 1) break
            }
            
            // Loop burst end.
            
            if (Atomics.load(i32, 0) === 1) {
                 Atomics.store(i32, 1, 1) // STATUS = RUNNING
            }
            if (sharedLoopActive.get(key)) setTimeout(() => runSharedLoop(key), 0)
        } else if (cmd === 3) { // STEP
            // Do one cycle
            const memory = /** @type {WebAssembly.Memory} */ (instance.wasm_exports.memory)
            const memBuffer = memory.buffer
            const mem8 = new Uint8Array(memBuffer)
            mem8.set(inputs, wasm_input_offset)
            
            instance.wasm_exports.run()
            
            const outData = new Uint8Array(memBuffer, wasm_output_offset, output_size)
            outputs.set(outData)
            
            const cycle = Atomics.load(i32, 6) + 1
            Atomics.store(i32, 6, cycle)
            // Use last cycle time from WASM if we don't have perf tracking here for single step?
            // Or just calculate it.
            // i32[7] = instance.wasm_exports.getLastCycleTimeUs()
    
            Atomics.store(i32, 0, 2) // CMD -> PAUSE
            Atomics.store(i32, 1, 2) // STATUS = PAUSED
            if (sharedLoopActive.get(key)) setTimeout(() => runSharedLoop(key), 10)
        } else {
            // IDLE or PAUSED
            Atomics.store(i32, 1, cmd === 2 ? 2 : 0) // STATUS matches CMD
            if (sharedLoopActive.get(key)) setTimeout(() => runSharedLoop(key), 50) 
        }
    } catch (e) {
        console.error('[WORKER] Error in runSharedLoop:', e)
        // Retry? Be careful of tight loops of death.
        if (sharedLoopActive.get(key)) setTimeout(() => runSharedLoop(key), 1000)
    }
}

/** @type { () => Promise<void> } */
const run = async () => {
    const port = await getPort()
    port.on(message => handleMessage(port.post, message))
}

run().catch(error => {
    console.error('VovkPLC worker failed to start:', error)
})
