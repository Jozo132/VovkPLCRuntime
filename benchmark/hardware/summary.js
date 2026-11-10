import { createHash } from 'node:crypto'
import { readFileSync } from 'node:fs'
import { createWorkloads } from './workloads.js'

export const coreCases = ['cycle_exit', 'add_u8', 'add_u16', 'cmp_eq_u8', 'logic_and', 'bit_read_0', 'bit_write_0', 'flow_jump']
const workloadSourceHash = createHash('sha256').update(readFileSync(new URL('./workloads.js', import.meta.url))).digest('hex')

const numeric = /^(const|add|sub|mul|div|mod|pow|neg|abs|sqrt|sin|cos)_(u8|i8|u16|i16|u32|i32|u64|i64|f32|f64)$/
const groups = [
    ['Cycle', name => name === 'cycle_exit'],
    ['Comparison', name => name.startsWith('cmp_')],
    ['Bitwise', name => name.startsWith('bitwise_') || name.startsWith('bit_')],
    ['Bool', name => name.startsWith('logic_') || name === 'const_bool'],
    ...[['u8/i8', ['u8', 'i8']], ['u16/i16', ['u16', 'i16']], ['u32/i32', ['u32', 'i32']], ['u64/i64', ['u64', 'i64']], ['f32', ['f32']], ['f64', ['f64']]].map(([label, types]) => [label, name => {
        const match = name.match(numeric)
        return !!match && types.includes(match[2])
    }]),
    ['Strings', name => name.startsWith('string_')],
    ['Flow', name => name.startsWith('flow_')],
    ['Stack/memory', name => /^(load_|move_|inc_|dec_|copy_|swap_|drop_|clear$|pick_|poke_|mem_fill$|cvt_|branch_|const_pointer$)/.test(name)],
    ['Timers', name => name.startsWith('timer_')],
    ['Counters', name => name.startsWith('counter_')],
    ['Edges', name => name.startsWith('edge_')],
    ['Other', name => /^(nop$|ffi_|comms_|config_|lang$|comment$)/.test(name)],
]

function geometricMean(values) {
    return Math.exp(values.reduce((sum, value) => sum + Math.log(value), 0) / values.length)
}

export function summarize(report) {
    const cases = new Map((report.results || []).map(result => [result.name, result]))
    const catalog = createWorkloads()
    const workloads = new Map(catalog.map(item => [item.name, item]))
    const compatible = report.workload_source_sha256 === workloadSourceHash
    const valid = result => {
        const workload = workloads.get(result?.name)
        return compatible && workload && result.status === 'ok' && result.opcode === workload.opcode && result.expected_instructions === workload.instructions && Number.isFinite(result.median_us_per_scan) && result.median_us_per_scan > 0
    }
    const core = coreCases.map(name => cases.get(name))
    const comparable = core.every(valid)
    const summaries = groups.map(([name, matches]) => {
        const members = catalog.filter(item => matches(item.name))
        const measured = members.map(item => cases.get(item.name)).filter(valid)
        return { name, count: measured.length, total: members.length, us_per_scan: measured.length ? geometricMean(measured.map(result => result.median_us_per_scan)) : null }
    })
    return { score_scans_per_second: comparable ? 1000000 / geometricMean(core.map(result => result.median_us_per_scan)) : null, groups: summaries }
}