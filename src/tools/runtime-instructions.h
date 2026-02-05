// runtime-instructions.h - 2022-12-11
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
    PROGRAM_CYCLE_LIMIT_EXCEEDED,
    FFI_NOT_FOUND,
    FFI_INVALID_PARAMS,
    FFI_EXECUTION_ERROR,
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
    STRINGIFY(PROGRAM_CYCLE_LIMIT_EXCEEDED),
    STRINGIFY(FFI_NOT_FOUND),
    STRINGIFY(FFI_INVALID_PARAMS),
    STRINGIFY(FFI_EXECUTION_ERROR),
};

const char* RUNTIME_ERROR_NAME(RuntimeError error);
void logRuntimeErrorList();

#endif

// Reverse Polish Notation (RPN) to custom bytecode


// Readable RPN code example:
//                                              - stack: []
//  (u8 4)             - bytecode [ 02, 04 ]    - stack: [4]    
//  (u8 6)             - bytecode [ 02, 06 ]    - stack: [4, 6]
//  (ADD u8)           - bytecode [ A1, 02 ]    - stack: [10]
//  (u8 2)             - bytecode [ 02, 02 ]    - stack: [10, 2]
//  (MUL u8)           - bytecode [ A3, 02 ]    - stack: [20]
//  (u8 5)             - bytecode [ 02, 05 ]    - stack: [20, 5]
//  (DIV u8)           - bytecode [ A4, 02 ]    - stack: [4]
//  (u8 3)             - bytecode [ 02, 03 ]    - stack: [4, 3]
//  (SUB u8)           - bytecode [ A2, 02 ]    - stack: [1]
//                                              - stack: [1] - result

// Bytecode buffer:
//  02 04 02 06 A1 02 02 02 A3 02 02 05 A4 02 02 03 A2 02

// Memory locations:
enum PLCRuntimeMemoryLocation {
    VOID = 0x00,        // VOID - oblivion
    STACK = 0x01,       // STACK - internal stack location
    MEMORY = 0x02,      // MEMORY - internal memory location
};

// Language IDs for LANG instruction (used for decompilation)
enum PLCSourceLanguage {
    LANG_UNKNOWN = 0x00,    // Unknown/unspecified source language
    LANG_PLCASM = 0x01,     // PLCASM assembly
    LANG_STL = 0x02,        // STL (Statement List)
    LANG_LADDER = 0x03,     // Ladder diagram (via Network IR)
    LANG_FBD = 0x04,        // Function Block Diagram
    LANG_SFC = 0x05,        // Sequential Function Chart
    LANG_ST = 0x06,         // Structured Text
    LANG_IL = 0x07,         // Instruction List (IEC 61131-3)
    LANG_PLCSCRIPT = 0x08,  // PLCScript (TypeScript/ES7 subset with PLC extensions)
    LANG_CUSTOM = 0xFF,     // Custom/user-defined language
};

const FSH* LANG_NAME(u8 lang_id);
const char* LANG_NAME_LOWER(u8 lang_id);

// Instruction set:
enum PLCRuntimeInstructionSet {
    NOP = 0x00,         // NOP - no operation

    // Data types
    type_pointer,       // Pointer to a memory location
    type_bool,          // Constant boolean value
    type_u8,            // Constant u8 value
    type_u16,           // Constant i8 value
    type_u32,           // Constant u16 integer value
    type_u64,           // Constant i16 integer value
    type_i8,            // Constant u32 integer value
    type_i16,           // Constant i32 integer value
    type_i32,           // Constant u64 integer value
    type_i64,           // Constant i64 integer value
    type_f32,           // Constant f32 value
    type_f64,           // Constant f64 value
    type_char,          // Character type (u8 ASCII value)
    type_str8,          // String8: [ u8 capacity, u8 length, char[capacity] ] - max 254 chars
    type_str16,         // String16: [ u16 capacity, u16 length, char[capacity] ] - max 65534 chars

    CVT = 0x10,         // Convert value from one type to another. Example: [ u8 CVT, u8 source_type, u8 destination_type ]
    LOAD,               // Load value from memory to stack using pointer from the stack. Example: [ u8 LOAD, u8 type ]
    MOVE,               // Move value from stack to memory using pointer from the stack. Example: [ u8 MOVE, u8 type ]
    MOVE_COPY,          // Move value from stack to memory using pointer from the stack and keep the value on the stack. Example: [ u8 MOVE_COPY, u8 type ]

    COPY,               // Make a duplicate of the top of the stack
    SWAP,               // Swap the top two values on the stack
    DROP,               // Remove the top of the stack
    CLEAR,              // Clear the stack
    LOAD_FROM,          // Load value from memory to stack using immediate address. Example: [ u8 LOAD_FROM, u8 type, u16 address ]
    MOVE_TO,            // Move value from stack to memory using immediate address. Example: [ u8 MOVE_TO, u8 type, u16 address ]
    INC_MEM,            // Increment value in memory by 1. Example: [ u8 INC_MEM, u8 type, u16 address ]
    DEC_MEM,            // Decrement value in memory by 1. Example: [ u8 DEC_MEM, u8 type, u16 address ]
    PICK,               // Copy value from stack at byte depth to top. Example: [ u8 PICK, u8 type, u16 depth ]
    POKE,               // Write top value to stack at byte depth. Example: [ u8 POKE, u8 type, u16 depth ]

    // Arithmetic operations
    ADD = 0x20,         // Addition, requires data type as argument
    SUB,                // Subtraction, requires data type as argument
    MUL,                // Multiplication, requires data type as argument
    DIV,                // Division, requires data type as argument
    MOD,                // Modulo, requires data type as argument
    POW,                // Power for given type. Example: POW u8
    SQRT,               // Square root
    NEG,                // Negate for signed types including f32 and f64

    /* TODO: */
    // MIN,                // Minimum 
    // MAX,                // Maximum 
    ABS,                // Absolute value for i8
    // MAP,                // Map  (x, in_min, in_max, out_min, out_max)
    // CON,                // Constrain  (x, min, max)
    // RAND,               // Random  (full range)
    // RAND1,              // Random  (max)
    // RAND2,              // Random  (min, max)
    // LN,                 // Natural logarithm
    // LOG10,              // Base 10 logarithm
    // LOG2,               // Base 2 logarithm for f32
    // EXP,                // Exponential
    SIN,                // Sine
    COS,                // Cosine

    // Timer operations
    TON_CONST = 0x30,   // Timer On-Delay (IN, PT: Constant u32) -> Q
    TON_MEM,            // Timer On-Delay (IN, PT: Memory u32) -> Q
    TOF_CONST,          // Timer Off-Delay (IN, PT: Constant u32) -> Q
    TOF_MEM,            // Timer Off-Delay (IN, PT: Memory u32) -> Q
    TP_CONST,           // Timer Pulse (IN, PT: Constant u32) -> Q
    TP_MEM,             // Timer Pulse (IN, PT: Memory u32) -> Q

    // Counter operations (IEC 61131-3)
    CTU_CONST,          // Counter Up (CU, R, PV: Constant u32) -> Q (CV >= PV)
    CTU_MEM,            // Counter Up (CU, R, PV: Memory u32) -> Q (CV >= PV)
    CTD_CONST,          // Counter Down (CD, LD, PV: Constant u32) -> Q (CV <= 0)
    CTD_MEM,            // Counter Down (CD, LD, PV: Memory u32) -> Q (CV <= 0)

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

    READ_BIT_DU = 0x80,      // Read the Rising Edge of a bit at the given address (x) and bit (y) and check against the old state at address (z) and bit (w)
    READ_BIT_DD,             // Read the Falling Edge of a bit at the given address (x) and bit (y) and check against the old state at address (z) and bit (w)

    READ_BIT_INV_DU,
    READ_BIT_INV_DD,

    WRITE_BIT_DU,
    WRITE_BIT_DD,

    WRITE_BIT_INV_DU,
    WRITE_BIT_INV_DD,

    WRITE_SET_DU,
    WRITE_SET_DD,

    WRITE_RSET_DU,
    WRITE_RSET_DD,

    STACK_DU,
    STACK_DD,
    STACK_DC,  // Differentiate Change - triggers on any state change

    // Branch stack operations (u32 Binary RLO Stack for parallel branches in ladder logic)
    BR_SAVE = 0x90,     // Save RLO to branch stack: BR = (BR << 1) | (pop_u8() ? 1 : 0)
    BR_READ,            // Read top of branch stack to RLO: push_u8((BR & 1) ? 1 : 0)
    BR_DROP,            // Drop top of branch stack: BR >>= 1
    BR_CLR,             // Clear branch stack: BR = 0

    // String operations
    // String memory layout:
    //   str8:  [ u8 capacity, u8 length, char[capacity] ] - header 2 bytes, max 254 chars
    //   str16: [ u16 capacity, u16 length, char[capacity] ] - header 4 bytes, max 65534 chars
    // All string ops take: [ opcode, u8 str_type (type_str8 or type_str16), u16 str_addr, ... ]
    STR_LEN = 0x94,     // Get string length -> push u16. [ STR_LEN, type, addr ]
    STR_CAP,            // Get string capacity -> push u16. [ STR_CAP, type, addr ]
    STR_GET,            // Get char at index (pop u16 index) -> push u8 char. [ STR_GET, type, addr ]
    STR_SET,            // Set char at index (pop u8 char, pop u16 index). [ STR_SET, type, addr ]
    STR_CLEAR,          // Clear string (set length to 0). [ STR_CLEAR, type, addr ]
    STR_CMP,            // Compare two strings -> push i8 (-1, 0, 1). [ STR_CMP, type, addr1, addr2 ]
    STR_EQ,             // Check string equality -> push bool. [ STR_EQ, type, addr1, addr2 ]
    STR_CONCAT,         // Concat src to dest (dest = dest + src). [ STR_CONCAT, type, dest_addr, src_addr ]
    STR_COPY,           // Copy src to dest (dest = src). [ STR_COPY, type, dest_addr, src_addr ]
    STR_SUBSTR,         // Extract substring (pop u16 len, pop u16 start) to dest. [ STR_SUBSTR, type, dest_addr, src_addr ]
    STR_FIND,           // Find substring -> push i16 index (-1 if not found). [ STR_FIND, type, haystack_addr, needle_addr ]
    STR_CHAR,           // Append char (pop u8 char). [ STR_CHAR, type, addr ]

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
    JMP = 0xE0,         // Jump to the given address in the program bytecode (u16)
    JMP_IF,             // Jump to the given address in the program bytecode if the top of the stack is true (u16)
    JMP_IF_NOT,         // Jump to the given address in the program bytecode if the top of the stack is false (u16)
    CALL,               // Call a function (u16)
    CALL_IF,            // Call a function if the top of the stack is true (u16)
    CALL_IF_NOT,        // Call a function if the top of the stack is false (u16)
    RET,                // Return from a function call
    RET_IF,             // Return from a function call if the top of the stack is true
    RET_IF_NOT,         // Return from a function call if the top of the stack is false

    JMP_REL,            // Jump relative to the next instruction address (i16)
    JMP_IF_REL,         // Jump relative to the next instruction address if the top of the stack is true (i16)
    JMP_IF_NOT_REL,     // Jump relative to the next instruction address if the top of the stack is false (i16)
    CALL_REL,           // Call a function relative to the next instruction address (i16)
    CALL_IF_REL,        // Call a function relative to the next instruction address if the top of the stack is true (i16)
    CALL_IF_NOT_REL,    // Call a function relative to the next instruction address if the top of the stack is false (i16)

    // FFI (Foreign Function Interface) operations
    FFI_CALL = 0xF0,        // Call FFI by index with memory addresses: [ FFI_CALL, u8 index, u8 param_count, u16 addr1, ..., u16 ret_addr ]
    FFI_CALL_STACK,         // Call FFI by index with params from stack: [ FFI_CALL_STACK, u8 index, u8 param_count ] - pops params, pushes result

    // Runtime configuration instruction
    CONFIG_TC = 0xFC,   // Configure Timer/Counter offsets: [ CONFIG_TC, u16 timer_offset, u8 timer_count, u16 counter_offset, u8 counter_count ] - 7 bytes

    // Metadata instructions (for decompilation and debugging)
    LANG = 0xFD,        // Language marker: [ LANG, u8 language_id ] - runtime skips 2 bytes, used to identify source language for decompilation
    COMMENT,            // Comment: [ COMMENT, u8 length, ASCII chars... ] - runtime skips 2+length bytes, embeds comment for decompilation

    EXIT = 0xFF         // Exit the program. This will cease the execution of the program and return an optional exit error code (u8)
};


bool OPCODE_EXISTS(PLCRuntimeInstructionSet opcode);
const FSH* OPCODE_NAME(PLCRuntimeInstructionSet opcode);
u8 OPCODE_SIZE(PLCRuntimeInstructionSet opcode);
void logRuntimeInstructionSet();


#include "runtime-instructions-impl.h"
