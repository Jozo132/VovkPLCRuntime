// runtime-parsing.h - 2022-12-11
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

#define EXTRACT_TYPE_8(type)                                                                                    \
    static RuntimeError type_##type(u8* bytecode, u32 bytecode_size, u32& index, type* value) {                 \
        u32 size = sizeof(type);                                                                                \
        SAFE_BOUNDS_CHECK(index + size > bytecode_size, PROGRAM_SIZE_EXCEEDED);                                 \
        *value = bytecode[index];                                                                               \
        index += size;                                                                                          \
        return STATUS_SUCCESS;                                                                                  \
    }

#define EXTRACT_TYPE_16(type)                                                                                   \
    static RuntimeError type_##type(u8* bytecode, u32 bytecode_size, u32& index, type* value) {                 \
        u32 size = sizeof(type);                                                                                \
        SAFE_BOUNDS_CHECK(index + size > bytecode_size, PROGRAM_SIZE_EXCEEDED);                                 \
        memcpy(value, bytecode + index, size); /* unaligned-safe */                                             \
        index += size;                                                                                          \
        return STATUS_SUCCESS;                                                                                  \
    }

#define EXTRACT_TYPE_32(type)                                                                                   \
    static RuntimeError type_##type(u8* bytecode, u32 bytecode_size, u32& index, type* value) {                 \
        u32 size = sizeof(type);                                                                                \
        SAFE_BOUNDS_CHECK(index + size > bytecode_size, PROGRAM_SIZE_EXCEEDED);                                 \
        memcpy(value, bytecode + index, size); /* unaligned-safe */                                             \
        index += size;                                                                                          \
        return STATUS_SUCCESS;                                                                                  \
    }

#define EXTRACT_TYPE_32_CVT(type)                                                                               \
    static RuntimeError type_##type(u8* bytecode, u32 bytecode_size, u32& index, type* value) {                 \
        u32 size = sizeof(type);                                                                                \
        SAFE_BOUNDS_CHECK(index + size > bytecode_size, PROGRAM_SIZE_EXCEEDED);                                 \
        memcpy(value, bytecode + index, size); /* unaligned-safe */                                             \
        index += size;                                                                                          \
        return STATUS_SUCCESS;                                                                                  \
    }

#define EXTRACT_TYPE_64(type)                                                                                   \
    static RuntimeError type_##type(u8* bytecode, u32 bytecode_size, u32& index, type* value) {                 \
        u32 size = sizeof(type);                                                                                \
        SAFE_BOUNDS_CHECK(index + size > bytecode_size, PROGRAM_SIZE_EXCEEDED);                                 \
        memcpy(value, bytecode + index, size); /* unaligned-safe */                                             \
        index += size;                                                                                          \
        return STATUS_SUCCESS;                                                                                  \
    }

#define EXTRACT_TYPE_64_CVT(type)                                                                               \
    static RuntimeError type_##type(u8* bytecode, u32 bytecode_size, u32& index, type* value) {                 \
        u32 size = sizeof(type);                                                                                \
        SAFE_BOUNDS_CHECK(index + size > bytecode_size, PROGRAM_SIZE_EXCEEDED);                                 \
        memcpy(value, bytecode + index, size); /* unaligned-safe */                                             \
        index += size;                                                                                          \
        return STATUS_SUCCESS;                                                                                  \
    }

struct Extract_t {

    static RuntimeError type_pointer(u8* bytecode, u32 bytecode_size, u32& index, MY_PTR_t* value) {
        u32 size = sizeof(MY_PTR_t);
        SAFE_BOUNDS_CHECK(index + size > bytecode_size, PROGRAM_SIZE_EXCEEDED);
        // Extract using native pointer read
        *value = read_ptr(bytecode + index);
        index += size;
        return STATUS_SUCCESS;
    }

    EXTRACT_TYPE_8(bool);
    EXTRACT_TYPE_8(u8);
    EXTRACT_TYPE_8(i8);

    EXTRACT_TYPE_16(u16);
    EXTRACT_TYPE_16(i16);

    EXTRACT_TYPE_32(u32);
    EXTRACT_TYPE_32(i32);
    EXTRACT_TYPE_32_CVT(f32);

#ifdef USE_X64_OPS
    EXTRACT_TYPE_64(u64);
    EXTRACT_TYPE_64(i64);
    EXTRACT_TYPE_64_CVT(f64);
#endif
} ProgramExtract;

RuntimeError extract_status;


RuntimeError printOpcodeAt(const u8* program, u32 size, u32 index) {
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
    u8 opcode_size = OPCODE_SIZE(opcode);
    Serial.print(F("Opcode ("));
    Serial.print(OPCODE_NAME(opcode));
    Serial.print(F(") ["));
    for (u8 i = 0; i < opcode_size; i++) {
        u8 value = program[index + i];
        if (value < 0x10) Serial.print('0');
        Serial.print(value, HEX);
        if (i < opcode_size - 1) Serial.print(' ');
    }
    Serial.print(']');
    return STATUS_SUCCESS;
}
RuntimeError printlnOpcodeAt(const u8* program, u32 size, u32 index) {
    RuntimeError error = printOpcodeAt(program, size, index);
    Serial.println();
    return error;
}


RuntimeError CHECK_PROGRAM_POINTER_BOUNDS_HEAD(u8* program, u32 prog_size, u32& index, u32 index_start) {
#ifdef __RUNTIME_DEBUG__
    Serial.print(F("Program pointer points out of bounds: "));
    Serial.print(index);
    Serial.print(F(" > "));
    Serial.println(prog_size);
    printOpcodeAt(program, prog_size, index_start);
    return PROGRAM_POINTER_OUT_OF_BOUNDS;
#else                            
    return PROGRAM_POINTER_OUT_OF_BOUNDS;
#endif
}