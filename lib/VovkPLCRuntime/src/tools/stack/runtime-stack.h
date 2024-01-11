// runtime-stack.h - 1.0.0 - 2022-12-11
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

#include "../runtime-tools.h"
#include "stack-struct.h"
#include "../runtime-instructions.h"

class RuntimeStack {
public:
#ifdef __WASM__
    Stack<u8>* stack = nullptr;
    Stack<u16>* call_stack = nullptr;
#else
    Stack<u8>* stack = new Stack<u8>;
    Stack<u16>* call_stack = new Stack<u16>;
#endif // __WASM__
    u32 max_size = 0;
    u32 max_call_stack_size = 0;

    Stack<u8>* memory = nullptr; // PLC memory to manipulate

    // Create a stack with a maximum size
    RuntimeStack(u32 max_size = 0, u32 call_stack_size = 10, u32 memory_size = 4);
    ~RuntimeStack();

    void format(u32 max_size = 0, u32 call_stack_size = 10, u32 memory_size = 4);

    int print();
    void println();

    RuntimeError pushCall(u32 return_address);

    u16 popCall();

    // Push an u8 value to the stack
    RuntimeError push(u8 value);
    // Pop an u8 value from the stack
    u8 pop();
    // Pop an u8 value from the stack
    void pop(int size);
    // Peek the top u8 value from the stack
    u8 peek(int depth);
    
    template <typename T> bool push_custom(T value);
    template <typename T> T pop_custom();
    template <typename T> T peek_custom();

    // Push a pointer to the stack
    RuntimeError push_pointer(MY_PTR_t value);
    // Pop a pointer from the stack
    MY_PTR_t pop_pointer();
    // Peek the top pointer from the stack
    MY_PTR_t peek_pointer();

    // Push a boolean value to the stack
    RuntimeError push_bool(bool value);
    // Pop a boolean value from the stack
    bool pop_bool();
    // Peek the top boolean value from the stack
    bool peek_bool();


    // Push an u8 value to the stack
    RuntimeError push_u8(u8 value);
    // Pop an u8 value from the stack
    u8 pop_u8();
    // Peek the top u8 value from the stack
    u8 peek_u8();

    // Push an u16 value to the stack
    RuntimeError push_u16(u16 value);
    // Pop an u16 value from the stack
    u16 pop_u16();
    // Peek the top u16 value from the stack
    u16 peek_u16();

    // Push an u32 value to the stack
    RuntimeError push_u32(u32 value);
    // Pop an u32 value from the stack
    u32 pop_u32();
    // Peek the top u32 value from the stack
    u32 peek_u32();

#ifdef USE_X64_OPS
    // Push an u64 value to the stack
    RuntimeError push_u64(u64 value);
    // Pop an u64 value from the stack
    u64 pop_u64();
    // Peek the top u64 value from the stack
    u64 peek_u64();
#endif // USE_X64_OPS

    RuntimeError push_i8(i8 value);
    RuntimeError push_i16(i16 value);
    RuntimeError push_i32(i32 value);
#ifdef USE_X64_OPS
    RuntimeError push_i64(i64 value);
#endif // USE_X64_OPS
    RuntimeError push_f32(f32 value);
#ifdef USE_X64_OPS
    RuntimeError push_f64(f64 value);
#endif // USE_X64_OPS
    i8 pop_i8();
    i16 pop_i16();
    i32 pop_i32();
    f32 pop_f32();
#ifdef USE_X64_OPS
    i64 pop_i64();
    f64 pop_f64();
#endif // USE_X64_OPS

    i8 peek_i8();
    i16 peek_i16();
    i32 peek_i32();
    f32 peek_f32();
#ifdef USE_X64_OPS
    i64 peek_i64();
    f64 peek_f64();
#endif // USE_X64_OPS

    RuntimeError load_from_memory_to_stack(u8 data_type);
    RuntimeError store_from_stack_to_memory(u8 data_type, bool copy = false);
    template <typename T> RuntimeError load_from_memory_to_stack();
    template <typename T> RuntimeError store_from_stack_to_memory(bool copy = false);

    u32 size();
    void clear();
};

#include "runtime-stack-impl.h"