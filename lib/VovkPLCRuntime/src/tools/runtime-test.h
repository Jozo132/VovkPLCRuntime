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

#if defined(__RUNTIME_FULL_UNIT_TEST___)
#warning RUNTIME FULL UNIT TEST ENABLED
#endif // __RUNTIME_FULL_UNIT_TEST___

#if defined(__RUNTIME_DEBUG__)
#warning RUNTIME DEBUG MODE ENABLED - Pleas do not use this in production code.This is only for testing purposesand might cause unexpected behaviour.
#endif // __RUNTIME_DEBUG__

template <typename T> struct TestCase {
    const char* name;
    RuntimeError expected_error;
    T expected_result;
    // function pointer for startup that takes a RuntimeProgram & type and returns void 
    void (*build)(RuntimeProgram&);
};

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

#ifdef __RUNTIME_FULL_UNIT_TEST___
struct UnitTest {
    static const uint16_t memory_size = 16;
    static const uint16_t stack_size = 32;
    static const uint16_t program_size = 64;
    RuntimeProgram program = RuntimeProgram(program_size);
    VovkPLCRuntime runtime = VovkPLCRuntime(stack_size, memory_size, program);
    UnitTest() {
        program.erase();
        runtime.formatMemory(memory_size);
    }
#ifdef __RUNTIME_DEBUG__
    template <typename T> void run(const TestCase<T>& test) {
        Serial.println();
        REPRINTLN(70, '#');
        program.erase();
        test.build(program);
        Serial.print(F("Running test: ")); Serial.println(test.name);
        RuntimeError status = fullProgramDebug(runtime, program);
        T output = runtime.read<T>();
        bool passed = status == test.expected_error && test.expected_result == output;
        Serial.print(F("Program result: ")); println(output);
        Serial.print(F("Expected result: ")); println(test.expected_result);
        Serial.print(F("Test status: ")); Serial.println(passed ? F("Passed") : F("--------------------> !!! FAILED !!! <--------------------"));
        Serial.println();
    }
#endif

    template <typename T> void review(const TestCase<T>& test) {
        size_t offset = Serial.print("Test \"");
        offset += Serial.print(test.name);
        offset += Serial.print('"');
        program.erase();
        test.build(program);
        long t = micros();
        runtime.cleanRun();
        T output = runtime.read<T>();
        t = micros() - t;
        float ms = (float) t * 0.001;
        bool passed = test.expected_result == output;
        for (; offset < 40; offset++) Serial.print(' ');
        Serial.print(passed ? F("Passed") : F("FAILED !!!"));
        Serial.print(F(" - ")); Serial.print(ms, 3); Serial.println(F(" ms"));
    }

    static RuntimeError fullProgramDebug(VovkPLCRuntime& runtime) {
        if (runtime.program == nullptr) {
            Serial.println(F("No program loaded in the runtime"));
            return NO_PROGRAM;
        }
        return fullProgramDebug(runtime, *runtime.program);
    }
    static RuntimeError fullProgramDebug(VovkPLCRuntime& runtime, RuntimeProgram& program) {
        runtime.clear(program);
        program.println();
#ifdef __RUNTIME_DEBUG__
        uint16_t program_size = program.getProgramSize();
        program.explain();
        Serial.println(F("Starting detailed program debug..."));
        uint16_t index = 0;
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
            Serial.printf("    Step %4d [%02X %02X]: ", index, index >> 8, index & 0xFF);
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

    template <typename T> void println(T result) { Serial.println(result); }
    void println(uint64_t result) { println__uint64_t(result); }
    void println(int64_t result) { println__int64_t(result); }
};

UnitTest Tester;

const TestCase<uint8_t>* case_demo_uint8_t = new TestCase<uint8_t>({ "demo_uint8_t => (1 + 2) * 3", STATUS_SUCCESS, 9, [](RuntimeProgram& program) {
    program.push_uint8_t(1);
    program.push_uint8_t(2);
    program.push(ADD, type_uint8_t);
    program.push_uint8_t(3);
    program.push(MUL, type_uint8_t);
} });
const TestCase<uint16_t>* case_demo_uint16_t = new TestCase<uint16_t>({ "demo_uint16_t => (1 + 2) * 3", STATUS_SUCCESS, 9, [](RuntimeProgram& program) {
    program.push_uint16_t(1);
    program.push_uint16_t(2);
    program.push(ADD, type_uint16_t);
    program.push_uint16_t(3);
    program.push(MUL, type_uint16_t);
} });

const TestCase<uint32_t>* case_demo_uint32_t = new TestCase<uint32_t>({ "demo_uint32_t => (1 + 2) * 3", STATUS_SUCCESS, 9, [](RuntimeProgram& program) {
    program.push_uint32_t(1);
    program.push_uint32_t(2);
    program.push(ADD, type_uint32_t);
    program.push_uint32_t(3);
    program.push(MUL, type_uint32_t);
} });
const TestCase<uint64_t>* case_demo_uint64_t = new TestCase<uint64_t>({ "demo_uint64_t => (1 + 2) * 3", STATUS_SUCCESS, 9, [](RuntimeProgram& program) {
    program.push_uint64_t(1);
    program.push_uint64_t(2);
    program.push(ADD, type_uint64_t);
    program.push_uint64_t(3);
    program.push(MUL, type_uint64_t);
} });

const TestCase<int8_t>* case_demo_int8_t = new TestCase<int8_t>({ "demo_int8_t => (1 - 2) * 3", STATUS_SUCCESS, -3, [](RuntimeProgram& program) {
    program.push_int8_t(1);
    program.push_int8_t(2);
    program.push(SUB, type_int8_t);
    program.push_int8_t(3);
    program.push(MUL, type_int8_t);
} });

const TestCase<float>* case_demo_float = new TestCase<float>({ "demo_float => (0.1 + 0.2) * -1", STATUS_SUCCESS, -0.3, [](RuntimeProgram& program) {
    program.push_float(0.1);
    program.push_float(0.2);
    program.push(ADD, type_float);
    program.push_float(-1);
    program.push(MUL, type_float);
} });
const TestCase<double>* case_demo_double = new TestCase<double>({ "demo_double => (0.1 + 0.2) * -1", STATUS_SUCCESS, -0.30000000000000004, [](RuntimeProgram& program) {
    program.push_double(0.1);
    program.push_double(0.2);
    program.push(ADD, type_double);
    program.push_double(-1);
    program.push(MUL, type_double);
} });

// Bitwise operations
const TestCase<uint8_t>* case_bitwise_and_X8 = new TestCase<uint8_t>({ "bitwise_and_X8", STATUS_SUCCESS, 0b00000101, [](RuntimeProgram& program) {
    program.push_uint8_t(0b00001111);
    program.push_uint8_t(0b01010101);
    program.push(BW_AND_X8);
} });
const TestCase<uint16_t>* case_bitwise_and_X16 = new TestCase<uint16_t>({ "bitwise_and_X16", STATUS_SUCCESS, 0x000F, [](RuntimeProgram& program) {
    program.push_uint16_t(0x00FF);
    program.push_uint16_t(0xF00F);
    program.push(BW_AND_X16);
} });

// Logic (boolean) operations
const TestCase<bool>* case_logic_and = new TestCase<bool>({ "logic_and => true && true", STATUS_SUCCESS, true, [](RuntimeProgram& program) {
    program.push_bool(true);
    program.push_bool(true);
    program.push(LOGIC_AND);
} });
const TestCase<bool>* case_logic_or = new TestCase<bool>({ "logic_or => true || false", STATUS_SUCCESS, true, [](RuntimeProgram& program) {
    program.push_bool(true);
    program.push_bool(false);
    program.push(LOGIC_OR);
} });

// Comparison operations
const TestCase<bool>* case_cmp_eq = new TestCase<bool>({ "cmp_eq => 1 == 1", STATUS_SUCCESS, true, [](RuntimeProgram& program) {
    program.push_bool(1);
    program.push_bool(1);
    program.push(CMP_EQ, type_bool);
} });
const TestCase<bool>* case_cmp_eq_1 = new TestCase<bool>({ "cmp_eq => 0.29 == 0.31", STATUS_SUCCESS, false, [](RuntimeProgram& program) {
    program.push_float(0.29);
    program.push_float(0.31);
    program.push(CMP_EQ, type_float);
} });

// Jump operations
const TestCase<uint8_t>* case_jump = new TestCase<uint8_t>({ "jump => push 1 and jump to exit", PROGRAM_EXITED, 1, [](RuntimeProgram& program) {
    program.push_uint8_t(1); // 0 [+2]
    uint16_t jump_index = program.size();
    program.pushJMP(0); // 2 [+3]
    program.push_uint8_t(1); // 5 [+2]
    program.push(ADD, type_uint8_t); // 7 [+2]
    program.push_uint8_t(3); // 9 [+2]
    program.push(MUL, type_uint8_t); // 11 [+2]
    uint16_t exit_index = program.size();
    program.push(EXIT); // 13 [+1]
    program.modifyValue(jump_index + 1, exit_index); // Change the jump address to the exit address
} });
const TestCase<uint8_t>* case_jump_if = new TestCase<uint8_t>({ "jump_if => for loop sum", PROGRAM_EXITED, 100, [](RuntimeProgram& program) {
    // uint8_t sum = 0; 
    // for (uint8_t i = 0; i < 10; i++) {
    //   sum += (uint8_t) 10;
    // }
    // return sum;
    static const uint8_t sum_ptr = 0;
    static const uint8_t i_ptr = 1;
    uint16_t loop_jump = 0;
    uint16_t loop_destination = 0;
    uint16_t end_jump = 0;
    uint16_t end_destination = 0;
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


void runtime_test() {
    Tester.runtime.startup();
    REPRINTLN(70, '-');
    Serial.println(F("Runtime Unit Test"));
    REPRINTLN(70, '-');
    logRuntimeInstructionSet();
    REPRINTLN(70, '-');
#ifdef __RUNTIME_DEBUG__
    REPRINTLN(70, '#');
    Serial.println(F("Executing Runtime Unit Tests..."));
    Tester.run(*case_demo_uint8_t);
    Tester.run(*case_demo_uint16_t);
    Tester.run(*case_demo_uint32_t);
    Tester.run(*case_demo_uint64_t);
    Tester.run(*case_demo_int8_t);
    Tester.run(*case_demo_float);
    Tester.run(*case_demo_double);
    Tester.run(*case_bitwise_and_X8);
    Tester.run(*case_bitwise_and_X16);
    Tester.run(*case_logic_and);
    Tester.run(*case_logic_or);
    Tester.run(*case_cmp_eq);
    Tester.run(*case_jump);
    Tester.run(*case_jump_if);
    REPRINTLN(70, '-');
    REPRINTLN(70, '#');
    Serial.println(F("Runtime Unit Tests Completed."));
    REPRINTLN(70, '#');
#endif
    REPRINTLN(70, '#');
    Serial.println(F("Report:"));
    REPRINTLN(70, '#');
    Tester.review(*case_demo_uint8_t); delete case_demo_uint8_t;
    Tester.review(*case_demo_uint16_t); delete case_demo_uint16_t;
    Tester.review(*case_demo_uint32_t); delete case_demo_uint32_t;
    Tester.review(*case_demo_uint64_t); delete case_demo_uint64_t;
    Tester.review(*case_demo_int8_t); delete case_demo_int8_t;
    Tester.review(*case_demo_float); delete case_demo_float;
    Tester.review(*case_demo_double); delete case_demo_double;
    Tester.review(*case_bitwise_and_X8); delete case_bitwise_and_X8;
    Tester.review(*case_bitwise_and_X16); delete case_bitwise_and_X16;
    Tester.review(*case_logic_and); delete case_logic_and;
    Tester.review(*case_logic_or); delete case_logic_or;
    Tester.review(*case_cmp_eq); delete case_cmp_eq;
    Tester.review(*case_jump); delete case_jump;
    Tester.review(*case_jump_if); delete case_jump_if;
    REPRINTLN(70, '#');
    Serial.println(F("Runtime Unit Tests Report Completed."));
    REPRINTLN(70, '#');
    Serial.println();
};

#else // __RUNTIME_UNIT_TEST__

bool runtime_test_called = false;
void runtime_test() {
    if (runtime_test_called) return;
    Serial.println(F("Unit tests are disabled."));
    Serial.println();
    runtime_test_called = true;
};
class UnitTest {
public:
    static RuntimeError fullProgramDebug(VovkPLCRuntime& runtime, RuntimeProgram& program) {
        program.print();
        Serial.println(F("Runtime working in production mode. Full program debugging is disabled."));
        Serial.println();
        runtime.clear(program);
        runtime.cleanRun(program);
        return program.status;
    }
};

#endif // __RUNTIME_UNIT_TEST__