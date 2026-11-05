import fs from 'node:fs'
import path from 'node:path'
import net from 'node:net'
import { randomUUID } from 'node:crypto'
import VovkPLC from '../wasm/dist/VovkPLC.js'

export default class VirtualPLC {
    constructor(dataDir) {
        if (!dataDir) throw new Error('A VirtualPLC data directory is required')
        this.dataDir = path.resolve(dataDir)
        this.statePath = path.join(this.dataDir, 'state.json')
        this.plc = new VovkPLC()
        this.state = null
        this.timer = null
        this.fault = null
        this.lockPath = path.join(this.dataDir, 'instance.lock')
        this.lockToken = null
    }

    async initialize(onLock = null) {
        fs.mkdirSync(this.dataDir, { recursive: true, mode: 0o700 })
        this.acquireLock()
        try {
            onLock?.(this.lockToken)
            await this.plc.initialize('', false, true)
            if (!this.plc.wasm_exports.db_setUserAreaEnd) throw new Error('VirtualPLC requires an updated runtime WASM build')
            const info = this.plc.getInfo()
            if (typeof info === 'string') throw new Error('Runtime information unavailable')
            const defaults = { name: 'VirtualPLC', intervalMs: 10, timerOffset: info.timer_offset, counterOffset: info.counter_offset, running: true, host: '0.0.0.0', port: 19616, discoveryPort: 19617 }
            if (fs.existsSync(this.statePath)) {
                this.state = JSON.parse(fs.readFileSync(this.statePath, 'utf8'))
                if (this.state.version !== 1 || !/^[0-9a-f-]{36}$/i.test(this.state.uid) || typeof this.state.program !== 'string' || !/^(?:[0-9a-f]{2})*$/i.test(this.state.program)) throw new Error('Invalid VirtualPLC state')
                this.state.config = this.validateConfig(this.state.config, info)
            } else {
                this.state = { version: 1, uid: randomUUID(), config: defaults, program: '' }
                this.save()
            }
            this.applyConfig()
            if (this.state.program) this.plc.downloadBytecodeHex(this.state.program)
            this.schedule()
            return this
        } catch (error) {
            this.close()
            throw error
        }
    }

    acquireLock() {
        const token = `${process.pid}:${randomUUID()}`
        for (let attempt = 0; attempt < 2; attempt++) {
            try {
                fs.writeFileSync(this.lockPath, token, { flag: 'wx', mode: 0o600 })
                this.lockToken = token
                return
            } catch (error) {
                if (error.code !== 'EEXIST') throw error
                const existing = fs.readFileSync(this.lockPath, 'utf8')
                const pid = Number(existing.split(':')[0])
                if (!Number.isInteger(pid) || pid <= 0) throw new Error('Invalid VirtualPLC lock file')
                try { process.kill(pid, 0) } catch (checkError) {
                    if (checkError.code !== 'ESRCH') throw checkError
                    if (fs.readFileSync(this.lockPath, 'utf8') === existing) fs.unlinkSync(this.lockPath)
                    continue
                }
                throw new Error('VirtualPLC data directory is already in use')
            }
        }
        throw new Error('Cannot acquire VirtualPLC lock')
    }

    validateConfig(config, info = this.plc.getInfo()) {
        if (!config || typeof config !== 'object' || Array.isArray(config)) throw new Error('Invalid configuration')
        const allowed = ['name', 'intervalMs', 'timerOffset', 'counterOffset', 'running', 'host', 'port', 'discoveryPort']
        if (Object.keys(config).some(key => !allowed.includes(key))) throw new Error('Unknown configuration option')
        const { name, intervalMs, timerOffset, counterOffset, running, host, port, discoveryPort } = config
        if (typeof name !== 'string' || !/^[a-zA-Z0-9 _.-]{1,48}$/.test(name)) throw new Error('Invalid device name')
        if (!Number.isInteger(intervalMs) || intervalMs < 1 || intervalMs > 60000) throw new Error('Invalid scan interval')
        if (typeof running !== 'boolean') throw new Error('Invalid running state')
        if (net.isIP(host) !== 4 || !Number.isInteger(port) || port < 1 || port > 65535 || !Number.isInteger(discoveryPort) || discoveryPort < 1 || discoveryPort > 65535) throw new Error('Invalid network settings')
        const timerSize = info.timer_count * info.timer_struct_size
        const counterSize = info.counter_count * info.counter_struct_size
        const minimum = info.marker_offset + info.marker_size
        if (!Number.isInteger(timerOffset) || !Number.isInteger(counterOffset) || timerOffset < minimum || counterOffset < minimum || timerOffset > 65535 || counterOffset > 65535 || timerOffset + timerSize > this.plc.dbGetLowestAddress() || counterOffset + counterSize > this.plc.dbGetLowestAddress() || (timerOffset < counterOffset + counterSize && counterOffset < timerOffset + timerSize)) throw new Error('Invalid timer or counter memory layout')
        return { name, intervalMs, timerOffset, counterOffset, running, host, port, discoveryPort }
    }

    save() {
        const temp = path.join(this.dataDir, `.state-${randomUUID()}.tmp`)
        try {
            fs.writeFileSync(temp, JSON.stringify(this.state), { mode: 0o600, flag: 'wx' })
            fs.renameSync(temp, this.statePath)
        } finally {
            if (fs.existsSync(temp)) fs.unlinkSync(temp)
        }
    }

    applyConfig() {
        const info = this.plc.getInfo()
        const { timerOffset, counterOffset } = this.state.config
        const areaEnd = Math.max(timerOffset + info.timer_count * info.timer_struct_size, counterOffset + info.counter_count * info.counter_struct_size)
        if (!this.plc.wasm_exports.db_setUserAreaEnd(areaEnd)) throw new Error('Invalid data-block memory layout')
        this.plc.wasm_exports.setRuntimeOffsetsEx(info.system_offset, info.input_offset, info.output_offset, info.marker_offset, timerOffset, counterOffset)
    }

    configure(changes) {
        if (!changes || typeof changes !== 'object' || Array.isArray(changes)) throw new Error('Invalid configuration')
        const previous = this.state.config
        const config = this.validateConfig({ ...previous, ...changes })
        if (this.networkActive && ['host', 'port', 'discoveryPort'].some(key => config[key] !== previous[key])) throw new Error('Network settings require a restart')
        this.state.config = config
        try { this.save() } catch (error) { this.state.config = previous; throw error }
        this.applyConfig()
        if (config.running) this.fault = null
        this.schedule()
        return { ...config }
    }

    download(program) {
        const hex = typeof program === 'string' ? program : Buffer.from(program).toString('hex')
        if (!/^(?:[0-9a-f]{2})+$/i.test(hex) || hex.length / 2 > this.plc.getInfo().program || hex.length + 1 > this.plc.wasm_exports.getHexDownloadBufferSize()) throw new Error('Invalid bytecode')
        const previous = this.state.program
        this.plc.downloadBytecodeHex(hex)
        this.state.program = hex.toUpperCase()
        try { this.save() } catch (error) {
            this.state.program = previous
            if (previous) this.plc.downloadBytecodeHex(previous)
            throw error
        }
    }

    schedule() {
        if (this.timer) clearInterval(this.timer)
        this.timer = null
        if (this.state.config.running) {
            this.timer = setInterval(() => {
                if (!this.state.program) return
                try {
                    const status = this.plc.run()
                    if (status !== 0) throw new Error(`Runtime status ${status}`)
                } catch (error) {
                    this.fault = error.message
                    this.state.config.running = false
                    clearInterval(this.timer)
                    this.timer = null
                    try { this.save() } catch (saveError) { console.error('VirtualPLC state save failed:', saveError.message) }
                    console.error('VirtualPLC stopped:', this.fault)
                }
            }, this.state.config.intervalMs)
        }
    }

    info() {
        const raw = this.plc.readCString(this.plc.wasm_exports.getInfoString())
        const fields = raw.slice(1, -1).split(',')
        fields[9] = String(Math.min(Number(fields[9]), (this.plc.wasm_exports.getHexDownloadBufferSize() - 1) / 2))
        fields[fields.length - 1] = this.state.config.name
        return `[${fields.join(',')}]`
    }

    close() {
        if (this.timer) clearInterval(this.timer)
        this.timer = null
        if (this.lockToken && fs.existsSync(this.lockPath) && fs.readFileSync(this.lockPath, 'utf8') === this.lockToken) fs.unlinkSync(this.lockPath)
        this.lockToken = null
    }
}