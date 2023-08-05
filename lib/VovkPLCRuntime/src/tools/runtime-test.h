// runtime-test.h - 1.0.0 - 2022-12-11
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

#include "runtime-lib.h"

#ifndef __SIMULATOR__
#if defined(__RUNTIME_FULL_UNIT_TEST___)
#warning RUNTIME FULL UNIT TEST ENABLED
#endif // __RUNTIME_FULL_UNIT_TEST___

#if defined(__RUNTIME_DEBUG__)
#warning RUNTIME DEBUG MODE ENABLED - Pleas do not use this in production code.This is only for testing purposesand might cause unexpected behaviour.
#endif // __RUNTIME_DEBUG__
#endif

template <typename T> struct TestCase {
    const char* name;
    RuntimeError expected_error;
    T expected_result;
    void (*build)(RuntimeProgram& program);
};

#ifdef USE_X64_OPS
void print__uint64_t(uint64_t big_number);
void println__uint64_t(uint64_t big_number);

void print__int64_t(int64_t big_number);
void println__int64_t(int64_t big_number);
#endif // USE_X64_OPS

int print_hex(uint32_t value);
int print_hex(uint16_t value);
int print_hex(uint8_t value);



#ifdef __RUNTIME_FULL_UNIT_TEST___
struct UnitTest {
    static const uint32_t memory_size = 16;
    static const uint32_t stack_size = 32;
    static const uint32_t program_size = 64;
#ifdef __WASM__
    RuntimeProgram* program = nullptr;
    VovkPLCRuntime* runtime = nullptr;
#else 
    RuntimeProgram* program = new RuntimeProgram(program_size);
    VovkPLCRuntime* runtime = new VovkPLCRuntime(stack_size, memory_size, *program);
#endif
    UnitTest();
    ~UnitTest();
    void begin();
#ifdef __RUNTIME_DEBUG__
    template <typename T> void run(const TestCase<T>& test);
#endif

    template <typename T> void review(const TestCase<T>& test);

    static RuntimeError fullProgramDebug(VovkPLCRuntime& runtime);
    static RuntimeError fullProgramDebug(VovkPLCRuntime& runtime, RuntimeProgram& program);

    template <typename T> void println(T result);
    void println(uint64_t result);
    void println(int64_t result);
#ifdef __WASM__ 
    void println(int8_t result);
#endif
};


#ifdef __WASM__
UnitTest* Tester = nullptr;
#define TesterMethod Tester->
#else
UnitTest Tester;
#define TesterMethod Tester.
#endif

const TestCase<uint8_t> case_demo_uint8_t({ "demo_uint8_t => (1 + 2) * 3", STATUS_SUCCESS, 9, [](RuntimeProgram& program) {
    program.push_uint8_t(1);
    program.push_uint8_t(2);
    program.push(ADD, type_uint8_t);
    program.push_uint8_t(3);
    program.push(MUL, type_uint8_t);
} });

const TestCase<uint16_t> case_demo_uint16_t({ "demo_uint16_t => (1 + 2) * 3", STATUS_SUCCESS, 9, [](RuntimeProgram& program) {
    program.push_uint16_t(1);
    program.push_uint16_t(2);
    program.push(ADD, type_uint16_t);
    program.push_uint16_t(3);
    program.push(MUL, type_uint16_t);
} });

const TestCase<uint32_t> case_demo_uint32_t({ "demo_uint32_t => (1 + 2) * 3", STATUS_SUCCESS, 9, [](RuntimeProgram& program) {
    program.push_uint32_t(1);
    program.push_uint32_t(2);
    program.push(ADD, type_uint32_t);
    program.push_uint32_t(3);
    program.push(MUL, type_uint32_t);
} });
#ifdef USE_X64_OPS
const TestCase<uint64_t> case_demo_uint64_t({ "demo_uint64_t => (1 + 2) * 3", STATUS_SUCCESS, 9, [](RuntimeProgram& program) {
    program.push_uint64_t(1);
    program.push_uint64_t(2);
    program.push(ADD, type_uint64_t);
    program.push_uint64_t(3);
    program.push(MUL, type_uint64_t);
} });
#endif // USE_X64_OPS

const TestCase<int8_t> case_demo_int8_t({ "demo_int8_t => (1 - 2) * 3", STATUS_SUCCESS, -3, [](RuntimeProgram& program) {
    program.push_int8_t(1);
    program.push_int8_t(2);
    program.push(SUB, type_int8_t);
    program.push_int8_t(3);
    program.push(MUL, type_int8_t);
} });

const TestCase<float> case_demo_float({ "demo_float => (0.1 + 0.2) * -1", STATUS_SUCCESS, -0.3, [](RuntimeProgram& program) {
    program.push_float(0.1);
    program.push_float(0.2);
    program.push(ADD, type_float);
    program.push_float(-1);
    program.push(MUL, type_float);
} });

#ifdef USE_X64_OPS
const TestCase<double> case_demo_double({ "demo_double => (0.1 + 0.2) * -1", STATUS_SUCCESS, -0.30000000000000004, [](RuntimeProgram& program) {
    program.push_double(0.1);
    program.push_double(0.2);
    program.push(ADD, type_double);
    program.push_double(-1);
    program.push(MUL, type_double);
} });
#endif // USE_X64_OPS

// Bitwise operations
const TestCase<uint8_t> case_bitwise_and_X8({ "bitwise_and_X8", STATUS_SUCCESS, 0b00000101, [](RuntimeProgram& program) {
    program.push_uint8_t(0b00001111);
    program.push_uint8_t(0b01010101);
    program.push(BW_AND_X8);
} });
const TestCase<uint16_t> case_bitwise_and_X16({ "bitwise_and_X16", STATUS_SUCCESS, 0x000F, [](RuntimeProgram& program) {
    program.push_uint16_t(0x00FF);
    program.push_uint16_t(0xF00F);
    program.push(BW_AND_X16);
} });

// Logic (boolean) operations
const TestCase<bool> case_logic_and({ "logic_and => true && true", STATUS_SUCCESS, true, [](RuntimeProgram& program) {
    program.push_bool(true);
    program.push_bool(true);
    program.push(LOGIC_AND);
} });
const TestCase<bool> case_logic_or({ "logic_or => true || false", STATUS_SUCCESS, true, [](RuntimeProgram& program) {
    program.push_bool(true);
    program.push_bool(false);
    program.push(LOGIC_OR);
} });

// Comparison operations
const TestCase<bool> case_cmp_eq_1({ "cmp_eq_1 => 1 == 1", STATUS_SUCCESS, true, [](RuntimeProgram& program) {
    program.push_bool(1);
    program.push_bool(1);
    program.push(CMP_EQ, type_bool);
} });
const TestCase<bool> case_cmp_eq_2({ "cmp_eq_2 => 0.29 == 0.31", STATUS_SUCCESS, false, [](RuntimeProgram& program) {
    program.push_float(0.29);
    program.push_float(0.31);
    program.push(CMP_EQ, type_float);
} });

// Jump operations
const TestCase<uint8_t> case_jump({ "jump => push 1 and jump to exit", PROGRAM_EXITED, 1, [](RuntimeProgram& program) {
    program.push_uint8_t(1); // 0 [+2]
    uint32_t jump_index = program.size();
    program.pushJMP(0); // 2 [+3]
    program.push_uint8_t(1); // 5 [+2]
    program.push(ADD, type_uint8_t); // 7 [+2]
    program.push_uint8_t(3); // 9 [+2]
    program.push(MUL, type_uint8_t); // 11 [+2]
    uint32_t exit_index = program.size();
    program.push(EXIT); // 13 [+1]
    program.modifyValue(jump_index + 1, exit_index); // Change the jump address to the exit address
} });
const TestCase<uint8_t> case_jump_if({ "jump_if => for loop sum", PROGRAM_EXITED, 100, [](RuntimeProgram& program) {
    // uint8_t sum = 0; 
    // for (uint8_t i = 0; i < 10; i++) {
    //   sum += (uint8_t) 10;
    // }
    // return sum;
    static const uint8_t sum_ptr = 0;
    static const uint8_t i_ptr = 1;
    uint32_t loop_jump = 0;
    uint32_t loop_destination = 0;
    uint32_t end_destination = 0;
    program.push_uint8_t(0);            // Add 0 to the stack           [0]  
    program.pushPUT(sum_ptr);           // Store 0 in sum_ptr           []
    program.push_uint8_t(0);            // Add 0 to the stack           [0]
    program.pushPUT(i_ptr);             // Store 0 in i_ptr             []
    loop_destination = program.size();  // :loop
    program.pushGET(i_ptr);             // Load i_ptr to the stack      [0]
    program.push_uint8_t(10);           // Add 10 to the stack          [0, 10]
    program.push(CMP_LT, type_uint8_t); // Compare 0 < 10               [1]
    loop_jump = program.size();
    program.pushJMP_IF_NOT(0);          // Jump to :end if 1 == 0       []
    program.pushGET(i_ptr);             // Load i_ptr to the stack      [0]
    program.push_uint8_t(1);            // Add 1 to the stack           [0, 1]
    program.push(ADD, type_uint8_t);    // Add 0 + 1                    [1]
    program.pushPUT(i_ptr);             // Store 1 in i_ptr             []
    program.pushGET(sum_ptr);           // Load sum_ptr to the stack    [0]
    program.push_uint8_t(10);           // Add 10 to the stack          [0, 10]
    program.push(ADD, type_uint8_t);    // Add 0 + 10                   [10]
    program.pushPUT(sum_ptr);           // Store 10 in sum_ptr          []
    program.pushJMP(loop_destination);  // Jump to :loop                []
    end_destination = program.size();   // :end
    program.pushGET(sum_ptr);           // Load sum_ptr to the stack    [0]
    program.push(EXIT);                 // Exit the program
    program.modifyValue(loop_jump + 1, end_destination); // Change the jump address to the exit address
} });

void runtime_unit_test();

#else // __RUNTIME_UNIT_TEST__

bool runtime_test_called = false;
void runtime_unit_test();
class UnitTest {
public:
    static RuntimeError fullProgramDebug(VovkPLCRuntime& runtime, RuntimeProgram& program);
};

#endif // __RUNTIME_UNIT_TEST__



#include "runtime-test-impl.h"