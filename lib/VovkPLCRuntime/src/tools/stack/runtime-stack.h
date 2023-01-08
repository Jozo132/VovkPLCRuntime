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

class RuntimeStack {
public:
    Stack<uint8_t>* stack = nullptr;
    Stack<uint16_t>* call_stack = nullptr;
    uint16_t max_size = 0;
    uint16_t max_call_stack_size = 0;

    Stack<uint8_t>* memory = nullptr; // PLC memory to manipulate

    // Create a stack with a maximum size
    RuntimeStack(uint16_t max_size, uint16_t call_stack_size = 10, uint16_t memory_size = 4) {
        this->max_size = max_size;
        this->max_call_stack_size = call_stack_size;
        if (stack == nullptr) stack = new Stack<uint8_t>;
        if (call_stack == nullptr) call_stack = new Stack<uint16_t>;
        if (memory == nullptr) memory = new Stack<uint8_t>;
        stack->format(max_size);
        call_stack->format(call_stack_size);
        memory->format(memory_size);
    }
    ~RuntimeStack() {
        if (stack != nullptr) delete stack;
        if (call_stack != nullptr) delete call_stack;
        if (memory != nullptr) delete memory;
        stack = nullptr;
        call_stack = nullptr;
        memory = nullptr;
    }

    int print() { return stack->print(); }
    void println() { stack->println(); }

    RuntimeError pushCall(uint16_t return_address) {
        if (call_stack->size() >= max_call_stack_size) return STACK_OVERFLOW;
        call_stack->push(return_address);
        return STATUS_SUCCESS;
    }

    uint16_t popCall() {
        if (call_stack->size() == 0) return STACK_UNDERFLOW;
        return call_stack->pop();
    }

    // Push an uint8_t value to the stack
    RuntimeError push(uint8_t value) {
        if (stack->size() >= max_size) return STACK_OVERFLOW;
        stack->push(value);
        return STATUS_SUCCESS;
    }
    // Pop an uint8_t value from the stack
    uint8_t pop() { return stack->pop(); }
    // Pop an uint8_t value from the stack
    void pop(int size) { stack->pop(size); }
    // Peek the top uint8_t value from the stack
    uint8_t peek(int depth) { return stack->peek(depth); }

    // Push a boolean value to the stack
    RuntimeError push_bool(bool value) {
        if (stack->size() >= max_size) return STACK_OVERFLOW;
        stack->push(value);
        return STATUS_SUCCESS;
    }
    // Pop a boolean value from the stack
    bool pop_bool() {
        bool value = stack->pop();
        return value;
    }
    // Peek the top boolean value from the stack
    bool peek_bool() { return stack->peek(); }


    // Push an uint8_t value to the stack
    RuntimeError push_uint8_t(uint8_t value) {
        if (stack->size() + 1 > max_size) return STACK_OVERFLOW;
        stack->push(value);
        return STATUS_SUCCESS;
    }
    // Pop an uint8_t value from the stack
    uint8_t pop_uint8_t() { return stack->pop(); }
    // Peek the top uint8_t value from the stack
    uint8_t peek_uint8_t() { return stack->peek(); }

    // Push an uint16_t value to the stack
    RuntimeError push_uint16_t(uint16_t value) {
        if (stack->size() + 2 > max_size) return STACK_OVERFLOW;
        stack->push(value >> 8);
        stack->push(value & 0xFF);
        return STATUS_SUCCESS;
    }
    // Pop an uint16_t value from the stack
    uint16_t pop_uint16_t() {
        uint16_t b = stack->pop();
        uint16_t a = stack->pop();
        return (a << 8) | b;
    }
    // Peek the top uint16_t value from the stack
    uint16_t peek_uint16_t() {
        uint16_t b = stack->peek(0);
        uint16_t a = stack->peek(1);
        return (a << 8) | b;
    }

    // Push an uint32_t value to the stack
    RuntimeError push_uint32_t(uint32_t value) {
        if (stack->size() + 4 > max_size) return STACK_OVERFLOW;
        stack->push(value >> 24);
        stack->push((value >> 16) & 0xFF);
        stack->push((value >> 8) & 0xFF);
        stack->push(value & 0xFF);
        return STATUS_SUCCESS;
    }
    // Pop an uint32_t value from the stack
    uint32_t pop_uint32_t() {
        uint32_t d = stack->pop();
        uint32_t c = stack->pop();
        uint32_t b = stack->pop();
        uint32_t a = stack->pop();
        return (a << 24) | (b << 16) | (c << 8) | d;
    }
    // Peek the top uint32_t value from the stack
    uint32_t peek_uint32_t() {
        uint32_t d = stack->peek(0);
        uint32_t c = stack->peek(1);
        uint32_t b = stack->peek(2);
        uint32_t a = stack->peek(3);
        return (a << 24) | (b << 16) | (c << 8) | d;
    }

    // Push an uint64_t value to the stack
    RuntimeError push_uint64_t(uint64_t value) {
        if (stack->size() + 8 > max_size) return STACK_OVERFLOW;
        stack->push(value >> 56);
        stack->push((value >> 48) & 0xFF);
        stack->push((value >> 40) & 0xFF);
        stack->push((value >> 32) & 0xFF);
        stack->push((value >> 24) & 0xFF);
        stack->push((value >> 16) & 0xFF);
        stack->push((value >> 8) & 0xFF);
        stack->push(value & 0xFF);
        return STATUS_SUCCESS;
    }
    // Pop an uint64_t value from the stack
    uint64_t pop_uint64_t() {
        uint64_t h = stack->pop();
        uint64_t g = stack->pop();
        uint64_t f = stack->pop();
        uint64_t e = stack->pop();
        uint64_t d = stack->pop();
        uint64_t c = stack->pop();
        uint64_t b = stack->pop();
        uint64_t a = stack->pop();
        return (a << 56) | (b << 48) | (c << 40) | (d << 32) | (e << 24) | (f << 16) | (g << 8) | h;
    }
    // Peek the top uint64_t value from the stack
    uint64_t peek_uint64_t() {
        uint64_t h = stack->peek(0);
        uint64_t g = stack->peek(1);
        uint64_t f = stack->peek(2);
        uint64_t e = stack->peek(3);
        uint64_t d = stack->peek(4);
        uint64_t c = stack->peek(5);
        uint64_t b = stack->peek(6);
        uint64_t a = stack->peek(7);
        return (a << 56) | (b << 48) | (c << 40) | (d << 32) | (e << 24) | (f << 16) | (g << 8) | h;
    }

    RuntimeError push_int8_t(int8_t value) { return push_uint8_t((uint8_t) value); }
    RuntimeError push_int16_t(int16_t value) { return push_uint16_t((uint16_t) value); }
    RuntimeError push_int32_t(int32_t value) { return push_uint32_t((uint32_t) value); }
    RuntimeError push_int64_t(int64_t value) { return push_uint64_t((uint64_t) value); }
    RuntimeError push_float(float value) { uint32_t_to_float cvt; cvt.type_float = value; return push_uint32_t(cvt.type_uint32_t); }
    RuntimeError push_double(double value) { uint64_t_to_double cvt; cvt.type_double = value; return push_uint64_t(cvt.type_uint64_t); }

    int8_t pop_int8_t() { return (int8_t) pop_uint8_t(); }
    int16_t pop_int16_t() { return (int16_t) pop_uint16_t(); }
    int32_t pop_int32_t() { return (int32_t) pop_uint32_t(); }
    int64_t pop_int64_t() { return (int64_t) pop_uint64_t(); }
    float pop_float() { uint32_t_to_float cvt; cvt.type_uint32_t = pop_uint32_t(); return cvt.type_float; }
    double pop_double() { uint64_t_to_double cvt; cvt.type_uint64_t = pop_uint64_t(); return cvt.type_double; }
    void clear() { while (!stack->empty()) stack->pop(); while (!call_stack->empty()) call_stack->pop(); }

    int8_t peek_int8_t() { return (int8_t) peek_uint8_t(); }
    int16_t peek_int16_t() { return (int16_t) peek_uint16_t(); }
    int32_t peek_int32_t() { return (int32_t) peek_uint32_t(); }
    int64_t peek_int64_t() { return (int64_t) peek_uint64_t(); }
    float peek_float() { uint32_t_to_float cvt; cvt.type_uint32_t = peek_uint32_t(); return cvt.type_float; }
    double peek_double() { uint64_t_to_double cvt; cvt.type_uint64_t = peek_uint64_t(); return cvt.type_double; }

    uint16_t size() { return stack->size(); }
};


#define EXTRACT_TYPE_8(type)                                                                                    \
    static RuntimeError type_##type(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, type* value) {  \
        uint16_t size = sizeof(type);                                                                           \
        if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;                                         \
        *value = bytecode[index];                                                                               \
        index += size;                                                                                          \
        return STATUS_SUCCESS;                                                                                  \
    }

#define EXTRACT_TYPE_16(type)                                                                                   \
    static RuntimeError type_##type(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, type* value) {  \
        uint16_t size = sizeof(type);                                                                           \
        if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;                                         \
        *value = ((uint16_t) bytecode[index] << 8) | bytecode[index + 1];                                        \
        index += size;                                                                                          \
        return STATUS_SUCCESS;                                                                                  \
    }

#define EXTRACT_TYPE_32(type)                                                                                                                               \
    static RuntimeError type_##type(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, type* value) {                                              \
        uint16_t size = sizeof(type);                                                                                                                       \
        if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;                                                                                     \
        *value = ((type) bytecode[index] << 24) | ((type) bytecode[index + 1] << 16) | ((type) bytecode[index + 2] << 8) | ((type) bytecode[index + 3]);     \
        index += size;                                                                                                                                      \
        return STATUS_SUCCESS;                                                                                                                              \
    }

#define EXTRACT_TYPE_32_CVT(type)                                                                                                                                                     \
    static RuntimeError type_##type(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, type* value) {                                                                        \
        uint16_t size = sizeof(type);                                                                                                                                                 \
        if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;                                                                                                               \
        CVT_32(type) cvt;                                                                                                                                                             \
        cvt.type_uint32_t = ((uint32_t) bytecode[index] << 24) | ((uint32_t) bytecode[index + 1] << 16) | ((uint32_t) bytecode[index + 2] << 8) | ((uint32_t) bytecode[index + 3]);   \
        *value = cvt.type_##type;                                                                                                                                                      \
        index += size;                                                                                                                                                                \
        return STATUS_SUCCESS;                                                                                                                                                        \
    }

#define EXTRACT_TYPE_64(type)                                                                                                                                       \
    static RuntimeError type_##type(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, type* value) {                                                      \
        uint16_t size = sizeof(type);                                                                                                                               \
        if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;                                                                                             \
        *value = ((type) bytecode[index] << 56) | ((type) bytecode[index + 1] << 48) | ((type) bytecode[index + 2] << 40) | ((type) bytecode[index + 3] << 32) |     \
                ((type) bytecode[index + 4] << 24) | ((type) bytecode[index + 5] << 16) | ((type) bytecode[index + 6] << 8) | ((type) bytecode[index + 7]);         \
        index += size;                                                                                                                                              \
        return STATUS_SUCCESS;                                                                                                                                      \
    }

#define EXTRACT_TYPE_64_CVT(type)                                                                                                                                                           \
    static RuntimeError type_##type(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, type* value) {                                                                              \
        uint16_t size = sizeof(type);                                                                                                                                                       \
        if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;                                                                                                                     \
        CVT_64(type) cvt;                                                                                                                                                                   \
        cvt.type_uint64_t = ((uint64_t) bytecode[index] << 56) | ((uint64_t) bytecode[index + 1] << 48) | ((uint64_t) bytecode[index + 2] << 40) | ((uint64_t) bytecode[index + 3] << 32) | \
                            ((uint64_t) bytecode[index + 4] << 24) | ((uint64_t) bytecode[index + 5] << 16) | ((uint64_t) bytecode[index + 6] << 8) | ((uint64_t) bytecode[index + 7]);     \
        *value = cvt.type_##type;                                                                                                                                                            \
        index += size;                                                                                                                                                                      \
        return STATUS_SUCCESS;                                                                                                                                                              \
    }

struct Extract_t {
    EXTRACT_TYPE_8(bool);
    EXTRACT_TYPE_8(uint8_t);
    EXTRACT_TYPE_8(int8_t);

    EXTRACT_TYPE_16(uint16_t);
    EXTRACT_TYPE_16(int16_t);

    EXTRACT_TYPE_32(uint32_t);
    EXTRACT_TYPE_32(int32_t);
    EXTRACT_TYPE_32_CVT(float);

    EXTRACT_TYPE_64(uint64_t);
    EXTRACT_TYPE_64(int64_t);
    EXTRACT_TYPE_64_CVT(double);
} ProgramExtract;

RuntimeError extract_status;