// test_ir.js - Test the IR (Intermediate Representation) export
import VovkPLC from '../../dist/VovkPLC.js'
import path from 'path'
import fs from 'fs'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

const run = async () => {
    const wasmPath = path.resolve(__dirname, '../../dist/VovkPLC.wasm')
    console.log(`Loading WASM from: ${wasmPath}`)

    if (!fs.existsSync(wasmPath)) {
        console.error('WASM file not found!')
        process.exit(1)
    }

    // Test with direct instance (not worker)
    const runtime = new VovkPLC(wasmPath)
    await runtime.initialize(wasmPath, false, true) // silent mode

    console.log('='.repeat(60))
    console.log('Testing IR (Intermediate Representation) Export')
    console.log('='.repeat(60))

    // Check the IR_Entry size from WASM
    const irEntrySize = runtime.wasm_exports.ir_get_entry_size()
    console.log(`IR_Entry struct size (from WASM): ${irEntrySize} bytes`)

    // Set runtime offsets for proper address mapping
    runtime.wasm_exports.setRuntimeOffsets(0, 64, 128, 192, 448)

    // Test assembly with various instruction types
    const assembly = `
        // Constants
        const DELAY 100
        
        // Read input bit
        u8.readBit X0.0
        
        // Timer with constant parameter
        ton S0, #50
        
        // Write output bit
        u8.writeBit Y0.0
        
        // Push constant value
        u32.const 12345
        
        // Conditional jump
        jmp_if skip_section
        
        // Some arithmetic
        u8.const 10
        u8.const 20
        u8.add
        
    skip_section:
        // End
        exit
    `

    // Compile the assembly
    const downloadError = runtime.downloadAssembly(assembly)
    if (downloadError) {
        console.error('Failed to download assembly')
        process.exit(1)
    }

    const compileError = runtime.wasm_exports.compileAssembly(false)
    if (compileError) {
        console.error('Compilation failed')
        process.exit(1)
    }

    console.log('\nAssembly compiled successfully!')
    console.log(`Labels count: ${runtime.getLabelsCount()}`)
    console.log(`Constants count: ${runtime.getConstsCount()}`)

    // Get IR
    const ir = runtime.getIR()
    console.log(`\nIR entries: ${ir.length}`)
    console.log('-'.repeat(60))

    // Display IR entries
    const IR_FLAGS = VovkPLC.IR_FLAGS
    const IR_OP_TYPES = VovkPLC.IR_OP_TYPES

    const flagNames = (flags) => {
        const names = []
        if (flags & IR_FLAGS.READ) names.push('READ')
        if (flags & IR_FLAGS.WRITE) names.push('WRITE')
        if (flags & IR_FLAGS.CONST) names.push('CONST')
        if (flags & IR_FLAGS.JUMP) names.push('JUMP')
        if (flags & IR_FLAGS.TIMER) names.push('TIMER')
        if (flags & IR_FLAGS.LABEL_TARGET) names.push('LABEL')
        if (flags & IR_FLAGS.EDITABLE) names.push('EDIT')
        return names.length > 0 ? names.join('|') : 'NONE'
    }

    const opTypeName = (type) => {
        const names = Object.entries(IR_OP_TYPES).find(([_, v]) => v === type)
        return names ? names[0] : `UNKNOWN(${type})`
    }

    for (const entry of ir) {
        console.log(`[${entry.bytecode_offset.toString().padStart(3)}] Opcode 0x${entry.opcode.toString(16).padStart(2, '0')} | Size: ${entry.bytecode_size} | Line: ${entry.source_line}:${entry.source_column} | Flags: ${flagNames(entry.flags)}`)
        
        if (entry.operand_count > 0) {
            for (let i = 0; i < entry.operand_count; i++) {
                const op = entry.operands[i]
                console.log(`       Operand ${i}: ${opTypeName(op.type)} = ${op.value} @ byte ${op.bytecode_pos}`)
            }
        }
    }

    // Summary statistics
    console.log('\n' + '='.repeat(60))
    console.log('IR Summary:')
    console.log('='.repeat(60))
    
    const timerEntries = ir.filter(e => e.flags & IR_FLAGS.TIMER)
    const jumpEntries = ir.filter(e => e.flags & IR_FLAGS.JUMP)
    const constEntries = ir.filter(e => e.flags & IR_FLAGS.CONST)
    const editableEntries = ir.filter(e => e.flags & IR_FLAGS.EDITABLE)
    
    console.log(`Total instructions: ${ir.length}`)
    console.log(`Timer instructions: ${timerEntries.length}`)
    console.log(`Jump instructions: ${jumpEntries.length}`)
    console.log(`Const instructions: ${constEntries.length}`)
    console.log(`Editable entries: ${editableEntries.length}`)

    if (editableEntries.length > 0) {
        console.log('\nEditable constants (for live editing):')
        for (const entry of editableEntries) {
            for (const op of entry.operands) {
                if (op.type !== IR_OP_TYPES.NONE) {
                    console.log(`  - Bytecode offset ${entry.bytecode_offset + op.bytecode_pos}: ${opTypeName(op.type)} = ${op.value}`)
                }
            }
        }
    }

    console.log('\n✓ IR test completed successfully!')
    process.exit(0)
}

run().catch(e => {
    console.error('Test failed:', e)
    process.exit(1)
})
