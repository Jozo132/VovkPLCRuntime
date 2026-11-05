import fs from 'node:fs'
import fsp from 'node:fs/promises'
import path from 'node:path'
import { createHash, randomUUID } from 'node:crypto'
import { StringDecoder } from 'node:string_decoder'
import { Transform } from 'node:stream'
import { pipeline } from 'node:stream/promises'
import { extractProject } from './firmware/project-loader.js'
import { FileJobStore, FileArtifactStore, terminalStates } from './firmware/stores.js'
import { LocalExecutor } from './firmware/executor.js'
import { PlatformIOAdapter } from './firmware/platformio.js'

export { FileJobStore, FileArtifactStore, LocalExecutor, PlatformIOAdapter }

class FirmwareBuilder {
    constructor(options) {
        if (!options.directory) throw new Error('A storage directory is required')
        this.directory = path.resolve(options.directory)
        this.limits = { uploadBytes: 128 * 1024 * 1024, extractBytes: 512 * 1024 * 1024, files: 10000, executionMs: 30 * 60 * 1000, concurrency: 1, queue: 100, retentionMs: 7 * 86400000, ...options.limits }
        for (const [name, value] of Object.entries(this.limits)) if (!Number.isSafeInteger(value) || value <= 0) throw new Error(`Invalid limit: ${name}`)
        this.jobs = options.jobStore || new FileJobStore(path.join(this.directory, 'jobs'))
        this.artifacts = options.artifactStore || new FileArtifactStore(path.join(this.directory, 'artifacts'))
        this.executor = options.executor || new LocalExecutor()
        this.loader = options.projectLoader || extractProject
        this.adapters = { platformio: new PlatformIOAdapter({ executable: options.platformioExecutable }), ...options.adapters }
        this.active = new Map()
        this.intakes = new Set()
        this.queue = []
        this.writes = new Map()
        this.closing = false
    }
    async initialize() {
        await fsp.mkdir(this.directory, { recursive: true })
        const lockPath = path.join(this.directory, '.firmware-builder.lock')
        try { this.lock = await fsp.open(lockPath, 'wx') } catch (error) {
            if (error.code !== 'EEXIST') throw error
            let pid
            try { pid = Number(await fsp.readFile(lockPath, 'utf8')); if (Number.isInteger(pid) && pid > 0) process.kill(pid, 0); else throw new Error('Invalid lock') } catch (check) {
                if (check.code !== 'ESRCH') throw new Error('Firmware builder storage is locked or has an invalid lock')
                await fsp.unlink(lockPath)
                this.lock = await fsp.open(lockPath, 'wx')
            }
            if (!this.lock) throw new Error('Firmware builder storage is already in use')
        }
        await this.lock.writeFile(String(process.pid))
        try {
            await this.jobs.initialize()
            await this.artifacts.initialize()
            for (const job of this.jobs.jobs.values()) {
                if (job.state === 'queued' && job.sourceSha256) this.queue.push(job.id)
                else if (!terminalStates.has(job.state)) {
                    await this.artifacts.remove(job.id)
                    await this.emit(job.id, 'state', { state: 'interrupted', finishedAt: new Date().toISOString(), error: { message: 'Build interrupted by service restart' } })
                }
            }
            this.pump()
            return this
        } catch (error) { await this.releaseLock(); throw error }
    }
    async releaseLock() { if (this.lock) { await this.lock.close(); this.lock = null; await fsp.unlink(path.join(this.directory, '.firmware-builder.lock')).catch(() => {}) } }
    emit(id, type, payload) {
        const previous = this.writes.get(id) || Promise.resolve()
        const next = previous.then(() => this.jobs.append(id, type, payload))
        this.writes.set(id, next)
        return next
    }
    async submit({ source, adapter = 'platformio', environment, projectRoot } = {}) {
        if (this.closing) throw new Error('Builder is closed')
        if (!this.adapters[adapter]) throw new Error(`Unknown build adapter: ${adapter}`)
        if (environment !== undefined && (typeof environment !== 'string' || !environment)) throw new Error('Invalid environment')
        if (this.queue.length + this.intakes.size >= this.limits.queue) throw new Error('Build queue is full')
        const stream = typeof source === 'string' ? fs.createReadStream(source) : source
        if (!stream || typeof stream.pipe !== 'function') throw new Error('Source must be a ZIP path or readable stream')
        let finishIntake
        const intake = { stream, finished: new Promise(resolve => { finishIntake = resolve }) }
        this.intakes.add(intake)
        const id = randomUUID()
        let created = false
        try {
            const now = new Date().toISOString()
            const job = { id, adapter, environment: environment || null, projectRoot: projectRoot ?? null, state: 'queued', createdAt: now, sequence: 0 }
            await this.jobs.create(job)
            created = true
            const hash = createHash('sha256')
            let size = 0
            const counter = new Transform({ transform: (chunk, encoding, callback) => { size += chunk.length; hash.update(chunk); callback(size > this.limits.uploadBytes ? new Error('Upload size limit exceeded') : null, chunk) } })
            await pipeline(stream, counter, fs.createWriteStream(path.join(this.jobs.dir(id), 'source.zip'), { flags: 'wx' }))
            if (this.closing) throw new Error('Builder is closed')
            job.sourceSha256 = hash.digest('hex')
            const metadataPath = path.join(this.jobs.dir(id), 'job.json')
            await fsp.writeFile(`${metadataPath}.tmp`, JSON.stringify(job))
            await fsp.rename(`${metadataPath}.tmp`, metadataPath)
            await this.emit(id, 'state', { state: 'queued', sourceSha256: job.sourceSha256, elapsedMs: 0, estimatedPercentage: null })
            this.queue.push(id)
            this.pump()
            return id
        } catch (error) { if (created) await this.jobs.remove(id); throw error }
        finally { this.intakes.delete(intake); finishIntake() }
    }
    pump() {
        while (!this.closing && this.active.size < this.limits.concurrency && this.queue.length) {
            const id = this.queue.shift()
            const control = { controller: new AbortController(), finishing: false }
            this.active.set(id, control)
            control.promise = this.run(id, control).finally(() => { this.active.delete(id); this.pump() })
        }
    }
    async run(id, control) {
        const job = this.jobs.get(id)
        const start = Date.now()
        let timedOut = false
        const timer = setTimeout(() => { timedOut = true; control.controller.abort(new Error('Build execution limit exceeded')) }, this.limits.executionMs)
        timer.unref()
        const signal = control.controller.signal
        const phase = async state => { signal.throwIfAborted(); await this.emit(id, 'state', { state, elapsedMs: Date.now() - start, estimatedPercentage: null }) }
        const decoder = { stdout: new StringDecoder('utf8'), stderr: new StringDecoder('utf8') }
        const pending = { stdout: '', stderr: '' }
        const onOutput = async (channel, chunk) => {
            const text = decoder[channel].write(chunk)
            await this.emit(id, 'output', { channel, data: chunk.toString('base64'), encoding: 'base64', text })
            pending[channel] += text
            const lines = pending[channel].split(/[\r\n]/)
            pending[channel] = lines.pop().slice(-8192)
            for (const line of lines) {
                const clean = line.replace(/\x1b\[[0-?]*[ -/]*[@-~]/g, '')
                const match = clean.match(/^(.*?):(\d+)(?::(\d+))?:\s*(fatal error|error|warning|note):\s*(.*)$/i)
                if (match) await this.emit(id, 'diagnostic', { file: match[1], line: Number(match[2]), column: match[3] ? Number(match[3]) : null, severity: match[4], message: match[5] })
            }
        }
        try {
            await phase('extracting')
            const directory = this.jobs.dir(id)
            const root = await this.loader(path.join(directory, 'source.zip'), path.join(directory, 'project'), { maxFiles: this.limits.files, maxExtractBytes: this.limits.extractBytes, projectRoot: job.projectRoot ?? undefined, signal })
            await phase('preparing')
            const buildDir = path.join(directory, 'build')
            const coreDir = path.join(this.directory, 'cache', id)
            await fsp.mkdir(coreDir, { recursive: true })
            const context = { executor: this.executor, root, buildDir, environment: job.environment, signal, timeoutMs: this.limits.executionMs, onOutput, env: { PLATFORMIO_CORE_DIR: coreDir, PLATFORMIO_BUILD_DIR: buildDir } }
            const adapter = this.adapters[job.adapter]
            if (!adapter) throw new Error(`Adapter unavailable after restart: ${job.adapter}`)
            const metadata = await adapter.prepare(context)
            const provenance = { sourceSha256: job.sourceSha256, adapter: job.adapter, ...metadata }
            await this.emit(id, 'metadata', provenance)
            await phase('building')
            await adapter.build(context, metadata)
            Object.assign(provenance, metadata)
            await this.emit(id, 'metadata', provenance)
            await phase('collecting')
            const collected = await adapter.collect(context, metadata)
            signal.throwIfAborted()
            const artifacts = await this.artifacts.publish(id, collected.files, collected.outputRoot, signal)
            signal.throwIfAborted()
            const manifest = adapter.manifest ? await adapter.manifest(collected, artifacts, provenance) : null
            signal.throwIfAborted()
            control.finishing = true
            for (const { source, ...artifact } of artifacts) await this.emit(id, 'artifact', artifact)
            await this.emit(id, 'state', { state: 'succeeded', finishedAt: new Date().toISOString(), elapsedMs: Date.now() - start, result: { provenance, manifest } })
        } catch (error) {
            await this.artifacts.remove(id).catch(() => {})
            const state = signal.aborted && !timedOut ? 'cancelled' : 'failed'
            control.finishing = true
            await this.emit(id, 'state', { state, finishedAt: new Date().toISOString(), elapsedMs: Date.now() - start, error: { message: timedOut ? 'Build execution limit exceeded' : error.message, ...(error.availableEnvironments ? { availableEnvironments: error.availableEnvironments } : {}) } })
        } finally { clearTimeout(timer) }
    }
    async status(id) { return structuredClone(this.jobs.get(id)) }
    async checkAdapter(name = 'platformio') {
        const adapter = this.adapters[name]
        if (!adapter || typeof adapter.check !== 'function') throw new Error(`Adapter check unavailable: ${name}`)
        return adapter.check({ executor: this.executor, timeoutMs: Math.min(this.limits.executionMs, 15000) })
    }
    events(id, options) { return this.jobs.events(id, options) }
    async result(id) { const job = this.jobs.get(id); return job.result || null }
    async cancel(id) {
        const job = this.jobs.get(id)
        if (terminalStates.has(job.state)) return false
        const control = this.active.get(id)
        if (control) { if (control.finishing) return false; control.controller.abort(); return true }
        const index = this.queue.indexOf(id)
        if (index < 0) return false
        this.queue.splice(index, 1)
        await this.emit(id, 'state', { state: 'cancelled', finishedAt: new Date().toISOString(), error: { message: 'Cancelled while queued' } })
        return true
    }
    async listArtifacts(id) { this.jobs.get(id); return this.artifacts.list(id) }
    async artifactInfo(id) { return this.artifacts.info(id) }
    openArtifact(id) { return this.artifacts.open(id) }
    async cleanup({ olderThanMs = this.limits.retentionMs } = {}) {
        if (!Number.isSafeInteger(olderThanMs) || olderThanMs < 0) throw new Error('Invalid retention age')
        const removed = []
        for (const job of this.jobs.jobs.values()) {
            if (!terminalStates.has(job.state) || this.active.has(job.id) || !job.finishedAt || Date.now() - Date.parse(job.finishedAt) < olderThanMs) continue
            await this.artifacts.remove(job.id)
            await fsp.rm(path.join(this.directory, 'cache', job.id), { recursive: true, force: true })
            await this.jobs.remove(job.id)
            this.writes.delete(job.id)
            removed.push(job.id)
        }
        return removed
    }
    async close() {
        if (this.closing) return
        this.closing = true
        for (const intake of this.intakes) intake.stream.destroy()
        await Promise.all([...this.intakes].map(intake => intake.finished))
        for (const id of [...this.queue]) await this.cancel(id)
        for (const control of this.active.values()) if (!control.finishing) control.controller.abort()
        await Promise.all([...this.active.values()].map(control => control.promise))
        await this.releaseLock()
    }
}

export async function createFirmwareBuilder(options = {}) { return new FirmwareBuilder(options).initialize() }