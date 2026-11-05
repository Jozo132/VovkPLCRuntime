import test from 'node:test'
import assert from 'node:assert/strict'
import fs from 'node:fs/promises'
import os from 'node:os'
import path from 'node:path'
import { extractProject } from './project-loader.js'

function zip(entries) {
    const locals = []
    const central = []
    let offset = 0
    for (const [name, content, mode] of entries) {
        const filename = Buffer.from(name)
        const data = Buffer.from(content)
        const local = Buffer.alloc(30)
        local.writeUInt32LE(0x04034b50, 0); local.writeUInt16LE(20, 4); local.writeUInt32LE(data.length, 18); local.writeUInt32LE(data.length, 22); local.writeUInt16LE(filename.length, 26)
        const header = Buffer.alloc(46)
        header.writeUInt32LE(0x02014b50, 0); header.writeUInt16LE(20, 6); header.writeUInt32LE(data.length, 20); header.writeUInt32LE(data.length, 24); header.writeUInt16LE(filename.length, 28); header.writeUInt32LE(offset, 42)
        if (mode) { header.writeUInt16LE(0x0314, 4); header.writeUInt32LE((mode * 65536) >>> 0, 38) }
        locals.push(local, filename, data); central.push(header, filename)
        offset += local.length + filename.length + data.length
    }
    const directory = Buffer.concat(central)
    const end = Buffer.alloc(22)
    end.writeUInt32LE(0x06054b50, 0); end.writeUInt16LE(entries.length, 8); end.writeUInt16LE(entries.length, 10); end.writeUInt32LE(directory.length, 12); end.writeUInt32LE(offset, 16)
    return Buffer.concat([...locals, directory, end])
}

test('extracts one nested project and rejects traversal', async () => {
    const directory = await fs.mkdtemp(path.join(os.tmpdir(), 'firmware-loader-'))
    try {
        const archive = path.join(directory, 'source.zip')
        await fs.writeFile(archive, zip([['project/platformio.ini', '[env:test]'], ['project/src/main.cpp', 'int main() {}']]))
        assert.equal(await extractProject(archive, path.join(directory, 'valid')), path.join(directory, 'valid/project'))
        await fs.writeFile(archive, zip([['../escape', 'bad']]))
        await assert.rejects(extractProject(archive, path.join(directory, 'invalid')), /invalid relative path|Unsafe ZIP path/i)
        await assert.rejects(fs.stat(path.join(directory, 'escape')), { code: 'ENOENT' })
    } finally { await fs.rm(directory, { recursive: true, force: true }) }
})

test('rejects links, absolute paths, duplicate entries, ambiguous roots and size limits', async () => {
    const directory = await fs.mkdtemp(path.join(os.tmpdir(), 'firmware-loader-'))
    try {
        const archive = path.join(directory, 'source.zip')
        const check = async (entries, options, pattern) => {
            await fs.writeFile(archive, zip(entries))
            await assert.rejects(extractProject(archive, path.join(directory, `out-${Math.random()}`), options), pattern)
        }
        await check([['link', '../outside', 0o120777]], {}, /unsafe.*type/i)
        await check([['/absolute', 'x']], {}, /absolute|unsafe/i)
        await check([['platformio.ini', 'x'], ['platformio.ini', 'x']], {}, /duplicate/i)
        await check([['one/platformio.ini', 'x'], ['two/platformio.ini', 'x']], {}, /ambiguous/i)
        await check([['platformio.ini', '12345']], { maxExtractBytes: 4 }, /size limit/i)
        await check([['platformio.ini', 'x'], ['main.c', 'x']], { maxFiles: 1 }, /count limit/i)
        await fs.writeFile(archive, zip([['one/platformio.ini', 'x'], ['two/platformio.ini', 'x']]))
        assert.equal(await extractProject(archive, path.join(directory, 'selected'), { projectRoot: 'two' }), path.join(directory, 'selected/two'))
    } finally { await fs.rm(directory, { recursive: true, force: true }) }
})

test('rejects build scripts and PlatformIO script hooks', async () => {
    const directory = await fs.mkdtemp(path.join(os.tmpdir(), 'firmware-loader-'))
    try {
        const archive = path.join(directory, 'source.zip')
        for (const [entries, pattern] of [
            [[['platformio.ini', '[env:demo]'], ['scripts/pre.py', 'print(1)']], /scripts and executables/i],
            [[['platformio.ini', '[env:demo]\nextra_scripts = pre:build.py']], /script hooks/i],
            [[['platformio.ini', '[platformio]\nextra_configs = other.ini']], /extra configuration/i],
            [[['platformio.ini', '[env:demo]\nbuild_flags = !python generate.py']], /command-generated/i],
            [[['platformio.ini', '[env:demo]\nbuild_flags =\n  !echo flag']], /command-generated/i],
            [[['platformio.ini', '[env:demo]'], ['CMakeLists.txt', 'execute_process(COMMAND sh)']], /scripts and executables/i],
            [[['platformio.ini', '[env:demo]'], ['lib/test/library.json', '{"build":{"extraScript":"run.py"}}']], /script hooks/i],
            [[['platformio.ini', '[env:demo]'], ['notes.txt', '#!/bin/sh\necho test']], /script content/i],
            [[['platformio.ini', '[env:demo]'], ['src/main.cpp', Buffer.from([0x7f, 0x45, 0x4c, 0x46, 0])]], /binary/i],
        ]) {
            await fs.writeFile(archive, zip(entries))
            await assert.rejects(extractProject(archive, path.join(directory, `out-${Math.random()}`)), pattern)
        }
    } finally { await fs.rm(directory, { recursive: true, force: true }) }
})

export { zip }