// runtime-parsing.h - 1.0.0 - 2022-12-11
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
    static RuntimeError type_##type(uint8_t* bytecode, uint32_t bytecode_size, uint32_t& index, type* value) {  \
        uint32_t size = sizeof(type);                                                                           \
        if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;                                         \
        *value = bytecode[index];                                                                               \
        index += size;                                                                                          \
        return STATUS_SUCCESS;                                                                                  \
    }

#define EXTRACT_TYPE_16(type)                                                                                   \
    static RuntimeError type_##type(uint8_t* bytecode, uint32_t bytecode_size, uint32_t& index, type* value) {  \
        uint32_t size = sizeof(type);                                                                           \
        if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;                                         \
        *value = ((uint16_t) bytecode[index] << 8) | bytecode[index + 1];                                        \
        index += size;                                                                                          \
        return STATUS_SUCCESS;                                                                                  \
    }

#define EXTRACT_TYPE_32(type)                                                                                                                               \
    static RuntimeError type_##type(uint8_t* bytecode, uint32_t bytecode_size, uint32_t& index, type* value) {                                              \
        uint32_t size = sizeof(type);                                                                                                                       \
        if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;                                                                                     \
        *value = ((type) bytecode[index] << 24) | ((type) bytecode[index + 1] << 16) | ((type) bytecode[index + 2] << 8) | ((type) bytecode[index + 3]);     \
        index += size;                                                                                                                                      \
        return STATUS_SUCCESS;                                                                                                                              \
    }

#define EXTRACT_TYPE_32_CVT(type)                                                                                                                                                     \
    static RuntimeError type_##type(uint8_t* bytecode, uint32_t bytecode_size, uint32_t& index, type* value) {                                                                        \
        uint32_t size = sizeof(type);                                                                                                                                                 \
        if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;                                                                                                               \
        CVT_32(type) cvt;                                                                                                                                                             \
        cvt.type_uint32_t = ((uint32_t) bytecode[index] << 24) | ((uint32_t) bytecode[index + 1] << 16) | ((uint32_t) bytecode[index + 2] << 8) | ((uint32_t) bytecode[index + 3]);   \
        *value = cvt.type_##type;                                                                                                                                                      \
        index += size;                                                                                                                                                                \
        return STATUS_SUCCESS;                                                                                                                                                        \
    }

#define EXTRACT_TYPE_64(type)                                                                                                                                       \
    static RuntimeError type_##type(uint8_t* bytecode, uint32_t bytecode_size, uint32_t& index, type* value) {                                                      \
        uint32_t size = sizeof(type);                                                                                                                               \
        if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;                                                                                             \
        *value = ((type) bytecode[index] << 56) | ((type) bytecode[index + 1] << 48) | ((type) bytecode[index + 2] << 40) | ((type) bytecode[index + 3] << 32) |     \
                ((type) bytecode[index + 4] << 24) | ((type) bytecode[index + 5] << 16) | ((type) bytecode[index + 6] << 8) | ((type) bytecode[index + 7]);         \
        index += size;                                                                                                                                              \
        return STATUS_SUCCESS;                                                                                                                                      \
    }

#define EXTRACT_TYPE_64_CVT(type)                                                                                                                                                           \
    static RuntimeError type_##type(uint8_t* bytecode, uint32_t bytecode_size, uint32_t& index, type* value) {                                                                              \
        uint32_t size = sizeof(type);                                                                                                                                                       \
        if (index + size > bytecode_size) return PROGRAM_SIZE_EXCEEDED;                                                                                                                     \
        CVT_64(type) cvt;                                                                                                                                                                   \
        cvt.type_uint64_t = ((uint64_t) bytecode[index] << 56) | ((uint64_t) bytecode[index + 1] << 48) | ((uint64_t) bytecode[index + 2] << 40) | ((uint64_t) bytecode[index + 3] << 32) | \
                            ((uint64_t) bytecode[index + 4] << 24) | ((uint64_t) bytecode[index + 5] << 16) | ((uint64_t) bytecode[index + 6] << 8) | ((uint64_t) bytecode[index + 7]);     \
        *value = cvt.type_##type;                                                                                                                                                            \
        index += size;                                                                                                                                                                      \
        return STATUS_SUCCESS;                                                                                                                                                              \
    }

struct Extract_t {
    EXTRACT_TYPE_8(bool);
    EXTRACT_TYPE_8(uint8_t);
    EXTRACT_TYPE_8(int8_t);

    EXTRACT_TYPE_16(uint16_t);
    EXTRACT_TYPE_16(int16_t);

    EXTRACT_TYPE_32(uint32_t);
    EXTRACT_TYPE_32(int32_t);
    EXTRACT_TYPE_32_CVT(float);

    EXTRACT_TYPE_64(uint64_t);
    EXTRACT_TYPE_64(int64_t);
    EXTRACT_TYPE_64_CVT(double);
} ProgramExtract;

RuntimeError extract_status;