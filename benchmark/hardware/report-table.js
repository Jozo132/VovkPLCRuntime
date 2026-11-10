#!/usr/bin/env node
import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'
import { summarize } from './summary.js'
import { parseReport } from './benchmark.js'

const project = path.dirname(fileURLToPath(import.meta.url))
const startMarker = '<!-- hardware-benchmark-table:start -->'
const endMarker = '<!-- hardware-benchmark-table:end -->'

function cell(value) {
    return String(value ?? 'n/a').replaceAll('|', '\\|').replaceAll('\n', ' ')
}

export function renderTable(reports) {
    const entries = reports.map(report => ({ report, summary: summarize(report) }))
    entries.sort((first, second) => (second.report.complete ? second.summary.score_scans_per_second || 0 : 0) - (first.report.complete ? first.summary.score_scans_per_second || 0 : 0))
    const baselines = entries.filter(entry => entry.report.environment === 'avr_nano' && entry.report.complete && entry.summary.score_scans_per_second)
    const baseline = baselines.length === 1 ? baselines[0].summary.score_scans_per_second : null
    const names = summarize({ results: [] }).groups.map(group => group.name)
    const headers = ['Platform', 'CPU (MHz)', 'Score (scans/s)', 'Nano (x)', 'Flash (B)', 'RAM (B)', 'VM program (B)', 'VM data (B)', 'VM stack (B)', ...names.map(name => `${name} (us)`)]
    const lines = [`| ${headers.join(' | ')} |`, `| ${headers.map(() => '---').join(' | ')} |`]
    for (const { report, summary } of entries) {
        const score = report.complete && summary.score_scans_per_second ? Math.round(summary.score_scans_per_second).toLocaleString('en-US') : 'n/a'
        const relative = report.complete && baseline && summary.score_scans_per_second ? (summary.score_scans_per_second / baseline).toFixed(2) : 'n/a'
        const clock = Number.isFinite(report.clock_hz_nominal) && report.clock_hz_nominal > 0 ? report.clock_hz_nominal / 1000000 : 'n/a'
        const groupCells = summary.groups.map(group => group.count ? `${group.us_per_scan.toFixed(2)} (${group.count}/${group.total})` : `- (0/${group.total})`)
        const values = [report.device || report.board?.name || report.environment, clock, score, relative, report.board?.flash_bytes, report.board?.ram_bytes, report.vm_program_bytes, report.vm_memory_bytes, report.vm_stack_bytes, ...groupCells]
        lines.push(`| ${values.map(cell).join(' | ')} |`)
    }
    return lines.join('\n')
}

function main() {
    const argumentsList = process.argv.slice(2)
    if (!argumentsList.length || argumentsList.includes('--help')) {
        console.log('Usage: node benchmark/hardware/report-table.js [--write-readme] REPORT.json [REPORT.json ...]')
        return
    }
    const write = argumentsList.includes('--write-readme')
    const paths = argumentsList.filter(value => value !== '--write-readme')
    if (!paths.length) throw new Error('Provide at least one benchmark report')
    const reports = paths.map(file => parseReport(fs.readFileSync(file, 'utf8')))
    for (const report of reports) if (![1, 2].includes(report.schema) || !Array.isArray(report.results)) throw new Error('Unsupported report schema')
    const table = renderTable(reports)
    if (!write) { console.log(table); return }
    const readme = path.resolve(project, '../../README.md')
    const content = fs.readFileSync(readme, 'utf8')
    const start = content.indexOf(startMarker)
    const end = content.indexOf(endMarker)
    if (start < 0 || end < start || content.indexOf(startMarker, start + 1) !== -1 || content.indexOf(endMarker, end + 1) !== -1) throw new Error('README benchmark table markers are missing or duplicated')
    fs.writeFileSync(readme, `${content.slice(0, start + startMarker.length)}\n${table}\n${content.slice(end)}`)
}

if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
    try { main() } catch (error) { console.error(error.message); process.exitCode = 1 }
}