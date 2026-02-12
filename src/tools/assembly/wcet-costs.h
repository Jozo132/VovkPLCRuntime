// wcet-costs.h - WCET static CPU cycle cost table per opcode
//
// Copyright (c) 2026 J.Vovk
//
// This file is part of VovkPLCRuntime.
//
// VovkPLCRuntime is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// VovkPLCRuntime is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with VovkPLCRuntime.  If not, see <https://www.gnu.org/licenses/>.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#ifdef __WASM__

// ============================================================================
// Static CPU cycle cost estimates per opcode
// ============================================================================
// These are abstract "cycle units" (CU) representing relative instruction cost.
// The actual wall-clock time depends on the target hardware, but the ratios
// are calibrated against a typical 32-bit MCU (STM32F4 / ESP32 class).
//
// Cost model:
//   1 CU ≈ 1 register move or stack push/pop
//   2 CU ≈ arithmetic (add/sub/logic)
//   3-4 CU ≈ multiply, compare
//   5-8 CU ≈ divide, modulo
//   10-15 CU ≈ memory access with bounds check
//   20-50 CU ≈ transcendental math (sin/cos/sqrt/pow)
//   50-100 CU ≈ string operations, FFI calls
//
// Each opcode has a BCET (best case) and WCET (worst case) cost.
// The difference comes from:
//   - Branch prediction (conditional jumps: 1 CU if not taken, 3 CU if taken)
//   - Memory access patterns (cache hit vs miss on embedded)
//   - Variable-length string operations
//   - FFI overhead

struct OpcodeCost {
    u8 bcet;    // Best-case execution time in cycle units
    u8 wcet;    // Worst-case execution time in cycle units
};

// Stack effect: how many bytes pushed/popped per instruction
// Positive = net push, negative = net pop, 0 = neutral
struct OpcodeStackEffect {
    i8 pop_bytes;   // Bytes consumed from stack (>= 0)
    i8 push_bytes;  // Bytes produced to stack (>= 0)
};

// Get the CPU cycle cost for an opcode
inline OpcodeCost wcet_opcode_cost(u8 opcode) {
    switch (opcode) {
        // NOP
        case NOP:               return { 1, 1 };

        // Data push instructions (fetch from bytecode + push to stack)
        case type_bool:         return { 2, 2 };
        case type_u8:           return { 2, 2 };
        case type_i8:           return { 2, 2 };
        case type_u16:          return { 2, 3 };
        case type_i16:          return { 2, 3 };
        case type_u32:          return { 3, 4 };
        case type_i32:          return { 3, 4 };
        case type_f32:          return { 3, 4 };
        case type_u64:          return { 4, 5 };
        case type_i64:          return { 4, 5 };
        case type_f64:          return { 4, 5 };
        case type_char:         return { 2, 2 };
        case type_pointer:      return { 2, 3 };

        // Type conversion
        case CVT:               return { 3, 8 }; // depends on src/dest type pair

        // Memory load/store (bounds-checked)
        case LOAD:              return { 8, 12 };
        case MOVE:              return { 8, 12 };
        case MOVE_COPY:         return { 9, 13 };
        case LOAD_FROM:         return { 6, 10 };
        case MOVE_TO:           return { 6, 10 };
        case INC_MEM:           return { 8, 12 };
        case DEC_MEM:           return { 8, 12 };
        case MEM_FILL:          return { 10, 80 }; // variable length

        // Stack operations
        case COPY:              return { 2, 4 };
        case SWAP:              return { 3, 5 };
        case DROP:              return { 1, 2 };
        case CLEAR:             return { 1, 3 };
        case PICK:              return { 4, 8 };
        case POKE:              return { 4, 8 };

        // Integer arithmetic (type-dependent, using worst case for larger types)
        case ADD:               return { 2, 4 };
        case SUB:               return { 2, 4 };
        case MUL:               return { 3, 6 };
        case DIV:               return { 5, 10 };
        case MOD:               return { 5, 10 };
        case NEG:               return { 2, 3 };
        case ABS:               return { 2, 3 };

        // Advanced math
        case POW:               return { 15, 40 };
        case SQRT:              return { 10, 30 };
        case SIN:               return { 15, 40 };
        case COS:               return { 15, 40 };

        // Timer operations (involve memory read/write + time comparison)
        case TON_CONST:
        case TOF_CONST:
        case TP_CONST:          return { 10, 20 };
        case TON_MEM:
        case TOF_MEM:
        case TP_MEM:            return { 12, 22 };

        // Counter operations
        case CTU_CONST:
        case CTD_CONST:         return { 10, 18 };
        case CTU_MEM:
        case CTD_MEM:           return { 12, 20 };

        // Bit get/set/reset (single byte, no address)
        case GET_X8_B0: case GET_X8_B1: case GET_X8_B2: case GET_X8_B3:
        case GET_X8_B4: case GET_X8_B5: case GET_X8_B6: case GET_X8_B7:
        case SET_X8_B0: case SET_X8_B1: case SET_X8_B2: case SET_X8_B3:
        case SET_X8_B4: case SET_X8_B5: case SET_X8_B6: case SET_X8_B7:
        case RSET_X8_B0: case RSET_X8_B1: case RSET_X8_B2: case RSET_X8_B3:
        case RSET_X8_B4: case RSET_X8_B5: case RSET_X8_B6: case RSET_X8_B7:
            return { 2, 3 };

        // Bit read/write with address (memory access)
        case READ_X8_B0: case READ_X8_B1: case READ_X8_B2: case READ_X8_B3:
        case READ_X8_B4: case READ_X8_B5: case READ_X8_B6: case READ_X8_B7:
        case WRITE_X8_B0: case WRITE_X8_B1: case WRITE_X8_B2: case WRITE_X8_B3:
        case WRITE_X8_B4: case WRITE_X8_B5: case WRITE_X8_B6: case WRITE_X8_B7:
        case WRITE_S_X8_B0: case WRITE_S_X8_B1: case WRITE_S_X8_B2: case WRITE_S_X8_B3:
        case WRITE_S_X8_B4: case WRITE_S_X8_B5: case WRITE_S_X8_B6: case WRITE_S_X8_B7:
        case WRITE_R_X8_B0: case WRITE_R_X8_B1: case WRITE_R_X8_B2: case WRITE_R_X8_B3:
        case WRITE_R_X8_B4: case WRITE_R_X8_B5: case WRITE_R_X8_B6: case WRITE_R_X8_B7:
        case WRITE_INV_X8_B0: case WRITE_INV_X8_B1: case WRITE_INV_X8_B2: case WRITE_INV_X8_B3:
        case WRITE_INV_X8_B4: case WRITE_INV_X8_B5: case WRITE_INV_X8_B6: case WRITE_INV_X8_B7:
            return { 5, 8 };

        // Edge detection (memory + state comparison)
        case READ_BIT_DU: case READ_BIT_DD:
        case READ_BIT_INV_DU: case READ_BIT_INV_DD:
        case WRITE_BIT_DU: case WRITE_BIT_DD:
        case WRITE_BIT_INV_DU: case WRITE_BIT_INV_DD:
        case WRITE_SET_DU: case WRITE_SET_DD:
        case WRITE_RSET_DU: case WRITE_RSET_DD:
            return { 8, 14 };

        case STACK_DU:
        case STACK_DD:
        case STACK_DC:          return { 6, 10 };

        // Branch stack operations
        case BR_SAVE:           return { 3, 4 };
        case BR_READ:           return { 2, 3 };
        case BR_DROP:           return { 1, 2 };
        case BR_CLR:            return { 1, 1 };

        // Bitwise operations
        case BW_AND_X8: case BW_OR_X8: case BW_XOR_X8: case BW_NOT_X8:
        case BW_LSHIFT_X8: case BW_RSHIFT_X8:
            return { 2, 3 };
        case BW_AND_X16: case BW_OR_X16: case BW_XOR_X16: case BW_NOT_X16:
        case BW_LSHIFT_X16: case BW_RSHIFT_X16:
            return { 2, 3 };
        case BW_AND_X32: case BW_OR_X32: case BW_XOR_X32: case BW_NOT_X32:
        case BW_LSHIFT_X32: case BW_RSHIFT_X32:
            return { 2, 4 };
        case BW_AND_X64: case BW_OR_X64: case BW_XOR_X64: case BW_NOT_X64:
        case BW_LSHIFT_X64: case BW_RSHIFT_X64:
            return { 3, 5 };

        // Logical operations
        case LOGIC_AND: case LOGIC_OR: case LOGIC_XOR: case LOGIC_NOT:
            return { 1, 2 };

        // Comparison operations
        case CMP_EQ: case CMP_NEQ:
        case CMP_GT: case CMP_LT:
        case CMP_GTE: case CMP_LTE:
            return { 2, 5 };

        // Control flow
        case JMP:               return { 2, 3 };
        case JMP_IF:            return { 2, 4 };  // BCET = not taken (skip), WCET = taken (jump)
        case JMP_IF_NOT:        return { 2, 4 };
        case CALL:              return { 4, 6 };   // push return addr + jump
        case CALL_IF:           return { 2, 7 };
        case CALL_IF_NOT:       return { 2, 7 };
        case RET:               return { 3, 5 };
        case RET_IF:            return { 2, 6 };
        case RET_IF_NOT:        return { 2, 6 };
        case JMP_REL:           return { 2, 3 };
        case JMP_IF_REL:        return { 2, 4 };
        case JMP_IF_NOT_REL:    return { 2, 4 };
        case CALL_REL:          return { 4, 6 };
        case CALL_IF_REL:       return { 2, 7 };
        case CALL_IF_NOT_REL:   return { 2, 7 };

        // FFI (highly variable — depends on the registered function)
        case FFI_CALL:          return { 30, 100 };
        case FFI_CALL_STACK:    return { 30, 100 };

        // String operations (variable length, memory intensive)
        case STR_LEN:           return { 5, 10 };
        case STR_CAP:           return { 5, 10 };
        case STR_GET:           return { 6, 12 };
        case STR_SET:           return { 6, 12 };
        case STR_CLEAR:         return { 5, 10 };
        case STR_CMP:           return { 10, 80 }; // linear scan
        case STR_EQ:            return { 10, 80 };
        case STR_CONCAT:        return { 15, 100 };
        case STR_COPY:          return { 10, 80 };
        case STR_SUBSTR:        return { 10, 60 };
        case STR_FIND:          return { 15, 120 }; // O(n*m)
        case STR_CHAR:          return { 5, 10 };
        case STR_TO_NUM:        return { 10, 30 };
        case STR_FROM_NUM:      return { 10, 40 };
        case STR_INIT:          return { 8, 15 };
        case CSTR_LIT:          return { 10, 80 };
        case CSTR_CPY:          return { 10, 80 };
        case CSTR_EQ:           return { 10, 80 };
        case CSTR_CAT:          return { 15, 100 };

        // Configuration (one-time setup, not in hot path normally)
        case CONFIG_DB:         return { 10, 50 };
        case CONFIG_TC:         return { 5, 10 };

        // Metadata (skipped by runtime)
        case LANG:              return { 1, 2 };
        case COMMENT:           return { 1, 3 };

        // Exit
        case EXIT:              return { 1, 2 };

        default:                return { 1, 5 }; // Unknown opcode fallback
    }
}

// Get the stack effect for an opcode (requires the type argument byte for typed ops)
// Returns {pop_bytes, push_bytes}
// For typed operations, type_arg is the data type byte following the opcode
inline OpcodeStackEffect wcet_stack_effect(u8 opcode, u8 type_arg = 0) {
    // Helper: size of a data type
    auto type_size = [](u8 t) -> i8 {
        switch (t) {
            case type_bool: case type_u8: case type_i8: case type_char: return 1;
            case type_u16: case type_i16: return 2;
            case type_u32: case type_i32: case type_f32: return 4;
            case type_u64: case type_i64: case type_f64: return 8;
            default: return 1;
        }
    };
    i8 ts = type_size(type_arg);

    switch (opcode) {
        // Data push: push literal value
        case type_bool: case type_u8: case type_i8: case type_char:
            return { 0, 1 };
        case type_u16: case type_i16:
            return { 0, 2 };
        case type_u32: case type_i32: case type_f32:
            return { 0, 4 };
        case type_u64: case type_i64: case type_f64:
            return { 0, 8 };
        case type_pointer:
            return { 0, 2 }; // u16 pointer

        // CVT: pop source type, push dest type — need both args
        case CVT:
            return { ts, ts }; // Approximation: same size in/out (conservative)

        // Memory load: pop pointer(2), push value
        case LOAD:
            return { 2, ts };
        // Memory store: pop value + pointer(2)
        case MOVE:
            return { (i8)(ts + 2), 0 };
        case MOVE_COPY:
            return { (i8)(ts + 2), ts }; // Pops ptr+value, pushes value back

        // LOAD_FROM: push value (address is immediate, not from stack)
        case LOAD_FROM:
            return { 0, ts };
        // MOVE_TO: pop value (address is immediate)
        case MOVE_TO:
            return { ts, 0 };

        // INC_MEM/DEC_MEM: no stack effect (operates on memory directly)
        case INC_MEM:
        case DEC_MEM:
            return { 0, 0 };

        case MEM_FILL:
            return { 0, 0 }; // All args are immediate

        // Stack manipulation
        case COPY:
            return { 0, ts }; // Duplicates top
        case SWAP:
            return { 0, 0 }; // Swaps top two, net zero
        case DROP:
            return { ts, 0 }; // Removes top
        case CLEAR:
            return { 0, 0 }; // Clears entire stack — special case
        case PICK:
            return { 0, ts }; // Copies from depth to top
        case POKE:
            return { ts, 0 }; // Writes top to depth

        // Binary arithmetic: pop two, push one
        case ADD: case SUB: case MUL: case DIV: case MOD: case POW:
            return { (i8)(ts * 2), ts };

        // Unary arithmetic: pop one, push one
        case SQRT: case NEG: case ABS: case SIN: case COS:
            return { ts, ts };

        // Timer/Counter: pop bool(1), push bool(1)
        case TON_CONST: case TOF_CONST: case TP_CONST:
        case TON_MEM: case TOF_MEM: case TP_MEM:
            return { 1, 1 };
        case CTU_CONST: case CTD_CONST:
        case CTU_MEM: case CTD_MEM:
            return { 2, 1 }; // pop CU/CD(1) + R/LD(1), push Q(1)

        // Bit operations on stack value (no address arg)
        case GET_X8_B0: case GET_X8_B1: case GET_X8_B2: case GET_X8_B3:
        case GET_X8_B4: case GET_X8_B5: case GET_X8_B6: case GET_X8_B7:
            return { 1, 1 }; // pop u8, push bool
        case SET_X8_B0: case SET_X8_B1: case SET_X8_B2: case SET_X8_B3:
        case SET_X8_B4: case SET_X8_B5: case SET_X8_B6: case SET_X8_B7:
        case RSET_X8_B0: case RSET_X8_B1: case RSET_X8_B2: case RSET_X8_B3:
        case RSET_X8_B4: case RSET_X8_B5: case RSET_X8_B6: case RSET_X8_B7:
            return { 1, 1 }; // pop u8, push modified u8

        // Bit read/write with address (address is in bytecode, push/pop bool)
        case READ_X8_B0: case READ_X8_B1: case READ_X8_B2: case READ_X8_B3:
        case READ_X8_B4: case READ_X8_B5: case READ_X8_B6: case READ_X8_B7:
            return { 0, 1 }; // push bool
        case WRITE_X8_B0: case WRITE_X8_B1: case WRITE_X8_B2: case WRITE_X8_B3:
        case WRITE_X8_B4: case WRITE_X8_B5: case WRITE_X8_B6: case WRITE_X8_B7:
            return { 1, 0 }; // pop bool
        case WRITE_S_X8_B0: case WRITE_S_X8_B1: case WRITE_S_X8_B2: case WRITE_S_X8_B3:
        case WRITE_S_X8_B4: case WRITE_S_X8_B5: case WRITE_S_X8_B6: case WRITE_S_X8_B7:
        case WRITE_R_X8_B0: case WRITE_R_X8_B1: case WRITE_R_X8_B2: case WRITE_R_X8_B3:
        case WRITE_R_X8_B4: case WRITE_R_X8_B5: case WRITE_R_X8_B6: case WRITE_R_X8_B7:
        case WRITE_INV_X8_B0: case WRITE_INV_X8_B1: case WRITE_INV_X8_B2: case WRITE_INV_X8_B3:
        case WRITE_INV_X8_B4: case WRITE_INV_X8_B5: case WRITE_INV_X8_B6: case WRITE_INV_X8_B7:
            return { 0, 0 }; // No stack effect (address-based)

        // Edge detection
        case READ_BIT_DU: case READ_BIT_DD:
        case READ_BIT_INV_DU: case READ_BIT_INV_DD:
            return { 0, 1 }; // push bool
        case WRITE_BIT_DU: case WRITE_BIT_DD:
        case WRITE_BIT_INV_DU: case WRITE_BIT_INV_DD:
        case WRITE_SET_DU: case WRITE_SET_DD:
        case WRITE_RSET_DU: case WRITE_RSET_DD:
            return { 1, 0 }; // pop bool

        case STACK_DU: case STACK_DD: case STACK_DC:
            return { 1, 1 }; // pop bool, push bool (edge result)

        // Branch stack
        case BR_SAVE:
            return { 1, 0 }; // pop u8 bool into BR register
        case BR_READ:
            return { 0, 1 }; // push u8 bool from BR register
        case BR_DROP:
        case BR_CLR:
            return { 0, 0 }; // No stack effect

        // Bitwise binary: pop two, push one (same size)
        case BW_AND_X8: case BW_OR_X8: case BW_XOR_X8:
        case BW_LSHIFT_X8: case BW_RSHIFT_X8:
            return { 2, 1 };
        case BW_AND_X16: case BW_OR_X16: case BW_XOR_X16:
        case BW_LSHIFT_X16: case BW_RSHIFT_X16:
            return { 4, 2 };
        case BW_AND_X32: case BW_OR_X32: case BW_XOR_X32:
        case BW_LSHIFT_X32: case BW_RSHIFT_X32:
            return { 8, 4 };
        case BW_AND_X64: case BW_OR_X64: case BW_XOR_X64:
        case BW_LSHIFT_X64: case BW_RSHIFT_X64:
            return { 16, 8 };
        // Bitwise unary: pop one, push one
        case BW_NOT_X8:     return { 1, 1 };
        case BW_NOT_X16:    return { 2, 2 };
        case BW_NOT_X32:    return { 4, 4 };
        case BW_NOT_X64:    return { 8, 8 };

        // Logical: pop 1+1, push 1
        case LOGIC_AND: case LOGIC_OR: case LOGIC_XOR:
            return { 2, 1 };
        case LOGIC_NOT:
            return { 1, 1 };

        // Comparison: pop two typed values, push bool(1)
        case CMP_EQ: case CMP_NEQ:
        case CMP_GT: case CMP_LT:
        case CMP_GTE: case CMP_LTE:
            return { (i8)(ts * 2), 1 };

        // Control flow
        case JMP:               return { 0, 0 };
        case JMP_IF:            return { 1, 0 }; // pop bool
        case JMP_IF_NOT:        return { 1, 0 };
        case CALL:              return { 0, 0 }; // call stack push (not data stack)
        case CALL_IF:           return { 1, 0 };
        case CALL_IF_NOT:       return { 1, 0 };
        case RET:               return { 0, 0 };
        case RET_IF:            return { 1, 0 };
        case RET_IF_NOT:        return { 1, 0 };
        case JMP_REL:           return { 0, 0 };
        case JMP_IF_REL:        return { 1, 0 };
        case JMP_IF_NOT_REL:    return { 1, 0 };
        case CALL_REL:          return { 0, 0 };
        case CALL_IF_REL:       return { 1, 0 };
        case CALL_IF_NOT_REL:   return { 1, 0 };

        // FFI
        case FFI_CALL:          return { 0, 0 };   // Uses memory addresses, not stack
        case FFI_CALL_STACK:    return { 0, 0 };   // Variable — depends on param count (conservative: 0)

        // String ops — most use memory addresses from bytecode, minimal stack
        case STR_LEN:           return { 0, 2 };   // push u16 length
        case STR_CAP:           return { 0, 2 };   // push u16 capacity
        case STR_GET:           return { 2, 1 };   // pop u16 index, push u8 char
        case STR_SET:           return { 3, 0 };   // pop u8 char + u16 index
        case STR_CLEAR:         return { 0, 0 };
        case STR_CMP:           return { 0, 1 };   // push i8 result
        case STR_EQ:            return { 0, 1 };   // push bool
        case STR_CONCAT:        return { 0, 0 };
        case STR_COPY:          return { 0, 0 };
        case STR_SUBSTR:        return { 4, 0 };   // pop u16 len + u16 start
        case STR_FIND:          return { 0, 2 };   // push i16 index
        case STR_CHAR:          return { 1, 0 };   // pop u8 char
        case STR_TO_NUM:        return { 0, ts };   // push number of num_type
        case STR_FROM_NUM:      return { ts, 0 };   // pop number of num_type
        case STR_INIT:          return { 2, 0 };    // pop u16 capacity
        case CSTR_LIT:          return { 0, 0 };
        case CSTR_CPY:          return { 0, 0 };
        case CSTR_EQ:           return { 0, 1 };   // push bool
        case CSTR_CAT:          return { 0, 0 };

        // Config/metadata — no stack effect
        case CONFIG_DB:         return { 0, 0 };
        case CONFIG_TC:         return { 0, 0 };
        case LANG:              return { 0, 0 };
        case COMMENT:           return { 0, 0 };

        case EXIT:              return { 0, 0 };

        default:                return { 0, 0 };
    }
}

#endif // __WASM__
