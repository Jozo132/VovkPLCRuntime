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

char foo;

class InstructionCompiler {
public:
    // Push a new sequence of bytes to the PLC Program
    static uint32_t push(uint8_t* location, uint8_t* code, uint32_t code_size) {
        memcpy(location, code, code_size);
        return code_size;
    }

    // Push a new instruction to the PLC Program
    static uint8_t push(uint8_t* location, uint8_t instruction) {
        location[0] = instruction;
        return 1;
    }

    // Push a new instruction to the PLC Program
    static uint8_t push(uint8_t* location, uint8_t instruction, uint8_t data_type) {
        location[0] = instruction;
        location[1] = data_type;
        return 2;
    }

    // Push a boolean value to the PLC Program
    static uint8_t push_bool(uint8_t* location, bool value) {
        return push_uint8_t(location, value ? 1 : 0);
    }

    // Push an uint8_t value to the PLC Program
    static uint8_t push_uint8_t(uint8_t* location, uint8_t value) {
        location[0] = type_uint8_t;
        location[1] = value;
        return 2;
    }

    // Push an int8_t value to the PLC Program
    static uint8_t push_int8_t(uint8_t* location, int8_t value) {
        location[0] = type_int8_t;
        location[1] = value;
        return 2;
    }

    // Push an uint16_t value to the PLC Program
    static uint8_t push_uint16_t(uint8_t* location, uint32_t value) {
        location[0] = type_uint16_t;
        location[1] = value >> 8;
        location[2] = value & 0xFF;
        return 3;
    }

    // Push an int16_t value to the PLC Program
    static uint8_t push_int16_t(uint8_t* location, int16_t value) {
        location[0] = type_int16_t;
        location[1] = value >> 8;
        location[2] = value & 0xFF;
        return 3;
    }

    // Push an uint32_t value to the PLC Program
    static uint8_t push_uint32_t(uint8_t* location, uint32_t value) {
        location[0] = type_uint32_t;
        location[1] = value >> 24;
        location[2] = (value >> 16) & 0xFF;
        location[3] = (value >> 8) & 0xFF;
        location[4] = value & 0xFF;
        return 5;
    }

    // Push an int32_t value to the PLC Program
    static uint8_t push_int32_t(uint8_t* location, int32_t value) {
        location[0] = type_int32_t;
        location[1] = value >> 24;
        location[2] = (value >> 16) & 0xFF;
        location[3] = (value >> 8) & 0xFF;
        location[4] = value & 0xFF;
        return 5;
    }

    // Push an uint64_t value to the PLC Program
    static uint8_t push_uint64_t(uint8_t* location, uint64_t value) {
        location[0] = type_uint64_t;
        location[1] = value >> (uint64_t) 56;
        location[2] = (value >> (uint64_t) 48) & 0xFF;
        location[3] = (value >> (uint64_t) 40) & 0xFF;
        location[4] = (value >> (uint64_t) 32) & 0xFF;
        location[5] = (value >> (uint64_t) 24) & 0xFF;
        location[6] = (value >> (uint64_t) 16) & 0xFF;
        location[7] = (value >> (uint64_t) 8) & 0xFF;
        location[8] = value & 0xFF;
        return 9;
    }

    // Push an int64_t value to the PLC Program
    static uint8_t push_int64_t(uint8_t* location, int64_t value) {
        location[0] = type_int64_t;
        location[1] = value >> (int64_t) 56;
        location[2] = (value >> (int64_t) 48) & 0xFF;
        location[3] = (value >> (int64_t) 40) & 0xFF;
        location[4] = (value >> (int64_t) 32) & 0xFF;
        location[5] = (value >> (int64_t) 24) & 0xFF;
        location[6] = (value >> (int64_t) 16) & 0xFF;
        location[7] = (value >> (int64_t) 8) & 0xFF;
        location[8] = value & 0xFF;
        return 9;
    }

    // Push a float value to the PLC Program
    static uint8_t push_float(uint8_t* location, float value) {
        location[0] = type_float;
        uint32_t* value_ptr = (uint32_t*) &value;
        location[1] = *value_ptr >> 24;
        location[2] = (*value_ptr >> 16) & 0xFF;
        location[3] = (*value_ptr >> 8) & 0xFF;
        location[4] = *value_ptr & 0xFF;
        return 5;
    }

    // Push a double value to the PLC Program
    static uint8_t push_double(uint8_t* location, double value) {
        location[0] = type_double;
        uint64_t* value_ptr = (uint64_t*) &value;
        location[1] = *value_ptr >> 56;
        location[2] = (*value_ptr >> 48) & 0xFF;
        location[3] = (*value_ptr >> 40) & 0xFF;
        location[4] = (*value_ptr >> 32) & 0xFF;
        location[5] = (*value_ptr >> 24) & 0xFF;
        location[6] = (*value_ptr >> 16) & 0xFF;
        location[7] = (*value_ptr >> 8) & 0xFF;
        location[8] = *value_ptr & 0xFF;
        return 9;
    }

    // Push instruction + uint16_t value to the PLC Program
    static uint8_t push_InstructionWithU32(uint8_t* location, PLCRuntimeInstructionSet instruction, uint16_t value) {
        location[0] = instruction;
        location[1] = value >> 8;
        location[2] = value & 0xFF;
        return 3;
    }

    // Push flow control instructions to the PLC Program
    static uint8_t pushJMP(uint8_t* location, uint32_t location_address) {
        location[0] = JMP;
        location[1] = location_address >> 8;
        location[2] = location_address & 0xFF;
        return 3;
    }
    static uint8_t pushJMP_IF(uint8_t* location, uint32_t location_address) {
        location[0] = JMP_IF;
        location[1] = location_address >> 8;
        location[2] = location_address & 0xFF;
        return 3;
    }
    static uint8_t pushJMP_IF_NOT(uint8_t* location, uint32_t location_address) {
        location[0] = JMP_IF_NOT;
        location[1] = location_address >> 8;
        location[2] = location_address & 0xFF;
        return 3;
    }

    static uint8_t pushCALL(uint8_t* location, uint32_t location_address) {
        location[0] = CALL;
        location[1] = location_address >> 8;
        location[2] = location_address & 0xFF;
        return 3;
    }
    static uint8_t pushCALL_IF(uint8_t* location, uint32_t location_address) {
        location[0] = CALL_IF;
        location[1] = location_address >> 8;
        location[2] = location_address & 0xFF;
        return 3;
    }
    static uint8_t pushCALL_IF_NOT(uint8_t* location, uint32_t location_address) {
        location[0] = CALL_IF_NOT;
        location[1] = location_address >> 8;
        location[2] = location_address & 0xFF;
        return 3;
    }


    // Convert a data type to another data type
    static uint8_t pushCVT(uint8_t* location, PLCRuntimeInstructionSet from, PLCRuntimeInstructionSet to) {
        location[0] = CVT;
        location[1] = (uint8_t) from;
        location[2] = (uint8_t) to;
        return 3;
    }

    // Pushes the byte at the given address of the memory into the stack
    static uint8_t pushGET(uint8_t* location, uint32_t location_address, PLCRuntimeInstructionSet type = type_uint8_t) {
        location[0] = GET;
        location[1] = (uint8_t) type;
        location[2] = location_address >> 8;
        location[3] = location_address & 0xFF;
        return 4;
    }
    // Stores the byte at the top of the stack into the given address in memory and pops the stack
    static uint8_t pushPUT(uint8_t* location, uint32_t location_address, PLCRuntimeInstructionSet type = type_uint8_t) {
        location[0] = PUT;
        location[1] = (uint8_t) type;
        location[2] = location_address >> 8;
        location[3] = location_address & 0xFF;
        return 4;
    }

    // Make a duplica of the top of the stack
    static uint8_t pushCOPY(uint8_t* location, PLCRuntimeInstructionSet type = type_uint8_t) {
        location[0] = COPY;
        location[1] = type;
        return 2;
    }
    // Swap the top two values on the stack
    static uint8_t pushSWAP(uint8_t* location, PLCRuntimeInstructionSet type = type_uint8_t) {
        location[0] = SWAP;
        location[1] = type;
        return 2;
    }
    // Drop the top value from the stack
    static uint8_t pushDROP(uint8_t* location, PLCRuntimeInstructionSet type = type_uint8_t) {
        location[0] = DROP;
        location[1] = type;
        return 2;
    }
};

class RuntimeProgram {
private:
    uint32_t MAX_PROGRAM_SIZE = PLCRUNTIME_DEFAULT_PROGRAM_SIZE; // Max program size in bytes
public:
#ifdef __WASM__
    uint8_t* program = nullptr; // PLC program to execute
#else
    uint8_t* program = new uint8_t[PLCRUNTIME_DEFAULT_PROGRAM_SIZE]; // PLC program to execute
#endif // __WASM__
    uint32_t program_size = 0; // Current program size in bytes
    uint32_t program_line = 0; // Active program line
    RuntimeError status = UNDEFINED_STATE;

    RuntimeProgram(uint32_t program_size) {
        if (program_size > PLCRUNTIME_DEFAULT_PROGRAM_SIZE) {
            if (program != nullptr) delete [] program;
            program = new uint8_t[program_size];
            this->MAX_PROGRAM_SIZE = program_size;
        }
    }
    RuntimeProgram() { }
    ~RuntimeProgram() {
        if (program != nullptr) delete [] program;
    }

    void begin(const uint8_t* program, uint32_t program_size, uint8_t checksum) {
        format(program_size);
        load(program, program_size, checksum);
    }

    void beginUnsafe(const uint8_t* program, uint32_t program_size) {
        format(program_size);
        loadUnsafe(program, program_size);
    }

    void begin(uint32_t program_size = 0) {
        format(program_size);
    }

    void format(uint32_t program_size = 0) {
        if (program_size == 0) program_size = MAX_PROGRAM_SIZE;
        if (program_size > MAX_PROGRAM_SIZE) program_size = MAX_PROGRAM_SIZE;
        if (this->program == nullptr) this->program = new uint8_t[MAX_PROGRAM_SIZE];
        this->program_size = 0;
        this->program_line = 0;
        this->status = UNDEFINED_STATE;
    }

    RuntimeError loadUnsafe(const uint8_t* program, uint32_t program_size) {
        if (program_size > MAX_PROGRAM_SIZE) status = PROGRAM_SIZE_EXCEEDED;
        else if (program_size == 0) status = UNDEFINED_STATE;
        else status = STATUS_SUCCESS;
        format(program_size);
        memcpy(this->program, program, program_size);
        this->program_size = program_size;
        return status;
    }

    RuntimeError load(const uint8_t* program, uint32_t program_size, uint8_t checksum) {
        uint8_t calculated_checksum = 0;
        crc8_simple(calculated_checksum, program, program_size);
        if (calculated_checksum != checksum) {
            status = INVALID_CHECKSUM;
            Serial.println(F("Failed to load program: CHECKSUM MISMATCH"));
            return status;
        }
        return loadUnsafe(program, program_size);
    }

    // Get the size of used program memory
    uint32_t size() { return program_size; }

    // Hot update the running program. This is a very dangerous operation, so use it with caution!
    RuntimeError modify(uint32_t index, uint8_t value) {
        if (index >= program_size) return INVALID_PROGRAM_INDEX;
        program[index] = value;
        return STATUS_SUCCESS;
    }

    // Hot update the running program. This is a very dangerous operation, so use it with caution!
    RuntimeError modify(uint32_t index, uint8_t* data, uint32_t size) {
        if (index + size > program_size) return INVALID_PROGRAM_INDEX;
        for (uint32_t i = 0; i < size; i++) program[index + i] = data[i];
        return STATUS_SUCCESS;
    }

    RuntimeError modifyValue(uint32_t index, uint32_t value) {
        if (index + sizeof(uint32_t) > program_size) return INVALID_PROGRAM_INDEX;
        program[index] = value >> 8;
        program[index + 1] = value & 0xFF;
        return STATUS_SUCCESS;
    }

    // Set the active PLC Program line number
    RuntimeError setLine(uint32_t line_number) {
        if (line_number >= MAX_PROGRAM_SIZE) return INVALID_PROGRAM_INDEX;
        program_line = line_number;
        return STATUS_SUCCESS;
    }

    // Get the active PLC Program line number
    uint32_t getLine() { return program_line; }

    // Reset the active PLC Program line number to the beginning
    void resetLine() { program_line = 0; }

    bool finished() { return program_line >= program_size; }

    uint32_t getProgramSize() { return program_size; }

    int print() {
        int length = Serial.print(F("Program["));
        length += Serial.print(program_size);
        if (program_size == 0) {
            length += Serial.print(F("] []"));
            return length;
        }
        length += Serial.print(F("] ["));
        for (uint32_t i = 0; i < program_size; i++) {
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
        uint32_t index = 0;
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
            while (length < 16) length += Serial.print('-');
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

    int printOpcodeAt(uint32_t index) {
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
        while (length < 27) length += Serial.print('-');
        while (length < 29) length += Serial.print(' ');
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
    void printlnOpcodeAt(uint32_t index) {
        printOpcodeAt(index);
        Serial.println();
    }

    // Push a new sequence of bytes to the PLC Program
    RuntimeError push(uint8_t* code, uint32_t code_size) {
        if (program_size + code_size > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push(program + program_size, code, code_size);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push a new instruction to the PLC Program
    RuntimeError push(uint8_t instruction) {
        if (program_size + 1 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push(program + program_size, instruction);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push a new instruction to the PLC Program
    RuntimeError push(uint8_t instruction, uint8_t data_type) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push(program + program_size, instruction, data_type);
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
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_uint8_t(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an int8_t value to the PLC Program
    RuntimeError push_int8_t(int8_t value) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_int8_t(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an uint16_t value to the PLC Program
    RuntimeError push_uint16_t(uint32_t value) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_uint16_t(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an int16_t value to the PLC Program
    RuntimeError push_int16_t(int16_t value) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_int16_t(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an uint32_t value to the PLC Program
    RuntimeError push_uint32_t(uint32_t value) {
        if (program_size + 5 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_uint32_t(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an int32_t value to the PLC Program
    RuntimeError push_int32_t(int32_t value) {
        if (program_size + 5 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_int32_t(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an uint64_t value to the PLC Program
    RuntimeError push_uint64_t(uint64_t value) {
        if (program_size + 9 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_uint64_t(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an int64_t value to the PLC Program
    RuntimeError push_int64_t(int64_t value) {
        if (program_size + 9 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_int64_t(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push a float value to the PLC Program
    RuntimeError push_float(float value) {
        if (program_size + 5 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_float(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push a double value to the PLC Program
    RuntimeError push_double(double value) {
        if (program_size + 9 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_double(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push flow control instructions to the PLC Program
    RuntimeError pushJMP(uint32_t program_address) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::pushJMP(program + program_size, program_address);
        status = STATUS_SUCCESS;
        return status;
    }
    RuntimeError pushJMP_IF(uint32_t program_address) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::pushJMP_IF(program + program_size, program_address);
        status = STATUS_SUCCESS;
        return status;
    }
    RuntimeError pushJMP_IF_NOT(uint32_t program_address) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::pushJMP_IF_NOT(program + program_size, program_address);
        status = STATUS_SUCCESS;
        return status;
    }

    // Convert a data type to another data type
    RuntimeError pushCVT(PLCRuntimeInstructionSet from, PLCRuntimeInstructionSet to) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        program_size += InstructionCompiler::pushCVT(program + program_size, from, to);
        status = STATUS_SUCCESS;
        return status;
    }

    // Pushes the byte at the given address of the memory into the stack
    RuntimeError pushGET(uint32_t program_address, PLCRuntimeInstructionSet type = type_uint8_t) {
        if (program_size + 4 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::pushGET(program + program_size, program_address, type);
        status = STATUS_SUCCESS;
        return status;
    }
    // Stores the byte at the top of the stack into the given address in memory and pops the stack
    RuntimeError pushPUT(uint32_t program_address, PLCRuntimeInstructionSet type = type_uint8_t) {
        if (program_size + 4 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::pushPUT(program + program_size, program_address, type);
        status = STATUS_SUCCESS;
        return status;
    }

    // Make a duplica of the top of the stack
    RuntimeError pushCOPY(PLCRuntimeInstructionSet type = type_uint8_t) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        program_size += InstructionCompiler::pushCOPY(program + program_size, type);
        status = STATUS_SUCCESS;
        return status;
    }
    // Swap the top two values on the stack
    RuntimeError pushSWAP(PLCRuntimeInstructionSet type = type_uint8_t) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        program_size += InstructionCompiler::pushSWAP(program + program_size, type);
        status = STATUS_SUCCESS;
        return status;
    }
    // Drop the top value from the stack
    RuntimeError pushDROP(PLCRuntimeInstructionSet type = type_uint8_t) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        program_size += InstructionCompiler::pushDROP(program + program_size, type);
        status = STATUS_SUCCESS;
        return status;
    }
};