import test from 'node:test'
import assert from 'node:assert/strict'
import fs from 'node:fs'
import os from 'node:os'
import path from 'node:path'
import net from 'node:net'
import dgram from 'node:dgram'
import { randomBytes } from 'node:crypto'
import { spawn, execFile } from 'node:child_process'
import { promisify } from 'node:util'
import { fileURLToPath } from 'node:url'
import VirtualPLCManager from './VirtualPLCManager.js'
import VirtualPLCManagementServer from './VirtualPLCManagementServer.js'
import VovkPLC from '../wasm/dist/VovkPLC.js'

const execFileAsync = promisify(execFile)

const availablePort = async () => {
    const server = net.createServer()
    await new Promise(resolve => server.listen(0, '127.0.0.1', resolve))
    const port = server.address().port
    await new Promise(resolve => server.close(resolve))
    return port
}

const exchange = (host, port, command) => new Promise((resolve, reject) => {
    const socket = net.createConnection(port, host)
    let response = ''
    socket.setTimeout(3000, () => socket.destroy(new Error('PLC response timed out')))
    socket.on('error', reject)
    socket.on('connect', () => socket.write(`${command}\n`))
    socket.on('data', data => {
        response += data.toString()
        if (response.includes('\n')) { socket.destroy(); resolve(response.split('\n')[0]) }
    })
})

test('manager runs independent workers on different IPs with the same port', async () => {
    const directory = fs.mkdtempSync(path.join(os.tmpdir(), 'vovk-manager-'))
    const reserve = net.createServer()
    await new Promise(resolve => reserve.listen(0, '127.0.0.1', resolve))
    const port = reserve.address().port
    await new Promise(resolve => reserve.close(resolve))
    const udp = dgram.createSocket('udp4')
    await new Promise(resolve => udp.bind(0, '127.0.0.1', resolve))
    const discoveryPort = udp.address().port
    udp.close()
    let manager = await new VirtualPLCManager(directory).initialize()
    try {
        const first = await manager.create({ host: '127.0.0.1', port, discoveryPort })
        const second = await manager.create({ host: '127.0.0.2', port, discoveryPort })
        const thirdPort = await availablePort()
        const third = await manager.create({ host: '127.0.0.1', port: thirdPort, discoveryPort })
        assert.notEqual(first.uid, second.uid)
        assert.equal(new Set([first.uid, second.uid, third.uid]).size, 3)
        assert.equal(manager.workers.size, 3)
        const receiver = dgram.createSocket('udp4')
        try {
            await new Promise(resolve => receiver.bind(0, '127.0.0.1', resolve))
            receiver.setBroadcast(true)
            const request = { magic: 'VOVKPLC_DISCOVER', version: 1, session: randomBytes(16).toString('hex'), id: 1, nonce: randomBytes(16).toString('hex') }
            const replies = new Promise((resolve, reject) => {
                const found = []
                const timeout = setTimeout(() => reject(new Error('Discovery timed out')), 2000)
                receiver.on('message', (data, remote) => {
                    found.push({ ...JSON.parse(data), host: remote.address })
                    if (found.length === 3) { clearTimeout(timeout); resolve(found) }
                })
            })
            receiver.send(Buffer.from(JSON.stringify(request)), discoveryPort, '127.255.255.255')
            const found = await replies
            assert.deepEqual(new Set(found.map(item => `${item.host}:${item.port}`)), new Set([`127.0.0.1:${port}`, `127.0.0.2:${port}`, `127.0.0.1:${thirdPort}`]))
            assert.deepEqual(new Set(found.map(item => item.uid)), new Set([first.uid, second.uid, third.uid]))
        } finally { receiver.close() }
        for (const host of ['127.0.0.1', '127.0.0.2']) {
            const socket = net.createConnection(port, host)
            await new Promise((resolve, reject) => { socket.once('connect', resolve); socket.once('error', reject) })
            socket.destroy()
        }
        const protocol = new VovkPLC()
        const firstProgram = '01C000032A1203FF'
        const secondProgram = '01C000032B1203FF'
        assert.equal(await exchange('127.0.0.1', port, protocol.buildCommand.programDownload(firstProgram)), 'PROGRAM DOWNLOAD COMPLETE')
        assert.equal(await exchange('127.0.0.2', port, protocol.buildCommand.programDownload(secondProgram)), 'PROGRAM DOWNLOAD COMPLETE')
        await new Promise(resolve => setTimeout(resolve, 30))
        assert.equal(await exchange('127.0.0.1', port, protocol.buildCommand.memoryRead(192, 1)), 'OK 2A')
        assert.equal(await exchange('127.0.0.2', port, protocol.buildCommand.memoryRead(192, 1)), 'OK 2B')
        assert.equal(await exchange('127.0.0.1', thirdPort, protocol.buildCommand.programUpload()), 'OK ')
        await manager.configure(first.id, { name: 'First PLC' })
        await assert.rejects(manager.configure(first.id, { host: '127.0.0.2' }), /address already in use/i)
        assert.equal(manager.list().find(item => item.id === first.id).config.host, '127.0.0.1')
        const newPort = await availablePort()
        await manager.configure(first.id, { port: newPort })
        assert.equal(await exchange('127.0.0.1', newPort, protocol.buildCommand.programUpload()), `OK ${firstProgram}`)
        await manager.configure(first.id, { port })
        await manager.workers.get(second.id).worker.terminate()
        assert.equal(await exchange('127.0.0.1', port, protocol.buildCommand.programUpload()), `OK ${firstProgram}`)
        assert.equal((await manager.start(second.id)).uid, second.uid)
        assert.equal(await exchange('127.0.0.2', port, protocol.buildCommand.programUpload()), `OK ${secondProgram}`)
        await manager.stop(first.id)
        await manager.configure(first.id, { name: 'Stopped PLC' })
        const restarted = await manager.start(first.id)
        assert.equal(restarted.uid, first.uid)
        assert.equal(restarted.config.name, 'Stopped PLC')
        await manager.close()
        manager = await new VirtualPLCManager(directory).initialize()
        assert.equal(manager.workers.size, 3)
        assert.deepEqual(new Set(manager.list().map(item => item.uid)), new Set([first.uid, second.uid, third.uid]))
        assert.equal(await exchange('127.0.0.1', port, protocol.buildCommand.programUpload()), `OK ${firstProgram}`)
        assert.equal(await exchange('127.0.0.2', port, protocol.buildCommand.programUpload()), `OK ${secondProgram}`)
        await manager.remove(first.id)
        assert.equal(manager.list().length, 2)
        assert.ok(fs.existsSync(path.join(directory, 'instances', first.id, 'state.json')))
    } finally {
        await manager.close()
        fs.rmSync(directory, { recursive: true, force: true })
    }
})

test('manager CLI creates instances and restores them across service restarts', async () => {
    const directory = fs.mkdtempSync(path.join(os.tmpdir(), 'vovk-manager-cli-'))
    const cli = fileURLToPath(new URL('./virtual-plc-manager.js', import.meta.url))
    const managementPort = await availablePort()
    const plcPort = await availablePort()
    let child
    const start = () => new Promise((resolve, reject) => {
        child = spawn(process.execPath, [cli, 'serve', '--data-dir', directory, '--port', String(managementPort)])
        let errors = ''
        child.stderr.on('data', data => { errors += data })
        child.once('error', reject)
        child.once('exit', code => reject(new Error(`Manager exited ${code}: ${errors}`)))
        child.stdout.once('data', resolve)
    })
    const stop = async () => {
        if (!child || child.exitCode !== null) return
        const exited = new Promise(resolve => child.once('exit', resolve))
        child.kill('SIGTERM')
        await exited
    }
    const run = async (...args) => JSON.parse((await execFileAsync(process.execPath, [cli, ...args, '--port', String(managementPort)])).stdout)
    try {
        await start()
        const created = await run('create', '--config', JSON.stringify({ host: '127.0.0.1', port: plcPort }))
        assert.equal((await run('list'))[0].uid, created.uid)
        await run('configure', '--id', created.id, '--config', '{"name":"CLI Managed PLC"}')
        await stop()
        await start()
        const restored = (await run('list'))[0]
        assert.equal(restored.uid, created.uid)
        assert.equal(restored.config.name, 'CLI Managed PLC')
        assert.equal(restored.active, true)
        await run('stop', '--id', created.id)
        assert.equal((await run('list'))[0].active, false)
    } finally {
        await stop()
        fs.rmSync(directory, { recursive: true, force: true })
    }
})

test('local management API creates and controls instances while running', async () => {
    const directory = fs.mkdtempSync(path.join(os.tmpdir(), 'vovk-manager-api-'))
    const reserve = net.createServer()
    await new Promise(resolve => reserve.listen(0, '127.0.0.1', resolve))
    const port = reserve.address().port
    await new Promise(resolve => reserve.close(resolve))
    const manager = await new VirtualPLCManager(directory).initialize()
    const api = await new VirtualPLCManagementServer(manager, 0).start()
    const request = async (method, endpoint, body) => {
        const response = await fetch(`http://127.0.0.1:${api.port}${endpoint}`, { method, headers: body ? { 'Content-Type': 'application/json' } : {}, body: body ? JSON.stringify(body) : undefined })
        return response.json()
    }
    try {
        const created = await request('POST', '/instances', { host: '127.0.0.1', port })
        assert.equal(created.ok, true)
        const id = created.result.id
        assert.equal((await request('PATCH', `/instances/${id}`, { name: 'API PLC' })).result.name, 'API PLC')
        assert.equal((await request('POST', `/instances/${id}/stop`)).ok, true)
        assert.equal((await request('GET', '/instances')).instances[0].active, false)
        assert.equal((await request('POST', `/instances/${id}/start`)).result.uid, created.result.uid)
        assert.equal((await request('DELETE', `/instances/${id}`)).ok, true)
        assert.equal((await request('GET', '/instances')).instances.length, 0)
        const blocked = await fetch(`http://127.0.0.1:${api.port}/instances`, { headers: { Origin: 'https://example.com' } })
        assert.equal(blocked.status, 403)
    } finally {
        await api.close()
        await manager.close()
        fs.rmSync(directory, { recursive: true, force: true })
    }
})

test('manager can be embedded in a Node eval module', async () => {
    const directory = fs.mkdtempSync(path.join(os.tmpdir(), 'vovk-manager-embedded-'))
    const port = await availablePort()
    const moduleUrl = new URL('./VirtualPLCManager.js', import.meta.url).href
    const script = `import Manager from ${JSON.stringify(moduleUrl)}; const manager = await new Manager(${JSON.stringify(directory)}).initialize(); try { const plc = await manager.create({host: '127.0.0.1', port: ${port}}); console.log(plc.uid); } finally { await manager.close(); }`
    try {
        const result = await execFileAsync(process.execPath, ['--input-type=module', '--eval', script])
        assert.match(result.stdout.trim(), /^[a-f0-9-]{36}$/)
    } finally { fs.rmSync(directory, { recursive: true, force: true }) }
})