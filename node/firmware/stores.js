import fs from 'node:fs'
import fsp from 'node:fs/promises'
import path from 'node:path'
import { createHash, randomUUID } from 'node:crypto'
import { EventEmitter } from 'node:events'
import { StringDecoder } from 'node:string_decoder'
import { Transform } from 'node:stream'
import { pipeline } from 'node:stream/promises'

export const terminalStates = new Set(['succeeded', 'failed', 'cancelled', 'interrupted'])
export const validId = id => typeof id === 'string' && /^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$/i.test(id)

export class FileJobStore {
    constructor(directory) { this.directory = directory; this.jobs = new Map(); this.notifications = new EventEmitter(); this.notifications.setMaxListeners(0) }
    async initialize() {
        await fsp.mkdir(this.directory, { recursive: true })
        for (const id of await fsp.readdir(this.directory)) {
            if (id.startsWith('.creating-')) { await fsp.rm(path.join(this.directory, id), { recursive: true, force: true }); continue }
            if (!validId(id)) continue
            try {
                const eventsPath = path.join(this.directory, id, 'events.ndjson')
                const handle = await fsp.open(eventsPath, 'r+')
                try {
                    const { size } = await handle.stat()
                    let position = size
                    let end = 0
                    const buffer = Buffer.alloc(65536)
                    while (position > 0) {
                        const length = Math.min(buffer.length, position)
                        position -= length
                        await handle.read(buffer, 0, length, position)
                        const last = buffer.lastIndexOf(10, length - 1)
                        if (last >= 0) { end = position + last + 1; break }
                    }
                    if (end !== size) await handle.truncate(end)
                } finally { await handle.close() }
                const job = JSON.parse(await fsp.readFile(path.join(this.directory, id, 'job.json'), 'utf8'))
                job.sequence = 0
                for await (const event of this.readEvents(id)) {
                    job.sequence = event.sequence
                    if (event.type === 'state') Object.assign(job, event.payload)
                }
                this.jobs.set(id, job)
            } catch (error) { throw new Error(`Cannot recover job ${id}: ${error.message}`) }
        }
    }
    dir(id) { if (!validId(id)) throw new Error('Invalid job ID'); return path.join(this.directory, id) }
    get(id) { const job = this.jobs.get(id); if (!job) throw new Error('Job not found'); return job }
    async create(job) {
        const staging = path.join(this.directory, `.creating-${job.id}`)
        await fsp.mkdir(staging)
        try {
            await fsp.writeFile(path.join(staging, 'job.json'), JSON.stringify(job), { flag: 'wx' })
            await fsp.writeFile(path.join(staging, 'events.ndjson'), '', { flag: 'wx' })
            await fsp.rename(staging, this.dir(job.id))
            this.jobs.set(job.id, job)
        } catch (error) { await fsp.rm(staging, { recursive: true, force: true }); throw error }
    }
    async append(id, type, payload) {
        const job = this.get(id)
        const event = { jobId: id, sequence: job.sequence + 1, timestamp: new Date().toISOString(), type, payload }
        await fsp.appendFile(path.join(this.dir(id), 'events.ndjson'), `${JSON.stringify(event)}\n`)
        job.sequence = event.sequence
        if (type === 'state') Object.assign(job, payload)
        this.notifications.emit(id)
        return event
    }
    async *readEvents(id, after = 0, through = Infinity) {
        const stream = fs.createReadStream(path.join(this.dir(id), 'events.ndjson'))
        const decoder = new StringDecoder('utf8')
        let pending = ''
        try {
            for await (const chunk of stream) {
                pending += decoder.write(chunk)
                let newline
                while ((newline = pending.indexOf('\n')) >= 0) {
                    const line = pending.slice(0, newline)
                    pending = pending.slice(newline + 1)
                    if (line) { const event = JSON.parse(line); if (event.sequence > after && event.sequence <= through) yield event }
                }
            }
        } finally { stream.destroy() }
    }
    async *events(id, { after = 0, signal } = {}) {
        if (!Number.isSafeInteger(after) || after < 0) throw new Error('Invalid event sequence')
        this.get(id)
        let offset = 0
        while (!signal?.aborted) {
            let wake
            const notified = new Promise(resolve => { wake = resolve; this.notifications.once(id, resolve); signal?.addEventListener('abort', resolve, { once: true }) })
            try {
                const through = this.get(id).sequence
                const stream = fs.createReadStream(path.join(this.dir(id), 'events.ndjson'), { start: offset })
                let pending = Buffer.alloc(0)
                let reachedUncommitted = false
                try {
                    for await (const chunk of stream) {
                        pending = Buffer.concat([pending, chunk])
                        let newline
                        while ((newline = pending.indexOf(10)) >= 0) {
                            const event = JSON.parse(pending.subarray(0, newline).toString('utf8'))
                            if (event.sequence > through) { reachedUncommitted = true; break }
                            offset += newline + 1
                            pending = pending.subarray(newline + 1)
                            if (event.sequence > after) {
                                if (signal?.aborted) return
                                after = event.sequence
                                yield event
                            }
                        }
                        if (reachedUncommitted) break
                    }
                } finally { stream.destroy() }
                const job = this.get(id)
                if (terminalStates.has(job.state) && after >= job.sequence) return
                if (after < job.sequence) continue
                await notified
            } finally { this.notifications.removeListener(id, wake); signal?.removeEventListener('abort', wake) }
        }
    }
    async remove(id) { this.get(id); await fsp.rm(this.dir(id), { recursive: true, force: true }); this.jobs.delete(id); this.notifications.emit(id) }
}

export class FileArtifactStore {
    constructor(directory) { this.directory = directory; this.index = new Map() }
    async initialize() {
        await fsp.mkdir(this.directory, { recursive: true })
        for (const jobId of await fsp.readdir(this.directory)) {
            if (jobId.startsWith('.staging-')) { await fsp.rm(path.join(this.directory, jobId), { recursive: true, force: true }); continue }
            if (!validId(jobId)) continue
            const records = JSON.parse(await fsp.readFile(path.join(this.directory, jobId, 'artifacts.json'), 'utf8'))
            for (const record of records) this.index.set(record.id, record)
        }
    }
    async publish(jobId, files, approvedRoot, signal) {
        const staging = path.join(this.directory, `.staging-${randomUUID()}`)
        await fsp.mkdir(staging)
        const records = []
        try {
            const root = await fsp.realpath(approvedRoot)
            for (const file of files) {
                signal?.throwIfAborted()
                const real = await fsp.realpath(file.path)
                if (!real.startsWith(root + path.sep) || !(await fsp.stat(real)).isFile()) throw new Error('Artifact outside approved output directory')
                const id = randomUUID()
                const hash = createHash('sha256')
                let size = 0
                const meter = new Transform({ transform(chunk, encoding, callback) { hash.update(chunk); size += chunk.length; callback(null, chunk) } })
                await pipeline(fs.createReadStream(real), meter, fs.createWriteStream(path.join(staging, id), { flags: 'wx' }), { signal })
                await fsp.chmod(path.join(staging, id), 0o400)
                records.push({ id, jobId, filename: path.basename(file.path), role: file.role || 'build-output', size, sha256: hash.digest('hex'), mediaType: file.mediaType || 'application/octet-stream', source: file.source || path.relative(root, real) })
            }
            await fsp.writeFile(path.join(staging, 'artifacts.json'), JSON.stringify(records), { flag: 'wx' })
            signal?.throwIfAborted()
            await fsp.rename(staging, path.join(this.directory, jobId))
            for (const record of records) this.index.set(record.id, record)
            return records
        } catch (error) { await fsp.rm(staging, { recursive: true, force: true }); throw error }
    }
    list(jobId) { return [...this.index.values()].filter(record => record.jobId === jobId).map(({ source, ...record }) => record) }
    info(id) {
        if (!validId(id) || !this.index.has(id)) throw new Error('Artifact not found')
        const { source, ...record } = this.index.get(id)
        return { ...record }
    }
    open(id) {
        if (!validId(id) || !this.index.has(id)) throw new Error('Artifact not found')
        const record = this.index.get(id)
        return fs.createReadStream(path.join(this.directory, record.jobId, id))
    }
    async remove(jobId) { await fsp.rm(path.join(this.directory, jobId), { recursive: true, force: true }); for (const [id, record] of this.index) if (record.jobId === jobId) this.index.delete(id) }
}