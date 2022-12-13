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
    STACK_OVERFLOW,
    STACK_UNDERFLOW,
    CALL_STACK_OVERFLOW,
    CALL_STACK_UNDERFLOW,
    INVALID_INSTRUCTION,
    INVALID_DATA_TYPE,
    INVALID_MEMORY_ADDRESS,
    INVALID_MEMORY_SIZE,
    INVALID_STACK_SIZE,
    EMPTY_STACK,
    EXECUTION_ERROR,
    EXECUTION_TIMEOUT,
    PROGRAM_SIZE_EXCEEDED,
    UNKNOWN_INSTRUCTION,
    INVALID_LINE_NUMBER,
    EMPTY_PROGRAM,
    PROGRAM_POINTER_OUT_OF_BOUNDS,
    PROGRAM_EXITED,
    UNDEFINED_STATE,
    INVALID_CHECKSUM,
    NO_PROGRAM,
};

#define STRINGIFY(X) #X

const char* const RuntimeErrorNames [] PROGMEM = {
    STRINGIFY(STATUS_SUCCESS),
     STRINGIFY(STACK_OVERFLOW),
     STRINGIFY(STACK_UNDERFLOW),
     STRINGIFY(CALL_STACK_OVERFLOW),
     STRINGIFY(CALL_STACK_UNDERFLOW),
     STRINGIFY(INVALID_INSTRUCTION),
     STRINGIFY(INVALID_DATA_TYPE),
     STRINGIFY(INVALID_MEMORY_ADDRESS),
     STRINGIFY(INVALID_MEMORY_SIZE),
     STRINGIFY(INVALID_STACK_SIZE),
     STRINGIFY(EMPTY_STACK),
     STRINGIFY(EXECUTION_ERROR),
     STRINGIFY(EXECUTION_TIMEOUT),
     STRINGIFY(PROGRAM_SIZE_EXCEEDED),
     STRINGIFY(UNKNOWN_INSTRUCTION),
     STRINGIFY(INVALID_LINE_NUMBER),
     STRINGIFY(EMPTY_PROGRAM),
     STRINGIFY(PROGRAM_POINTER_OUT_OF_BOUNDS),
     STRINGIFY(PROGRAM_EXITED),
     STRINGIFY(UNDEFINED_STATE),
     STRINGIFY(INVALID_CHECKSUM),
     STRINGIFY(NO_PROGRAM),
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

#define SIZE_OF_ARRAY(a) (sizeof(a) / sizeof(a[0]))


const char* const unknows_error_code_str PROGMEM = "UNKNOWN_ERROR_CODE";

const char* getRuntimeErrorName(RuntimeError error) {
    char* output = new char[40];
    if (error < 0 || error > SIZE_OF_ARRAY(RuntimeErrorNames)) fstrcpy(output, unknows_error_code_str);
    else fstrcpy(output, RuntimeErrorNames[error]);
    return output;
}

union uint32_t_to_float { uint32_t type_uint32_t; float type_float; };
union float_to_uint32_t { float type_float; uint32_t type_uint32_t; };
union uint64_t_to_double { uint64_t type_uint64_t; double type_double; };
union double_to_uint64_t { double type_double; uint64_t type_uint64_t; };

union u8A_to_u16 {
    uint8_t u8A[2];
    uint16_t u16;
};


#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char* __brkval;
#endif  // __arm__

int freeMemory() {
    char top;
#ifdef __arm__
    return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
    return &top - __brkval;
#else  // __arm__
    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

class RuntimeStack {
public:
    Stack<uint8_t>* stack;
    Stack<uint16_t>* call_stack;
    uint16_t max_size = 0;
    uint16_t max_call_stack_size = 0;

    // Create a stack with a maximum size
    RuntimeStack(uint16_t max_size, uint16_t call_stack_size = 10) {
        stack = new Stack<uint8_t>(max_size);
        call_stack = new Stack<uint16_t>(call_stack_size);
        this->max_size = max_size;
        this->max_call_stack_size = call_stack_size;
    }

    void print() { stack->print(); }
    void println() { stack->println(); }

    RuntimeError pushCall(uint16_t return_address) {
        if (call_stack->size() >= max_call_stack_size) return STACK_OVERFLOW;
        call_stack->push(return_address);
        return STATUS_SUCCESS;
    }

    uint16_t popCall() {
        if (call_stack->size() == 0) return STACK_UNDERFLOW;
        return call_stack->pop();
    }

    // Push an uint8_t value to the stack
    RuntimeError push(uint8_t value) {
        if (stack->size() >= max_size) return STACK_OVERFLOW;
        stack->push(value);
        return STATUS_SUCCESS;
    }
    // Pop an uint8_t value from the stack
    uint8_t pop() { return stack->pop(); }
    // Peek the top uint8_t value from the stack
    uint8_t peek() { return stack->peek(); }

    // Push a boolean value to the stack
    RuntimeError push_bool(bool value) {
        if (stack->size() >= max_size) return STACK_OVERFLOW;
        stack->push(value);
        return STATUS_SUCCESS;
    }
    // Pop a boolean value from the stack
    bool pop_bool() {
        bool value = stack->pop();
        return value;
    }
    // Peek the top boolean value from the stack
    bool peek_bool() { return stack->peek(); }


    // Push an uint8_t value to the stack
    RuntimeError push_uint8_t(uint8_t value) {
        if (stack->size() + 1 > max_size) return STACK_OVERFLOW;
        stack->push(value);
        return STATUS_SUCCESS;
    }
    // Pop an uint8_t value from the stack
    uint8_t pop_uint8_t() { return stack->pop(); }
    // Peek the top uint8_t value from the stack
    uint8_t peek_uint8_t() { return stack->peek(); }

    // Push an uint16_t value to the stack
    RuntimeError push_uint16_t(uint16_t value) {
        if (stack->size() + 2 > max_size) return STACK_OVERFLOW;
        stack->push(value >> 8);
        stack->push(value & 0xFF);
        return STATUS_SUCCESS;
    }
    // Pop an uint16_t value from the stack
    uint16_t pop_uint16_t() {
        uint16_t b = stack->pop();
        uint16_t a = stack->pop();
        return (a << 8) | b;
    }
    // Peek the top uint16_t value from the stack
    uint16_t peek_uint16_t() {
        uint16_t b = stack->peek(0);
        uint16_t a = stack->peek(1);
        return (a << 8) | b;
    }

    // Push an uint32_t value to the stack
    RuntimeError push_uint32_t(uint32_t value) {
        if (stack->size() + 4 > max_size) return STACK_OVERFLOW;
        stack->push(value >> 24);
        stack->push((value >> 16) & 0xFF);
        stack->push((value >> 8) & 0xFF);
        stack->push(value & 0xFF);
        return STATUS_SUCCESS;
    }
    // Pop an uint32_t value from the stack
    uint32_t pop_uint32_t() {
        uint32_t d = stack->pop();
        uint32_t c = stack->pop();
        uint32_t b = stack->pop();
        uint32_t a = stack->pop();
        return (a << 24) | (b << 16) | (c << 8) | d;
    }
    // Peek the top uint32_t value from the stack
    uint32_t peek_uint32_t() {
        uint32_t d = stack->peek(0);
        uint32_t c = stack->peek(1);
        uint32_t b = stack->peek(2);
        uint32_t a = stack->peek(3);
        return (a << 24) | (b << 16) | (c << 8) | d;
    }

    // Push an uint64_t value to the stack
    RuntimeError push_uint64_t(uint64_t value) {
        if (stack->size() + 8 > max_size) return STACK_OVERFLOW;
        stack->push(value >> 56);
        stack->push((value >> 48) & 0xFF);
        stack->push((value >> 40) & 0xFF);
        stack->push((value >> 32) & 0xFF);
        stack->push((value >> 24) & 0xFF);
        stack->push((value >> 16) & 0xFF);
        stack->push((value >> 8) & 0xFF);
        stack->push(value & 0xFF);
        return STATUS_SUCCESS;
    }
    // Pop an uint64_t value from the stack
    uint64_t pop_uint64_t() {
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
    uint64_t peek_uint64_t() {
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

    RuntimeError push_int8_t(int8_t value) { return push_uint8_t((uint8_t) value); }
    RuntimeError push_int16_t(int16_t value) { return push_uint16_t((uint16_t) value); }
    RuntimeError push_int32_t(int32_t value) { return push_uint32_t((uint32_t) value); }
    RuntimeError push_int64_t(int64_t value) { return push_uint64_t((uint64_t) value); }
    RuntimeError push_float(float value) { uint32_t_to_float cvt; cvt.type_float = value; return push_uint32_t(cvt.type_uint32_t); }
    RuntimeError push_double(double value) { uint64_t_to_double cvt; cvt.type_double = value; return push_uint64_t(cvt.type_uint64_t); }

    int8_t pop_int8_t() { return (int8_t) pop_uint8_t(); }
    int16_t pop_int16_t() { return (int16_t) pop_uint16_t(); }
    int32_t pop_int32_t() { return (int32_t) pop_uint32_t(); }
    int64_t pop_int64_t() { return (int64_t) pop_uint64_t(); }
    float pop_float() { uint32_t_to_float cvt; cvt.type_uint32_t = pop_uint32_t(); return cvt.type_float; }
    double pop_double() { uint64_t_to_double cvt; cvt.type_uint64_t = pop_uint64_t(); return cvt.type_double; }
    void clear() { while (!stack->empty()) stack->pop(); while (!call_stack->empty()) call_stack->pop(); }

    int8_t peek_int8_t() { return (int8_t) peek_uint8_t(); }
    int16_t peek_int16_t() { return (int16_t) peek_uint16_t(); }
    int32_t peek_int32_t() { return (int32_t) peek_uint32_t(); }
    int64_t peek_int64_t() { return (int64_t) peek_uint64_t(); }
    float peek_float() { uint32_t_to_float cvt; cvt.type_uint32_t = peek_uint32_t(); return cvt.type_float; }
    double peek_double() { uint64_t_to_double cvt; cvt.type_uint64_t = peek_uint64_t(); return cvt.type_double; }

    uint16_t size() { return stack->size(); }
};

#define EXTRACT_TYPE_8(type)                                                                                \
    RuntimeError type_##type(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, type& value) {     \
        uint16_t size = sizeof(type);                                                                       \
        if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;                                     \
        value = bytecode[index];                                                                            \
        index += size;                                                                                      \
        return STATUS_SUCCESS;                                                                              \
    }

#define EXTRACT_TYPE_16(type)                                                                               \
    RuntimeError type_##type(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, type& value) {     \
        uint16_t size = sizeof(type);                                                                       \
        if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;                                     \
        value = ((uint16_t) bytecode[index] << 8) | bytecode[index + 1];                                    \
        index += size;                                                                                      \
        return STATUS_SUCCESS;                                                                              \
    }

#define EXTRACT_TYPE_32(type)                                                                                                                               \
    RuntimeError type_##type(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, type& value) {                                                     \
        uint16_t size = sizeof(type);                                                                                                                       \
        if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;                                                                                     \
        value = ((type) bytecode[index] << 24) | ((type) bytecode[index + 1] << 16) | ((type) bytecode[index + 2] << 8) | ((type) bytecode[index + 3]);     \
        index += size;                                                                                                                                      \
        return STATUS_SUCCESS;                                                                                                                              \
    }

#define EXTRACT_TYPE_32_CVT(type)                                                                                                                                                     \
    RuntimeError type_##type(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, type& value) {                                                                               \
        uint16_t size = sizeof(type);                                                                                                                                                 \
        if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;                                                                                                               \
        uint32_t_to_##type cvt;                                                                                                                                                     \
        cvt.type_uint32_t = ((uint32_t) bytecode[index] << 24) | ((uint32_t) bytecode[index + 1] << 16) | ((uint32_t) bytecode[index + 2] << 8) | ((uint32_t) bytecode[index + 3]);   \
        value = cvt.type_##type;                                                                                                                                                      \
        index += size;                                                                                                                                                                \
        return STATUS_SUCCESS;                                                                                                                                                        \
    }

#define EXTRACT_TYPE_64(type)                                                                                                                                       \
    RuntimeError type_##type(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, type& value) {                                                             \
        uint16_t size = sizeof(type);                                                                                                                               \
        if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;                                                                                             \
        value = ((type) bytecode[index] << 56) | ((type) bytecode[index + 1] << 48) | ((type) bytecode[index + 2] << 40) | ((type) bytecode[index + 3] << 32) |     \
                ((type) bytecode[index + 4] << 24) | ((type) bytecode[index + 5] << 16) | ((type) bytecode[index + 6] << 8) | ((type) bytecode[index + 7]);         \
        index += size;                                                                                                                                              \
        return STATUS_SUCCESS;                                                                                                                                      \
    }

#define EXTRACT_TYPE_64_CVT(type)                                                                                                                                                           \
    RuntimeError type_##type(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, type& value) {                                                                                     \
        uint16_t size = sizeof(type);                                                                                                                                                       \
        if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;                                                                                                                     \
        uint64_t_to_##type cvt;                                                                                                                                                           \
        cvt.type_uint64_t = ((uint64_t) bytecode[index] << 56) | ((uint64_t) bytecode[index + 1] << 48) | ((uint64_t) bytecode[index + 2] << 40) | ((uint64_t) bytecode[index + 3] << 32) | \
                            ((uint64_t) bytecode[index + 4] << 24) | ((uint64_t) bytecode[index + 5] << 16) | ((uint64_t) bytecode[index + 6] << 8) | ((uint64_t) bytecode[index + 7]);     \
        value = cvt.type_##type;                                                                                                                                                            \
        index += size;                                                                                                                                                                      \
        return STATUS_SUCCESS;                                                                                                                                                              \
    }

struct Extract_t {
    // // Extract bool value from program bytecode
    // RuntimeError type_bool(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, bool& value) {
    //     uint16_t size = 1;
    //     if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;
    //     value = bytecode[index];
    //     index += size;
    //     return STATUS_SUCCESS;
    // }

    // // Extract uint8_t value from program bytecode
    // RuntimeError type_uint8_t(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, uint8_t& value) {
    //     uint16_t size = 1;
    //     if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;
    //     value = bytecode[index];
    //     index += size;
    //     return STATUS_SUCCESS;
    // }

    EXTRACT_TYPE_8(bool)
    EXTRACT_TYPE_8(uint8_t)
    EXTRACT_TYPE_8(int8_t)

    EXTRACT_TYPE_16(uint16_t)
    EXTRACT_TYPE_16(int16_t)

    EXTRACT_TYPE_32(uint32_t)
    EXTRACT_TYPE_32(int32_t)
    EXTRACT_TYPE_32_CVT(float)

    EXTRACT_TYPE_64(uint64_t)
    EXTRACT_TYPE_64(int64_t)
    EXTRACT_TYPE_64_CVT(double)

    // // Extract uint16_t value from program bytecode
    // RuntimeError type_uint16_t(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, uint16_t& value) {
    //     uint16_t size = 2;
    //     if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;
    //     value = ((uint16_t) bytecode[index] << 8) | bytecode[index + 1];
    //     index += size;
    //     return STATUS_SUCCESS;
    // }
    // // Extract uint32_t value from program bytecode
    // RuntimeError type_uint32_t(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, uint32_t& value) {
    //     uint16_t size = 4;
    //     if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;
    //     value = ((uint32_t) bytecode[index] << 24) | ((uint32_t) bytecode[index + 1] << 16) | ((uint32_t) bytecode[index + 2] << 8) | bytecode[index + 3];
    //     index += 4;
    //     return STATUS_SUCCESS;
    // }
    // // Extract uint64_t value from program bytecode
    // RuntimeError type_uint64_t(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, uint64_t& value) {
    //     uint16_t size = 8;
    //     if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;
    //     value = ((uint64_t) bytecode[index] << 56) | ((uint64_t) bytecode[index + 1] << 48) | ((uint64_t) bytecode[index + 2] << 40) | ((uint64_t) bytecode[index + 3] << 32) | ((uint64_t) bytecode[index + 4] << 24) | ((uint64_t) bytecode[index + 5] << 16) | ((uint64_t) bytecode[index + 6] << 8) | bytecode[index + 7];
    //     index += size;
    //     return STATUS_SUCCESS;
    // }

    // // Extract int8_t value from program bytecode
    // RuntimeError type_int8_t(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, int8_t& value) {
    //     uint16_t size = 1;
    //     if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;
    //     value = (int8_t) bytecode[index];
    //     index += size;
    //     return STATUS_SUCCESS;
    // }
    // // Extract int16_t value from program bytecode
    // RuntimeError type_int16_t(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, int16_t& value) {
    //     uint16_t size = 2;
    //     if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;
    //     value = ((int16_t) bytecode[index] << 8) | bytecode[index + 1];
    //     index += size;
    //     return STATUS_SUCCESS;
    // }
    // // Extract int32_t value from program bytecode
    // RuntimeError type_int32_t(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, int32_t& value) {
    //     uint16_t size = 4;
    //     if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;
    //     value = ((int32_t) bytecode[index] << 24) | ((int32_t) bytecode[index + 1] << 16) | ((int32_t) bytecode[index + 2] << 8) | bytecode[index + 3];
    //     index += size;
    //     return STATUS_SUCCESS;
    // }
    // // Extract int64_t value from program bytecode
    // RuntimeError type_int64_t(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, int64_t& value) {
    //     uint16_t size = 8;
    //     if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;
    //     value = ((int64_t) bytecode[index] << 56) | ((int64_t) bytecode[index + 1] << 48) | ((int64_t) bytecode[index + 2] << 40) | ((int64_t) bytecode[index + 3] << 32) | ((int64_t) bytecode[index + 4] << 24) | ((int64_t) bytecode[index + 5] << 16) | ((int64_t) bytecode[index + 6] << 8) | bytecode[index + 7];
    //     index += size;
    //     return STATUS_SUCCESS;
    // }

    // // Extract float value from program bytecode
    // RuntimeError type_float(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, float& value) {
    //     uint16_t size = 4;
    //     if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;
    //     uint32_t_to_float cvt;
    //     cvt.type_uint32_t = ((uint32_t) bytecode[index] << 24) | ((uint32_t) bytecode[index + 1] << 16) | ((uint32_t) bytecode[index + 2] << 8) | bytecode[index + 3];
    //     value = cvt.type_float;
    //     index += size;
    //     return STATUS_SUCCESS;
    // }
    // // Extract double value from program bytecode
    // RuntimeError type_double(uint8_t* bytecode, uint16_t bytecode_size, uint16_t& index, double& value) {
    //     uint16_t size = 8;
    //     if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;
    //     uint64_t_to_double cvt;
    //     cvt.type_uint64_t = ((uint64_t) bytecode[index] << 56) | ((uint64_t) bytecode[index + 1] << 48) | ((uint64_t) bytecode[index + 2] << 40) | ((uint64_t) bytecode[index + 3] << 32) | ((uint64_t) bytecode[index + 4] << 24) | ((uint64_t) bytecode[index + 5] << 16) | ((uint64_t) bytecode[index + 6] << 8) | bytecode[index + 7];
    //     value = cvt.type_double;
    //     index += size;
    //     return STATUS_SUCCESS;
    // }

} Extract;

// Reverse Polish Notation (RPN) to custom bytecode


// Readable RPN code example:
//                - bytecode []            - stack: []
//  (uint8_t 4)        - bytecode [ 02, 04 ]    - stack: [4]    
//  (uint8_t 6)        - bytecode [ 02, 06 ]    - stack: [4, 6]
//  (ADD uint8_t)      - bytecode [ A1, 02 ]    - stack: [10]
//  (uint8_t 2)        - bytecode [ 02, 02 ]    - stack: [10, 2]
//  (MUL uint8_t)      - bytecode [ A3, 02 ]    - stack: [20]
//  (uint8_t 5)        - bytecode [ 02, 05 ]    - stack: [20, 5]
//  (DIV uint8_t)      - bytecode [ A4, 02 ]    - stack: [4]
//  (uint8_t 3)        - bytecode [ 02, 03 ]    - stack: [4, 3]
//  (SUB uint8_t)      - bytecode [ A2, 02 ]    - stack: [1]

// Bytecode buffer:
//  02 04 02 06 A1 02 02 02 A3 02 02 05 A4 02 02 03 A2 02


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

    POINTER = 0x10,     // Pointer (uint16_t) to variable, requires data type as argument

    // Arithmetic operations
    ADD = 0x20,         // Addition, requires data type as argument
    SUB,                // Subtraction, requires data type as argument
    MUL,                // Multiplication, requires data type as argument
    DIV,                // Division, requires data type as argument
    MOD,                // Modulo, requires data type as argument

    POW,                // Power for given type. Example: POW uint8_t
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
        case POINTER:
        case ADD:
        case SUB:
        case MUL:
        case DIV:
        case MOD:
        case POW:
        case SQRT:
        case MIN:
        case MAX:
        case ABS:
        case MAP:
        case CON:
        case RAND:
        case RAND1:
        case RAND2:
        case LN:
        case LOG10:
        case LOG2:
        case EXP:
        case SIN:
        case COS:
        case TAN:
        case ASIN:
        case ACOS:
        case ATAN:
        case ATAN2:
        case SINH:
        case COSH:
        case TANH:
        case ASINH:
        case ACOSH:
        case ATANH:
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

const __FlashStringHelper* OPCODE_NAME(PLCRuntimeInstructionSet opcode) {
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
        case POINTER: return F("POINTER");
        case ADD: return F("ADD");
        case SUB: return F("SUB");
        case MUL: return F("MUL");
        case DIV: return F("DIV");
        case MOD: return F("MOD");
        case POW: return F("POW");
        case SQRT: return F("SQRT");
        case MIN: return F("MIN");
        case MAX: return F("MAX");
        case ABS: return F("ABS");
        case MAP: return F("MAP");
        case CON: return F("CON");
        case RAND: return F("RAND");
        case RAND1: return F("RAND1");
        case RAND2: return F("RAND2");
        case LN: return F("LN");
        case LOG10: return F("LOG10");
        case LOG2: return F("LOG2");
        case EXP: return F("EXP");
        case SIN: return F("SIN");
        case COS: return F("COS");
        case TAN: return F("TAN");
        case ASIN: return F("ASIN");
        case ACOS: return F("ACOS");
        case ATAN: return F("ATAN");
        case ATAN2: return F("ATAN2");
        case SINH: return F("SINH");
        case COSH: return F("COSH");
        case TANH: return F("TANH");
        case ASINH: return F("ASINH");
        case ACOSH: return F("ACOSH");
        case ATANH: return F("ATANH");
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
        default: break;
    }
    return 0;
}