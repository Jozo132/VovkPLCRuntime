// memory-manipulation.h - 2022-12-11
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

    RuntimeError PUSH_pointer(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        MY_PTR_t value = 0;
        extract_status = ProgramExtract.type_pointer(program, prog_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack.push_pointer(value);
    }

    RuntimeError PUSH_bool(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        bool value = 0;
        extract_status = ProgramExtract.type_bool(program, prog_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack.push_bool(value);
    }
    RuntimeError push_u8(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        u8 value = 0;
        extract_status = ProgramExtract.type_u8(program, prog_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack.push_u8(value);
    }
    RuntimeError push_u16(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        u16 value = 0;
        extract_status = ProgramExtract.type_u16(program, prog_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack.push_u16(value);
    }
    RuntimeError push_u32(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        u32 value = 0;
        extract_status = ProgramExtract.type_u32(program, prog_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack.push_u32(value);
    }
#ifdef USE_X64_OPS
    RuntimeError push_u64(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        u64 value = 0;
        extract_status = ProgramExtract.type_u64(program, prog_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack.push_u64(value);
    }
#endif // USE_X64_OPS
    RuntimeError push_i8(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        i8 value = 0;
        extract_status = ProgramExtract.type_i8(program, prog_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack.push_i8(value);
    }
    RuntimeError push_i16(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        i16 value = 0;
        extract_status = ProgramExtract.type_i16(program, prog_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack.push_i16(value);
    }
    RuntimeError push_i32(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        i32 value = 0;
        extract_status = ProgramExtract.type_i32(program, prog_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack.push_i32(value);
    }

#ifdef USE_X64_OPS
    RuntimeError push_i64(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        i64 value = 0;
        extract_status = ProgramExtract.type_i64(program, prog_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack.push_i64(value);
    }
#endif // USE_X64_OPS
    RuntimeError push_f32(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        f32 value = 0;
        extract_status = ProgramExtract.type_f32(program, prog_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack.push_f32(value);
    }

#ifdef USE_X64_OPS
    RuntimeError push_f64(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        f64 value = 0;
        extract_status = ProgramExtract.type_f64(program, prog_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack.push_f64(value);
    }
#endif // USE_X64_OPS

    /*
    u8 data_type = program[index++];
            switch (data_type) {
                case type_bool:
                case type_u8: return ADD_uint8_t(stack);
                case type_u16: return ADD_uint16_t(stack);
                case type_u32: return ADD_uint32_t(stack);
                case type_u64: return ADD_uint64_t(stack);
                case type_i8: return ADD_int8_t(stack);
                case type_i16: return ADD_int16_t(stack);
                case type_i32: return ADD_int32_t(stack);
                case type_i64: return ADD_int64_t(stack);
                case type_f32: return ADD_float(stack);
                case type_f64: return ADD_double(stack);
                default: return INVALID_DATA_TYPE;
            }
    */

    // Convert value from one type to another ()
    RuntimeError CVT(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        u32 size = 2;
        if (index + size > prog_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 from_type = 0;
        u8 to_type = 0;
        extract_status = ProgramExtract.type_u8(program, prog_size, index, &from_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        extract_status = ProgramExtract.type_u8(program, prog_size, index, &to_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        switch (from_type) {
            case type_pointer:
                switch (to_type) {
                    // case type_pointer: return stack.push_pointer(stack.pop_pointer());
                    case type_bool: return stack.push_bool(stack.pop_pointer());
                    case type_u8: return stack.push_u8(stack.pop_pointer());
                    case type_u16: return stack.push_u16(stack.pop_pointer());
                    case type_u32: return stack.push_u32(stack.pop_pointer());
                    case type_i8: return stack.push_i8(stack.pop_pointer());
                    case type_i16: return stack.push_i16(stack.pop_pointer());
                    case type_i32: return stack.push_i32(stack.pop_pointer());
                    case type_f32: return stack.push_f32(stack.pop_pointer());
#ifdef USE_X64_OPS
                    case type_u64: return stack.push_u64(stack.pop_pointer());
                    case type_i64: return stack.push_i64(stack.pop_pointer());
                    case type_f64: return stack.push_f64(stack.pop_pointer());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
            case type_bool:
                switch (to_type) {
                    // case type_bool: return stack.push_bool(stack.pop_bool());
                    case type_u8: return stack.push_u8(stack.pop_bool());
                    case type_u16: return stack.push_u16(stack.pop_bool());
                    case type_u32: return stack.push_u32(stack.pop_bool());
                    case type_i8: return stack.push_i8(stack.pop_bool());
                    case type_i16: return stack.push_i16(stack.pop_bool());
                    case type_i32: return stack.push_i32(stack.pop_bool());
                    case type_f32: return stack.push_f32(stack.pop_bool());
#ifdef USE_X64_OPS
                    case type_u64: return stack.push_u64(stack.pop_bool());
                    case type_i64: return stack.push_i64(stack.pop_bool());
                    case type_f64: return stack.push_f64(stack.pop_bool());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
            case type_u8:
                switch (to_type) {
                    case type_pointer: return stack.push_pointer(stack.pop_u8());
                    case type_bool: return stack.push_bool(stack.pop_u8());
                        // case type_u8: return stack.push_u8(stack.pop_u8());
                    case type_u16: return stack.push_u16(stack.pop_u8());
                    case type_u32: return stack.push_u32(stack.pop_u8());
                    case type_i8: return stack.push_i8(stack.pop_u8());
                    case type_i16: return stack.push_i16(stack.pop_u8());
                    case type_i32: return stack.push_i32(stack.pop_u8());
                    case type_f32: return stack.push_f32(stack.pop_u8());
#ifdef USE_X64_OPS
                    case type_u64: return stack.push_u64(stack.pop_u8());
                    case type_i64: return stack.push_i64(stack.pop_u8());
                    case type_f64: return stack.push_f64(stack.pop_u8());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
            case type_u16:
                switch (to_type) {
                    case type_pointer: return stack.push_pointer(stack.pop_u16());
                    case type_bool: return stack.push_bool(stack.pop_u16());
                    case type_u8: return stack.push_u8(stack.pop_u16());
                        // case type_u16: return stack.push_u16(stack.pop_u16());
                    case type_u32: return stack.push_u32(stack.pop_u16());
                    case type_i8: return stack.push_i8(stack.pop_u16());
                    case type_i16: return stack.push_i16(stack.pop_u16());
                    case type_i32: return stack.push_i32(stack.pop_u16());
                    case type_f32: return stack.push_f32(stack.pop_u16());
#ifdef USE_X64_OPS
                    case type_u64: return stack.push_u64(stack.pop_u16());
                    case type_i64: return stack.push_i64(stack.pop_u16());
                    case type_f64: return stack.push_f64(stack.pop_u16());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
            case type_u32:
                switch (to_type) {
                    case type_pointer: return stack.push_pointer(stack.pop_u32());
                    case type_bool: return stack.push_bool(stack.pop_u32());
                    case type_u8: return stack.push_u8(stack.pop_u32());
                    case type_u16: return stack.push_u16(stack.pop_u32());
                        // case type_u32: return stack.push_u32(stack.pop_u32());
                    case type_i8: return stack.push_i8(stack.pop_u32());
                    case type_i16: return stack.push_i16(stack.pop_u32());
                    case type_i32: return stack.push_i32(stack.pop_u32());
                    case type_f32: return stack.push_f32(stack.pop_u32());
#ifdef USE_X64_OPS
                    case type_u64: return stack.push_u64(stack.pop_u32());
                    case type_i64: return stack.push_i64(stack.pop_u32());
                    case type_f64: return stack.push_f64(stack.pop_u32());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }

#ifdef USE_X64_OPS
            case type_u64:
                switch (to_type) {
                    case type_pointer: return stack.push_pointer(stack.pop_u64());
                    case type_bool: return stack.push_bool(stack.pop_u64());
                    case type_u8: return stack.push_u8(stack.pop_u64());
                    case type_u16: return stack.push_u16(stack.pop_u64());
                    case type_u32: return stack.push_u32(stack.pop_u64());
                        // case type_u64: return stack.push_u64(stack.pop_u64());
                    case type_i8: return stack.push_i8(stack.pop_u64());
                    case type_i16: return stack.push_i16(stack.pop_u64());
                    case type_i32: return stack.push_i32(stack.pop_u64());
                    case type_i64: return stack.push_i64(stack.pop_u64());
                    case type_f32: return stack.push_f32(stack.pop_u64());
                    case type_f64: return stack.push_f64(stack.pop_u64());
                    default: return INVALID_DATA_TYPE;
                }
#endif // USE_X64_OPS
            case type_i8:
                switch (to_type) {
                    case type_pointer: return stack.push_pointer(stack.pop_i8());
                    case type_bool: return stack.push_bool(stack.pop_i8());
                    case type_u8: return stack.push_u8(stack.pop_i8());
                    case type_u16: return stack.push_u16(stack.pop_i8());
                    case type_u32: return stack.push_u32(stack.pop_i8());
                        // case type_i8: return stack.push_i8(stack.pop_i8());
                    case type_i16: return stack.push_i16(stack.pop_i8());
                    case type_i32: return stack.push_i32(stack.pop_i8());
                    case type_f32: return stack.push_f32(stack.pop_i8());
#ifdef USE_X64_OPS
                    case type_u64: return stack.push_u64(stack.pop_i8());
                    case type_i64: return stack.push_i64(stack.pop_i8());
                    case type_f64: return stack.push_f64(stack.pop_i8());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
            case type_i16:
                switch (to_type) {
                    case type_pointer: return stack.push_pointer(stack.pop_i16());
                    case type_bool: return stack.push_bool(stack.pop_i16());
                    case type_u8: return stack.push_u8(stack.pop_i16());
                    case type_u16: return stack.push_u16(stack.pop_i16());
                    case type_u32: return stack.push_u32(stack.pop_i16());
                    case type_i8: return stack.push_i8(stack.pop_i16());
                        // case type_i16: return stack.push_i16(stack.pop_i16());
                    case type_i32: return stack.push_i32(stack.pop_i16());
                    case type_f32: return stack.push_f32(stack.pop_i16());
#ifdef USE_X64_OPS
                    case type_u64: return stack.push_u64(stack.pop_i16());
                    case type_i64: return stack.push_i64(stack.pop_i16());
                    case type_f64: return stack.push_f64(stack.pop_i16());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
            case type_i32:
                switch (to_type) {
                    case type_pointer: return stack.push_pointer(stack.pop_i32());
                    case type_bool: return stack.push_bool(stack.pop_i32());
                    case type_u8: return stack.push_u8(stack.pop_i32());
                    case type_u16: return stack.push_u16(stack.pop_i32());
                    case type_u32: return stack.push_u32(stack.pop_i32());
                    case type_i8: return stack.push_i8(stack.pop_i32());
                    case type_i16: return stack.push_i16(stack.pop_i32());
                        // case type_i32: return stack.push_i32(stack.pop_i32());
                    case type_f32: return stack.push_f32(stack.pop_i32());
#ifdef USE_X64_OPS
                    case type_u64: return stack.push_u64(stack.pop_i32());
                    case type_i64: return stack.push_i64(stack.pop_i32());
                    case type_f64: return stack.push_f64(stack.pop_i32());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
#ifdef USE_X64_OPS
            case type_i64:
                switch (to_type) {
                    case type_pointer: return stack.push_pointer(stack.pop_i64());
                    case type_bool: return stack.push_bool(stack.pop_i64());
                    case type_u8: return stack.push_u8(stack.pop_i64());
                    case type_u16: return stack.push_u16(stack.pop_i64());
                    case type_u32: return stack.push_u32(stack.pop_i64());
                    case type_u64: return stack.push_u64(stack.pop_i64());
                    case type_i8: return stack.push_i8(stack.pop_i64());
                    case type_i16: return stack.push_i16(stack.pop_i64());
                    case type_i32: return stack.push_i32(stack.pop_i64());
                        // case type_i64: return stack.push_i64(stack.pop_i64());
                    case type_f32: return stack.push_f32(stack.pop_i64());
                    case type_f64: return stack.push_f64(stack.pop_i64());
                    default: return INVALID_DATA_TYPE;
                }
#endif // USE_X64_OPS
            case type_f32:
                switch (to_type) {
                    case type_pointer: return stack.push_pointer(stack.pop_f32());
                    case type_bool: return stack.push_bool((int) stack.pop_f32());
                    case type_u8: return stack.push_u8((int) stack.pop_f32());
                    case type_u16: return stack.push_u16(stack.pop_f32());
                    case type_u32: return stack.push_u32(stack.pop_f32());
                    case type_i8: return stack.push_i8(stack.pop_f32());
                    case type_i16: return stack.push_i16(stack.pop_f32());
                    case type_i32: return stack.push_i32(stack.pop_f32());
                        // case type_f32: return stack.push_f32(stack.pop_f32());
#ifdef USE_X64_OPS
                    case type_u64: return stack.push_u64(stack.pop_f32());
                    case type_i64: return stack.push_i64(stack.pop_f32());
                    case type_f64: return stack.push_f64(stack.pop_f32());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
#ifdef USE_X64_OPS
            case type_f64:
                switch (to_type) {
                    case type_pointer: return stack.push_pointer(stack.pop_f64());
                    case type_bool: return stack.push_bool((int) stack.pop_f64());
                    case type_u8: return stack.push_u8((int) stack.pop_f64());
                    case type_u16: return stack.push_u16(stack.pop_f64());
                    case type_u32: return stack.push_u32(stack.pop_f64());
                    case type_i8: return stack.push_i8(stack.pop_f64());
                    case type_i16: return stack.push_i16(stack.pop_f64());
                    case type_i32: return stack.push_i32(stack.pop_f64());
                    case type_u64: return stack.push_u64(stack.pop_f64());
                    case type_i64: return stack.push_i64(stack.pop_f64());
                    case type_f32: return stack.push_f32(stack.pop_f64());
                        // case type_f64: return stack.push_f64(stack.pop_f64());
                    default: return INVALID_DATA_TYPE;
                }
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }


    // Pop the pointer from the stack and push the value from the memory to the stack
    RuntimeError LOAD(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        u32 size = 1;
        if (index + size > prog_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = 0;
        extract_status = ProgramExtract.type_u8(program, prog_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        RuntimeError status = stack.load_from_memory_to_stack(memory, data_type);
        return status;
    }

    // (1.) Pop the value from the stack, (2.) pop the pointer from the stack, (3.) put the value to the memory
    RuntimeError MOVE(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        u32 size = 1;
        if (index + size > prog_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = 0;
        extract_status = ProgramExtract.type_u8(program, prog_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        RuntimeError status = stack.store_from_stack_to_memory(memory, data_type);
        return status;
    }

    // (1.) Pop the value from the stack, (2.) pop the pointer from the stack, (3.) put the value to the memory, (4.) push the value back to the stack
    RuntimeError MOVE_COPY(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        u32 size = 1;
        if (index + size > prog_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = 0;
        extract_status = ProgramExtract.type_u8(program, prog_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        RuntimeError status = stack.store_from_stack_to_memory(memory, data_type, true);
        return status;
    }

    template <typename T> RuntimeError load_from_memory_to_stack_at(RuntimeStack& stack, u8* memory, MY_PTR_t address) {
        if (address + sizeof(T) > PLCRUNTIME_MAX_MEMORY_SIZE) return RuntimeError::INVALID_MEMORY_ADDRESS;
        T value = 0;
        bool error = readArea_u8(memory, address, reinterpret_cast<u8*>(&value), sizeof(T));
        if (error) return RuntimeError::INVALID_MEMORY_ADDRESS;
        error = stack.push_custom(value);
        return error ? RuntimeError::STACK_OVERFLOW : RuntimeError::STATUS_SUCCESS;
    }

    template <typename T> RuntimeError store_from_stack_to_memory_at(RuntimeStack& stack, u8* memory, MY_PTR_t address) {
        if (stack.size() < sizeof(T)) return RuntimeError::STACK_UNDERFLOW;
        T value = stack.pop_custom<T>();
        if (address + sizeof(T) > PLCRUNTIME_MAX_MEMORY_SIZE) return RuntimeError::INVALID_MEMORY_ADDRESS;
        bool error = writeArea_u8(memory, address, reinterpret_cast<u8*>(&value), sizeof(T));
        return error ? RuntimeError::INVALID_MEMORY_ADDRESS : RuntimeError::STATUS_SUCCESS;
    }

    // Load value from memory to stack using immediate address
    // Both memory and stack use native endianness
    RuntimeError LOAD_FROM(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        u32 size = 1 + sizeof(MY_PTR_t);
        if (index + size > prog_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = 0;
        extract_status = ProgramExtract.type_u8(program, prog_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        MY_PTR_t address = 0;
        extract_status = ProgramExtract.type_pointer(program, prog_size, index, &address);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        // ProgramExtract already converts from big-endian bytecode to native, no need to reverse
        
        // Read native-endian from memory, push native-endian to stack
        switch (data_type) {
            case type_pointer: {
                if (address + sizeof(MY_PTR_t) > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                MY_PTR_t value = 0;
                for (u32 i = 0; i < sizeof(MY_PTR_t); i++) {
                    value |= ((MY_PTR_t)memory[address + i] << (8 * i));
                }
                return stack.push_pointer(value);
            }
            case type_bool:
            case type_u8: {
                if (address + 1 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                return stack.push_u8(memory[address]);
            }
            case type_i8: {
                if (address + 1 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                return stack.push_i8((i8)memory[address]);
            }
            case type_u16:
            case type_i16: {
                if (address + 2 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                // Read little-endian from memory
                u16 value = memory[address] | ((u16)memory[address + 1] << 8);
                return stack.push_u16(value);
            }
            case type_u32:
            case type_i32:
            case type_f32: {
                if (address + 4 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                // Read little-endian from memory
                u32 value = memory[address] | ((u32)memory[address + 1] << 8) |
                            ((u32)memory[address + 2] << 16) | ((u32)memory[address + 3] << 24);
                return stack.push_u32(value);
            }
#ifdef USE_X64_OPS
            case type_u64:
            case type_i64:
            case type_f64: {
                if (address + 8 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                // Read little-endian from memory
                u64 value = memory[address] | ((u64)memory[address + 1] << 8) |
                            ((u64)memory[address + 2] << 16) | ((u64)memory[address + 3] << 24) |
                            ((u64)memory[address + 4] << 32) | ((u64)memory[address + 5] << 40) |
                            ((u64)memory[address + 6] << 48) | ((u64)memory[address + 7] << 56);
                return stack.push_u64(value);
            }
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }

    // Move value from stack to memory using immediate address
    // Both stack and memory use native endianness
    RuntimeError MOVE_TO(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        u32 size = 1 + sizeof(MY_PTR_t);
        if (index + size > prog_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = 0;
        extract_status = ProgramExtract.type_u8(program, prog_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        MY_PTR_t address = 0;
        extract_status = ProgramExtract.type_pointer(program, prog_size, index, &address);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        // ProgramExtract already converts from big-endian bytecode to native, no need to reverse
        
        // Pop native-endian from stack, write native-endian to memory
        switch (data_type) {
            case type_pointer: {
                if (stack.size() < sizeof(MY_PTR_t)) return STACK_UNDERFLOW;
                if (address + sizeof(MY_PTR_t) > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                MY_PTR_t value = stack.pop_pointer();
                for (u32 i = 0; i < sizeof(MY_PTR_t); i++) {
                    memory[address + i] = (value >> (8 * i)) & 0xFF;
                }
                return STATUS_SUCCESS;
            }
            case type_bool:
            case type_u8:
            case type_i8: {
                if (stack.size() < 1) return STACK_UNDERFLOW;
                if (address + 1 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                memory[address] = stack.pop_u8();
                return STATUS_SUCCESS;
            }
            case type_u16:
            case type_i16: {
                if (stack.size() < 2) return STACK_UNDERFLOW;
                if (address + 2 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                u16 value = stack.pop_u16();
                // Write little-endian to memory
                memory[address] = value & 0xFF;
                memory[address + 1] = (value >> 8) & 0xFF;
                return STATUS_SUCCESS;
            }
            case type_u32:
            case type_i32:
            case type_f32: {
                if (stack.size() < 4) return STACK_UNDERFLOW;
                if (address + 4 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                u32 value = stack.pop_u32();
                // Write little-endian to memory
                memory[address] = value & 0xFF;
                memory[address + 1] = (value >> 8) & 0xFF;
                memory[address + 2] = (value >> 16) & 0xFF;
                memory[address + 3] = (value >> 24) & 0xFF;
                return STATUS_SUCCESS;
            }
#ifdef USE_X64_OPS
            case type_u64:
            case type_i64:
            case type_f64: {
                if (stack.size() < 8) return STACK_UNDERFLOW;
                if (address + 8 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                u64 value = stack.pop_u64();
                // Write little-endian to memory
                memory[address] = value & 0xFF;
                memory[address + 1] = (value >> 8) & 0xFF;
                memory[address + 2] = (value >> 16) & 0xFF;
                memory[address + 3] = (value >> 24) & 0xFF;
                memory[address + 4] = (value >> 32) & 0xFF;
                memory[address + 5] = (value >> 40) & 0xFF;
                memory[address + 6] = (value >> 48) & 0xFF;
                memory[address + 7] = (value >> 56) & 0xFF;
                return STATUS_SUCCESS;
            }
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }

    // Increment value in memory by 1
    // Memory uses native endianness
    RuntimeError INC_MEM(u8* memory, u8* program, u32 prog_size, u32& index) {
        u32 size = 1 + sizeof(MY_PTR_t);
        if (index + size > prog_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = 0;
        extract_status = ProgramExtract.type_u8(program, prog_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        MY_PTR_t address = 0;
        extract_status = ProgramExtract.type_pointer(program, prog_size, index, &address);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        // ProgramExtract already converts from big-endian bytecode to native, no need to reverse
        
        switch (data_type) {
            case type_bool:
            case type_u8: {
                if (address + 1 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                memory[address]++;
                return STATUS_SUCCESS;
            }
            case type_i8: {
                if (address + 1 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                i8 value = (i8)memory[address];
                value++;
                memory[address] = (u8)value;
                return STATUS_SUCCESS;
            }
            case type_u16:
            case type_i16: {
                if (address + 2 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                u16 value = memory[address] | ((u16)memory[address + 1] << 8);
                value++;
                memory[address] = value & 0xFF;
                memory[address + 1] = (value >> 8) & 0xFF;
                return STATUS_SUCCESS;
            }
            case type_u32:
            case type_i32: {
                if (address + 4 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                u32 value = memory[address] | ((u32)memory[address + 1] << 8) |
                            ((u32)memory[address + 2] << 16) | ((u32)memory[address + 3] << 24);
                value++;
                memory[address] = value & 0xFF;
                memory[address + 1] = (value >> 8) & 0xFF;
                memory[address + 2] = (value >> 16) & 0xFF;
                memory[address + 3] = (value >> 24) & 0xFF;
                return STATUS_SUCCESS;
            }
            case type_f32: {
                if (address + 4 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                u32 bits = memory[address] | ((u32)memory[address + 1] << 8) |
                           ((u32)memory[address + 2] << 16) | ((u32)memory[address + 3] << 24);
                float value; memcpy(&value, &bits, 4);
                value += 1.0f;
                memcpy(&bits, &value, 4);
                memory[address] = bits & 0xFF;
                memory[address + 1] = (bits >> 8) & 0xFF;
                memory[address + 2] = (bits >> 16) & 0xFF;
                memory[address + 3] = (bits >> 24) & 0xFF;
                return STATUS_SUCCESS;
            }
#ifdef USE_X64_OPS
            case type_u64:
            case type_i64: {
                if (address + 8 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                u64 value = memory[address] | ((u64)memory[address + 1] << 8) |
                            ((u64)memory[address + 2] << 16) | ((u64)memory[address + 3] << 24) |
                            ((u64)memory[address + 4] << 32) | ((u64)memory[address + 5] << 40) |
                            ((u64)memory[address + 6] << 48) | ((u64)memory[address + 7] << 56);
                value++;
                memory[address] = value & 0xFF;
                memory[address + 1] = (value >> 8) & 0xFF;
                memory[address + 2] = (value >> 16) & 0xFF;
                memory[address + 3] = (value >> 24) & 0xFF;
                memory[address + 4] = (value >> 32) & 0xFF;
                memory[address + 5] = (value >> 40) & 0xFF;
                memory[address + 6] = (value >> 48) & 0xFF;
                memory[address + 7] = (value >> 56) & 0xFF;
                return STATUS_SUCCESS;
            }
            case type_f64: {
                if (address + 8 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                u64 bits = memory[address] | ((u64)memory[address + 1] << 8) |
                           ((u64)memory[address + 2] << 16) | ((u64)memory[address + 3] << 24) |
                           ((u64)memory[address + 4] << 32) | ((u64)memory[address + 5] << 40) |
                           ((u64)memory[address + 6] << 48) | ((u64)memory[address + 7] << 56);
                double value; memcpy(&value, &bits, 8);
                value += 1.0;
                memcpy(&bits, &value, 8);
                memory[address] = bits & 0xFF;
                memory[address + 1] = (bits >> 8) & 0xFF;
                memory[address + 2] = (bits >> 16) & 0xFF;
                memory[address + 3] = (bits >> 24) & 0xFF;
                memory[address + 4] = (bits >> 32) & 0xFF;
                memory[address + 5] = (bits >> 40) & 0xFF;
                memory[address + 6] = (bits >> 48) & 0xFF;
                memory[address + 7] = (bits >> 56) & 0xFF;
                return STATUS_SUCCESS;
            }
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }

    // Decrement value in memory by 1
    // Memory uses native endianness
    RuntimeError DEC_MEM(u8* memory, u8* program, u32 prog_size, u32& index) {
        u32 size = 1 + sizeof(MY_PTR_t);
        if (index + size > prog_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = 0;
        extract_status = ProgramExtract.type_u8(program, prog_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        MY_PTR_t address = 0;
        extract_status = ProgramExtract.type_pointer(program, prog_size, index, &address);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        // ProgramExtract already converts from big-endian bytecode to native, no need to reverse
        
        switch (data_type) {
            case type_bool:
            case type_u8: {
                if (address + 1 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                memory[address]--;
                return STATUS_SUCCESS;
            }
            case type_i8: {
                if (address + 1 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                i8 value = (i8)memory[address];
                value--;
                memory[address] = (u8)value;
                return STATUS_SUCCESS;
            }
            case type_u16:
            case type_i16: {
                if (address + 2 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                u16 value = memory[address] | ((u16)memory[address + 1] << 8);
                value--;
                memory[address] = value & 0xFF;
                memory[address + 1] = (value >> 8) & 0xFF;
                return STATUS_SUCCESS;
            }
            case type_u32:
            case type_i32: {
                if (address + 4 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                u32 value = memory[address] | ((u32)memory[address + 1] << 8) |
                            ((u32)memory[address + 2] << 16) | ((u32)memory[address + 3] << 24);
                value--;
                memory[address] = value & 0xFF;
                memory[address + 1] = (value >> 8) & 0xFF;
                memory[address + 2] = (value >> 16) & 0xFF;
                memory[address + 3] = (value >> 24) & 0xFF;
                return STATUS_SUCCESS;
            }
            case type_f32: {
                if (address + 4 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                u32 bits = memory[address] | ((u32)memory[address + 1] << 8) |
                           ((u32)memory[address + 2] << 16) | ((u32)memory[address + 3] << 24);
                float value; memcpy(&value, &bits, 4);
                value -= 1.0f;
                memcpy(&bits, &value, 4);
                memory[address] = bits & 0xFF;
                memory[address + 1] = (bits >> 8) & 0xFF;
                memory[address + 2] = (bits >> 16) & 0xFF;
                memory[address + 3] = (bits >> 24) & 0xFF;
                return STATUS_SUCCESS;
            }
#ifdef USE_X64_OPS
            case type_u64:
            case type_i64: {
                if (address + 8 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                u64 value = memory[address] | ((u64)memory[address + 1] << 8) |
                            ((u64)memory[address + 2] << 16) | ((u64)memory[address + 3] << 24) |
                            ((u64)memory[address + 4] << 32) | ((u64)memory[address + 5] << 40) |
                            ((u64)memory[address + 6] << 48) | ((u64)memory[address + 7] << 56);
                value--;
                memory[address] = value & 0xFF;
                memory[address + 1] = (value >> 8) & 0xFF;
                memory[address + 2] = (value >> 16) & 0xFF;
                memory[address + 3] = (value >> 24) & 0xFF;
                memory[address + 4] = (value >> 32) & 0xFF;
                memory[address + 5] = (value >> 40) & 0xFF;
                memory[address + 6] = (value >> 48) & 0xFF;
                memory[address + 7] = (value >> 56) & 0xFF;
                return STATUS_SUCCESS;
            }
            case type_f64: {
                if (address + 8 > PLCRUNTIME_MAX_MEMORY_SIZE) return INVALID_MEMORY_ADDRESS;
                u64 bits = memory[address] | ((u64)memory[address + 1] << 8) |
                           ((u64)memory[address + 2] << 16) | ((u64)memory[address + 3] << 24) |
                           ((u64)memory[address + 4] << 32) | ((u64)memory[address + 5] << 40) |
                           ((u64)memory[address + 6] << 48) | ((u64)memory[address + 7] << 56);
                double value; memcpy(&value, &bits, 8);
                value -= 1.0;
                memcpy(&bits, &value, 8);
                memory[address] = bits & 0xFF;
                memory[address + 1] = (bits >> 8) & 0xFF;
                memory[address + 2] = (bits >> 16) & 0xFF;
                memory[address + 3] = (bits >> 24) & 0xFF;
                memory[address + 4] = (bits >> 32) & 0xFF;
                memory[address + 5] = (bits >> 40) & 0xFF;
                memory[address + 6] = (bits >> 48) & 0xFF;
                memory[address + 7] = (bits >> 56) & 0xFF;
                return STATUS_SUCCESS;
            }
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }


    // Duplicate the value on top of the stack
    RuntimeError COPY(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        u32 size = 1;
        if (index + size > prog_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = 0;
        extract_status = ProgramExtract.type_u8(program, prog_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        switch (data_type) {
            case type_pointer: {
                RuntimeError e = STATUS_SUCCESS;
                MY_PTR_t value = stack.peek_pointer();
                // Stack uses native endianness, no need to reverse
                e = stack.push_pointer(value);
                return e;
            }
            case type_bool: return stack.push_bool(stack.peek_bool());
            case type_i8:
            case type_u8: return stack.push_u8(stack.peek_u8());
            case type_i16:
            case type_u16: return stack.push_u16(stack.peek_u16());
            case type_i32:
            case type_u32:
            case type_f32: return stack.push_u32(stack.peek_u32());
#ifdef USE_X64_OPS
            case type_u64:
            case type_i64:
            case type_f64: return stack.push_u64(stack.peek_u64());
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }


    // Pick (copy) value from stack at byte depth and push to top
    // Format: [ PICK, type, depth (MY_PTR_t) ]
    // Depth is in bytes from top of stack (0 = top)
    RuntimeError PICK(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        u32 size = 1 + MY_PTR_SIZE_BYTES;
        if (index + size > prog_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = 0;
        MY_PTR_t depth = 0;
        extract_status = ProgramExtract.type_u8(program, prog_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        extract_status = ProgramExtract.type_pointer(program, prog_size, index, &depth);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        
        // Check if depth is within stack bounds
        if (depth >= stack.size()) return STACK_UNDERFLOW;
        
        // Get size of the type to pick
        u8 type_size = 0;
        switch (data_type) {
            case type_pointer: type_size = MY_PTR_SIZE_BYTES; break;
            case type_bool:
            case type_u8:
            case type_i8: type_size = 1; break;
            case type_u16:
            case type_i16: type_size = 2; break;
            case type_u32:
            case type_i32:
            case type_f32: type_size = 4; break;
#ifdef USE_X64_OPS
            case type_u64:
            case type_i64:
            case type_f64: type_size = 8; break;
#endif
            default: return INVALID_DATA_TYPE;
        }
        
        // Check if the full value is within stack bounds
        if (depth + type_size > stack.size()) return STACK_UNDERFLOW;
        
        // Read value from stack at depth (byte-by-byte, big-endian)
        // depth is from top, so we read from stack.size() - depth - type_size
        u32 base_index = stack.size() - depth - type_size;
        
        switch (data_type) {
            case type_pointer: {
                MY_PTR_t value = 0;
                for (u8 i = 0; i < MY_PTR_SIZE_BYTES; i++) {
                    value = (value << 8) | stack.peek(stack.size() - base_index - MY_PTR_SIZE_BYTES + i);
                }
                return stack.push_pointer(value);
            }
            case type_bool: return stack.push_bool(stack.peek(depth));
            case type_u8: return stack.push_u8(stack.peek(depth));
            case type_i8: return stack.push_i8((i8)stack.peek(depth));
            case type_u16: {
                u16 value = ((u16)stack.peek(depth + 1) << 8) | stack.peek(depth);
                return stack.push_u16(value);
            }
            case type_i16: {
                i16 value = ((i16)stack.peek(depth + 1) << 8) | stack.peek(depth);
                return stack.push_i16(value);
            }
            case type_u32:
            case type_i32:
            case type_f32: {
                u32 value = 0;
                for (u8 i = 0; i < 4; i++) {
                    value = (value << 8) | stack.peek(depth + 3 - i);
                }
                return stack.push_u32(value);
            }
#ifdef USE_X64_OPS
            case type_u64:
            case type_i64:
            case type_f64: {
                u64 value = 0;
                for (u8 i = 0; i < 8; i++) {
                    value = (value << 8) | stack.peek(depth + 7 - i);
                }
                return stack.push_u64(value);
            }
#endif
            default: return INVALID_DATA_TYPE;
        }
    }


    // Poke (write) top of stack to stack at byte depth (does not pop the source value)
    // Format: [ POKE, type, depth (MY_PTR_t) ]
    // Depth is in bytes from top of stack (after accounting for source value)
    RuntimeError POKE(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        u32 size = 1 + MY_PTR_SIZE_BYTES;
        if (index + size > prog_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = 0;
        MY_PTR_t depth = 0;
        extract_status = ProgramExtract.type_u8(program, prog_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        extract_status = ProgramExtract.type_pointer(program, prog_size, index, &depth);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        
        // Get size of the type to poke
        u8 type_size = 0;
        switch (data_type) {
            case type_pointer: type_size = MY_PTR_SIZE_BYTES; break;
            case type_bool:
            case type_u8:
            case type_i8: type_size = 1; break;
            case type_u16:
            case type_i16: type_size = 2; break;
            case type_u32:
            case type_i32:
            case type_f32: type_size = 4; break;
#ifdef USE_X64_OPS
            case type_u64:
            case type_i64:
            case type_f64: type_size = 8; break;
#endif
            default: return INVALID_DATA_TYPE;
        }
        
        // Check stack has enough data (source value + depth + target space)
        // Depth is measured from below the source value
        if (stack.size() < type_size) return STACK_UNDERFLOW;
        if (depth + type_size > stack.size() - type_size) return STACK_UNDERFLOW;
        
        // Calculate target index (from bottom of stack)
        // depth is from the top (excluding source value), so target is at:
        // stack.size() - type_size (source) - depth - type_size (target)
        u32 target_index = stack.size() - type_size - depth - type_size;
        
        // Read source value and write to target position byte-by-byte
        switch (data_type) {
            case type_bool:
            case type_u8:
            case type_i8: {
                u8 value = stack.peek(0);  // Top of stack
                stack.stack.set(target_index, value);
                return STATUS_SUCCESS;
            }
            case type_u16:
            case type_i16: {
                // Read source (top 2 bytes)
                u8 hi = stack.peek(1);
                u8 lo = stack.peek(0);
                stack.stack.set(target_index, hi);
                stack.stack.set(target_index + 1, lo);
                return STATUS_SUCCESS;
            }
            case type_u32:
            case type_i32:
            case type_f32: {
                for (u8 i = 0; i < 4; i++) {
                    stack.stack.set(target_index + i, stack.peek(3 - i));
                }
                return STATUS_SUCCESS;
            }
#ifdef USE_X64_OPS
            case type_u64:
            case type_i64:
            case type_f64: {
                for (u8 i = 0; i < 8; i++) {
                    stack.stack.set(target_index + i, stack.peek(7 - i));
                }
                return STATUS_SUCCESS;
            }
#endif
            case type_pointer: {
                for (u8 i = 0; i < MY_PTR_SIZE_BYTES; i++) {
                    stack.stack.set(target_index + i, stack.peek(MY_PTR_SIZE_BYTES - 1 - i));
                }
                return STATUS_SUCCESS;
            }
            default: return INVALID_DATA_TYPE;
        }
    }


    template <typename A, typename B> RuntimeError _SWAP_CUSTOM(RuntimeStack& stack) {
        if (sizeof(A) + sizeof(B) > stack.size()) return STACK_UNDERFLOW;
        B b = stack.pop_custom<B>();
        A a = stack.pop_custom<A>();
        // Stack now uses native endianness, no need to reverse
        stack.push_custom<B>(b);
        stack.push_custom<A>(a);
        return STATUS_SUCCESS;
    }

    // Swap the two values on top of the stack
    RuntimeError SWAP(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        u32 size = 2;
        if (index + size > prog_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 type_A = 0, type_B = 0;
        RuntimeError status;
        status = ProgramExtract.type_u8(program, prog_size, index, &type_A); if (status != STATUS_SUCCESS) return status;
        status = ProgramExtract.type_u8(program, prog_size, index, &type_B); if (status != STATUS_SUCCESS) return status;
        u8 size_A = 0, size_B = 0;
        switch (type_A) {
            case type_pointer: size_A = size_ptr; break;
            case type_bool: size_A = size_bool; break;
            case type_u8:
            case type_i8: size_A = size_u8; break;
            case type_u16:
            case type_i16: size_A = size_u16; break;
            case type_u32:
            case type_i32:
            case type_f32: size_A = size_u32; break;
#ifdef USE_X64_OPS
            case type_u64:
            case type_i64:
            case type_f64: size_A = size_u64; break;
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
        switch (type_B) {
            case type_pointer: size_B = size_ptr; break;
            case type_bool:
            case type_u8:
            case type_i8: size_B = size_u8; break;
            case type_u16:
            case type_i16: size_B = size_u16; break;
            case type_u32:
            case type_i32:
            case type_f32: size_B = size_u32; break;
#ifdef USE_X64_OPS
            case type_u64:
            case type_i64:
            case type_f64: size_B = size_u64; break;
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }

        switch (size_A) {
            case size_u8:
                switch (size_B) {
                    case size_u8: return _SWAP_CUSTOM<u8, u8>(stack);
                    case size_u16: return _SWAP_CUSTOM<u8, u16>(stack);
                    case size_u32: return _SWAP_CUSTOM<u8, u32>(stack);
#ifdef USE_X64_OPS
                    case size_u64: return _SWAP_CUSTOM<u8, u64>(stack);
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
            case size_u16:
                switch (size_B) {
                    case size_u8: return _SWAP_CUSTOM<u16, u8>(stack);
                    case size_u16: return _SWAP_CUSTOM<u16, u16>(stack);
                    case size_u32: return _SWAP_CUSTOM<u16, u32>(stack);
#ifdef USE_X64_OPS
                    case size_u64: return _SWAP_CUSTOM<u16, u64>(stack);
#endif // USE_X64_OPS   
                    default: return INVALID_DATA_TYPE;
                }
            case size_u32:
                switch (size_B) {
                    case size_u8: return _SWAP_CUSTOM<u32, u8>(stack);
                    case size_u16: return _SWAP_CUSTOM<u32, u16>(stack);
                    case size_u32: return _SWAP_CUSTOM<u32, u32>(stack);
#ifdef USE_X64_OPS
                    case size_u64: return _SWAP_CUSTOM<u32, u64>(stack);
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
#ifdef USE_X64_OPS
            case size_u64:
                switch (size_B) {
                    case size_u8: return _SWAP_CUSTOM<u64, u8>(stack);
                    case size_u16: return _SWAP_CUSTOM<u64, u16>(stack);
                    case size_u32: return _SWAP_CUSTOM<u64, u32>(stack);
                    case size_u64: return _SWAP_CUSTOM<u64, u64>(stack);
                    default: return INVALID_DATA_TYPE;
                }
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }

    // Drop the value on top of the stack
    RuntimeError DROP(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        u32 size = 1;
        if (index + size > prog_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = 0;
        extract_status = ProgramExtract.type_u8(program, prog_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        switch (data_type) {
            case type_pointer: stack.pop_pointer(); break;
            case type_bool: stack.pop_bool(); break;
            case type_u8: stack.pop_u8(); break;
            case type_u16: stack.pop_u16(); break;
            case type_u32: stack.pop_u32(); break;
            case type_i8: stack.pop_i8(); break;
            case type_i16: stack.pop_i16(); break;
            case type_i32: stack.pop_i32(); break;
            case type_f32: stack.pop_f32(); break;
#ifdef USE_X64_OPS
            case type_u64: stack.pop_u64(); break;
            case type_i64: stack.pop_i64(); break;
            case type_f64: stack.pop_f64(); break;
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
        return STATUS_SUCCESS;
    }
    // Clear the stack
    RuntimeError CLEAR(RuntimeStack& stack) {
        stack.clear();
        return STATUS_SUCCESS;
    }
}
