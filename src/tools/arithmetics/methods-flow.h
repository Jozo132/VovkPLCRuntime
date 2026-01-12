// methods-flow.h - 2022-12-11
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

    RuntimeError handle_JMP(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        IGNORE_UNUSED u32 index_start = index;
        u32 size = 2;
        if (index + size > prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        u8A_to_u16 cvt;
        cvt.u8A[1] = program[index];
        cvt.u8A[0] = program[index + 1];
        index = cvt._u16;
        if (index >= prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_JMP_IF(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        IGNORE_UNUSED u32 index_start = index;
        u32 size = 2;
        if (index + size > prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        if (stack.pop_bool()) {
            u8A_to_u16 cvt;
            cvt.u8A[1] = program[index];
            cvt.u8A[0] = program[index + 1];
            index = cvt._u16;
            if (index >= prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        } else index += size;
        return STATUS_SUCCESS;
    }
    RuntimeError handle_JMP_IF_NOT(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        IGNORE_UNUSED u32 index_start = index;
        u32 size = 2;
        if (index + size > prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        if (!stack.pop_bool()) {
            u8A_to_u16 cvt;
            cvt.u8A[1] = program[index];
            cvt.u8A[0] = program[index + 1];
            index = cvt._u16;
            if (index >= prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        } else index += size;
        return STATUS_SUCCESS;
    }

    RuntimeError handle_CALL(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        IGNORE_UNUSED u32 index_start = index;
        u32 size = 2;
        if (index + size > prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        u8A_to_u16 cvt;
        cvt.u8A[1] = program[index];
        cvt.u8A[0] = program[index + 1];
        RuntimeError call_store_status = stack.pushCall(index + size);
        index = cvt._u16;
        if (index >= prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        if (call_store_status != STATUS_SUCCESS) return call_store_status;
        if (index >= prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_CALL_IF(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        IGNORE_UNUSED u32 index_start = index;
        u32 size = 2;
        if (index + size > prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        if (stack.pop_bool()) {
            u8A_to_u16 cvt;
            cvt.u8A[1] = program[index];
            cvt.u8A[0] = program[index + 1];
            RuntimeError call_store_status = stack.pushCall(index + size);
            index = cvt._u16;
            if (index == 0 || index >= prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
            if (call_store_status != STATUS_SUCCESS) return call_store_status;
            if (index >= prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        } else index += size;
        return STATUS_SUCCESS;
    }
    RuntimeError handle_CALL_IF_NOT(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        IGNORE_UNUSED u32 index_start = index;
        u32 size = 2;
        if (index + size > prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        if (!stack.pop_bool()) {
            u8A_to_u16 cvt;
            cvt.u8A[1] = program[index];
            cvt.u8A[0] = program[index + 1];
            RuntimeError call_store_status = stack.pushCall(index + size);
            index = cvt._u16;
            if (index == 0 || index >= prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
            if (call_store_status != STATUS_SUCCESS) return call_store_status;
            if (index >= prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        } else index += size;
        return STATUS_SUCCESS;
    }
    RuntimeError handle_RET(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        IGNORE_UNUSED u32 index_start = index;
        if (stack.call_stack.size() == 0) return CALL_STACK_UNDERFLOW;
        u16 ret_index = stack.popCall();
        index = ret_index;
        if (index >= prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_RET_IF(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        IGNORE_UNUSED u32 index_start = index;
        if (stack.call_stack.size() == 0) return CALL_STACK_UNDERFLOW;
        if (stack.pop_bool()) {
            u16 ret_index = stack.popCall();
            index = ret_index;
            if (index >= prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        }
        return STATUS_SUCCESS;
    }
    RuntimeError handle_RET_IF_NOT(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        IGNORE_UNUSED u32 index_start = index;
        if (stack.call_stack.size() == 0) return CALL_STACK_UNDERFLOW;
        if (!stack.pop_bool()) {
            u16 ret_index = stack.popCall();
            index = ret_index;
            if (index >= prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        }
        return STATUS_SUCCESS;
    }

    RuntimeError handle_JMP_REL(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        IGNORE_UNUSED u32 index_start = index;
        u32 size = 2;
        if (index + size > prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        i16 offset = (i16)((program[index] << 8) | program[index + 1]);
        i32 new_index = (i32)index + size + offset;
        if (new_index < 0 || new_index >= (i32)prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        index = (u32)new_index;
        return STATUS_SUCCESS;
    }

    RuntimeError handle_JMP_IF_REL(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        IGNORE_UNUSED u32 index_start = index;
        u32 size = 2;
        if (index + size > prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        if (stack.pop_bool()) {
            i16 offset = (i16)((program[index] << 8) | program[index + 1]);
            i32 new_index = (i32)index + size + offset;
            if (new_index < 0 || new_index >= (i32)prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
            index = (u32)new_index;
        } else index += size;
        return STATUS_SUCCESS;
    }

    RuntimeError handle_JMP_IF_NOT_REL(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        IGNORE_UNUSED u32 index_start = index;
        u32 size = 2;
        if (index + size > prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        if (!stack.pop_bool()) {
            i16 offset = (i16)((program[index] << 8) | program[index + 1]);
            i32 new_index = (i32)index + size + offset;
            if (new_index < 0 || new_index >= (i32)prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
            index = (u32)new_index;
        } else index += size;
        return STATUS_SUCCESS;
    }

    RuntimeError handle_CALL_REL(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        IGNORE_UNUSED u32 index_start = index;
        u32 size = 2;
        if (index + size > prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        RuntimeError call_store_status = stack.pushCall(index + size);
        if (call_store_status != STATUS_SUCCESS) return call_store_status;
        i16 offset = (i16)((program[index] << 8) | program[index + 1]);
        i32 new_index = (i32)index + size + offset;
        if (new_index < 0 || new_index >= (i32)prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        index = (u32)new_index;
        return STATUS_SUCCESS;
    }

    RuntimeError handle_CALL_IF_REL(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        IGNORE_UNUSED u32 index_start = index;
        u32 size = 2;
        if (index + size > prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        if (stack.pop_bool()) {
            RuntimeError call_store_status = stack.pushCall(index + size);
            if (call_store_status != STATUS_SUCCESS) return call_store_status;
            i16 offset = (i16)((program[index] << 8) | program[index + 1]);
            i32 new_index = (i32)index + size + offset;
            if (new_index < 0 || new_index >= (i32)prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
            index = (u32)new_index;
        } else index += size;
        return STATUS_SUCCESS;
    }

    RuntimeError handle_CALL_IF_NOT_REL(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        IGNORE_UNUSED u32 index_start = index;
        u32 size = 2;
        if (index + size > prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
        if (!stack.pop_bool()) {
            RuntimeError call_store_status = stack.pushCall(index + size);
            if (call_store_status != STATUS_SUCCESS) return call_store_status;
            i16 offset = (i16)((program[index] << 8) | program[index + 1]);
            i32 new_index = (i32)index + size + offset;
            if (new_index < 0 || new_index >= (i32)prog_size) return CHECK_PROGRAM_POINTER_BOUNDS_HEAD(program, prog_size, index, index_start);
            index = (u32)new_index;
        } else index += size;
        return STATUS_SUCCESS;
    }


    RuntimeError handle_EXIT(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        if (index >= prog_size) return STATUS_SUCCESS;
        u8 exit_code = program[index++];
        return (RuntimeError) exit_code;
    }

}