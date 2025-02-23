// runtime-types.h - 1.0.0 - 2024-01-11
//
// Copyright (c) 2024 J.Vovk
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


// Pointer type for the runtime memory
typedef uint16_t  MY_PTR_t;
#define MY_PTR_SIZE 16
#define MY_PTR_SIZE_BYTES 2

typedef uint8_t    u8;
#ifndef __AVR__
typedef uint16_t  u16;
#endif // __AVR__
typedef uint32_t  u32;
typedef uint64_t  u64;

typedef int8_t     i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;

typedef float     f32;
typedef double    f64;


constexpr int size_ptr = sizeof(MY_PTR_t);
constexpr int size_bool = sizeof(u8);
constexpr int size_u8 = sizeof(u8);
constexpr int size_u16 = sizeof(u16);
constexpr int size_u32 = sizeof(u32);
constexpr int size_u64 = sizeof(u64);

union Converter {
#ifdef USE_X64_OPS
    u8 data[8];
#else
    u8 data[4];
#endif // USE_X64_OPS
    u8 u8_value;
    i8 i8_value;
    u16 u16_value;
    i16 i16_value;
    u32 u32_value;
    i32 i32_value;
    float float_value;
#ifdef USE_X64_OPS
    u64 u64_value;
    i64 i64_value;
    double double_value;
#endif // USE_X64_OPS
};