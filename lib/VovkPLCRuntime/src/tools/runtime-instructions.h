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

const char* const unknows_error_code_str PROGMEM = "UNKNOWN_ERROR_CODE";
char runtime_error_msg[40];
const char* RUNTIME_ERROR_NAME(RuntimeError error) {
    char* output = runtime_error_msg;
    if (error > SIZE_OF_ARRAY(RuntimeErrorNames)) fstrcpy(output, unknows_error_code_str);
    else fstrcpy(output, RuntimeErrorNames[error]);
    return output;
}

void logRuntimeErrorList() {
    Serial.println(F("RuntimeError Status List:\n")); // "    %2d: %s"
    for (uint32_t i = 0; i < SIZE_OF_ARRAY(RuntimeErrorNames); i++) {
        Serial.print(F("    "));
        if (i < 10) Serial.print(' ');
        Serial.print(i);
        Serial.print(F(": "));
        Serial.println(RuntimeErrorNames[i]);
    }
    Serial.println();
}

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


    // Bitwise operations
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
    LOGIC_AND = 0x80,   // Logical AND for bool (x, y)
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


bool OPCODE_EXISTS(PLCRuntimeInstructionSet opcode) {
    switch (opcode) {
        case NOP:
        case type_bool:
        case type_uint8_t:
        case type_int8_t:
        case type_uint16_t:
        case type_int16_t:
        case type_uint32_t:
        case type_int32_t:
        case type_uint64_t:
        case type_int64_t:
        case type_float:
        case type_double:
        case MOV:
        case CVT:
        case PUT:
        case GET:
        case ADD:
        case SUB:
        case MUL:
        case DIV:
            /* TODO: */
            // case MOD:
            // case POW:
            // case SQRT:
            // case MIN:
            // case MAX:
            // case ABS:
            // case MAP:
            // case CON:
            // case RAND:
            // case RAND1:
            // case RAND2:
            // case LN:
            // case LOG10:
            // case LOG2:
            // case EXP:
            // case SIN:
            // case COS:
            // case TAN:
            // case ASIN:
            // case ACOS:
            // case ATAN:
            // case ATAN2:
            // case SINH:
            // case COSH:
            // case TANH:
            // case ASINH:
            // case ACOSH:
            // case ATANH:
        case GET_X8_B0:
        case GET_X8_B1:
        case GET_X8_B2:
        case GET_X8_B3:
        case GET_X8_B4:
        case GET_X8_B5:
        case GET_X8_B6:
        case GET_X8_B7:
        case SET_X8_B0:
        case SET_X8_B1:
        case SET_X8_B2:
        case SET_X8_B3:
        case SET_X8_B4:
        case SET_X8_B5:
        case SET_X8_B6:
        case SET_X8_B7:
        case RSET_X8_B0:
        case RSET_X8_B1:
        case RSET_X8_B2:
        case RSET_X8_B3:
        case RSET_X8_B4:
        case RSET_X8_B5:
        case RSET_X8_B6:
        case RSET_X8_B7:

        case BW_AND_X8:
        case BW_AND_X16:
        case BW_AND_X32:
        case BW_AND_X64:
        case BW_OR_X8:
        case BW_OR_X16:
        case BW_OR_X32:
        case BW_OR_X64:
        case BW_XOR_X8:
        case BW_XOR_X16:
        case BW_XOR_X32:
        case BW_XOR_X64:
        case BW_NOT_X8:
        case BW_NOT_X16:
        case BW_NOT_X32:
        case BW_NOT_X64:
        case BW_LSHIFT_X8:
        case BW_LSHIFT_X16:
        case BW_LSHIFT_X32:
        case BW_LSHIFT_X64:
        case BW_RSHIFT_X8:
        case BW_RSHIFT_X16:
        case BW_RSHIFT_X32:
        case BW_RSHIFT_X64:
        case LOGIC_AND:
        case LOGIC_OR:
        case LOGIC_XOR:
        case LOGIC_NOT:
        case CMP_EQ:
        case CMP_NEQ:
        case CMP_GT:
        case CMP_LT:
        case CMP_GTE:
        case CMP_LTE:
        case JMP:
        case JMP_IF:
        case JMP_IF_NOT:
        case CALL:
        case CALL_IF:
        case CALL_IF_NOT:
        case RET:
        case EXIT: return true;
        default: break;
    }
    return false;
}

const FSH* OPCODE_NAME(PLCRuntimeInstructionSet opcode) {
    switch (opcode) {
        case NOP: return F("NOP");
        case type_bool: return F("PUSH boolean");
        case type_uint8_t: return F("PUSH uint8_t");
        case type_int8_t: return F("PUSH int8_t");
        case type_uint16_t: return F("PUSH uint16_t");
        case type_int16_t: return F("PUSH int16_t");
        case type_uint32_t: return F("PUSH uint32_t");
        case type_int32_t: return F("PUSH int32_t");
        case type_uint64_t: return F("PUSH uint64_t");
        case type_int64_t: return F("PUSH int64_t");
        case type_float: return F("PUSH float");
        case type_double: return F("PUSH double");
        case MOV: return F("MOV");
        case CVT: return F("CVT");
        case PUT: return F("PUT");
        case GET: return F("GET");
        case ADD: return F("ADD");
        case SUB: return F("SUB");
        case MUL: return F("MUL");
        case DIV: return F("DIV");
            /* TODO: */
            // case MOD: return F("MOD");
            // case POW: return F("POW");
            // case SQRT: return F("SQRT");
            // case MIN: return F("MIN");
            // case MAX: return F("MAX");
            // case ABS: return F("ABS");
            // case MAP: return F("MAP");
            // case CON: return F("CON");
            // case RAND: return F("RAND");
            // case RAND1: return F("RAND1");
            // case RAND2: return F("RAND2");
            // case LN: return F("LN");
            // case LOG10: return F("LOG10");
            // case LOG2: return F("LOG2");
            // case EXP: return F("EXP");
            // case SIN: return F("SIN");
            // case COS: return F("COS");
            // case TAN: return F("TAN");
            // case ASIN: return F("ASIN");
            // case ACOS: return F("ACOS");
            // case ATAN: return F("ATAN");
            // case ATAN2: return F("ATAN2");
            // case SINH: return F("SINH");
            // case COSH: return F("COSH");
            // case TANH: return F("TANH");
            // case ASINH: return F("ASINH");
            // case ACOSH: return F("ACOSH");
            // case ATANH: return F("ATANH");>
        case GET_X8_B0: return F("GET_X8_B0");
        case GET_X8_B1: return F("GET_X8_B1");
        case GET_X8_B2: return F("GET_X8_B2");
        case GET_X8_B3: return F("GET_X8_B3");
        case GET_X8_B4: return F("GET_X8_B4");
        case GET_X8_B5: return F("GET_X8_B5");
        case GET_X8_B6: return F("GET_X8_B6");
        case GET_X8_B7: return F("GET_X8_B7");
        case SET_X8_B0: return F("SET_X8_B0");
        case SET_X8_B1: return F("SET_X8_B1");
        case SET_X8_B2: return F("SET_X8_B2");
        case SET_X8_B3: return F("SET_X8_B3");
        case SET_X8_B4: return F("SET_X8_B4");
        case SET_X8_B5: return F("SET_X8_B5");
        case SET_X8_B6: return F("SET_X8_B6");
        case SET_X8_B7: return F("SET_X8_B7");
        case RSET_X8_B0: return F("RSET_X8_B0");
        case RSET_X8_B1: return F("RSET_X8_B1");
        case RSET_X8_B2: return F("RSET_X8_B2");
        case RSET_X8_B3: return F("RSET_X8_B3");
        case RSET_X8_B4: return F("RSET_X8_B4");
        case RSET_X8_B5: return F("RSET_X8_B5");
        case RSET_X8_B6: return F("RSET_X8_B6");
        case RSET_X8_B7: return F("RSET_X8_B7");

        case BW_AND_X8: return F("BW_AND_X8");
        case BW_AND_X16: return F("BW_AND_X16");
        case BW_AND_X32: return F("BW_AND_X32");
        case BW_AND_X64: return F("BW_AND_X64");
        case BW_OR_X8: return F("BW_OR_X8");
        case BW_OR_X16: return F("BW_OR_X16");
        case BW_OR_X32: return F("BW_OR_X32");
        case BW_OR_X64: return F("BW_OR_X64");
        case BW_XOR_X8: return F("BW_XOR_X8");
        case BW_XOR_X16: return F("BW_XOR_X16");
        case BW_XOR_X32: return F("BW_XOR_X32");
        case BW_XOR_X64: return F("BW_XOR_X64");
        case BW_NOT_X8: return F("BW_NOT_X8");
        case BW_NOT_X16: return F("BW_NOT_X16");
        case BW_NOT_X32: return F("BW_NOT_X32");
        case BW_NOT_X64: return F("BW_NOT_X64");
        case BW_LSHIFT_X8: return F("BW_LSHIFT_X8");
        case BW_LSHIFT_X16: return F("BW_LSHIFT_X16");
        case BW_LSHIFT_X32: return F("BW_LSHIFT_X32");
        case BW_LSHIFT_X64: return F("BW_LSHIFT_X64");
        case BW_RSHIFT_X8: return F("BW_RSHIFT_X8");
        case BW_RSHIFT_X16: return F("BW_RSHIFT_X16");
        case BW_RSHIFT_X32: return F("BW_RSHIFT_X32");
        case BW_RSHIFT_X64: return F("BW_RSHIFT_X64");
        case LOGIC_AND: return F("LOGIC_AND");
        case LOGIC_OR: return F("LOGIC_OR");
        case LOGIC_XOR: return F("LOGIC_XOR");
        case LOGIC_NOT: return F("LOGIC_NOT");
        case CMP_EQ: return F("CMP_EQ");
        case CMP_NEQ: return F("CMP_NEQ");
        case CMP_GT: return F("CMP_GT");
        case CMP_LT: return F("CMP_LT");
        case CMP_GTE: return F("CMP_GTE");
        case CMP_LTE: return F("CMP_LTE");
        case JMP: return F("JMP");
        case JMP_IF: return F("JMP_IF");
        case JMP_IF_NOT: return F("JMP_IF_NOT");
        case CALL: return F("CALL");
        case CALL_IF: return F("CALL_IF");
        case CALL_IF_NOT: return F("CALL_IF_NOT");
        case RET: return F("RET");
        case EXIT: return F("EXIT");
        default: break;
    }
    return F("UNKNOWN OPCODE");
}

uint8_t OPCODE_SIZE(PLCRuntimeInstructionSet opcode) {
    switch (opcode) {
        case NOP: return 1;
        case type_bool: return 2;
        case type_uint8_t: return 2;
        case type_int8_t: return 2;
        case type_uint16_t: return 3;
        case type_int16_t: return 3;
        case type_uint32_t: return 5;
        case type_int32_t: return 5;
        case type_uint64_t: return 9;
        case type_int64_t: return 9;
        case type_float: return 5;
        case type_double: return 9;
        case MOV: return 8;
        case CVT: return 3;
        case PUT: return 4;
        case GET: return 4;
        case ADD: return 2;
        case SUB: return 2;
        case MUL: return 2;
        case DIV: return 2;
            // case MOD: return 2;
            // case POW: return 2;
            // case SQRT: return 2;
            // case MIN: return 2;
            // case MAX: return 2;
            // case ABS: return 2;
            // case MAP: return 2;
            // case CON: return 2;
            // case RAND: return 2;
            // case RAND1: return 2;
            // case RAND2: return 2;
            // case LN: return 2;
            // case LOG10: return 2;
            // case LOG2: return 2;
            // case EXP: return 2;
            // case SIN: return 2;
            // case COS: return 2;
            // case TAN: return 2;
            // case ASIN: return 2;
            // case ACOS: return 2;
            // case ATAN: return 2;
            // case ATAN2: return 2;
            // case SINH: return 2;
            // case COSH: return 2;
            // case TANH: return 2;
            // case ASINH: return 2;
            // case ACOSH: return 2;
            // case ATANH: return 2;

        case GET_X8_B0: return 1;
        case GET_X8_B1: return 1;
        case GET_X8_B2: return 1;
        case GET_X8_B3: return 1;
        case GET_X8_B4: return 1;
        case GET_X8_B5: return 1;
        case GET_X8_B6: return 1;
        case GET_X8_B7: return 1;
        case SET_X8_B0: return 1;
        case SET_X8_B1: return 1;
        case SET_X8_B2: return 1;
        case SET_X8_B3: return 1;
        case SET_X8_B4: return 1;
        case SET_X8_B5: return 1;
        case SET_X8_B6: return 1;
        case SET_X8_B7: return 1;
        case RSET_X8_B0: return 1;
        case RSET_X8_B1: return 1;
        case RSET_X8_B2: return 1;
        case RSET_X8_B3: return 1;
        case RSET_X8_B4: return 1;
        case RSET_X8_B5: return 1;
        case RSET_X8_B6: return 1;
        case RSET_X8_B7: return 1;

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
        default: break;
    }
    return 0;
}



void logRuntimeInstructionSet() {
    Serial.println(F("Bytecode Instruction Set:"));
    uint32_t position = 0;
    bool was_valid = true;
    for (int opcode = 0x00; opcode < 256; opcode++) {
        bool is_valid = OPCODE_EXISTS((PLCRuntimeInstructionSet) opcode);
        if ((is_valid && !was_valid) || opcode < 0x02) Serial.println();
        was_valid = is_valid;
        if (is_valid) {
            position = Serial.print(F("    0x"));
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
    Serial.println();
}