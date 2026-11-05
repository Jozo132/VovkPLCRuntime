import fs from 'node:fs'
import fsp from 'node:fs/promises'
import path from 'node:path'
import os from 'node:os'
import { fileURLToPath } from 'node:url'
import { spawn } from 'node:child_process'
import { createHash, randomUUID } from 'node:crypto'
import { createInterface } from 'node:readline'
import { Readable, Transform } from 'node:stream'
import { pipeline } from 'node:stream/promises'

const usage = `Usage: node examples/esp32-c3-supermini-wifi/firmware-consumer.js --url URL --output DIR [--port PORT] [--esptool PATH] [--segment OFFSET=FILE ...]
Set FIRMWARE_API_TOKEN to the server's bearer token. --port enables flashing.
When the server has no complete flash manifest, --segment must explicitly specify every flash offset and file.
FILE may be an artifact filename in DIR or an absolute path to a verified external image.`

function parseArgs(args) {
    const options = { segments: [] }
    for (let index = 0; index < args.length; index++) {
        const name = args[index]
        if (name === '--help') return { help: true }
        if (!['--url', '--output', '--port', '--esptool', '--segment'].includes(name) || !args[index + 1] || args[index + 1].startsWith('--')) throw new Error(`Invalid argument: ${name}\n${usage}`)
        const value = args[++index]
        if (name === '--segment') options.segments.push(value)
        else options[name.slice(2)] = value
    }
    if (!options.url || !options.output) throw new Error(usage)
    if (options.segments.length && !options.port) throw new Error('--segment requires --port')
    return options
}

function run(command, args, options = {}) {
    return new Promise((resolve, reject) => {
        const child = spawn(command, args, { stdio: 'inherit', ...options })
        child.once('error', reject)
        child.once('close', (code, signal) => code === 0 ? resolve() : reject(new Error(`${command} failed (exit ${code}, signal ${signal || 'none'})`)))
    })
}

async function request(url, options) {
    const response = await fetch(url, options)
    if (!response.ok) throw new Error(`HTTP ${response.status}: ${await response.text()}`)
    return response
}

async function main() {
    const options = parseArgs(process.argv.slice(2))
    if (options.help) { console.log(usage); return }
    const token = process.env.FIRMWARE_API_TOKEN
    if (!token) throw new Error('FIRMWARE_API_TOKEN is required')
    const base = new URL(options.url.endsWith('/') ? options.url : `${options.url}/`)
    if (!['http:', 'https:'].includes(base.protocol) || base.username || base.password || base.search || base.hash) throw new Error('Invalid service URL')
    const headers = { authorization: `Bearer ${token}` }
    const example = path.dirname(fileURLToPath(import.meta.url))
    const temporary = await fsp.mkdtemp(path.join(os.tmpdir(), 'firmware-consumer-'))
    try {
        const project = path.join(temporary, 'project')
        await fsp.mkdir(project)
        for (const name of ['src', 'partitions.csv', 'README.md']) await fsp.cp(path.join(example, name), path.join(project, name), { recursive: true })
        const ini = await fsp.readFile(path.join(example, 'platformio.ini'), 'utf8')
        const dependency = /^\s*lib_deps\s*=\s*symlink:\/\/\.\.\/\.\.\s*$/m
        if (!dependency.test(ini)) throw new Error('Expected runtime symlink dependency not found in platformio.ini')
        await fsp.writeFile(path.join(project, 'platformio.ini'), ini.replace(dependency, 'lib_deps = VovkPLCRuntime'))
        const archive = path.join(temporary, 'project.zip')
        await run('zip', ['-q', '-r', archive, '.'], { cwd: project })
        const submission = await request(new URL('builds?environment=esp32-c3-supermini', base), { method: 'POST', headers: { ...headers, 'content-type': 'application/zip' }, body: fs.createReadStream(archive), duplex: 'half' })
        const { jobId } = await submission.json()
        console.log(`Build: ${jobId}`)
        const events = await request(new URL(`builds/${encodeURIComponent(jobId)}/events`, base), { headers })
        const lines = createInterface({ input: Readable.fromWeb(events.body), crlfDelay: Infinity })
        for await (const line of lines) {
            if (!line.startsWith('data: ')) continue
            const event = JSON.parse(line.slice(6))
            if (event.type === 'state') console.log(`State: ${event.payload.state}`)
            if (event.type === 'output') (event.payload.channel === 'stderr' ? process.stderr : process.stdout).write(Buffer.from(event.payload.data, 'base64'))
        }
        const status = await (await request(new URL(`builds/${encodeURIComponent(jobId)}`, base), { headers })).json()
        if (status.state !== 'succeeded') throw new Error(`Build ${status.state}: ${status.error?.message || 'unknown error'}`)
        const artifacts = await (await request(new URL(`builds/${encodeURIComponent(jobId)}/artifacts`, base), { headers })).json()
        const output = path.resolve(options.output)
        await fsp.mkdir(output, { recursive: true })
        const downloaded = new Map()
        for (const artifact of artifacts) {
            if (!artifact.filename || path.basename(artifact.filename) !== artifact.filename || downloaded.has(artifact.filename)) throw new Error('Unsafe or duplicate artifact filename')
            const destination = path.join(output, artifact.filename)
            const response = await request(new URL(`artifacts/${encodeURIComponent(artifact.id)}`, base), { headers })
            const hash = createHash('sha256')
            let size = 0
            const checksum = new Transform({ transform(chunk, encoding, callback) { hash.update(chunk); size += chunk.length; callback(null, chunk) } })
            const partial = path.join(output, `.download-${randomUUID()}`)
            try {
                await pipeline(Readable.fromWeb(response.body), checksum, fs.createWriteStream(partial, { flags: 'wx' }))
                if (size !== artifact.size || hash.digest('hex') !== artifact.sha256) throw new Error(`Artifact integrity check failed: ${artifact.filename}`)
                await fsp.link(partial, destination)
            } finally { await fsp.rm(partial, { force: true }) }
            downloaded.set(artifact.filename, { ...artifact, destination })
            console.log(`Downloaded ${destination} (${artifact.sha256})`)
        }
        const manifest = status.result?.manifest
        if (!options.port) { console.log(`Flash manifest: ${manifest?.status || 'unavailable'}${manifest?.reason ? ` (${manifest.reason})` : ''}`); return }
        let segments
        let flashSettings = []
        if (options.segments.length) {
            segments = options.segments.map(value => {
                const match = /^(0x[\da-f]+)=(.+)$/i.exec(value)
                if (!match) throw new Error(`Invalid flash segment: ${value}`)
                const file = path.isAbsolute(match[2]) ? match[2] : downloaded.get(match[2])?.destination
                if (!file) throw new Error(`Segment must name a downloaded artifact or an absolute file: ${match[2]}`)
                return { offset: match[1], file }
            })
        } else {
            if (manifest?.status !== 'complete' || !manifest.segments?.length) throw new Error(`Flash manifest unavailable: ${manifest?.reason || 'no segments'}. Supply verified --segment arguments.`)
            if (manifest.chip !== 'esp32c3') throw new Error(`Unexpected manifest chip: ${manifest.chip}`)
            if (!manifest.flash?.mode || !manifest.flash?.frequency || !manifest.flash?.size) throw new Error('Flash manifest is missing flash settings')
            flashSettings = ['--flash_mode', manifest.flash.mode, '--flash_freq', manifest.flash.frequency, '--flash_size', manifest.flash.size]
            segments = manifest.segments.map(segment => {
                const artifact = [...downloaded.values()].find(item => item.id === segment.artifactId && item.sha256 === segment.sha256)
                if (!artifact || !Number.isSafeInteger(segment.offset) || segment.offset < 0) throw new Error('Invalid flash manifest segment')
                return { offset: `0x${segment.offset.toString(16)}`, file: artifact.destination }
            })
        }
        for (const segment of segments) await fsp.access(segment.file, fs.constants.R_OK)
        console.log(`Flashing ${options.port}: ${segments.map(segment => `${segment.offset}=${segment.file}`).join(', ')}`)
        await run(options.esptool || 'esptool', ['--chip', 'esp32c3', '--port', options.port, 'write_flash', ...flashSettings, ...segments.flatMap(segment => [segment.offset, segment.file])])
    } finally { await fsp.rm(temporary, { recursive: true, force: true }) }
}

main().catch(error => { console.error(error.message); process.exitCode = 1 })