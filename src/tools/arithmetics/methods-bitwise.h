// methods-bitwise.h - 2022-12-11
//
// Copyright (c) 2022 J.Vovk
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

RuntimeError MANIPULATE_GET_X8_MACRO(RuntimeStack& stack, u8 bit_index) { u8 a = stack.pop_u8(); stack.push_u8((a >> bit_index) & 1); return STATUS_SUCCESS; }
RuntimeError MANIPULATE_SET_X8_MACRO(RuntimeStack& stack, u8 bit_index) { u8 a = stack.pop_u8(); stack.push_u8(a | 1 << bit_index); return STATUS_SUCCESS; }
RuntimeError MANIPULATE_RSET_X8_MACRO(RuntimeStack& stack, u8 bit_index) { u8 a = stack.pop_u8(); stack.push_u8(a & ~(1 << bit_index)); return STATUS_SUCCESS; }

#if MY_PTR_SIZE_BYTES == 2
#define READ_ADDRESS_FROM_MEMORY \
    IGNORE_UNUSED u32 index_start = index; \
    u32 size = 2; \
    if (index + size > prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start); \
    u8A_to_u16 cvt; \
    cvt.u8A[1] = program[index]; \
    cvt.u8A[0] = program[index + 1]; \
    MY_PTR_t address = cvt._u16;
#elif MY_PTR_SIZE_BYTES == 4
#define READ_ADDRESS_FROM_MEMORY \
    IGNORE_UNUSED u32 index_start = index; \
    u32 size = 4; \
    if (index + size > prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start); \
    u8A_to_u32 cvt; \
    cvt.u8A[3] = program[index]; \
    cvt.u8A[2] = program[index + 1]; \
    cvt.u8A[1] = program[index + 2]; \
    cvt.u8A[0] = program[index + 3]; \
    MY_PTR_t address = cvt._u32;
#else // MY_PTR_SIZE_BYTES == 8
#define READ_ADDRESS_FROM_MEMORY \
    IGNORE_UNUSED u32 index_start = index; \
    u32 size = 8; \
    if (index + size > prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start); \
    u8A_to_u64 cvt; \
    cvt.u8A[7] = program[index]; \
    cvt.u8A[6] = program[index + 1]; \
    cvt.u8A[5] = program[index + 2]; \
    cvt.u8A[4] = program[index + 3]; \
    cvt.u8A[3] = program[index + 4]; \
    cvt.u8A[2] = program[index + 5]; \
    cvt.u8A[1] = program[index + 6]; \
    cvt.u8A[0] = program[index + 7]; \
    MY_PTR_t address = cvt._u64;
#endif


RuntimeError READ_X8_MACRO(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index, u8 bit_index) {
    READ_ADDRESS_FROM_MEMORY;
    u8 x = 0;
    bool error = get_u8(memory, address, x);
    if (error) return INVALID_MEMORY_ADDRESS;
    x = (x >> bit_index) & 1;
    stack.push_u8(x);
    index += size;
    if (index >= prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
    return STATUS_SUCCESS;
}

RuntimeError WRITE_X8_MACRO(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index, u8 bit_index) {
    READ_ADDRESS_FROM_MEMORY;
    u8 x = 0;
    bool error = get_u8(memory, address, x);
    if (error) return INVALID_MEMORY_ADDRESS;
    u8 bit = stack.pop_u8();
    x = bit ? x | 1 << bit_index : x & ~(1 << bit_index);
    error = set_u8(memory, address, x);
    if (error) return INVALID_MEMORY_ADDRESS;
    index += size;
    if (index >= prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
    return STATUS_SUCCESS;
}

RuntimeError WRITE_S_X8_MACRO(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index, u8 bit_index) {
    READ_ADDRESS_FROM_MEMORY;
    u8 x = 0;
    bool error = get_u8(memory, address, x);
    if (error) return INVALID_MEMORY_ADDRESS;
    x = x | 1 << bit_index;
    error = set_u8(memory, address, x);
    if (error) return INVALID_MEMORY_ADDRESS;
    index += size;
    if (index >= prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
    return STATUS_SUCCESS;
}

RuntimeError WRITE_R_X8_MACRO(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index, u8 bit_index) {
    READ_ADDRESS_FROM_MEMORY;
    u8 x = 0;
    bool error = get_u8(memory, address, x);
    if (error) return INVALID_MEMORY_ADDRESS;
    x = x & ~(1 << bit_index);
    error = set_u8(memory, address, x);
    if (error) return INVALID_MEMORY_ADDRESS;
    index += size;
    if (index >= prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
    return STATUS_SUCCESS;
}

RuntimeError WRITE_INV_X8_MACRO(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index, u8 bit_index) {
    READ_ADDRESS_FROM_MEMORY;
    u8 x = 0;
    bool error = get_u8(memory, address, x);
    if (error) return INVALID_MEMORY_ADDRESS;
    bool state = (x >> bit_index) & 1;
    x = state ? x & ~(1 << bit_index) : x | 1 << bit_index;
    error = set_u8(memory, address, x);
    if (error) return INVALID_MEMORY_ADDRESS;
    index += size;
    if (index >= prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
    return STATUS_SUCCESS;
}
namespace PLCMethods {

    RuntimeError handle_GET_X8_B0(RuntimeStack& stack) { return MANIPULATE_GET_X8_MACRO(stack, 0); }
    RuntimeError handle_GET_X8_B1(RuntimeStack& stack) { return MANIPULATE_GET_X8_MACRO(stack, 1); }
    RuntimeError handle_GET_X8_B2(RuntimeStack& stack) { return MANIPULATE_GET_X8_MACRO(stack, 2); }
    RuntimeError handle_GET_X8_B3(RuntimeStack& stack) { return MANIPULATE_GET_X8_MACRO(stack, 3); }
    RuntimeError handle_GET_X8_B4(RuntimeStack& stack) { return MANIPULATE_GET_X8_MACRO(stack, 4); }
    RuntimeError handle_GET_X8_B5(RuntimeStack& stack) { return MANIPULATE_GET_X8_MACRO(stack, 5); }
    RuntimeError handle_GET_X8_B6(RuntimeStack& stack) { return MANIPULATE_GET_X8_MACRO(stack, 6); }
    RuntimeError handle_GET_X8_B7(RuntimeStack& stack) { return MANIPULATE_GET_X8_MACRO(stack, 7); }

    RuntimeError handle_SET_X8_B0(RuntimeStack& stack) { return MANIPULATE_SET_X8_MACRO(stack, 0); }
    RuntimeError handle_SET_X8_B1(RuntimeStack& stack) { return MANIPULATE_SET_X8_MACRO(stack, 1); }
    RuntimeError handle_SET_X8_B2(RuntimeStack& stack) { return MANIPULATE_SET_X8_MACRO(stack, 2); }
    RuntimeError handle_SET_X8_B3(RuntimeStack& stack) { return MANIPULATE_SET_X8_MACRO(stack, 3); }
    RuntimeError handle_SET_X8_B4(RuntimeStack& stack) { return MANIPULATE_SET_X8_MACRO(stack, 4); }
    RuntimeError handle_SET_X8_B5(RuntimeStack& stack) { return MANIPULATE_SET_X8_MACRO(stack, 5); }
    RuntimeError handle_SET_X8_B6(RuntimeStack& stack) { return MANIPULATE_SET_X8_MACRO(stack, 6); }
    RuntimeError handle_SET_X8_B7(RuntimeStack& stack) { return MANIPULATE_SET_X8_MACRO(stack, 7); }

    RuntimeError handle_RSET_X8_B0(RuntimeStack& stack) { return MANIPULATE_RSET_X8_MACRO(stack, 0); }
    RuntimeError handle_RSET_X8_B1(RuntimeStack& stack) { return MANIPULATE_RSET_X8_MACRO(stack, 1); }
    RuntimeError handle_RSET_X8_B2(RuntimeStack& stack) { return MANIPULATE_RSET_X8_MACRO(stack, 2); }
    RuntimeError handle_RSET_X8_B3(RuntimeStack& stack) { return MANIPULATE_RSET_X8_MACRO(stack, 3); }
    RuntimeError handle_RSET_X8_B4(RuntimeStack& stack) { return MANIPULATE_RSET_X8_MACRO(stack, 4); }
    RuntimeError handle_RSET_X8_B5(RuntimeStack& stack) { return MANIPULATE_RSET_X8_MACRO(stack, 5); }
    RuntimeError handle_RSET_X8_B6(RuntimeStack& stack) { return MANIPULATE_RSET_X8_MACRO(stack, 6); }
    RuntimeError handle_RSET_X8_B7(RuntimeStack& stack) { return MANIPULATE_RSET_X8_MACRO(stack, 7); }


    RuntimeError handle_READ_X8_B0(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return READ_X8_MACRO(stack, memory, program, prog_size, index, 0); }
    RuntimeError handle_READ_X8_B1(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return READ_X8_MACRO(stack, memory, program, prog_size, index, 1); }
    RuntimeError handle_READ_X8_B2(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return READ_X8_MACRO(stack, memory, program, prog_size, index, 2); }
    RuntimeError handle_READ_X8_B3(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return READ_X8_MACRO(stack, memory, program, prog_size, index, 3); }
    RuntimeError handle_READ_X8_B4(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return READ_X8_MACRO(stack, memory, program, prog_size, index, 4); }
    RuntimeError handle_READ_X8_B5(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return READ_X8_MACRO(stack, memory, program, prog_size, index, 5); }
    RuntimeError handle_READ_X8_B6(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return READ_X8_MACRO(stack, memory, program, prog_size, index, 6); }
    RuntimeError handle_READ_X8_B7(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return READ_X8_MACRO(stack, memory, program, prog_size, index, 7); }

    RuntimeError handle_WRITE_X8_B0(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_X8_MACRO(stack, memory, program, prog_size, index, 0); }
    RuntimeError handle_WRITE_X8_B1(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_X8_MACRO(stack, memory, program, prog_size, index, 1); }
    RuntimeError handle_WRITE_X8_B2(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_X8_MACRO(stack, memory, program, prog_size, index, 2); }
    RuntimeError handle_WRITE_X8_B3(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_X8_MACRO(stack, memory, program, prog_size, index, 3); }
    RuntimeError handle_WRITE_X8_B4(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_X8_MACRO(stack, memory, program, prog_size, index, 4); }
    RuntimeError handle_WRITE_X8_B5(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_X8_MACRO(stack, memory, program, prog_size, index, 5); }
    RuntimeError handle_WRITE_X8_B6(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_X8_MACRO(stack, memory, program, prog_size, index, 6); }
    RuntimeError handle_WRITE_X8_B7(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_X8_MACRO(stack, memory, program, prog_size, index, 7); }

    RuntimeError handle_WRITE_S_X8_B0(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_S_X8_MACRO(stack, memory, program, prog_size, index, 0); }
    RuntimeError handle_WRITE_S_X8_B1(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_S_X8_MACRO(stack, memory, program, prog_size, index, 1); }
    RuntimeError handle_WRITE_S_X8_B2(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_S_X8_MACRO(stack, memory, program, prog_size, index, 2); }
    RuntimeError handle_WRITE_S_X8_B3(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_S_X8_MACRO(stack, memory, program, prog_size, index, 3); }
    RuntimeError handle_WRITE_S_X8_B4(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_S_X8_MACRO(stack, memory, program, prog_size, index, 4); }
    RuntimeError handle_WRITE_S_X8_B5(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_S_X8_MACRO(stack, memory, program, prog_size, index, 5); }
    RuntimeError handle_WRITE_S_X8_B6(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_S_X8_MACRO(stack, memory, program, prog_size, index, 6); }
    RuntimeError handle_WRITE_S_X8_B7(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_S_X8_MACRO(stack, memory, program, prog_size, index, 7); }

    RuntimeError handle_WRITE_R_X8_B0(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_R_X8_MACRO(stack, memory, program, prog_size, index, 0); }
    RuntimeError handle_WRITE_R_X8_B1(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_R_X8_MACRO(stack, memory, program, prog_size, index, 1); }
    RuntimeError handle_WRITE_R_X8_B2(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_R_X8_MACRO(stack, memory, program, prog_size, index, 2); }
    RuntimeError handle_WRITE_R_X8_B3(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_R_X8_MACRO(stack, memory, program, prog_size, index, 3); }
    RuntimeError handle_WRITE_R_X8_B4(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_R_X8_MACRO(stack, memory, program, prog_size, index, 4); }
    RuntimeError handle_WRITE_R_X8_B5(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_R_X8_MACRO(stack, memory, program, prog_size, index, 5); }
    RuntimeError handle_WRITE_R_X8_B6(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_R_X8_MACRO(stack, memory, program, prog_size, index, 6); }
    RuntimeError handle_WRITE_R_X8_B7(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_R_X8_MACRO(stack, memory, program, prog_size, index, 7); }

    RuntimeError handle_WRITE_INV_X8_B0(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_INV_X8_MACRO(stack, memory, program, prog_size, index, 0); }
    RuntimeError handle_WRITE_INV_X8_B1(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_INV_X8_MACRO(stack, memory, program, prog_size, index, 1); }
    RuntimeError handle_WRITE_INV_X8_B2(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_INV_X8_MACRO(stack, memory, program, prog_size, index, 2); }
    RuntimeError handle_WRITE_INV_X8_B3(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_INV_X8_MACRO(stack, memory, program, prog_size, index, 3); }
    RuntimeError handle_WRITE_INV_X8_B4(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_INV_X8_MACRO(stack, memory, program, prog_size, index, 4); }
    RuntimeError handle_WRITE_INV_X8_B5(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_INV_X8_MACRO(stack, memory, program, prog_size, index, 5); }
    RuntimeError handle_WRITE_INV_X8_B6(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_INV_X8_MACRO(stack, memory, program, prog_size, index, 6); }
    RuntimeError handle_WRITE_INV_X8_B7(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return WRITE_INV_X8_MACRO(stack, memory, program, prog_size, index, 7); }

    RuntimeError handle_BW_AND_X8(RuntimeStack& stack) {
        u8 b = stack.pop_u8();
        u8 a = stack.pop_u8();
        stack.push_u8(a & b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_AND_X16(RuntimeStack& stack) {
        u16 b = stack.pop_u16();
        u16 a = stack.pop_u16();
        stack.push_u16(a & b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_AND_X32(RuntimeStack& stack) {
        u32 b = stack.pop_u32();
        u32 a = stack.pop_u32();
        stack.push_u32(a & b);
        return STATUS_SUCCESS;
    }

    RuntimeError handle_BW_OR_X8(RuntimeStack& stack) {
        u8 b = stack.pop_u8();
        u8 a = stack.pop_u8();
        stack.push_u8(a | b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_OR_X16(RuntimeStack& stack) {
        u16 b = stack.pop_u16();
        u16 a = stack.pop_u16();
        stack.push_u16(a | b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_OR_X32(RuntimeStack& stack) {
        u32 b = stack.pop_u32();
        u32 a = stack.pop_u32();
        stack.push_u32(a | b);
        return STATUS_SUCCESS;
    }

    RuntimeError handle_BW_XOR_X8(RuntimeStack& stack) {
        u8 b = stack.pop_u8();
        u8 a = stack.pop_u8();
        stack.push_u8(a ^ b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_XOR_X16(RuntimeStack& stack) {
        u16 b = stack.pop_u16();
        u16 a = stack.pop_u16();
        stack.push_u16(a ^ b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_XOR_X32(RuntimeStack& stack) {
        u32 b = stack.pop_u32();
        u32 a = stack.pop_u32();
        stack.push_u32(a ^ b);
        return STATUS_SUCCESS;
    }

    RuntimeError handle_BW_NOT_X8(RuntimeStack& stack) {
        u8 a = stack.pop_u8();
        stack.push_u8(~a);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_NOT_X16(RuntimeStack& stack) {
        u16 a = stack.pop_u16();
        stack.push_u16(~a);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_NOT_X32(RuntimeStack& stack) {
        u32 a = stack.pop_u32();
        stack.push_u32(~a);
        return STATUS_SUCCESS;
    }

    RuntimeError handle_BW_LSHIFT_X8(RuntimeStack& stack) {
        u8 b = stack.pop_u8();
        u8 a = stack.pop_u8();
        stack.push_u8(a << b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_LSHIFT_X16(RuntimeStack& stack) {
        u16 b = stack.pop_u16();
        u16 a = stack.pop_u16();
        stack.push_u16(a << b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_LSHIFT_X32(RuntimeStack& stack) {
        u32 b = stack.pop_u32();
        u32 a = stack.pop_u32();
        stack.push_u32(a << b);
        return STATUS_SUCCESS;
    }

    RuntimeError handle_BW_RSHIFT_X8(RuntimeStack& stack) {
        u8 b = stack.pop_u8();
        u8 a = stack.pop_u8();
        stack.push_u8(a >> b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_RSHIFT_X16(RuntimeStack& stack) {
        u16 b = stack.pop_u16();
        u16 a = stack.pop_u16();
        stack.push_u16(a >> b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_RSHIFT_X32(RuntimeStack& stack) {
        u32 b = stack.pop_u32();
        u32 a = stack.pop_u32();
        stack.push_u32(a >> b);
        return STATUS_SUCCESS;
    }

#ifdef USE_X64_OPS
    RuntimeError handle_BW_AND_X64(RuntimeStack& stack) {
        u64 b = stack.pop_u64();
        u64 a = stack.pop_u64();
        stack.push_u64(a & b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_OR_X64(RuntimeStack& stack) {
        u64 b = stack.pop_u64();
        u64 a = stack.pop_u64();
        stack.push_u64(a | b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_XOR_X64(RuntimeStack& stack) {
        u64 b = stack.pop_u64();
        u64 a = stack.pop_u64();
        stack.push_u64(a ^ b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_NOT_X64(RuntimeStack& stack) {
        u64 a = stack.pop_u64();
        stack.push_u64(~a);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_LSHIFT_X64(RuntimeStack& stack) {
        u64 b = stack.pop_u64();
        u64 a = stack.pop_u64();
        stack.push_u64(a << b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_RSHIFT_X64(RuntimeStack& stack) {
        u64 b = stack.pop_u64();
        u64 a = stack.pop_u64();
        stack.push_u64(a >> b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS

    // Helper for Edge Detection
    RuntimeError handle_READ_BIT_EDGE(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index, bool rising, bool invert = false) {
        MY_PTR_t input_addr;
        MY_PTR_t state_addr;
        u8 input_bit;
        u8 state_bit;

#if MY_PTR_SIZE_BYTES == 2
        if (index + 7 > prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index);
        u8A_to_u16 cvt;
        
        // Input Addr
        cvt.u8A[1] = program[index]; cvt.u8A[0] = program[index + 1];
        input_addr = cvt._u16;
        index += 2;
        
        // Input Bit
        input_bit = program[index++];
        
        // State Addr
        cvt.u8A[1] = program[index]; cvt.u8A[0] = program[index + 1];
        state_addr = cvt._u16;
        index += 2;
        
        // State Bit
        state_bit = program[index++];
        
#elif MY_PTR_SIZE_BYTES == 4
        if (index + 11 > prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index);
        u8A_to_u32 cvt;
        
        // Input Addr
        cvt.u8A[3] = program[index]; cvt.u8A[2] = program[index + 1];
        cvt.u8A[1] = program[index + 2]; cvt.u8A[0] = program[index + 3];
        input_addr = cvt._u32;
        index += 4;
        
        // Input Bit
        input_bit = program[index++];
        
        // State Addr
        cvt.u8A[3] = program[index]; cvt.u8A[2] = program[index + 1];
        cvt.u8A[1] = program[index + 2]; cvt.u8A[0] = program[index + 3];
        state_addr = cvt._u32;
        index += 4;
        
        // State Bit
        state_bit = program[index++];
#endif

        u8 val_byte = 0;
        
        // Read Input
        if (get_u8(memory, input_addr, val_byte)) return INVALID_MEMORY_ADDRESS;
        bool input_val = (val_byte >> input_bit) & 1;
        if (invert) input_val = !input_val;

        // Read State
        if (get_u8(memory, state_addr, val_byte)) return INVALID_MEMORY_ADDRESS;
        bool state_val = (val_byte >> state_bit) & 1;

        // Calculate Edge
        bool result = rising ? (input_val && !state_val) : (!input_val && state_val);
        
        // Update State if changed
        if (input_val != state_val) {
            if (input_val) val_byte |= (1 << state_bit);
            else val_byte &= ~(1 << state_bit);
            if (set_u8(memory, state_addr, val_byte)) return INVALID_MEMORY_ADDRESS;
        }

        stack.push_u8(result ? 1 : 0);
        return STATUS_SUCCESS;
    }

    RuntimeError handle_READ_BIT_DU(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        return handle_READ_BIT_EDGE(stack, memory, program, prog_size, index, true);
    }
    RuntimeError handle_READ_BIT_DD(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        return handle_READ_BIT_EDGE(stack, memory, program, prog_size, index, false);
    }
    
    RuntimeError handle_READ_BIT_INV_DU(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        return handle_READ_BIT_EDGE(stack, memory, program, prog_size, index, true, true);
    }
    RuntimeError handle_READ_BIT_INV_DD(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        return handle_READ_BIT_EDGE(stack, memory, program, prog_size, index, false, true);
    }

    RuntimeError handle_WRITE_BIT_EDGE(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index, bool rising, bool invert = false, bool set = false, bool reset = false) {
        if (index + 6 > prog_size) return PROGRAM_SIZE_EXCEEDED;

        if (stack.size() < 1) return STACK_UNDERFLOW;
        u8 val_stack_byte = stack.pop_u8();
        bool input_val = (val_stack_byte != 0);
        if (invert) input_val = !input_val;
        
        // Target Address
        u16 target_addr;
        target_addr = program[index++] << 8;
        target_addr |= program[index++];
        u8 target_bit = program[index++];
        
        // State Address
        u16 state_addr;
        state_addr = program[index++] << 8;
        state_addr |= program[index++];
        u8 state_bit = program[index++];

        // Read State
        u8 val_byte_state = 0;
        if (get_u8(memory, state_addr, val_byte_state)) return INVALID_MEMORY_ADDRESS;
        bool state_val = (val_byte_state >> state_bit) & 1;

        // Calculate Edge
        bool edge_detected = rising ? (input_val && !state_val) : (!input_val && state_val);
        
        // Update State
        if (input_val != state_val) {
            if (input_val) val_byte_state |= (1 << state_bit);
            else val_byte_state &= ~(1 << state_bit);
            if (set_u8(memory, state_addr, val_byte_state)) return INVALID_MEMORY_ADDRESS;
        }

        // Write Target
        if (set) {
            if (edge_detected) {
                u8 val_byte_target = 0;
                if (get_u8(memory, target_addr, val_byte_target)) return INVALID_MEMORY_ADDRESS;
                val_byte_target |= (1 << target_bit);
                if (set_u8(memory, target_addr, val_byte_target)) return INVALID_MEMORY_ADDRESS;
            }
        } else if (reset) {
            if (edge_detected) {
                u8 val_byte_target = 0;
                if (get_u8(memory, target_addr, val_byte_target)) return INVALID_MEMORY_ADDRESS;
                val_byte_target &= ~(1 << target_bit);
                if (set_u8(memory, target_addr, val_byte_target)) return INVALID_MEMORY_ADDRESS;
            }
        } else {
            // Normal Pulse Coil (Write 1 if edge, 0 otherwise) or (Write Edge)
            // But usually Pulse Coil writes 1 for one cycle.
            // Since we just detected "edge_detected", we write that.
            u8 val_byte_target = 0;
            if (get_u8(memory, target_addr, val_byte_target)) return INVALID_MEMORY_ADDRESS;
            if (edge_detected) val_byte_target |= (1 << target_bit);
            else val_byte_target &= ~(1 << target_bit);
            if (set_u8(memory, target_addr, val_byte_target)) return INVALID_MEMORY_ADDRESS;
        }

        return STATUS_SUCCESS;
    }

    RuntimeError handle_WRITE_BIT_DU(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return handle_WRITE_BIT_EDGE(stack, memory, program, prog_size, index, true, false); }
    RuntimeError handle_WRITE_BIT_DD(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return handle_WRITE_BIT_EDGE(stack, memory, program, prog_size, index, false, false); }
    RuntimeError handle_WRITE_BIT_INV_DU(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return handle_WRITE_BIT_EDGE(stack, memory, program, prog_size, index, true, true); }
    RuntimeError handle_WRITE_BIT_INV_DD(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return handle_WRITE_BIT_EDGE(stack, memory, program, prog_size, index, false, true); }
    RuntimeError handle_WRITE_SET_DU(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return handle_WRITE_BIT_EDGE(stack, memory, program, prog_size, index, true, false, true); }
    RuntimeError handle_WRITE_SET_DD(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return handle_WRITE_BIT_EDGE(stack, memory, program, prog_size, index, false, false, true); }
    RuntimeError handle_WRITE_RSET_DU(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return handle_WRITE_BIT_EDGE(stack, memory, program, prog_size, index, true, false, false, true); }
    RuntimeError handle_WRITE_RSET_DD(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return handle_WRITE_BIT_EDGE(stack, memory, program, prog_size, index, false, false, false, true); }

    RuntimeError handle_STACK_EDGE(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index, bool rising) {
        if (index + 3 > prog_size) return PROGRAM_SIZE_EXCEEDED;

        if (stack.size() < 1) return STACK_UNDERFLOW;
        u8 val_stack_byte = stack.pop_u8(); // Assumption: user ensured stack has bool-convertible u8 at top. Or we should "peek" and convert but we pop here.
        // User said "all datatypes (value == 0)".
        // Since RuntimeStack doesn't track types on pop well without known type, we might assume u8 or try to pop `any`. 
        // But popping `any` requires knowing size.
        // However, standard boolean ops in this runtime usually work on u8.
        // If a user pushed f32, they should have compared it to get u8 result before DU.
        // But "general diff ... for all datatypes" might mean it handles it.
        // But pop() is just `u8`. `pop_u32` pops 4 bytes.
        // If we don't know the type, we can't pop correctly.
        // BUT, the instruction stream doesn't say the type.
        // So we MUST assume the user has converted it to a boolean on the stack (0/1 u8) OR checks != 0.
        // But if I pop u8, and the user pushed u32 (0x00000001), Little Endian: 01 00 00 00.
        // Pop u8 gives 01. Correct. 
        // But then 00 00 00 remain on stack. That's Bad.
        // So the instruction MUST be typed or assumed u8.
        // Given existing instructions are u8.*, I'll assume u8.
        
        bool input_val = (val_stack_byte != 0); // Implicit u8->bool

        // State Address
        u16 state_addr;
        state_addr = program[index++] << 8;
        state_addr |= program[index++];
        u8 state_bit = program[index++];

        // Read State
        u8 val_byte_state = 0;
        if (get_u8(memory, state_addr, val_byte_state)) return INVALID_MEMORY_ADDRESS;
        bool state_val = (val_byte_state >> state_bit) & 1;

        // Calculate Edge
        bool edge_detected = rising ? (input_val && !state_val) : (!input_val && state_val);
        
        // Update State
        if (input_val != state_val) {
            if (input_val) val_byte_state |= (1 << state_bit);
            else val_byte_state &= ~(1 << state_bit);
            if (set_u8(memory, state_addr, val_byte_state)) return INVALID_MEMORY_ADDRESS;
        }

        stack.push_u8(edge_detected ? 1 : 0);
        return STATUS_SUCCESS;
    }
    
    RuntimeError handle_STACK_DU(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return handle_STACK_EDGE(stack, memory, program, prog_size, index, true); }
    RuntimeError handle_STACK_DD(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) { return handle_STACK_EDGE(stack, memory, program, prog_size, index, false); }

    // Differentiate Change - detects any state change (rising OR falling)
    RuntimeError handle_STACK_DC(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        if (index + 3 > prog_size) return PROGRAM_SIZE_EXCEEDED;

        if (stack.size() < 1) return STACK_UNDERFLOW;
        u8 val_stack_byte = stack.pop_u8();
        bool input_val = (val_stack_byte != 0);

        // State Address
        u16 state_addr;
        state_addr = program[index++] << 8;
        state_addr |= program[index++];
        u8 state_bit = program[index++];

        // Read State
        u8 val_byte_state = 0;
        if (get_u8(memory, state_addr, val_byte_state)) return INVALID_MEMORY_ADDRESS;
        bool state_val = (val_byte_state >> state_bit) & 1;

        // Detect any change (XOR)
        bool edge_detected = (input_val != state_val);
        
        // Update State
        if (edge_detected) {
            if (input_val) val_byte_state |= (1 << state_bit);
            else val_byte_state &= ~(1 << state_bit);
            if (set_u8(memory, state_addr, val_byte_state)) return INVALID_MEMORY_ADDRESS;
        }

        stack.push_u8(edge_detected ? 1 : 0);
        return STATUS_SUCCESS;
    }

}