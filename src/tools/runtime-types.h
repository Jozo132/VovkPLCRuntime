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
// Feature Flags - Enable/disable runtime features for resource-constrained devices
// ============================================================================
// By default, all features are ENABLED. Define these macros BEFORE including
// VovkPLCRuntime.h to disable specific features and reduce code size.
//
// For Arduino Nano or similar resource-constrained devices, you might want:
//   #define PLCRUNTIME_NO_STRINGS       // Disable string operations (~4KB savings)
//   #define PLCRUNTIME_NO_COUNTERS      // Disable counter operations (~1KB savings)
//   #define PLCRUNTIME_NO_TIMERS        // Disable timer operations (~2KB savings)
//   #define PLCRUNTIME_NO_FFI           // Disable FFI function calls (~1KB savings)
//   #define PLCRUNTIME_NO_X64_OPS       // Disable 64-bit operations (~2KB savings)
//   #define PLCRUNTIME_NO_FLOAT_OPS     // Disable float (f32) operations (~3KB savings)
//   #define PLCRUNTIME_NO_TRANSPORT     // Disable transport system
//   #define PLCRUNTIME_NO_ADVANCED_MATH // Disable POW, SQRT, SIN, COS (~3KB from math libs)
//   #define PLCRUNTIME_NO_32BIT_OPS     // Only u8/i8/u16/i16 types (~2KB savings)
//   #define PLCRUNTIME_NO_CVT           // Disable type conversion (~1.5KB savings)
//   #define PLCRUNTIME_NO_STACK_OPS     // Disable SWAP, PICK, POKE (~1KB savings)
//   #define PLCRUNTIME_NO_BITWISE_OPS   // Disable bitwise AND/OR/XOR/NOT/SHIFT (~1KB savings)
//   #define PLCRUNTIME_NUMERIC_DEBUG    // Use numeric codes instead of string names (~2KB savings)
//
// Or use a preset:
//   #define PLCRUNTIME_MINIMAL  // Disables strings, FFI, x64, float - keeps timers/counters
//   #define PLCRUNTIME_TINY     // Also disables counters, transport
//   #define PLCRUNTIME_NANO     // For ATmega328P: maximum stripping (~18KB target)
//
// Feature status is reported in:
// - System memory byte 0-1 (S0-S1): Runtime feature flags (u16 little-endian)
// - Device info response: runtime_flags field (same bit positions)
// ============================================================================

// ============================================================================
// Preset Configurations
// ============================================================================

// PLCRUNTIME_MINIMAL - For severely resource-constrained devices (Arduino Nano, ATtiny)
// Disables: strings, FFI, 64-bit operations, float operations
// Keeps: timers, counters (essential for PLC operations)
#ifdef PLCRUNTIME_MINIMAL
    #ifndef PLCRUNTIME_NO_STRINGS
        #define PLCRUNTIME_NO_STRINGS
    #endif
    #ifndef PLCRUNTIME_NO_FFI
        #define PLCRUNTIME_NO_FFI
    #endif
    #ifndef PLCRUNTIME_NO_X64_OPS
        #define PLCRUNTIME_NO_X64_OPS
    #endif
    #ifndef PLCRUNTIME_NO_FLOAT_OPS
        #define PLCRUNTIME_NO_FLOAT_OPS
    #endif
#endif // PLCRUNTIME_MINIMAL

// PLCRUNTIME_TINY - For extremely constrained devices (absolute minimum)
// Disables: strings, FFI, 64-bit ops, float ops, counters
// Keeps: timers only
#ifdef PLCRUNTIME_TINY
    #ifndef PLCRUNTIME_NO_STRINGS
        #define PLCRUNTIME_NO_STRINGS
    #endif
    #ifndef PLCRUNTIME_NO_FFI
        #define PLCRUNTIME_NO_FFI
    #endif
    #ifndef PLCRUNTIME_NO_X64_OPS
        #define PLCRUNTIME_NO_X64_OPS
    #endif
    #ifndef PLCRUNTIME_NO_FLOAT_OPS
        #define PLCRUNTIME_NO_FLOAT_OPS
    #endif
    #ifndef PLCRUNTIME_NO_COUNTERS
        #define PLCRUNTIME_NO_COUNTERS
    #endif
    #ifndef PLCRUNTIME_NO_TRANSPORT
        #define PLCRUNTIME_NO_TRANSPORT
    #endif
#endif // PLCRUNTIME_TINY

// PLCRUNTIME_NANO - For ATmega328P and similar (32KB flash, aggressive stripping)
// Target: ~18KB flash footprint for basic PLC functionality
// Disables: strings, FFI, x64, float, counters, transport, advanced math, 32-bit ops, CVT, stack ops
// Keeps: timers (TON/TOF/TP), basic logic, 8/16-bit arithmetic, memory ops, jumps
#ifdef PLCRUNTIME_NANO
    #ifndef PLCRUNTIME_NO_STRINGS
        #define PLCRUNTIME_NO_STRINGS
    #endif
    #ifndef PLCRUNTIME_NO_FFI
        #define PLCRUNTIME_NO_FFI
    #endif
    #ifndef PLCRUNTIME_NO_X64_OPS
        #define PLCRUNTIME_NO_X64_OPS
    #endif
    #ifndef PLCRUNTIME_NO_FLOAT_OPS
        #define PLCRUNTIME_NO_FLOAT_OPS
    #endif
    #ifndef PLCRUNTIME_NO_COUNTERS
        #define PLCRUNTIME_NO_COUNTERS
    #endif
    #ifndef PLCRUNTIME_NO_TRANSPORT
        #define PLCRUNTIME_NO_TRANSPORT
    #endif
    #ifndef PLCRUNTIME_NO_ADVANCED_MATH
        #define PLCRUNTIME_NO_ADVANCED_MATH
    #endif
    #ifndef PLCRUNTIME_NO_32BIT_OPS
        #define PLCRUNTIME_NO_32BIT_OPS
    #endif
    #ifndef PLCRUNTIME_NO_CVT
        #define PLCRUNTIME_NO_CVT
    #endif
    #ifndef PLCRUNTIME_NO_STACK_OPS
        #define PLCRUNTIME_NO_STACK_OPS
    #endif
    #ifndef PLCRUNTIME_NO_BITWISE_OPS
        #define PLCRUNTIME_NO_BITWISE_OPS
    #endif
    #ifndef PLCRUNTIME_NUMERIC_DEBUG
        #define PLCRUNTIME_NUMERIC_DEBUG
    #endif
#endif // PLCRUNTIME_NANO

// ============================================================================
// Feature enable/disable macros (inverted for cleaner code)
// ============================================================================
#ifndef PLCRUNTIME_NO_STRINGS
    #define PLCRUNTIME_STRINGS_ENABLED
#endif

#ifndef PLCRUNTIME_NO_COUNTERS
    #define PLCRUNTIME_COUNTERS_ENABLED
#endif

#ifndef PLCRUNTIME_NO_TIMERS
    #define PLCRUNTIME_TIMERS_ENABLED
#endif

#ifndef PLCRUNTIME_NO_FFI
    #define PLCRUNTIME_FFI_ENABLED
#endif

#ifndef PLCRUNTIME_NO_FLOAT_OPS
    #define PLCRUNTIME_FLOAT_OPS_ENABLED
#endif

// Transport system (Serial, WiFi, Ethernet, etc.)
// Note: Also requires PLCRUNTIME_TRANSPORT to be defined for backwards compatibility
#ifndef PLCRUNTIME_NO_TRANSPORT
    #ifdef PLCRUNTIME_TRANSPORT
        #define PLCRUNTIME_TRANSPORT_ENABLED
    #endif
#else
    #undef PLCRUNTIME_TRANSPORT
#endif

// 64-bit operations can also be disabled via USE_X64_OPS (legacy)
#ifndef PLCRUNTIME_NO_X64_OPS
    #ifndef USE_X64_OPS
        #define USE_X64_OPS
    #endif
#else
    #undef USE_X64_OPS
#endif

// Advanced math operations (POW, SQRT, SIN, COS) - pulls in heavy math libraries
#ifndef PLCRUNTIME_NO_ADVANCED_MATH
    #define PLCRUNTIME_ADVANCED_MATH_ENABLED
#endif

// 32-bit integer operations (u32, i32)
#ifndef PLCRUNTIME_NO_32BIT_OPS
    #define PLCRUNTIME_32BIT_OPS_ENABLED
#endif

// Type conversion (CVT instruction)
#ifndef PLCRUNTIME_NO_CVT
    #define PLCRUNTIME_CVT_ENABLED
#endif

// Stack manipulation operations (SWAP, PICK, POKE)
#ifndef PLCRUNTIME_NO_STACK_OPS
    #define PLCRUNTIME_STACK_OPS_ENABLED
#endif

// Bitwise operations (AND, OR, XOR, NOT, LSHIFT, RSHIFT)
#ifndef PLCRUNTIME_NO_BITWISE_OPS
    #define PLCRUNTIME_BITWISE_OPS_ENABLED
#endif

// ============================================================================
// Runtime Feature Flags Bit Definitions
// ============================================================================
// These bits are used in system memory S0 and device info runtime_flags
// to indicate which features are available at runtime.
//
// Bit layout (16-bit flags):
//   Bit 0:  Endianness (1 = little-endian, 0 = big-endian)
//   Bit 1:  String operations enabled
//   Bit 2:  Counter operations enabled  
//   Bit 3:  Timer operations enabled
//   Bit 4:  FFI (Foreign Function Interface) enabled
//   Bit 5:  64-bit operations enabled
//   Bit 6:  Safe mode enabled (bounds checking)
//   Bit 7:  Transport system enabled (Serial/WiFi/Ethernet)
//   Bit 8:  Float (f32) operations enabled
//   Bit 9-15: Reserved for future use
// ============================================================================

#define PLCRUNTIME_FLAG_LITTLE_ENDIAN   0x0001  // Bit 0
#define PLCRUNTIME_FLAG_STRINGS         0x0002  // Bit 1
#define PLCRUNTIME_FLAG_COUNTERS        0x0004  // Bit 2
#define PLCRUNTIME_FLAG_TIMERS          0x0008  // Bit 3
#define PLCRUNTIME_FLAG_FFI             0x0010  // Bit 4
#define PLCRUNTIME_FLAG_X64_OPS         0x0020  // Bit 5
#define PLCRUNTIME_FLAG_SAFE_MODE       0x0040  // Bit 6
#define PLCRUNTIME_FLAG_TRANSPORT       0x0080  // Bit 7
#define PLCRUNTIME_FLAG_FLOAT_OPS       0x0100  // Bit 8

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

// ============================================================================
// System memory byte 0 (S0) stores runtime feature flags
// ============================================================================
// The flags byte in system memory allows external tools to detect which
// features are available at runtime. Same bit layout as device info flags.
#define PLCRUNTIME_SYSTEM_FLAGS_BYTE 0

// Legacy compatibility - endianness is now bit 0 of the flags byte
#define PLCRUNTIME_SYSTEM_ENDIAN_BYTE PLCRUNTIME_SYSTEM_FLAGS_BYTE
#define PLCRUNTIME_SYSTEM_ENDIAN_BIT  0

// Helper function to compute runtime flags (called at startup)
// Returns a u16 with all enabled feature flags
inline uint16_t plcruntime_get_feature_flags() {
    uint16_t flags = 0;
    
    // Bit 0: Endianness
    if (PLCRUNTIME_LITTLE_ENDIAN) flags |= PLCRUNTIME_FLAG_LITTLE_ENDIAN;
    
    // Bit 1: Strings
#ifdef PLCRUNTIME_STRINGS_ENABLED
    flags |= PLCRUNTIME_FLAG_STRINGS;
#endif

    // Bit 2: Counters
#ifdef PLCRUNTIME_COUNTERS_ENABLED
    flags |= PLCRUNTIME_FLAG_COUNTERS;
#endif

    // Bit 3: Timers
#ifdef PLCRUNTIME_TIMERS_ENABLED
    flags |= PLCRUNTIME_FLAG_TIMERS;
#endif

    // Bit 4: FFI
#ifdef PLCRUNTIME_FFI_ENABLED
    flags |= PLCRUNTIME_FLAG_FFI;
#endif

    // Bit 5: 64-bit operations
#ifdef USE_X64_OPS
    flags |= PLCRUNTIME_FLAG_X64_OPS;
#endif

    // Bit 6: Safe mode
#ifdef PLCRUNTIME_SAFE_MODE
    flags |= PLCRUNTIME_FLAG_SAFE_MODE;
#endif

    // Bit 7: Transport system
#ifdef PLCRUNTIME_TRANSPORT_ENABLED
    flags |= PLCRUNTIME_FLAG_TRANSPORT;
#endif

    // Bit 8: Float operations
#ifdef PLCRUNTIME_FLOAT_OPS_ENABLED
    flags |= PLCRUNTIME_FLAG_FLOAT_OPS;
#endif

    return flags;
}


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