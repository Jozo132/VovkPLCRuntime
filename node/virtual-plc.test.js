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
import VirtualPLC from './VirtualPLC.js'
import VirtualPLCServer from './VirtualPLCServer.js'

const execFileAsync = promisify(execFile)

test('virtual PLC executes bytecode and restores identity, configuration and program', async () => {
    const directory = fs.mkdtempSync(path.join(os.tmpdir(), 'vovk-virtual-plc-'))
    let instance
    try {
        instance = await new VirtualPLC(directory).initialize()
        const uid = instance.state.uid
        await assert.rejects(new VirtualPLC(directory).initialize(), /already in use/)
        const program = instance.plc.compilePLCASM('ptr.const 192\nu8.const 42\nu8.move\nexit').output
        instance.download(program)
        instance.configure({ name: 'Test PLC', intervalMs: 1, timerOffset: 500, counterOffset: 700 })
        assert.equal(instance.plc.dbGetFreeSpace(), instance.plc.dbGetLowestAddress() - 780)
        await new Promise(resolve => setTimeout(resolve, 20))
        assert.equal(instance.plc.readMemoryArea(192, 1)[0], 42)
        assert.match(instance.info(), /,500,16,9,700,16,5,.*Test PLC\]$/)
        const save = instance.save
        instance.save = () => { throw new Error('disk failure') }
        assert.throws(() => instance.download('FF'), /disk failure/)
        assert.equal(instance.state.program, program)
        instance.save = save
        instance.close()
        instance = await new VirtualPLC(directory).initialize()
        assert.equal(instance.state.uid, uid)
        assert.equal(instance.state.program, program)
        assert.equal(instance.state.config.intervalMs, 1)
        await new Promise(resolve => setTimeout(resolve, 20))
        assert.equal(instance.plc.readMemoryArea(192, 1)[0], 42)
        assert.throws(() => instance.configure({ timerOffset: 700 }), /layout/)
        assert.ok(instance.plc.dbDeclare(1, 1000) >= 0)
        assert.throws(() => instance.configure({ timerOffset: 64500, counterOffset: 64800 }), /layout/)
    } finally {
        instance?.close()
        fs.rmSync(directory, { recursive: true, force: true })
    }
})

test('CLI changes a running instance and restores its UID and settings', async () => {
    const directory = fs.mkdtempSync(path.join(os.tmpdir(), 'vovk-virtual-cli-'))
    const cli = fileURLToPath(new URL('./virtual-plc.js', import.meta.url))
    const reserve = net.createServer()
    await new Promise(resolve => reserve.listen(0, '127.0.0.1', resolve))
    const port = reserve.address().port
    await new Promise(resolve => reserve.close(resolve))
    const udp = dgram.createSocket('udp4')
    await new Promise(resolve => udp.bind(0, '127.0.0.1', resolve))
    const discoveryPort = udp.address().port
    udp.close()
    let child
    const start = () => new Promise((resolve, reject) => {
        child = spawn(process.execPath, [cli, '--data-dir', directory, '--host', '127.0.0.1', '--port', String(port), '--discovery-port', String(discoveryPort)])
        let output = ''
        child.stderr.on('data', data => { output += data })
        child.once('error', reject)
        child.once('exit', code => reject(new Error(`VirtualPLC exited ${code}: ${output}`)))
        child.stdout.once('data', () => resolve())
    })
    const stop = async () => {
        if (!child || child.exitCode !== null) return
        const stopped = new Promise(resolve => child.once('exit', resolve))
        child.kill('SIGTERM')
        await stopped
    }
    const invoke = async (...args) => (await execFileAsync(process.execPath, [cli, ...args, '--port', String(port)])).stdout.trim()
    try {
        await start()
        const first = JSON.parse((await invoke('--inspect')).slice(3))
        assert.equal(await invoke('--configure', '{"name":"CLI PLC","intervalMs":25}'), 'OK VIRTUAL CONFIG')
        await stop()
        await start()
        const second = JSON.parse((await invoke('--inspect')).slice(3))
        assert.equal(second.uid, first.uid)
        assert.equal(second.config.name, 'CLI PLC')
        assert.equal(second.config.intervalMs, 25)
    } finally {
        await stop()
        fs.rmSync(directory, { recursive: true, force: true })
    }
})

test('scan faults stop the instance and persist the stopped state', async () => {
    const directory = fs.mkdtempSync(path.join(os.tmpdir(), 'vovk-virtual-fault-'))
    let instance
    try {
        instance = await new VirtualPLC(directory).initialize()
        instance.download('FF')
        instance.plc.run = () => { throw new Error('test scan fault') }
        await new Promise(resolve => setTimeout(resolve, 30))
        assert.equal(instance.state.config.running, false)
        assert.equal(instance.fault, 'test scan fault')
        assert.equal(JSON.parse(fs.readFileSync(instance.statePath)).config.running, false)
    } finally {
        instance?.close()
        fs.rmSync(directory, { recursive: true, force: true })
    }
})

test('virtual PLC serves checksummed commands and discovers multiple instances over broadcast', async () => {
    const directory = fs.mkdtempSync(path.join(os.tmpdir(), 'vovk-virtual-network-'))
    const instances = []
    const servers = []
    const discovery = dgram.createSocket('udp4')
    let socket
    try {
        for (const name of ['first', 'second']) instances.push(await new VirtualPLC(path.join(directory, name)).initialize())
        servers.push(await new VirtualPLCServer(instances[0], { host: '127.0.0.1', port: 0, discoveryPort: 0 }).start())
        servers.push(await new VirtualPLCServer(instances[1], { host: '127.0.0.1', port: 0, discoveryPort: servers[0].discoveryPort }).start())
        assert.notEqual(instances[0].state.uid, instances[1].state.uid)
        await new Promise(resolve => discovery.bind(0, '127.0.0.1', resolve))
        discovery.setBroadcast(true)
        const request = { magic: 'VOVKPLC_DISCOVER', version: 1, session: randomBytes(16).toString('hex'), id: 1, nonce: randomBytes(16).toString('hex') }
        const replies = new Promise((resolve, reject) => {
            const found = []
            const timeout = setTimeout(() => reject(new Error('Discovery timed out')), 2000)
            discovery.on('message', data => {
                found.push(JSON.parse(data))
                if (found.length === 2) { clearTimeout(timeout); resolve(found) }
            })
        })
        discovery.send(Buffer.from(JSON.stringify(request)), servers[0].discoveryPort, '127.255.255.255')
        const found = await replies
        assert.deepEqual(new Set(found.map(reply => reply.uid)), new Set(instances.map(instance => instance.state.uid)))
        assert.ok(found.every(reply => reply.nonce === request.nonce && reply.session === request.session && reply.id === request.id))
        socket = net.createConnection(servers[0].port, '127.0.0.1')
        await new Promise((resolve, reject) => { socket.once('connect', resolve); socket.once('error', reject) })
        let pending = ''
        const exchange = command => new Promise(resolve => {
            const receive = data => {
                pending += data.toString()
                const end = pending.indexOf('\n')
                if (end < 0) return
                socket.off('data', receive)
                const result = pending.slice(0, end)
                pending = pending.slice(end + 1)
                resolve(result)
            }
            socket.on('data', receive)
            socket.write(`${command}\n`)
        })
        const plc = instances[0].plc
        const simple = command => command + plc.crc8([...Buffer.from(command)]).toString(16).padStart(2, '0')
        assert.match(await exchange(simple('PI')), /\[VovkPLCRuntime,/)
        assert.match(await exchange(simple('DA')), /^DA[0-9A-F]{20,}$/)
        assert.equal(await exchange('PI00'), 'Invalid checksum')
        assert.equal(await exchange(plc.buildCommand.memoryWrite(192, [17])), 'OK MEMORY WRITE')
        assert.equal(await exchange(plc.buildCommand.memoryRead(192, 1)), 'OK 11')
        assert.equal(await exchange(plc.buildCommand.tcConfig(500, 700)), 'OK TC CONFIG T=500 C=700')
        const program = plc.compilePLCASM('ptr.const 192\nu8.const 42\nu8.move\nexit').output
        assert.equal(await exchange(plc.buildCommand.programDownload(program)), 'PROGRAM DOWNLOAD COMPLETE')
        assert.equal(await exchange(plc.buildCommand.programUpload()), `OK ${program}`)
        assert.equal(await exchange(plc.buildCommand.programStop()), 'OK PROGRAM STOP')
        assert.equal(await exchange(plc.buildCommand.programRun()), 'OK PROGRAM RUN')
        const changes = Buffer.from(JSON.stringify({ name: 'Network PLC', intervalMs: 5 }))
        const payload = Buffer.alloc(4 + changes.length)
        payload.writeUInt32BE(changes.length)
        changes.copy(payload, 4)
        const checksum = plc.crc8([...Buffer.from('VC'), ...payload]).toString(16).padStart(2, '0')
        assert.equal(await exchange(`VC${payload.toString('hex')}${checksum}`), 'OK VIRTUAL CONFIG')
        assert.equal(instances[0].state.config.name, 'Network PLC')
        assert.match(await exchange(simple('VG')), /Network PLC/)
        assert.throws(() => instances[0].configure({ port: 2000 }), /restart/)
        const largeProgram = 'FF'.repeat(64535)
        assert.equal(await exchange(plc.buildCommand.programDownload(largeProgram)), 'PROGRAM DOWNLOAD COMPLETE')
        assert.equal(await exchange(plc.buildCommand.programUpload()), `OK ${largeProgram}`)
    } finally {
        socket?.destroy()
        discovery.close()
        for (const server of servers) await server.close()
        for (const instance of instances) instance.close()
        fs.rmSync(directory, { recursive: true, force: true })
    }
})