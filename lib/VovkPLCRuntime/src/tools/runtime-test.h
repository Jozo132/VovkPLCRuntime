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

#define REPRINT(count, str) for (uint8_t i = 0; i < count; i++) { Serial.print(str); }
#define REPRINTLN(count, str) REPRINT(count, str); Serial.println();

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
    // static const uint16_t memory_size = 16;
    static const uint16_t stack_size = 32;
    static const uint16_t program_size = 64;
    // uint8_t memory[memory_size];
    RuntimeProgram program = RuntimeProgram(program_size);
    VovkPLCRuntime runtime = VovkPLCRuntime(stack_size/* , memory, memory_size */, program);

#ifdef __RUNTIME_DEBUG__
    template <typename T> void run(const TestCase<T>& test) {
        REPRINTLN(50, '-');
        program.erase();
        test.build(program);
        Serial.print(F("Running test: ")); Serial.println(test.name);
        RuntimeError status = fullProgramDebug(runtime, program);
        T output = runtime.read<T>();
        bool passed = status == test.expected_error && test.expected_result == output;
        Serial.print(F("Program result: ")); println(output);
        Serial.print(F("Expected result: ")); println(test.expected_result);
        Serial.print(F("Test passed: ")); Serial.println(passed ? F("YES") : F("NO - TEST DID NOT PASS !!!"));
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
        uint16_t program_pointer = 0;
        bool finished = false;
        RuntimeError status = STATUS_SUCCESS;
        while (!finished) {
            program_pointer = program.getLine();
            long t = micros();
            status = runtime.step(program);
            t = micros() - t;
            float ms = (float) t * 0.001;
            if (status == PROGRAM_EXITED) finished = true;
            bool problem = status != STATUS_SUCCESS && status != PROGRAM_EXITED;
            if (problem) {
                const char* error = getRuntimeErrorName(status);
                Serial.print(F("Error at program pointer "));
                Serial.print(program_pointer);
                Serial.print(F(": "));
                Serial.println(error);
                return status;
            }
            Serial.print(F("Stack trace @Program ["));
            Serial.print(program_pointer);
            Serial.print(F("]: "));
            runtime.printStack();
            Serial.print(F("   <= "));
            program.printOpcodeAt(program_pointer);
            Serial.print(F("  (executed in "));
            Serial.print(ms, 3);
            Serial.print(F(" ms)"));
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
        if (status != STATUS_SUCCESS) { Serial.print(F("Debug failed with error: ")); Serial.println(getRuntimeErrorName(status)); }
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
const TestCase<uint8_t>* case_jump = new TestCase<uint8_t>({ "jump => 1", PROGRAM_EXITED, 1, [](RuntimeProgram& program) {
    program.push_uint8_t(1); // 0 [+2]
    program.pushJMP(13); // 2 [+3]
    program.push_uint8_t(1); // 5 [+2]
    program.push(ADD, type_uint8_t); // 7 [+2]
    program.push_uint8_t(3); // 9 [+2]
    program.push(MUL, type_uint8_t); // 11 [+2]
    program.push(EXIT); // 13 [+1]
} });


void runtime_test() {
    REPRINTLN(50, '-');
    Serial.println(F("Runtime Unit Test"));
    REPRINTLN(50, '-');
    Serial.println(F("Bytecode Instruction Set:"));
    size_t position = 0;
    bool was_valid = true;
    for (uint8_t opcode = 0x00; opcode < 256; opcode++) {
        bool is_valid = OPCODE_EXISTS((PLCRuntimeInstructionSet) opcode);
        if (is_valid && !was_valid) Serial.println();
        was_valid = is_valid;
        if (is_valid) {
            position = Serial.print(F("   0x"));
            if (opcode < 0x10) position += Serial.print('0');
            position += Serial.print(opcode, HEX);
            position += Serial.print(F(":  "));
            position += Serial.print(OPCODE_NAME((PLCRuntimeInstructionSet) opcode));
            uint8_t size = OPCODE_SIZE((PLCRuntimeInstructionSet) opcode);
            if (size > 0) {
                for (; position < 26; position++) Serial.print(' ');
                Serial.print(F("("));
                Serial.print(size);
                Serial.print(F(" byte"));
                if (size > 1) Serial.print('s');
                Serial.print(')');
            }
            Serial.println();
        }
        if (opcode == 0xFF) break;
    }
#ifdef __RUNTIME_DEBUG__
    REPRINTLN(50, '-');
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
    Serial.println(F("Runtime Unit Tests Completed."));
#endif
    REPRINTLN(50, '-');
    Serial.println(F("Report:"));
    REPRINTLN(50, '-');
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
    REPRINTLN(50, '-');
    Serial.println(F("Runtime Unit Tests Report Completed."));
    REPRINTLN(50, '-');
};

#else // __RUNTIME_UNIT_TEST__

bool runtime_test_called = false;
void runtime_test() {
    if (runtime_test_called) return;
    Serial.println(F("Unit tests are disabled."));
    runtime_test_called = true;
};
class UnitTest {
public:
    static RuntimeError fullProgramDebug(VovkPLCRuntime& runtime, RuntimeProgram& program) {
        program.print();
        Serial.println(F("Runtime working in production mode. Full program debugging is disabled."));
        runtime.clear(program);
        runtime.cleanRun(program);
        return program.status;
    }
};

#endif // __RUNTIME_UNIT_TEST__