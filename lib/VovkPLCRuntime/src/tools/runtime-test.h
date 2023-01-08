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
    RuntimeProgram* program = nullptr;
    VovkPLCRuntime* runtime = nullptr;
    UnitTest() {
        program = new RuntimeProgram(program_size);
        runtime = new VovkPLCRuntime(stack_size, memory_size, *program);
        program->begin();
        program->format(program_size);
        runtime->formatMemory(memory_size);
    }
    ~UnitTest() {
        delete program;
        delete runtime;
    }
#ifdef __RUNTIME_DEBUG__
    template <typename T> void run(const TestCase<T>& test) {
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

    template <typename T> void review(const TestCase<T>& test) {
        program->format();
        if (test.build) test.build(*program);
        size_t offset = Serial.print(F("Test \""));
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

    template <typename T> void println(T result) { Serial.println(result); }
    void println(uint64_t result) { println__uint64_t(result); }
    void println(int64_t result) { println__int64_t(result); }
};

UnitTest* Tester = nullptr;

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
const TestCase<uint64_t> case_demo_uint64_t({ "demo_uint64_t => (1 + 2) * 3", STATUS_SUCCESS, 9, [](RuntimeProgram& program) {
    program.push_uint64_t(1);
    program.push_uint64_t(2);
    program.push(ADD, type_uint64_t);
    program.push_uint64_t(3);
    program.push(MUL, type_uint64_t);
} });

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
const TestCase<double> case_demo_double({ "demo_double => (0.1 + 0.2) * -1", STATUS_SUCCESS, -0.30000000000000004, [](RuntimeProgram& program) {
    program.push_double(0.1);
    program.push_double(0.2);
    program.push(ADD, type_double);
    program.push_double(-1);
    program.push(MUL, type_double);
} });

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
const TestCase<uint8_t> case_jump_if({ "jump_if => for loop sum", PROGRAM_EXITED, 100, [](RuntimeProgram& program) {
    // uint8_t sum = 0; 
    // for (uint8_t i = 0; i < 10; i++) {
    //   sum += (uint8_t) 10;
    // }
    // return sum;
    static const uint8_t sum_ptr = 0;
    static const uint8_t i_ptr = 1;
    uint16_t loop_jump = 0;
    uint16_t loop_destination = 0;
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


void runtime_unit_test() {
    int mem = 0;
    mem = get_used_memory(); Serial.print(F("  Used memory [ 1]: ")); Serial.print(mem); Serial.println(F(" bytes."));
    Tester = new UnitTest();
    mem = get_used_memory(); Serial.print(F("  Used memory [ 2]: ")); Serial.print(mem); Serial.println(F(" bytes."));
    Tester->runtime->startup();
    mem = get_used_memory(); Serial.print(F("  Used memory [ 3]: ")); Serial.print(mem); Serial.println(F(" bytes."));
    REPRINTLN(70, '-');
    Serial.println(F("Runtime Unit Test"));
    REPRINTLN(70, '-');
    mem = get_used_memory(); Serial.print(F("  Used memory [ 4]: ")); Serial.print(mem); Serial.println(F(" bytes."));
#ifdef __RUNTIME_DEBUG__
    logRuntimeErrorList();
    mem = get_used_memory(); Serial.print(F("  Used memory [ 6]: ")); Serial.print(mem); Serial.println(F(" bytes."));
    REPRINTLN(70, '-');
    logRuntimeInstructionSet();
    mem = get_used_memory(); Serial.print(F("  Used memory [ 7]: ")); Serial.print(mem); Serial.println(F(" bytes."));
    REPRINTLN(70, '-');
    REPRINTLN(70, '#');
    Serial.println(F("Executing Runtime Unit Tests..."));
    mem = get_used_memory(); Serial.print(F("  Used memory [ 8]: ")); Serial.print(mem); Serial.println(F(" bytes."));
    Tester->run(case_demo_uint8_t);
    Tester->run(case_demo_uint16_t);
    Tester->run(case_demo_uint32_t);
    Tester->run(case_demo_uint64_t);
    Tester->run(case_demo_int8_t);
    Tester->run(case_demo_float);
    Tester->run(case_demo_double);
    Tester->run(case_bitwise_and_X8);
    Tester->run(case_bitwise_and_X16);
    Tester->run(case_logic_and);
    Tester->run(case_logic_or);
    Tester->run(case_cmp_eq_1);
    Tester->run(case_cmp_eq_2);
    Tester->run(case_jump);
    Tester->run(case_jump_if);
    mem = get_used_memory(); Serial.print(F("  Used memory [ 9]: ")); Serial.print(mem); Serial.println(F(" bytes."));
    REPRINTLN(70, '-');
    REPRINTLN(70, '#');
    Serial.println(F("Runtime Unit Tests Completed."));
    REPRINTLN(70, '#');
#endif
    REPRINTLN(70, '#');
    Serial.println(F("Report:"));
    REPRINTLN(70, '#');
    Tester->review(case_demo_uint8_t);
    Tester->review(case_demo_uint16_t);
    Tester->review(case_demo_uint32_t);
    Tester->review(case_demo_uint64_t);
    Tester->review(case_demo_int8_t);
    Tester->review(case_demo_float);
    Tester->review(case_demo_double);
    Tester->review(case_bitwise_and_X8);
    Tester->review(case_bitwise_and_X16);
    Tester->review(case_logic_and);
    Tester->review(case_logic_or);
    Tester->review(case_cmp_eq_1);
    Tester->review(case_cmp_eq_2);
    Tester->review(case_jump);
    Tester->review(case_jump_if);
    mem = get_used_memory(); Serial.print(F("  Used memory [10]: ")); Serial.print(mem); Serial.println(F(" bytes."));
    REPRINTLN(70, '#');
    Serial.println(F("Runtime Unit Tests Report Completed."));
    REPRINTLN(70, '#');
    delete Tester;
    Tester = nullptr;
    mem = get_used_memory(); Serial.print(F("  Used memory [11]: ")); Serial.print(mem); Serial.println(F(" bytes."));
};

#else // __RUNTIME_UNIT_TEST__

bool runtime_test_called = false;
void runtime_unit_test() {
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



















/*
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Using VovkPLCRuntime Library - Author J.Vovk <jozo132@gmail.com> ::
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
----------------------------------------------------------------------
Runtime Unit Test
----------------------------------------------------------------------
RuntimeError Status List:

     0: STATUS_SUCCESS
     1: UNKNOWN_INSTRUCTION
     2: INVALID_INSTRUCTION
     3: INVALID_DATA_TYPE
     4: STACK_OVERFLOW
     5: STACK_UNDERFLOW
     6: INVALID_STACK_SIZE
     7: EMPTY_STACK
     8: CALL_STACK_OVERFLOW
     9: CALL_STACK_UNDERFLOW
    10: INVALID_MEMORY_ADDRESS
    11: INVALID_MEMORY_SIZE
    12: UNDEFINED_STATE
    13: INVALID_PROGRAM_INDEX
    14: PROGRAM_SIZE_EXCEEDED
    15: PROGRAM_POINTER_OUT_OF_BOUNDS
    16: PROGRAM_EXITED
    17: EMPTY_PROGRAM
    18: NO_PROGRAM
    19: INVALID_CHECKSUM
    20: EXECUTION_ERROR
    21: EXECUTION_TIMEOUT
    22: MEMORY_ACCESS_ERROR

----------------------------------------------------------------------
Bytecode Instruction Set:

    0x00:  NOP            (1 byte)

    0x01:  PUSH boolean   (2 bytes)
    0x02:  PUSH uint8_t   (2 bytes)
    0x03:  PUSH uint16_t  (3 bytes)
    0x04:  PUSH uint32_t  (5 bytes)
    0x05:  PUSH uint64_t  (9 bytes)
    0x06:  PUSH int8_t    (2 bytes)
    0x07:  PUSH int16_t   (3 bytes)
    0x08:  PUSH int32_t   (5 bytes)
    0x09:  PUSH int64_t   (9 bytes)
    0x0a:  PUSH float     (5 bytes)
    0x0b:  PUSH double    (9 bytes)

    0x10:  MOV            (8 bytes)
    0x11:  PUT            (4 bytes)
    0x12:  GET            (4 bytes)

    0x20:  ADD            (2 bytes)
    0x21:  SUB            (2 bytes)
    0x22:  MUL            (2 bytes)
    0x23:  DIV            (2 bytes)

    0x60:  BW_AND_X8      (1 byte)
    0x61:  BW_AND_X16     (1 byte)
    0x62:  BW_AND_X32     (1 byte)
    0x63:  BW_AND_X64     (1 byte)
    0x64:  BW_OR_X8       (1 byte)
    0x65:  BW_OR_X16      (1 byte)
    0x66:  BW_OR_X32      (1 byte)
    0x67:  BW_OR_X64      (1 byte)
    0x68:  BW_XOR_X8      (1 byte)
    0x69:  BW_XOR_X16     (1 byte)
    0x6a:  BW_XOR_X32     (1 byte)
    0x6b:  BW_XOR_X64     (1 byte)
    0x6c:  BW_NOT_X8      (1 byte)
    0x6d:  BW_NOT_X16     (1 byte)
    0x6e:  BW_NOT_X32     (1 byte)
    0x6f:  BW_NOT_X64     (1 byte)
    0x70:  BW_LSHIFT_X8   (1 byte)
    0x71:  BW_LSHIFT_X16  (1 byte)
    0x72:  BW_LSHIFT_X32  (1 byte)
    0x73:  BW_LSHIFT_X64  (1 byte)
    0x74:  BW_RSHIFT_X8   (1 byte)
    0x75:  BW_RSHIFT_X16  (1 byte)
    0x76:  BW_RSHIFT_X32  (1 byte)
    0x77:  BW_RSHIFT_X64  (1 byte)

    0x80:  LOGIC_AND      (1 byte)
    0x81:  LOGIC_OR       (1 byte)
    0x82:  LOGIC_XOR      (1 byte)
    0x83:  LOGIC_NOT      (1 byte)

    0xa0:  CMP_EQ         (2 bytes)
    0xa1:  CMP_NEQ        (2 bytes)
    0xa2:  CMP_GT         (2 bytes)
    0xa3:  CMP_LT         (2 bytes)
    0xa4:  CMP_GTE        (2 bytes)
    0xa5:  CMP_LTE        (2 bytes)

    0xc0:  JMP            (3 bytes)
    0xc1:  JMP_IF         (3 bytes)
    0xc2:  JMP_IF_NOT     (3 bytes)
    0xc3:  CALL           (3 bytes)
    0xc4:  CALL_IF        (3 bytes)
    0xc5:  CALL_IF_NOT    (3 bytes)
    0xc6:  RET            (1 byte)

    0xff:  EXIT           (1 byte)

----------------------------------------------------------------------
######################################################################
Executing Runtime Unit Tests...

######################################################################
Erasing program ...
Building program ...
Running test:
Program[10] [02 01 02 02 20 02 02 03 22 02]
#### Program Explanation:
       0 [00 00] - 02: PUSH uint8_t  [size  2] 0201
       2 [00 02] - 02: PUSH uint8_t  [size  2] 0202
       4 [00 04] - 20: ADD --------- [size  2] 2002
       6 [00 06] - 02: PUSH uint8_t  [size  2] 0203
       8 [00 08] - 22: MUL --------- [size  2] 2202
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Executed in [ 0 ms]  Opcode[02]: PUSH uint8_t   [02 01]  --------------------------  Stack( 0) []
    Step    2 [00 02]: Executed in [ 0 ms]  Opcode[02]: PUSH uint8_t   [02 02]  --------------------------  Stack( 0) []
    Step    4 [00 04]: Executed in [ 0 ms]  Opcode[20]: ADD  --------  [20 02]  --------------------------  Stack( 0) []
    Step    6 [00 06]: Executed in [ 0 ms]  Opcode[02]: PUSH uint8_t   [02 03]  --------------------------  Stack( 0) []
    Step    8 [00 08]: Executed in [ 0 ms]  Opcode[22]: MUL  --------  [22 02]  --------------------------  Stack( 0) []
Leftover Stack( 0) []
Time to execute program: 0 ms
Program result: 0
Expected result: 34
Test status: --------------------> !!! FAILED !!! <--------------------


######################################################################
Erasing program ...
Building program ...
Running test:
Program[13] [03 00 01 03 00 02 20 03 03 00 03 22 03]
#### Program Explanation:
       0 [00 00] - 03: PUSH uint16_t  [size  3] 030001
       3 [00 03] - 03: PUSH uint16_t  [size  3] 030002
       6 [00 06] - 20: ADD --------- [size  2] 2003
       8 [00 08] - 03: PUSH uint16_t  [size  3] 030003
      11 [00 0b] - 22: MUL --------- [size  2] 2203
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Executed in [ 0 ms]  Opcode[03]: PUSH uint16_t  [03 00 01]  -----------------------  Stack( 0) []
    Step    3 [00 03]: Executed in [ 0 ms]  Opcode[03]: PUSH uint16_t  [03 00 02]  -----------------------  Stack( 0) []
    Step    6 [00 06]: Executed in [ 0 ms]  Opcode[20]: ADD  --------  [20 03]  --------------------------  Stack( 0) []
    Step    8 [00 08]: Executed in [ 0 ms]  Opcode[03]: PUSH uint16_t  [03 00 03]  -----------------------  Stack( 0) []
    Step   11 [00 0b]: Executed in [ 0 ms]  Opcode[22]: MUL  --------  [22 03]  --------------------------  Stack( 0) []
Leftover Stack( 0) []
Time to execute program: 0 ms
Program result: 0
Expected result: 3
Test status: --------------------> !!! FAILED !!! <--------------------


######################################################################
Erasing program ...
Building program ...
Running test: 
Program[19] [04 00 00 00 01 04 00 00 00 02 20 04 00 00 00 00 03 22 04]
#### Program Explanation:
       0 [00 00] - 04: PUSH uint32_t  [size  5] 0400000001
       5 [00 05] - 04: PUSH uint32_t  [size  5] 0400000002
      10 [00 0a] - 20: ADD --------- [size  2] 2004
      12 [00 0c] - 00: NOP --------- [size  1] 00
      13 [00 0d] - 00: NOP --------- [size  1] 00
      14 [00 0e] - 00: NOP --------- [size  1] 00
      15 [00 0f] - 00: NOP --------- [size  1] 00
      16 [00 10] - 03: PUSH uint16_t  [size  3] 032204
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Executed in [ 0 ms]  Opcode[04]: PUSH uint32_t  [04 00 00 00 01]  -----------------  Stack( 4) [00 00 00 01]
    Step    5 [00 05]: Executed in [ 0 ms]  Opcode[04]: PUSH uint32_t  [04 00 00 00 02]  -----------------  Stack( 8) [00 00 00 01 00 00 00 02]
    Step   10 [00 0a]: Executed in [ 0 ms]  Opcode[20]: ADD  --------  [20 04]  --------------------------  Stack( 4) [00 00 00 03]
    Step   12 [00 0c]: Executed in [ 0 ms]  Opcode[08]: PUSH int32_t   [08 00 00 00 03]  -----------------  Stack( 8) [00 00 00 03 00 00 00 03]
    Step   17 [00 11]: Executed in [ 0 ms]  Opcode[22]: MUL  --------  [22 04]  --------------------------  Stack( 4) [00 00 00 09]
Leftover Stack( 4) [00 00 00 09]
Time to execute program: 0 ms
Program result: 9
Expected result: 69206528
Test status: --------------------> !!! FAILED !!! <--------------------


######################################################################
Erasing program ...
Building program ...
Running test:
Program[31] [05 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 20 05 05 00 00 00 00 00 00 00 03 22 05]
#### Program Explanation:
       0 [00 00] - 05: PUSH uint64_t  [size  9] 050000000000000000
       9 [00 09] - 00: NOP --------- [size  1] 00
      10 [00 0a] - 00: NOP --------- [size  1] 00
      11 [00 0b] - 00: NOP --------- [size  1] 00
      12 [00 0c] - 00: NOP --------- [size  1] 00
      13 [00 0d] - 00: NOP --------- [size  1] 00
      14 [00 0e] - 00: NOP --------- [size  1] 00
      15 [00 0f] - 00: NOP --------- [size  1] 00
      16 [00 10] - 00: NOP --------- [size  1] 00
      17 [00 11] - 02: PUSH uint8_t  [size  2] 0220
      19 [00 13] - 05: PUSH uint64_t  [size  9] 050500000000000000
      28 [00 1c] - 03: PUSH uint16_t  [size  3] 032205
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Error at program pointer 0: STACK_OVERFLOW
*/