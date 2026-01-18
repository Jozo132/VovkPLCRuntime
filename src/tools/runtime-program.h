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

// ==================== EEPROM/Flash Program Storage Support ====================
#ifdef PLCRUNTIME_EEPROM_STORAGE

// EEPROM storage format: [u32 size][u8[] bytecode][u8 crc]
// Total storage overhead: 5 bytes (4 for size + 1 for crc)

#ifndef PLCRUNTIME_EEPROM_START_ADDRESS
#define PLCRUNTIME_EEPROM_START_ADDRESS 0 // Default start address within storage area
#endif

// ============================================================================
// STM32 Direct Flash Implementation (no RAM buffer overhead, multi-page support)
// ============================================================================
#if defined(STM32) || defined(ARDUINO_ARCH_STM32)

// ---- STM32F4 Sector Layout (non-uniform!) ----
// Sector 0-3:  16KB each  (0x08000000 - 0x0800FFFF)
// Sector 4:    64KB       (0x08010000 - 0x0801FFFF)
// Sector 5-7:  128KB each (0x08020000 - 0x0807FFFF) for 512KB flash
// Sector 5-11: 128KB each (0x08020000 - 0x080FFFFF) for 1MB flash
//
// Flash size detection: STM32 stores flash size at 0x1FFF7A22 (F4) or defined by FLASH_SIZE macro
// FLASH_END is typically defined in the CMSIS device header (e.g., stm32f401xe.h)

#if defined(STM32F4) || defined(STM32F4xx)

// Auto-detect flash size using CMSIS FLASH_SIZE_DATA_REGISTER or fallback
// On STM32F4, flash size (in KB) is stored at address 0x1FFF7A22
#ifndef PLCRUNTIME_STM32_FLASH_SIZE_KB
  #if defined(FLASH_SIZE)
    // FLASH_SIZE is typically in bytes in some HAL versions
    #if FLASH_SIZE > 0x10000
      #define PLCRUNTIME_STM32_FLASH_SIZE_KB (FLASH_SIZE / 1024)
    #else
      #define PLCRUNTIME_STM32_FLASH_SIZE_KB FLASH_SIZE
    #endif
  #elif defined(FLASHSIZE_BASE)
    // Read from flash size register (16-bit value in KB at FLASHSIZE_BASE)
    #define PLCRUNTIME_STM32_FLASH_SIZE_KB (*(volatile uint16_t*)FLASHSIZE_BASE)
  #else
    // Fallback: read from known F4 flash size register address
    #define PLCRUNTIME_STM32_FLASH_SIZE_KB (*(volatile uint16_t*)0x1FFF7A22)
  #endif
#endif

// For F4, auto-detect the last sector based on flash size
// User can override with PLCRUNTIME_STM32_FLASH_SECTOR
#ifndef PLCRUNTIME_STM32_FLASH_SECTOR
  // Use compile-time detection if available, otherwise runtime detection is needed
  // Flash sizes: 128KB=sector 3, 256KB=sector 5, 384KB=sector 6, 512KB=sector 7
  //              768KB=sector 9, 1024KB=sector 11, 1536KB=sector 17, 2048KB=sector 23
  #if defined(FLASH_SIZE) && (FLASH_SIZE <= 131072 || (defined(FLASH_SIZE) && FLASH_SIZE <= 128))
    // 128KB or less - last sector is sector 3 (16KB at 0x0800C000)
    #define PLCRUNTIME_STM32_FLASH_SECTOR FLASH_SECTOR_3
    #define PLCRUNTIME_STM32_FLASH_ADDRESS 0x0800C000
    #define PLCRUNTIME_STM32_FLASH_PAGE_SIZE 16384  // 16KB
  #elif defined(FLASH_SIZE) && (FLASH_SIZE <= 262144 || (defined(FLASH_SIZE) && FLASH_SIZE <= 256))
    // 256KB - last sector is sector 5 (128KB at 0x08020000)
    #define PLCRUNTIME_STM32_FLASH_SECTOR FLASH_SECTOR_5
    #define PLCRUNTIME_STM32_FLASH_ADDRESS 0x08020000
    #define PLCRUNTIME_STM32_FLASH_PAGE_SIZE 131072  // 128KB
  #elif defined(FLASH_SIZE) && (FLASH_SIZE <= 524288 || (defined(FLASH_SIZE) && FLASH_SIZE <= 512))
    // 512KB - last sector is sector 7 (128KB at 0x08060000)
    #define PLCRUNTIME_STM32_FLASH_SECTOR FLASH_SECTOR_7
    #define PLCRUNTIME_STM32_FLASH_ADDRESS 0x08060000
    #define PLCRUNTIME_STM32_FLASH_PAGE_SIZE 131072  // 128KB
  #elif defined(FLASH_SECTOR_11)
    // 1MB - last sector is sector 11
    #define PLCRUNTIME_STM32_FLASH_SECTOR FLASH_SECTOR_11
    #define PLCRUNTIME_STM32_FLASH_ADDRESS 0x080E0000
    #define PLCRUNTIME_STM32_FLASH_PAGE_SIZE 131072  // 128KB
  #elif defined(FLASH_SECTOR_7)
    // Fallback to sector 7
    #define PLCRUNTIME_STM32_FLASH_SECTOR FLASH_SECTOR_7
    #define PLCRUNTIME_STM32_FLASH_ADDRESS 0x08060000
    #define PLCRUNTIME_STM32_FLASH_PAGE_SIZE 131072  // 128KB
  #elif defined(FLASH_SECTOR_5)
    // Fallback to sector 5
    #define PLCRUNTIME_STM32_FLASH_SECTOR FLASH_SECTOR_5
    #define PLCRUNTIME_STM32_FLASH_ADDRESS 0x08020000
    #define PLCRUNTIME_STM32_FLASH_PAGE_SIZE 131072  // 128KB
  #else
    // Minimum fallback to sector 4 (64KB)
    #define PLCRUNTIME_STM32_FLASH_SECTOR FLASH_SECTOR_4
    #define PLCRUNTIME_STM32_FLASH_ADDRESS 0x08010000
    #define PLCRUNTIME_STM32_FLASH_PAGE_SIZE 65536   // 64KB
  #endif
#endif

// Allow user to override flash address and page size if sector is defined
#ifndef PLCRUNTIME_STM32_FLASH_ADDRESS
  #if defined(FLASH_END) && defined(PLCRUNTIME_STM32_FLASH_PAGE_SIZE)
    // Use FLASH_END to calculate last sector address
    #define PLCRUNTIME_STM32_FLASH_ADDRESS ((FLASH_END + 1) - PLCRUNTIME_STM32_FLASH_PAGE_SIZE)
  #endif
#endif

#ifndef PLCRUNTIME_STM32_NUM_SECTORS
#define PLCRUNTIME_STM32_NUM_SECTORS 1
#endif

#define PLCRUNTIME_STM32_NUM_PAGES PLCRUNTIME_STM32_NUM_SECTORS
#define PLCRUNTIME_STM32_ACTUAL_SIZE PLCRUNTIME_STM32_FLASH_PAGE_SIZE

#ifndef PLCRUNTIME_EEPROM_SIZE
#define PLCRUNTIME_EEPROM_SIZE PLCRUNTIME_STM32_FLASH_PAGE_SIZE
#endif

namespace EEPROMStorage {
    
    static const u8* const _flash_base = (const u8*)PLCRUNTIME_STM32_FLASH_ADDRESS;
    
    inline u32 getAvailableSpace() {
        return PLCRUNTIME_STM32_ACTUAL_SIZE - PLCRUNTIME_EEPROM_START_ADDRESS;
    }
    
    inline u32 getPageSize() { return PLCRUNTIME_STM32_FLASH_PAGE_SIZE; }
    inline u32 getNumPages() { return PLCRUNTIME_STM32_NUM_SECTORS; }
    inline u32 getFlashAddress() { return PLCRUNTIME_STM32_FLASH_ADDRESS; }
    
    inline bool _unlockFlash() {
        HAL_FLASH_Unlock();
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                               FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
        return true;
    }
    
    inline void _lockFlash() {
        HAL_FLASH_Lock();
    }
    
    inline bool _eraseAllPages() {
        FLASH_EraseInitTypeDef eraseInit;
        uint32_t sectorError = 0;
        
        eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
        eraseInit.Banks = FLASH_BANK_1;
        eraseInit.Sector = PLCRUNTIME_STM32_FLASH_SECTOR;
        eraseInit.NbSectors = PLCRUNTIME_STM32_NUM_SECTORS;
        eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3; // 2.7V - 3.6V
        
        HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&eraseInit, &sectorError);
        if (status != HAL_OK) {
            Serial.print(F("Erase error: "));
            Serial.print(status);
            Serial.print(F(" at sector "));
            Serial.println(sectorError);
            return false;
        }
        return true;
    }
    
    inline bool _programWord(u32 address, u32 data) {
        HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);
        if (status != HAL_OK) {
            Serial.print(F("Write error at 0x"));
            Serial.print(address, HEX);
            Serial.print(F(": "));
            Serial.println(status);
            return false;
        }
        return true;
    }
    
    inline bool saveProgram(const u8* program, u32 prog_size, u8 checksum) {
        u32 required_space = 4 + prog_size + 1;
        if (required_space > getAvailableSpace()) {
            Serial.print(F("Flash storage error: need "));
            Serial.print(required_space);
            Serial.print(F(" bytes, available: "));
            Serial.println(getAvailableSpace());
            return false;
        }
        
        if (!_unlockFlash()) {
            Serial.println(F("Flash unlock failed"));
            return false;
        }
        
        if (!_eraseAllPages()) {
            _lockFlash();
            Serial.println(F("Flash erase failed"));
            return false;
        }
        
        u32 addr = PLCRUNTIME_STM32_FLASH_ADDRESS + PLCRUNTIME_EEPROM_START_ADDRESS;
        bool success = true;
        
        // Write size (4 bytes as one 32-bit word)
        if (!_programWord(addr, prog_size)) success = false;
        addr += 4;
        
        // Write bytecode - 4 bytes at a time as 32-bit words
        for (u32 i = 0; i < prog_size && success; i += 4) {
            u32 word = program[i];
            if (i + 1 < prog_size) word |= ((u32)program[i + 1]) << 8;
            if (i + 2 < prog_size) word |= ((u32)program[i + 2]) << 16;
            if (i + 3 < prog_size) word |= ((u32)program[i + 3]) << 24;
            if (!_programWord(addr, word)) success = false;
            addr += 4;
        }
        
        // Write CRC (as 32-bit word with padding)
        if (success) {
            if (!_programWord(addr, (u32)checksum)) success = false;
        }
        
        _lockFlash();
        
        if (success) {
            Serial.print(F("Program saved to flash: "));
            Serial.print(prog_size);
            Serial.print(F(" bytes in sector "));
            Serial.println(PLCRUNTIME_STM32_FLASH_SECTOR);
        }
        
        return success;
    }
    
    inline bool loadProgram(u8* program, u32& prog_size, u8& stored_checksum) {
        const u8* flash_ptr = _flash_base + PLCRUNTIME_EEPROM_START_ADDRESS;
        
        prog_size = *((const u32*)flash_ptr);
        flash_ptr += 4;
        
        if (prog_size == 0 || prog_size == 0xFFFFFFFF || prog_size > PLCRUNTIME_MAX_PROGRAM_SIZE) {
            prog_size = 0;
            return false;
        }
        
        if (4 + prog_size + 1 > getAvailableSpace()) {
            prog_size = 0;
            return false;
        }
        
        for (u32 i = 0; i < prog_size; i++) {
            program[i] = flash_ptr[i];
        }
        flash_ptr += prog_size;
        
        // Align to 4-byte boundary for CRC read
        u32 offset = prog_size % 4;
        if (offset != 0) flash_ptr += (4 - offset);
        
        stored_checksum = *flash_ptr;
        
        u8 calculated_checksum = 0;
        crc8_simple(calculated_checksum, program, prog_size);
        
        if (calculated_checksum != stored_checksum) {
            prog_size = 0;
            return false;
        }
        
        return true;
    }
    
    inline bool hasValidProgram() {
        const u8* flash_ptr = _flash_base + PLCRUNTIME_EEPROM_START_ADDRESS;
        u32 prog_size = *((const u32*)flash_ptr);
        
        return (prog_size > 0 && prog_size != 0xFFFFFFFF && 
                prog_size <= PLCRUNTIME_MAX_PROGRAM_SIZE &&
                4 + prog_size + 1 <= getAvailableSpace());
    }
    
    inline void printConfig() {
        Serial.println(F("=== STM32F4 Flash Storage Config ==="));
        // Print detected flash size (read from OTP at runtime)
        uint16_t flashSizeKB = *(volatile uint16_t*)0x1FFF7A22;
        Serial.print(F("Device flash size: "));
        Serial.print(flashSizeKB);
        Serial.println(F(" KB"));
#ifdef FLASH_END
        Serial.print(F("FLASH_END: 0x"));
        Serial.println(FLASH_END, HEX);
#endif
        Serial.print(F("Storage address: 0x"));
        Serial.println(PLCRUNTIME_STM32_FLASH_ADDRESS, HEX);
        Serial.print(F("Sector: "));
        Serial.println(PLCRUNTIME_STM32_FLASH_SECTOR);
        Serial.print(F("Sector size: "));
        Serial.print(PLCRUNTIME_STM32_FLASH_PAGE_SIZE / 1024);
        Serial.println(F(" KB"));
        Serial.print(F("Available for program: "));
        Serial.print((getAvailableSpace() - 5) / 1024);
        Serial.println(F(" KB"));
        Serial.println(F("===================================="));
    }
}

// ---- STM32F1 and other STM32 families (uniform page layout) ----
#else // Not STM32F4

#ifndef PLCRUNTIME_STM32_FLASH_PAGE_SIZE
  #if defined(STM32F1) || defined(STM32F1xx)
    #if defined(STM32F103xB) || defined(STM32F103x8)
      #define PLCRUNTIME_STM32_FLASH_PAGE_SIZE 1024  // Medium density: 1KB pages
    #else
      #define PLCRUNTIME_STM32_FLASH_PAGE_SIZE 2048  // High density: 2KB pages
    #endif
  #else
    #define PLCRUNTIME_STM32_FLASH_PAGE_SIZE 2048    // Default fallback
  #endif
#endif

#ifndef PLCRUNTIME_EEPROM_SIZE
#define PLCRUNTIME_EEPROM_SIZE PLCRUNTIME_STM32_FLASH_PAGE_SIZE
#endif

#define PLCRUNTIME_STM32_NUM_PAGES ((PLCRUNTIME_EEPROM_SIZE + PLCRUNTIME_STM32_FLASH_PAGE_SIZE - 1) / PLCRUNTIME_STM32_FLASH_PAGE_SIZE)

#ifndef PLCRUNTIME_STM32_FLASH_ADDRESS
  #if defined(FLASH_BANK1_END)
    #define PLCRUNTIME_STM32_FLASH_ADDRESS (FLASH_BANK1_END - (PLCRUNTIME_STM32_NUM_PAGES * PLCRUNTIME_STM32_FLASH_PAGE_SIZE) + 1)
  #elif defined(FLASH_END)
    #define PLCRUNTIME_STM32_FLASH_ADDRESS (FLASH_END - (PLCRUNTIME_STM32_NUM_PAGES * PLCRUNTIME_STM32_FLASH_PAGE_SIZE) + 1)
  #else
    #define PLCRUNTIME_STM32_FLASH_ADDRESS (0x08010000 - (PLCRUNTIME_STM32_NUM_PAGES * PLCRUNTIME_STM32_FLASH_PAGE_SIZE))
  #endif
#endif

#define PLCRUNTIME_STM32_ACTUAL_SIZE (PLCRUNTIME_STM32_NUM_PAGES * PLCRUNTIME_STM32_FLASH_PAGE_SIZE)

namespace EEPROMStorage {
    
    static const u8* const _flash_base = (const u8*)PLCRUNTIME_STM32_FLASH_ADDRESS;
    
    inline u32 getAvailableSpace() {
        return PLCRUNTIME_STM32_ACTUAL_SIZE - PLCRUNTIME_EEPROM_START_ADDRESS;
    }
    
    inline u32 getPageSize() { return PLCRUNTIME_STM32_FLASH_PAGE_SIZE; }
    inline u32 getNumPages() { return PLCRUNTIME_STM32_NUM_PAGES; }
    inline u32 getFlashAddress() { return PLCRUNTIME_STM32_FLASH_ADDRESS; }
    
    inline bool _unlockFlash() {
        HAL_FLASH_Unlock();
        return true;
    }
    
    inline void _lockFlash() {
        HAL_FLASH_Lock();
    }
    
    inline bool _eraseAllPages() {
        FLASH_EraseInitTypeDef eraseInit;
        uint32_t pageError = 0;
        
        eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
        eraseInit.PageAddress = PLCRUNTIME_STM32_FLASH_ADDRESS;
        eraseInit.NbPages = PLCRUNTIME_STM32_NUM_PAGES;
        
        return HAL_FLASHEx_Erase(&eraseInit, &pageError) == HAL_OK;
    }
    
    inline bool saveProgram(const u8* program, u32 prog_size, u8 checksum) {
        u32 required_space = 4 + prog_size + 1;
        if (required_space > getAvailableSpace()) {
            Serial.print(F("Flash storage error: need "));
            Serial.print(required_space);
            Serial.print(F(" bytes, available: "));
            Serial.print(getAvailableSpace());
            Serial.print(F(" ("));
            Serial.print(PLCRUNTIME_STM32_NUM_PAGES);
            Serial.println(F(" pages)"));
            return false;
        }
        
        if (!_unlockFlash()) {
            Serial.println(F("Flash unlock failed"));
            return false;
        }
        
        if (!_eraseAllPages()) {
            _lockFlash();
            Serial.println(F("Flash erase failed"));
            return false;
        }
        
        u32 addr = PLCRUNTIME_STM32_FLASH_ADDRESS + PLCRUNTIME_EEPROM_START_ADDRESS;
        bool success = true;
        
        // Write size (4 bytes as two 16-bit half-words)
        uint16_t size_lo = (uint16_t)(prog_size & 0xFFFF);
        uint16_t size_hi = (uint16_t)((prog_size >> 16) & 0xFFFF);
        
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, size_lo) != HAL_OK) success = false;
        addr += 2;
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, size_hi) != HAL_OK) success = false;
        addr += 2;
        
        // Write bytecode - two bytes at a time as 16-bit values
        for (u32 i = 0; i < prog_size && success; i += 2) {
            uint16_t halfword = program[i];
            if (i + 1 < prog_size) {
                halfword |= ((uint16_t)program[i + 1]) << 8;
            }
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, halfword) != HAL_OK) {
                success = false;
            }
            addr += 2;
        }
        
        // Write CRC (as 16-bit with padding)
        if (success) {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, (uint16_t)checksum) != HAL_OK) {
                success = false;
            }
        }
        
        _lockFlash();
        
        if (!success) {
            Serial.println(F("Flash write failed"));
        } else {
            Serial.print(F("Program saved to flash: "));
            Serial.print(prog_size);
            Serial.print(F(" bytes across "));
            Serial.print(PLCRUNTIME_STM32_NUM_PAGES);
            Serial.println(F(" page(s)"));
        }
        
        return success;
    }
    
    inline bool loadProgram(u8* program, u32& prog_size, u8& stored_checksum) {
        const u8* flash_ptr = _flash_base + PLCRUNTIME_EEPROM_START_ADDRESS;
        
        prog_size = *((const u32*)flash_ptr);
        flash_ptr += 4;
        
        if (prog_size == 0 || prog_size == 0xFFFFFFFF || prog_size > PLCRUNTIME_MAX_PROGRAM_SIZE) {
            prog_size = 0;
            return false;
        }
        
        if (4 + prog_size + 1 > getAvailableSpace()) {
            prog_size = 0;
            return false;
        }
        
        for (u32 i = 0; i < prog_size; i++) {
            program[i] = flash_ptr[i];
        }
        flash_ptr += prog_size;
        
        if (prog_size & 1) flash_ptr++;
        
        stored_checksum = *flash_ptr;
        
        u8 calculated_checksum = 0;
        crc8_simple(calculated_checksum, program, prog_size);
        
        if (calculated_checksum != stored_checksum) {
            prog_size = 0;
            return false;
        }
        
        return true;
    }
    
    inline bool hasValidProgram() {
        const u8* flash_ptr = _flash_base + PLCRUNTIME_EEPROM_START_ADDRESS;
        u32 prog_size = *((const u32*)flash_ptr);
        
        return (prog_size > 0 && prog_size != 0xFFFFFFFF && 
                prog_size <= PLCRUNTIME_MAX_PROGRAM_SIZE &&
                4 + prog_size + 1 <= getAvailableSpace());
    }
    
    inline void printConfig() {
        Serial.println(F("=== STM32 Flash Storage Config ==="));
        Serial.print(F("Flash address: 0x"));
        Serial.println(PLCRUNTIME_STM32_FLASH_ADDRESS, HEX);
        Serial.print(F("Page size: "));
        Serial.print(PLCRUNTIME_STM32_FLASH_PAGE_SIZE);
        Serial.println(F(" bytes"));
        Serial.print(F("Number of pages: "));
        Serial.println(PLCRUNTIME_STM32_NUM_PAGES);
        Serial.print(F("Total capacity: "));
        Serial.print(PLCRUNTIME_STM32_ACTUAL_SIZE);
        Serial.println(F(" bytes"));
        Serial.print(F("Available for program: "));
        Serial.print(getAvailableSpace() - 5);
        Serial.println(F(" bytes"));
        Serial.println(F("=================================="));
    }
}

#endif // STM32F4 vs other STM32

// ============================================================================
// ESP8266/ESP32 and AVR Implementation (uses EEPROM library)
// ============================================================================
#else // Not STM32

#include <EEPROM.h>

// Define EEPROM size for platforms that require EEPROM.begin(size)
#ifndef PLCRUNTIME_EEPROM_SIZE
#define PLCRUNTIME_EEPROM_SIZE 2048 // Default EEPROM size for ESP platforms
#endif

namespace EEPROMStorage {
    
    static bool _eeprom_initialized = false;
    
    inline void ensureInitialized() {
        if (_eeprom_initialized) return;
        _eeprom_initialized = true;
#if defined(ESP8266) || defined(ESP32)
        EEPROM.begin(PLCRUNTIME_EEPROM_SIZE);
#endif
        // AVR has true EEPROM, no initialization needed
    }
    
    inline u32 getAvailableSpace() {
        ensureInitialized();
#if defined(ESP8266) || defined(ESP32)
        return PLCRUNTIME_EEPROM_SIZE - PLCRUNTIME_EEPROM_START_ADDRESS;
#else
        return EEPROM.length() - PLCRUNTIME_EEPROM_START_ADDRESS;
#endif
    }
    
    inline bool saveProgram(const u8* program, u32 prog_size, u8 checksum) {
        ensureInitialized();
        
        u32 required_space = 4 + prog_size + 1;
        if (required_space > getAvailableSpace()) {
            Serial.print(F("EEPROM storage error: need "));
            Serial.print(required_space);
            Serial.print(F(" bytes, available: "));
            Serial.println(getAvailableSpace());
            return false;
        }
        
        u32 addr = PLCRUNTIME_EEPROM_START_ADDRESS;
        
        // Write size
        EEPROM.put(addr, prog_size);
        addr += sizeof(u32);
        
        // Write bytecode
        for (u32 i = 0; i < prog_size; i++) {
            EEPROM.update(addr++, program[i]);
        }
        
        // Write CRC
        EEPROM.update(addr, checksum);
        
#if defined(ESP8266) || defined(ESP32)
        if (!EEPROM.commit()) {
            Serial.println(F("EEPROM commit failed"));
            return false;
        }
#endif
        
        return true;
    }
    
    inline bool loadProgram(u8* program, u32& prog_size, u8& stored_checksum) {
        ensureInitialized();
        
        u32 addr = PLCRUNTIME_EEPROM_START_ADDRESS;
        
        EEPROM.get(addr, prog_size);
        addr += sizeof(u32);
        
        if (prog_size == 0 || prog_size == 0xFFFFFFFF || prog_size > PLCRUNTIME_MAX_PROGRAM_SIZE) {
            prog_size = 0;
            return false;
        }
        
        if (4 + prog_size + 1 > getAvailableSpace()) {
            prog_size = 0;
            return false;
        }
        
        for (u32 i = 0; i < prog_size; i++) {
            program[i] = EEPROM.read(addr++);
        }
        
        stored_checksum = EEPROM.read(addr);
        
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
        u32 prog_size = 0;
        
        EEPROM.get(addr, prog_size);
        
        return (prog_size > 0 && prog_size != 0xFFFFFFFF && 
                prog_size <= PLCRUNTIME_MAX_PROGRAM_SIZE &&
                4 + prog_size + 1 <= getAvailableSpace());
    }
}

#endif // STM32 vs other platforms

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
