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

#include "runtime-stack.h"

RuntimeStack::RuntimeStack(u32 max_size, u32 call_stack_size, u32 memory_size) {
    this->format(max_size, call_stack_size, memory_size);
}
RuntimeStack::~RuntimeStack() {
    if (stack != nullptr) delete stack;
    if (call_stack != nullptr) delete call_stack;
    if (memory != nullptr) delete memory;
    stack = nullptr;
    call_stack = nullptr;
    memory = nullptr;
}
void RuntimeStack::format(u32 max_size, u32 call_stack_size, u32 memory_size) {
    this->max_size = max_size;
    this->max_call_stack_size = call_stack_size;
    if (stack == nullptr) stack = new Stack<u8>;
    if (call_stack == nullptr) call_stack = new Stack<u16>;
    if (memory == nullptr) memory = new Stack<u8>;
    stack->format(max_size);
    call_stack->format(call_stack_size);
    memory->format(memory_size);
}
int RuntimeStack::print() { return stack->print(); }
void RuntimeStack::println() { stack->println(); }
RuntimeError RuntimeStack::pushCall(u32 return_address) {
    if (call_stack->size() >= max_call_stack_size) return STACK_OVERFLOW;
    call_stack->push(return_address);
    return STATUS_SUCCESS;
}
u16 RuntimeStack::popCall() {
    if (call_stack->size() == 0) return STACK_UNDERFLOW;
    return call_stack->pop();
}
// Push an u8 value to the stack
RuntimeError RuntimeStack::push(u8 value) {
    if (stack->size() >= max_size) return STACK_OVERFLOW;
    stack->push(value);
    return STATUS_SUCCESS;
}
// Pop an u8 value from the stack
u8 RuntimeStack::pop() { return stack->pop(); }
// Pop an u8 value from the stack
void RuntimeStack::pop(int size) { stack->pop(size); }
// Peek the top u8 value from the stack
u8 RuntimeStack::peek(int depth) { return stack->peek(depth); }

template <typename T> bool RuntimeStack::push_custom(T value) {
    if (stack->size() + sizeof(T) > max_size) return true;
    for (u32 i = 0; i < sizeof(T); i++) {
        // stack->push((value >> (8 * (sizeof(T) - i - 1))) & 0xFF);
        stack->push((value >> (8 * i)) & 0xFF);
    }
    return false;
}

template <typename T> T RuntimeStack::pop_custom() {
    T value = 0;
    // for (u32 i = 0; i < sizeof(T); i++) value |= (stack->pop() << (8 * (sizeof(T) - i - 1)));
    for (u32 i = 0; i < sizeof(T); i++) value |= (stack->pop() << (8 * i));
    return value;
}

template <typename T> T RuntimeStack::peek_custom() {
    T value = 0;
    for (u32 i = 0; i < sizeof(T); i++) value |= (stack->peek(i) << (8 * i));
    return value;
}

// Push a pointer to the stack
RuntimeError RuntimeStack::push_pointer(MY_PTR_t value) {
    if (stack->size() + sizeof(MY_PTR_t) > max_size) return STACK_OVERFLOW;
    return push_custom<MY_PTR_t>(value) ? STACK_OVERFLOW : STATUS_SUCCESS;
}

// Pop a pointer from the stack
MY_PTR_t RuntimeStack::pop_pointer() { return pop_custom<MY_PTR_t>(); }

// Peek the top pointer from the stack
MY_PTR_t RuntimeStack::peek_pointer() { return peek_custom<MY_PTR_t>(); }

// Push a boolean value to the stack
RuntimeError RuntimeStack::push_bool(bool value) {
    if (stack->size() >= max_size) return STACK_OVERFLOW;
    stack->push(value);
    return STATUS_SUCCESS;
}
// Pop a boolean value from the stack
bool RuntimeStack::pop_bool() {
    bool value = stack->pop();
    return value;
}
// Peek the top boolean value from the stack
bool RuntimeStack::peek_bool() { return stack->peek(); }

// Push an u8 value to the stack
RuntimeError RuntimeStack::push_u8(u8 value) {
    if (stack->size() + 1 > max_size) return STACK_OVERFLOW;
    stack->push(value);
    return STATUS_SUCCESS;
}
// Pop an u8 value from the stack
u8 RuntimeStack::pop_u8() { return stack->pop(); }
// Peek the top u8 value from the stack
u8 RuntimeStack::peek_u8() { return stack->peek(); }

// Push an u16 value to the stack
RuntimeError RuntimeStack::push_u16(u16 value) {
    if (stack->size() + 2 > max_size) return STACK_OVERFLOW;
    stack->push(value >> 8);
    stack->push(value & 0xFF);
    return STATUS_SUCCESS;
}
// Pop an u16 value from the stack
u16 RuntimeStack::pop_u16() {
    u16 b = stack->pop();
    u16 a = stack->pop();
    return (a << 8) | b;
}
// Peek the top u16 value from the stack
u16 RuntimeStack::peek_u16() {
    u16 b = stack->peek(0);
    u16 a = stack->peek(1);
    return (a << 8) | b;
}

// Push an u32 value to the stack
RuntimeError RuntimeStack::push_u32(u32 value) {
    if (stack->size() + 4 > max_size) return STACK_OVERFLOW;
    stack->push(value >> 24);
    stack->push((value >> 16) & 0xFF);
    stack->push((value >> 8) & 0xFF);
    stack->push(value & 0xFF);
    return STATUS_SUCCESS;
}
// Pop an u32 value from the stack
u32 RuntimeStack::pop_u32() {
    u32 d = stack->pop();
    u32 c = stack->pop();
    u32 b = stack->pop();
    u32 a = stack->pop();
    return (a << 24) | (b << 16) | (c << 8) | d;
}
// Peek the top u32 value from the stack
u32 RuntimeStack::peek_u32() {
    u32 d = stack->peek(0);
    u32 c = stack->peek(1);
    u32 b = stack->peek(2);
    u32 a = stack->peek(3);
    return (a << 24) | (b << 16) | (c << 8) | d;
}

#ifdef USE_X64_OPS
// Push an u64 value to the stack
RuntimeError RuntimeStack::push_u64(u64 value) {
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
// Pop an u64 value from the stack
u64 RuntimeStack::pop_u64() {
    u64 h = stack->pop();
    u64 g = stack->pop();
    u64 f = stack->pop();
    u64 e = stack->pop();
    u64 d = stack->pop();
    u64 c = stack->pop();
    u64 b = stack->pop();
    u64 a = stack->pop();
    return (a << 56) | (b << 48) | (c << 40) | (d << 32) | (e << 24) | (f << 16) | (g << 8) | h;
}
// Peek the top u64 value from the stack
u64 RuntimeStack::peek_u64() {
    u64 h = stack->peek(0);
    u64 g = stack->peek(1);
    u64 f = stack->peek(2);
    u64 e = stack->peek(3);
    u64 d = stack->peek(4);
    u64 c = stack->peek(5);
    u64 b = stack->peek(6);
    u64 a = stack->peek(7);
    return (a << 56) | (b << 48) | (c << 40) | (d << 32) | (e << 24) | (f << 16) | (g << 8) | h;
}
#endif // USE_X64_OPS

RuntimeError RuntimeStack::push_i8(i8 value) { return push_u8((u8) value); }
RuntimeError RuntimeStack::push_i16(i16 value) { return push_u16((u16) value); }
RuntimeError RuntimeStack::push_i32(i32 value) { return push_u32((u32) value); }
RuntimeError RuntimeStack::push_f32(f32 value) { u32_to_f32 cvt; cvt.type_f32 = value; return push_u32(cvt.type_u32); }
#ifdef USE_X64_OPS
RuntimeError RuntimeStack::push_i64(i64 value) { return push_u64((u64) value); }
RuntimeError RuntimeStack::push_f64(f64 value) { u64_to_f64 cvt; cvt.type_f64 = value; return push_u64(cvt.type_u64); }
#endif // USE_X64_OPS

i8 RuntimeStack::pop_i8() { return (i8) pop_u8(); }
i16 RuntimeStack::pop_i16() { return (i16) pop_u16(); }
i32 RuntimeStack::pop_i32() { return (i32) pop_u32(); }
f32 RuntimeStack::pop_f32() { u32_to_f32 cvt; cvt.type_u32 = pop_u32(); return cvt.type_f32; }
#ifdef USE_X64_OPS
i64 RuntimeStack::pop_i64() { return (i64) pop_u64(); }
f64 RuntimeStack::pop_f64() { u64_to_f64 cvt; cvt.type_u64 = pop_u64(); return cvt.type_f64; }
#endif // USE_X64_OPS

i8 RuntimeStack::peek_i8() { return (i8) peek_u8(); }
i16 RuntimeStack::peek_i16() { return (i16) peek_u16(); }
i32 RuntimeStack::peek_i32() { return (i32) peek_u32(); }
f32 RuntimeStack::peek_f32() { u32_to_f32 cvt; cvt.type_u32 = peek_u32(); return cvt.type_f32; }
#ifdef USE_X64_OPS
i64 RuntimeStack::peek_i64() { return (i64) peek_u64(); }
f64 RuntimeStack::peek_f64() { u64_to_f64 cvt; cvt.type_u64 = peek_u64(); return cvt.type_f64; }
#endif // USE_X64_OPS

u32 RuntimeStack::size() { return stack->size(); }
void RuntimeStack::clear() { while (!stack->empty()) stack->pop(); while (!call_stack->empty()) call_stack->pop(); }




template <typename T> RuntimeError RuntimeStack::load_from_memory_to_stack() {
    if (stack->size() < sizeof(MY_PTR_t)) return  RuntimeError::STACK_UNDERFLOW;
    MY_PTR_t address = pop_pointer();
    if (address + sizeof(T) > memory->size()) return  RuntimeError::INVALID_MEMORY_ADDRESS;
    T value = 0;
    bool error = memory->readArea(address, reinterpret_cast<u8*>(&value), sizeof(T));
    if (error) return RuntimeError::INVALID_MEMORY_ADDRESS;
    error = push_custom(value);
    if (error) return RuntimeError::STACK_OVERFLOW;
    return  RuntimeError::STATUS_SUCCESS;
}

template <typename T> RuntimeError RuntimeStack::store_from_stack_to_memory(bool copy) {
    if (stack->size() < (sizeof(T) + sizeof(MY_PTR_t))) return  RuntimeError::STACK_UNDERFLOW;
    T value = pop_custom<T>();
    MY_PTR_t address = pop_pointer();
    if ((address + sizeof(T)) > memory->size()) return  RuntimeError::INVALID_MEMORY_ADDRESS;
    bool error = memory->writeArea(address, reinterpret_cast<u8*>(&value), sizeof(T));
    if (error) return  RuntimeError::INVALID_MEMORY_ADDRESS;
    if (copy) error = push_custom(value);
    return error ? RuntimeError::STACK_OVERFLOW : RuntimeError::STATUS_SUCCESS;
}

RuntimeError RuntimeStack::load_from_memory_to_stack(u8 data_type) {
    switch (data_type) {
        case type_pointer: return load_from_memory_to_stack<MY_PTR_t>();
        case type_bool:
        case type_u8:
        case type_i8: return load_from_memory_to_stack<u8>();
        case type_u16:
        case type_i16: return load_from_memory_to_stack<u16>();
        case type_u32:
        case type_i32:
        case type_f32: return load_from_memory_to_stack<u32>();
#ifdef USE_X64_OPS
        case type_u64:
        case type_i64:
        case type_f64: return load_from_memory_to_stack<u64>();
#endif // USE_X64_OPS
        default: return  RuntimeError::INVALID_DATA_TYPE;
    }
    return RuntimeError::INVALID_DATA_TYPE;
}

RuntimeError RuntimeStack::store_from_stack_to_memory(u8 data_type, bool copy) {
    switch (data_type) {
        case type_pointer: return store_from_stack_to_memory<MY_PTR_t>(copy);
        case type_bool:
        case type_u8:
        case type_i8: return store_from_stack_to_memory<u8>(copy);
        case type_u16:
        case type_i16: return store_from_stack_to_memory<u16>(copy);
        case type_u32:
        case type_i32:
        case type_f32: return store_from_stack_to_memory<u32>(copy);
#ifdef USE_X64_OPS
        case type_u64:
        case type_i64:
        case type_f64: return store_from_stack_to_memory<u64>(copy);
#endif // USE_X64_OPS
        default: return RuntimeError::INVALID_DATA_TYPE;
    }
    return RuntimeError::INVALID_DATA_TYPE;
}