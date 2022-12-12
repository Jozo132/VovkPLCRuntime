// runtime-program.h - 1.0.0 - 2022-12-11
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

class RuntimeProgram {
public:
    uint8_t* program = NULL; // PLC program to execute
    uint16_t MAX_PROGRAM_SIZE = 0; // Max program size in bytes
    uint16_t program_size = 0; // Current program size in bytes
    uint16_t program_line = 0; // Active program line

    RuntimeProgram(uint8_t* program = nullptr, uint16_t program_size = 0) {
        this->program = program;
        this->MAX_PROGRAM_SIZE = program_size;
        this->program_size = program_size;
    }
    RuntimeProgram(uint16_t program_size) {
        this->MAX_PROGRAM_SIZE = program_size;
        this->program = new uint8_t[program_size];
    }

    void erase() {
        program_size = 0;
        program_line = 0;
    }

    // Set the active PLC Program line number
    RuntimeError setLine(uint16_t line_number) {
        if (line_number < 0 || line_number >= MAX_PROGRAM_SIZE) return RTE_INVALID_LINE_NUMBER;
        program_line = line_number;
        return RTE_SUCCESS;
    }

    // Get the active PLC Program line number
    uint16_t getLine() { return program_line; }

    // Reset the active PLC Program line number to the beginning
    void resetLine() { program_line = 0; }

    bool finished() { return program_line >= program_size; }

    uint16_t getProgramSize() { return program_size; }

    void print() {
        Serial.print(F("Program[")); Serial.print(program_size);
        if (program_size == 0) {
            Serial.print(F("] <Empty>"));
            return;
        }
        Serial.print(F("] <Buffer "));
        for (uint16_t i = 0; i < program_size; i++) {
            uint8_t value = program[i];
            if (value < 0x10) Serial.print('0');
            Serial.print(value, HEX);
            if (i < program_size - 1) Serial.print(' ');
        }
        Serial.print('>');
    }

    void println() { print(); Serial.println(); }

    PLCRuntimeInstructionSet getCurrentBytecode() {
        if (program_line >= program_size) return EXIT;
        return (PLCRuntimeInstructionSet) program[program_line];
    }

    uint8_t sizeOfCurrentBytecode() { return get_OPCODE_SIZE(getCurrentBytecode()); }

    RuntimeError printOpcodeAt(uint16_t index) {
        if (index >= program_size) return RTE_INVALID_LINE_NUMBER;
        uint8_t opcode = program[index];
        uint8_t opcode_size = get_OPCODE_SIZE((PLCRuntimeInstructionSet) opcode);
#ifdef __RUNTIME_TEST_ADVANCED_DEBUGGING__
        Serial.print(F("Opcode["));
        print_OPCODE_NAME((PLCRuntimeInstructionSet) opcode);
        if (opcode_size > 0) Serial.print(F("] <Buffer "));
        else Serial.print(F("] <Empty"));
#else
        if (opcode_size > 0)Serial.print(F("Opcode <Buffer "));
        else Serial.print(F("Opcode <Empty"));
#endif
        for (uint8_t i = 0; i < opcode_size; i++) {
            uint8_t value = program[index + i];
            if (value < 0x10) Serial.print('0');
            Serial.print(value, HEX);
            if (i < opcode_size - 1) Serial.print(' ');
        }
        Serial.print('>');
        return RTE_SUCCESS;
    }
    RuntimeError printlnOpcodeAt(uint16_t index) {
        RuntimeError error = printOpcodeAt(index);
        Serial.println();
        return error;
    }



    // Push a new sequence of bytes to the PLC Program
    RuntimeError push(uint8_t* code, uint16_t code_size) {
        if (program_size + code_size > MAX_PROGRAM_SIZE) return RTE_PROGRAM_SIZE_EXCEEDED;
        memcpy(program + program_size, code, code_size);
        program_size += code_size;
        return RTE_SUCCESS;
    }

    // Push a new instruction to the PLC Program
    RuntimeError push(uint8_t instruction) {
        if (program_size + 1 > MAX_PROGRAM_SIZE) return RTE_PROGRAM_SIZE_EXCEEDED;
        program[program_size] = instruction;
        program_size++;
        return RTE_SUCCESS;
    }

    // Push a new instruction to the PLC Program
    RuntimeError push(uint8_t instruction, uint8_t data_type) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) return RTE_PROGRAM_SIZE_EXCEEDED;
        program[program_size] = instruction;
        program[program_size + 1] = data_type;
        program_size += 2;
        return RTE_SUCCESS;
    }

    // Push a boolean value to the PLC Program
    RuntimeError pushBool(bool value) {
        return pushU8(value ? 1 : 0);
    }

    // Push an uint8_t value to the PLC Program
    RuntimeError pushU8(uint8_t value) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) return RTE_PROGRAM_SIZE_EXCEEDED;
        program[program_size] = U8;
        program[program_size + 1] = value;
        program_size += 2;
        return RTE_SUCCESS;
    }

    // Push an int8_t value to the PLC Program
    RuntimeError pushS8(int8_t value) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) return RTE_PROGRAM_SIZE_EXCEEDED;
        program[program_size] = S8;
        program[program_size + 1] = value;
        program_size += 2;
        return RTE_SUCCESS;
    }

    // Push an uint16_t value to the PLC Program
    RuntimeError pushU16(uint16_t value) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) return RTE_PROGRAM_SIZE_EXCEEDED;
        program[program_size] = U16;
        program[program_size + 1] = value >> 8;
        program[program_size + 2] = value & 0xFF;
        program_size += 3;
        return RTE_SUCCESS;
    }

    // Push an int16_t value to the PLC Program
    RuntimeError pushS16(int16_t value) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) return RTE_PROGRAM_SIZE_EXCEEDED;
        program[program_size] = S16;
        program[program_size + 1] = value >> 8;
        program[program_size + 2] = value & 0xFF;
        program_size += 3;
        return RTE_SUCCESS;
    }

    // Push an uint32_t value to the PLC Program
    RuntimeError pushU32(uint32_t value) {
        if (program_size + 5 > MAX_PROGRAM_SIZE) return RTE_PROGRAM_SIZE_EXCEEDED;
        program[program_size] = U32;
        program[program_size + 1] = value >> 24;
        program[program_size + 2] = (value >> 16) & 0xFF;
        program[program_size + 3] = (value >> 8) & 0xFF;
        program[program_size + 4] = value & 0xFF;
        program_size += 5;
        return RTE_SUCCESS;
    }

    // Push an int32_t value to the PLC Program
    RuntimeError pushS32(int32_t value) {
        if (program_size + 5 > MAX_PROGRAM_SIZE) return RTE_PROGRAM_SIZE_EXCEEDED;
        program[program_size] = S32;
        program[program_size + 1] = value >> 24;
        program[program_size + 2] = (value >> 16) & 0xFF;
        program[program_size + 3] = (value >> 8) & 0xFF;
        program[program_size + 4] = value & 0xFF;
        program_size += 5;
        return RTE_SUCCESS;
    }

    // Push an uint64_t value to the PLC Program
    RuntimeError pushU64(uint64_t value) {
        if (program_size + 9 > MAX_PROGRAM_SIZE) return RTE_PROGRAM_SIZE_EXCEEDED;
        program[program_size] = U64;
        program[program_size + 1] = value >> 56;
        program[program_size + 2] = (value >> 48) & 0xFF;
        program[program_size + 3] = (value >> 40) & 0xFF;
        program[program_size + 4] = (value >> 32) & 0xFF;
        program[program_size + 5] = (value >> 24) & 0xFF;
        program[program_size + 6] = (value >> 16) & 0xFF;
        program[program_size + 7] = (value >> 8) & 0xFF;
        program[program_size + 8] = value & 0xFF;
        program_size += 9;
        return RTE_SUCCESS;
    }

    // Push an int64_t value to the PLC Program
    RuntimeError pushS64(int64_t value) {
        if (program_size + 9 > MAX_PROGRAM_SIZE) return RTE_PROGRAM_SIZE_EXCEEDED;
        program[program_size] = S64;
        program[program_size + 1] = value >> 56;
        program[program_size + 2] = (value >> 48) & 0xFF;
        program[program_size + 3] = (value >> 40) & 0xFF;
        program[program_size + 4] = (value >> 32) & 0xFF;
        program[program_size + 5] = (value >> 24) & 0xFF;
        program[program_size + 6] = (value >> 16) & 0xFF;
        program[program_size + 7] = (value >> 8) & 0xFF;
        program[program_size + 8] = value & 0xFF;
        program_size += 9;
        return RTE_SUCCESS;
    }

    // Push a float value to the PLC Program
    RuntimeError pushF32(float value) {
        if (program_size + 5 > MAX_PROGRAM_SIZE) return RTE_PROGRAM_SIZE_EXCEEDED;
        program[program_size] = F32;
        uint32_t* value_ptr = (uint32_t*) &value;
        program[program_size + 1] = *value_ptr >> 24;
        program[program_size + 2] = (*value_ptr >> 16) & 0xFF;
        program[program_size + 3] = (*value_ptr >> 8) & 0xFF;
        program[program_size + 4] = *value_ptr & 0xFF;
        program_size += 5;
        return RTE_SUCCESS;
    }

    // Push a double value to the PLC Program
    RuntimeError pushF64(double value) {
        if (program_size + 9 > MAX_PROGRAM_SIZE) return RTE_PROGRAM_SIZE_EXCEEDED;
        program[program_size] = F64;
        uint64_t* value_ptr = (uint64_t*) &value;
        program[program_size + 1] = *value_ptr >> 56;
        program[program_size + 2] = (*value_ptr >> 48) & 0xFF;
        program[program_size + 3] = (*value_ptr >> 40) & 0xFF;
        program[program_size + 4] = (*value_ptr >> 32) & 0xFF;
        program[program_size + 5] = (*value_ptr >> 24) & 0xFF;
        program[program_size + 6] = (*value_ptr >> 16) & 0xFF;
        program[program_size + 7] = (*value_ptr >> 8) & 0xFF;
        program[program_size + 8] = *value_ptr & 0xFF;
        program_size += 9;
        return RTE_SUCCESS;
    }

    // Push flow control instructions to the PLC Program
    RuntimeError pushJMP(uint16_t program_address) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) return RTE_PROGRAM_SIZE_EXCEEDED;
        program[program_size] = JMP;
        program[program_size + 1] = program_address >> 8;
        program[program_size + 2] = program_address & 0xFF;
        program_size += 3;
        return RTE_SUCCESS;
    }
    RuntimeError pushJMP_IF(uint16_t program_address) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) return RTE_PROGRAM_SIZE_EXCEEDED;
        program[program_size] = JMP_IF;
        program[program_size + 1] = program_address >> 8;
        program[program_size + 2] = program_address & 0xFF;
        program_size += 3;
        return RTE_SUCCESS;
    }
    RuntimeError pushJMP_IF_NOT(uint16_t program_address) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) return RTE_PROGRAM_SIZE_EXCEEDED;
        program[program_size] = JMP_IF_NOT;
        program[program_size + 1] = program_address >> 8;
        program[program_size + 2] = program_address & 0xFF;
        program_size += 3;
        return RTE_SUCCESS;
    }
};