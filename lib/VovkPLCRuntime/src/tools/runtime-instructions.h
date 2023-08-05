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

#include "runtime-tools.h"

enum RuntimeError {
    STATUS_SUCCESS = 0,
    UNKNOWN_INSTRUCTION,
    INVALID_INSTRUCTION,
    INVALID_DATA_TYPE,
    STACK_OVERFLOW,
    STACK_UNDERFLOW,
    INVALID_STACK_SIZE,
    EMPTY_STACK,
    CALL_STACK_OVERFLOW,
    CALL_STACK_UNDERFLOW,
    INVALID_MEMORY_ADDRESS,
    INVALID_MEMORY_SIZE,
    UNDEFINED_STATE,
    INVALID_PROGRAM_INDEX,
    PROGRAM_SIZE_EXCEEDED,
    PROGRAM_POINTER_OUT_OF_BOUNDS,
    PROGRAM_EXITED,
    EMPTY_PROGRAM,
    NO_PROGRAM,
    INVALID_CHECKSUM,
    EXECUTION_ERROR,
    EXECUTION_TIMEOUT,
    MEMORY_ACCESS_ERROR,
};

#ifdef __RUNTIME_DEBUG__
const char* const RuntimeErrorNames [] PROGMEM = {
    STRINGIFY(STATUS_SUCCESS),
    STRINGIFY(UNKNOWN_INSTRUCTION),
    STRINGIFY(INVALID_INSTRUCTION),
    STRINGIFY(INVALID_DATA_TYPE),
    STRINGIFY(STACK_OVERFLOW),
    STRINGIFY(STACK_UNDERFLOW),
    STRINGIFY(INVALID_STACK_SIZE),
    STRINGIFY(EMPTY_STACK),
    STRINGIFY(CALL_STACK_OVERFLOW),
    STRINGIFY(CALL_STACK_UNDERFLOW),
    STRINGIFY(INVALID_MEMORY_ADDRESS),
    STRINGIFY(INVALID_MEMORY_SIZE),
    STRINGIFY(UNDEFINED_STATE),
    STRINGIFY(INVALID_PROGRAM_INDEX),
    STRINGIFY(PROGRAM_SIZE_EXCEEDED),
    STRINGIFY(PROGRAM_POINTER_OUT_OF_BOUNDS),
    STRINGIFY(PROGRAM_EXITED),
    STRINGIFY(EMPTY_PROGRAM),
    STRINGIFY(NO_PROGRAM),
    STRINGIFY(INVALID_CHECKSUM),
    STRINGIFY(EXECUTION_ERROR),
    STRINGIFY(EXECUTION_TIMEOUT),
    STRINGIFY(MEMORY_ACCESS_ERROR),
};

const char* RUNTIME_ERROR_NAME(RuntimeError error);
void logRuntimeErrorList();

#endif

// Reverse Polish Notation (RPN) to custom bytecode


// Readable RPN code example:
//                                              - stack: []
//  (uint8_t 4)        - bytecode [ 02, 04 ]    - stack: [4]    
//  (uint8_t 6)        - bytecode [ 02, 06 ]    - stack: [4, 6]
//  (ADD uint8_t)      - bytecode [ A1, 02 ]    - stack: [10]
//  (uint8_t 2)        - bytecode [ 02, 02 ]    - stack: [10, 2]
//  (MUL uint8_t)      - bytecode [ A3, 02 ]    - stack: [20]
//  (uint8_t 5)        - bytecode [ 02, 05 ]    - stack: [20, 5]
//  (DIV uint8_t)      - bytecode [ A4, 02 ]    - stack: [4]
//  (uint8_t 3)        - bytecode [ 02, 03 ]    - stack: [4, 3]
//  (SUB uint8_t)      - bytecode [ A2, 02 ]    - stack: [1]
//                                              - stack: [1] - result

// Bytecode buffer:
//  02 04 02 06 A1 02 02 02 A3 02 02 05 A4 02 02 03 A2 02

// Memory locations:
enum PLCRuntimeMemoryLocation {
    VOID = 0x00,        // VOID - oblivion
    STACK = 0x01,       // STACK - internal stack location
    MEMORY = 0x02,      // MEMORY - internal memory location
};

// Instruction set:
enum PLCRuntimeInstructionSet {
    NOP = 0x00,         // NOP - no operation

    // Data types
    type_bool,          // Constant boolean value
    type_uint8_t,       // Constant uint8_t value
    type_uint16_t,      // Constant int8_t value
    type_uint32_t,      // Constant uint16_t integer value
    type_uint64_t,      // Constant int16_t integer value
    type_int8_t,        // Constant uint32_t integer value
    type_int16_t,       // Constant int32_t integer value
    type_int32_t,       // Constant uint64_t integer value
    type_int64_t,       // Constant int64_t integer value
    type_float,         // Constant float value
    type_double,        // Constant double value

    MOV = 0x10,         // Universal value move. Example: [ uint8_t MOV, uint8_t type, uint8_t source_location, uint16_t source_address, uint8_t destination_location, uint16_t destination_address ]     
    CVT,                // Convert value from one type to another. Example: [ uint8_t CVT, uint8_t source_type, uint8_t destination_type ]
    PUT,                // Put value from stack to memory. Example: [ uint8_t PUT, uint8_t type, uint16_t address ]
    GET,                // Get value from memory to stack. Example: [ uint8_t GET, uint8_t type, uint16_t address ]

    COPY,               // Make a duplicate of the top of the stack
    SWAP,               // Swap the top two values on the stack
    DROP,               // Remove the top of the stack
    CLEAR,              // Clear the stack

    // Arithmetic operations
    ADD = 0x20,         // Addition, requires data type as argument
    SUB,                // Subtraction, requires data type as argument
    MUL,                // Multiplication, requires data type as argument
    DIV,                // Division, requires data type as argument

    /* TODO: */
    // MOD,                // Modulo, requires data type as argument
    // POW,                // Power for given type. Example: POW uint8_t
    // SQRT,               // Square root
    // MIN,                // Minimum 
    // MAX,                // Maximum 
    // ABS,                // Absolute value for int8_t
    // MAP,                // Map  (x, in_min, in_max, out_min, out_max)
    // CON,                // Constrain  (x, min, max)
    // RAND,               // Random  (full range)
    // RAND1,              // Random  (max)
    // RAND2,              // Random  (min, max)
    // LN,                 // Natural logarithm
    // LOG10,              // Base 10 logarithm
    // LOG2,               // Base 2 logarithm for float
    // EXP,                // Exponential
    // SIN,                // Sine
    // COS,                // Cosine
    // TAN,                // Tangent
    // ASIN,               // Arc sine
    // ACOS,               // Arc cosine
    // ATAN,               // Arc tangent
    // ATAN2,              // Arc tangent 2
    // SINH,               // Hyperbolic sine
    // COSH,               // Hyperbolic cosine
    // TANH,               // Hyperbolic tangent
    // ASINH,              // Hyperbolic arc sine
    // ACOSH,              // Hyperbolic arc cosine
    // ATANH,              // Hyperbolic arc tangent

    // Bit operations (PLC specific for simpler bytecode generation)
    GET_X8_B0 = 0x40,   // Get the first bit of the 1 byte size value (x)
    GET_X8_B1,          // Get the second bit of the 1 byte size value (x)
    GET_X8_B2,          // Get the third bit of the 1 byte size value (x)
    GET_X8_B3,          // Get the fourth bit of the 1 byte size value (x)
    GET_X8_B4,          // Get the fifth bit of the 1 byte size value (x)
    GET_X8_B5,          // Get the sixth bit of the 1 byte size value (x)
    GET_X8_B6,          // Get the seventh bit of the 1 byte size value (x)
    GET_X8_B7,          // Get the eighth bit of the 1 byte size value (x)

    SET_X8_B0,          // Set the first bit of the 1 byte size value (x)
    SET_X8_B1,          // Set the second bit of the 1 byte size value (x)
    SET_X8_B2,          // Set the third bit of the 1 byte size value (x)
    SET_X8_B3,          // Set the fourth bit of the 1 byte size value (x)
    SET_X8_B4,          // Set the fifth bit of the 1 byte size value (x)
    SET_X8_B5,          // Set the sixth bit of the 1 byte size value (x)
    SET_X8_B6,          // Set the seventh bit of the 1 byte size value (x)
    SET_X8_B7,          // Set the eighth bit of the 1 byte size value (x)

    RSET_X8_B0,         // Reset the first bit of the 1 byte size value (x)
    RSET_X8_B1,         // Reset the second bit of the 1 byte size value (x)
    RSET_X8_B2,         // Reset the third bit of the 1 byte size value (x)
    RSET_X8_B3,         // Reset the fourth bit of the 1 byte size value (x)
    RSET_X8_B4,         // Reset the fifth bit of the 1 byte size value (x)
    RSET_X8_B5,         // Reset the sixth bit of the 1 byte size value (x)
    RSET_X8_B6,         // Reset the seventh bit of the 1 byte size value (x)
    RSET_X8_B7,         // Reset the eighth bit of the 1 byte size value (x)

    READ_X8_B0,         // Read the first bit of the 1 byte at the given address (x)
    READ_X8_B1,         // Read the second bit of the 1 byte at the given address (x)
    READ_X8_B2,         // Read the third bit of the 1 byte at the given address (x)
    READ_X8_B3,         // Read the fourth bit of the 1 byte at the given address (x)
    READ_X8_B4,         // Read the fifth bit of the 1 byte at the given address (x)
    READ_X8_B5,         // Read the sixth bit of the 1 byte at the given address (x)
    READ_X8_B6,         // Read the seventh bit of the 1 byte at the given address (x)
    READ_X8_B7,         // Read the eighth bit of the 1 byte at the given address (x)

    WRITE_X8_B0,        // Write the first bit of the 1 byte at the given address (x)
    WRITE_X8_B1,        // Write the second bit of the 1 byte at the given address (x)
    WRITE_X8_B2,        // Write the third bit of the 1 byte at the given address (x)
    WRITE_X8_B3,        // Write the fourth bit of the 1 byte at the given address (x)
    WRITE_X8_B4,        // Write the fifth bit of the 1 byte at the given address (x)
    WRITE_X8_B5,        // Write the sixth bit of the 1 byte at the given address (x)
    WRITE_X8_B6,        // Write the seventh bit of the 1 byte at the given address (x)
    WRITE_X8_B7,        // Write the eighth bit of the 1 byte at the given address (x)

    WRITE_S_X8_B0,      // Write the first bit of the 1 byte at the given address (x) to value 1 (SET)
    WRITE_S_X8_B1,      // Write the second bit of the 1 byte at the given address (x) to value 1 (SET)
    WRITE_S_X8_B2,      // Write the third bit of the 1 byte at the given address (x) to value 1 (SET)
    WRITE_S_X8_B3,      // Write the fourth bit of the 1 byte at the given address (x) to value 1 (SET)
    WRITE_S_X8_B4,      // Write the fifth bit of the 1 byte at the given address (x) to value 1 (SET)
    WRITE_S_X8_B5,      // Write the sixth bit of the 1 byte at the given address (x) to value 1 (SET)
    WRITE_S_X8_B6,      // Write the seventh bit of the 1 byte at the given address (x) to value 1 (SET)
    WRITE_S_X8_B7,      // Write the eighth bit of the 1 byte at the given address (x) to value 1 (SET)

    WRITE_R_X8_B0,      // Write the first bit of the 1 byte at the given address (x) to value 0 (RESET)
    WRITE_R_X8_B1,      // Write the second bit of the 1 byte at the given address (x) to value 0 (RESET)
    WRITE_R_X8_B2,      // Write the third bit of the 1 byte at the given address (x) to value 0 (RESET)
    WRITE_R_X8_B3,      // Write the fourth bit of the 1 byte at the given address (x) to value 0 (RESET)
    WRITE_R_X8_B4,      // Write the fifth bit of the 1 byte at the given address (x) to value 0 (RESET)
    WRITE_R_X8_B5,      // Write the sixth bit of the 1 byte at the given address (x) to value 0 (RESET)
    WRITE_R_X8_B6,      // Write the seventh bit of the 1 byte at the given address (x) to value 0 (RESET)
    WRITE_R_X8_B7,      // Write the eighth bit of the 1 byte at the given address (x) to value 0 (RESET)

    WRITE_INV_X8_B0,    // Write the first bit of the 1 byte at the given address (x) to inverted value (INVERT)
    WRITE_INV_X8_B1,    // Write the second bit of the 1 byte at the given address (x) to inverted value (INVERT)
    WRITE_INV_X8_B2,    // Write the third bit of the 1 byte at the given address (x) to inverted value (INVERT)
    WRITE_INV_X8_B3,    // Write the fourth bit of the 1 byte at the given address (x) to inverted value (INVERT)
    WRITE_INV_X8_B4,    // Write the fifth bit of the 1 byte at the given address (x) to inverted value (INVERT)
    WRITE_INV_X8_B5,    // Write the sixth bit of the 1 byte at the given address (x) to inverted value (INVERT)
    WRITE_INV_X8_B6,    // Write the seventh bit of the 1 byte at the given address (x) to inverted value (INVERT)
    WRITE_INV_X8_B7,    // Write the eighth bit of the 1 byte at the given address (x) to inverted value (INVERT)

    // Bitwise operations
    BW_AND_X8 = 0xA0,   // Bitwise AND for 1 byte size values (x, y)
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
    LOGIC_AND = 0xC0,   // Logical AND for bool (x, y)
    LOGIC_OR,           // Logical OR for bool (x, y)
    LOGIC_XOR,          // Logical XOR for bool (x, y)
    LOGIC_NOT,          // Logical NOT for bool (x)

    // Comparison operations
    CMP_EQ = 0xD0,      // Compare  (x, y)
    CMP_NEQ,            // Compare  (x, y)
    CMP_GT,             // Compare  (x, y)
    CMP_LT,             // Compare  (x, y)
    CMP_GTE,            // Compare  (x, y)
    CMP_LTE,            // Compare  (x, y)

    // Control flow
    JMP = 0xE0,         // Jump to the given address in the program bytecode (uint16_t)
    JMP_IF,             // Jump to the given address in the program bytecode if the top of the stack is true (uint16_t)
    JMP_IF_NOT,         // Jump to the given address in the program bytecode if the top of the stack is false (uint16_t)
    CALL,               // Call a function (uint16_t)
    CALL_IF,            // Call a function if the top of the stack is true (uint16_t)
    CALL_IF_NOT,        // Call a function if the top of the stack is false (uint16_t)
    RET,                // Return from a function call
    RET_IF,             // Return from a function call if the top of the stack is true
    RET_IF_NOT,         // Return from a function call if the top of the stack is false

    EXIT = 0xFF         // Exit the program. This will cease the execution of the program and return an optional exit error code (uint8_t)
};


bool OPCODE_EXISTS(PLCRuntimeInstructionSet opcode);
const FSH* OPCODE_NAME(PLCRuntimeInstructionSet opcode);
uint8_t OPCODE_SIZE(PLCRuntimeInstructionSet opcode);
void logRuntimeInstructionSet();