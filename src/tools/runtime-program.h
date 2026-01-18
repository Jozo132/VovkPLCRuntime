// runtime-program.h - 2022-12-11
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

    // Push instruction + u8 value to the PLC Program
    static u8 push_InstructionWithU8(u8* location, PLCRuntimeInstructionSet instruction, u8 value) {
        location[0] = instruction;
        location[1] = value;
        return 2;
    }
    // Push instruction + u16 value to the PLC Program
    static u8 push_InstructionWithU16(u8* location, PLCRuntimeInstructionSet instruction, u16 value) {
        location[0] = instruction;
        location[1] = value >> 8;
        location[2] = value & 0xFF;
        return 3;
    }
    // Push instruction + u32 value to the PLC Program
    static u8 push_InstructionWithU32(u8* location, PLCRuntimeInstructionSet instruction, u32 value) {
        location[0] = instruction;
        location[1] = value >> 24;
        location[2] = (value >> 16) & 0xFF;
        location[3] = (value >> 8) & 0xFF;
        location[4] = value & 0xFF;
        return 5;
    }

    // check sizeof MY_PTR_t to decide which instruction size is needed
#if MY_PTR_SIZE == 8
    static u8 push_InstructionWithPointer(u8* location, PLCRuntimeInstructionSet instruction, MY_PTR_t value) {
        return push_InstructionWithU8(location, instruction, value);
    }
#elif MY_PTR_SIZE == 32
    static u8 push_InstructionWithPointer(u8* location, PLCRuntimeInstructionSet instruction, MY_PTR_t value) {
        return push_InstructionWithU32(location, instruction, value);
    }
#else // Default to 16 bit pointers
    static u8 push_InstructionWithPointer(u8* location, PLCRuntimeInstructionSet instruction, MY_PTR_t value) {
        return push_InstructionWithU16(location, instruction, value);
    }
#endif

    static u8 push_InstructionWithPointer(u8* location, PLCRuntimeInstructionSet instruction, MY_PTR_t value, u8 bit_offset) {
        u8 offset = push_InstructionWithPointer(location, instruction, value);
        location[offset++] = bit_offset;
        return offset;
    }

    static u8 push_InstructionWithTwoPointers(u8* location, PLCRuntimeInstructionSet instruction, MY_PTR_t addr1, u8 bit1, MY_PTR_t addr2, u8 bit2) {
        return push_read_bit_edge(location, instruction, addr1, bit1, addr2, bit2);
    }

    static u8 push_read_bit_edge(u8* location, PLCRuntimeInstructionSet instruction, MY_PTR_t addr1, u8 bit1, MY_PTR_t addr2, u8 bit2) {
        location[0] = instruction;
        u8 offset = 1;
        // Addr1
        for (u8 i = 0; i < sizeof(MY_PTR_t); i++) location[offset++] = (addr1 >> ((sizeof(MY_PTR_t) - i - 1) * 8)) & 0xFF;
        // Bit1
        location[offset++] = bit1;
        // Addr2
        for (u8 i = 0; i < sizeof(MY_PTR_t); i++) location[offset++] = (addr2 >> ((sizeof(MY_PTR_t) - i - 1) * 8)) & 0xFF;
        // Bit2
        location[offset++] = bit2;
        return offset;
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

    static u8 push_jmp_rel(u8* location, i16 offset) {
        location[0] = JMP_REL;
        u16 u_offset = (u16)offset;
        location[1] = u_offset >> 8;
        location[2] = u_offset & 0xFF;
        return 3;
    }
    static u8 push_jmp_if_rel(u8* location, i16 offset) {
        location[0] = JMP_IF_REL;
        u16 u_offset = (u16)offset;
        location[1] = u_offset >> 8;
        location[2] = u_offset & 0xFF;
        return 3;
    }
    static u8 push_jmp_if_not_rel(u8* location, i16 offset) {
        location[0] = JMP_IF_NOT_REL;
        u16 u_offset = (u16)offset;
        location[1] = u_offset >> 8;
        location[2] = u_offset & 0xFF;
        return 3;
    }

    static u8 pushCALL_REL(u8* location, i16 offset) {
        location[0] = CALL_REL;
        u16 u_offset = (u16)offset;
        location[1] = u_offset >> 8;
        location[2] = u_offset & 0xFF;
        return 3;
    }
    static u8 pushCALL_IF_REL(u8* location, i16 offset) {
        location[0] = CALL_IF_REL;
        u16 u_offset = (u16)offset;
        location[1] = u_offset >> 8;
        location[2] = u_offset & 0xFF;
        return 3;
    }
    static u8 pushCALL_IF_NOT_REL(u8* location, i16 offset) {
        location[0] = CALL_IF_NOT_REL;
        u16 u_offset = (u16)offset;
        location[1] = u_offset >> 8;
        location[2] = u_offset & 0xFF;
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

    // Load value from memory to stack using immediate address
    static u8 push_load_from(u8* location, PLCRuntimeInstructionSet type, MY_PTR_t address) {
        location[0] = LOAD_FROM;
        location[1] = type;
        for (u8 i = 0; i < sizeof(MY_PTR_t); i++) location[i + 2] = (address >> ((sizeof(MY_PTR_t) - i - 1) * 8)) & 0xFF;
        return 2 + sizeof(MY_PTR_t);
    }

    // Move value from stack to memory using immediate address
    static u8 push_move_to(u8* location, PLCRuntimeInstructionSet type, MY_PTR_t address) {
        location[0] = MOVE_TO;
        location[1] = type;
        for (u8 i = 0; i < sizeof(MY_PTR_t); i++) location[i + 2] = (address >> ((sizeof(MY_PTR_t) - i - 1) * 8)) & 0xFF;
        return 2 + sizeof(MY_PTR_t);
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

    // Push timer instruction with pointer (timer address) and constant u32 (preset time)
    // Format: [opcode][timer_addr (MY_PTR_t)][pt_value (u32)]
    static u8 push_timer_const(u8* location, PLCRuntimeInstructionSet instruction, MY_PTR_t timer_addr, u32 pt_value) {
        location[0] = instruction;
        u8 offset = 1;
        // Timer address (uses MY_PTR_t for portability)
        for (u8 i = 0; i < sizeof(MY_PTR_t); i++) location[offset++] = (timer_addr >> ((sizeof(MY_PTR_t) - i - 1) * 8)) & 0xFF;
        // Preset time value (u32 big endian)
        location[offset++] = (pt_value >> 24) & 0xFF;
        location[offset++] = (pt_value >> 16) & 0xFF;
        location[offset++] = (pt_value >> 8) & 0xFF;
        location[offset++] = pt_value & 0xFF;
        return offset;
    }

    // Push timer instruction with two pointers (timer address and preset time address)
    // Format: [opcode][timer_addr (MY_PTR_t)][pt_addr (MY_PTR_t)]
    static u8 push_timer_mem(u8* location, PLCRuntimeInstructionSet instruction, MY_PTR_t timer_addr, MY_PTR_t pt_addr) {
        location[0] = instruction;
        u8 offset = 1;
        // Timer address
        for (u8 i = 0; i < sizeof(MY_PTR_t); i++) location[offset++] = (timer_addr >> ((sizeof(MY_PTR_t) - i - 1) * 8)) & 0xFF;
        // Preset time address
        for (u8 i = 0; i < sizeof(MY_PTR_t); i++) location[offset++] = (pt_addr >> ((sizeof(MY_PTR_t) - i - 1) * 8)) & 0xFF;
        return offset;
    }
};

// ==================== EEPROM Program Storage Support ====================
#ifdef PLCRUNTIME_EEPROM_STORAGE

#include <EEPROM.h>

// EEPROM storage format: [u32 size][u8[] bytecode][u8 crc]
// Total storage overhead: 5 bytes (4 for size + 1 for crc)

#ifndef PLCRUNTIME_EEPROM_START_ADDRESS
#define PLCRUNTIME_EEPROM_START_ADDRESS 0 // Default start address in EEPROM
#endif

// Define EEPROM size for platforms that require EEPROM.begin(size)
// STM32, ESP8266, ESP32 require this. AVR does not.
#ifndef PLCRUNTIME_EEPROM_SIZE
#define PLCRUNTIME_EEPROM_SIZE 2048 // Default EEPROM size (adjust based on your platform)
#endif

namespace EEPROMStorage {
    
    static bool _eeprom_initialized = false;
    
    inline void ensureInitialized() {
        if (_eeprom_initialized) return;
        _eeprom_initialized = true;
#if defined(ESP8266) || defined(ESP32) || defined(STM32) || defined(ARDUINO_ARCH_STM32)
        EEPROM.begin(PLCRUNTIME_EEPROM_SIZE);
#endif
    }
    
    inline u32 getAvailableSpace() {
        ensureInitialized();
#if defined(ESP8266) || defined(ESP32) || defined(STM32) || defined(ARDUINO_ARCH_STM32)
        return PLCRUNTIME_EEPROM_SIZE - PLCRUNTIME_EEPROM_START_ADDRESS;
#else
        return EEPROM.length() - PLCRUNTIME_EEPROM_START_ADDRESS;
#endif
    }
    
    inline bool saveProgram(const u8* program, u32 prog_size, u8 checksum) {
        ensureInitialized();
        
        // Check if program fits in EEPROM (size + bytecode + crc)
        u32 required_space = 4 + prog_size + 1; // 4 bytes for size, prog_size bytes for bytecode, 1 byte for crc
        if (required_space > getAvailableSpace()) {
            Serial.print(F("EEPROM storage error: need "));
            Serial.print(required_space);
            Serial.print(F(" bytes, available: "));
            Serial.println(getAvailableSpace());
            return false;
        }
        
        u32 addr = PLCRUNTIME_EEPROM_START_ADDRESS;
        
        // Write size (4 bytes, little-endian)
        EEPROM.write(addr++, (u8)(prog_size & 0xFF));
        EEPROM.write(addr++, (u8)((prog_size >> 8) & 0xFF));
        EEPROM.write(addr++, (u8)((prog_size >> 16) & 0xFF));
        EEPROM.write(addr++, (u8)((prog_size >> 24) & 0xFF));
        
        // Write bytecode
        for (u32 i = 0; i < prog_size; i++) {
            EEPROM.write(addr++, program[i]);
        }
        
        // Write CRC
        EEPROM.write(addr, checksum);
        
        // Commit changes to flash (required for STM32, ESP8266, ESP32)
#if defined(ESP8266) || defined(ESP32) || defined(STM32) || defined(ARDUINO_ARCH_STM32)
        bool success = EEPROM.commit();
        if (!success) {
            Serial.println(F("EEPROM commit failed"));
            return false;
        }
#endif
        
        return true;
    }
    
    inline bool loadProgram(u8* program, u32& prog_size, u8& stored_checksum) {
        ensureInitialized();
        
        u32 addr = PLCRUNTIME_EEPROM_START_ADDRESS;
        
        // Read size (4 bytes, little-endian)
        prog_size = (u32)EEPROM.read(addr++);
        prog_size |= ((u32)EEPROM.read(addr++) << 8);
        prog_size |= ((u32)EEPROM.read(addr++) << 16);
        prog_size |= ((u32)EEPROM.read(addr++) << 24);
        
        // Validate size
        if (prog_size == 0 || prog_size == 0xFFFFFFFF || prog_size > PLCRUNTIME_MAX_PROGRAM_SIZE) {
            prog_size = 0;
            return false;
        }
        
        // Check if stored program fits in available EEPROM
        if (4 + prog_size + 1 > getAvailableSpace()) {
            prog_size = 0;
            return false;
        }
        
        // Read bytecode
        for (u32 i = 0; i < prog_size; i++) {
            program[i] = EEPROM.read(addr++);
        }
        
        // Read CRC
        stored_checksum = EEPROM.read(addr);
        
        // Verify CRC
        u8 calculated_checksum = 0;
        crc8_simple(calculated_checksum, program, prog_size);
        
        if (calculated_checksum != stored_checksum) {
            prog_size = 0;
            return false;
        }
        
        return true;
    }
    
    inline bool hasValidProgram() {
        ensureInitialized();
        
        u32 addr = PLCRUNTIME_EEPROM_START_ADDRESS;
        
        // Read size (4 bytes, little-endian)
        u32 prog_size = (u32)EEPROM.read(addr++);
        prog_size |= ((u32)EEPROM.read(addr++) << 8);
        prog_size |= ((u32)EEPROM.read(addr++) << 16);
        prog_size |= ((u32)EEPROM.read(addr++) << 24);
        
        return (prog_size > 0 && prog_size != 0xFFFFFFFF && 
                prog_size <= PLCRUNTIME_MAX_PROGRAM_SIZE &&
                4 + prog_size + 1 <= getAvailableSpace());
    }
}

#endif // PLCRUNTIME_EEPROM_STORAGE
// =====================================================================

class RuntimeProgram {
private:
    u32 MAX_PROGRAM_SIZE = PLCRUNTIME_MAX_PROGRAM_SIZE; // Max program size in bytes
public:
    u8 program[PLCRUNTIME_MAX_PROGRAM_SIZE]; // PLC program to execute
    u32 prog_size = 0; // Current program size in bytes
    u32 program_line = 0; // Active program line
    RuntimeError status = UNDEFINED_STATE;

    RuntimeProgram(u32 prog_size) {
        if (prog_size > PLCRUNTIME_MAX_PROGRAM_SIZE) prog_size = PLCRUNTIME_MAX_PROGRAM_SIZE;
        this->MAX_PROGRAM_SIZE = prog_size;
    }
    RuntimeProgram() {}

    void begin(const u8* program, u32 prog_size, u8 checksum) {
        format();
        load(program, prog_size, checksum);
    }

    void beginUnsafe(const u8* program, u32 prog_size) {
        format();
        loadUnsafe(program, prog_size);
    }

    void format() {
        this->prog_size = 0;
        this->program_line = 0;
        this->status = UNDEFINED_STATE;
    }

    RuntimeError loadUnsafe(const u8* program, u32 prog_size) {
        if (prog_size > PLCRUNTIME_MAX_PROGRAM_SIZE) status = PROGRAM_SIZE_EXCEEDED;
        if (prog_size > MAX_PROGRAM_SIZE) status = PROGRAM_SIZE_EXCEEDED;
        else if (prog_size == 0) {
            this->prog_size = 0;
            status = UNDEFINED_STATE;
        } else {
            format();
            // memcpy(this->program, program, prog_size);
            for (u32 i = 0; i < prog_size; i++) this->program[i] = program[i];
            this->prog_size = prog_size;
            status = STATUS_SUCCESS;
        }
        return status;
    }

    RuntimeError load(const u8* program, u32 prog_size, u8 checksum) {
        u8 calculated_checksum = 0;
        crc8_simple(calculated_checksum, program, prog_size);
        if (calculated_checksum != checksum) {
            status = INVALID_CHECKSUM;
            Serial.println(F("Failed to load program: CHECKSUM MISMATCH"));
            return status;
        }
        RuntimeError result = loadUnsafe(program, prog_size);
#ifdef PLCRUNTIME_EEPROM_STORAGE
        // Save valid program to EEPROM for persistence
        if (result == STATUS_SUCCESS) {
            if (!EEPROMStorage::saveProgram(program, prog_size, checksum)) {
                Serial.println(F("Warning: Failed to save program to EEPROM"));
            }
        }
#endif // PLCRUNTIME_EEPROM_STORAGE
        return result;
    }

#ifdef PLCRUNTIME_EEPROM_STORAGE
    // Load program from EEPROM storage
    // Returns STATUS_SUCCESS if a valid program was loaded, INVALID_CHECKSUM if CRC failed, UNDEFINED_STATE if no program stored
    RuntimeError loadFromEEPROM() {
        u8 stored_checksum = 0;
        u32 eeprom_prog_size = 0;
        
        if (!EEPROMStorage::hasValidProgram()) {
            status = UNDEFINED_STATE;
            return status;
        }
        
        if (!EEPROMStorage::loadProgram(this->program, eeprom_prog_size, stored_checksum)) {
            status = INVALID_CHECKSUM;
            Serial.println(F("Failed to load program from EEPROM: CHECKSUM MISMATCH"));
            return status;
        }
        
        this->prog_size = eeprom_prog_size;
        this->program_line = 0;
        status = STATUS_SUCCESS;
        Serial.print(F("Loaded program from EEPROM: "));
        Serial.print(eeprom_prog_size);
        Serial.println(F(" bytes"));
        return status;
    }
#endif // PLCRUNTIME_EEPROM_STORAGE

    // Get the size of used program memory
    u32 size() { return prog_size; }

    // Hot update the running program. This is a very dangerous operation, so use it with caution!
    RuntimeError modify(u32 index, u8 value) {
        if (index >= prog_size) return INVALID_PROGRAM_INDEX;
        program[index] = value;
        return STATUS_SUCCESS;
    }

    // Hot update the running program. This is a very dangerous operation, so use it with caution!
    RuntimeError modify(u32 index, u8* data, u32 size) {
        if (index + size > prog_size) return INVALID_PROGRAM_INDEX;
        for (u32 i = 0; i < size; i++) program[index + i] = data[i];
        return STATUS_SUCCESS;
    }

    RuntimeError modifyValue(u32 index, u32 value) {
        if (index + sizeof(u32) > prog_size) return INVALID_PROGRAM_INDEX;
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

    bool finished() { return program_line >= prog_size; }

    u32 getProgramSize() { return prog_size; }

    int print() {
        int length = Serial.print(F("Program["));
        length += Serial.print(prog_size);
        if (prog_size == 0) {
            length += Serial.print(F("] []"));
            return length;
        }
        length += Serial.print(F("] ["));
        for (u32 i = 0; i < prog_size; i++) {
            u8 value = program[i];
            if (value < 0x10) length += Serial.print('0');
            length += Serial.print(value, HEX);
            if (i < prog_size - 1) length += Serial.print(' ');
        }
        length += Serial.print(']');
        return length;
    }

    int println() { int length = print(); length += Serial.println(); return length; }

    void explain() {
        Serial.println(F("#### Program Explanation:"));
        if (prog_size == 0) {
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
                if ((i + index) >= prog_size) {
                    Serial.println(F(" - OUT OF PROGRAM BOUNDS\n"));
                    return;
                }
                u8 value = program[i + index];
                // Serial.printf(" %02X", value);
                print_number_padStart(value, 2, '0', HEX);
            }
            Serial.println();
            index += instruction_size;
            if (index >= prog_size) done = true;
        }
        Serial.println(F("#### End of program explanation."));
    }

    PLCRuntimeInstructionSet getCurrentBytecode() {
        if (program_line >= prog_size) return EXIT;
        return (PLCRuntimeInstructionSet) program[program_line];
    }

    u8 sizeOfCurrentBytecode() { return OPCODE_SIZE(getCurrentBytecode()); }

    int printOpcodeAt(u32 index) {
        int length = 0;
        if (index >= prog_size) return length;
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
        if (prog_size + code_size > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(prog_size);
            return status;
        }
        prog_size += InstructionCompiler::push(program + prog_size, code, code_size);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push a new instruction to the PLC Program
    RuntimeError push(u8 instruction) {
        if (prog_size + 1 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(prog_size);
            return status;
        }
        prog_size += InstructionCompiler::push(program + prog_size, instruction);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push a new instruction to the PLC Program
    RuntimeError push(u8 instruction, u8 data_type) {
        if (prog_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(prog_size);
            return status;
        }
        prog_size += InstructionCompiler::push(program + prog_size, instruction, data_type);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push a pointer to the PLC Program
    RuntimeError push_pointer(MY_PTR_t pointer) {
        if (prog_size + sizeof(MY_PTR_t) > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        prog_size += InstructionCompiler::push_pointer(program + prog_size, pointer);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push a boolean value to the PLC Program
    RuntimeError push_bool(bool value) {
        return push_u8(value ? 1 : 0);
    }

    // Push an u8 value to the PLC Program
    RuntimeError push_u8(u8 value) {
        if (prog_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(prog_size);
            return status;
        }
        prog_size += InstructionCompiler::push_u8(program + prog_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an i8 value to the PLC Program
    RuntimeError push_i8(i8 value) {
        if (prog_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(prog_size);
            return status;
        }
        prog_size += InstructionCompiler::push_i8(program + prog_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an u16 value to the PLC Program
    RuntimeError push_u16(u32 value) {
        if (prog_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(prog_size);
            return status;
        }
        prog_size += InstructionCompiler::push_u16(program + prog_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an i16 value to the PLC Program
    RuntimeError push_i16(i16 value) {
        if (prog_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(prog_size);
            return status;
        }
        prog_size += InstructionCompiler::push_i16(program + prog_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an u32 value to the PLC Program
    RuntimeError push_u32(u32 value) {
        if (prog_size + 5 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(prog_size);
            return status;
        }
        prog_size += InstructionCompiler::push_u32(program + prog_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an i32 value to the PLC Program
    RuntimeError push_i32(i32 value) {
        if (prog_size + 5 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(prog_size);
            return status;
        }
        prog_size += InstructionCompiler::push_i32(program + prog_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

#ifdef USE_X64_OPS
    // Push an u64 value to the PLC Program
    RuntimeError push_u64(u64 value) {
        if (prog_size + 9 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(prog_size);
            return status;
        }
        prog_size += InstructionCompiler::push_u64(program + prog_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

    // Push an i64 value to the PLC Program
    RuntimeError push_i64(i64 value) {
        if (prog_size + 9 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(prog_size);
            return status;
        }
        prog_size += InstructionCompiler::push_i64(program + prog_size, value);
        status = STATUS_SUCCESS;
        return status;
    }
#endif // USE_X64_OPS

    // Push a float value to the PLC Program
    RuntimeError push_f32(float value) {
        if (prog_size + 5 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(prog_size);
            return status;
        }
        prog_size += InstructionCompiler::push_f32(program + prog_size, value);
        status = STATUS_SUCCESS;
        return status;
    }

#ifdef USE_X64_OPS
    // Push a double value to the PLC Program
    RuntimeError push_f64(double value) {
        if (prog_size + 9 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(prog_size);
            return status;
        }
        prog_size += InstructionCompiler::push_f64(program + prog_size, value);
        status = STATUS_SUCCESS;
        return status;
    }
#endif // USE_X64_OPS

    // Push flow control instructions to the PLC Program
    RuntimeError push_jmp(u32 program_address) {
        if (prog_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(prog_size);
            return status;
        }
        prog_size += InstructionCompiler::push_jmp(program + prog_size, program_address);
        status = STATUS_SUCCESS;
        return status;
    }
    RuntimeError push_jmp_if(u32 program_address) {
        if (prog_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(prog_size);
            return status;
        }
        prog_size += InstructionCompiler::push_jmp_if(program + prog_size, program_address);
        status = STATUS_SUCCESS;
        return status;
    }
    RuntimeError push_jmp_if_not(u32 program_address) {
        if (prog_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            Serial.print(PROGRAM_SIZE_EXCEEDED_MSG); Serial.println(prog_size);
            return status;
        }
        prog_size += InstructionCompiler::push_jmp_if_not(program + prog_size, program_address);
        status = STATUS_SUCCESS;
        return status;
    }

    // Convert a data type to another data type
    RuntimeError push_cvt(PLCRuntimeInstructionSet from, PLCRuntimeInstructionSet to) {
        if (prog_size + 3 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        prog_size += InstructionCompiler::push_cvt(program + prog_size, from, to);
        status = STATUS_SUCCESS;
        return status;
    }

    RuntimeError push_load(PLCRuntimeInstructionSet type = type_u8) {
        if (prog_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        prog_size += InstructionCompiler::push_load(program + prog_size, type);
        status = STATUS_SUCCESS;
        return status;
    }

    RuntimeError push_move(PLCRuntimeInstructionSet type = type_u8) {
        if (prog_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        prog_size += InstructionCompiler::push_move(program + prog_size, type);
        status = STATUS_SUCCESS;
        return status;
    }

    RuntimeError push_move_copy(PLCRuntimeInstructionSet type = type_u8) {
        if (prog_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        prog_size += InstructionCompiler::push_move_copy(program + prog_size, type);
        status = STATUS_SUCCESS;
        return status;
    }

    RuntimeError push_load_from(MY_PTR_t address, PLCRuntimeInstructionSet type = type_u8) {
        if (prog_size + 2 + sizeof(MY_PTR_t) > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        prog_size += InstructionCompiler::push_load_from(program + prog_size, type, address);
        status = STATUS_SUCCESS;
        return status;
    }

    RuntimeError push_move_to(MY_PTR_t address, PLCRuntimeInstructionSet type = type_u8) {
        if (prog_size + 2 + sizeof(MY_PTR_t) > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        prog_size += InstructionCompiler::push_move_to(program + prog_size, type, address);
        status = STATUS_SUCCESS;
        return status;
    }

    // Make a duplica of the top of the stack
    RuntimeError push_copy(PLCRuntimeInstructionSet type = type_u8) {
        if (prog_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        prog_size += InstructionCompiler::push_copy(program + prog_size, type);
        status = STATUS_SUCCESS;
        return status;
    }
    // Swap the top two values on the stack
    RuntimeError push_swap(PLCRuntimeInstructionSet type = type_u8) {
        if (prog_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        prog_size += InstructionCompiler::push_swap(program + prog_size, type);
        status = STATUS_SUCCESS;
        return status;
    }
    // Drop the top value from the stack
    RuntimeError push_drop(PLCRuntimeInstructionSet type = type_u8) {
        if (prog_size + 2 > MAX_PROGRAM_SIZE) {
            status = PROGRAM_SIZE_EXCEEDED;
            return status;
        }
        prog_size += InstructionCompiler::push_drop(program + prog_size, type);
        status = STATUS_SUCCESS;
        return status;
    }
};
