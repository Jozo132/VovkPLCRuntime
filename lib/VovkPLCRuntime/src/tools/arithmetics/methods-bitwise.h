// methods-bitwise.h - 1.0.0 - 2022-12-11
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

RuntimeError MANIPULATE_GET_X8_MACRO(RuntimeStack* stack, uint8_t bit_index) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t((a >> bit_index) & 1); return STATUS_SUCCESS; }
RuntimeError MANIPULATE_SET_X8_MACRO(RuntimeStack* stack, uint8_t bit_index) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a | 1 << bit_index); return STATUS_SUCCESS; }
RuntimeError MANIPULATE_RSET_X8_MACRO(RuntimeStack* stack, uint8_t bit_index) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a & ~(1 << bit_index)); return STATUS_SUCCESS; }

RuntimeError READ_X8_MACRO(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index, uint8_t bit_index) {
    IGNORE_UNUSED uint32_t index_start = index;
    uint32_t size = 2;
    if (index + size > program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
    u8A_to_u16 cvt;
    cvt.u8A[1] = program[index];
    cvt.u8A[0] = program[index + 1];
    uint16_t address = cvt.u16;
    uint8_t x = 0;
    bool error = stack->memory->get(address, x);
    if (error) return INVALID_MEMORY_ADDRESS;
    x = (x >> bit_index) & 1;
    stack->push_uint8_t(x);
    index += size;
    if (index >= program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
    return STATUS_SUCCESS;
}

RuntimeError WRITE_X8_MACRO(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index, uint8_t bit_index) {
    IGNORE_UNUSED uint32_t index_start = index;
    uint32_t size = 2;
    if (index + size > program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
    u8A_to_u16 cvt;
    cvt.u8A[1] = program[index];
    cvt.u8A[0] = program[index + 1];
    uint16_t address = cvt.u16;
    uint8_t x = 0;
    bool error = stack->memory->get(address, x);
    if (error) return INVALID_MEMORY_ADDRESS;
    uint8_t bit = stack->pop_uint8_t();
    x = bit ? x | 1 << bit_index : x & ~(1 << bit_index);
    error = stack->memory->set(address, x);
    if (error) return INVALID_MEMORY_ADDRESS;
    index += size;
    if (index >= program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
    return STATUS_SUCCESS;
}

RuntimeError WRITE_S_X8_MACRO(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index, uint8_t bit_index) {
    IGNORE_UNUSED uint32_t index_start = index;
    uint32_t size = 2;
    if (index + size > program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
    u8A_to_u16 cvt;
    cvt.u8A[1] = program[index];
    cvt.u8A[0] = program[index + 1];
    uint16_t address = cvt.u16;
    uint8_t x = 0;
    bool error = stack->memory->get(address, x);
    if (error) return INVALID_MEMORY_ADDRESS;
    x = x | 1 << bit_index;
    error = stack->memory->set(address, x);
    if (error) return INVALID_MEMORY_ADDRESS;
    index += size;
    if (index >= program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
    return STATUS_SUCCESS;
}

RuntimeError WRITE_R_X8_MACRO(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index, uint8_t bit_index) {
    IGNORE_UNUSED uint32_t index_start = index;
    uint32_t size = 2;
    if (index + size > program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
    u8A_to_u16 cvt;
    cvt.u8A[1] = program[index];
    cvt.u8A[0] = program[index + 1];
    uint16_t address = cvt.u16;
    uint8_t x = 0;
    bool error = stack->memory->get(address, x);
    if (error) return INVALID_MEMORY_ADDRESS;
    x = x & ~(1 << bit_index);
    error = stack->memory->set(address, x);
    if (error) return INVALID_MEMORY_ADDRESS;
    index += size;
    if (index >= program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
    return STATUS_SUCCESS;
}

RuntimeError WRITE_INV_X8_MACRO(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index, uint8_t bit_index) {
    IGNORE_UNUSED uint32_t index_start = index;
    uint32_t size = 2;
    if (index + size > program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
    u8A_to_u16 cvt;
    cvt.u8A[1] = program[index];
    cvt.u8A[0] = program[index + 1];
    uint16_t address = cvt.u16;
    uint8_t x = 0;
    bool error = stack->memory->get(address, x);
    if (error) return INVALID_MEMORY_ADDRESS;
    bool state = (x >> bit_index) & 1;
    x = state ? x & ~(1 << bit_index) : x | 1 << bit_index;
    error = stack->memory->set(address, x);
    if (error) return INVALID_MEMORY_ADDRESS;
    index += size;
    if (index >= program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
    return STATUS_SUCCESS;
}
namespace PLCMethods {

    RuntimeError handle_GET_X8_B0(RuntimeStack* stack) { return MANIPULATE_GET_X8_MACRO(stack, 0); }
    RuntimeError handle_GET_X8_B1(RuntimeStack* stack) { return MANIPULATE_GET_X8_MACRO(stack, 1); }
    RuntimeError handle_GET_X8_B2(RuntimeStack* stack) { return MANIPULATE_GET_X8_MACRO(stack, 2); }
    RuntimeError handle_GET_X8_B3(RuntimeStack* stack) { return MANIPULATE_GET_X8_MACRO(stack, 3); }
    RuntimeError handle_GET_X8_B4(RuntimeStack* stack) { return MANIPULATE_GET_X8_MACRO(stack, 4); }
    RuntimeError handle_GET_X8_B5(RuntimeStack* stack) { return MANIPULATE_GET_X8_MACRO(stack, 5); }
    RuntimeError handle_GET_X8_B6(RuntimeStack* stack) { return MANIPULATE_GET_X8_MACRO(stack, 6); }
    RuntimeError handle_GET_X8_B7(RuntimeStack* stack) { return MANIPULATE_GET_X8_MACRO(stack, 7); }

    RuntimeError handle_SET_X8_B0(RuntimeStack* stack) { return MANIPULATE_SET_X8_MACRO(stack, 0); }
    RuntimeError handle_SET_X8_B1(RuntimeStack* stack) { return MANIPULATE_SET_X8_MACRO(stack, 1); }
    RuntimeError handle_SET_X8_B2(RuntimeStack* stack) { return MANIPULATE_SET_X8_MACRO(stack, 2); }
    RuntimeError handle_SET_X8_B3(RuntimeStack* stack) { return MANIPULATE_SET_X8_MACRO(stack, 3); }
    RuntimeError handle_SET_X8_B4(RuntimeStack* stack) { return MANIPULATE_SET_X8_MACRO(stack, 4); }
    RuntimeError handle_SET_X8_B5(RuntimeStack* stack) { return MANIPULATE_SET_X8_MACRO(stack, 5); }
    RuntimeError handle_SET_X8_B6(RuntimeStack* stack) { return MANIPULATE_SET_X8_MACRO(stack, 6); }
    RuntimeError handle_SET_X8_B7(RuntimeStack* stack) { return MANIPULATE_SET_X8_MACRO(stack, 7); }

    RuntimeError handle_RSET_X8_B0(RuntimeStack* stack) { return MANIPULATE_RSET_X8_MACRO(stack, 0); }
    RuntimeError handle_RSET_X8_B1(RuntimeStack* stack) { return MANIPULATE_RSET_X8_MACRO(stack, 1); }
    RuntimeError handle_RSET_X8_B2(RuntimeStack* stack) { return MANIPULATE_RSET_X8_MACRO(stack, 2); }
    RuntimeError handle_RSET_X8_B3(RuntimeStack* stack) { return MANIPULATE_RSET_X8_MACRO(stack, 3); }
    RuntimeError handle_RSET_X8_B4(RuntimeStack* stack) { return MANIPULATE_RSET_X8_MACRO(stack, 4); }
    RuntimeError handle_RSET_X8_B5(RuntimeStack* stack) { return MANIPULATE_RSET_X8_MACRO(stack, 5); }
    RuntimeError handle_RSET_X8_B6(RuntimeStack* stack) { return MANIPULATE_RSET_X8_MACRO(stack, 6); }
    RuntimeError handle_RSET_X8_B7(RuntimeStack* stack) { return MANIPULATE_RSET_X8_MACRO(stack, 7); }


    RuntimeError handle_READ_X8_B0(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return READ_X8_MACRO(stack, program, program_size, index, 0); }
    RuntimeError handle_READ_X8_B1(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return READ_X8_MACRO(stack, program, program_size, index, 1); }
    RuntimeError handle_READ_X8_B2(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return READ_X8_MACRO(stack, program, program_size, index, 2); }
    RuntimeError handle_READ_X8_B3(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return READ_X8_MACRO(stack, program, program_size, index, 3); }
    RuntimeError handle_READ_X8_B4(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return READ_X8_MACRO(stack, program, program_size, index, 4); }
    RuntimeError handle_READ_X8_B5(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return READ_X8_MACRO(stack, program, program_size, index, 5); }
    RuntimeError handle_READ_X8_B6(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return READ_X8_MACRO(stack, program, program_size, index, 6); }
    RuntimeError handle_READ_X8_B7(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return READ_X8_MACRO(stack, program, program_size, index, 7); }

    RuntimeError handle_WRITE_X8_B0(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_X8_MACRO(stack, program, program_size, index, 0); }
    RuntimeError handle_WRITE_X8_B1(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_X8_MACRO(stack, program, program_size, index, 1); }
    RuntimeError handle_WRITE_X8_B2(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_X8_MACRO(stack, program, program_size, index, 2); }
    RuntimeError handle_WRITE_X8_B3(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_X8_MACRO(stack, program, program_size, index, 3); }
    RuntimeError handle_WRITE_X8_B4(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_X8_MACRO(stack, program, program_size, index, 4); }
    RuntimeError handle_WRITE_X8_B5(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_X8_MACRO(stack, program, program_size, index, 5); }
    RuntimeError handle_WRITE_X8_B6(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_X8_MACRO(stack, program, program_size, index, 6); }
    RuntimeError handle_WRITE_X8_B7(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_X8_MACRO(stack, program, program_size, index, 7); }

    RuntimeError handle_WRITE_S_X8_B0(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_S_X8_MACRO(stack, program, program_size, index, 0); }
    RuntimeError handle_WRITE_S_X8_B1(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_S_X8_MACRO(stack, program, program_size, index, 1); }
    RuntimeError handle_WRITE_S_X8_B2(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_S_X8_MACRO(stack, program, program_size, index, 2); }
    RuntimeError handle_WRITE_S_X8_B3(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_S_X8_MACRO(stack, program, program_size, index, 3); }
    RuntimeError handle_WRITE_S_X8_B4(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_S_X8_MACRO(stack, program, program_size, index, 4); }
    RuntimeError handle_WRITE_S_X8_B5(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_S_X8_MACRO(stack, program, program_size, index, 5); }
    RuntimeError handle_WRITE_S_X8_B6(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_S_X8_MACRO(stack, program, program_size, index, 6); }
    RuntimeError handle_WRITE_S_X8_B7(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_S_X8_MACRO(stack, program, program_size, index, 7); }

    RuntimeError handle_WRITE_R_X8_B0(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_R_X8_MACRO(stack, program, program_size, index, 0); }
    RuntimeError handle_WRITE_R_X8_B1(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_R_X8_MACRO(stack, program, program_size, index, 1); }
    RuntimeError handle_WRITE_R_X8_B2(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_R_X8_MACRO(stack, program, program_size, index, 2); }
    RuntimeError handle_WRITE_R_X8_B3(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_R_X8_MACRO(stack, program, program_size, index, 3); }
    RuntimeError handle_WRITE_R_X8_B4(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_R_X8_MACRO(stack, program, program_size, index, 4); }
    RuntimeError handle_WRITE_R_X8_B5(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_R_X8_MACRO(stack, program, program_size, index, 5); }
    RuntimeError handle_WRITE_R_X8_B6(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_R_X8_MACRO(stack, program, program_size, index, 6); }
    RuntimeError handle_WRITE_R_X8_B7(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_R_X8_MACRO(stack, program, program_size, index, 7); }

    RuntimeError handle_WRITE_INV_X8_B0(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_INV_X8_MACRO(stack, program, program_size, index, 0); }
    RuntimeError handle_WRITE_INV_X8_B1(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_INV_X8_MACRO(stack, program, program_size, index, 1); }
    RuntimeError handle_WRITE_INV_X8_B2(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_INV_X8_MACRO(stack, program, program_size, index, 2); }
    RuntimeError handle_WRITE_INV_X8_B3(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_INV_X8_MACRO(stack, program, program_size, index, 3); }
    RuntimeError handle_WRITE_INV_X8_B4(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_INV_X8_MACRO(stack, program, program_size, index, 4); }
    RuntimeError handle_WRITE_INV_X8_B5(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_INV_X8_MACRO(stack, program, program_size, index, 5); }
    RuntimeError handle_WRITE_INV_X8_B6(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_INV_X8_MACRO(stack, program, program_size, index, 6); }
    RuntimeError handle_WRITE_INV_X8_B7(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) { return WRITE_INV_X8_MACRO(stack, program, program_size, index, 7); }
    
    RuntimeError handle_BW_AND_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a & b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_AND_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a & b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_AND_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a & b);
        return STATUS_SUCCESS;
    }

    RuntimeError handle_BW_OR_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a | b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_OR_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a | b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_OR_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a | b);
        return STATUS_SUCCESS;
    }

    RuntimeError handle_BW_XOR_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a ^ b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_XOR_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a ^ b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_XOR_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a ^ b);
        return STATUS_SUCCESS;
    }

    RuntimeError handle_BW_NOT_X8(RuntimeStack* stack) {
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(~a);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_NOT_X16(RuntimeStack* stack) {
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(~a);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_NOT_X32(RuntimeStack* stack) {
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(~a);
        return STATUS_SUCCESS;
    }

    RuntimeError handle_BW_LSHIFT_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a << b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_LSHIFT_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a << b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_LSHIFT_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a << b);
        return STATUS_SUCCESS;
    }

    RuntimeError handle_BW_RSHIFT_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a >> b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_RSHIFT_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a >> b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_RSHIFT_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a >> b);
        return STATUS_SUCCESS;
    }

#ifdef USE_X64_OPS
    RuntimeError handle_BW_AND_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a & b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_OR_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a | b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_XOR_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a ^ b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_NOT_X64(RuntimeStack* stack) {
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(~a);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_LSHIFT_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a << b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_RSHIFT_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a >> b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
}