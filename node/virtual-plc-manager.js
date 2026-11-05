import os from 'node:os'
import path from 'node:path'
import { parseArgs } from 'node:util'
import VirtualPLCManager from './VirtualPLCManager.js'
import VirtualPLCManagementServer from './VirtualPLCManagementServer.js'

const { values, positionals } = parseArgs({ allowPositionals: true, options: {
    'data-dir': { type: 'string' },
    port: { type: 'string', default: '19615' },
    id: { type: 'string' },
    config: { type: 'string' },
    help: { type: 'boolean' },
} })

const main = async () => {
    const action = positionals[0]
    if (values.help || !action) {
        console.log('Usage: node node/virtual-plc-manager.js serve [--data-dir DIR] [--port MANAGEMENT_PORT]\n       node node/virtual-plc-manager.js list [--port MANAGEMENT_PORT]\n       node node/virtual-plc-manager.js create --config JSON [--port MANAGEMENT_PORT]\n       node node/virtual-plc-manager.js configure --id ID --config JSON [--port MANAGEMENT_PORT]\n       node node/virtual-plc-manager.js start|stop|remove --id ID [--port MANAGEMENT_PORT]')
        return
    }
    const port = Number(values.port)
    if (!Number.isInteger(port) || port < 1 || port > 65535) throw new Error('Invalid management port')
    if (action === 'serve') {
        const directory = values['data-dir'] || path.join(os.homedir(), '.vovkplc', 'manager')
        const manager = await new VirtualPLCManager(directory).initialize()
        try {
            const server = await new VirtualPLCManagementServer(manager, port).start()
            console.log(`VirtualPLC manager listening on http://127.0.0.1:${server.port}; state ${directory}`)
            const shutdown = async () => { await server.close(); await manager.close() }
            process.once('SIGINT', shutdown)
            process.once('SIGTERM', shutdown)
        } catch (error) {
            await manager.close()
            throw error
        }
        return
    }
    const actions = { list: ['GET', '/instances'], create: ['POST', '/instances'], configure: ['PATCH', `/instances/${values.id || ''}`], start: ['POST', `/instances/${values.id || ''}/start`], stop: ['POST', `/instances/${values.id || ''}/stop`], remove: ['DELETE', `/instances/${values.id || ''}`] }
    if (!Object.hasOwn(actions, action)) throw new Error('Unknown action')
    if (['configure', 'start', 'stop', 'remove'].includes(action) && !/^[a-f0-9-]{36}$/.test(values.id || '')) throw new Error('A valid --id is required')
    let body
    if (action === 'create' || action === 'configure') {
        if (!values.config) throw new Error('--config JSON is required')
        body = JSON.parse(values.config)
    }
    const [method, endpoint] = actions[action]
    const response = await fetch(`http://127.0.0.1:${port}${endpoint}`, { method, headers: body ? { 'Content-Type': 'application/json' } : {}, body: body ? JSON.stringify(body) : undefined, signal: AbortSignal.timeout(10000) })
    const result = await response.json()
    if (!response.ok || !result.ok) throw new Error(result.error || 'Management request failed')
    console.log(JSON.stringify(result.instances ?? result.result ?? { ok: true }, null, 2))
}

main().catch(error => { console.error(error.message); process.exitCode = 1 })