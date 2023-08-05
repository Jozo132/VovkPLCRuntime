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

RuntimeStack::RuntimeStack(uint32_t max_size, uint32_t call_stack_size, uint32_t memory_size) {
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
void RuntimeStack::format(uint32_t max_size, uint32_t call_stack_size, uint32_t memory_size) {
    this->max_size = max_size;
    this->max_call_stack_size = call_stack_size;
    if (stack == nullptr) stack = new Stack<uint8_t>;
    if (call_stack == nullptr) call_stack = new Stack<uint16_t>;
    if (memory == nullptr) memory = new Stack<uint8_t>;
    stack->format(max_size);
    call_stack->format(call_stack_size);
    memory->format(memory_size);
}
int RuntimeStack::print() { return stack->print(); }
void RuntimeStack::println() { stack->println(); }
RuntimeError RuntimeStack::pushCall(uint32_t return_address) {
    if (call_stack->size() >= max_call_stack_size) return STACK_OVERFLOW;
    call_stack->push(return_address);
    return STATUS_SUCCESS;
}
uint16_t RuntimeStack::popCall() {
    if (call_stack->size() == 0) return STACK_UNDERFLOW;
    return call_stack->pop();
}
// Push an uint8_t value to the stack
RuntimeError RuntimeStack::push(uint8_t value) {
    if (stack->size() >= max_size) return STACK_OVERFLOW;
    stack->push(value);
    return STATUS_SUCCESS;
}
// Pop an uint8_t value from the stack
uint8_t RuntimeStack::pop() { return stack->pop(); }
// Pop an uint8_t value from the stack
void RuntimeStack::pop(int size) { stack->pop(size); }
// Peek the top uint8_t value from the stack
uint8_t RuntimeStack::peek(int depth) { return stack->peek(depth); }

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

// Push an uint8_t value to the stack
RuntimeError RuntimeStack::push_uint8_t(uint8_t value) {
    if (stack->size() + 1 > max_size) return STACK_OVERFLOW;
    stack->push(value);
    return STATUS_SUCCESS;
}
// Pop an uint8_t value from the stack
uint8_t RuntimeStack::pop_uint8_t() { return stack->pop(); }
// Peek the top uint8_t value from the stack
uint8_t RuntimeStack::peek_uint8_t() { return stack->peek(); }

// Push an uint16_t value to the stack
RuntimeError RuntimeStack::push_uint16_t(uint16_t value) {
    if (stack->size() + 2 > max_size) return STACK_OVERFLOW;
    stack->push(value >> 8);
    stack->push(value & 0xFF);
    return STATUS_SUCCESS;
}
// Pop an uint16_t value from the stack
uint16_t RuntimeStack::pop_uint16_t() {
    uint16_t b = stack->pop();
    uint16_t a = stack->pop();
    return (a << 8) | b;
}
// Peek the top uint16_t value from the stack
uint16_t RuntimeStack::peek_uint16_t() {
    uint16_t b = stack->peek(0);
    uint16_t a = stack->peek(1);
    return (a << 8) | b;
}

// Push an uint32_t value to the stack
RuntimeError RuntimeStack::push_uint32_t(uint32_t value) {
    if (stack->size() + 4 > max_size) return STACK_OVERFLOW;
    stack->push(value >> 24);
    stack->push((value >> 16) & 0xFF);
    stack->push((value >> 8) & 0xFF);
    stack->push(value & 0xFF);
    return STATUS_SUCCESS;
}
// Pop an uint32_t value from the stack
uint32_t RuntimeStack::pop_uint32_t() {
    uint32_t d = stack->pop();
    uint32_t c = stack->pop();
    uint32_t b = stack->pop();
    uint32_t a = stack->pop();
    return (a << 24) | (b << 16) | (c << 8) | d;
}
// Peek the top uint32_t value from the stack
uint32_t RuntimeStack::peek_uint32_t() {
    uint32_t d = stack->peek(0);
    uint32_t c = stack->peek(1);
    uint32_t b = stack->peek(2);
    uint32_t a = stack->peek(3);
    return (a << 24) | (b << 16) | (c << 8) | d;
}

#ifdef USE_X64_OPS
// Push an uint64_t value to the stack
RuntimeError RuntimeStack::push_uint64_t(uint64_t value) {
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
uint64_t RuntimeStack::pop_uint64_t() {
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
uint64_t RuntimeStack::peek_uint64_t() {
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
#endif // USE_X64_OPS

RuntimeError RuntimeStack::push_int8_t(int8_t value) { return push_uint8_t((uint8_t) value); }
RuntimeError RuntimeStack::push_int16_t(int16_t value) { return push_uint16_t((uint16_t) value); }
RuntimeError RuntimeStack::push_int32_t(int32_t value) { return push_uint32_t((uint32_t) value); }
RuntimeError RuntimeStack::push_float(float value) { uint32_t_to_float cvt; cvt.type_float = value; return push_uint32_t(cvt.type_uint32_t); }
#ifdef USE_X64_OPS
RuntimeError RuntimeStack::push_int64_t(int64_t value) { return push_uint64_t((uint64_t) value); }
RuntimeError RuntimeStack::push_double(double value) { uint64_t_to_double cvt; cvt.type_double = value; return push_uint64_t(cvt.type_uint64_t); }
#endif // USE_X64_OPS

int8_t RuntimeStack::pop_int8_t() { return (int8_t) pop_uint8_t(); }
int16_t RuntimeStack::pop_int16_t() { return (int16_t) pop_uint16_t(); }
int32_t RuntimeStack::pop_int32_t() { return (int32_t) pop_uint32_t(); }
float RuntimeStack::pop_float() { uint32_t_to_float cvt; cvt.type_uint32_t = pop_uint32_t(); return cvt.type_float; }
#ifdef USE_X64_OPS
int64_t RuntimeStack::pop_int64_t() { return (int64_t) pop_uint64_t(); }
double RuntimeStack::pop_double() { uint64_t_to_double cvt; cvt.type_uint64_t = pop_uint64_t(); return cvt.type_double; }
#endif // USE_X64_OPS

int8_t RuntimeStack::peek_int8_t() { return (int8_t) peek_uint8_t(); }
int16_t RuntimeStack::peek_int16_t() { return (int16_t) peek_uint16_t(); }
int32_t RuntimeStack::peek_int32_t() { return (int32_t) peek_uint32_t(); }
float RuntimeStack::peek_float() { uint32_t_to_float cvt; cvt.type_uint32_t = peek_uint32_t(); return cvt.type_float; }
#ifdef USE_X64_OPS
int64_t RuntimeStack::peek_int64_t() { return (int64_t) peek_uint64_t(); }
double RuntimeStack::peek_double() { uint64_t_to_double cvt; cvt.type_uint64_t = peek_uint64_t(); return cvt.type_double; }
#endif // USE_X64_OPS

uint32_t RuntimeStack::size() { return stack->size(); }
void RuntimeStack::clear() { while (!stack->empty()) stack->pop(); while (!call_stack->empty()) call_stack->pop(); }