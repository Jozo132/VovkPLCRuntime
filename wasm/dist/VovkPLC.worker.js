// @ts-check
'use strict'

import VovkPLC from './VovkPLC.js'

const isNodeRuntime = typeof process !== 'undefined' && !!(process.versions && process.versions.node)

/**
 * @typedef {{ post: (message: any) => void, on: (handler: (message: any) => void) => void }} VovkPLCWorkerPort
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
            post: message => parentPort.postMessage(message),
            on: handler => parentPort.on('message', handler),
        }
    }
    if (typeof self === 'undefined') throw new Error('Worker global scope not available')
    return {
        post: message => self.postMessage(message),
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

/** @type { (post: (message: VovkPLCWorkerResponse | VovkPLCWorkerEvent) => void, message: VovkPLCWorkerRequest) => Promise<void> } */
const handleMessage = async (post, message) => {
    if (!message || typeof message !== 'object') return
    const {id, type} = message
    if (typeof id !== 'number') return
    try {
        let result
        if (type === 'init') {
            const {wasmPath = '', debug = false, silent = false} = message
            result = await enqueue('default', () => ensureDefault(wasmPath, debug, silent))
        } else if (type === 'create') {
            const {wasmPath = '', debug = false, silent = false} = message
            result = await createInstance(wasmPath, debug, silent)
        } else if (type === 'dispose') {
            result = disposeInstance(message.instanceId)
        } else if (type === 'call') {
            const {instanceId, method, args = []} = message
            if (!method) throw new Error('Method name is required')
            const key = getInstanceKey(instanceId)
            result = await enqueue(key, () => callInstance(instanceId, method, args))
        } else if (type === 'subscribe') {
            const {instanceId, stream} = message
            if (!stream) throw new Error('Stream name is required')
            result = subscribeInstance(post, instanceId, stream)
        } else {
            throw new Error(`Unknown worker request: ${type}`)
        }
        post({id, ok: true, result})
    } catch (error) {
        post({id, ok: false, error: String(error instanceof Error ? error.message || error : error)})
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
