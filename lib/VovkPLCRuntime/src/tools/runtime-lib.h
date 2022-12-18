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

#include "runtime-tools.h"
#include "stack/stack-struct.h"
#include "runtime-memory.h"
#include "runtime-instructions.h"
#include "stack/runtime-stack.h"
#include "arithmetics/runtime-arithmetics.h"
#include "runtime-program.h"
#include "runtime-cmd-parser.h"

class VovkPLCRuntime {
private:
    bool started_up = false;
public:
    void disableStartupMessage() {
        started_up = true;
    }
    void startup() {
        if (!started_up) {
            started_up = true;
            Serial.println();
            REPRINTLN(70, ':');
            Serial.println(F(":: Using VovkPLCRuntime Library - Author J.Vovk <jozo132@gmail.com> ::"));
            REPRINTLN(70, ':');
        }
    }
    RuntimeStack* stack; // Active memory stack for PLC execution
    uint16_t max_stack_size = 0; // Maximum stack size
    Stack<uint8_t> memory; // PLC memory to manipulate
    RuntimeProgram* program = nullptr; // Active PLC program
    RuntimeCommandParser cmd_parser; // Command parser for PLC commands
    void formatMemory(uint16_t size, uint8_t* data = nullptr) {
        if (size == 0) return;
        memory.format(size);
        if (data == nullptr) return;
        for (uint16_t i = 0; i < size; i++)
            memory.push(data[i]);
    }

    VovkPLCRuntime(uint16_t max_stack_size, uint16_t memory_size = 0) {
        stack = new RuntimeStack(max_stack_size);
        this->max_stack_size = max_stack_size;
        formatMemory(memory_size);
    }
    VovkPLCRuntime(uint16_t max_stack_size, uint16_t memory_size, uint8_t* program, uint16_t program_size) {
        stack = new RuntimeStack(max_stack_size);
        this->max_stack_size = max_stack_size;
        this->program = new RuntimeProgram(program, program_size);
        formatMemory(memory_size);
    }
    VovkPLCRuntime(uint16_t max_stack_size, RuntimeProgram& program) {
        stack = new RuntimeStack(max_stack_size);
        this->max_stack_size = max_stack_size;
        this->program = &program;
    }
    VovkPLCRuntime(uint16_t max_stack_size, uint16_t memory_size, RuntimeProgram& program) {
        stack = new RuntimeStack(max_stack_size);
        this->max_stack_size = max_stack_size;
        this->program = &program;
        formatMemory(memory_size);
    }

    void attachProgram(RuntimeProgram& program) {
        this->program = &program;
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
    // Execute one PLC instruction at index, returns an error code (0 on success)
    RuntimeError step(uint16_t& index) {
        if (program == NULL) return NO_PROGRAM;
        return step(program->program, program->program_size, index);
    }
    // Execute the whole PLC program, returns an error code (0 on success)
    RuntimeError run() {
        if (!started_up) startup();
        if (program == NULL) return NO_PROGRAM;
        return run(program->program, program->program_size);
    }
    // Run the whole PLC program from the beginning, returns an error code (0 on success)
    RuntimeError cleanRun(RuntimeProgram& program) {
        if (!started_up) startup();
        clear(program);
        return run(program);
    }
    // Run the whole PLC program from the beginning, returns an error code (0 on success)
    RuntimeError cleanRun() {
        if (!started_up)startup();
        if (program == NULL) return NO_PROGRAM;
        clear();
        return run(program->program, program->program_size);
    }
    // Read a custom type T value from the stack. This will pop the stack by sizeof(T) bytes and return the value.
    template <typename T> T read() {
        if (!started_up) startup();
        if (stack->size() < (uint16_t) sizeof(T)) return 0;
        uint8_t temp[sizeof(T)];
        for (uint16_t i = 0; i < (uint16_t) sizeof(T); i++)
            temp[i] = stack->pop();
        T value = *(T*) temp;
        return value;
    }

    // Print the stack buffer to the serial port
    int printStack();
};

// Clear the runtime stack
void VovkPLCRuntime::clear() {
    if (program != nullptr) program->resetLine();
    stack->clear();
}
// Clear the runtime stack and reset the program line
void VovkPLCRuntime::clear(RuntimeProgram& program) {
    program.resetLine();
    stack->clear();
}

// Print the stack
int VovkPLCRuntime::printStack() { return stack->print(); }


// Execute the whole PLC program, returns an erro code (0 on success)
RuntimeError VovkPLCRuntime::run(RuntimeProgram& program) { return run(program.program, program.program_size); }

// Execute the whole PLC program, returns an erro code (0 on success)
RuntimeError VovkPLCRuntime::run(uint8_t* program, uint16_t program_size) {
    if (!started_up) startup();
    uint16_t index = 0;
    while (index < program_size) {
        RuntimeError status = step(program, program_size, index);
        if (status != STATUS_SUCCESS) {
            if (status == PROGRAM_EXITED)
                return STATUS_SUCCESS;
            return status;
        }
    }
    return STATUS_SUCCESS;
}




// Execute one PLC instruction at index, returns an error code (0 on success)
RuntimeError VovkPLCRuntime::step(RuntimeProgram& program) { return step(program.program, program.program_size, program.program_line); }

// Execute one PLC instruction at index, returns an error code (0 on success)
RuntimeError VovkPLCRuntime::step(uint8_t* program, uint16_t program_size, uint16_t& index) {
    if (program_size == 0) return EMPTY_PROGRAM;
    if (index >= program_size) return PROGRAM_SIZE_EXCEEDED;
    uint8_t opcode = program[index];
    index++;
    switch (opcode) {
        case NOP: return STATUS_SUCCESS;
        case LOGIC_AND: return PLCMethods::LOGIC_AND(this->stack);
        case LOGIC_OR: return PLCMethods::LOGIC_OR(this->stack);
        case LOGIC_NOT: return PLCMethods::LOGIC_NOT(this->stack);
        case LOGIC_XOR: return PLCMethods::LOGIC_XOR(this->stack);
        case PUT: return PLCMethods::PUT(this->stack, program, program_size, index);
        case GET: return PLCMethods::GET(this->stack, program, program_size, index);
        case JMP: return PLCMethods::handle_JMP(this->stack, program, program_size, index);
        case JMP_IF: return PLCMethods::handle_JMP_IF(this->stack, program, program_size, index);
        case JMP_IF_NOT: return PLCMethods::handle_JMP_IF_NOT(this->stack, program, program_size, index);
        case CALL: return PLCMethods::handle_CALL(this->stack, program, program_size, index);
        case CALL_IF: return PLCMethods::handle_CALL_IF(this->stack, program, program_size, index);
        case CALL_IF_NOT: return PLCMethods::handle_CALL_IF_NOT(this->stack, program, program_size, index);
        case RET: return PLCMethods::handle_RET(this->stack, program, program_size, index);
        case type_bool: return PLCMethods::PUSH_bool(this->stack, program, program_size, index);
        case type_uint8_t: return PLCMethods::PUSH_uint8_t(this->stack, program, program_size, index);
        case type_int8_t: return PLCMethods::PUSH_int8_t(this->stack, program, program_size, index);
        case type_uint16_t: return PLCMethods::PUSH_uint16_t(this->stack, program, program_size, index);
        case type_int16_t: return PLCMethods::PUSH_int16_t(this->stack, program, program_size, index);
        case type_uint32_t: return PLCMethods::PUSH_uint32_t(this->stack, program, program_size, index);
        case type_int32_t: return PLCMethods::PUSH_int32_t(this->stack, program, program_size, index);
        case type_uint64_t: return PLCMethods::PUSH_uint64_t(this->stack, program, program_size, index);
        case type_int64_t: return PLCMethods::PUSH_int64_t(this->stack, program, program_size, index);
        case type_float: return PLCMethods::PUSH_float(this->stack, program, program_size, index);
        case type_double: return PLCMethods::PUSH_double(this->stack, program, program_size, index);
        case ADD: return PLCMethods::handle_ADD(this->stack, program, program_size, index);
        case SUB: return PLCMethods::handle_SUB(this->stack, program, program_size, index);
        case MUL: return PLCMethods::handle_MUL(this->stack, program, program_size, index);
        case DIV: return PLCMethods::handle_DIV(this->stack, program, program_size, index);
        case BW_AND_X8: return PLCMethods::BW_AND_X8(this->stack);
        case BW_AND_X16: return PLCMethods::BW_AND_X16(this->stack);
        case BW_AND_X32: return PLCMethods::BW_AND_X32(this->stack);
        case BW_AND_X64: return PLCMethods::BW_AND_X64(this->stack);
        case BW_OR_X8: return PLCMethods::BW_OR_X8(this->stack);
        case BW_OR_X16: return PLCMethods::BW_OR_X16(this->stack);
        case BW_OR_X32: return PLCMethods::BW_OR_X32(this->stack);
        case BW_OR_X64: return PLCMethods::BW_OR_X64(this->stack);
        case BW_XOR_X8: return PLCMethods::BW_XOR_X8(this->stack);
        case BW_XOR_X16: return PLCMethods::BW_XOR_X16(this->stack);
        case BW_XOR_X32: return PLCMethods::BW_XOR_X32(this->stack);
        case BW_XOR_X64: return PLCMethods::BW_XOR_X64(this->stack);
        case BW_NOT_X8: return PLCMethods::BW_NOT_X8(this->stack);
        case BW_NOT_X16: return PLCMethods::BW_NOT_X16(this->stack);
        case BW_NOT_X32: return PLCMethods::BW_NOT_X32(this->stack);
        case BW_NOT_X64: return PLCMethods::BW_NOT_X64(this->stack);
        case CMP_EQ: return PLCMethods::handle_CMP_EQ(this->stack, program, program_size, index);
        case CMP_NEQ: return PLCMethods::handle_CMP_NEQ(this->stack, program, program_size, index);
        case CMP_GT: return PLCMethods::handle_CMP_GT(this->stack, program, program_size, index);
        case CMP_GTE: return PLCMethods::handle_CMP_GTE(this->stack, program, program_size, index);
        case CMP_LT: return PLCMethods::handle_CMP_LT(this->stack, program, program_size, index);
        case CMP_LTE: return PLCMethods::handle_CMP_LTE(this->stack, program, program_size, index);
        case EXIT: {
            return PROGRAM_EXITED;
            // return PLCMethods::handle_EXIT(this->stack, program, program_size, index);
        }
        default: return UNKNOWN_INSTRUCTION;
    }
}