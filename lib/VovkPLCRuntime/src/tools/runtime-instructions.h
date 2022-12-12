// runtime-instructions.h - 1.0.0 - 2022-12-11
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

// RuntimeError status codes
enum RuntimeError {
    RTE_SUCCESS = 0,
    RTE_STACK_OVERFLOW,
    RTE_STACK_UNDERFLOW,
    RTE_CALL_STACK_OVERFLOW,
    RTE_CALL_STACK_UNDERFLOW,
    RTE_INVALID_INSTRUCTION,
    RTE_INVALID_DATA_TYPE,
    RTE_INVALID_MEMORY_ADDRESS,
    RTE_INVALID_MEMORY_SIZE,
    RTE_INVALID_STACK_SIZE,
    RTE_EMPTY_STACK,
    RTE_EXECUTION_ERROR,
    RTE_EXECUTION_TIMEOUT,
    RTE_PROGRAM_SIZE_EXCEEDED,
    RTE_UNKNOWN_INSTRUCTION,
    RTE_INVALID_LINE_NUMBER,
    RTE_EMPTY_PROGRAM,
    RTE_PROGRAM_POINTER_OUT_OF_BOUNDS,
    RTE_PROGRAM_EXITED,
};

void fstrcpy(char* buff, const char* fstr) {
    uint8_t i, c = 0;
    for (;;) {
        c = pgm_read_byte(fstr + i);
        buff[i++] = c;
        if (c == 0) return;
        if (i > 254) {
            buff[255] = 0;
            return;
        }
    }
}

// RuntimeError status names for printing stored in PROGMEM
const char* const RuntimeErrorNames [] PROGMEM = {
    "SUCCESS",
    "STACK_OVERFLOW",
    "STACK_UNDERFLOW",
    "CALL_STACK_OVERFLOW",
    "CALL_STACK_UNDERFLOW",
    "INVALID_INSTRUCTION",
    "INVALID_DATA_TYPE",
    "INVALID_MEMORY_ADDRESS",
    "INVALID_MEMORY_SIZE",
    "INVALID_STACK_SIZE",
    "EMPTY_STACK",
    "EXECUTION_ERROR",
    "EXECUTION_TIMEOUT",
    "PROGRAM_SIZE_EXCEEDED",
    "UNKNOWN_INSTRUCTION",
    "INVALID_LINE_NUMBER",
    "EMPTY_PROGRAM",
    "PROGRAM_POINTER_OUT_OF_BOUNDS",
    "PROGRAM_EXITED",
};

const char* const unknows_error_code_str PROGMEM = "UNKNOWN_ERROR_CODE";

const char* getRuntimeErrorName(RuntimeError error) {
    char* output = new char[35];
    if (error < 0 || error > 18) fstrcpy(output, unknows_error_code_str);
    else fstrcpy(output, RuntimeErrorNames[error]);
    return output;
}

union U32_to_F32 {
    uint32_t U32;
    float F32;
};

union U64_to_F64 {
    uint64_t U64;
    double F64;
};

union U8A_to_U16 {
    uint8_t U8A[2];
    uint16_t U16;
};


class MyStack {
public:
    Stack<uint8_t>* stack;
    Stack<uint16_t>* call_stack;
    uint16_t max_size = 0;
    uint16_t max_call_stack_size = 0;
    // Create a stack with a maximum size
    MyStack(uint16_t max_size, uint16_t call_stack_size = 10) {
        stack = new Stack<uint8_t>(max_size);
        call_stack = new Stack<uint16_t>(call_stack_size);
        this->max_size = max_size;
        this->max_call_stack_size = call_stack_size;
    }


    void print() { stack->print(); }
    void println() { stack->println(); }

    RuntimeError pushCall(uint16_t return_address) {
        if (call_stack->size() >= max_call_stack_size) return RTE_STACK_OVERFLOW;
        call_stack->push(return_address);
        return RTE_SUCCESS;
    }

    uint16_t popCall() {
        if (call_stack->size() == 0) return RTE_STACK_UNDERFLOW;
        uint16_t return_address = call_stack->pop();
        return return_address;
    }

    // Push an uint8_t value to the stack
    RuntimeError push(uint8_t value) {
        if (stack->size() >= max_size) return RTE_STACK_OVERFLOW;
        stack->push(value);
        return RTE_SUCCESS;
    }
    // Pop an uint8_t value from the stack
    uint8_t pop() {
        uint8_t value = stack->pop();
        return value;
    }
    // Peek the top uint8_t value from the stack
    uint8_t peek() { return stack->peek(); }

    // Push a boolean value to the stack
    RuntimeError pushBool(bool value) {
        if (stack->size() >= max_size) return RTE_STACK_OVERFLOW;
        stack->push(value);
        return RTE_SUCCESS;
    }
    // Pop a boolean value from the stack
    bool popBool() {
        bool value = stack->pop();
        return value;
    }
    // Peek the top boolean value from the stack
    bool peekBool() { return stack->peek(); }


    // Push an uint8_t value to the stack
    RuntimeError pushU8(uint8_t value) {
        if (stack->size() + 1 > max_size) return RTE_STACK_OVERFLOW;
        stack->push(value);
        return RTE_SUCCESS;
    }
    // Pop an uint8_t value from the stack
    uint8_t popU8() {
        uint8_t value = stack->pop();
        return value;
    }
    // Peek the top uint8_t value from the stack
    uint8_t peekU8() { return stack->peek(); }

    // Push an uint16_t value to the stack
    RuntimeError pushU16(uint16_t value) {
        if (stack->size() + 2 > max_size) return RTE_STACK_OVERFLOW;
        stack->push(value >> 8);
        stack->push(value & 0xFF);
        return RTE_SUCCESS;
    }
    // Pop an uint16_t value from the stack
    uint16_t popU16() {
        uint16_t b = stack->pop();
        uint16_t a = stack->pop();
        return (a << 8) | b;
    }
    // Peek the top uint16_t value from the stack
    uint16_t peekU16() {
        uint16_t b = stack->peek(0);
        uint16_t a = stack->peek(1);
        return (a << 8) | b;
    }

    // Push an uint32_t value to the stack
    RuntimeError pushU32(uint32_t value) {
        if (stack->size() + 4 > max_size) return RTE_STACK_OVERFLOW;
        stack->push(value >> 24);
        stack->push((value >> 16) & 0xFF);
        stack->push((value >> 8) & 0xFF);
        stack->push(value & 0xFF);
        return RTE_SUCCESS;
    }
    // Pop an uint32_t value from the stack
    uint32_t popU32() {
        uint32_t d = stack->pop();
        uint32_t c = stack->pop();
        uint32_t b = stack->pop();
        uint32_t a = stack->pop();
        return (a << 24) | (b << 16) | (c << 8) | d;
    }
    // Peek the top uint32_t value from the stack
    uint32_t peekU32() {
        uint32_t d = stack->peek(0);
        uint32_t c = stack->peek(1);
        uint32_t b = stack->peek(2);
        uint32_t a = stack->peek(3);
        return (a << 24) | (b << 16) | (c << 8) | d;
    }

    // Push an uint64_t value to the stack
    RuntimeError pushU64(uint64_t value) {
        if (stack->size() + 8 > max_size) return RTE_STACK_OVERFLOW;
        stack->push(value >> 56);
        stack->push((value >> 48) & 0xFF);
        stack->push((value >> 40) & 0xFF);
        stack->push((value >> 32) & 0xFF);
        stack->push((value >> 24) & 0xFF);
        stack->push((value >> 16) & 0xFF);
        stack->push((value >> 8) & 0xFF);
        stack->push(value & 0xFF);
        return RTE_SUCCESS;
    }
    // Pop an uint64_t value from the stack
    uint64_t popU64() {
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
    uint64_t peekU64() {
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

    // Push an int8_t value to the stack
    RuntimeError pushS8(int8_t value) { return push((uint8_t) value); }
    // Pop an int8_t value from the stack
    int8_t popS8() { return (int8_t) pop(); }
    // Peek the top int8_t value from the stack
    int8_t peekS8() { return (int8_t) peek(); }

    // Push an int16_t value to the stack
    RuntimeError pushS16(int16_t value) { return pushU16((uint16_t) value); }
    // Pop an int16_t value from the stack
    int16_t popS16() { return (int16_t) popU16(); }
    // Peek the top int16_t value from the stack
    int16_t peekS16() { return (int16_t) peekU16(); }

    // Push an int32_t value to the stack
    RuntimeError pushS32(int32_t value) { return pushU32((uint32_t) value); }
    // Pop an int32_t value from the stack
    int32_t popS32() { return (int32_t) popU32(); }
    // Peek the top int32_t value from the stack
    int32_t peekS32() { return (int32_t) peekU32(); }

    // Push an int64_t value to the stack
    RuntimeError pushS64(int64_t value) { return pushU64((uint64_t) value); }
    // Pop an int64_t value from the stack
    int64_t popS64() { return (int64_t) popU64(); }
    // Peek the top int64_t value from the stack
    int64_t peekS64() { return (int64_t) peekU64(); }

    // Push a float value to the stack
    RuntimeError pushF32(float value) {
        U32_to_F32 cvt;
        cvt.F32 = value;
        return pushU32(cvt.U32);
    }
    // Pop a float value from the stack
    float popF32() {
        U32_to_F32 cvt;
        cvt.U32 = popU32();
        return cvt.F32;
    }
    // Peek the top float value from the stack
    float peekF32() {
        U32_to_F32 cvt;
        cvt.U32 = peekU32();
        return cvt.F32;
    }

    // Push a double value to the stack
    RuntimeError pushF64(double value) {
        U64_to_F64 cvt;
        cvt.F64 = value;
        return pushU64(cvt.U64);
    }
    // Pop a double value from the stack
    double popF64() {
        U64_to_F64 cvt;
        cvt.U64 = popU64();
        return cvt.F64;
    }
    // Peek the top double value from the stack
    double peekF64() {
        U64_to_F64 cvt;
        cvt.U64 = peekU64();
        return cvt.F64;
    }

    // Clear the stack
    void clear() {
        while (!stack->empty()) stack->pop();
        while (!call_stack->empty()) call_stack->pop();
    }
    uint16_t size() { return stack->size(); }
};



struct Extract_t {
    // Extract bool value from program bytecode
    RuntimeError BOOL(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, bool& value) {
        uint16_t size = 1;
        if (index + size > bytecode_size) return RTE_PROGRAM_SIZE_EXCEEDED;
        value = bytecode[index];
        index += size;
        return RTE_SUCCESS;
    }

    // Extract uint8_t value from program bytecode
    RuntimeError U8(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, uint8_t& value) {
        uint16_t size = 1;
        if (index + size > bytecode_size) return RTE_PROGRAM_SIZE_EXCEEDED;
        value = bytecode[index];
        index += size;
        return RTE_SUCCESS;
    }
    // Extract uint16_t value from program bytecode
    RuntimeError U16(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, uint16_t& value) {
        uint16_t size = 2;
        if (index + size > bytecode_size) return RTE_PROGRAM_SIZE_EXCEEDED;
        value = ((uint16_t) bytecode[index] << 8) | bytecode[index + 1];
        index += size;
        return RTE_SUCCESS;
    }
    // Extract uint32_t value from program bytecode
    RuntimeError U32(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, uint32_t& value) {
        uint16_t size = 4;
        if (index + size > bytecode_size) return RTE_PROGRAM_SIZE_EXCEEDED;
        value = ((uint32_t) bytecode[index] << 24) | ((uint32_t) bytecode[index + 1] << 16) | ((uint32_t) bytecode[index + 2] << 8) | bytecode[index + 3];
        index += 4;
        return RTE_SUCCESS;
    }
    // Extract uint64_t value from program bytecode
    RuntimeError U64(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, uint64_t& value) {
        uint16_t size = 8;
        if (index + size > bytecode_size) return RTE_PROGRAM_SIZE_EXCEEDED;
        value = ((uint64_t) bytecode[index] << 56) | ((uint64_t) bytecode[index + 1] << 48) | ((uint64_t) bytecode[index + 2] << 40) | ((uint64_t) bytecode[index + 3] << 32) | ((uint64_t) bytecode[index + 4] << 24) | ((uint64_t) bytecode[index + 5] << 16) | ((uint64_t) bytecode[index + 6] << 8) | bytecode[index + 7];
        index += size;
        return RTE_SUCCESS;
    }

    // Extract int8_t value from program bytecode
    RuntimeError S8(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, int8_t& value) {
        uint16_t size = 1;
        if (index + size > bytecode_size) return RTE_PROGRAM_SIZE_EXCEEDED;
        value = (int8_t) bytecode[index];
        index += size;
        return RTE_SUCCESS;
    }
    // Extract int16_t value from program bytecode
    RuntimeError S16(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, int16_t& value) {
        uint16_t size = 2;
        if (index + size > bytecode_size) return RTE_PROGRAM_SIZE_EXCEEDED;
        value = ((int16_t) bytecode[index] << 8) | bytecode[index + 1];
        index += size;
        return RTE_SUCCESS;
    }
    // Extract int32_t value from program bytecode
    RuntimeError S32(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, int32_t& value) {
        uint16_t size = 4;
        if (index + size > bytecode_size) return RTE_PROGRAM_SIZE_EXCEEDED;
        value = ((int32_t) bytecode[index] << 24) | ((int32_t) bytecode[index + 1] << 16) | ((int32_t) bytecode[index + 2] << 8) | bytecode[index + 3];
        index += size;
        return RTE_SUCCESS;
    }
    // Extract int64_t value from program bytecode
    RuntimeError S64(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, int64_t& value) {
        uint16_t size = 8;
        if (index + size > bytecode_size) return RTE_PROGRAM_SIZE_EXCEEDED;
        value = ((int64_t) bytecode[index] << 56) | ((int64_t) bytecode[index + 1] << 48) | ((int64_t) bytecode[index + 2] << 40) | ((int64_t) bytecode[index + 3] << 32) | ((int64_t) bytecode[index + 4] << 24) | ((int64_t) bytecode[index + 5] << 16) | ((int64_t) bytecode[index + 6] << 8) | bytecode[index + 7];
        index += size;
        return RTE_SUCCESS;
    }

    // Extract float value from program bytecode
    RuntimeError F32(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, float& value) {
        uint16_t size = 4;
        if (index + size > bytecode_size) return RTE_PROGRAM_SIZE_EXCEEDED;
        U32_to_F32 cvt;
        cvt.U32 = ((uint32_t) bytecode[index] << 24) | ((uint32_t) bytecode[index + 1] << 16) | ((uint32_t) bytecode[index + 2] << 8) | bytecode[index + 3];
        value = cvt.F32;
        index += size;
        return RTE_SUCCESS;
    }
    // Extract double value from program bytecode
    RuntimeError F64(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, double& value) {
        uint16_t size = 8;
        if (index + size > bytecode_size) return RTE_PROGRAM_SIZE_EXCEEDED;
        U64_to_F64 cvt;
        cvt.U64 = ((uint64_t) bytecode[index] << 56) | ((uint64_t) bytecode[index + 1] << 48) | ((uint64_t) bytecode[index + 2] << 40) | ((uint64_t) bytecode[index + 3] << 32) | ((uint64_t) bytecode[index + 4] << 24) | ((uint64_t) bytecode[index + 5] << 16) | ((uint64_t) bytecode[index + 6] << 8) | bytecode[index + 7];
        value = cvt.F64;
        index += size;
        return RTE_SUCCESS;
    }

} Extract;

// Reverse Polish Notation (RPN) to custom bytecode


// Readable RPN code example:
//                - bytecode []            - stack: []
//  (U8 4)        - bytecode [ 02, 04 ]    - stack: [4]    
//  (U8 6)        - bytecode [ 02, 06 ]    - stack: [4, 6]
//  (ADD U8)      - bytecode [ A1, 02 ]    - stack: [10]
//  (U8 2)        - bytecode [ 02, 02 ]    - stack: [10, 2]
//  (MUL U8)      - bytecode [ A3, 02 ]    - stack: [20]
//  (U8 5)        - bytecode [ 02, 05 ]    - stack: [20, 5]
//  (DIV U8)      - bytecode [ A4, 02 ]    - stack: [4]
//  (U8 3)        - bytecode [ 02, 03 ]    - stack: [4, 3]
//  (SUB U8)      - bytecode [ A2, 02 ]    - stack: [1]

// Bytecode buffer:
//  02 04 02 06 A1 02 02 02 A3 02 02 05 A4 02 02 03 A2 02


// Instruction set:
enum PLCRuntimeInstructionSet {
    NOP = 0x00,         // NOP - no operation

    // Data types
    BOOL,               // Constant boolean value
    U8,                 // Constant uint8_t value
    S8,                 // Constant int8_t value
    U16,                // Constant uint16_t integer value
    S16,                // Constant int16_t integer value
    U32,                // Constant uint32_t integer value
    S32,                // Constant int32_t integer value
    U64,                // Constant uint64_t integer value
    S64,                // Constant int64_t integer value
    F32,                // Constant float value
    F64,                // Constant double value

    POINTER = 0x10,     // Pointer (uint16_t) to variable, requires data type as argument

    // Arithmetic operations
    ADD = 0x20,         // Addition, requires data type as argument
    SUB,                // Subtraction, requires data type as argument
    MUL,                // Multiplication, requires data type as argument
    DIV,                // Division, requires data type as argument
    MOD,                // Modulo, requires data type as argument

    POW,                // Power for given type. Example: POW U8
    SQRT,               // Square root
    MIN,                // Minimum 
    MAX,                // Maximum 
    ABS,                // Absolute value for int8_t
    MAP,                // Map  (x, in_min, in_max, out_min, out_max)
    CON,                // Constrain  (x, min, max)
    RAND,               // Random  (full range)
    RAND1,              // Random  (max)
    RAND2,              // Random  (min, max)

    LN,                 // Natural logarithm
    LOG10,              // Base 10 logarithm
    LOG2,               // Base 2 logarithm for float
    EXP,                // Exponential
    SIN,                // Sine
    COS,                // Cosine
    TAN,                // Tangent
    ASIN,               // Arc sine
    ACOS,               // Arc cosine
    ATAN,               // Arc tangent
    ATAN2,              // Arc tangent 2
    SINH,               // Hyperbolic sine
    COSH,               // Hyperbolic cosine
    TANH,               // Hyperbolic tangent
    ASINH,              // Hyperbolic arc sine
    ACOSH,              // Hyperbolic arc cosine
    ATANH,              // Hyperbolic arc tangent

    BW_AND_X8 = 0x60,   // Bitwise AND for 1 byte size values (x, y)
    BW_AND_X16,         // Bitwise AND for 2 byte size values (x, y)
    BW_AND_X32,         // Bitwise AND for 4 byte size values (x, y)
    BW_AND_X64,         // Bitwise AND for 8 byte size values (x, y)

    BW_OR_X8,           // Bitwise OR for 1 byte size values (x, y)
    BW_OR_X16,          // Bitwise OR for 2 byte size values (x, y)
    BW_OR_X32,          // Bitwise OR for 4 byte size values (x, y)
    BW_OR_X64,          // Bitwise OR for 8 byte size values (x, y)

    BW_XOR_X8,          // Bitwise XOR for 1 byte size values (x, y)
    BW_XOR_X16,         // Bitwise XOR for 2 byte size values (x, y)
    BW_XOR_X32,         // Bitwise XOR for 4 byte size values (x, y)
    BW_XOR_X64,         // Bitwise XOR for 8 byte size values (x, y)

    BW_NOT_X8,          // Bitwise NOT for 1 byte size values (x)
    BW_NOT_X16,         // Bitwise NOT for 2 byte size values (x)
    BW_NOT_X32,         // Bitwise NOT for 4 byte size values (x)
    BW_NOT_X64,         // Bitwise NOT for 8 byte size values (x)

    BW_LSHIFT_X8,       // Bitwise left shift for 1 byte size values (x, y)
    BW_LSHIFT_X16,      // Bitwise left shift for 2 byte size values (x, y)
    BW_LSHIFT_X32,      // Bitwise left shift for 4 byte size values (x, y)
    BW_LSHIFT_X64,      // Bitwise left shift for 8 byte size values (x, y)

    BW_RSHIFT_X8,       // Bitwise right shift for 1 byte size values (x, y)
    BW_RSHIFT_X16,      // Bitwise right shift for 2 byte size values (x, y)
    BW_RSHIFT_X32,      // Bitwise right shift for 4 byte size values (x, y)
    BW_RSHIFT_X64,      // Bitwise right shift for 8 byte size values (x, y)

    // Logical operations
    LOGIC_AND,          // Logical AND for bool (x, y)
    LOGIC_OR,           // Logical OR for bool (x, y)
    LOGIC_XOR,          // Logical XOR for bool (x, y)
    LOGIC_NOT,          // Logical NOT for bool (x)

    // Comparison operations
    CMP_EQ = 0xA0,      // Compare  (x, y)
    CMP_NEQ,            // Compare  (x, y)
    CMP_GT,             // Compare  (x, y)
    CMP_LT,             // Compare  (x, y)
    CMP_GTE,            // Compare  (x, y)
    CMP_LTE,            // Compare  (x, y)

    // Control flow
    JMP = 0xC0,         // Jump to the given address in the program bytecode (uint16_t)
    JMP_IF,             // Jump to the given address in the program bytecode if the top of the stack is true (uint16_t)
    JMP_IF_NOT,         // Jump to the given address in the program bytecode if the top of the stack is false (uint16_t)
    CALL,               // Call a function (uint16_t)
    CALL_IF,            // Call a function if the top of the stack is true (uint16_t)
    CALL_IF_NOT,        // Call a function if the top of the stack is false (uint16_t)
    RET,                // Return from a function call

    EXIT = 0xFF         // Exit the program. This will cease the execution of the program and return an optional exit error code (uint8_t)
};

#ifdef __RUNTIME_TEST_ADVANCED_DEBUGGING__
void print_OPCODE_NAME(PLCRuntimeInstructionSet opcode) {
    switch (opcode) {
        case NOP: Serial.print(F("NOP")); break;
        case BOOL: Serial.print(F("BOOL")); break;
        case U8: Serial.print(F("U8")); break;
        case S8: Serial.print(F("S8")); break;
        case U16: Serial.print(F("U16")); break;
        case S16: Serial.print(F("S16")); break;
        case U32: Serial.print(F("U32")); break;
        case S32: Serial.print(F("S32")); break;
        case U64: Serial.print(F("U64")); break;
        case S64: Serial.print(F("S64")); break;
        case F32: Serial.print(F("F32")); break;
        case F64: Serial.print(F("F64")); break;
        case POINTER: Serial.print(F("POINTER")); break;
        case ADD: Serial.print(F("ADD")); break;
        case SUB: Serial.print(F("SUB")); break;
        case MUL: Serial.print(F("MUL")); break;
        case DIV: Serial.print(F("DIV")); break;
        case MOD: Serial.print(F("MOD")); break;
        case POW: Serial.print(F("POW")); break;
        case SQRT: Serial.print(F("SQRT")); break;
        case MIN: Serial.print(F("MIN")); break;
        case MAX: Serial.print(F("MAX")); break;
        case ABS: Serial.print(F("ABS")); break;
        case MAP: Serial.print(F("MAP")); break;
        case CON: Serial.print(F("CON")); break;
        case RAND: Serial.print(F("RAND")); break;
        case RAND1: Serial.print(F("RAND1")); break;
        case RAND2: Serial.print(F("RAND2")); break;
        case LN: Serial.print(F("LN")); break;
        case LOG10: Serial.print(F("LOG10")); break;
        case LOG2: Serial.print(F("LOG2")); break;
        case EXP: Serial.print(F("EXP")); break;
        case SIN: Serial.print(F("SIN")); break;
        case COS: Serial.print(F("COS")); break;
        case TAN: Serial.print(F("TAN")); break;
        case ASIN: Serial.print(F("ASIN")); break;
        case ACOS: Serial.print(F("ACOS")); break;
        case ATAN: Serial.print(F("ATAN")); break;
        case ATAN2: Serial.print(F("ATAN2")); break;
        case SINH: Serial.print(F("SINH")); break;
        case COSH: Serial.print(F("COSH")); break;
        case TANH: Serial.print(F("TANH")); break;
        case ASINH: Serial.print(F("ASINH")); break;
        case ACOSH: Serial.print(F("ACOSH")); break;
        case ATANH: Serial.print(F("ATANH")); break;
        case BW_AND_X8: Serial.print(F("BW_AND_X8")); break;
        case BW_AND_X16: Serial.print(F("BW_AND_X16")); break;
        case BW_AND_X32: Serial.print(F("BW_AND_X32")); break;
        case BW_AND_X64: Serial.print(F("BW_AND_X64")); break;
        case BW_OR_X8: Serial.print(F("BW_OR_X8")); break;
        case BW_OR_X16: Serial.print(F("BW_OR_X16")); break;
        case BW_OR_X32: Serial.print(F("BW_OR_X32")); break;
        case BW_OR_X64: Serial.print(F("BW_OR_X64")); break;
        case BW_XOR_X8: Serial.print(F("BW_XOR_X8")); break;
        case BW_XOR_X16: Serial.print(F("BW_XOR_X16")); break;
        case BW_XOR_X32: Serial.print(F("BW_XOR_X32")); break;
        case BW_XOR_X64: Serial.print(F("BW_XOR_X64")); break;
        case BW_NOT_X8: Serial.print(F("BW_NOT_X8")); break;
        case BW_NOT_X16: Serial.print(F("BW_NOT_X16")); break;
        case BW_NOT_X32: Serial.print(F("BW_NOT_X32")); break;
        case BW_NOT_X64: Serial.print(F("BW_NOT_X64")); break;
        case BW_LSHIFT_X8: Serial.print(F("BW_LSHIFT_X8")); break;
        case BW_LSHIFT_X16: Serial.print(F("BW_LSHIFT_X16")); break;
        case BW_LSHIFT_X32: Serial.print(F("BW_LSHIFT_X32")); break;
        case BW_LSHIFT_X64: Serial.print(F("BW_LSHIFT_X64")); break;
        case BW_RSHIFT_X8: Serial.print(F("BW_RSHIFT_X8")); break;
        case BW_RSHIFT_X16: Serial.print(F("BW_RSHIFT_X16")); break;
        case BW_RSHIFT_X32: Serial.print(F("BW_RSHIFT_X32")); break;
        case BW_RSHIFT_X64: Serial.print(F("BW_RSHIFT_X64")); break;
        case LOGIC_AND: Serial.print(F("LOGIC_AND")); break;
        case LOGIC_OR: Serial.print(F("LOGIC_OR")); break;
        case LOGIC_XOR: Serial.print(F("LOGIC_XOR")); break;
        case LOGIC_NOT: Serial.print(F("LOGIC_NOT")); break;
        case CMP_EQ: Serial.print(F("CMP_EQ")); break;
        case CMP_NEQ: Serial.print(F("CMP_NEQ")); break;
        case CMP_GT: Serial.print(F("CMP_GT")); break;
        case CMP_LT: Serial.print(F("CMP_LT")); break;
        case CMP_GTE: Serial.print(F("CMP_GTE")); break;
        case CMP_LTE: Serial.print(F("CMP_LTE")); break;
        case JMP: Serial.print(F("JMP")); break;
        case JMP_IF: Serial.print(F("JMP_IF")); break;
        case JMP_IF_NOT: Serial.print(F("JMP_IF_NOT")); break;
        case CALL: Serial.print(F("CALL")); break;
        case CALL_IF: Serial.print(F("CALL_IF")); break;
        case CALL_IF_NOT: Serial.print(F("CALL_IF_NOT")); break;
        case RET: Serial.print(F("RET")); break;
        case EXIT: Serial.print(F("EXIT")); break;
        default: Serial.print(F("UNKNOWN OPCODE")); break;
    }
}
#endif

uint8_t get_OPCODE_SIZE(PLCRuntimeInstructionSet opcode) {
    switch (opcode) {
        case NOP: return 1;
        case BOOL: return 2;
        case U8: return 2;
        case S8: return 2;
        case U16: return 3;
        case S16: return 3;
        case U32: return 5;
        case S32: return 5;
        case U64: return 9;
        case S64: return 9;
        case F32: return 5;
        case F64: return 9;
        case POINTER: return 3;
        case ADD: return 2;
        case SUB: return 2;
        case MUL: return 2;
        case DIV: return 2;
        case MOD: return 2;
        case POW: return 2;
        case SQRT: return 2;
        case MIN: return 2;
        case MAX: return 2;
        case ABS: return 2;
        case MAP: return 2;
        case CON: return 2;
        case RAND: return 2;
        case RAND1: return 2;
        case RAND2: return 2;
        case LN: return 2;
        case LOG10: return 2;
        case LOG2: return 2;
        case EXP: return 2;
        case SIN: return 2;
        case COS: return 2;
        case TAN: return 2;
        case ASIN: return 2;
        case ACOS: return 2;
        case ATAN: return 2;
        case ATAN2: return 2;
        case SINH: return 2;
        case COSH: return 2;
        case TANH: return 2;
        case ASINH: return 2;
        case ACOSH: return 2;
        case ATANH: return 2;
        case BW_AND_X8: return 1;
        case BW_AND_X16: return 1;
        case BW_AND_X32: return 1;
        case BW_AND_X64: return 1;
        case BW_OR_X8: return 1;
        case BW_OR_X16: return 1;
        case BW_OR_X32: return 1;
        case BW_OR_X64: return 1;
        case BW_XOR_X8: return 1;
        case BW_XOR_X16: return 1;
        case BW_XOR_X32: return 1;
        case BW_XOR_X64: return 1;
        case BW_NOT_X8: return 1;
        case BW_NOT_X16: return 1;
        case BW_NOT_X32: return 1;
        case BW_NOT_X64: return 1;
        case BW_LSHIFT_X8: return 1;
        case BW_LSHIFT_X16: return 1;
        case BW_LSHIFT_X32: return 1;
        case BW_LSHIFT_X64: return 1;
        case BW_RSHIFT_X8: return 1;
        case BW_RSHIFT_X16: return 1;
        case BW_RSHIFT_X32: return 1;
        case BW_RSHIFT_X64: return 1;
        case LOGIC_AND: return 1;
        case LOGIC_OR: return 1;
        case LOGIC_XOR: return 1;
        case LOGIC_NOT: return 1;
        case CMP_EQ: return 2;
        case CMP_NEQ: return 2;
        case CMP_GT: return 2;
        case CMP_LT: return 2;
        case CMP_GTE: return 2;
        case CMP_LTE: return 2;

        case JMP: return 3;
        case JMP_IF: return 3;
        case JMP_IF_NOT: return 3;
        case CALL: return 3;
        case CALL_IF: return 3;
        case CALL_IF_NOT: return 3;
        case RET: return 1;
        case EXIT: return 1;
        default: return 0;
    }
}