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
    RuntimeError status = UNDEFINED_STATE;

    RuntimeProgram(const uint8_t* program, uint16_t program_size) {
        this->MAX_PROGRAM_SIZE = program_size;
        this->program_size = program_size;
        this->program = (uint8_t*) program;
    }
    RuntimeProgram(uint16_t program_size) {
        this->MAX_PROGRAM_SIZE = program_size;
    }
    RuntimeProgram() { }
    ~RuntimeProgram() {
        if (program != NULL) delete [] program;
    }

    void begin(const uint8_t* program, uint16_t program_size) {
        format(program_size + 1);
        load(program, program_size);
    }
    void begin(uint16_t program_size = 0) {
        if (program_size == 0) return;
        MAX_PROGRAM_SIZE = program_size;
        if (MAX_PROGRAM_SIZE > 0 && program == nullptr) format(MAX_PROGRAM_SIZE);
    }

    void format(uint16_t program_size) {
        this->MAX_PROGRAM_SIZE = program_size;
        if (this->program != NULL) delete [] this->program;
        this->program = new uint8_t[program_size];
        this->program_size = 0;
        erase();
    }

    static uint32_t calculateChecksum(const uint8_t* data, uint16_t size) {
        uint32_t checksum = 0;
        for (uint32_t i = 0; i < size; i++) checksum += (uint32_t) data[i];
        return checksum;
    }

    RuntimeError loadUnsafe(const uint8_t* program, uint16_t program_size) {
        this->program = (uint8_t*) program;
        if (program_size > MAX_PROGRAM_SIZE) status = PROGRAM_SIZE_EXCEEDED;
        else if (program_size == 0) status = UNDEFINED_STATE;
        else status = STATUS_SUCCESS;
        this->MAX_PROGRAM_SIZE = program_size;
        this->program_size = program_size;
        return status;
    }

    RuntimeError load(const uint8_t* program, uint16_t program_size, uint32_t checksum = 0) {
        uint32_t calculated_checksum = calculateChecksum(program, program_size);
        if (calculated_checksum != checksum) {
            status = INVALID_CHECKSUM;
            Serial.println(F("Failed to load program: CHECKSUM MISMATCH"));
            return status;
        }
        return loadUnsafe(program, program_size);
    }

    // Get the size of used program memory
    uint16_t size() { return program_size; }

    // Hot update the running program. This is a very dangerous operation, so use it with caution!
    RuntimeError modify(uint16_t index, uint8_t value) {
        if (index >= program_size) return INVALID_PROGRAM_INDEX;
        program[index] = value;
        return STATUS_SUCCESS;
    }

    // Hot update the running program. This is a very dangerous operation, so use it with caution!
    RuntimeError modify(uint16_t index, uint8_t* data, uint16_t size) {
        if (index + size > program_size) return INVALID_PROGRAM_INDEX;
        for (uint16_t i = 0; i < size; i++) program[index + i] = data[i];
        return STATUS_SUCCESS;
    }

    RuntimeError modifyValue(uint16_t index, uint16_t value) {
        if (index + sizeof(uint16_t) > program_size) return INVALID_PROGRAM_INDEX;
        program[index] = value >> 8;
        program[index + 1] = value & 0xFF;
        return STATUS_SUCCESS;
    }


    void erase() {
        program_size = 0;
        program_line = 0;
        status = UNDEFINED_STATE;
    }

    // Set the active PLC Program line number
    RuntimeError setLine(uint16_t line_number) {
        if (line_number < 0 || line_number >= MAX_PROGRAM_SIZE) return INVALID_PROGRAM_INDEX;
        program_line = line_number;
        return STATUS_SUCCESS;
    }

    // Get the active PLC Program line number
    uint16_t getLine() { return program_line; }

    // Reset the active PLC Program line number to the beginning
    void resetLine() { program_line = 0; }

    bool finished() { return program_line >= program_size; }

    uint16_t getProgramSize() { return program_size; }

    int print() {
        int length = Serial.print(F("Program["));
        length += Serial.print(program_size);
        if (program_size == 0) {
            length += Serial.print(F("] []"));
            return length;
        }
        length += Serial.print(F("] ["));
        for (uint16_t i = 0; i < program_size; i++) {
            uint8_t value = program[i];
            if (value < 0x10) length += Serial.print('0');
            length += Serial.print(value, HEX);
            if (i < program_size - 1) length += Serial.print(' ');
        }
        length += Serial.print(']');
        return length;
    }

    int println() { int length = print(); length += Serial.println(); return length; }

    void explain() {
        Serial.println(F("#### Program Explanation:"));
        if (program_size == 0) {
            Serial.println(F("Program is empty."));
            return;
        }
        uint16_t index = 0;
        bool done = false;
        while (!done) {
            // Get current opcode
            PLCRuntimeInstructionSet opcode = (PLCRuntimeInstructionSet) program[index];
            // Serial.printf("    %4d [%02X %02X] - %02X: ", index, index >> 8, index & 0xFF, opcode);
            Serial.print(F("    "));
            print_number_padStart(index, 4);
            Serial.print(F(" ["));
            uint8_t hi = index >> 8;
            uint8_t lo = index & 0xFF;
            print_number_padStart(hi, 2, '0', HEX);
            Serial.print(' ');
            print_number_padStart(lo, 2, '0', HEX);
            Serial.print(F("] - "));
            print_number_padStart(opcode, 2, '0', HEX);
            Serial.print(F(": "));

            bool exists = OPCODE_EXISTS(opcode);
            if (!exists) {
                Serial.println(F("INVALID OPCODE\n"));
                return;
            }

            // Get current instruction size
            uint8_t instruction_size = OPCODE_SIZE(opcode);
            // Get current instruction name
            auto instruction_name = OPCODE_NAME(opcode);
            int length = Serial.print(instruction_name);
            length += Serial.print(' ');
            while (length < 13) length += Serial.print('-');
            Serial.print(' ');
            Serial.print(F("[size "));
            if (instruction_size < 10) Serial.print(' ');
            Serial.print(instruction_size);
            Serial.print(F("] "));
            // Print instruction arguments
            for (uint8_t i = 0; i < instruction_size; i++) {
                if ((i + index) >= program_size) {
                    Serial.println(F(" - OUT OF PROGRAM BOUNDS\n"));
                    return;
                }
                uint8_t value = program[i + index];
                // Serial.printf(" %02X", value);
                print_number_padStart(value, 2, '0', HEX);
            }
            Serial.println();
            index += instruction_size;
            if (index >= program_size) done = true;
        }
        Serial.println(F("#### End of program explanation."));
    }

    PLCRuntimeInstructionSet getCurrentBytecode() {
        if (program_line >= program_size) return EXIT;
        return (PLCRuntimeInstructionSet) program[program_line];
    }

    uint8_t sizeOfCurrentBytecode() { return OPCODE_SIZE(getCurrentBytecode()); }

    int printOpcodeAt(uint16_t index) {
        int length = 0;
        if (index >= program_size) return length;
        PLCRuntimeInstructionSet opcode = (PLCRuntimeInstructionSet) program[index];
        bool valid_opcode = OPCODE_EXISTS(opcode);
        if (!valid_opcode) {
            length += Serial.print(F("Opcode: [0x"));
            if (opcode < 0x10) length += Serial.print('0');
            length += Serial.print(opcode, HEX);
            length += Serial.print(F("] <Invalid>"));
            return length;
        }
        uint8_t opcode_size = OPCODE_SIZE(opcode);
        length += Serial.print(F("Opcode["));
        if (opcode < 0x10) length += Serial.print('0');
        length += Serial.print(opcode, HEX);
        length += Serial.print(F("]: "));
        length += Serial.print(OPCODE_NAME(opcode));
        length += Serial.print(' ');
        length += Serial.print(' ');
        while (length < 25) length += Serial.print('-');
        while (length < 27) length += Serial.print(' ');
        Serial.print('[');
        for (uint8_t i = 0; i < opcode_size; i++) {
            uint8_t value = program[index + i];
            if (value < 0x10) length += Serial.print('0');
            length += Serial.print(value, HEX);
            if (i < opcode_size - 1) length += Serial.print(' ');
        }
        length += Serial.print(']');
        return length;
    }
    void printlnOpcodeAt(uint16_t index) {
        printOpcodeAt(index);
        Serial.println();
    }

    // Push a new sequence of bytes to the PLC Program
    RuntimeError push(uint8_t* code, uint16_t code_size) {
        if (program_size + code_size > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(F("Program size exceeded: ")); Serial.println(program_size);
            return status;
        }
        Serial.print(F("Pushing ")); Serial.print(code_size); Serial.println(F(" bytes to the program."));
        memcpy(program + program_size, code, code_size);
        program_size += code_size;
        status = STATUS_SUCCESS;
        return status;
    }

    // Push a new instruction to the PLC Program
    RuntimeError push(uint8_t instruction) {
        if (program_size + 1 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(F("Program size exceeded: ")); Serial.println(program_size);
            return status;
        }
        program[program_size] = instruction;
        program_size++;
        status = STATUS_SUCCESS;
        return status;
    }

    // Push a new instruction to the PLC Program
    RuntimeError push(uint8_t instruction, uint8_t data_type) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(F("Program size exceeded: ")); Serial.println(program_size);
            return status;
        }
        program[program_size] = instruction;
        program[program_size + 1] = data_type;
        program_size += 2;
        status = STATUS_SUCCESS;
        return status;
    }

    // Push a boolean value to the PLC Program
    RuntimeError push_bool(bool value) {
        return push_uint8_t(value ? 1 : 0);
    }

    // Push an uint8_t value to the PLC Program
    RuntimeError push_uint8_t(uint8_t value) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(F("Program size exceeded: ")); Serial.println(program_size);
            return status;
        }
        program[program_size] = type_uint8_t;
        program[program_size + 1] = value;
        program_size += 2;
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an int8_t value to the PLC Program
    RuntimeError push_int8_t(int8_t value) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(F("Program size exceeded: ")); Serial.println(program_size);
            return status;
        }
        program[program_size] = type_int8_t;
        program[program_size + 1] = value;
        program_size += 2;
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an uint16_t value to the PLC Program
    RuntimeError push_uint16_t(uint16_t value) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(F("Program size exceeded: ")); Serial.println(program_size);
            return status;
        }
        program[program_size] = type_uint16_t;
        program[program_size + 1] = value >> 8;
        program[program_size + 2] = value & 0xFF;
        program_size += 3;
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an int16_t value to the PLC Program
    RuntimeError push_int16_t(int16_t value) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(F("Program size exceeded: ")); Serial.println(program_size);
            return status;
        }
        program[program_size] = type_int16_t;
        program[program_size + 1] = value >> 8;
        program[program_size + 2] = value & 0xFF;
        program_size += 3;
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an uint32_t value to the PLC Program
    RuntimeError push_uint32_t(uint32_t value) {
        if (program_size + 5 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(F("Program size exceeded: ")); Serial.println(program_size);
            return status;
        }
        program[program_size] = type_uint32_t;
        program[program_size + 1] = value >> 24;
        program[program_size + 2] = (value >> 16) & 0xFF;
        program[program_size + 3] = (value >> 8) & 0xFF;
        program[program_size + 4] = value & 0xFF;
        program_size += 5;
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an int32_t value to the PLC Program
    RuntimeError push_int32_t(int32_t value) {
        if (program_size + 5 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(F("Program size exceeded: ")); Serial.println(program_size);
            return status;
        }
        program[program_size] = type_int32_t;
        program[program_size + 1] = value >> 24;
        program[program_size + 2] = (value >> 16) & 0xFF;
        program[program_size + 3] = (value >> 8) & 0xFF;
        program[program_size + 4] = value & 0xFF;
        program_size += 5;
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an uint64_t value to the PLC Program
    RuntimeError push_uint64_t(uint64_t value) {
        if (program_size + 9 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(F("Program size exceeded: ")); Serial.println(program_size);
            return status;
        }
        program[program_size] = type_uint64_t;
        program[program_size + 1] = value >> (uint64_t) 56;
        program[program_size + 2] = (value >> (uint64_t) 48) & 0xFF;
        program[program_size + 3] = (value >> (uint64_t) 40) & 0xFF;
        program[program_size + 4] = (value >> (uint64_t) 32) & 0xFF;
        program[program_size + 5] = (value >> (uint64_t) 24) & 0xFF;
        program[program_size + 6] = (value >> (uint64_t) 16) & 0xFF;
        program[program_size + 7] = (value >> (uint64_t) 8) & 0xFF;
        program[program_size + 8] = value & 0xFF;
        program_size += 9;
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an int64_t value to the PLC Program
    RuntimeError push_int64_t(int64_t value) {
        if (program_size + 9 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(F("Program size exceeded: ")); Serial.println(program_size);
            return status;
        }
        program[program_size] = type_int64_t;
        program[program_size + 1] = value >> (int64_t) 56;
        program[program_size + 2] = (value >> (int64_t) 48) & 0xFF;
        program[program_size + 3] = (value >> (int64_t) 40) & 0xFF;
        program[program_size + 4] = (value >> (int64_t) 32) & 0xFF;
        program[program_size + 5] = (value >> (int64_t) 24) & 0xFF;
        program[program_size + 6] = (value >> (int64_t) 16) & 0xFF;
        program[program_size + 7] = (value >> (int64_t) 8) & 0xFF;
        program[program_size + 8] = value & 0xFF;
        program_size += 9;
        status = STATUS_SUCCESS;
        return status;
    }

    // Push a float value to the PLC Program
    RuntimeError push_float(float value) {
        if (program_size + 5 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(F("Program size exceeded: ")); Serial.println(program_size);
            return status;
        }
        program[program_size] = type_float;
        uint32_t* value_ptr = (uint32_t*) &value;
        program[program_size + 1] = *value_ptr >> 24;
        program[program_size + 2] = (*value_ptr >> 16) & 0xFF;
        program[program_size + 3] = (*value_ptr >> 8) & 0xFF;
        program[program_size + 4] = *value_ptr & 0xFF;
        program_size += 5;
        status = STATUS_SUCCESS;
        return status;
    }

    // Push a double value to the PLC Program
    RuntimeError push_double(double value) {
        if (program_size + 9 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(F("Program size exceeded: ")); Serial.println(program_size);
            return status;
        }
        program[program_size] = type_double;
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
        status = STATUS_SUCCESS;
        return status;
    }

    // Push flow control instructions to the PLC Program
    RuntimeError pushJMP(uint16_t program_address) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(F("Program size exceeded: ")); Serial.println(program_size);
            return status;
        }
        program[program_size] = JMP;
        program[program_size + 1] = program_address >> 8;
        program[program_size + 2] = program_address & 0xFF;
        program_size += 3;
        status = STATUS_SUCCESS;
        return status;
    }
    RuntimeError pushJMP_IF(uint16_t program_address) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(F("Program size exceeded: ")); Serial.println(program_size);
            return status;
        }
        program[program_size] = JMP_IF;
        program[program_size + 1] = program_address >> 8;
        program[program_size + 2] = program_address & 0xFF;
        program_size += 3;
        status = STATUS_SUCCESS;
        return status;
    }
    RuntimeError pushJMP_IF_NOT(uint16_t program_address) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(F("Program size exceeded: ")); Serial.println(program_size);
            return status;
        }
        program[program_size] = JMP_IF_NOT;
        program[program_size + 1] = program_address >> 8;
        program[program_size + 2] = program_address & 0xFF;
        program_size += 3;
        status = STATUS_SUCCESS;
        return status;
    }

    // Pushes the byte at the given address of the memory into the stack
    RuntimeError pushGET(uint16_t program_address, PLCRuntimeInstructionSet type = type_uint8_t) {
        if (program_size + 4 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(F("Program size exceeded: ")); Serial.println(program_size);
            return status;
        }
        program[program_size] = GET;
        program[program_size + 1] = (uint8_t) type;
        program[program_size + 2] = program_address >> 8;
        program[program_size + 3] = program_address & 0xFF;
        program_size += 4;
        status = STATUS_SUCCESS;
        return status;
    }
    // Stores the byte at the top of the stack into the given address in memory and pops the stack
    RuntimeError pushPUT(uint16_t program_address, PLCRuntimeInstructionSet type = type_uint8_t) {
        if (program_size + 4 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(F("Program size exceeded: ")); Serial.println(program_size);
            return status;
        }
        program[program_size] = PUT;
        program[program_size + 1] = (uint8_t) type;
        program[program_size + 2] = program_address >> 8;
        program[program_size + 3] = program_address & 0xFF;
        program_size += 4;
        status = STATUS_SUCCESS;
        return status;
    }
};