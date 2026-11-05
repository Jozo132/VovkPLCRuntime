import test from 'node:test'
import assert from 'node:assert/strict'
import fs from 'node:fs/promises'
import os from 'node:os'
import path from 'node:path'
import { createHash, randomUUID } from 'node:crypto'
import { Readable } from 'node:stream'
import http from 'node:http'
import { execFile } from 'node:child_process'
import { promisify } from 'node:util'
import { createFirmwareBuilder, FileJobStore, LocalExecutor } from './firmware-builder.js'
import { createFirmwareHttpHandler } from './firmware/http.js'
import { esp32Manifest, parseEsptoolPreview } from './firmware/platformio.js'
import { zip } from './firmware/project-loader.test.js'

const source = () => Readable.from([zip([['project/platformio.ini', '[env:demo]\nplatform = espressif32'], ['project/src/main.cpp', 'void setup() {}'], ['project/lib/runtime/library.properties', 'name=VovkPLCRuntime\nversion=2.3.4\n']])])
const finished = async (builder, id) => { const events = []; for await (const event of builder.events(id)) events.push(event); return events }

class FakeExecutor {
    constructor({ wait = false, fail = false, environments = ['demo'] } = {}) { this.wait = wait; this.fail = fail; this.environments = environments }
    async run({ args, env, onOutput = async () => {}, signal }) {
        if (args[0] === '--version') return { code: 0, stdout: 'PlatformIO Core, version fake' }
        if (args[0] === 'project') return { code: 0, stdout: JSON.stringify(this.environments.map(name => [`env:${name}`, [['board', 'esp32dev'], ['framework', ['espidf']]]])) }
        if (args[0] === 'system') { await fs.mkdir(path.join(env.PLATFORMIO_CORE_DIR, 'packages'), { recursive: true }); return { code: 0, stdout: JSON.stringify({ core_dir: { value: env.PLATFORMIO_CORE_DIR } }) } }
        if (args[0] === 'run' && args.includes('upload')) {
            const output = path.join(env.PLATFORMIO_BUILD_DIR, args[args.indexOf('--environment') + 1])
            return { code: 0, stdout: `python /opt/esptool.py --chip esp32 --port vovkplc-dry-run write_flash --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 "${path.join(output, 'bootloader', 'bootloader.bin')}" 0x10000 "${path.join(output, 'firmware.bin')}"` }
        }
        assert.deepEqual(args.slice(-2), ['--target', 'buildprog'])
        await onOutput('stdout', Buffer.from('\u001b[31mmain.cpp:4:2: war'))
        await onOutput('stdout', Buffer.from('ning: test\u001b[0m\rprogress\n'))
        if (this.wait) await new Promise((resolve, reject) => { if (signal.aborted) reject(signal.reason); else signal.addEventListener('abort', () => reject(signal.reason), { once: true }) })
        if (this.fail) return { code: 1, signal: null }
        const output = path.join(env.PLATFORMIO_BUILD_DIR, args[args.indexOf('--environment') + 1])
        await fs.mkdir(path.join(output, 'bootloader'), { recursive: true })
        await fs.mkdir(path.join(output, 'CMakeFiles'), { recursive: true })
        await fs.writeFile(path.join(output, 'CMakeFiles', 'compiler-probe.bin'), 'not firmware')
        await fs.writeFile(path.join(output, 'firmware.bin'), 'firmware')
        await fs.writeFile(path.join(output, 'firmware.elf'), 'elf')
        await fs.writeFile(path.join(output, 'bootloader', 'bootloader.bin'), 'bootloader')
        await fs.writeFile(path.join(output, 'flasher_args.json'), JSON.stringify({ flash_files: { '0x1000': 'bootloader/bootloader.bin', '0x10000': 'firmware.bin' }, flash_settings: { flash_mode: 'dio', flash_freq: '40m', flash_size: '4MB' }, extra_esptool_args: { chip: 'esp32' } }))
        return { code: 0, signal: null }
    }
}

test('build lifecycle, event replay, diagnostics and artifact integrity', async () => {
    const directory = await fs.mkdtemp(path.join(os.tmpdir(), 'firmware-builder-'))
    const builder = await createFirmwareBuilder({ directory, executor: new FakeExecutor() })
    try {
        assert.equal((await builder.checkAdapter()).available, true)
        const id = await builder.submit({ source: source() })
        const live = builder.events(id)[Symbol.asyncIterator]()
        const first = await live.next()
        const expected = await finished(builder, id)
        const events = [first.value]
        for await (const event of { [Symbol.asyncIterator]: () => live }) events.push(event)
        assert.deepEqual(events, expected)
        assert.deepEqual(events.filter(event => event.type === 'state').map(event => event.payload.state), ['queued', 'extracting', 'preparing', 'building', 'collecting', 'succeeded'])
        assert.deepEqual(events.map(event => event.sequence), events.map((_, index) => index + 1))
        assert.equal(events.find(event => event.type === 'diagnostic').payload.message, 'test')
        const replay = []
        for await (const event of builder.events(id, { after: 3 })) replay.push(event)
        assert.deepEqual(replay, events.slice(3))
        const files = await builder.listArtifacts(id)
        assert.equal(files.length, 3)
        for (const artifact of files) {
            const buffers = []
            for await (const chunk of builder.openArtifact(artifact.id)) buffers.push(chunk)
            const data = Buffer.concat(buffers)
            assert.equal(artifact.size, data.length)
            assert.equal(artifact.sha256, createHash('sha256').update(data).digest('hex'))
            assert.equal('source' in artifact, false)
        }
        assert.throws(() => builder.openArtifact('../source.zip'), /Artifact not found/)
        const { manifest } = await builder.result(id)
        assert.equal(manifest.provenance.runtimeVersion, '2.3.4')
        assert.equal(manifest.provenance.runtimeRevision, null)
        assert.equal(manifest.status, 'complete')
        assert.deepEqual(manifest.segments.map(item => item.offset), [0x1000, 0x10000])
        assert.equal(manifest.ota, null)
    } finally { await builder.close(); await fs.rm(directory, { recursive: true, force: true }) }
})

test('build metadata includes streamed RAM and flash usage when reported', async () => {
    const directory = await fs.mkdtemp(path.join(os.tmpdir(), 'firmware-memory-'))
    const fake = new FakeExecutor()
    let chunks = [
        ['stdout', '\u001b[32mRAM:   [==        ]  24.'],
        ['stdout', '1% (used 78844 bytes from 327680 bytes)\u001b[0m\r'],
        ['stderr', 'Flash: [=] 99.0% (used 99 bytes from 100 bytes)\n'],
        ['stdout', 'Flash: [====      ]  41.6% (used 791394 bytes '],
        ['stdout', 'from 1900544 bytes)'],
    ]
    const executor = { async run(options) {
        const result = await fake.run(options)
        if (options.args[0] === 'run' && options.args.includes('buildprog')) {
            for (const [channel, text] of chunks) await options.onOutput(channel, Buffer.from(text))
            return { ...result, truncated: true }
        }
        return result
    } }
    const builder = await createFirmwareBuilder({ directory, executor })
    try {
        let id = await builder.submit({ source: source() })
        let events = await finished(builder, id)
        const usage = { ram: { usedBytes: 78844, totalBytes: 327680, percentage: 24.1 }, flash: { usedBytes: 791394, totalBytes: 1900544, percentage: 41.6 } }
        assert.deepEqual((await builder.result(id)).provenance.memoryUsage, usage)
        assert.deepEqual((await builder.status(id)).result.provenance.memoryUsage, usage)
        assert.deepEqual(events.filter(event => event.type === 'metadata').at(-1).payload.memoryUsage, usage)
        const output = events.filter(event => event.type === 'output' && event.payload.channel === 'stdout').map(event => Buffer.from(event.payload.data, 'base64').toString()).join('')
        for (const [channel, text] of chunks) if (channel === 'stdout') assert.ok(output.includes(text))
        chunks = [['stdout', 'RAM: [=] 1.0% (used 1 bytes from 0 bytes)\n'], ['stderr', 'Flash: [=] 1.0% (used 1 bytes from 100 bytes)\n']]
        id = await builder.submit({ source: source() })
        events = await finished(builder, id)
        assert.equal(events.at(-1).payload.state, 'succeeded')
        assert.deepEqual((await builder.result(id)).provenance.memoryUsage, { ram: null, flash: null })
    } finally { await builder.close(); await fs.rm(directory, { recursive: true, force: true }) }
})

test('failures, environment ambiguity, and active cancellation have one terminal outcome', async () => {
    const directory = await fs.mkdtemp(path.join(os.tmpdir(), 'firmware-builder-'))
    let builder
    try {
        builder = await createFirmwareBuilder({ directory, executor: new FakeExecutor({ environments: ['one', 'two'] }) })
        let id = await builder.submit({ source: source() })
        let events = await finished(builder, id)
        assert.deepEqual((await builder.status(id)).error.availableEnvironments, ['one', 'two'])
        assert.equal(events.filter(event => ['failed', 'cancelled', 'succeeded', 'interrupted'].includes(event.payload.state)).length, 1)
        await builder.close()
        builder = await createFirmwareBuilder({ directory, executor: new FakeExecutor({ wait: true }) })
        id = await builder.submit({ source: source() })
        for await (const event of builder.events(id)) if (event.payload.state === 'building') { await builder.cancel(id); break }
        events = await finished(builder, id)
        assert.equal((await builder.status(id)).state, 'cancelled')
        assert.equal(events.filter(event => ['failed', 'cancelled', 'succeeded', 'interrupted'].includes(event.payload.state)).length, 1)
    } finally { await builder?.close(); await fs.rm(directory, { recursive: true, force: true }) }
})

test('restart marks active jobs interrupted and replays queued jobs', async () => {
    const directory = await fs.mkdtemp(path.join(os.tmpdir(), 'firmware-builder-'))
    const store = new FileJobStore(path.join(directory, 'jobs'))
    await store.initialize()
    const active = randomUUID()
    const queued = randomUUID()
    for (const id of [active, queued]) {
        await store.create({ id, state: 'queued', adapter: 'platformio', environment: null, projectRoot: null, sourceSha256: 'test', sequence: 0 })
        await fs.writeFile(path.join(store.dir(id), 'source.zip'), zip([['platformio.ini', '[env:demo]']]))
        await store.append(id, 'state', { state: id === active ? 'building' : 'queued' })
    }
    await fs.appendFile(path.join(store.dir(active), 'events.ndjson'), '{"incomplete":')
    const builder = await createFirmwareBuilder({ directory, executor: new FakeExecutor() })
    try {
        assert.equal((await builder.status(active)).state, 'interrupted')
        await finished(builder, queued)
        assert.equal((await builder.status(queued)).state, 'succeeded')
    } finally { await builder.close(); await fs.rm(directory, { recursive: true, force: true }) }
})

test('build failures and artifact path escapes are retained as failures', async () => {
    const directory = await fs.mkdtemp(path.join(os.tmpdir(), 'firmware-builder-'))
    let builder
    try {
        builder = await createFirmwareBuilder({ directory, executor: new FakeExecutor({ fail: true }) })
        let id = await builder.submit({ source: source() })
        await finished(builder, id)
        assert.match((await builder.status(id)).error.message, /build failed/i)
        const replay = []
        for await (const event of builder.events(id)) replay.push(event)
        assert.ok(replay.some(event => event.type === 'output'))
        await builder.close()
        const outside = path.join(directory, 'outside.bin')
        await fs.writeFile(outside, 'secret')
        const unsafe = { prepare: async () => ({ environment: 'demo' }), build: async () => {}, collect: async ({ root }) => ({ outputRoot: root, files: [{ path: outside }] }) }
        builder = await createFirmwareBuilder({ directory, adapters: { unsafe } })
        id = await builder.submit({ source: source(), adapter: 'unsafe' })
        await finished(builder, id)
        assert.match((await builder.status(id)).error.message, /outside approved/i)
        assert.deepEqual(await builder.listArtifacts(id), [])
    } finally { await builder?.close(); await fs.rm(directory, { recursive: true, force: true }) }
})

test('upload limits and cleanup preserve active work', async () => {
    const directory = await fs.mkdtemp(path.join(os.tmpdir(), 'firmware-builder-'))
    const builder = await createFirmwareBuilder({ directory, executor: new FakeExecutor({ wait: true }), limits: { uploadBytes: 1024 } })
    try {
        await assert.rejects(builder.submit({ source: Readable.from([Buffer.alloc(1025)]) }), /upload size limit/i)
        const id = await builder.submit({ source: source() })
        for await (const event of builder.events(id)) if (event.payload.state === 'building') break
        assert.deepEqual(await builder.cleanup({ olderThanMs: 0 }), [])
        await builder.cancel(id)
        await finished(builder, id)
        while (builder.active.has(id)) await new Promise(resolve => setImmediate(resolve))
        assert.deepEqual(await builder.cleanup({ olderThanMs: 0 }), [id])
        await assert.rejects(builder.status(id), /not found/i)
    } finally { await builder.close(); await fs.rm(directory, { recursive: true, force: true }) }
})

test('script-bearing projects fail before compilation, including resolved configuration', async () => {
    const directory = await fs.mkdtemp(path.join(os.tmpdir(), 'firmware-policy-'))
    let runs = 0
    let buildRuns = 0
    let resolvedOptions = []
    const fake = new FakeExecutor()
    const executor = { async run(options) {
        runs++
        if (options.args[0] === 'run') buildRuns++
        if (options.args[0] === 'project') return { code: 0, stdout: JSON.stringify([['env:demo', resolvedOptions]]) }
        return fake.run(options)
    } }
    const builder = await createFirmwareBuilder({ directory, executor })
    try {
        const archive = zip([['platformio.ini', '[env:demo]'], ['extra.py', 'print(1)']])
        let id = await builder.submit({ source: Readable.from([archive]) })
        await finished(builder, id)
        assert.match((await builder.status(id)).error.message, /scripts and executables/i)
        assert.equal(runs, 0)
        for (const option of [['extra_scripts', ['pre:script.py']], ['build_flags', ['!echo command']]]) {
            resolvedOptions = [option]
            id = await builder.submit({ source: source() })
            await finished(builder, id)
            assert.equal((await builder.status(id)).state, 'failed')
            assert.match((await builder.status(id)).error.message, /not allowed/i)
            assert.equal(buildRuns, 0)
        }
    } finally { await builder.close(); await fs.rm(directory, { recursive: true, force: true }) }
})

test('local executor terminates an entire process group on cancellation', { skip: process.platform === 'win32' }, async () => {
    const executor = new LocalExecutor()
    const controller = new AbortController()
    let childPid
    let stderr = ''
    let stdout = ''
    const running = executor.run({ command: process.execPath, args: ['-e', "const c=require('child_process').spawn(process.execPath,['-e','setInterval(()=>{},1000)'],{stdio:'ignore'}); console.log(c.pid); setInterval(()=>{},1000)"], env: { NODE_TEST_CONTEXT: '' }, signal: controller.signal, onOutput: async (channel, chunk) => { if (channel === 'stderr') stderr += chunk.toString(); else { stdout += chunk.toString(); childPid = Number(stdout.replace(/\x1b\[[0-9;]*m/g, '').trim()); controller.abort() } } })
    let failure
    await assert.rejects(running, error => { failure = error; return true })
    assert.ok(childPid > 0, `${JSON.stringify(stdout)} ${stderr} ${failure?.stack}`)
    const status = await fs.readFile(`/proc/${childPid}/status`, 'utf8').catch(() => '')
    assert.ok(!status || /^State:\s+Z/m.test(status))
})

test('manifest does not invent offsets or OTA eligibility', () => {
    const provenance = { environment: 'demo', board: 'esp32dev' }
    assert.equal(esp32Manifest(null, [], provenance).status, 'unavailable')
    assert.equal(esp32Manifest({ flash_files: { '0x1000': '../outside.bin' } }, [], provenance).status, 'unavailable')
})

test('upload preview parser rejects ambiguous and unsupported commands', () => {
    const command = '"/usr/bin/python3" "/opt/tools/esptool.py" --chip esp32c3 --port "not a port" write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 "/tmp/build with spaces/bootloader.bin" 0x30000 /tmp/firmware.bin'
    const parsed = parseEsptoolPreview(command)
    assert.deepEqual(parsed.flash_files, { '0x0': '/tmp/build with spaces/bootloader.bin', '0x30000': '/tmp/firmware.bin' })
    assert.deepEqual(parsed.flash_settings, { flash_mode: 'dio', flash_freq: '80m', flash_size: '4MB' })
    assert.equal(parseEsptoolPreview(`${command}\n${command}`), null)
    assert.equal(parseEsptoolPreview(`${command} --encrypt`), null)
    assert.equal(parseEsptoolPreview(`${command} 0x00 /tmp/other.bin`), null)
    assert.equal(parseEsptoolPreview(command.replace('--flash_size 4MB', '')), null)
    assert.equal(parseEsptoolPreview(`${command}; dangerous`), null)
    assert.equal(parseEsptoolPreview(command, 'vovkplc-dry-run'), null)
    assert.equal(parseEsptoolPreview(command.replace('--chip esp32c3', '--chip esp32c3 --no-stub')), null)
})

test('upload preview publishes framework images and rejects outside paths', async () => {
    const directory = await fs.mkdtemp(path.join(os.tmpdir(), 'firmware-preview-'))
    const core = path.join(directory, 'core')
    const extra = path.join(core, 'packages', 'framework', 'boot_app0.bin')
    await fs.mkdir(path.dirname(extra), { recursive: true })
    await fs.writeFile(extra, 'framework-image')
    const fake = new FakeExecutor()
    let image = extra
    let extraOffset = '0xe000'
    const executor = { async run(options) {
        if (options.args[0] === 'system') return { code: 0, stdout: JSON.stringify({ core_dir: { value: core } }) }
        if (options.args[0] === 'run' && options.args.includes('upload')) {
            assert.equal(options.env.SCONSFLAGS, '-n')
            assert.equal(options.args[options.args.indexOf('--upload-port') + 1], 'vovkplc-dry-run')
            const output = path.join(options.env.PLATFORMIO_BUILD_DIR, 'demo')
            return { code: 0, stdout: `python /opt/esptool.py --chip esp32c3 --port vovkplc-dry-run write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 "${path.join(output, 'bootloader', 'bootloader.bin')}" ${extraOffset} "${image}" 0x30000 "${path.join(output, 'firmware.bin')}"` }
        }
        return fake.run(options)
    } }
    const builder = await createFirmwareBuilder({ directory: path.join(directory, 'builder'), executor })
    try {
        let id = await builder.submit({ source: source() })
        await finished(builder, id)
        const { manifest } = await builder.result(id)
        assert.equal(manifest.status, 'complete')
        assert.equal(manifest.chip, 'esp32c3')
        assert.deepEqual(manifest.segments.map(segment => segment.offset), [0, 0xe000, 0x30000])
        const artifact = await builder.artifactInfo(manifest.segments[1].artifactId)
        assert.equal(artifact.sha256, createHash('sha256').update('framework-image').digest('hex'))
        const chunks = []
        for await (const chunk of builder.openArtifact(artifact.id)) chunks.push(chunk)
        assert.equal(Buffer.concat(chunks).toString(), 'framework-image')
        image = path.join(directory, 'outside.bin')
        await fs.writeFile(image, 'outside')
        id = await builder.submit({ source: source() })
        await finished(builder, id)
        assert.equal((await builder.result(id)).manifest.status, 'unavailable')
        image = extra
        await fs.writeFile(extra, Buffer.alloc(8192))
        for (const offset of ['0x2f000', '0x30100', '0x400000']) {
            extraOffset = offset
            id = await builder.submit({ source: source() })
            await finished(builder, id)
            assert.equal((await builder.result(id)).manifest.status, 'unavailable')
        }
    } finally { await builder.close(); await fs.rm(directory, { recursive: true, force: true }) }
})

test('HTTP adapter authorizes every operation and replays SSE event IDs', async () => {
    const directory = await fs.mkdtemp(path.join(os.tmpdir(), 'firmware-http-'))
    const builder = await createFirmwareBuilder({ directory, executor: new FakeExecutor() })
    const actions = []
    const server = http.createServer(createFirmwareHttpHandler(builder, { authorize: ({ request, action }) => { actions.push(action); return request.headers.authorization === 'Bearer allowed' } }))
    await new Promise(resolve => server.listen(0, '127.0.0.1', resolve))
    const base = `http://127.0.0.1:${server.address().port}`
    const headers = { authorization: 'Bearer allowed' }
    try {
        assert.equal((await fetch(`${base}/builds`, { method: 'POST', body: 'bad' })).status, 403)
        const submission = await fetch(`${base}/builds`, { method: 'POST', headers, body: zip([['platformio.ini', '[env:demo]']]) })
        assert.equal(submission.status, 202)
        const { jobId } = await submission.json()
        await finished(builder, jobId)
        const replay = await fetch(`${base}/builds/${jobId}/events`, { headers: { ...headers, 'last-event-id': '2' } })
        const text = await replay.text()
        assert.match(text, /^id: 3\n/)
        assert.equal((await fetch(`${base}/builds/${jobId}`, { headers })).status, 200)
        const artifacts = await (await fetch(`${base}/builds/${jobId}/artifacts`, { headers })).json()
        assert.equal((await fetch(base + artifacts[0].downloadUrl)).status, 403)
        assert.equal((await fetch(base + artifacts[0].downloadUrl, { headers })).status, 200)
        assert.equal((await fetch(`${base}/builds/${jobId}/cancel`, { method: 'POST', headers })).status, 200)
        assert.deepEqual(new Set(actions), new Set(['submit', 'events', 'status', 'listArtifacts', 'openArtifact', 'cancel']))
    } finally { server.closeAllConnections(); await new Promise(resolve => server.close(resolve)); await builder.close(); await fs.rm(directory, { recursive: true, force: true }) }
})

test('example consumer uploads the project, downloads artifacts and prepares flashing', async () => {
    const directory = await fs.mkdtemp(path.join(os.tmpdir(), 'firmware-consumer-test-'))
    const fake = new FakeExecutor()
    let hasManifest = true
    const executor = { async run(options) {
        if (options.args[0] === 'project') {
            const root = options.args[options.args.indexOf('--project-dir') + 1]
            const ini = await fs.readFile(path.join(root, 'platformio.ini'), 'utf8')
            assert.match(ini, /^lib_deps = VovkPLCRuntime$/m)
            assert.doesNotMatch(ini, /symlink:/)
            assert.ok((await fs.readFile(path.join(root, 'src', 'main.cpp'), 'utf8')).includes('VovkPLCRuntime'))
            assert.ok((await fs.readFile(path.join(root, 'partitions.csv'), 'utf8')).length)
            return { code: 0, stdout: JSON.stringify([['env:esp32-c3-supermini', [['board', 'lolin_c3_mini']]]]) }
        }
        if (options.args[0] === 'run' && options.args.includes('upload')) {
            if (!hasManifest) return { code: 1, stdout: '' }
            const preview = await fake.run(options)
            return { ...preview, stdout: preview.stdout.replace('--chip esp32 ', '--chip esp32c3 ') }
        }
        const result = await fake.run(options)
        return result
    } }
    const builder = await createFirmwareBuilder({ directory: path.join(directory, 'server'), executor })
    const server = http.createServer(createFirmwareHttpHandler(builder, { authorize: ({ request }) => request.headers.authorization === 'Bearer demo-token' }))
    await new Promise(resolve => server.listen(0, '127.0.0.1', resolve))
    const output = path.join(directory, 'downloads')
    const script = path.resolve('examples/esp32-c3-supermini-wifi/firmware-consumer.js')
    const args = [script, '--url', `http://127.0.0.1:${server.address().port}`, '--output', output, '--port', '/dev/example', '--esptool', '/bin/echo']
    try {
        const execute = promisify(execFile)
        const result = await execute(process.execPath, args, { env: { ...process.env, FIRMWARE_API_TOKEN: 'demo-token' } })
        assert.match(result.stdout, /State: succeeded/)
        assert.match(result.stdout, /--chip esp32c3 --port \/dev\/example write_flash --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000/)
        assert.equal(await fs.readFile(path.join(output, 'firmware.bin'), 'utf8'), 'firmware')
        await assert.rejects(execute(process.execPath, args, { env: { ...process.env, FIRMWARE_API_TOKEN: 'demo-token' } }), /EEXIST/)
        assert.equal(await fs.readFile(path.join(output, 'firmware.bin'), 'utf8'), 'firmware')
        hasManifest = false
        const missing = [...args]
        missing[missing.indexOf('--output') + 1] = path.join(directory, 'missing-manifest')
        await assert.rejects(execute(process.execPath, missing, { env: { ...process.env, FIRMWARE_API_TOKEN: 'demo-token' } }), /Flash manifest unavailable/)
        const explicit = [...args, '--segment', '0x30000=firmware.bin']
        explicit[explicit.indexOf('--output') + 1] = path.join(directory, 'explicit-segments')
        const manual = await execute(process.execPath, explicit, { env: { ...process.env, FIRMWARE_API_TOKEN: 'demo-token' } })
        assert.match(manual.stdout, /write_flash 0x30000 .*firmware\.bin/)
    } finally { server.closeAllConnections(); await new Promise(resolve => server.close(resolve)); await builder.close(); await fs.rm(directory, { recursive: true, force: true }) }
})

const realBuildTimeoutMs = process.env.FIRMWARE_TEST_TIMEOUT_MS === undefined ? 14 * 60 * 1000 : Number(process.env.FIRMWARE_TEST_TIMEOUT_MS)

test('real PlatformIO ESP32 build', { skip: process.env.FIRMWARE_REAL_BUILD !== '1', timeout: realBuildTimeoutMs + 60 * 1000 }, async () => {
    const directory = await fs.mkdtemp(path.join(os.tmpdir(), 'firmware-real-'))
    const executor = process.env.FIRMWARE_TEST_CORE_DIR ? new class extends LocalExecutor {
        run(options) { return super.run({ ...options, env: { ...options.env, PLATFORMIO_CORE_DIR: process.env.FIRMWARE_TEST_CORE_DIR } }) }
    }() : undefined
    const builder = await createFirmwareBuilder({ directory, executor, limits: { executionMs: realBuildTimeoutMs } })
    let phase = 'starting'
    const started = Date.now()
    const heartbeat = setInterval(() => console.log(`integration: ${phase}, elapsed ${Math.floor((Date.now() - started) / 1000)}s`), 30000)
    heartbeat.unref()
    try {
        const archive = zip([
            ['platformio.ini', '[env:esp32]\nplatform = espressif32\nboard = lolin_c3_mini\nframework = arduino\n'],
            ['src/main.cpp', '#include <Arduino.h>\nvoid setup() {}\nvoid loop() {}\n'],
        ])
        const id = await builder.submit({ source: Readable.from([archive]) })
        const stdout = []
        for await (const event of builder.events(id)) {
            if (event.type === 'state') { phase = event.payload.state; console.log('integration state:', phase) }
            if (event.type === 'output') {
                const chunk = Buffer.from(event.payload.data, 'base64')
                if (event.payload.channel === 'stdout') stdout.push(chunk)
                ;(event.payload.channel === 'stderr' ? process.stderr : process.stdout).write(chunk)
            }
        }
        const status = await builder.status(id)
        assert.equal(status.state, 'succeeded', status.error?.message)
        const output = Buffer.concat(stdout).toString('utf8').replace(/\x1b\[[0-?]*[ -/]*[@-~]/g, '')
        for (const [label, key] of [['RAM', 'ram'], ['Flash', 'flash']]) {
            const reported = output.match(new RegExp(`^\\s*${label}:\\s*\\[[^\\]]*\\]\\s*(\\d+(?:\\.\\d+)?)%\\s*\\(used\\s+(\\d+)\\s+bytes\\s+from\\s+(\\d+)\\s+bytes\\)`, 'm'))
            assert.ok(reported, `${label} usage missing from PlatformIO output`)
            assert.deepEqual(status.result.provenance.memoryUsage[key], { usedBytes: Number(reported[2]), totalBytes: Number(reported[3]), percentage: Number(reported[1]) })
        }
        const artifacts = await builder.listArtifacts(id)
        assert.ok(artifacts.some(item => item.filename.endsWith('.bin')))
        for (const artifact of artifacts) {
            const hash = createHash('sha256')
            for await (const chunk of builder.openArtifact(artifact.id)) hash.update(chunk)
            assert.equal(hash.digest('hex'), artifact.sha256)
        }
        const manifest = (await builder.result(id)).manifest
        assert.equal(manifest.status, 'complete', manifest.reason)
        assert.equal(manifest.chip, 'esp32c3')
        assert.ok(manifest.segments.length >= 3)
        for (const segment of manifest.segments) assert.ok(artifacts.some(item => item.id === segment.artifactId && item.sha256 === segment.sha256))
        console.log('integration manifest:', manifest.status, manifest.reason || '', 'segments:', manifest.segments.length)
    } finally { clearInterval(heartbeat); await builder.close(); if (process.env.FIRMWARE_KEEP_OUTPUT === '1') console.log('integration directory:', directory); else await fs.rm(directory, { recursive: true, force: true }) }
})