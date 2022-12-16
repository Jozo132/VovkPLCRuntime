// methods-flow.h - 1.0.0 - 2022-12-11
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




RuntimeError printOpcodeAt(const uint8_t* program, uint16_t size, uint16_t index) {
    if (index >= size) return INVALID_PROGRAM_INDEX;
    PLCRuntimeInstructionSet opcode = (PLCRuntimeInstructionSet) program[index];
    bool valid_opcode = OPCODE_EXISTS(opcode);
    if (!valid_opcode) {
        Serial.print(F("Opcode [0x"));
        if (opcode < 0x10) Serial.print('0');
        Serial.print(opcode, HEX);
        Serial.print(F("] <Invalid>"));
        return INVALID_INSTRUCTION;
    }
    uint8_t opcode_size = OPCODE_SIZE(opcode);
    Serial.print(F("Opcode ("));
    Serial.print(OPCODE_NAME(opcode));
    Serial.print(F(") ["));
    for (uint8_t i = 0; i < opcode_size; i++) {
        uint8_t value = program[index + i];
        if (value < 0x10) Serial.print('0');
        Serial.print(value, HEX);
        if (i < opcode_size - 1) Serial.print(' ');
    }
    Serial.print(']');
    return STATUS_SUCCESS;
}
RuntimeError printlnOpcodeAt(const uint8_t* program, uint16_t size, uint16_t index) {
    RuntimeError error = printOpcodeAt(program, size, index);
    Serial.println();
    return error;
}

#ifdef __RUNTIME_DEBUG__
#define CHECK_PROGRAM_POINTER_BOUNDS_HEAD                               \
{                                                                       \
    Serial.print(F("Program pointer points out of bounds: "));          \
    Serial.print(index);                                                \
    Serial.print(F(" > "));                                             \
    Serial.println(program_size);                                       \
    printOpcodeAt(program, program_size, index_start);                  \
    return PROGRAM_POINTER_OUT_OF_BOUNDS;                               \
}
#else
#define CHECK_PROGRAM_POINTER_BOUNDS_HEAD                               \
    return PROGRAM_POINTER_OUT_OF_BOUNDS; 
#endif

namespace PLCMethods {

    RuntimeError handle_JMP(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        IGNORE_UNUSED uint16_t index_start = index;
        uint16_t size = 2;
        if (index + size > program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
        u8A_to_u16 cvt;
        cvt.u8A[1] = program[index];
        cvt.u8A[0] = program[index + 1];
        index = cvt.u16;
        if (index >= program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
        return STATUS_SUCCESS;
    }
    RuntimeError handle_JMP_IF(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        IGNORE_UNUSED uint16_t index_start = index;
        uint16_t size = 2;
        if (index + size > program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
        if (stack->pop_bool()) {
            u8A_to_u16 cvt;
            cvt.u8A[1] = program[index];
            cvt.u8A[0] = program[index + 1];
            index = cvt.u16;
            if (index >= program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
        } else index += size;
        return STATUS_SUCCESS;
    }
    RuntimeError handle_JMP_IF_NOT(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        IGNORE_UNUSED uint16_t index_start = index;
        uint16_t size = 2;
        if (index + size > program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
        if (!stack->pop_bool()) {
            u8A_to_u16 cvt;
            cvt.u8A[1] = program[index];
            cvt.u8A[0] = program[index + 1];
            index = cvt.u16;
            if (index >= program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
        } else index += size;
        return STATUS_SUCCESS;
    }

    RuntimeError handle_CALL(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        IGNORE_UNUSED uint16_t index_start = index;
        uint16_t size = 2;
        if (index + size > program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
        u8A_to_u16 cvt;
        cvt.u8A[1] = program[index];
        cvt.u8A[0] = program[index + 1];
        index = cvt.u16;
        if (index >= program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
        RuntimeError call_store_status = stack->pushCall(index);
        if (call_store_status != STATUS_SUCCESS) return call_store_status;
        if (index >= program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
        return STATUS_SUCCESS;
    }
    RuntimeError handle_CALL_IF(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        IGNORE_UNUSED uint16_t index_start = index;
        uint16_t size = 2;
        if (index + size > program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
        if (stack->pop_bool()) {
            u8A_to_u16 cvt;
            cvt.u8A[1] = program[index];
            cvt.u8A[0] = program[index + 1];
            index = cvt.u16;
            if (index == 0 || index >= program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
            RuntimeError call_store_status = stack->pushCall(index);
            if (call_store_status != STATUS_SUCCESS) return call_store_status;
            if (index >= program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
        } else index += size;
        return STATUS_SUCCESS;
    }
    RuntimeError handle_CALL_IF_NOT(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        IGNORE_UNUSED uint16_t index_start = index;
        uint16_t size = 2;
        if (index + size > program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
        if (!stack->pop_bool()) {
            u8A_to_u16 cvt;
            cvt.u8A[1] = program[index];
            cvt.u8A[0] = program[index + 1];
            index = cvt.u16;
            if (index == 0 || index >= program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
            RuntimeError call_store_status = stack->pushCall(index);
            if (call_store_status != STATUS_SUCCESS) return call_store_status;
            if (index >= program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
        } else index += size;
        return STATUS_SUCCESS;
    }
    RuntimeError handle_RET(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        IGNORE_UNUSED uint16_t index_start = index;
        if (stack->call_stack->size() == 0) return CALL_STACK_UNDERFLOW;
        uint16_t ret_index = stack->popCall();
        index = ret_index;

        if (index >= program_size) CHECK_PROGRAM_POINTER_BOUNDS_HEAD;
        return STATUS_SUCCESS;
    }


    RuntimeError handle_EXIT(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        IGNORE_UNUSED uint16_t index_start = index;
        if (index >= program_size) return STATUS_SUCCESS;
        uint8_t exit_code = program[index++];
        return (RuntimeError) exit_code;
    }

}