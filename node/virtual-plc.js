import os from 'node:os'
import path from 'node:path'
import net from 'node:net'
import { parseArgs } from 'node:util'
import VovkPLC from '../wasm/dist/VovkPLC.js'
import VirtualPLC from './VirtualPLC.js'
import VirtualPLCServer from './VirtualPLCServer.js'

const { values } = parseArgs({ options: {
    instance: { type: 'string', default: 'default' },
    'data-dir': { type: 'string' },
    host: { type: 'string' },
    port: { type: 'string' },
    'discovery-port': { type: 'string' },
    configure: { type: 'string' },
    inspect: { type: 'boolean' },
    help: { type: 'boolean' },
} })

const main = async () => {
    if (values.help) {
        console.log('Usage: node node/virtual-plc.js [--instance NAME] [--data-dir DIR] [--host IP] [--port PORT] [--discovery-port PORT]\n       node node/virtual-plc.js --configure JSON [--host IP] [--port PORT]\n       node node/virtual-plc.js --inspect [--host IP] [--port PORT]')
        return
    }
    if (values.configure !== undefined || values.inspect) {
        if (values.configure !== undefined && values.inspect) throw new Error('Choose either --configure or --inspect')
        const plc = new VovkPLC()
        let command = 'VG'
        let payload = Buffer.alloc(0)
        if (values.configure !== undefined) {
            JSON.parse(values.configure)
            command = 'VC'
            const data = Buffer.from(values.configure)
            payload = Buffer.alloc(4 + data.length)
            payload.writeUInt32BE(data.length)
            data.copy(payload, 4)
        }
        const checksum = plc.crc8([...Buffer.from(command), ...payload]).toString(16).padStart(2, '0')
        const response = await new Promise((resolve, reject) => {
            const socket = net.createConnection({ host: values.host || '127.0.0.1', port: Number(values.port || 19616) })
            let buffer = ''
            socket.setTimeout(5000, () => socket.destroy(new Error('Connection timed out')))
            socket.on('error', reject)
            socket.on('connect', () => socket.write(`${command}${payload.toString('hex')}${checksum}\n`))
            socket.on('data', data => {
                buffer += data.toString()
                if (buffer.length > 4096) socket.destroy(new Error('Response too large'))
                const end = buffer.indexOf('\n')
                if (end !== -1) { socket.destroy(); resolve(buffer.slice(0, end)) }
            })
            socket.on('end', () => reject(new Error('Connection closed')))
        })
        if (!response.startsWith('OK ')) throw new Error(response)
        console.log(response)
        return
    }
    if (!/^[a-zA-Z0-9_-]{1,64}$/.test(values.instance)) throw new Error('Invalid instance name')
    const directory = values['data-dir'] || path.join(os.homedir(), '.vovkplc', 'virtualplc', values.instance)
    const instance = await new VirtualPLC(directory).initialize()
    try {
        const settings = {}
        if (values.host !== undefined) settings.host = values.host
        if (values.port !== undefined) settings.port = Number(values.port)
        if (values['discovery-port'] !== undefined) settings.discoveryPort = Number(values['discovery-port'])
        if (Object.keys(settings).length) instance.configure(settings)
        const server = await new VirtualPLCServer(instance).start()
        console.log(`VirtualPLC ${instance.state.uid} listening on ${server.host}:${server.port}; discovery UDP ${server.discoveryPort}; state ${instance.statePath}`)
        const shutdown = async () => { await server.close(); instance.close() }
        process.once('SIGINT', shutdown)
        process.once('SIGTERM', shutdown)
    } catch (error) {
        instance.close()
        throw error
    }
}

main().catch(error => { console.error(error.message); process.exitCode = 1 })