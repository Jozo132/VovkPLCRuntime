import fs from 'node:fs'
import path from 'node:path'
import { randomUUID } from 'node:crypto'
import { Worker } from 'node:worker_threads'
import VirtualPLC from './VirtualPLC.js'

export default class VirtualPLCManager {
    constructor(directory) {
        if (!directory) throw new Error('A manager data directory is required')
        this.directory = path.resolve(directory)
        this.workers = new Map()
        this.registryPath = path.join(this.directory, 'manager.json')
        this.registry = { version: 1, instances: {} }
        this.lock = new VirtualPLC(this.directory)
        this.errors = new Map()
    }

    async initialize() {
        fs.mkdirSync(this.directory, { recursive: true, mode: 0o700 })
        this.lock.acquireLock()
        try {
            if (fs.existsSync(this.registryPath)) {
                this.registry = JSON.parse(fs.readFileSync(this.registryPath, 'utf8'))
                if (this.registry.version !== 1 || !this.registry.instances || typeof this.registry.instances !== 'object' || Array.isArray(this.registry.instances) || Object.entries(this.registry.instances).some(([id, entry]) => !/^[a-f0-9-]{36}$/.test(id) || typeof entry?.autostart !== 'boolean')) throw new Error('Invalid manager registry')
            } else this.save()
            for (const [id, entry] of Object.entries(this.registry.instances)) {
                if (entry.autostart) {
                    try { await this.spawn(id) } catch (error) { this.errors.set(id, error.message) }
                }
            }
            return this
        } catch (error) {
            await this.close()
            throw error
        }
    }

    save() {
        const temp = path.join(this.directory, `.manager-${randomUUID()}.tmp`)
        try {
            fs.writeFileSync(temp, JSON.stringify(this.registry), { mode: 0o600, flag: 'wx' })
            fs.renameSync(temp, this.registryPath)
        } finally {
            if (fs.existsSync(temp)) fs.unlinkSync(temp)
        }
    }

    list() {
        return Object.entries(this.registry.instances).map(([id, entry]) => {
            let state
            try { state = JSON.parse(fs.readFileSync(path.join(this.directory, 'instances', id, 'state.json'), 'utf8')) } catch {}
            return { id, uid: state?.uid || null, config: state?.config || null, active: this.workers.has(id), autostart: entry.autostart, error: this.errors.get(id) || null }
        })
    }

    async create(config) {
        if (!config || typeof config !== 'object' || Array.isArray(config) || !Object.hasOwn(config, 'host') || !Object.hasOwn(config, 'port')) throw new Error('A host and TCP port are required')
        const id = randomUUID()
        const result = await this.spawn(id, config)
        this.registry.instances[id] = { autostart: true }
        try { this.save() } catch (error) {
            delete this.registry.instances[id]
            await this.stopWorker(id)
            throw error
        }
        return result
    }

    async spawn(id, config = null) {
        if (!/^[a-zA-Z0-9_-]{1,64}$/.test(id)) throw new Error('Invalid instance ID')
        if (this.workers.has(id)) throw new Error('Instance is already active')
        const directory = path.join(this.directory, 'instances', id)
        const execArgv = process.execArgv.filter((argument, index, argumentsList) => argument !== '--input-type' && !argument.startsWith('--input-type=') && argumentsList[index - 1] !== '--input-type')
        const worker = new Worker(new URL('./virtual-plc-worker.js', import.meta.url), { workerData: { directory, config }, execArgv })
        const record = { id, worker, directory, uid: null, config: null, lockToken: null, stopping: false }
        this.workers.set(id, record)
        worker.on('error', error => { this.errors.set(id, error.message) })
        worker.on('exit', code => {
            if (this.workers.get(id) === record) this.workers.delete(id)
            if (!record.stopping && record.uid) this.errors.set(id, `Worker exited (${code})`)
            const lock = path.join(directory, 'instance.lock')
            try { if (record.lockToken && fs.readFileSync(lock, 'utf8') === record.lockToken) fs.unlinkSync(lock) } catch {}
        })
        try {
            const ready = await new Promise((resolve, reject) => {
                worker.once('error', reject)
                worker.once('exit', code => reject(new Error(`Worker exited during startup (${code})`)))
                worker.on('message', message => {
                    if (message.type === 'lock') record.lockToken = message.lockToken
                    else if (message.type === 'ready') resolve(message)
                    else if (message.type === 'startup-error') reject(new Error(message.error || 'Worker failed to start'))
                })
            })
            record.uid = ready.uid
            record.config = ready.config
            record.lockToken = ready.lockToken
            this.errors.delete(id)
            return { id, uid: record.uid, config: record.config }
        } catch (error) {
            await worker.terminate()
            throw error
        }
    }

    async stopWorker(id) {
        const record = this.workers.get(id)
        if (!record) throw new Error('Instance is not active')
        record.stopping = true
        const exited = new Promise(resolve => record.worker.once('exit', resolve))
        record.worker.postMessage({ type: 'shutdown' })
        let timeout
        await Promise.race([exited, new Promise(resolve => { timeout = setTimeout(() => resolve(record.worker.terminate()), 3000) })])
        clearTimeout(timeout)
    }

    async start(id) {
        const entry = this.registry.instances[id]
        if (!entry) throw new Error('Unknown instance')
        let result
        try { result = await this.spawn(id) } catch (error) { this.errors.set(id, error.message); throw error }
        const previous = entry.autostart
        entry.autostart = true
        try { this.save() } catch (error) { entry.autostart = previous; await this.stopWorker(id); throw error }
        return result
    }

    async stop(id) {
        const entry = this.registry.instances[id]
        if (!entry) throw new Error('Unknown instance')
        const previous = entry.autostart
        entry.autostart = false
        try { this.save() } catch (error) { entry.autostart = previous; throw error }
        if (this.workers.has(id)) await this.stopWorker(id)
        this.errors.delete(id)
    }

    async remove(id) {
        const entry = this.registry.instances[id]
        if (!entry) throw new Error('Unknown instance')
        if (this.workers.has(id)) await this.stop(id)
        delete this.registry.instances[id]
        try { this.save() } catch (error) { this.registry.instances[id] = entry; throw error }
        this.errors.delete(id)
    }

    async configure(id, config) {
        if (!this.registry.instances[id]) throw new Error('Unknown instance')
        const record = this.workers.get(id)
        if (!record) {
            const instance = await new VirtualPLC(path.join(this.directory, 'instances', id)).initialize()
            try { return instance.configure(config) } finally { instance.close() }
        }
        const request = randomUUID()
        return new Promise((resolve, reject) => {
            const onMessage = message => {
                if (message.type !== 'response' || message.request !== request) return
                cleanup()
                if (message.error) reject(new Error(message.error))
                else resolve(message.config)
            }
            const onExit = () => { cleanup(); reject(new Error('Worker exited')) }
            const timeout = setTimeout(() => { cleanup(); reject(new Error('Worker configuration timed out')) }, 5000)
            const cleanup = () => { clearTimeout(timeout); record.worker.off('message', onMessage); record.worker.off('exit', onExit) }
            record.worker.on('message', onMessage)
            record.worker.once('exit', onExit)
            record.worker.postMessage({ type: 'configure', request, config })
        })
    }

    async close() {
        await Promise.all([...this.workers.keys()].map(id => this.stopWorker(id)))
        this.lock.close()
    }
}