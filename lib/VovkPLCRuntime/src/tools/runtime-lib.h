// runtime-lib.h - 1.0.0 - 2022-12-11
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

#include <Arduino.h>

#include "runtime-stack.h"
#include "runtime-instructions.h"
#include "runtime-arithmetics.h"
#include "runtime-program.h"

class VovkPLCRuntime {
public:
    MyStack* stack; // Active memory stack for PLC execution
    uint16_t max_stack_size = 0; // Maximum stack size
    uint8_t* memory = NULL; // PLC memory to manipulate
    uint16_t memory_size = 0; // Memory size in bytes
    VovkPLCRuntime(uint16_t max_stack_size = 32, uint8_t* memory = nullptr, uint16_t memory_size = 0) {
        stack = new MyStack(max_stack_size);
        this->memory = memory;
        this->memory_size = memory_size;
        this->max_stack_size = max_stack_size;
    }
    // Clear the stack
    void clear();
    // Clear the stack and reset the program line
    void clear(RuntimeProgram& program);
    // Execute one PLC instruction at index, returns an error code (0 on success)
    RuntimeError step(uint8_t* program, uint16_t program_size, uint16_t& index);
    // Execute the whole PLC program, returns an error code (0 on success)
    RuntimeError run(uint8_t* program, uint16_t program_size);
    // Execute one PLC instruction, returns an error code (0 on success)
    RuntimeError step(RuntimeProgram& program);
    // Run/Continue the whole PLC program from where it left off, returns an error code (0 on success)
    RuntimeError run(RuntimeProgram& program);
    // Run the whole PLC program from the beginning, returns an error code (0 on success)
    RuntimeError cleanRun(RuntimeProgram& program) {
        clear(program);
        return run(program);
    }
    // Read a custom type T value from the stack. This will pop the stack by sizeof(T) bytes and return the value.
    template <typename T> T read() {
        if (stack->size() < (uint16_t) sizeof(T)) return 0;
        uint8_t temp[sizeof(T)];
        for (uint16_t i = 0; i < (uint16_t) sizeof(T); i++)
            temp[i] = stack->pop();
        T value = *(T*) temp;
        return value;
    }

    // Print the stack buffer to the serial port
    void printStack();
};









/*
// Readable RPN code example:
//                - bytecode []            - stack: []
//  (U8 4)        - bytecode [ 02, 04 ]    - stack: [4]
//  (U8 6)        - bytecode [ 02, 06 ]    - stack: [4, 6]
//  (ADD_U8)      - bytecode [ A1 ]        - stack: [10]
//  (U8 2)        - bytecode [ 02, 02 ]    - stack: [10, 2]
//  (MUL_U8)      - bytecode [ A3 ]        - stack: [20]
//  (U8 5)        - bytecode [ 02, 05 ]    - stack: [20, 5]
//  (DIV_U8)      - bytecode [ A4 ]        - stack: [4]
//  (U8 3)        - bytecode [ 02, 03 ]    - stack: [4, 3]
//  (SUB_U8)      - bytecode [ A2 ]        - stack: [1]

// Bytecode buffer:
//  02 04 02 06 A1 02 02 A3 02 05 A4 02 03 A2


// Instruction set:
enum PLCRuntimeInstructionSet {
    UNDEFINED = 0x00,   // Undefined instruction, same as NOP

    BOOL = 0x01,        // Constant boolean value

    U8 = 0x02,          // Constant uint8_t value
    S8 = 0x03,          // Constant int8_t value

    U16 = 0x04,         // Constant uint16_t integer value
    S16 = 0x05,         // Constant int16_t integer value

    U32 = 0x06,         // Constant uint32_t integer value
    S32 = 0x07,         // Constant int32_t integer value

    U64 = 0x08,         // Constant uint64_t integer value
    S64 = 0x09,         // Constant int64_t integer value

    F32 = 0x0A,         // Constant float value
    F64 = 0x0B,         // Constant double value

    ADD_U8 = 0xA1,         // Addition for uint8_t
    SUB_U8 = 0xA2,         // Subtraction for uint8_t
    MUL_U8 = 0xA3,         // Multiplication for uint8_t
    DIV_U8 = 0xA4,         // Division for uint8_t

    ADD_U16 = 0xA5,        // Addition for uint16_t
    SUB_U16 = 0xA6,        // Subtraction for uint16_t
    MUL_U16 = 0xA7,        // Multiplication for uint16_t
    DIV_U16 = 0xA8,        // Division for uint16_t

    ADD_U32 = 0xA9,        // Addition for uint32_t
    SUB_U32 = 0xAA,        // Subtraction for uint32_t
    MUL_U32 = 0xAB,        // Multiplication for uint32_t
    DIV_U32 = 0xAC,        // Division for uint32_t

    ADD_U64 = 0xAD,        // Addition for uint64_t
    SUB_U64 = 0xAE,        // Subtraction for uint64_t
    MUL_U64 = 0xAF,        // Multiplication for uint64_t
    DIV_U64 = 0xB0,        // Division for uint64_t

    ADD_S8 = 0xB1,         // Addition for int8_t
    SUB_S8 = 0xB2,         // Subtraction for int8_t
    MUL_S8 = 0xB3,         // Multiplication for int8_t
    DIV_S8 = 0xB4,         // Division for int8_t

    ADD_S16 = 0xB5,        // Addition for int16_t
    SUB_S16 = 0xB6,        // Subtraction for int16_t
    MUL_S16 = 0xB7,        // Multiplication for int16_t
    DIV_S16 = 0xB8,        // Division for int16_t

    ADD_S32 = 0xB9,        // Addition for int32_t
    SUB_S32 = 0xBA,        // Subtraction for int32_t
    MUL_S32 = 0xBB,        // Multiplication for int32_t
    DIV_S32 = 0xBC,        // Division for int32_t

    ADD_S64 = 0xBD,        // Addition for int64_t
    SUB_S64 = 0xBE,        // Subtraction for int64_t
    MUL_S64 = 0xBF,        // Multiplication for int64_t
    DIV_S64 = 0xC0,        // Division for int64_t

    ADD_F32 = 0xC1,        // Addition for float
    SUB_F32 = 0xC2,        // Subtraction for float
    MUL_F32 = 0xC3,        // Multiplication for float
    DIV_F32 = 0xC4,        // Division for float

    ADD_F64 = 0xC5,        // Addition for double
    SUB_F64 = 0xC6,        // Subtraction for double
    MUL_F64 = 0xC7,        // Multiplication for double
    DIV_F64 = 0xC8,        // Division for double
};
*/





// Clear the runtime stack
void VovkPLCRuntime::clear() { stack->clear(); }
// Clear the runtime stack and reset the program line
void VovkPLCRuntime::clear(RuntimeProgram& program) {
    program.resetLine();
    stack->clear();
}

// Print the stack
void VovkPLCRuntime::printStack() { stack->print(); }


// Execute the whole PLC program, returns an erro code (0 on success)
RuntimeError VovkPLCRuntime::run(RuntimeProgram& program) { return run(program.program, program.program_size); }

// Execute the whole PLC program, returns an erro code (0 on success)
RuntimeError VovkPLCRuntime::run(uint8_t* program, uint16_t program_size) {
    uint16_t index = 0;
    while (index < program_size) {
        RuntimeError status = step(program, program_size, index);
        if (status != RTE_SUCCESS) {
            if (status == RTE_PROGRAM_EXITED) {
                return RTE_SUCCESS;
            }
            return status;
        }
    }
    return RTE_SUCCESS;
}




// Execute one PLC instruction at index, returns an error code (0 on success)
RuntimeError VovkPLCRuntime::step(RuntimeProgram& program) { return step(program.program, program.program_size, program.program_line); }

// Execute one PLC instruction at index, returns an error code (0 on success)
RuntimeError VovkPLCRuntime::step(uint8_t* program, uint16_t program_size, uint16_t& index) {
    if (program_size == 0) return RTE_EMPTY_PROGRAM;
    if (index >= program_size) return RTE_PROGRAM_SIZE_EXCEEDED;
    uint8_t opcode = program[index];
    index++;
    switch (opcode) {
        case NOP: return RTE_SUCCESS;
        case LOGIC_AND: return PLCMethods.LOGIC_AND(this->stack);
        case LOGIC_OR: return PLCMethods.LOGIC_OR(this->stack);
        case LOGIC_NOT: return PLCMethods.LOGIC_NOT(this->stack);
        case LOGIC_XOR: return PLCMethods.LOGIC_XOR(this->stack);
        case JMP: return PLCMethods.handle_JMP(this->stack, program, program_size, index);
        case JMP_IF: return PLCMethods.handle_JMP_IF(this->stack, program, program_size, index);
        case JMP_IF_NOT: return PLCMethods.handle_JMP_IF_NOT(this->stack, program, program_size, index);
        case CALL: return PLCMethods.handle_CALL(this->stack, program, program_size, index);
        case CALL_IF: return PLCMethods.handle_CALL_IF(this->stack, program, program_size, index);
        case CALL_IF_NOT: return PLCMethods.handle_CALL_IF_NOT(this->stack, program, program_size, index);
        case RET: return PLCMethods.handle_RET(this->stack, program, program_size, index);
        case BOOL: return PLCMethods.PUSH_BOOL(this->stack, program, program_size, index);
        case U8: return PLCMethods.PUSH_U8(this->stack, program, program_size, index);
        case S8: return PLCMethods.PUSH_S8(this->stack, program, program_size, index);
        case U16: return PLCMethods.PUSH_U16(this->stack, program, program_size, index);
        case S16: return PLCMethods.PUSH_S16(this->stack, program, program_size, index);
        case U32: return PLCMethods.PUSH_U32(this->stack, program, program_size, index);
        case S32: return PLCMethods.PUSH_S32(this->stack, program, program_size, index);
        case U64: return PLCMethods.PUSH_U64(this->stack, program, program_size, index);
        case S64: return PLCMethods.PUSH_S64(this->stack, program, program_size, index);
        case F32: return PLCMethods.PUSH_F32(this->stack, program, program_size, index);
        case F64: return PLCMethods.PUSH_F64(this->stack, program, program_size, index);
        case ADD: return PLCMethods.handle_ADD(this->stack, program, program_size, index);
        case SUB: return PLCMethods.handle_SUB(this->stack, program, program_size, index);
        case MUL: return PLCMethods.handle_MUL(this->stack, program, program_size, index);
        case DIV: return PLCMethods.handle_DIV(this->stack, program, program_size, index);
        case BW_AND_X8: return PLCMethods.BW_AND_X8(this->stack);
        case BW_AND_X16: return PLCMethods.BW_AND_X16(this->stack);
        case BW_AND_X32: return PLCMethods.BW_AND_X32(this->stack);
        case BW_AND_X64: return PLCMethods.BW_AND_X64(this->stack);
        case BW_OR_X8: return PLCMethods.BW_OR_X8(this->stack);
        case BW_OR_X16: return PLCMethods.BW_OR_X16(this->stack);
        case BW_OR_X32: return PLCMethods.BW_OR_X32(this->stack);
        case BW_OR_X64: return PLCMethods.BW_OR_X64(this->stack);
        case BW_XOR_X8: return PLCMethods.BW_XOR_X8(this->stack);
        case BW_XOR_X16: return PLCMethods.BW_XOR_X16(this->stack);
        case BW_XOR_X32: return PLCMethods.BW_XOR_X32(this->stack);
        case BW_XOR_X64: return PLCMethods.BW_XOR_X64(this->stack);
        case BW_NOT_X8: return PLCMethods.BW_NOT_X8(this->stack);
        case BW_NOT_X16: return PLCMethods.BW_NOT_X16(this->stack);
        case BW_NOT_X32: return PLCMethods.BW_NOT_X32(this->stack);
        case BW_NOT_X64: return PLCMethods.BW_NOT_X64(this->stack);
        case CMP_EQ: return PLCMethods.handle_CMP_EQ(this->stack, program, program_size, index);
        case CMP_NEQ: return PLCMethods.handle_CMP_NEQ(this->stack, program, program_size, index);
        case CMP_GT: return PLCMethods.handle_CMP_GT(this->stack, program, program_size, index);
        case CMP_GTE: return PLCMethods.handle_CMP_GTE(this->stack, program, program_size, index);
        case CMP_LT: return PLCMethods.handle_CMP_LT(this->stack, program, program_size, index);
        case CMP_LTE: return PLCMethods.handle_CMP_LTE(this->stack, program, program_size, index);
        case EXIT: {
            return RTE_PROGRAM_EXITED;
            // return PLCMethods.handle_EXIT(this->stack, program, program_size, index);
        }
        default: return RTE_UNKNOWN_INSTRUCTION;
    }
}