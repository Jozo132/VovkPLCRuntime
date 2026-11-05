import fs from 'node:fs'
import fsp from 'node:fs/promises'
import path from 'node:path'
import { createInterface } from 'node:readline'
import { Transform } from 'node:stream'
import { pipeline } from 'node:stream/promises'
import yauzl from 'yauzl'

const openZip = file => new Promise((resolve, reject) => yauzl.open(file, { lazyEntries: true, autoClose: false, strictFileNames: true }, (error, zip) => error ? reject(error) : resolve(zip)))
const openEntry = (zip, entry) => new Promise((resolve, reject) => zip.openReadStream(entry, (error, stream) => error ? reject(error) : resolve(stream)))
const allowedExtensions = new Set(['.c', '.cc', '.cpp', '.cxx', '.h', '.hh', '.hpp', '.hxx', '.ino', '.s', '.asm', '.inc', '.ld', '.csv', '.md', '.txt', '.ini', '.json', '.yaml', '.yml', '.xml', '.properties'])
const allowedNames = new Set(['license', 'readme', 'copying', 'authors', '.gitignore', '.gitattributes'])

function validateFileName(name) {
    const base = path.posix.basename(name).toLowerCase()
    if (base === 'cmakelists.txt' || (!allowedNames.has(base) && !allowedExtensions.has(path.posix.extname(base)))) throw new Error(`Unsupported project file (scripts and executables are not allowed): ${name}`)
}

async function validateIni(file, name) {
    const stream = fs.createReadStream(file, { encoding: 'utf8' })
    const lines = createInterface({ input: stream, crlfDelay: Infinity })
    try {
        for await (const line of lines) {
            if (/^\s*(?:extra_scripts|extra_configs)\s*[:=]/i.test(line)) throw new Error(`PlatformIO script hooks and extra configuration files are not allowed: ${name}`)
            if (/^\s*(?:[\w.-]+\s*[:=]\s*)?!/.test(line)) throw new Error(`Command-generated PlatformIO configuration is not allowed: ${name}`)
        }
    } finally { lines.close(); stream.destroy() }
}

async function validateManifest(file, name) {
    let data
    try { data = JSON.parse(await fsp.readFile(file, 'utf8')) } catch { throw new Error(`Invalid project manifest: ${name}`) }
    const visit = value => {
        if (!value || typeof value !== 'object') return
        for (const [key, nested] of Object.entries(value)) {
            if (/^(?:extrascript|extra_script|scripts)$/i.test(key)) throw new Error(`Executable script hooks are not allowed in project manifest: ${name}`)
            visit(nested)
        }
    }
    visit(data)
}

export async function extractProject(archive, destination, { maxFiles = 10000, maxExtractBytes = 512 * 1024 * 1024, projectRoot, signal } = {}) {
    const zip = await openZip(archive)
    const roots = []
    const seen = new Set()
    let files = 0
    let bytes = 0
    await fsp.mkdir(destination, { recursive: true })
    try {
        await new Promise((resolve, reject) => {
            const fail = error => { zip.removeAllListeners('entry'); reject(error) }
            zip.on('error', fail)
            zip.on('end', resolve)
            zip.on('entry', async entry => {
                try {
                    signal?.throwIfAborted()
                    const name = entry.fileName
                    const parts = name.split('/')
                    if (!name || name.includes('\\') || name.startsWith('/') || name.includes(':') || parts.some(part => part === '..' || part === '.') || name.includes('\0')) throw new Error(`Unsafe ZIP path: ${name}`)
                    if (parts.some(part => ['.git', '.pio', 'node_modules', '.venv'].includes(part.toLowerCase()))) throw new Error(`Unsupported project directory: ${name}`)
                    const mode = entry.externalFileAttributes >>> 16
                    const type = mode & 0o170000
                    if (type && type !== 0o100000 && type !== 0o040000) throw new Error(`Unsafe ZIP entry type: ${name}`)
                    if (++files > maxFiles) throw new Error('ZIP file count limit exceeded')
                    const normalized = parts.filter(Boolean).join('/')
                    if (!normalized || seen.has(normalized)) throw new Error(`Duplicate ZIP entry: ${name}`)
                    seen.add(normalized)
                    const target = path.join(destination, ...parts)
                    if (name.endsWith('/')) await fsp.mkdir(target, { recursive: true })
                    else {
                        validateFileName(normalized)
                        if (entry.uncompressedSize > maxExtractBytes - bytes) throw new Error('ZIP extraction size limit exceeded')
                        const extension = path.extname(target).toLowerCase()
                        const manifest = ['library.json', 'package.json'].includes(path.basename(target).toLowerCase())
                        if ((extension === '.ini' || manifest) && entry.uncompressedSize > 1024 * 1024) throw new Error(`Project configuration file is too large: ${name}`)
                        await fsp.mkdir(path.dirname(target), { recursive: true })
                        let prefix = Buffer.alloc(0)
                        const counter = new Transform({ transform(chunk, encoding, callback) {
                            bytes += chunk.length
                            if (prefix.length < 5) prefix = Buffer.concat([prefix, chunk.subarray(0, 5 - prefix.length)])
                            const start = prefix.subarray(prefix.subarray(0, 3).equals(Buffer.from([0xef, 0xbb, 0xbf])) ? 3 : 0)
                            const error = bytes > maxExtractBytes ? new Error('ZIP extraction size limit exceeded') : chunk.includes(0) || start.subarray(0, 2).equals(Buffer.from('#!')) ? new Error(`Binary or script content is not allowed: ${name}`) : null
                            callback(error, chunk)
                        } })
                        await pipeline(await openEntry(zip, entry), counter, fs.createWriteStream(target, { flags: 'wx' }), { signal })
                        if (extension === '.ini') await validateIni(target, name)
                        if (manifest) await validateManifest(target, name)
                        if (path.basename(target).toLowerCase() === 'platformio.ini') roots.push(parts.slice(0, -1).join('/'))
                    }
                    zip.readEntry()
                } catch (error) { fail(error) }
            })
            zip.readEntry()
        })
    } finally { zip.close() }
    if (projectRoot !== undefined) {
        if (typeof projectRoot !== 'string' || projectRoot.includes('\\') || projectRoot.startsWith('/') || /^[A-Za-z]:/.test(projectRoot) || projectRoot.split('/').some(part => part === '..')) throw new Error('Invalid projectRoot')
        const selected = projectRoot === '.' ? '' : projectRoot.replace(/\/$/, '')
        if (!roots.includes(selected)) throw new Error('projectRoot does not contain platformio.ini')
        return path.join(destination, selected)
    }
    if (roots.length !== 1) throw new Error(roots.length ? `Ambiguous project roots: ${roots.map(root => root || '.').join(', ')}` : 'No platformio.ini found in ZIP')
    return path.join(destination, roots[0])
}