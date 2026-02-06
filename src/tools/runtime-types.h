// runtime-types.h - 2024-01-11
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

// ============================================================================
// Safe Mode - Enable runtime bounds checking
// ============================================================================
// By default, bounds checking is DISABLED for maximum performance.
// Define PLCRUNTIME_SAFE_MODE to enable all safety checks.
// Use safe mode during development/debugging, disable for production.
//
// When DISABLED: Assumes bytecode is valid and trusted (faster execution)
// When ENABLED:  Full bounds checking on stack, memory, and program access
// ============================================================================
// #define PLCRUNTIME_SAFE_MODE  // Uncomment to enable safe mode

// Conditional bounds check macros
#ifdef PLCRUNTIME_SAFE_MODE
    #define SAFE_BOUNDS_CHECK(condition, error_return) if (condition) return error_return
    #define SAFE_BOUNDS_CHECK_VOID(condition) if (condition) return
    #define SAFE_BOUNDS_CHECK_BOOL(condition) if (condition) return true
    #define SAFE_BOUNDS_CHECK_ZERO(condition) if (condition) return 0
#else
    #define SAFE_BOUNDS_CHECK(condition, error_return) ((void)0)
    #define SAFE_BOUNDS_CHECK_VOID(condition) ((void)0)
    #define SAFE_BOUNDS_CHECK_BOOL(condition) ((void)0)
    #define SAFE_BOUNDS_CHECK_ZERO(condition) ((void)0)
#endif

// ============================================================================
// Endianness detection - detect at compile time
// ============================================================================
// Runtime uses native endianness for both stack and memory operations.
// The endianness is stored in system memory byte 0 (S0) so external tools
// can correctly interpret multi-byte values.
// 
// PLCRUNTIME_LITTLE_ENDIAN = 1 for little-endian (x86, ARM, WASM)
// PLCRUNTIME_LITTLE_ENDIAN = 0 for big-endian (some PowerPC, network byte order)
// ============================================================================

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define PLCRUNTIME_LITTLE_ENDIAN 1
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define PLCRUNTIME_LITTLE_ENDIAN 0
#elif defined(__LITTLE_ENDIAN__) || defined(__ARMEL__) || defined(__THUMBEL__) || \
      defined(__AARCH64EL__) || defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__) || \
      defined(_M_IX86) || defined(_M_X64) || defined(_M_IA64) || defined(_M_ARM) || \
      defined(__i386__) || defined(__x86_64__) || defined(__amd64__) || \
      defined(__WASM__) || defined(__wasm__) || defined(__EMSCRIPTEN__)
    #define PLCRUNTIME_LITTLE_ENDIAN 1
#elif defined(__BIG_ENDIAN__) || defined(__ARMEB__) || defined(__THUMBEB__) || \
      defined(__AARCH64EB__) || defined(_MIPSEB) || defined(__MIPSEB) || defined(__MIPSEB__)
    #define PLCRUNTIME_LITTLE_ENDIAN 0
#else
    // Default to little-endian (most common)
    #define PLCRUNTIME_LITTLE_ENDIAN 1
#endif

// System memory byte 0 (S0) stores endianness flag
// Bit 0: 1 = little-endian, 0 = big-endian
#define PLCRUNTIME_SYSTEM_ENDIAN_BYTE 0
#define PLCRUNTIME_SYSTEM_ENDIAN_BIT  0


// Pointer type for the runtime memory
typedef uint16_t  MY_PTR_t;
#define MY_PTR_SIZE 16
#define MY_PTR_SIZE_BYTES 2
#define MY_PTR_MAX ((MY_PTR_t)-1)  // Maximum value for MY_PTR_t (65535 for 16-bit, etc.)

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

// ============================================================================
// Unaligned Memory Access Helpers (Little-Endian)
// ============================================================================
// These functions read/write multi-byte values byte-by-byte to avoid
// unaligned memory access issues on platforms like ESP32-C3 (RISC-V)
// and some ARM cores. All values are stored in little-endian format.
// ============================================================================

// Write multi-byte values to buffer (little-endian, unaligned-safe)
inline void write_u16(u8* buf, u16 value) {
    buf[0] = (u8)(value & 0xFF);
    buf[1] = (u8)((value >> 8) & 0xFF);
}
inline void write_i16(u8* buf, i16 value) { write_u16(buf, (u16)value); }
inline void write_u32(u8* buf, u32 value) {
    buf[0] = (u8)(value & 0xFF);
    buf[1] = (u8)((value >> 8) & 0xFF);
    buf[2] = (u8)((value >> 16) & 0xFF);
    buf[3] = (u8)((value >> 24) & 0xFF);
}
inline void write_i32(u8* buf, i32 value) { write_u32(buf, (u32)value); }
inline void write_f32(u8* buf, f32 value) {
    union { f32 f; u32 u; } cvt;
    cvt.f = value;
    write_u32(buf, cvt.u);
}
inline void write_ptr(u8* buf, MY_PTR_t value) {
#if MY_PTR_SIZE_BYTES == 2
    write_u16(buf, (u16)value);
#else
    write_u32(buf, (u32)value);
#endif
}
#ifdef USE_X64_OPS
inline void write_u64(u8* buf, u64 value) {
    buf[0] = (u8)(value & 0xFF);
    buf[1] = (u8)((value >> 8) & 0xFF);
    buf[2] = (u8)((value >> 16) & 0xFF);
    buf[3] = (u8)((value >> 24) & 0xFF);
    buf[4] = (u8)((value >> 32) & 0xFF);
    buf[5] = (u8)((value >> 40) & 0xFF);
    buf[6] = (u8)((value >> 48) & 0xFF);
    buf[7] = (u8)((value >> 56) & 0xFF);
}
inline void write_i64(u8* buf, i64 value) { write_u64(buf, (u64)value); }
inline void write_f64(u8* buf, f64 value) {
    union { f64 f; u64 u; } cvt;
    cvt.f = value;
    write_u64(buf, cvt.u);
}
#endif // USE_X64_OPS

// Read multi-byte values from buffer (little-endian, unaligned-safe)
inline u16 read_u16(const u8* buf) {
    return (u16)buf[0] | ((u16)buf[1] << 8);
}
inline i16 read_i16(const u8* buf) { return (i16)read_u16(buf); }
inline u32 read_u32(const u8* buf) {
    return (u32)buf[0] | ((u32)buf[1] << 8) | ((u32)buf[2] << 16) | ((u32)buf[3] << 24);
}
inline i32 read_i32(const u8* buf) { return (i32)read_u32(buf); }
inline f32 read_f32(const u8* buf) {
    union { u32 u; f32 f; } cvt;
    cvt.u = read_u32(buf);
    return cvt.f;
}
inline MY_PTR_t read_ptr(const u8* buf) {
#if MY_PTR_SIZE_BYTES == 2
    return (MY_PTR_t)read_u16(buf);
#else
    return (MY_PTR_t)read_u32(buf);
#endif
}
#ifdef USE_X64_OPS
inline u64 read_u64(const u8* buf) {
    return (u64)buf[0] | ((u64)buf[1] << 8) | ((u64)buf[2] << 16) | ((u64)buf[3] << 24) |
           ((u64)buf[4] << 32) | ((u64)buf[5] << 40) | ((u64)buf[6] << 48) | ((u64)buf[7] << 56);
}
inline i64 read_i64(const u8* buf) { return (i64)read_u64(buf); }
inline f64 read_f64(const u8* buf) {
    union { u64 u; f64 f; } cvt;
    cvt.u = read_u64(buf);
    return cvt.f;
}
#endif // USE_X64_OPS