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
    Stack<uint8_t>* stack = nullptr;
    Stack<uint16_t>* call_stack = nullptr;
#else
    Stack<uint8_t>* stack = new Stack<uint8_t>;
    Stack<uint16_t>* call_stack = new Stack<uint16_t>;
#endif // __WASM__
    uint32_t max_size = 0;
    uint32_t max_call_stack_size = 0;

    Stack<uint8_t>* memory = nullptr; // PLC memory to manipulate

    // Create a stack with a maximum size
    RuntimeStack(uint32_t max_size = 0, uint32_t call_stack_size = 10, uint32_t memory_size = 4);
    ~RuntimeStack();

    void format(uint32_t max_size = 0, uint32_t call_stack_size = 10, uint32_t memory_size = 4);

    int print();
    void println();

    RuntimeError pushCall(uint32_t return_address);

    uint16_t popCall();

    // Push an uint8_t value to the stack
    RuntimeError push(uint8_t value);
    // Pop an uint8_t value from the stack
    uint8_t pop();
    // Pop an uint8_t value from the stack
    void pop(int size);
    // Peek the top uint8_t value from the stack
    uint8_t peek(int depth);

    // Push a boolean value to the stack
    RuntimeError push_bool(bool value);
    // Pop a boolean value from the stack
    bool pop_bool();
    // Peek the top boolean value from the stack
    bool peek_bool();


    // Push an uint8_t value to the stack
    RuntimeError push_uint8_t(uint8_t value);
    // Pop an uint8_t value from the stack
    uint8_t pop_uint8_t();
    // Peek the top uint8_t value from the stack
    uint8_t peek_uint8_t();

    // Push an uint16_t value to the stack
    RuntimeError push_uint16_t(uint16_t value);
    // Pop an uint16_t value from the stack
    uint16_t pop_uint16_t();
    // Peek the top uint16_t value from the stack
    uint16_t peek_uint16_t();

    // Push an uint32_t value to the stack
    RuntimeError push_uint32_t(uint32_t value);
    // Pop an uint32_t value from the stack
    uint32_t pop_uint32_t();
    // Peek the top uint32_t value from the stack
    uint32_t peek_uint32_t();

#ifdef USE_X64_OPS
    // Push an uint64_t value to the stack
    RuntimeError push_uint64_t(uint64_t value);
    // Pop an uint64_t value from the stack
    uint64_t pop_uint64_t();
    // Peek the top uint64_t value from the stack
    uint64_t peek_uint64_t();
#endif // USE_X64_OPS

    RuntimeError push_int8_t(int8_t value);
    RuntimeError push_int16_t(int16_t value);
    RuntimeError push_int32_t(int32_t value);
#ifdef USE_X64_OPS
    RuntimeError push_int64_t(int64_t value);
#endif // USE_X64_OPS
    RuntimeError push_float(float value);
#ifdef USE_X64_OPS
    RuntimeError push_double(double value);
#endif // USE_X64_OPS
    int8_t pop_int8_t();
    int16_t pop_int16_t();
    int32_t pop_int32_t();
    float pop_float();
#ifdef USE_X64_OPS
    int64_t pop_int64_t();
    double pop_double();
#endif // USE_X64_OPS

    int8_t peek_int8_t();
    int16_t peek_int16_t();
    int32_t peek_int32_t();
    float peek_float();
#ifdef USE_X64_OPS
    int64_t peek_int64_t();
    double peek_double();
#endif // USE_X64_OPS

    uint32_t size();
    void clear();
};

#include "runtime-stack-impl.h"