// runtime-test.h - 2022-12-11
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
#warning RUNTIME DEBUG MODE ENABLED - Pleas do not use this in production code.This is only for testing purposes and might cause unexpected behaviour.
#endif // __RUNTIME_DEBUG__
#endif

template <typename T> struct TestCase {
    const char* name;
    RuntimeError expected_error;
    T expected_result;
    void (*build)(RuntimeProgram& program);
};

#ifdef USE_X64_OPS
void print__u64(u64 big_number);
void println__u64(u64 big_number);

void print__u64(i64 big_number);
void println__i64(i64 big_number);
#endif // USE_X64_OPS

int print_hex(u32 value);
int print_hex(u16 value);
int print_hex(u8 value);



#ifdef __RUNTIME_FULL_UNIT_TEST___
struct UnitTest {
    UnitTest();
#ifdef __RUNTIME_DEBUG__
    template <typename T> void run(VovkPLCRuntime& runtime, const TestCase<T>& test);
#endif

    template <typename T> void review(VovkPLCRuntime& runtime, const TestCase<T>& test);

    static RuntimeError fullProgramDebug(VovkPLCRuntime& runtime);

    template <typename T> void println(T result);
    void println(u64 result);
    void println(i64 result);
#ifdef __WASM__ 
    void println(i8 result);
#endif
};


UnitTest Tester;
#define TesterMethod Tester.


const TestCase<u8> case_demo_uint8_t({ "demo_uint8_t => (1 + 2) * 3", STATUS_SUCCESS, 9, [](RuntimeProgram& program) {
    program.push_u8(1);
    program.push_u8(2);
    program.push(ADD, type_u8);
    program.push_u8(3);
    program.push(MUL, type_u8);
} });

const TestCase<u16> case_demo_uint16_t({ "demo_uint16_t => (1 + 2) * 3", STATUS_SUCCESS, 9, [](RuntimeProgram& program) {
    program.push_u16(1);
    program.push_u16(2);
    program.push(ADD, type_u16);
    program.push_u16(3);
    program.push(MUL, type_u16);
} });

const TestCase<u32> case_demo_uint32_t({ "demo_uint32_t => (1 + 2) * 3", STATUS_SUCCESS, 9, [](RuntimeProgram& program) {
    program.push_u32(1);
    program.push_u32(2);
    program.push(ADD, type_u32);
    program.push_u32(3);
    program.push(MUL, type_u32);
} });
#ifdef USE_X64_OPS
const TestCase<u64> case_demo_uint64_t({ "demo_uint64_t => (1 + 2) * 3", STATUS_SUCCESS, 9, [](RuntimeProgram& program) {
    program.push_u64(1);
    program.push_u64(2);
    program.push(ADD, type_u64);
    program.push_u64(3);
    program.push(MUL, type_u64);
} });
#endif // USE_X64_OPS

const TestCase<i8> case_demo_int8_t({ "demo_int8_t => (1 - 2) * 3", STATUS_SUCCESS, -3, [](RuntimeProgram& program) {
    program.push_i8(1);
    program.push_i8(2);
    program.push(SUB, type_i8);
    program.push_i8(3);
    program.push(MUL, type_i8);
} });

const TestCase<float> case_demo_float({ "demo_float => (0.1 + 0.2) * -1", STATUS_SUCCESS, -0.3, [](RuntimeProgram& program) {
    program.push_f32(0.1);
    program.push_f32(0.2);
    program.push(ADD, type_f32);
    program.push_f32(-1);
    program.push(MUL, type_f32);
} });

#ifdef USE_X64_OPS
const TestCase<f64> case_demo_double({ "demo_double => (0.1 + 0.2) * -1", STATUS_SUCCESS, -0.30000000000000004, [](RuntimeProgram& program) {
    program.push_f64(0.1);
    program.push_f64(0.2);
    program.push(ADD, type_f64);
    program.push_f64(-1);
    program.push(MUL, type_f64);
} });
#endif // USE_X64_OPS

// Bitwise operations
const TestCase<u8> case_bitwise_and_X8({ "bitwise_and_X8", STATUS_SUCCESS, 0b00000101, [](RuntimeProgram& program) {
    program.push_u8(0b00001111);
    program.push_u8(0b01010101);
    program.push(BW_AND_X8);
} });
const TestCase<u16> case_bitwise_and_X16({ "bitwise_and_X16", STATUS_SUCCESS, 0x000F, [](RuntimeProgram& program) {
    program.push_u16(0x00FF);
    program.push_u16(0xF00F);
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
    program.push_f32(0.29);
    program.push_f32(0.31);
    program.push(CMP_EQ, type_f32);
} });

// Jump operations
const TestCase<u8> case_jump({ "jump => push 1 and jump to exit", PROGRAM_EXITED, 1, [](RuntimeProgram& program) {
    program.push_u8(1); // 0 [+2]
    u32 jump_index = program.size();
    program.push_jmp(0); // 2 [+3]
    program.push_u8(1); // 5 [+2]
    program.push(ADD, type_u8); // 7 [+2]
    program.push_u8(3); // 9 [+2]
    program.push(MUL, type_u8); // 11 [+2]
    u32 exit_index = program.size();
    program.push(EXIT); // 13 [+1]
    program.modifyValue(jump_index + 1, exit_index); // Change the jump address to the exit address
} });
const TestCase<u8> case_jump_if({ "jump_if => for loop sum", PROGRAM_EXITED, 100, [](RuntimeProgram& program) {
    // u8 sum = 0; 
    // for (u8 i = 0; i < 10; i++) {
    //   sum += (u8) 10;
    // }
    // return sum;
    static const u8 sum_ptr = 0;
    static const u8 i_ptr = 1;
    u32 loop_jump = 0;
    u32 loop_destination = 0;
    u32 end_destination = 0;
    program.push_u32(0x12345678);
    program.push_pointer(sum_ptr);       // Add sum_ptr to the stack     [sum_ptr]
    program.push_u8(0);                  // Add 0 to the stack           [sum_ptr, 0]  
    program.push_move(type_u8);          // Store 0 in sum_ptr           []
    program.push_pointer(i_ptr);         // Add i_ptr to the stack       [i_ptr]
    program.push_u8(0);                  // Add 0 to the stack           [i_ptr, 0]
    program.push_move(type_u8);          // Store 0 in i_ptr             []
    loop_destination = program.size();   // :loop
    program.push_pointer(i_ptr);         // Load i_ptr to the stack      [i_ptr]
    program.push_load(type_u8);          // Load 0 from i_ptr            [0]
    program.push_u8(10);                 // Add 10 to the stack          [0, 10]
    program.push(CMP_LT, type_u8);       // Compare 0 < 10               [1]
    loop_jump = program.size();
    program.push_jmp_if_not(0);          // Jump to :end if 1 == 0       []
    program.push_pointer(i_ptr);         // Load i_ptr to the stack      [i_ptr]
    program.push_copy(type_pointer);     // Load i_ptr to the stack      [i_ptr, i_ptr]
    program.push_load(type_u8);          // Load 0 from i_ptr            [i_ptr, 0]
    program.push_u8(1);                  // Add 1 to the stack           [i_ptr, 0, 1]
    program.push(ADD, type_u8);          // Add 0 + 1                    [i_ptr, 1]
    program.push_move(type_u8);          // Store 1 in i_ptr             []
    program.push_pointer(sum_ptr);       // Load sum_ptr to the stack    [sum_ptr]
    program.push_copy(type_pointer);     // Load sum_ptr to the stack    [sum_ptr, sum_ptr]
    program.push_load(type_u8);          // Load 0 from sum_ptr          [sum_ptr, 0]
    program.push_u8(10);                 // Add 10 to the stack          [sum_ptr, 0, 10]
    program.push(ADD, type_u8);          // Add 0 + 10                   [sum_ptr, 10]
    program.push_move(type_u8);          // Store 10 in sum_ptr          []
    program.push_jmp(loop_destination);  // Jump to :loop                []
    end_destination = program.size();    // :end
    program.push_pointer(sum_ptr);       // Load sum_ptr to the stack    [sum_ptr]
    program.push_load(type_u8);          // Load 0 from sum_ptr          [0]
    program.push(EXIT);                  // Exit the program
    program.modifyValue(loop_jump + 1, end_destination); // Change the jump address to the exit address
} });

void runtime_unit_test(VovkPLCRuntime& runtime);

#else // __RUNTIME_UNIT_TEST__

bool runtime_test_called = false;
void runtime_unit_test(VovkPLCRuntime& runtime);
class UnitTest {
public:
    static RuntimeError fullProgramDebug(VovkPLCRuntime& runtime);
};

#endif // __RUNTIME_UNIT_TEST__



#include "runtime-test-impl.h"