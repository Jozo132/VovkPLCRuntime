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

#include "runtime-test.h"

#ifdef USE_X64_OPS
void print__u64(u64 big_number) {
    const u16 NUM_DIGITS = log10(big_number) + 1;
    char sz[NUM_DIGITS + 1];
    sz[NUM_DIGITS] = 0;
    for (u16 i = NUM_DIGITS; i--; big_number /= 10)
        sz[i] = '0' + (big_number % 10);
    Serial.print(sz);
}
void println__u64(u64 big_number) {
    print__u64(big_number);
    Serial.println();
}

void print__i64(i64 big_number) {
    if (big_number < 0) {
        Serial.print('-');
        big_number = -big_number;
    }
    print__u64(big_number);
}
void println__i64(i64 big_number) {
    print__u64(big_number);
    Serial.println();
}
#endif // USE_X64_OPS

int print_hex(u32 value) {
    int length = 0;
    if (value < 0x10) length += Serial.print('0');
    if (value < 0x100) length += Serial.print('0');
    if (value < 0x1000) length += Serial.print('0');
    if (value < 0x10000) length += Serial.print('0');
    if (value < 0x100000) length += Serial.print('0');
    if (value < 0x1000000) length += Serial.print('0');
    if (value < 0x10000000) length += Serial.print('0');
    length += Serial.print(value, HEX);
    return length;
}
int print_hex(u16 value) {
    int length = 0;
    if (value < 0x10) length += Serial.print('0');
    if (value < 0x100) length += Serial.print('0');
    if (value < 0x1000) length += Serial.print('0');
    length += Serial.print(value, HEX);
    return length;
}
int print_hex(u8 value) {
    int length = 0;
    if (value < 0x10) length += Serial.print('0');
    length += Serial.print(value, HEX);
    return length;
}



#ifdef __RUNTIME_FULL_UNIT_TEST___
UnitTest::UnitTest() {}
#ifdef __RUNTIME_DEBUG__
template <typename T> void UnitTest::run(VovkPLCRuntime& runtime, const TestCase<T>& test) {
    Serial.println();
    REPRINTLN(70, '#');
    runtime.program.format();
    auto& program = runtime.program;
    if (test.build) test.build(program);
    Serial.print(F("Running test: ")); Serial.println(test.name);
    RuntimeError status = fullProgramDebug(runtime);
    T output = runtime.read<T>();
    bool passed = status == test.expected_error && test.expected_result == output;
    Serial.print(F("Program result: ")); println(output);
    Serial.print(F("Expected result: ")); println(test.expected_result);
    Serial.print(F("Test status: ")); Serial.println(passed ? F("Passed") : F("--------------------> !!! FAILED !!! <--------------------"));
    Serial.println();
}
#endif

template <typename T> void UnitTest::review(VovkPLCRuntime& runtime, const TestCase<T>& test) {
    auto& program = runtime.program;
    program.format();
    if (test.build) test.build(program);
    u32 offset = Serial.print(F("Test \""));
    offset += Serial.print(test.name);
    offset += Serial.print('"');
    u32 t = micros();
    runtime.run();
    T output = runtime.read<T>();
    t = micros() - t;
    f32 ms = (f32) t * 0.001;
    bool passed = test.expected_result == output;
    for (; offset < 40; offset++) Serial.print(' ');
    Serial.print(passed ? F("Passed") : F("FAILED !!!"));
    Serial.print(F(" - ")); Serial.print(ms, 3); Serial.println(F(" ms"));
}

RuntimeError UnitTest::fullProgramDebug(VovkPLCRuntime& runtime) {
    runtime.clear();
    auto& program = runtime.program;
    program.println();
#ifdef __RUNTIME_DEBUG__
    program.explain();
    Serial.println(F("Starting detailed program debug..."));
    u32 cycle = 0;
    u32 index = 0;
    bool finished = false;
    RuntimeError status = STATUS_SUCCESS;
    while (!finished) {
        cycle++;
        index = program.getLine();
        u32 t = micros();
        status = runtime.step(program);
        t = micros() - t;
        f32 ms = (f32) t * 0.001;
        if (cycle > MAX_PROGRAM_CYCLE_COUNT) status = PROGRAM_CYCLE_LIMIT_EXCEEDED;
        if (status == PROGRAM_EXITED) finished = true;
        bool problem = status != STATUS_SUCCESS && status != PROGRAM_EXITED;
        // Serial.printf("    Step %4d [%02X %02X]: ", index, index >> 8, index & 0xFF);
        Serial.print(F("    Step "));
        print_number_padStart(index, 4);
        Serial.print(F(" ["));
        u8 hi = index >> 8;
        u8 lo = index & 0xFF;
        print_number_padStart(hi, 2, '0', HEX);
        Serial.print(' ');
        print_number_padStart(lo, 2, '0', HEX);
        Serial.print(F("]: "));
        if (problem) {
            const char* error = RUNTIME_ERROR_NAME(status);
            Serial.print(F("Error at program pointer "));
            Serial.print(index);
            Serial.print(F(": "));
            Serial.println(error);
            return status;
        }
        Serial.print(F("Executed in ["));
        if (ms < 10)  Serial.print(' ');
        Serial.print(ms, 3);
        Serial.print(F(" ms]  "));
        int length = program.printOpcodeAt(index);
        Serial.print(' ');
        Serial.print(' ');
        while (length < 59) length += Serial.print('-');
        while (length < 61) length += Serial.print(' ');
        runtime.printStack();
        Serial.println();
        if (program.finished()) finished = true;
    }
    u32 t = micros();
    runtime.run(program);
    t = micros() - t;
#else
    u32 t = micros();
    RuntimeError status = runtime.run(program);
    t = micros() - t;
#endif
    f32 ms = (f32) t * 0.001;
    Serial.print(F("Leftover ")); runtime.printStack(); Serial.println();
    Serial.print(F("Time to execute program: ")); Serial.print(ms, 3); Serial.println(F(" ms"));
    if (status != STATUS_SUCCESS) { Serial.print(F("Debug failed with error: ")); Serial.println(RUNTIME_ERROR_NAME(status)); }
    return status;
}

template <typename T> void UnitTest::println(T result) { Serial.println(result); }
#ifdef USE_X64_OPS
void UnitTest::println(u64 result) { println__u64(result); }
void UnitTest::println(i64 result) { println__i64(result); }
#endif // USE_X64_OPS
#ifdef __WASM__
void UnitTest::println(i8 result) {
    char buffer[4];
    sprintf(buffer, "%d", result);
    Serial.println(buffer);
}
#endif


void runtime_unit_test(VovkPLCRuntime& runtime) {
    runtime.initialize();
    REPRINTLN(70, '-');
    Serial.println(F("Runtime Unit Test"));
    REPRINTLN(70, '-');
#ifdef __RUNTIME_DEBUG__
    logRuntimeErrorList();
    REPRINTLN(70, '-');
    logRuntimeInstructionSet();
    REPRINTLN(70, '-');
    REPRINTLN(70, '#');
    Serial.println(F("Executing Runtime Unit Tests..."));
    // Tester.run(case_demo_uint8_t);
    Tester.run(runtime, case_demo_uint8_t);
    Tester.run(runtime, case_demo_uint16_t);
    Tester.run(runtime, case_demo_uint32_t);
#ifdef USE_X64_OPS
    Tester.run(runtime, case_demo_uint64_t);
#endif // USE_X64_OPS
    Tester.run(runtime, case_demo_int8_t);
    Tester.run(runtime, case_demo_float);
#ifdef USE_X64_OPS
    Tester.run(runtime, case_demo_double);
#endif // USE_X64_OPS
    Tester.run(runtime, case_bitwise_and_X8);
    Tester.run(runtime, case_bitwise_and_X16);
    Tester.run(runtime, case_logic_and);
    Tester.run(runtime, case_logic_or);
    Tester.run(runtime, case_cmp_eq_1);
    Tester.run(runtime, case_cmp_eq_2);
    Tester.run(runtime, case_jump);
    Tester.run(runtime, case_jump_if);
    REPRINTLN(70, '-');
    REPRINTLN(70, '#');
    Serial.println(F("Runtime Unit Tests Completed."));
    REPRINTLN(70, '#');
#endif
    REPRINTLN(70, '#');
    Serial.println(F("Report:"));
    REPRINTLN(70, '#');
    Tester.review(runtime, case_demo_uint8_t);
    Tester.review(runtime, case_demo_uint16_t);
    Tester.review(runtime, case_demo_uint32_t);
#ifdef USE_X64_OPS
    Tester.review(runtime, case_demo_uint64_t);
#endif // USE_X64_OPS
    Tester.review(runtime, case_demo_int8_t);
    Tester.review(runtime, case_demo_float);
#ifdef USE_X64_OPS
    Tester.review(runtime, case_demo_double);
#endif // USE_X64_OPS
    Tester.review(runtime, case_bitwise_and_X8);
    Tester.review(runtime, case_bitwise_and_X16);
    Tester.review(runtime, case_logic_and);
    Tester.review(runtime, case_logic_or);
    Tester.review(runtime, case_cmp_eq_1);
    Tester.review(runtime, case_cmp_eq_2);
    Tester.review(runtime, case_jump);
    Tester.review(runtime, case_jump_if);
    REPRINTLN(70, '#');
    Serial.println(F("Runtime Unit Tests Report Completed."));
    REPRINTLN(70, '#');
    Serial.flush();
};

#else // __RUNTIME_UNIT_TEST__

void runtime_unit_test(VovkPLCRuntime& runtime) {
    if (runtime_test_called) return;
    Serial.println(F("Unit tests are disabled."));
    Serial.println();
    runtime_test_called = true;
};

RuntimeError UnitTest::fullProgramDebug(VovkPLCRuntime& runtime) {
    runtime.program.print();
    Serial.println(F("Runtime working in production mode. Full program debugging is disabled."));
    Serial.println();
    runtime.clear();
    runtime.run();
    return runtime.program.status;
}

#endif // __RUNTIME_UNIT_TEST__