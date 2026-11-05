import net from 'node:net'
import dgram from 'node:dgram'

const MAX_FRAME = 140000
const hex32 = value => (value >>> 0).toString(16).padStart(8, '0').toUpperCase()
const hex16 = value => (value & 0xffff).toString(16).padStart(4, '0').toUpperCase()

export default class VirtualPLCServer {
    constructor(instance, { host = instance.state.config.host, port = instance.state.config.port, discoveryPort = instance.state.config.discoveryPort } = {}) {
        if (net.isIP(host) !== 4 || !Number.isInteger(port) || port < 0 || port > 65535 || !Number.isInteger(discoveryPort) || discoveryPort < 0 || discoveryPort > 65535) throw new Error('Invalid network settings')
        this.instance = instance
        this.host = host
        this.port = port
        this.discoveryPort = discoveryPort
        this.clients = new Set()
        this.discoverySessions = new Map()
        this.discoverySources = new Map()
        this.server = null
        this.discovery = null
    }

    async start() {
        this.server = net.createServer(socket => this.accept(socket))
        try {
            await new Promise((resolve, reject) => {
                this.server.once('error', reject)
                this.server.listen(this.port, this.host, () => { this.server.removeListener('error', reject); resolve() })
            })
            this.port = this.server.address().port
            this.discovery = dgram.createSocket({ type: 'udp4', reuseAddr: true })
            await new Promise((resolve, reject) => {
                this.discovery.once('error', reject)
                this.discovery.bind(this.discoveryPort, '0.0.0.0', () => { this.discovery.removeListener('error', reject); resolve() })
            })
            this.discoveryPort = this.discovery.address().port
            this.discovery.on('error', error => console.error('VirtualPLC discovery error:', error.message))
            this.discovery.on('message', (message, remote) => this.discover(message, remote))
            this.server.on('error', error => console.error('VirtualPLC TCP error:', error.message))
            this.instance.networkActive = true
            return this
        } catch (error) {
            await this.close()
            throw error
        }
    }

    discover(message, remote) {
        if (message.length > 1024) return
        try {
            const request = JSON.parse(message.toString('utf8'))
            if (request.magic !== 'VOVKPLC_DISCOVER' || request.version !== 1 || !/^[0-9a-f]{32}$/.test(request.session) || !/^[0-9a-f]{32}$/.test(request.nonce) || !Number.isSafeInteger(request.id) || request.id < 1) return
            const now = Date.now()
            const sessionKey = `${remote.address}:${request.session}`
            const previous = this.discoverySessions.get(sessionKey)
            if (previous && now - previous.time < 60000 && request.id <= previous.id) return
            if (now - (this.discoverySources.get(remote.address) || 0) < 100) return
            this.discoverySessions.delete(sessionKey)
            this.discoverySessions.set(sessionKey, { id: request.id, time: now })
            this.discoverySources.delete(remote.address)
            this.discoverySources.set(remote.address, now)
            if (this.discoverySessions.size > 256) this.discoverySessions.delete(this.discoverySessions.keys().next().value)
            if (this.discoverySources.size > 256) this.discoverySources.delete(this.discoverySources.keys().next().value)
            const reply = { magic: 'VOVKPLC_DEVICE', version: 1, session: request.session, id: request.id, nonce: request.nonce, uid: this.instance.state.uid, port: this.port, info: this.instance.info() }
            const data = Buffer.from(JSON.stringify(reply))
            if (this.host === '0.0.0.0') this.discovery.send(data, remote.port, remote.address, () => {})
            else {
                const sender = dgram.createSocket('udp4')
                sender.on('error', () => { try { sender.close() } catch {} })
                sender.bind(0, this.host, () => sender.send(data, remote.port, remote.address, () => sender.close()))
            }
        } catch {}
    }

    accept(socket) {
        this.clients.add(socket)
        socket.setEncoding('utf8')
        socket.on('error', () => {})
        socket.on('close', () => this.clients.delete(socket))
        let buffer = ''
        socket.on('data', data => {
            buffer += data
            if (buffer.length > MAX_FRAME) return socket.destroy()
            let end
            while ((end = buffer.indexOf('\n')) !== -1) {
                const line = buffer.slice(0, end).replace(/\r$/, '')
                buffer = buffer.slice(end + 1)
                let response
                try { response = this.command(line) } catch { response = 'ERR COMMAND' }
                socket.write(`${response}\n`)
            }
        })
    }

    command(line) {
        if (line === '?') return '<VovkPLC>'
        if (!/^[A-Z]{2}(?:[0-9a-fA-F]{2})+$/.test(line)) return 'ERR COMMAND'
        const command = line.slice(0, 2)
        const bytes = Buffer.from(line.slice(2), 'hex')
        const checksum = bytes[bytes.length - 1]
        const payload = bytes.subarray(0, -1)
        if (this.instance.plc.crc8([...Buffer.from(command), ...payload]) !== checksum) return 'Invalid checksum'
        const plc = this.instance.plc
        const noPayload = () => { if (payload.length !== 0) throw new Error('Invalid length') }
        if (command === 'PI') { noPayload(); return `PLC INFO - ${this.instance.info()}` }
        if (command === 'SL') { noPayload(); return '[PS,0]' }
        if (command === 'TI') { noPayload(); return '[TI,0]' }
        if (command === 'VG') { noPayload(); return `OK ${JSON.stringify({ uid: this.instance.state.uid, config: this.instance.state.config, fault: this.instance.fault })}` }
        if (command === 'PH') { noPayload(); return `PH${Object.values(plc.getDeviceHealth()).map(hex32).join('')}` }
        if (command === 'RH') { noPayload(); plc.resetDeviceHealth(); return 'OK HEALTH RESET' }
        if (command === 'DA') {
            noPayload()
            const values = [plc.dbGetSlotCount(), plc.dbGetActiveCount(), plc.dbGetTableOffset(), plc.dbGetFreeSpace(), plc.dbGetLowestAddress()]
            for (const entry of plc.dbGetActiveEntries()) values.push(entry.db_number, entry.offset, entry.size)
            return `DA${values.map(hex16).join('')}`
        }
        if (command === 'PU') { noPayload(); return `OK ${this.instance.state.program}` }
        if (command === 'PR' || command === 'PS') {
            noPayload()
            this.instance.configure({ running: command === 'PR' })
            return command === 'PR' ? 'OK PROGRAM RUN' : 'OK PROGRAM STOP'
        }
        if (command === 'TC') {
            if (payload.length !== 4) throw new Error('Invalid length')
            const timerOffset = payload.readUInt16BE(0)
            const counterOffset = payload.readUInt16BE(2)
            this.instance.configure({ timerOffset, counterOffset })
            return `OK TC CONFIG T=${timerOffset} C=${counterOffset}`
        }
        if (command === 'PD' || command === 'VC') {
            if (payload.length < 4 || payload.readUInt32BE(0) !== payload.length - 4) throw new Error('Invalid length')
            if (command === 'PD') {
                this.instance.download(payload.subarray(4))
                return 'PROGRAM DOWNLOAD COMPLETE'
            }
            if (payload.length > 2052) throw new Error('Invalid length')
            const changes = JSON.parse(payload.subarray(4).toString('utf8'))
            this.instance.configure(changes)
            return 'OK VIRTUAL CONFIG'
        }
        if (['MR', 'MW', 'MM', 'MF'].includes(command)) {
            if (payload.length < 8) throw new Error('Invalid length')
            const address = payload.readUInt32BE(0)
            const size = payload.readUInt32BE(4)
            const expected = 8 + (command === 'MW' ? size : command === 'MM' ? 2 * size : command === 'MF' ? 1 : 0)
            if (payload.length !== expected || size > 16384 || address + size > plc.getInfo().memory) throw new Error('Invalid memory range')
            if (command === 'MR') return `OK ${Buffer.from(plc.readMemoryArea(address, size)).toString('hex').toUpperCase()}`
            if (command === 'MW') plc.writeMemoryArea(address, [...payload.subarray(8)])
            if (command === 'MM') plc.writeMemoryAreaMasked(address, [...payload.subarray(8, 8 + size)], [...payload.subarray(8 + size)])
            if (command === 'MF') plc.writeMemoryArea(address, Array(size).fill(payload[8]))
            return command === 'MW' ? 'OK MEMORY WRITE' : command === 'MM' ? 'OK MEMORY WRITE MASK' : 'OK MEMORY FORMAT'
        }
        return 'ERR UNSUPPORTED'
    }

    async close() {
        this.instance.networkActive = false
        for (const client of this.clients) client.destroy()
        if (this.discovery) {
            try { this.discovery.close() } catch {}
            this.discovery = null
        }
        if (this.server) {
            await new Promise(resolve => this.server.close(() => resolve()))
            this.server = null
        }
    }
}