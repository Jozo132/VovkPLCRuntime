// methods-flow.h - 1.0.0 - 2022-12-11
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
namespace PLCMethods {

    RuntimeError handle_JMP(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        IGNORE_UNUSED uint32_t index_start = index;
        uint32_t size = 2;
        if (index + size > program_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, program_size, index, index_start);
        u8A_to_u16 cvt;
        cvt.u8A[1] = program[index];
        cvt.u8A[0] = program[index + 1];
        index = cvt.u16;
        if (index >= program_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, program_size, index, index_start);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_JMP_IF(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        IGNORE_UNUSED uint32_t index_start = index;
        uint32_t size = 2;
        if (index + size > program_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, program_size, index, index_start);
        if (stack->pop_bool()) {
            u8A_to_u16 cvt;
            cvt.u8A[1] = program[index];
            cvt.u8A[0] = program[index + 1];
            index = cvt.u16;
            if (index >= program_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, program_size, index, index_start);
        } else index += size;
        return STATUS_SUCCESS;
    }
    RuntimeError handle_JMP_IF_NOT(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        IGNORE_UNUSED uint32_t index_start = index;
        uint32_t size = 2;
        if (index + size > program_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, program_size, index, index_start);
        if (!stack->pop_bool()) {
            u8A_to_u16 cvt;
            cvt.u8A[1] = program[index];
            cvt.u8A[0] = program[index + 1];
            index = cvt.u16;
            if (index >= program_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, program_size, index, index_start);
        } else index += size;
        return STATUS_SUCCESS;
    }

    RuntimeError handle_CALL(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        IGNORE_UNUSED uint32_t index_start = index;
        uint32_t size = 2;
        if (index + size > program_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, program_size, index, index_start);
        u8A_to_u16 cvt;
        cvt.u8A[1] = program[index];
        cvt.u8A[0] = program[index + 1];
        RuntimeError call_store_status = stack->pushCall(index + size);
        index = cvt.u16;
        if (index >= program_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, program_size, index, index_start);
        if (call_store_status != STATUS_SUCCESS) return call_store_status;
        if (index >= program_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, program_size, index, index_start);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_CALL_IF(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        IGNORE_UNUSED uint32_t index_start = index;
        uint32_t size = 2;
        if (index + size > program_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, program_size, index, index_start);
        if (stack->pop_bool()) {
            u8A_to_u16 cvt;
            cvt.u8A[1] = program[index];
            cvt.u8A[0] = program[index + 1];
            RuntimeError call_store_status = stack->pushCall(index + size);
            index = cvt.u16;
            if (index == 0 || index >= program_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, program_size, index, index_start);
            if (call_store_status != STATUS_SUCCESS) return call_store_status;
            if (index >= program_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, program_size, index, index_start);
        } else index += size;
        return STATUS_SUCCESS;
    }
    RuntimeError handle_CALL_IF_NOT(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        IGNORE_UNUSED uint32_t index_start = index;
        uint32_t size = 2;
        if (index + size > program_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, program_size, index, index_start);
        if (!stack->pop_bool()) {
            u8A_to_u16 cvt;
            cvt.u8A[1] = program[index];
            cvt.u8A[0] = program[index + 1];
            RuntimeError call_store_status = stack->pushCall(index + size);
            index = cvt.u16;
            if (index == 0 || index >= program_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, program_size, index, index_start);
            if (call_store_status != STATUS_SUCCESS) return call_store_status;
            if (index >= program_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, program_size, index, index_start);
        } else index += size;
        return STATUS_SUCCESS;
    }
    RuntimeError handle_RET(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        IGNORE_UNUSED uint32_t index_start = index;
        if (stack->call_stack->size() == 0) return CALL_STACK_UNDERFLOW;
        uint16_t ret_index = stack->popCall();
        index = ret_index;
        if (index >= program_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, program_size, index, index_start);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_RET_IF(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        IGNORE_UNUSED uint32_t index_start = index;
        if (stack->call_stack->size() == 0) return CALL_STACK_UNDERFLOW;
        if (stack->pop_bool()) {
            uint16_t ret_index = stack->popCall();
            index = ret_index;
            if (index >= program_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, program_size, index, index_start);
        }
        return STATUS_SUCCESS;
    }
    RuntimeError handle_RET_IF_NOT(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        IGNORE_UNUSED uint32_t index_start = index;
        if (stack->call_stack->size() == 0) return CALL_STACK_UNDERFLOW;
        if (!stack->pop_bool()) {
            uint16_t ret_index = stack->popCall();
            index = ret_index;
            if (index >= program_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, program_size, index, index_start);
        }
        return STATUS_SUCCESS;
    }


    RuntimeError handle_EXIT(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        if (index >= program_size) return STATUS_SUCCESS;
        uint8_t exit_code = program[index++];
        return (RuntimeError) exit_code;
    }

}