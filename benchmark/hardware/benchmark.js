#!/usr/bin/env node
import { SerialPort } from 'serialport'
import { spawnSync } from 'node:child_process'
import { fileURLToPath, pathToFileURL } from 'node:url'
import { createHash } from 'node:crypto'
import fs from 'node:fs'
import path from 'node:path'
import { createWorkloads } from './workloads.js'
import { summarize } from './summary.js'

const project = path.dirname(fileURLToPath(import.meta.url))
const root = path.resolve(project, '../..')
const args = process.argv.slice(2)
function option(name, fallback) {
    const index = args.indexOf(name)
    if (index < 0) return fallback
    if (!args[index + 1] || args[index + 1].startsWith('--')) throw new Error(`${name} requires a value`)
    return args[index + 1]
}

function positiveInt(value, name, maximum) {
    if (!/^\d+$/.test(value) || Number(value) < 1 || Number(value) > maximum) throw new Error(`Invalid ${name}: ${value}`)
    return Number(value)
}

function expandFields(value, fields) {
    return Object.fromEntries(Object.entries(fields).filter(([short]) => value[short] !== undefined).map(([short, key]) => [key, value[short]]))
}

export function uploadArguments(projectPath, environment, portName, uploadPort) {
    const argumentsList = ['run', '-d', projectPath, '-e', environment, '-t', 'upload']
    const selectedPort = uploadPort || portName
    if (selectedPort !== 'auto') argumentsList.push('--upload-port', selectedPort)
    return argumentsList
}

export function serializeReport(report) {
    const topFields = {
        created_at: 'at', environment: 'env', device: 'dev', port: 'p', clock_hz_nominal: 'hz',
        flags: 'fl', dispatch: 'dp', opcode_mask: 'om', vm_program_bytes: 'vp', vm_memory_bytes: 'vd',
        vm_stack_bytes: 'vs', scans: 'sc', samples: 'sn', firmware_sha256: 'fw',
        workload_source_sha256: 'wh', firmware_source_sha256: 'sh', platformio_config_sha256: 'pc',
        complete: 'done', unmeasured_supported_opcodes: 'uo',
    }
    const compact = { v: 2 }
    for (const [key, short] of Object.entries(topFields)) if (report[key] !== undefined) compact[short] = report[key]
    if (report.board) compact.bd = { id: report.board.id, name: report.board.name, mcu: report.board.mcu, f: report.board.flash_bytes, r: report.board.ram_bytes }
    compact.r = report.results.map(result => {
        const fields = { name: 'n', opcode: 'op', repeats: 'rep', expected_instructions: 'ins', samples_us: 'ts', status: 's', error: 'e', median_us_per_scan: 'us', speedup_vs_baseline: 'x' }
        const compactResult = {}
        for (const [key, short] of Object.entries(fields)) if (result[key] !== undefined) compactResult[short] = result[key]
        return compactResult
    })
    if (report.summary) compact.sum = {
        score: report.summary.score_scans_per_second,
        g: report.summary.groups.map(group => ({ n: group.name, c: group.count, t: group.total, us: group.us_per_scan })),
    }
    return `${JSON.stringify(compact)}\n`
}

export function parseReport(text) {
    const compact = JSON.parse(text)
    if (compact.schema === 1 && Array.isArray(compact.results)) return compact
    if (compact.v !== 2 || !Array.isArray(compact.r)) throw new Error('Unsupported report schema')
    const report = { schema: 2 }
    const topFields = {
        at: 'created_at', env: 'environment', dev: 'device', p: 'port', hz: 'clock_hz_nominal',
        fl: 'flags', dp: 'dispatch', om: 'opcode_mask', vp: 'vm_program_bytes', vd: 'vm_memory_bytes',
        vs: 'vm_stack_bytes', sc: 'scans', sn: 'samples', fw: 'firmware_sha256', wh: 'workload_source_sha256',
        sh: 'firmware_source_sha256', pc: 'platformio_config_sha256', done: 'complete', uo: 'unmeasured_supported_opcodes',
    }
    for (const [short, key] of Object.entries(topFields)) if (compact[short] !== undefined) report[key] = compact[short]
    if (compact.bd) report.board = expandFields(compact.bd, { id: 'id', name: 'name', mcu: 'mcu', f: 'flash_bytes', r: 'ram_bytes' })
    report.results = compact.r.map(result => expandFields(result, {
        n: 'name', op: 'opcode', rep: 'repeats', ins: 'expected_instructions', ts: 'samples_us',
        s: 'status', e: 'error', us: 'median_us_per_scan', x: 'speedup_vs_baseline',
    }))
    if (compact.sum) report.summary = {
        score_scans_per_second: compact.sum.score,
        groups: compact.sum.g.map(group => ({ name: group.n, count: group.c, total: group.t, us_per_scan: group.us })),
    }
    return report
}

export function firmwareFilename(platform, boardId) {
    if (platform === 'atmelavr') return 'firmware.hex'
    if (boardId === 'air105_coreboard') return 'program.bin'
    return 'firmware.bin'
}

async function main() {
    if (args.includes('--help')) {
        console.log('Usage: node benchmark/hardware/benchmark.js --env esp32c3 --port /dev/ttyACM0 [--upload-port PORT|auto] [--device NAME] [--scans 1000] [--samples 3] [--dispatch auto|switch|goto] [--output FILE] [--compare FILE] [--only NAME] [--skip-upload]')
        return
    }
    const environment = option('--env', 'esp32c3')
    const portName = option('--port', null)
    if (!portName) throw new Error('--port is required')
    const uploadPort = option('--upload-port', null)
    const device = option('--device', environment)
    const scans = positiveInt(option('--scans', '1000'), 'scans', 50000)
    const samples = positiveInt(option('--samples', '3'), 'samples', 100)
    const dispatch = option('--dispatch', 'auto')
    const only = option('--only', '')
    const comparePath = option('--compare', null)
    const output = option('--output', path.join(project, 'results', `${environment}-${new Date().toISOString().replaceAll(':', '-')}.json`))
    if (!['auto', 'switch', 'goto'].includes(dispatch)) throw new Error('Invalid --dispatch')
    const flags = dispatch === 'switch' ? '-DPLCRUNTIME_NO_COMPUTED_GOTO' : dispatch === 'goto' ? '-DPLCRUNTIME_FORCE_COMPUTED_GOTO' : ''
    if (!args.includes('--skip-upload')) {
        const build = spawnSync('pio', uploadArguments(project, environment, portName, uploadPort), {
            stdio: 'inherit', env: { ...process.env, VOVK_BENCH_FLAGS: flags }
        })
        if (build.error) throw build.error
        if (build.status !== 0) throw new Error(`PlatformIO failed: ${build.status}`)
    }
    const port = new SerialPort({ path: portName, baudRate: 115200, autoOpen: false })
    await new Promise((resolve, reject) => port.open(error => error ? reject(error) : resolve()))
    if (environment === 'air105_core_v1_204mhz') {
        await new Promise((resolve, reject) => port.set({ rts: false, dtr: false }, error => error ? reject(error) : resolve()))
    }
    let buffer = ''
    const lines = []
    port.on('data', data => {
        buffer += data.toString('utf8')
        const parts = buffer.split('\n')
        buffer = parts.pop()
        lines.push(...parts.map(line => line.trim()).filter(line => line.startsWith('@')))
    })
    async function request(command, prefix, timeout = 10000) {
        lines.length = 0
        const message = `${command}\n`
        for (let offset = 0; offset < message.length; offset += 32) {
            await new Promise((resolve, reject) => port.write(message.slice(offset, offset + 32), error => error ? reject(error) : resolve()))
            await new Promise((resolve, reject) => port.drain(error => error ? reject(error) : resolve()))
            await new Promise(resolve => setTimeout(resolve, 2))
        }
        const deadline = Date.now() + timeout
        while (Date.now() < deadline) {
            while (lines.length) {
                const line = lines.shift()
                if (line.startsWith('@ERROR,')) throw new Error(`Device: ${line}`)
                if (line.startsWith(prefix)) return line
            }
            await new Promise(resolve => setTimeout(resolve, 10))
        }
        throw new Error(`Timed out waiting for ${prefix}`)
    }
    try {
        let info
        for (let attempt = 0; attempt < 5; attempt++) {
            try { info = await request('I', '@INFO,', 2000); break } catch (error) { if (attempt === 4) throw error }
        }
        const [, version, clock, flagsValue, actualDispatch, mask, programLimit, memoryLimit, stackLimit] = info.split(',')
        if (version !== '2' || !/^\d+$/.test(clock) || !/^\d+$/.test(flagsValue) || !/^[0-9A-F]{64}$/.test(mask) || ![programLimit, memoryLimit, stackLimit].every(value => /^\d+$/.test(value))) throw new Error(`Invalid device information: ${info}`)
        if (dispatch !== 'auto' && actualDispatch !== dispatch) throw new Error(`Requested ${dispatch}, device reports ${actualDispatch}`)
        const flagsValueNumber = Number(flagsValue)
        const supported = opcode => (parseInt(mask[Math.floor(opcode / 4)], 16) & (1 << (opcode % 4))) !== 0
        const workloads = createWorkloads()
        const selected = only ? workloads.filter(item => item.name.includes(only)) : workloads
        if (!selected.length) throw new Error(`No workloads match ${only}`)
        const digest = data => createHash('sha256').update(data).digest('hex')
        const pioJson = argumentsList => {
            const result = spawnSync('pio', argumentsList, { cwd: root, encoding: 'utf8' })
            if (result.status !== 0) return null
            try { return JSON.parse(result.stdout) } catch { return null }
        }
        const config = pioJson(['project', 'config', '-d', project, '--json-output'])
        const environmentConfig = config?.find(entry => entry[0] === `env:${environment}`)?.[1]
        const boardId = environmentConfig?.find(entry => entry[0] === 'board')?.[1]
        const platform = environmentConfig?.find(entry => entry[0] === 'platform')?.[1]
        const firmware = path.join(project, '.pio', 'build', environment, firmwareFilename(platform, boardId))
        const boards = boardId ? pioJson(['boards', boardId, '--json-output']) : null
        const board = boards?.find(entry => entry.id === boardId) || null
        const report = {
            schema: 2,
            created_at: new Date().toISOString(),
            environment,
            device,
            board: board && { id: board.id, name: board.name, mcu: board.mcu, flash_bytes: board.rom, ram_bytes: board.ram },
            port: portName,
            clock_hz_nominal: Number(clock),
            flags: flagsValueNumber,
            dispatch: actualDispatch,
            opcode_mask: mask,
            vm_program_bytes: Number(programLimit),
            vm_memory_bytes: Number(memoryLimit),
            vm_stack_bytes: Number(stackLimit),
            scans,
            samples,
            firmware_sha256: !args.includes('--skip-upload') && fs.existsSync(firmware) ? digest(fs.readFileSync(firmware)) : null,
            workload_source_sha256: digest(fs.readFileSync(path.join(project, 'workloads.js'))),
            firmware_source_sha256: digest(fs.readFileSync(path.join(project, 'src', 'main.cpp'))),
            platformio_config_sha256: digest(fs.readFileSync(path.join(project, 'platformio.ini'))),
            complete: false,
            results: [],
        }
        const previous = comparePath ? parseReport(fs.readFileSync(comparePath, 'utf8')) : null
        const previousCases = new Map((previous?.results || []).map(result => [result.name, result]))
        let interrupted = false
        for (const item of selected) {
            const hex = Buffer.from(item.bytecode).toString('hex').toUpperCase()
            const result = { name: item.name, opcode: item.opcode, repeats: item.repeats, expected_instructions: item.instructions, samples_us: [] }
            report.results.push(result)
            if (!supported(item.opcode) || (flagsValueNumber & (item.requires || 0)) !== (item.requires || 0)) {
                result.status = 'unsupported'
                continue
            }
            if (item.bytecode.length > Number(programLimit)) {
                result.status = 'program_too_large'
                continue
            }
            for (let sample = -1; sample < samples; sample++) {
                let response
                try { response = await request(`B,${scans},${hex}`, '@RESULT,', 60000) } catch (error) {
                    result.status = 'failed'
                    result.error = `${item.name}: ${error.message}`
                    interrupted = true
                    break
                }
                const [, completed, elapsed, status, stack, instructions] = response.split(',').map(Number)
                if (completed !== scans || !Number.isFinite(elapsed) || elapsed <= 0 || status !== 0 || stack !== 0 || instructions !== item.instructions) {
                    result.status = 'failed'
                    result.error = response
                    break
                }
                if (sample >= 0) result.samples_us.push(elapsed)
            }
            if (!result.status) {
                result.status = 'ok'
                const ordered = [...result.samples_us].sort((first, second) => first - second)
                result.median_us_per_scan = (ordered.length % 2 ? ordered[(ordered.length - 1) / 2] : (ordered[ordered.length / 2 - 1] + ordered[ordered.length / 2]) / 2) / scans
                const prior = previousCases.get(item.name)
                if (previous?.workload_source_sha256 === report.workload_source_sha256 && prior?.status === 'ok' && prior.opcode === item.opcode && prior.expected_instructions === item.instructions && prior.median_us_per_scan > 0) result.speedup_vs_baseline = prior.median_us_per_scan / result.median_us_per_scan
            }
            console.log(`${result.name}: ${result.status}${result.status === 'ok' ? ` ${result.median_us_per_scan.toFixed(3)} us/scan${result.speedup_vs_baseline ? ` (${result.speedup_vs_baseline.toFixed(3)}x)` : ''}` : ` ${result.error || ''}`}`)
            if (interrupted) break
        }
        report.complete = !interrupted && !only && report.results.length === workloads.length && !report.results.some(result => result.status === 'failed')
        report.summary = summarize(report)
        const measuredOpcodes = new Set(report.results.filter(result => result.status === 'ok').map(result => result.opcode))
        report.unmeasured_supported_opcodes = Array.from({ length: 256 }, (_, opcode) => opcode).filter(opcode => supported(opcode) && !measuredOpcodes.has(opcode))
        fs.mkdirSync(path.dirname(path.resolve(output)), { recursive: true })
        fs.writeFileSync(output, serializeReport(report))
        console.log(`Saved ${output}; ${report.unmeasured_supported_opcodes.length} supported opcodes not measured.`)
        if (report.results.some(result => result.status === 'failed')) process.exitCode = 1
    } finally {
        await new Promise(resolve => port.close(resolve))
    }
}

if (process.argv[1] && import.meta.url === pathToFileURL(path.resolve(process.argv[1])).href) {
    main().catch(error => { console.error(error.message); process.exitCode = 1 })
}