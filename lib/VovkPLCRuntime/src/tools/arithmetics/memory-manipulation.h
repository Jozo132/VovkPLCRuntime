// memory-manipulation.h - 1.0.0 - 2022-12-11
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

    RuntimeError PUSH_pointer(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        MY_PTR_t value = 0;
        extract_status = ProgramExtract.type_pointer(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_pointer(value);
    }

    RuntimeError PUSH_bool(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        bool value = 0;
        extract_status = ProgramExtract.type_bool(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_bool(value);
    }
    RuntimeError push_u8(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u8 value = 0;
        extract_status = ProgramExtract.type_u8(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_u8(value);
    }
    RuntimeError push_u16(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u16 value = 0;
        extract_status = ProgramExtract.type_u16(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_u16(value);
    }
    RuntimeError push_u32(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u32 value = 0;
        extract_status = ProgramExtract.type_u32(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_u32(value);
    }
#ifdef USE_X64_OPS
    RuntimeError push_u64(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u64 value = 0;
        extract_status = ProgramExtract.type_u64(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_u64(value);
    }
#endif // USE_X64_OPS
    RuntimeError push_i8(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        i8 value = 0;
        extract_status = ProgramExtract.type_i8(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_i8(value);
    }
    RuntimeError push_i16(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        i16 value = 0;
        extract_status = ProgramExtract.type_i16(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_i16(value);
    }
    RuntimeError push_i32(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        i32 value = 0;
        extract_status = ProgramExtract.type_i32(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_i32(value);
    }

#ifdef USE_X64_OPS
    RuntimeError push_i64(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        i64 value = 0;
        extract_status = ProgramExtract.type_i64(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_i64(value);
    }
#endif // USE_X64_OPS
    RuntimeError push_f32(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        f32 value = 0;
        extract_status = ProgramExtract.type_f32(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_f32(value);
    }

#ifdef USE_X64_OPS
    RuntimeError push_f64(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        f64 value = 0;
        extract_status = ProgramExtract.type_f64(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_f64(value);
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
    RuntimeError CVT(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u32 size = 2;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 from_type = 0;
        u8 to_type = 0;
        extract_status = ProgramExtract.type_u8(program, program_size, index, &from_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        extract_status = ProgramExtract.type_u8(program, program_size, index, &to_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        switch (from_type) {
            case type_pointer:
                switch (to_type) {
                    // case type_pointer: return stack->push_pointer(stack->pop_pointer());
                    case type_bool: return stack->push_bool(stack->pop_pointer());
                    case type_u8: return stack->push_u8(stack->pop_pointer());
                    case type_u16: return stack->push_u16(stack->pop_pointer());
                    case type_u32: return stack->push_u32(stack->pop_pointer());
                    case type_i8: return stack->push_i8(stack->pop_pointer());
                    case type_i16: return stack->push_i16(stack->pop_pointer());
                    case type_i32: return stack->push_i32(stack->pop_pointer());
                    case type_f32: return stack->push_f32(stack->pop_pointer());
#ifdef USE_X64_OPS
                    case type_u64: return stack->push_u64(stack->pop_pointer());
                    case type_i64: return stack->push_i64(stack->pop_pointer());
                    case type_f64: return stack->push_f64(stack->pop_pointer());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
            case type_bool:
                switch (to_type) {
                    // case type_bool: return stack->push_bool(stack->pop_bool());
                    case type_u8: return stack->push_u8(stack->pop_bool());
                    case type_u16: return stack->push_u16(stack->pop_bool());
                    case type_u32: return stack->push_u32(stack->pop_bool());
                    case type_i8: return stack->push_i8(stack->pop_bool());
                    case type_i16: return stack->push_i16(stack->pop_bool());
                    case type_i32: return stack->push_i32(stack->pop_bool());
                    case type_f32: return stack->push_f32(stack->pop_bool());
#ifdef USE_X64_OPS
                    case type_u64: return stack->push_u64(stack->pop_bool());
                    case type_i64: return stack->push_i64(stack->pop_bool());
                    case type_f64: return stack->push_f64(stack->pop_bool());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
            case type_u8:
                switch (to_type) {
                    case type_pointer: return stack->push_pointer(stack->pop_u8());
                    case type_bool: return stack->push_bool(stack->pop_u8());
                        // case type_u8: return stack->push_u8(stack->pop_u8());
                    case type_u16: return stack->push_u16(stack->pop_u8());
                    case type_u32: return stack->push_u32(stack->pop_u8());
                    case type_i8: return stack->push_i8(stack->pop_u8());
                    case type_i16: return stack->push_i16(stack->pop_u8());
                    case type_i32: return stack->push_i32(stack->pop_u8());
                    case type_f32: return stack->push_f32(stack->pop_u8());
#ifdef USE_X64_OPS
                    case type_u64: return stack->push_u64(stack->pop_u8());
                    case type_i64: return stack->push_i64(stack->pop_u8());
                    case type_f64: return stack->push_f64(stack->pop_u8());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
            case type_u16:
                switch (to_type) {
                    case type_pointer: return stack->push_pointer(stack->pop_u16());
                    case type_bool: return stack->push_bool(stack->pop_u16());
                    case type_u8: return stack->push_u8(stack->pop_u16());
                        // case type_u16: return stack->push_u16(stack->pop_u16());
                    case type_u32: return stack->push_u32(stack->pop_u16());
                    case type_i8: return stack->push_i8(stack->pop_u16());
                    case type_i16: return stack->push_i16(stack->pop_u16());
                    case type_i32: return stack->push_i32(stack->pop_u16());
                    case type_f32: return stack->push_f32(stack->pop_u16());
#ifdef USE_X64_OPS
                    case type_u64: return stack->push_u64(stack->pop_u16());
                    case type_i64: return stack->push_i64(stack->pop_u16());
                    case type_f64: return stack->push_f64(stack->pop_u16());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
            case type_u32:
                switch (to_type) {
                    case type_pointer: return stack->push_pointer(stack->pop_u32());
                    case type_bool: return stack->push_bool(stack->pop_u32());
                    case type_u8: return stack->push_u8(stack->pop_u32());
                    case type_u16: return stack->push_u16(stack->pop_u32());
                        // case type_u32: return stack->push_u32(stack->pop_u32());
                    case type_i8: return stack->push_i8(stack->pop_u32());
                    case type_i16: return stack->push_i16(stack->pop_u32());
                    case type_i32: return stack->push_i32(stack->pop_u32());
                    case type_f32: return stack->push_f32(stack->pop_u32());
#ifdef USE_X64_OPS
                    case type_u64: return stack->push_u64(stack->pop_u32());
                    case type_i64: return stack->push_i64(stack->pop_u32());
                    case type_f64: return stack->push_f64(stack->pop_u32());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }

#ifdef USE_X64_OPS
            case type_u64:
                switch (to_type) {
                    case type_pointer: return stack->push_pointer(stack->pop_u64());
                    case type_bool: return stack->push_bool(stack->pop_u64());
                    case type_u8: return stack->push_u8(stack->pop_u64());
                    case type_u16: return stack->push_u16(stack->pop_u64());
                    case type_u32: return stack->push_u32(stack->pop_u64());
                        // case type_u64: return stack->push_u64(stack->pop_u64());
                    case type_i8: return stack->push_i8(stack->pop_u64());
                    case type_i16: return stack->push_i16(stack->pop_u64());
                    case type_i32: return stack->push_i32(stack->pop_u64());
                    case type_i64: return stack->push_i64(stack->pop_u64());
                    case type_f32: return stack->push_f32(stack->pop_u64());
                    case type_f64: return stack->push_f64(stack->pop_u64());
                    default: return INVALID_DATA_TYPE;
                }
#endif // USE_X64_OPS
            case type_i8:
                switch (to_type) {
                    case type_pointer: return stack->push_pointer(stack->pop_i8());
                    case type_bool: return stack->push_bool(stack->pop_i8());
                    case type_u8: return stack->push_u8(stack->pop_i8());
                    case type_u16: return stack->push_u16(stack->pop_i8());
                    case type_u32: return stack->push_u32(stack->pop_i8());
                        // case type_i8: return stack->push_i8(stack->pop_i8());
                    case type_i16: return stack->push_i16(stack->pop_i8());
                    case type_i32: return stack->push_i32(stack->pop_i8());
                    case type_f32: return stack->push_f32(stack->pop_i8());
#ifdef USE_X64_OPS
                    case type_u64: return stack->push_u64(stack->pop_i8());
                    case type_i64: return stack->push_i64(stack->pop_i8());
                    case type_f64: return stack->push_f64(stack->pop_i8());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
            case type_i16:
                switch (to_type) {
                    case type_pointer: return stack->push_pointer(stack->pop_i16());
                    case type_bool: return stack->push_bool(stack->pop_i16());
                    case type_u8: return stack->push_u8(stack->pop_i16());
                    case type_u16: return stack->push_u16(stack->pop_i16());
                    case type_u32: return stack->push_u32(stack->pop_i16());
                    case type_i8: return stack->push_i8(stack->pop_i16());
                        // case type_i16: return stack->push_i16(stack->pop_i16());
                    case type_i32: return stack->push_i32(stack->pop_i16());
                    case type_f32: return stack->push_f32(stack->pop_i16());
#ifdef USE_X64_OPS
                    case type_u64: return stack->push_u64(stack->pop_i16());
                    case type_i64: return stack->push_i64(stack->pop_i16());
                    case type_f64: return stack->push_f64(stack->pop_i16());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
            case type_i32:
                switch (to_type) {
                    case type_pointer: return stack->push_pointer(stack->pop_i32());
                    case type_bool: return stack->push_bool(stack->pop_i32());
                    case type_u8: return stack->push_u8(stack->pop_i32());
                    case type_u16: return stack->push_u16(stack->pop_i32());
                    case type_u32: return stack->push_u32(stack->pop_i32());
                    case type_i8: return stack->push_i8(stack->pop_i32());
                    case type_i16: return stack->push_i16(stack->pop_i32());
                        // case type_i32: return stack->push_i32(stack->pop_i32());
                    case type_f32: return stack->push_f32(stack->pop_i32());
#ifdef USE_X64_OPS
                    case type_u64: return stack->push_u64(stack->pop_i32());
                    case type_i64: return stack->push_i64(stack->pop_i32());
                    case type_f64: return stack->push_f64(stack->pop_i32());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
#ifdef USE_X64_OPS
            case type_i64:
                switch (to_type) {
                    case type_pointer: return stack->push_pointer(stack->pop_i64());
                    case type_bool: return stack->push_bool(stack->pop_i64());
                    case type_u8: return stack->push_u8(stack->pop_i64());
                    case type_u16: return stack->push_u16(stack->pop_i64());
                    case type_u32: return stack->push_u32(stack->pop_i64());
                    case type_u64: return stack->push_u64(stack->pop_i64());
                    case type_i8: return stack->push_i8(stack->pop_i64());
                    case type_i16: return stack->push_i16(stack->pop_i64());
                    case type_i32: return stack->push_i32(stack->pop_i64());
                        // case type_i64: return stack->push_i64(stack->pop_i64());
                    case type_f32: return stack->push_f32(stack->pop_i64());
                    case type_f64: return stack->push_f64(stack->pop_i64());
                    default: return INVALID_DATA_TYPE;
                }
#endif // USE_X64_OPS
            case type_f32:
                switch (to_type) {
                    case type_pointer: return stack->push_pointer(stack->pop_f32());
                    case type_bool: return stack->push_bool((int) stack->pop_f32());
                    case type_u8: return stack->push_u8((int) stack->pop_f32());
                    case type_u16: return stack->push_u16(stack->pop_f32());
                    case type_u32: return stack->push_u32(stack->pop_f32());
                    case type_i8: return stack->push_i8(stack->pop_f32());
                    case type_i16: return stack->push_i16(stack->pop_f32());
                    case type_i32: return stack->push_i32(stack->pop_f32());
                        // case type_f32: return stack->push_f32(stack->pop_f32());
#ifdef USE_X64_OPS
                    case type_u64: return stack->push_u64(stack->pop_f32());
                    case type_i64: return stack->push_i64(stack->pop_f32());
                    case type_f64: return stack->push_f64(stack->pop_f32());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
#ifdef USE_X64_OPS
            case type_f64:
                switch (to_type) {
                    case type_pointer: return stack->push_pointer(stack->pop_f64());
                    case type_bool: return stack->push_bool((int) stack->pop_f64());
                    case type_u8: return stack->push_u8((int) stack->pop_f64());
                    case type_u16: return stack->push_u16(stack->pop_f64());
                    case type_u32: return stack->push_u32(stack->pop_f64());
                    case type_i8: return stack->push_i8(stack->pop_f64());
                    case type_i16: return stack->push_i16(stack->pop_f64());
                    case type_i32: return stack->push_i32(stack->pop_f64());
                    case type_u64: return stack->push_u64(stack->pop_f64());
                    case type_i64: return stack->push_i64(stack->pop_f64());
                    case type_f32: return stack->push_f32(stack->pop_f64());
                        // case type_f64: return stack->push_f64(stack->pop_f64());
                    default: return INVALID_DATA_TYPE;
                }
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }


    // Pop the pointer from the stack and push the value from the memory to the stack
    RuntimeError LOAD(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u32 size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = 0;
        extract_status = ProgramExtract.type_u8(program, program_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        RuntimeError status = stack->load_from_memory_to_stack(data_type);
        return status;
    }

    // (1.) Pop the value from the stack, (2.) pop the pointer from the stack, (3.) put the value to the memory
    RuntimeError MOVE(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u32 size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = 0;
        extract_status = ProgramExtract.type_u8(program, program_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        RuntimeError status = stack->store_from_stack_to_memory(data_type);
        return status;
    }

    // (1.) Pop the value from the stack, (2.) pop the pointer from the stack, (3.) put the value to the memory, (4.) push the value back to the stack
    RuntimeError MOVE_COPY(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u32 size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = 0;
        extract_status = ProgramExtract.type_u8(program, program_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        RuntimeError status = stack->store_from_stack_to_memory(data_type, true);
        return status;
    }


    // Duplicate the value on top of the stack
    RuntimeError COPY(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u32 size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = 0;
        extract_status = ProgramExtract.type_u8(program, program_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        switch (data_type) {
            case type_pointer: {
                RuntimeError e = STATUS_SUCCESS;
                MY_PTR_t value = stack->peek_pointer();
                value = reverse_byte_order(value);
                e = stack->push_pointer(value);
                return e;
            }
            case type_bool: return stack->push_bool(stack->peek_bool());
            case type_i8:
            case type_u8: return stack->push_u8(stack->peek_u8());
            case type_i16:
            case type_u16: return stack->push_u16(stack->peek_u16());
            case type_i32:
            case type_u32:
            case type_f32: return stack->push_u32(stack->peek_u32());
#ifdef USE_X64_OPS
            case type_u64:
            case type_i64:
            case type_f64: return stack->push_u64(stack->peek_u64());
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }


    template <typename A, typename B> RuntimeError _SWAP_CUSTOM(RuntimeStack* stack) {
        if (sizeof(A) + sizeof(B) > stack->size()) return STACK_UNDERFLOW;
        B b = stack->pop_custom<B>();
        A a = stack->pop_custom<A>();
        a = reverse_byte_order(a);
        b = reverse_byte_order(b);
        stack->push_custom<B>(b);
        stack->push_custom<A>(a);
        return STATUS_SUCCESS;
    }

    // Swap the two values on top of the stack
    RuntimeError SWAP(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u32 size = 2;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 type_A = 0, type_B = 0;
        RuntimeError status;
        status = ProgramExtract.type_u8(program, program_size, index, &type_A); if (status != STATUS_SUCCESS) return status;
        status = ProgramExtract.type_u8(program, program_size, index, &type_B); if (status != STATUS_SUCCESS) return status;
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
    RuntimeError DROP(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u32 size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = 0;
        extract_status = ProgramExtract.type_u8(program, program_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        switch (data_type) {
            case type_pointer: stack->pop_pointer(); break;
            case type_bool: stack->pop_bool(); break;
            case type_u8: stack->pop_u8(); break;
            case type_u16: stack->pop_u16(); break;
            case type_u32: stack->pop_u32(); break;
            case type_i8: stack->pop_i8(); break;
            case type_i16: stack->pop_i16(); break;
            case type_i32: stack->pop_i32(); break;
            case type_f32: stack->pop_f32(); break;
#ifdef USE_X64_OPS
            case type_u64: stack->pop_u64(); break;
            case type_i64: stack->pop_i64(); break;
            case type_f64: stack->pop_f64(); break;
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
        return STATUS_SUCCESS;
    }
    // Clear the stack
    RuntimeError CLEAR(RuntimeStack* stack) {
        stack->clear();
        return STATUS_SUCCESS;
    }
}