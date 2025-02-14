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

class InstructionCompiler {
public:
    // Push a new sequence of bytes to the PLC Program
    static u32 push(u8* location, u8* code, u32 code_size) {
        memcpy(location, code, code_size);
        return code_size;
    }

    // Push a new instruction to the PLC Program
    static u8 push(u8* location, u8 instruction) {
        location[0] = instruction;
        return 1;
    }

    // Push a new instruction to the PLC Program
    static u8 push(u8* location, u8 instruction, u8 data_type) {
        location[0] = instruction;
        location[1] = data_type;
        return 2;
    }

    static u8 push_pointer(u8* location, MY_PTR_t pointer) {
        location[0] = type_pointer;
        // memcpy(location + 1, &pointer, sizeof(MY_PTR_t)); // Wrong endianess
        for (u8 i = 0; i < sizeof(MY_PTR_t); i++) location[i + 1] = (pointer >> ((sizeof(MY_PTR_t) - i - 1) * 8)) & 0xFF;
        return 1 + sizeof(MY_PTR_t);        
    }

    // Push a boolean value to the PLC Program
    static u8 push_bool(u8* location, bool value) {
        return push_u8(location, value ? 1 : 0);
    }

    // Push an u8 value to the PLC Program
    static u8 push_u8(u8* location, u8 value) {
        location[0] = type_u8;
        location[1] = value;
        return 2;
    }

    // Push an i8 value to the PLC Program
    static u8 push_i8(u8* location, i8 value) {
        location[0] = type_i8;
        location[1] = value;
        return 2;
    }

    // Push an u16 value to the PLC Program
    static u8 push_u16(u8* location, u32 value) {
        location[0] = type_u16;
        location[1] = value >> 8;
        location[2] = value & 0xFF;
        return 3;
    }

    // Push an i16 value to the PLC Program
    static u8 push_i16(u8* location, i16 value) {
        location[0] = type_i16;
        location[1] = value >> 8;
        location[2] = value & 0xFF;
        return 3;
    }

    // Push an u32 value to the PLC Program
    static u8 push_u32(u8* location, u32 value) {
        location[0] = type_u32;
        location[1] = value >> 24;
        location[2] = (value >> 16) & 0xFF;
        location[3] = (value >> 8) & 0xFF;
        location[4] = value & 0xFF;
        return 5;
    }

    // Push an i32 value to the PLC Program
    static u8 push_i32(u8* location, i32 value) {
        location[0] = type_i32;
        location[1] = value >> 24;
        location[2] = (value >> 16) & 0xFF;
        location[3] = (value >> 8) & 0xFF;
        location[4] = value & 0xFF;
        return 5;
    }

#ifdef USE_X64_OPS
    // Push an u64 value to the PLC Program
    static u8 push_u64(u8* location, u64 value) {
        location[0] = type_u64;
        location[1] = value >> (u64) 56;
        location[2] = (value >> (u64) 48) & 0xFF;
        location[3] = (value >> (u64) 40) & 0xFF;
        location[4] = (value >> (u64) 32) & 0xFF;
        location[5] = (value >> (u64) 24) & 0xFF;
        location[6] = (value >> (u64) 16) & 0xFF;
        location[7] = (value >> (u64) 8) & 0xFF;
        location[8] = value & 0xFF;
        return 9;
    }

    // Push an i64 value to the PLC Program
    static u8 push_i64(u8* location, i64 value) {
        location[0] = type_i64;
        location[1] = value >> (i64) 56;
        location[2] = (value >> (i64) 48) & 0xFF;
        location[3] = (value >> (i64) 40) & 0xFF;
        location[4] = (value >> (i64) 32) & 0xFF;
        location[5] = (value >> (i64) 24) & 0xFF;
        location[6] = (value >> (i64) 16) & 0xFF;
        location[7] = (value >> (i64) 8) & 0xFF;
        location[8] = value & 0xFF;
        return 9;
    }
#endif // USE_X64_OPS

    // Push a float value to the PLC Program
    static u8 push_f32(u8* location, float value) {
        location[0] = type_f32;
        u32* value_ptr = (u32*) &value;
        location[1] = *value_ptr >> 24;
        location[2] = (*value_ptr >> 16) & 0xFF;
        location[3] = (*value_ptr >> 8) & 0xFF;
        location[4] = *value_ptr & 0xFF;
        return 5;
    }

#ifdef USE_X64_OPS
    // Push a double value to the PLC Program
    static u8 push_f64(u8* location, double value) {
        location[0] = type_f64;
        u64* value_ptr = (u64*) &value;
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
#endif // USE_X64_OPS

    // Push instruction + u16 value to the PLC Program
    static u8 push_InstructionWithU32(u8* location, PLCRuntimeInstructionSet instruction, u16 value) {
        location[0] = instruction;
        location[1] = value >> 8;
        location[2] = value & 0xFF;
        return 3;
    }

    // Push flow control instructions to the PLC Program
    static u8 push_jmp(u8* location, u32 location_address) {
        location[0] = JMP;
        location[1] = location_address >> 8;
        location[2] = location_address & 0xFF;
        return 3;
    }
    static u8 push_jmp_if(u8* location, u32 location_address) {
        location[0] = JMP_IF;
        location[1] = location_address >> 8;
        location[2] = location_address & 0xFF;
        return 3;
    }
    static u8 push_jmp_if_not(u8* location, u32 location_address) {
        location[0] = JMP_IF_NOT;
        location[1] = location_address >> 8;
        location[2] = location_address & 0xFF;
        return 3;
    }

    static u8 pushCALL(u8* location, u32 location_address) {
        location[0] = CALL;
        location[1] = location_address >> 8;
        location[2] = location_address & 0xFF;
        return 3;
    }
    static u8 pushCALL_IF(u8* location, u32 location_address) {
        location[0] = CALL_IF;
        location[1] = location_address >> 8;
        location[2] = location_address & 0xFF;
        return 3;
    }
    static u8 pushCALL_IF_NOT(u8* location, u32 location_address) {
        location[0] = CALL_IF_NOT;
        location[1] = location_address >> 8;
        location[2] = location_address & 0xFF;
        return 3;
    }


    // Convert a data type to another data type
    static u8 push_cvt(u8* location, PLCRuntimeInstructionSet from, PLCRuntimeInstructionSet to) {
        location[0] = CVT;
        location[1] = (u8) from;
        location[2] = (u8) to;
        return 3;
    }

    // Pop the pointer from the stack and load the value type from the pointer address
    static u8 push_load(u8* location, PLCRuntimeInstructionSet type = type_u8) {
        location[0] = LOAD;
        location[1] = type;
        return 2;
    }

    // Pop(1) the value from the stack and pop(2) the pointer from the stack and set the memory at the pointer address to the value
    static u8 push_move(u8* location, PLCRuntimeInstructionSet type = type_u8) {
        location[0] = MOVE;
        location[1] = type;
        return 2;
    }

    // Pop(1) the value from the stack, pop(2) the pointer from the stack, push(3) the value back into the stack and set the memory at the pointer address to the value
    static u8 push_move_copy(u8* location, PLCRuntimeInstructionSet type = type_u8) {
        location[0] = MOVE_COPY;
        location[1] = type;
        return 2;
    }

    // Make a duplica of the top of the stack
    static u8 push_copy(u8* location, PLCRuntimeInstructionSet type = type_u8) {
        location[0] = COPY;
        location[1] = type;
        return 2;
    }
    // Swap the top two values on the stack
    static u8 push_swap(u8* location, PLCRuntimeInstructionSet type_A = type_u8, PLCRuntimeInstructionSet type_B = type_u8) {
        location[0] = SWAP;
        location[1] = type_A;
        location[2] = type_B;
        return 3;
    }
    // Drop the top value from the stack
    static u8 push_drop(u8* location, PLCRuntimeInstructionSet type = type_u8) {
        location[0] = DROP;
        location[1] = type;
        return 2;
    }
};

class RuntimeProgram {
private:
    u32 MAX_PROGRAM_SIZE = PLCRUNTIME_DEFAULT_PROGRAM_SIZE; // Max program size in bytes
public:
#ifdef __WASM__
    u8* program = nullptr; // PLC program to execute
#else
    u8* program = new u8[PLCRUNTIME_DEFAULT_PROGRAM_SIZE]; // PLC program to execute
#endif // __WASM__
    u32 program_size = 0; // Current program size in bytes
    u32 program_line = 0; // Active program line
    RuntimeError status = UNDEFINED_STATE;

    RuntimeProgram(u32 program_size) {
        if (program_size > PLCRUNTIME_DEFAULT_PROGRAM_SIZE) {
            if (program != nullptr) delete [] program;
            program = new u8[program_size];
            this->MAX_PROGRAM_SIZE = program_size;
        }
    }
    RuntimeProgram() { }
    ~RuntimeProgram() {
        if (program != nullptr) delete [] program;
    }

    void begin(const u8* program, u32 program_size, u8 checksum) {
        format(program_size);
        load(program, program_size, checksum);
    }

    void beginUnsafe(const u8* program, u32 program_size) {
        format(program_size);
        loadUnsafe(program, program_size);
    }

    void begin(u32 program_size = 0) {
        format(program_size);
    }

    void format(u32 program_size = 0) {
        if (program_size == 0) program_size = MAX_PROGRAM_SIZE;
        if (program_size > MAX_PROGRAM_SIZE) program_size = MAX_PROGRAM_SIZE;
        if (this->program == nullptr) this->program = new u8[MAX_PROGRAM_SIZE];
        this->program_size = 0;
        this->program_line = 0;
        this->status = UNDEFINED_STATE;
    }

    RuntimeError loadUnsafe(const u8* program, u32 program_size) {
        if (program_size > MAX_PROGRAM_SIZE) status = PROGRAM_SIZE_EXCEEDED;
        else if (program_size == 0) status = UNDEFINED_STATE;
        else status = STATUS_SUCCESS;
        format(program_size);
        memcpy(this->program, program, program_size);
        this->program_size = program_size;
        return status;
    }

    RuntimeError load(const u8* program, u32 program_size, u8 checksum) {
        u8 calculated_checksum = 0;
        crc8_simple(calculated_checksum, program, program_size);
        if (calculated_checksum != checksum) {
            status = INVALID_CHECKSUM;
            Serial.println(F("Failed to load program: CHECKSUM MISMATCH"));
            return status;
        }
        return loadUnsafe(program, program_size);
    }

    // Get the size of used program memory
    u32 size() { return program_size; }

    // Hot update the running program. This is a very dangerous operation, so use it with caution!
    RuntimeError modify(u32 index, u8 value) {
        if (index >= program_size) return INVALID_PROGRAM_INDEX;
        program[index] = value;
        return STATUS_SUCCESS;
    }

    // Hot update the running program. This is a very dangerous operation, so use it with caution!
    RuntimeError modify(u32 index, u8* data, u32 size) {
        if (index + size > program_size) return INVALID_PROGRAM_INDEX;
        for (u32 i = 0; i < size; i++) program[index + i] = data[i];
        return STATUS_SUCCESS;
    }

    RuntimeError modifyValue(u32 index, u32 value) {
        if (index + sizeof(u32) > program_size) return INVALID_PROGRAM_INDEX;
        program[index] = value >> 8;
        program[index + 1] = value & 0xFF;
        return STATUS_SUCCESS;
    }

    // Set the active PLC Program line number
    RuntimeError setLine(u32 line_number) {
        if (line_number >= MAX_PROGRAM_SIZE) return INVALID_PROGRAM_INDEX;
        program_line = line_number;
        return STATUS_SUCCESS;
    }

    // Get the active PLC Program line number
    u32 getLine() { return program_line; }

    // Reset the active PLC Program line number to the beginning
    void resetLine() { program_line = 0; }

    bool finished() { return program_line >= program_size; }

    u32 getProgramSize() { return program_size; }

    int print() {
        int length = Serial.print(F("Program["));
        length += Serial.print(program_size);
        if (program_size == 0) {
            length += Serial.print(F("] []"));
            return length;
        }
        length += Serial.print(F("] ["));
        for (u32 i = 0; i < program_size; i++) {
            u8 value = program[i];
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
        u32 index = 0;
        bool done = false;
        while (!done) {
            // Get current opcode
            PLCRuntimeInstructionSet opcode = (PLCRuntimeInstructionSet) program[index];
            // Serial.printf("    %4d [%02X %02X] - %02X: ", index, index >> 8, index & 0xFF, opcode);
            Serial.print(F("    "));
            print_number_padStart(index, 4);
            Serial.print(F(" ["));
            u8 hi = index >> 8;
            u8 lo = index & 0xFF;
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
            u8 instruction_size = OPCODE_SIZE(opcode);
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
            for (u8 i = 0; i < instruction_size; i++) {
                if ((i + index) >= program_size) {
                    Serial.println(F(" - OUT OF PROGRAM BOUNDS\n"));
                    return;
                }
                u8 value = program[i + index];
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

    u8 sizeOfCurrentBytecode() { return OPCODE_SIZE(getCurrentBytecode()); }

    int printOpcodeAt(u32 index) {
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
        u8 opcode_size = OPCODE_SIZE(opcode);
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
        for (u8 i = 0; i < opcode_size; i++) {
            u8 value = program[index + i];
            if (value < 0x10) length += Serial.print('0');
            length += Serial.print(value, HEX);
            if (i < opcode_size - 1) length += Serial.print(' ');
        }
        length += Serial.print(']');
        return length;
    }
    void printlnOpcodeAt(u32 index) {
        printOpcodeAt(index);
        Serial.println();
    }

    // Push a new sequence of bytes to the PLC Program
    RuntimeError push(u8* code, u32 code_size) {
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
    RuntimeError push(u8 instruction) {
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
    RuntimeError push(u8 instruction, u8 data_type) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push(program + program_size, instruction, data_type);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push a pointer to the PLC Program
    RuntimeError push_pointer(MY_PTR_t pointer) {
        if (program_size + sizeof(MY_PTR_t) > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        program_size += InstructionCompiler::push_pointer(program + program_size, pointer);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push a boolean value to the PLC Program
    RuntimeError push_bool(bool value) {
        return push_u8(value ? 1 : 0);
    }

    // Push an u8 value to the PLC Program
    RuntimeError push_u8(u8 value) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_u8(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an i8 value to the PLC Program
    RuntimeError push_i8(i8 value) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_i8(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an u16 value to the PLC Program
    RuntimeError push_u16(u32 value) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_u16(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an i16 value to the PLC Program
    RuntimeError push_i16(i16 value) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_i16(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an u32 value to the PLC Program
    RuntimeError push_u32(u32 value) {
        if (program_size + 5 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_u32(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an i32 value to the PLC Program
    RuntimeError push_i32(i32 value) {
        if (program_size + 5 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_i32(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

#ifdef USE_X64_OPS
    // Push an u64 value to the PLC Program
    RuntimeError push_u64(u64 value) {
        if (program_size + 9 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_u64(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an i64 value to the PLC Program
    RuntimeError push_i64(i64 value) {
        if (program_size + 9 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_i64(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }
#endif // USE_X64_OPS

    // Push a float value to the PLC Program
    RuntimeError push_f32(float value) {
        if (program_size + 5 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_f32(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

#ifdef USE_X64_OPS
    // Push a double value to the PLC Program
    RuntimeError push_f64(double value) {
        if (program_size + 9 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_f64(program + program_size, value);
        status = STATUS_SUCCESS;
        return status;
    }
#endif // USE_X64_OPS

    // Push flow control instructions to the PLC Program
    RuntimeError push_jmp(u32 program_address) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_jmp(program + program_size, program_address);
        status = STATUS_SUCCESS;
        return status;
    }
    RuntimeError push_jmp_if(u32 program_address) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_jmp_if(program + program_size, program_address);
        status = STATUS_SUCCESS;
        return status;
    }
    RuntimeError push_jmp_if_not(u32 program_address) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(program_size);
            return status;
        }
        program_size += InstructionCompiler::push_jmp_if_not(program + program_size, program_address);
        status = STATUS_SUCCESS;
        return status;
    }

    // Convert a data type to another data type
    RuntimeError push_cvt(PLCRuntimeInstructionSet from, PLCRuntimeInstructionSet to) {
        if (program_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        program_size += InstructionCompiler::push_cvt(program + program_size, from, to);
        status = STATUS_SUCCESS;
        return status;
    }

    RuntimeError push_load(PLCRuntimeInstructionSet type = type_u8) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        program_size += InstructionCompiler::push_load(program + program_size, type);
        status = STATUS_SUCCESS;
        return status;
    }

    RuntimeError push_move(PLCRuntimeInstructionSet type = type_u8) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        program_size += InstructionCompiler::push_move(program + program_size, type);
        status = STATUS_SUCCESS;
        return status;
    }

    RuntimeError push_move_copy(PLCRuntimeInstructionSet type = type_u8) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        program_size += InstructionCompiler::push_move_copy(program + program_size, type);
        status = STATUS_SUCCESS;
        return status;
    }

    // Make a duplica of the top of the stack
    RuntimeError push_copy(PLCRuntimeInstructionSet type = type_u8) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        program_size += InstructionCompiler::push_copy(program + program_size, type);
        status = STATUS_SUCCESS;
        return status;
    }
    // Swap the top two values on the stack
    RuntimeError push_swap(PLCRuntimeInstructionSet type = type_u8) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        program_size += InstructionCompiler::push_swap(program + program_size, type);
        status = STATUS_SUCCESS;
        return status;
    }
    // Drop the top value from the stack
    RuntimeError push_drop(PLCRuntimeInstructionSet type = type_u8) {
        if (program_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        program_size += InstructionCompiler::push_drop(program + program_size, type);
        status = STATUS_SUCCESS;
        return status;
    }
};