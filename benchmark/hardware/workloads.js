const FLAGS = { STRINGS: 0x02, COUNTERS: 0x04, TIMERS: 0x08, FFI: 0x10, X64: 0x20, FLOAT: 0x100, ADVANCED: 0x200, INT32: 0x400, CVT: 0x800, STACK: 0x1000, BITWISE: 0x2000 }
const REPEATS = 8
const EXIT = 0xff
const U8 = 0x03
const DROP = 0x16
const ptr = [0x2c, 0x01]

const types = [
    { name: 'u8', opcode: 0x03, size: 1 },
    { name: 'u16', opcode: 0x04, size: 2 },
    { name: 'u32', opcode: 0x05, size: 4, requires: FLAGS.INT32 },
    { name: 'u64', opcode: 0x06, size: 8, requires: FLAGS.X64 },
    { name: 'i8', opcode: 0x07, size: 1, signed: true },
    { name: 'i16', opcode: 0x08, size: 2, signed: true },
    { name: 'i32', opcode: 0x09, size: 4, signed: true, requires: FLAGS.INT32 },
    { name: 'i64', opcode: 0x0a, size: 8, signed: true, requires: FLAGS.X64 },
    { name: 'f32', opcode: 0x0b, size: 4, float: true, requires: FLAGS.FLOAT },
    { name: 'f64', opcode: 0x0c, size: 8, float: true, requires: FLAGS.X64 },
]

function constant(type, value) {
    const buffer = Buffer.alloc(type.size)
    if (type.float) {
        if (type.size === 4) buffer.writeFloatLE(value)
        else buffer.writeDoubleLE(value)
    } else buffer[0] = value
    return [type.opcode, ...buffer]
}

function workload(name, opcode, unit, instructions, requires = 0) {
    return { name, opcode, requires, repeats: REPEATS, instructions: instructions * REPEATS + 1, bytecode: [...Array(REPEATS).fill(unit).flat(), EXIT] }
}

function jumpWorkload(name, opcode, condition, relative) {
    const bytecode = []
    for (let index = 0; index < REPEATS; index++) {
        if (condition !== null) bytecode.push(U8, condition)
        const target = relative ? 0 : bytecode.length + 3
        bytecode.push(opcode, target & 0xff, target >> 8)
    }
    bytecode.push(EXIT)
    return { name, opcode, repeats: REPEATS, instructions: REPEATS * (condition === null ? 1 : 2) + 1, bytecode }
}

function callWorkload(name, opcode, callOpcode, condition, relative, returnOpcode, returnCondition) {
    const bytecode = []
    const callPositions = []
    for (let index = 0; index < REPEATS; index++) {
        if (condition !== null) bytecode.push(U8, condition)
        callPositions.push(bytecode.length)
        bytecode.push(callOpcode, 0, 0)
    }
    bytecode.push(EXIT)
    const target = bytecode.length
    if (returnCondition !== null) bytecode.push(U8, returnCondition)
    bytecode.push(returnOpcode)
    for (const position of callPositions) {
        const operand = relative ? target - (position + 3) : target
        bytecode[position + 1] = operand & 0xff
        bytecode[position + 2] = operand >> 8
    }
    return { name, opcode, repeats: REPEATS, instructions: REPEATS * (2 + (condition !== null ? 1 : 0) + (returnCondition !== null ? 1 : 0)) + 1, bytecode }
}

export function createWorkloads() {
    const cases = [
        { name: 'cycle_exit', opcode: EXIT, requires: 0, repeats: 1, instructions: 1, bytecode: [EXIT] },
        workload('nop', 0x00, [0x00], 1),
    ]
    for (const type of types) {
        const push = value => constant(type, value)
        const drop = [DROP, type.opcode]
        cases.push(workload(`const_${type.name}`, type.opcode, [...push(4), ...drop], 2, type.requires))
        for (const [name, opcode] of [['add', 0x20], ['sub', 0x21], ['mul', 0x22], ['div', 0x23], ['mod', 0x24], ['pow', 0x25]]) {
            if (type.float && name === 'mod') continue
            const requires = (type.requires || 0) | (name === 'pow' ? FLAGS.ADVANCED : 0)
            cases.push(workload(`${name}_${type.name}`, opcode, [...push(4), ...push(2), opcode, type.opcode, ...drop], 4, requires))
        }
        if (type.signed || type.float) {
            for (const [name, opcode] of [['neg', 0x27], ['abs', 0x28]]) {
                cases.push(workload(`${name}_${type.name}`, opcode, [...push(4), opcode, type.opcode, ...drop], 3, type.requires))
            }
        }
        if (type.float) {
            for (const [name, opcode] of [['sqrt', 0x26], ['sin', 0x29], ['cos', 0x2a]]) {
                cases.push(workload(`${name}_${type.name}`, opcode, [...push(4), opcode, type.opcode, ...drop], 3, (type.requires || 0) | FLAGS.ADVANCED))
            }
        }
        for (const [name, opcode] of [['eq', 0xd0], ['neq', 0xd1], ['gt', 0xd2], ['lt', 0xd3], ['gte', 0xd4], ['lte', 0xd5]]) {
            cases.push(workload(`cmp_${name}_${type.name}`, opcode, [...push(4), ...push(2), opcode, type.opcode, DROP, U8], 4, type.requires))
        }
    }
    const pushU8 = [U8, 1]
    for (const [name, opcode, unary] of [['and', 0xc0, false], ['or', 0xc1, false], ['xor', 0xc2, false], ['not', 0xc3, true]]) {
        cases.push(workload(`logic_${name}`, opcode, [...pushU8, ...(unary ? [] : pushU8), opcode, DROP, U8], unary ? 3 : 4))
    }
    const widths = types.filter(type => ['u8', 'u16', 'u32', 'u64'].includes(type.name))
    for (const [name, base, unary, shift] of [['and', 0xa0, false, false], ['or', 0xa4, false, false], ['xor', 0xa8, false, false], ['not', 0xac, true, false], ['lshift', 0xb0, false, true], ['rshift', 0xb4, false, true]]) {
        widths.forEach((type, index) => {
            const opcode = base + index
            const second = unary ? [] : constant(shift ? types[0] : type, 1)
            cases.push(workload(`bitwise_${name}_${type.name}`, opcode, [...constant(type, 4), ...second, opcode, DROP, type.opcode], unary ? 3 : 4, FLAGS.BITWISE | (type.requires || 0)))
        })
    }
    for (const [name, base, mode] of [
        ['get', 0x40, 'stack'], ['set', 0x48, 'stack'], ['reset', 0x50, 'stack'],
        ['read', 0x58, 'read'], ['write', 0x60, 'write'], ['write_set', 0x68, 'memory'],
        ['write_reset', 0x70, 'memory'], ['write_invert', 0x78, 'memory'],
    ]) {
        for (let bit = 0; bit < 8; bit++) {
            const opcode = base + bit
            let unit
            let count
            if (mode === 'stack') { unit = [...pushU8, opcode, DROP, U8]; count = 3 }
            else if (mode === 'read') { unit = [opcode, ...ptr, DROP, U8]; count = 2 }
            else if (mode === 'write') { unit = [...pushU8, opcode, ...ptr]; count = 2 }
            else { unit = [opcode, ...ptr]; count = 1 }
            cases.push(workload(`bit_${name}_${bit}`, opcode, unit, count))
        }
    }
    for (const [name, opcode] of [['ton', 0x30], ['tof', 0x32], ['tp', 0x34]]) {
        cases.push(workload(`timer_${name}_const`, opcode, [...pushU8, opcode, ...ptr, 10, 0, 0, 0, DROP, U8], 3, FLAGS.TIMERS))
        cases.push(workload(`timer_${name}_mem`, opcode + 1, [...pushU8, opcode + 1, ...ptr, 0x54, 0x01, DROP, U8], 3, FLAGS.TIMERS))
    }
    for (const [name, opcode] of [['ctu', 0x36], ['ctd', 0x38]]) {
        cases.push(workload(`counter_${name}_const`, opcode, [...pushU8, ...pushU8, opcode, ...ptr, 10, 0, 0, 0, DROP, U8], 4, FLAGS.COUNTERS))
        cases.push(workload(`counter_${name}_mem`, opcode + 1, [...pushU8, ...pushU8, opcode + 1, ...ptr, 0x54, 0x01, DROP, U8], 4, FLAGS.COUNTERS))
    }
    for (const [name, opcode] of [['read_du', 0x80], ['read_dd', 0x81], ['read_inv_du', 0x82], ['read_inv_dd', 0x83]]) {
        cases.push(workload(`edge_${name}`, opcode, [opcode, ...ptr, 0, 0x36, 0x01, 0, DROP, U8], 2))
    }
    for (const [name, opcode] of [['write_du', 0x84], ['write_dd', 0x85], ['write_inv_du', 0x86], ['write_inv_dd', 0x87], ['set_du', 0x88], ['set_dd', 0x89], ['reset_du', 0x8a], ['reset_dd', 0x8b]]) {
        cases.push(workload(`edge_${name}`, opcode, [...pushU8, opcode, ...ptr, 0, 0x36, 0x01, 0], 2))
    }
    for (const [name, opcode] of [['du', 0x8c], ['dd', 0x8d], ['dc', 0x8e]]) {
        cases.push(workload(`edge_stack_${name}`, opcode, [...pushU8, opcode, ...ptr, 0, DROP, U8], 3))
    }
    for (const [name, opcode, condition, relative] of [
        ['jump', 0xe0, null, false], ['jump_if', 0xe1, 1, false], ['jump_if_not', 0xe2, 0, false],
        ['jump_rel', 0xe9, null, true], ['jump_if_rel', 0xea, 1, true], ['jump_if_not_rel', 0xeb, 0, true],
    ]) cases.push(jumpWorkload(`flow_${name}`, opcode, condition, relative))
    for (const [name, opcode, condition, relative] of [
        ['call', 0xe3, null, false], ['call_if', 0xe4, 1, false], ['call_if_not', 0xe5, 0, false],
        ['call_rel', 0xec, null, true], ['call_if_rel', 0xed, 1, true], ['call_if_not_rel', 0xee, 0, true],
    ]) cases.push(callWorkload(`flow_${name}`, opcode, opcode, condition, relative, 0xe6, null))
    for (const [name, opcode, condition] of [['ret', 0xe6, null], ['ret_if', 0xe7, 1], ['ret_if_not', 0xe8, 0]]) {
        cases.push(callWorkload(`flow_${name}`, opcode, 0xe3, null, false, opcode, condition))
    }
    const stringType = 0x0e
    const secondString = [0x40, 0x01]
    const stringInit = address => [0x04, 8, 0, 0xf4, stringType, ...address, 0xf5, stringType, ...address, 2, 0, 0x34, 0x32]
    const stringSingle = opcode => [opcode, stringType, ...ptr]
    const stringDual = opcode => [opcode, stringType, stringType, ...ptr, ...secondString]
    const stringOne = stringInit(ptr)
    const stringTwo = [...stringOne, ...stringInit(secondString)]
    const stringCases = [
        ['len', 0x94, stringOne, [...stringSingle(0x94), DROP, 0x04], 5],
        ['cap', 0x95, stringOne, [...stringSingle(0x95), DROP, 0x04], 5],
        ['get', 0x96, stringOne, [0x04, 0, 0, ...stringSingle(0x96), DROP, U8], 6],
        ['set', 0x97, stringOne, [0x04, 0, 0, U8, 0x33, ...stringSingle(0x97)], 6],
        ['clear', 0x98, stringOne, stringSingle(0x98), 4],
        ['cmp', 0x99, stringTwo, [...stringDual(0x99), DROP, 0x07], 8],
        ['eq', 0x9a, stringTwo, [...stringDual(0x9a), DROP, U8], 8],
        ['concat', 0x9b, stringTwo, stringDual(0x9b), 7],
        ['copy', 0x9c, stringTwo, stringDual(0x9c), 7],
        ['substr', 0x9d, stringTwo, [0x04, 0, 0, 0x04, 1, 0, ...stringDual(0x9d)], 9],
        ['find', 0x9e, stringTwo, [...stringDual(0x9e), DROP, 0x08], 8],
        ['char', 0x9f, stringOne, [U8, 0x33, ...stringSingle(0x9f)], 5],
        ['to_num', 0xf2, stringOne, [0xf2, stringType, ...ptr, U8, DROP, U8], 5],
        ['from_num', 0xf3, stringOne, [U8, 42, 0xf3, stringType, ...ptr, U8, 10], 5],
        ['init', 0xf4, [], [0x04, 8, 0, ...stringSingle(0xf4)], 2],
        ['literal', 0xf5, stringOne, [0xf5, stringType, ...ptr, 2, 0, 0x34, 0x32], 4],
        ['literal_concat', 0xf8, stringOne, [0xf8, stringType, ...ptr, 2, 0, 0x34, 0x32], 4],
    ]
    for (const [name, opcode, setup, operation, instructions] of stringCases) {
        cases.push(workload(`string_${name}`, opcode, [...setup, ...operation], instructions, FLAGS.STRINGS))
    }
    for (const [name, opcode] of [['literal_copy', 0xf6], ['literal_eq', 0xf7]]) {
        const unit = [...stringOne, opcode, 0x10, stringType, ...ptr, 0, 0, ...(opcode === 0xf7 ? [DROP, U8] : [])]
        const item = workload(`string_${name}`, opcode, unit, opcode === 0xf7 ? 5 : 4, FLAGS.STRINGS)
        const constantOffset = item.bytecode.length
        for (let index = 0; index < REPEATS; index++) {
            const offset = index * unit.length + stringOne.length + 5
            item.bytecode[offset] = constantOffset & 0xff
            item.bytecode[offset + 1] = constantOffset >> 8
        }
        item.bytecode.push(2, 0x34, 0x32)
        cases.push(item)
    }
    cases.push(
        workload('const_pointer', 0x01, [0x01, ...ptr, DROP, 0x01], 2),
        workload('const_bool', 0x02, [0x02, 1, DROP, 0x02], 2),
        workload('load_from_u8', 0x18, [0x18, U8, ...ptr, DROP, U8], 2),
        workload('move_to_u8', 0x19, [...pushU8, 0x19, U8, ...ptr], 2),
        workload('inc_u16', 0x1a, [0x1a, 0x04, ...ptr], 1),
        workload('dec_u16', 0x1b, [0x1b, 0x04, ...ptr], 1),
        workload('load_u8', 0x11, [0x01, ...ptr, 0x11, U8, DROP, U8], 3),
        workload('move_u8', 0x12, [0x01, ...ptr, ...pushU8, 0x12, U8], 3),
        workload('move_copy_u8', 0x13, [0x01, ...ptr, ...pushU8, 0x13, U8, DROP, U8], 4),
        workload('copy_u8', 0x14, [...pushU8, 0x14, U8, DROP, U8, DROP, U8], 4),
        workload('swap_u8', 0x15, [...pushU8, ...pushU8, 0x15, U8, U8, DROP, U8, DROP, U8], 5, FLAGS.STACK),
        workload('drop_u8', DROP, [...pushU8, DROP, U8], 2),
        workload('clear', 0x17, [...pushU8, 0x17], 2),
        workload('pick_u8', 0x1c, [...pushU8, 0x1c, U8, 0, 0, DROP, U8, DROP, U8], 4, FLAGS.STACK),
        workload('poke_u8', 0x1d, [...pushU8, ...pushU8, 0x1d, U8, 0, 0, DROP, U8, DROP, U8], 5, FLAGS.STACK),
        workload('mem_fill', 0x1e, [0x1e, 0x55, ...ptr, 4, 0], 1),
        workload('cvt_u8_u16', 0x10, [...pushU8, 0x10, U8, 0x04, DROP, 0x04], 3, FLAGS.CVT),
        workload('branch_save', 0x90, [...pushU8, 0x90, 0x92], 3),
        workload('branch_read', 0x91, [0x91, DROP, U8], 2),
        workload('branch_drop', 0x92, [0x92], 1),
        workload('branch_clear', 0x93, [0x93], 1),
        workload('ffi_memory', 0xf0, [0xf0, 0, 0, ...ptr], 1, FLAGS.FFI),
        workload('ffi_stack', 0xf1, [0xf1, 0, 0, DROP, U8], 2, FLAGS.FFI),
        workload('comms_disabled_status', 0xf9, [0xf9, 0x03, 0, DROP, U8], 2),
        workload('config_db_empty', 0xfb, [0xfb, 0], 1),
        workload('config_tc', 0xfc, [0xfc, 0, 0, 0, 0, 0, 0], 1),
        workload('lang', 0xfd, [0xfd, 1], 1),
        workload('comment', 0xfe, [0xfe, 2, 0x6f, 0x6b], 1),
    )
    return cases
}