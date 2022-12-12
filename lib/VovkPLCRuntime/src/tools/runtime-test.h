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

#define TRY_CATCH(error, code) {        \
  if (error) {                          \
    uint16_t counter = 0;               \
    while (true) {                      \
      if (counter == 0) {               \
        code                            \
      }                                 \
      digitalWrite(LED_BUILTIN, HIGH);  \
      delay(100);                       \
      digitalWrite(LED_BUILTIN, LOW);   \
      delay(100);                       \
      counter = (counter + 1) % 10;     \
    }                                   \
  }                                     \
}

#ifdef __RUNTIME_TEST__

#warning "RUNTIME TEST ENABLED - Pleas do not use this in production code. This is only for testing purposes and might cause unexpected behaviour."

#ifdef __RUNTIME_TEST_ADVANCED_DEBUGGING__
#warning "RUNTIME TEST ADVANCED DEBUGGING ENABLED - Pleas do not use this in production code. This is only for testing purposes and might cause unexpected behaviour."
#endif

template <typename T>
struct RuntimeTestCase {
    const char* name;
    RuntimeError expected_error;
    T expected_result;
    // function pointer for startup that takes a RuntimeProgram & type and returns void 
    void (*build)(RuntimeProgram&);
};


void print_U64(uint64_t big_number) {
    const uint16_t NUM_DIGITS = log10(big_number) + 1;
    char sz[NUM_DIGITS + 1];
    sz[NUM_DIGITS] = 0;
    for (uint16_t i = NUM_DIGITS; i--; big_number /= 10)
        sz[i] = '0' + (big_number % 10);
    Serial.print(sz);
}
void println_U64(uint64_t big_number) {
    print_U64(big_number);
    Serial.println();
}

void print_S64(int64_t big_number) {
    if (big_number < 0) {
        Serial.print('-');
        big_number = -big_number;
    }
    print_U64(big_number);
}
void println_S64(int64_t big_number) {
    print_S64(big_number);
    Serial.println();
}

struct RuntimeTest_t {
    static const uint16_t memory_size = 16;
    static const uint16_t stack_size = 32;
    static const uint16_t program_size = 64;
    uint8_t memory[memory_size];
    VovkPLCRuntime runtime = VovkPLCRuntime(stack_size, memory, memory_size);
    RuntimeProgram program = RuntimeProgram(program_size);
    void initialize() {
        runtime.clear(program);
        program.erase();
    }
    template <typename T> void run(const RuntimeTestCase<T>& test) {
        Serial.println("----------------------------------------");
        initialize();
        test.build(program);
        Serial.print(F("Running test: ")); Serial.println(test.name);
        program.println();
        uint16_t program_pointer = 0;
        bool finished = false;
        while (!finished) {
            program_pointer = program.getLine();
            long t = micros();
            RuntimeError status = runtime.step(program);
            t = micros() - t;
            float ms = (float) t * 0.001;
            if (status == RTE_PROGRAM_EXITED) finished = true;
            bool problem = status != RTE_SUCCESS && status != RTE_PROGRAM_EXITED;
            TRY_CATCH(problem, {
                    const char* error = getRuntimeErrorName(status);
                    Serial.print(F("Error at program pointer "));
                    Serial.print(program_pointer);
                    Serial.print(F(": "));
                    Serial.println(error);
                });
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
        float ms = (float) t * 0.001;
        Serial.print(F("Leftover ")); runtime.printStack(); Serial.println();
        T output = runtime.read<T>();
        Serial.print(F("Time to execute program: ")); Serial.print(ms, 3); Serial.println(F(" ms"));
        print(test, output);
        Serial.print(F("Test passed: ")); Serial.println(test.expected_result == output ? F("YES") : F("NO - TEST DID NOT PASS !!!"));
    }

    template <typename T> void print(const RuntimeTestCase<T>& test, T result) {
        Serial.print(F("Expected result: ")); Serial.println(test.expected_result);
        Serial.print(F("Actual result: ")); Serial.println(result);
    }

    void print(const RuntimeTestCase<uint64_t>& test, uint64_t result) {
        Serial.print(F("Expected result: ")); println_U64(test.expected_result);
        Serial.print(F("Actual result: ")); println_U64(result);
    }

    void print(const RuntimeTestCase<int64_t>& test, int64_t result) {
        Serial.print(F("Expected result: ")); println_S64(test.expected_result);
        Serial.print(F("Actual result: ")); println_S64(result);
    }
} Tester;


RuntimeTestCase<uint8_t> case_add_U8 = { "add_U8 => (1 + 2) * 3", RTE_SUCCESS, 9, [](RuntimeProgram& program) {
    program.pushU8(1);
    program.pushU8(2);
    program.push(ADD, U8);
    program.pushU8(3);
    program.push(MUL, U8);
} };
RuntimeTestCase<uint16_t> case_add_U16 = { "add_U16 => (1 + 2) * 3", RTE_SUCCESS, 9, [](RuntimeProgram& program) {
    program.pushU16(1);
    program.pushU16(2);
    program.push(ADD, U16);
    program.pushU16(3);
    program.push(MUL, U16);
} };
#ifdef __RUNTIME_TEST_ADVANCED_DEBUGGING__
RuntimeTestCase<uint32_t> case_add_U32 = { "add_U32 => (1 + 2) * 3", RTE_SUCCESS, 9, [](RuntimeProgram& program) {
    program.pushU32(1);
    program.pushU32(2);
    program.push(ADD,U32);
    program.pushU32(3);
    program.push(MUL,U32);
} };
RuntimeTestCase<uint64_t> case_add_U64 = { "add_U64 => (1 + 2) * 3", RTE_SUCCESS, 9, [](RuntimeProgram& program) {
    program.pushU64(1);
    program.pushU64(2);
    program.push(ADD, U64);
    program.pushU64(3);
    program.push(MUL, U64);
} };
#endif

RuntimeTestCase<int8_t> case_sub_S8 = { "sub_S8 => (1 - 2) * 3", RTE_SUCCESS, -3, [](RuntimeProgram& program) {
    program.pushS8(1);
    program.pushS8(2);
    program.push(SUB,S8);
    program.pushS8(3);
    program.push(MUL,S8);
} };
RuntimeTestCase<int16_t> case_sub_S16 = { "sub_S16 => (1 - 2) * 3", RTE_SUCCESS, -3, [](RuntimeProgram& program) {
    program.pushS16(1);
    program.pushS16(2);
    program.push(SUB,S16);
    program.pushS16(3);
    program.push(MUL,S16);
} };

#ifdef __RUNTIME_TEST_ADVANCED_DEBUGGING__
RuntimeTestCase<int32_t> case_sub_S32 = { "sub_S32 => (1 - 2) * 3", RTE_SUCCESS, -3, [](RuntimeProgram& program) {
    program.pushS32(1);
    program.pushS32(2);
    program.push(SUB,S32);
    program.pushS32(3);
    program.push(MUL,S32);
} };
RuntimeTestCase<int64_t> case_sub_S64 = { "sub_S64 => (1 - 2) * 3", RTE_SUCCESS, -3, [](RuntimeProgram& program) {
    program.pushS64(1);
    program.pushS64(2);
    program.push(SUB,S64);
    program.pushS64(3);
    program.push(MUL,S64);
} };
#endif

RuntimeTestCase<float> case_sub_F32 = { "sub_F32 => (0.1 + 0.2) * -1", RTE_SUCCESS, -0.3, [](RuntimeProgram& program) {
    program.pushF32(0.1);
    program.pushF32(0.2);
    program.push(ADD,F32);
    program.pushF32(-1);
    program.push(MUL,F32);
} };
#ifdef __RUNTIME_TEST_ADVANCED_DEBUGGING__
RuntimeTestCase<double> case_sub_F64 = { "sub_F64 => (0.1 + 0.2) * -1", RTE_SUCCESS, -0.3, [](RuntimeProgram& program) {
    program.pushF64(0.1);
    program.pushF64(0.2);
    program.push(ADD,F64);
    program.pushF64(-1);
    program.push(MUL,F64);
} };
#endif

// Bitwise operations
RuntimeTestCase<uint8_t> case_bitwise_and_X8 = { "bitwise_and_X8", RTE_SUCCESS, 0b00000101, [](RuntimeProgram& program) {
    program.pushU8(0b00001111);
    program.pushU8(0b01010101);
    program.push(BW_AND_X8);
} };
RuntimeTestCase<uint16_t> case_bitwise_and_X16 = { "bitwise_and_X16", RTE_SUCCESS, 0x000F, [](RuntimeProgram& program) {
    program.pushU16(0x00FF);
    program.pushU16(0xF00F);
    program.push(BW_AND_X16);
} };
#ifdef __RUNTIME_TEST_ADVANCED_DEBUGGING__
RuntimeTestCase<uint32_t> case_bitwise_and_X32 = { "bitwise_and_X32", RTE_SUCCESS, 0x0F0F0000, [](RuntimeProgram& program) {
    program.pushU32(0x0F0F0F0F);
    program.pushU32(0xFFFF0000);
    program.push(BW_AND_X32);
} };
RuntimeTestCase<uint64_t> case_bitwise_and_X64 = { "bitwise_and_X64", RTE_SUCCESS, 0b00000101, [](RuntimeProgram& program) {
    program.pushU64(0b00001111);
    program.pushU64(0b01010101);
    program.push(BW_AND_X64);
} };

// Logic (boolean) operations
RuntimeTestCase<bool> case_logic_and = { "logic_and => true && false", RTE_SUCCESS, false, [](RuntimeProgram& program) {
    program.pushBool(true);
    program.pushBool(false);
    program.push(LOGIC_AND);
} };
#endif
RuntimeTestCase<bool> case_logic_and_2 = { "logic_and => true && true", RTE_SUCCESS, true, [](RuntimeProgram& program) {
    program.pushBool(true);
    program.pushBool(true);
    program.push(LOGIC_AND);
} };
RuntimeTestCase<bool> case_logic_or = { "logic_or => true || false", RTE_SUCCESS, true, [](RuntimeProgram& program) {
    program.pushBool(true);
    program.pushBool(false);
    program.push(LOGIC_OR);
} };

#ifdef __RUNTIME_TEST_ADVANCED_DEBUGGING__
RuntimeTestCase<bool> case_logic_or_2 = { "logic_or => false || false", RTE_SUCCESS, false, [](RuntimeProgram& program) {
    program.pushBool(false);
    program.pushBool(false);
    program.push(LOGIC_OR);
} };
#endif

// Comparison operations
RuntimeTestCase<bool> case_cmp_eq = { "cmp_eq => 1 == 1", RTE_SUCCESS, true, [](RuntimeProgram& program) {
    program.pushBool(1);
    program.pushBool(1);
    program.push(CMP_EQ, BOOL);
} };

#ifdef __RUNTIME_TEST_ADVANCED_DEBUGGING__
RuntimeTestCase<bool> case_cmp_eq_2 = { "cmp_eq => 0.3 == 0.3", RTE_SUCCESS, true, [](RuntimeProgram& program) {
    program.pushF32(0.3);
    program.pushF32(0.3);
    program.push(CMP_EQ, F32);
} };
RuntimeTestCase<bool> case_cmp_eq_3 = { "cmp_eq => 0.29 == 0.31", RTE_SUCCESS, false, [](RuntimeProgram& program) {
    program.pushF32(0.29);
    program.pushF32(0.31);
    program.push(CMP_EQ, F32);
} };
#endif

// Jump operations
RuntimeTestCase<uint8_t> case_jump = { "jump => 1", RTE_SUCCESS, 1, [](RuntimeProgram& program) {
    program.pushU8(1); // 0 [+2]
    program.pushJMP(13); // 2 [+3]
    program.pushU8(1); // 5 [+2]
    program.push(ADD, U8); // 7 [+2]
    program.pushU8(3); // 9 [+2]
    program.push(MUL, U8); // 11 [+2]
    program.push(EXIT); // 13 [+1]
} };


void runtime_test() {
    Tester.run(case_add_U8);
    Tester.run(case_add_U16);
#ifdef __RUNTIME_TEST_ADVANCED_DEBUGGING__
    Tester.run(case_add_U32);
    Tester.run(case_add_U64);
#endif
    Tester.run(case_sub_S8);
    Tester.run(case_sub_S16);
#ifdef __RUNTIME_TEST_ADVANCED_DEBUGGING__
    Tester.run(case_sub_S32);
    Tester.run(case_sub_S64);
#endif
    Tester.run(case_sub_F32);
#ifdef __RUNTIME_TEST_ADVANCED_DEBUGGING__
    Tester.run(case_sub_F64);
#endif

    Tester.run(case_bitwise_and_X8);
    Tester.run(case_bitwise_and_X16);
#ifdef __RUNTIME_TEST_ADVANCED_DEBUGGING__
    Tester.run(case_bitwise_and_X32);
    Tester.run(case_bitwise_and_X64);
#endif

#ifdef __RUNTIME_TEST_ADVANCED_DEBUGGING__
    Tester.run(case_logic_and);
#endif
    Tester.run(case_logic_and_2);
    Tester.run(case_logic_or);

#ifdef __RUNTIME_TEST_ADVANCED_DEBUGGING__
    Tester.run(case_logic_or_2);
#endif

    Tester.run(case_cmp_eq);
#ifdef __RUNTIME_TEST_ADVANCED_DEBUGGING__
    Tester.run(case_cmp_eq_2);
    Tester.run(case_cmp_eq_3);
#endif
    Tester.run(case_jump);

    Serial.println(F("----------------------------------------"));
}

#else // __RUNTIME_TEST__

bool runtime_test_called = false;
void runtime_test() {
    if (runtime_test_called) return;
    Serial.println("Runtime working in production mode. Unit tests are disabled");
    runtime_test_called = true;
}

#endif // __RUNTIME_TEST__