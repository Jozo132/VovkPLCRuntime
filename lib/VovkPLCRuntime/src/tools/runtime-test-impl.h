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
void print__uint64_t(uint64_t big_number) {
    const uint16_t NUM_DIGITS = log10(big_number) + 1;
    char sz[NUM_DIGITS + 1];
    sz[NUM_DIGITS] = 0;
    for (uint16_t i = NUM_DIGITS; i--; big_number /= 10)
        sz[i] = '0' + (big_number % 10);
    Serial.print(sz);
}
void println__uint64_t(uint64_t big_number) {
    print__uint64_t(big_number);
    Serial.println();
}

void print__int64_t(int64_t big_number) {
    if (big_number < 0) {
        Serial.print('-');
        big_number = -big_number;
    }
    print__uint64_t(big_number);
}
void println__int64_t(int64_t big_number) {
    print__int64_t(big_number);
    Serial.println();
}
#endif // USE_X64_OPS

int print_hex(uint32_t value) {
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
int print_hex(uint16_t value) {
    int length = 0;
    if (value < 0x10) length += Serial.print('0');
    if (value < 0x100) length += Serial.print('0');
    if (value < 0x1000) length += Serial.print('0');
    length += Serial.print(value, HEX);
    return length;
}
int print_hex(uint8_t value) {
    int length = 0;
    if (value < 0x10) length += Serial.print('0');
    length += Serial.print(value, HEX);
    return length;
}



#ifdef __RUNTIME_FULL_UNIT_TEST___
UnitTest::UnitTest() { }
UnitTest::~UnitTest() {
    if (program != nullptr) delete program;
    if (runtime != nullptr) delete runtime;
}
void UnitTest::begin() {
    if (program == nullptr) program = new RuntimeProgram(program_size);
    if (runtime == nullptr) runtime = new VovkPLCRuntime(stack_size, memory_size, *program);
    program->begin();
    program->format(program_size);
    runtime->formatMemory(memory_size);
}
#ifdef __RUNTIME_DEBUG__
template <typename T> void UnitTest::run(const TestCase<T>& test) {
    Serial.println();
    REPRINTLN(70, '#');
    program->format();
    if (test.build) test.build(*program);
    Serial.print(F("Running test: ")); Serial.println(test.name);
    RuntimeError status = fullProgramDebug(*runtime, *program);
    T output = runtime->read<T>();
    bool passed = status == test.expected_error && test.expected_result == output;
    Serial.print(F("Program result: ")); println(output);
    Serial.print(F("Expected result: ")); println(test.expected_result);
    Serial.print(F("Test status: ")); Serial.println(passed ? F("Passed") : F("--------------------> !!! FAILED !!! <--------------------"));
    Serial.println();
}
#endif

template <typename T> void UnitTest::review(const TestCase<T>& test) {
    program->format();
    if (test.build) test.build(*program);
    uint32_t offset = Serial.print(F("Test \""));
    offset += Serial.print(test.name);
    offset += Serial.print('"');
    long t = micros();
    runtime->cleanRun();
    T output = runtime->read<T>();
    t = micros() - t;
    float ms = (float) t * 0.001;
    bool passed = test.expected_result == output;
    for (; offset < 40; offset++) Serial.print(' ');
    Serial.print(passed ? F("Passed") : F("FAILED !!!"));
    Serial.print(F(" - ")); Serial.print(ms, 3); Serial.println(F(" ms"));
}

RuntimeError UnitTest::fullProgramDebug(VovkPLCRuntime& runtime) {
    if (runtime.program == nullptr) {
        Serial.println(F("No program loaded in the runtime"));
        return NO_PROGRAM;
    }
    return fullProgramDebug(runtime, *runtime.program);
}

RuntimeError UnitTest::fullProgramDebug(VovkPLCRuntime& runtime, RuntimeProgram& program) {
    runtime.clear(program);
    program.println();
#ifdef __RUNTIME_DEBUG__
    program.explain();
    Serial.println(F("Starting detailed program debug..."));
    uint32_t index = 0;
    bool finished = false;
    RuntimeError status = STATUS_SUCCESS;
    while (!finished) {
        index = program.getLine();
        long t = micros();
        status = runtime.step(program);
        t = micros() - t;
        float ms = (float) t * 0.001;
        if (status == PROGRAM_EXITED) finished = true;
        bool problem = status != STATUS_SUCCESS && status != PROGRAM_EXITED;
        // Serial.printf("    Step %4d [%02X %02X]: ", index, index >> 8, index & 0xFF);
        Serial.print(F("    Step "));
        print_number_padStart(index, 4);
        Serial.print(F(" ["));
        uint8_t hi = index >> 8;
        uint8_t lo = index & 0xFF;
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
    long t = micros();
    runtime.cleanRun(program);
    t = micros() - t;
#else
    long t = micros();
    RuntimeError status = runtime.cleanRun(program);
    t = micros() - t;
#endif
    float ms = (float) t * 0.001;
    Serial.print(F("Leftover ")); runtime.printStack(); Serial.println();
    Serial.print(F("Time to execute program: ")); Serial.print(ms, 3); Serial.println(F(" ms"));
    if (status != STATUS_SUCCESS) { Serial.print(F("Debug failed with error: ")); Serial.println(RUNTIME_ERROR_NAME(status)); }
    return status;
}

template <typename T> void UnitTest::println(T result) { Serial.println(result); }
void UnitTest::println(uint64_t result) { println__uint64_t(result); }
void UnitTest::println(int64_t result) { println__int64_t(result); }
#ifdef __WASM__
void UnitTest::println(int8_t result) {
    char buffer[4];
    sprintf(buffer, "%d", result);
    Serial.println(buffer);
}
#endif


void runtime_unit_test() {
#ifdef __WASM__
    Tester = new UnitTest();
#endif
    TesterMethod begin();
    TesterMethod runtime->startup();
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
    // Tester->run(case_demo_uint8_t);
    TesterMethod run(case_demo_uint8_t);
    TesterMethod run(case_demo_uint16_t);
    TesterMethod run(case_demo_uint32_t);
#ifdef USE_X64_OPS
    TesterMethod run(case_demo_uint64_t);
#endif // USE_X64_OPS
    TesterMethod run(case_demo_int8_t);
    TesterMethod run(case_demo_float);
#ifdef USE_X64_OPS
    TesterMethod run(case_demo_double);
#endif // USE_X64_OPS
    TesterMethod run(case_bitwise_and_X8);
    TesterMethod run(case_bitwise_and_X16);
    TesterMethod run(case_logic_and);
    TesterMethod run(case_logic_or);
    TesterMethod run(case_cmp_eq_1);
    TesterMethod run(case_cmp_eq_2);
    TesterMethod run(case_jump);
    TesterMethod run(case_jump_if);
    REPRINTLN(70, '-');
    REPRINTLN(70, '#');
    Serial.println(F("Runtime Unit Tests Completed."));
    REPRINTLN(70, '#');
#endif
    REPRINTLN(70, '#');
    Serial.println(F("Report:"));
    REPRINTLN(70, '#');
    TesterMethod review(case_demo_uint8_t);
    TesterMethod review(case_demo_uint16_t);
    TesterMethod review(case_demo_uint32_t);
#ifdef USE_X64_OPS
    TesterMethod review(case_demo_uint64_t);
#endif // USE_X64_OPS
    TesterMethod review(case_demo_int8_t);
    TesterMethod review(case_demo_float);
#ifdef USE_X64_OPS
    TesterMethod review(case_demo_double);
#endif // USE_X64_OPS
    TesterMethod review(case_bitwise_and_X8);
    TesterMethod review(case_bitwise_and_X16);
    TesterMethod review(case_logic_and);
    TesterMethod review(case_logic_or);
    TesterMethod review(case_cmp_eq_1);
    TesterMethod review(case_cmp_eq_2);
    TesterMethod review(case_jump);
    TesterMethod review(case_jump_if);
    REPRINTLN(70, '#');
    Serial.println(F("Runtime Unit Tests Report Completed."));
    REPRINTLN(70, '#');
#ifdef __WASM__
    delete Tester;
    Tester = nullptr;
#endif
};

#else // __RUNTIME_UNIT_TEST__

void runtime_unit_test() {
    if (runtime_test_called) return;
    Serial.println(F("Unit tests are disabled."));
    Serial.println();
    runtime_test_called = true;
};

RuntimeError UnitTest::fullProgramDebug(VovkPLCRuntime& runtime, RuntimeProgram& program) {
    program.print();
    Serial.println(F("Runtime working in production mode. Full program debugging is disabled."));
    Serial.println();
    runtime.clear(program);
    runtime.cleanRun(program);
    return program.status;
}

#endif // __RUNTIME_UNIT_TEST__