import fs from 'node:fs/promises'
import { constants } from 'node:fs'
import path from 'node:path'
import { StringDecoder } from 'node:string_decoder'

export function parseEsptoolPreview(output, expectedPort) {
    const candidates = output.split(/\r?\n/).filter(line => /(?:^|\s)write[-_]flash(?:\s|$)/.test(line) && /(?:^|[\\/"'\s])esptool(?:\.py|\.exe)?(?:["'\s]|$)/i.test(line))
    if (candidates.length !== 1) return null
    const tokens = []
    let token = ''
    let quote = null
    let started = false
    const line = candidates[0]
    for (let index = 0; index < line.length; index++) {
        const char = line[index]
        if (char === '\\' && quote !== "'") {
            if (++index >= line.length) return null
            token += line[index]
            started = true
        } else if (quote) {
            if (char === quote) quote = null
            else if (quote === '"' && /[$`]/.test(char)) return null
            else token += char
        } else if (char === '"' || char === "'") { quote = char; started = true }
        else if (/\s/.test(char)) { if (started) tokens.push(token); token = ''; started = false }
        else if (/[;&|<>$`]/.test(char)) return null
        else { token += char; started = true }
    }
    if (quote) return null
    if (started) tokens.push(token)
    const command = tokens.findIndex(item => item === 'write_flash' || item === 'write-flash')
    if (command < 0) return null
    const executable = tokens.findIndex(item => /(?:^|[\\/])esptool(?:\.py|\.exe)?$/i.test(item))
    if (executable < 0 || executable > 1 || executable >= command) return null
    if (executable === 1 && !/(?:^|[\\/])python(?:\d+(?:\.\d+)?)?(?:\.exe)?$/i.test(tokens[0])) return null
    const global = tokens.slice(executable + 1, command)
    for (let index = 0; index < global.length; index++) {
        if (!['--chip', '--port', '--baud', '--before', '--after'].includes(global[index]) || !global[index + 1] || global[index + 1].startsWith('-')) return null
        index++
    }
    const option = (items, name) => {
        const indices = items.flatMap((item, index) => item === name ? [index] : [])
        return indices.length === 1 && indices[0] + 1 < items.length ? items[indices[0] + 1] : null
    }
    const chip = option(global, '--chip')
    if (expectedPort !== undefined && option(global, '--port') !== expectedPort) return null
    const args = tokens.slice(command + 1)
    const settings = { flash_mode: null, flash_freq: null, flash_size: null }
    const flashFiles = {}
    for (let index = 0; index < args.length; index++) {
        const arg = args[index]
        if (arg === '-z' || arg === '--compress' || arg === '--no-compress') continue
        const setting = /^--flash[_-](mode|freq|size)$/.exec(arg)
        if (setting) {
            const key = `flash_${setting[1]}`
            if (settings[key] || !args[index + 1] || args[index + 1].startsWith('-')) return null
            settings[key] = args[++index]
        } else if (/^0x[\da-f]+$/i.test(arg) && args[index + 1] && !args[index + 1].startsWith('-')) {
            const offset = Number.parseInt(arg, 16)
            if (!Number.isSafeInteger(offset) || Object.hasOwn(flashFiles, `0x${offset.toString(16)}`)) return null
            flashFiles[`0x${offset.toString(16)}`] = args[++index]
        } else return null
    }
    if (!/^esp32[a-z0-9]*$/i.test(chip || '') || !/^(?:qio|qout|dio|dout|keep)$/i.test(settings.flash_mode || '') || !/^(?:keep|\d+m)$/i.test(settings.flash_freq || '') || !/^(?:detect|keep|\d+(?:KB|MB)(?:-c1)?)$/i.test(settings.flash_size || '') || !Object.keys(flashFiles).length) return null
    return { flash_files: flashFiles, flash_settings: settings, extra_esptool_args: { chip } }
}

async function smallText(file) {
    try { const stat = await fs.lstat(file); return stat.isFile() && stat.size <= 1024 * 1024 ? await fs.readFile(file, 'utf8') : null }
    catch (error) { if (error.code === 'ENOENT' || error.code === 'ENOTDIR') return null; throw error }
}

async function bundledRuntime(root) {
    const directory = path.join(root, 'lib')
    let entries
    try { entries = await fs.readdir(directory, { withFileTypes: true }) } catch (error) { if (error.code === 'ENOENT') return null; throw error }
    for (const entry of entries) {
        if (!entry.isDirectory()) continue
        const library = path.join(directory, entry.name)
        const properties = await smallText(path.join(library, 'library.properties'))
        if (!properties) continue
        const values = Object.fromEntries(properties.split(/\r?\n/).map(line => line.match(/^\s*([^#=]+?)\s*=\s*(.*?)\s*$/)).filter(Boolean).map(match => [match[1], match[2]]))
        if (values.name?.toLowerCase() !== 'vovkplcruntime') continue
        const gitDirectory = path.join(library, '.git')
        if (!(await fs.lstat(gitDirectory).catch(() => null))?.isDirectory()) return { version: values.version || null, revision: null }
        let revision = (await smallText(path.join(gitDirectory, 'HEAD')))?.trim() || null
        if (revision?.startsWith('ref: ')) {
            const ref = revision.slice(5)
            revision = /^refs\/[\w./-]+$/.test(ref) && !ref.split('/').includes('..') ? (await smallText(path.join(gitDirectory, ref)))?.trim() || null : null
        }
        if (!/^[\da-f]{40,64}$/i.test(revision || '')) revision = null
        return { version: values.version || null, revision }
    }
    return null
}

export function esp32Manifest(metadata, artifacts, provenance) {
    const result = { version: 1, status: 'unavailable', reason: 'PlatformIO flash upload metadata is unavailable or unsupported', environment: provenance.environment, board: provenance.board, chip: null, flash: { mode: null, frequency: null, size: null }, segments: [], ota: null, provenance, artifacts: artifacts.map(({ id, sha256 }) => ({ id, sha256 })) }
    if (!metadata || !metadata.flash_files || typeof metadata.flash_files !== 'object') return result
    result.chip = metadata.extra_esptool_args?.chip || null
    result.flash = { mode: metadata.flash_settings?.flash_mode || null, frequency: metadata.flash_settings?.flash_freq || null, size: metadata.flash_settings?.flash_size || null }
    const segments = []
    for (const [offset, filename] of Object.entries(metadata.flash_files)) {
        if (!/^0x[\da-f]+$/i.test(offset) || !Number.isSafeInteger(Number.parseInt(offset, 16)) || typeof filename !== 'string') { result.reason = 'Invalid flash metadata'; return result }
        const normalized = path.posix.normalize(filename.replaceAll('\\', '/')).replace(/^\.\//, '')
        const artifact = artifacts.find(item => item.source === normalized)
        if (!artifact) { result.reason = `Flash output not available: ${filename}`; return result }
        segments.push({ offset: Number.parseInt(offset, 16), artifactId: artifact.id, sha256: artifact.sha256 })
    }
    if (!segments.length) { result.reason = 'Flash metadata has no segments'; return result }
    result.segments = segments.sort((left, right) => left.offset - right.offset)
    result.status = 'complete'
    result.reason = null
    return result
}

export class PlatformIOAdapter {
    constructor({ executable = 'pio' } = {}) { this.executable = executable }
    async check({ executor, signal, timeoutMs = 15000 } = {}) {
        try {
            const result = await executor.run({ command: this.executable, args: ['--version'], signal, timeoutMs })
            return result.code === 0 ? { available: true, version: result.stdout.trim() } : { available: false, version: null, error: `PlatformIO exited with code ${result.code}` }
        } catch (error) { return { available: false, version: null, error: error.message } }
    }
    async prepare(context) {
        const { executor, root, env, signal, timeoutMs, onOutput } = context
        const check = await this.check({ executor, signal, timeoutMs })
        if (!check.available) throw new Error(`PlatformIO unavailable: ${check.error}`)
        const config = await executor.run({ command: this.executable, args: ['project', 'config', '--json-output', '--project-dir', root], cwd: root, env, signal, timeoutMs, onOutput })
        if (config.code !== 0 || config.truncated) throw new Error('PlatformIO project configuration failed or exceeded the metadata limit')
        let sections
        try { sections = JSON.parse(config.stdout) } catch { throw new Error('PlatformIO returned invalid project configuration JSON') }
        for (const [, entries] of sections) {
            for (const [key, value] of entries) {
                if (/^(?:extra_scripts|extra_configs)$/i.test(key)) throw new Error(`PlatformIO script hooks and extra configuration files are not allowed: ${key}`)
                const values = Array.isArray(value) ? value : [value]
                if (values.some(item => typeof item === 'string' && /^\s*!/.test(item))) throw new Error(`Command-generated PlatformIO configuration is not allowed: ${key}`)
            }
        }
        const environments = sections.filter(([section]) => section.startsWith('env:')).map(([section]) => section.slice(4))
        const selected = context.environment || (environments.length === 1 ? environments[0] : null)
        if (!selected || !environments.includes(selected)) {
            const error = new Error(`Select one PlatformIO environment. Available: ${environments.join(', ') || '(none)'}`)
            error.availableEnvironments = environments
            throw error
        }
        if (selected === '.' || selected === '..' || /[\\/:\0]/.test(selected)) throw new Error('PlatformIO environment name cannot be used as an output directory')
        const options = Object.fromEntries(sections.find(([section]) => section === `env:${selected}`)[1])
        return { environment: selected, environments, board: options.board || null, framework: options.framework || null, platform: options.platform || null, adapterVersion: '1', toolVersion: check.version, packages: null, runtimeVersion: null, runtimeRevision: null, memoryUsage: { ram: null, flash: null } }
    }
    async build(context, metadata) {
        const decoder = new StringDecoder('utf8')
        let pending = ''
        const record = line => {
            const clean = line.replace(/\x1b\[[0-?]*[ -/]*[@-~]/g, '')
            const match = clean.match(/^\s*(RAM|Flash)\s*:\s*\[[^\]]*\]\s*(\d+(?:\.\d+)?)%\s*\(used\s+(\d+)\s+bytes\s+from\s+(\d+)\s+bytes\)\s*$/i)
            if (!match) return
            const percentage = Number(match[2])
            const usedBytes = Number(match[3])
            const totalBytes = Number(match[4])
            if (Number.isFinite(percentage) && Number.isSafeInteger(usedBytes) && Number.isSafeInteger(totalBytes) && totalBytes > 0) metadata.memoryUsage[match[1].toLowerCase()] = { usedBytes, totalBytes, percentage }
        }
        const onOutput = async (channel, chunk) => {
            if (channel === 'stdout') {
                const lines = (pending + decoder.write(chunk)).split(/[\r\n]/)
                pending = lines.pop().slice(-8192)
                for (const line of lines) record(line)
            }
            await context.onOutput(channel, chunk)
        }
        const result = await context.executor.run({ command: this.executable, args: ['run', '--project-dir', context.root, '--environment', metadata.environment, '--target', 'buildprog'], cwd: context.root, env: context.env, signal: context.signal, timeoutMs: context.timeoutMs, onOutput })
        record(pending + decoder.end())
        if (result.code !== 0) throw new Error(`PlatformIO build failed (exit ${result.code}, signal ${result.signal || 'none'})`)
        try {
            const preview = await context.executor.run({ command: this.executable, args: ['run', '--project-dir', context.root, '--environment', metadata.environment, '--target', 'upload', '--upload-port', 'vovkplc-dry-run', '--verbose'], cwd: context.root, env: { ...context.env, SCONSFLAGS: '-n' }, signal: context.signal, timeoutMs: context.timeoutMs })
            if (preview.code === 0 && !preview.truncated) context.flashPreview = parseEsptoolPreview(preview.stdout, 'vovkplc-dry-run')
            if (context.flashPreview) {
                const info = await context.executor.run({ command: this.executable, args: ['system', 'info', '--json-output'], cwd: context.root, env: context.env, signal: context.signal, timeoutMs: context.timeoutMs })
                if (info.code === 0 && !info.truncated) context.platformioCoreDir = JSON.parse(info.stdout).core_dir?.value
            }
        } catch (error) { context.signal.throwIfAborted(); context.flashPreview = null }
        try {
            const listing = await context.executor.run({ command: this.executable, args: ['pkg', 'list', '--project-dir', context.root, '--environment', metadata.environment], cwd: context.root, env: context.env, signal: context.signal, timeoutMs: context.timeoutMs, onOutput: context.onOutput })
            if (listing.code === 0 && !listing.truncated) {
                const packages = []
                for (const line of listing.stdout.split(/\r?\n/)) {
                    const match = line.replace(/\x1b\[[0-?]*[ -/]*[@-~]/g, '').match(/(?:^|\s|─)([\w.-]+) @ ([^\s(]+)/)
                    if (match) packages.push({ name: match[1], version: match[2] })
                }
                metadata.packages = packages.length ? packages : null
                metadata.runtimeVersion = packages.find(item => item.name.toLowerCase() === 'vovkplcruntime')?.version || null
            }
        } catch (error) { context.signal.throwIfAborted() }
        const runtime = await bundledRuntime(context.root).catch(() => null)
        if (runtime) { metadata.runtimeVersion ||= runtime.version; metadata.runtimeRevision = runtime.revision }
    }
    async collect(context, metadata) {
        const root = path.join(context.buildDir, metadata.environment)
        const outputs = []
        async function walk(directory) {
            for (const entry of await fs.readdir(directory, { withFileTypes: true })) {
                const file = path.join(directory, entry.name)
                if (entry.isDirectory() && entry.name !== 'CMakeFiles') await walk(file)
                else if (entry.isFile() && /\.(bin|elf|hex|uf2|map)$/i.test(entry.name)) {
                    const role = /\.elf$/i.test(entry.name) ? 'elf' : /\.map$/i.test(entry.name) ? 'map' : /bootloader/i.test(entry.name) ? 'bootloader' : /partition/i.test(entry.name) ? 'partition-table' : 'firmware'
                    outputs.push({ path: file, source: path.relative(root, file).split(path.sep).join('/'), role })
                }
            }
        }
        await walk(root)
        if (!outputs.length) throw new Error('PlatformIO produced no recognized build artifacts')
        let flash = null
        if (context.flashPreview && typeof context.platformioCoreDir === 'string') {
            try {
                const buildRoot = await fs.realpath(root)
                const packagesRoot = await fs.realpath(path.join(context.platformioCoreDir, 'packages'))
                const inside = (parent, file) => { const relative = path.relative(parent, file); return relative && relative !== '..' && !relative.startsWith(`..${path.sep}`) && !path.isAbsolute(relative) }
                const verified = []
                for (const [offset, filename] of Object.entries(context.flashPreview.flash_files)) {
                    const file = await fs.realpath(path.resolve(context.root, filename))
                    const stat = await fs.stat(file)
                    const address = Number.parseInt(offset, 16)
                    if (!stat.isFile() || !stat.size || address % 4096 || (!inside(buildRoot, file) && !inside(packagesRoot, file))) throw new Error('Invalid flash image or path')
                    verified.push({ offset, file, address, end: address + Math.ceil(stat.size / 4096) * 4096 })
                }
                verified.sort((left, right) => left.address - right.address)
                const flashSize = /^(\d+)(KB|MB)$/i.exec(context.flashPreview.flash_settings.flash_size)
                const limit = flashSize ? Number(flashSize[1]) * (flashSize[2].toUpperCase() === 'MB' ? 1024 * 1024 : 1024) : null
                for (const [index, item] of verified.entries()) if (!Number.isSafeInteger(item.end) || (limit !== null && item.end > limit) || (index && verified[index - 1].end > item.address)) throw new Error('Flash images overlap or exceed flash size')
                const flashFiles = {}
                let directory
                for (const [index, item] of verified.entries()) {
                    let output = outputs.find(candidate => path.resolve(candidate.path) === item.file)
                    if (!output) {
                        directory ||= await fs.mkdtemp(path.join(root, '.flash-images-'))
                        const file = path.join(directory, `${index}-${path.basename(item.file)}`)
                        await fs.copyFile(item.file, file, constants.COPYFILE_EXCL)
                        output = { path: file, source: path.relative(root, file).split(path.sep).join('/'), role: 'flash-image' }
                        outputs.push(output)
                    }
                    flashFiles[item.offset] = output.source
                }
                flash = { ...context.flashPreview, flash_files: flashFiles }
            } catch (error) { context.signal.throwIfAborted(); flash = null }
        }
        return { outputRoot: root, files: outputs, flash }
    }
    manifest(collected, artifacts, provenance) { return esp32Manifest(collected.flash, artifacts, provenance) }
}