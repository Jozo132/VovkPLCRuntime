import test from 'node:test'
import assert from 'node:assert/strict'
import { createHash } from 'node:crypto'
import { readFileSync } from 'node:fs'
import { createWorkloads } from './workloads.js'
import { coreCases, summarize } from './summary.js'
import { renderTable } from './report-table.js'
import { firmwareFilename, parseReport, serializeReport, uploadArguments } from './benchmark.js'

const workloadSourceHash = createHash('sha256').update(readFileSync(new URL('./workloads.js', import.meta.url))).digest('hex')

function report(name, time) {
    return {
        complete: true, device: name, git_revision: '0123456789abcdef', git_dirty: false, workload_source_sha256: workloadSourceHash,
        results: createWorkloads().map(item => ({ name: item.name, opcode: item.opcode, expected_instructions: item.instructions, bytecode: Buffer.from(item.bytecode).toString('hex').toUpperCase(), status: 'ok', median_us_per_scan: time })),
    }
}

test('upload port can be independent of the measurement port', () => {
    const base = ['run', '-d', '/project', '-e', 'target', '-t', 'upload']
    assert.deepEqual(uploadArguments('/project', 'target', '/dev/ttyUSB0', null), [...base, '--upload-port', '/dev/ttyUSB0'])
    assert.deepEqual(uploadArguments('/project', 'target', '/dev/ttyUSB0', '/dev/ttyACM0'), [...base, '--upload-port', '/dev/ttyACM0'])
    assert.deepEqual(uploadArguments('/project', 'target', '/dev/ttyUSB0', 'auto'), base)
})

test('firmware filename follows the target platform', () => {
    assert.equal(firmwareFilename('atmelavr', 'uno'), 'firmware.hex')
    assert.equal(firmwareFilename('https://github.com/Jozo132/PIO-Air105.git', 'air105_coreboard'), 'program.bin')
    assert.equal(firmwareFilename('raspberrypi', 'pico'), 'firmware.bin')
})

test('benchmark reports serialize compactly without changing their data', () => {
    const value = report('first', 10)
    const serialized = serializeReport(value)
    const compact = JSON.parse(serialized)
    assert.equal(compact.v, 2)
    assert.equal(compact.r[0].n, value.results[0].name)
    assert.equal('bytecode' in compact.r[0], false)
    assert.equal(Object.keys(compact).some(key => key.startsWith('git_')), false)
    assert.deepEqual(parseReport(serialized).results.map(result => result.median_us_per_scan), value.results.map(result => result.median_us_per_scan))
})

test('legacy benchmark reports remain readable', () => {
    const value = report('first', 10)
    value.schema = 1
    assert.deepEqual(parseReport(JSON.stringify(value)), value)
})

test('workloads have unique names, bounded bytecode, and valid instruction counts', () => {
    const workloads = createWorkloads()
    assert.equal(new Set(workloads.map(item => item.name)).size, workloads.length)
    for (const item of workloads) {
        assert.ok(item.bytecode.length > 0 && item.bytecode.length <= 384, item.name)
        assert.ok(item.bytecode.every(byte => Number.isInteger(byte) && byte >= 0 && byte <= 255), item.name)
        assert.ok(item.instructions > 0 && item.repeats > 0, item.name)
    }
    for (const name of coreCases) assert.ok(workloads.some(item => item.name === name), name)
})

test('common-core score requires all core cases to be measured', () => {
    const result = report('first', 10)
    assert.ok(Math.abs(summarize(result).score_scans_per_second - 100000) < 0.001)
    result.workload_source_sha256 = 'different-workloads'
    assert.equal(summarize(result).score_scans_per_second, null)
    result.workload_source_sha256 = workloadSourceHash
    result.results.find(item => item.name === coreCases[0]).status = 'unsupported'
    assert.equal(summarize(result).score_scans_per_second, null)
    assert.equal(summarize({ results: [] }).score_scans_per_second, null)
})

test('group coverage includes unsupported and missing cases', () => {
    const result = report('first', 10)
    result.results = result.results.filter(item => item.name !== 'logic_and')
    result.results.find(item => item.name === 'logic_or').status = 'unsupported'
    const group = summarize(result).groups.find(item => item.name === 'Bool')
    assert.deepEqual([group.count, group.total], [3, 5])
})

test('table ranks complete reports without Git metadata', () => {
    const slower = report('slow', 20)
    const faster = report('fast', 10)
    const table = renderTable([slower, faster])
    assert.ok(table.indexOf('| fast |') < table.indexOf('| slow |'))
    assert.equal(table.includes('Commit'), false)
    assert.match(table, /100,000/)
    slower.complete = false
    assert.match(renderTable([slower]), /\| n\/a \|/)
})

test('table normalizes complete compatible scores against one Nano baseline', () => {
    const nano = { ...report('Nano', 20), environment: 'avr_nano' }
    const faster = report('fast', 10)
    assert.match(renderTable([nano, faster]), /\| fast \|[^\n]*\| 100,000 \| 2\.00 \|/)
    assert.match(renderTable([nano, faster]), /\| Nano \|[^\n]*\| 50,000 \| 1\.00 \|/)
    assert.match(renderTable([faster]), /\| fast \|[^\n]*\| 100,000 \| n\/a \|/)
    assert.match(renderTable([nano, nano, faster]), /\| fast \|[^\n]*\| 100,000 \| n\/a \|/)
    nano.complete = false
    assert.match(renderTable([nano, faster]), /\| fast \|[^\n]*\| 100,000 \| n\/a \|/)
})