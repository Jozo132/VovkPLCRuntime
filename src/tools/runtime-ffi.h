// runtime-ffi.h - Foreign Function Interface for VovkPLCRuntime
//
// Copyright (c) 2024-2026 J.Vovk
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
//
// ============================================================================
// FFI System Architecture
// ============================================================================
//
// Enable with: #define PLCRUNTIME_FFI_ENABLED before including VovkPLCRuntime.h
//
// The FFI system allows PLC programs to call external functions (drivers,
// hardware interfaces, etc.) from PLCASM bytecode. Functions are registered
// with a name, signature, and description that can be queried by the editor.
//
// ============================================================================
// Simple Usage Example (Arduino)
// ============================================================================
//
// #define PLCRUNTIME_FFI_ENABLED
// #include <VovkPLCRuntime.h>
//
// // Just write normal functions!
// bool motor_goto(i32 position) {
//     // ... motor control logic ...
//     return true; // success
// }
//
// i32 motor_get_pos() {
//     return currentPosition;
// }
//
// void motor_stop() {
//     // ... stop motor ...
// }
//
// bool motor_config(u8 axis, u16 speed, u16 accel) {
//     // ... configure motor ...
//     return true;
// }
//
// VovkPLCRuntime runtime;
//
// void setup() {
//     runtime.registerFFI("F_motor_goto", "i32->bool", "Move to position", motor_goto);
//     runtime.registerFFI("F_motor_get_pos", "void->i32", "Get current position", motor_get_pos);
//     runtime.registerFFI("F_motor_stop", "void->void", "Stop motor", motor_stop);
//     runtime.registerFFI("F_motor_config", "u8,u16,u16->bool", "Configure motor", motor_config);
//     runtime.initialize();
// }
//
// ============================================================================
// Signature Format
// ============================================================================
//
// Signatures describe function parameters and return type:
//   "u8,u16->i32"     - Takes u8 and u16, returns i32
//   "u8,u16->void"    - Takes u8 and u16, returns nothing
//   "void->u32"       - Takes nothing, returns u32
//   "f32,f32,f32->f32" - Takes three f32, returns f32
//   "str8->void"      - Takes str8 string address, returns nothing
//   "str8,i32->str8"  - Takes str8 and i32, returns str8 address
//
// Supported types: void, bool, u8, i8, u16, i16, u32, i32, u64, i64, f32, f64, str8, str16
//
// ============================================================================
// PLCASM Usage
// ============================================================================
//
// ; Call FFI function by index with params from memory, result to memory
// ffi.call 0, 2, M10, M12, M20    ; Call FFI #0, 2 params at M10/M12, result to M20
//
// ; Call FFI with params from stack (pop N params, push result)
// i32.const 1000          ; position parameter
// ffi.call_stack 0, 1     ; call FFI #0 with 1 param from stack, push result
//
// ============================================================================

#pragma once

#include "runtime-types.h"
#include "runtime-instructions.h"

// Configuration
#ifndef PLCRUNTIME_MAX_FFI_FUNCTIONS
#define PLCRUNTIME_MAX_FFI_FUNCTIONS 32
#endif

#ifndef PLCRUNTIME_FFI_MAX_NAME_LEN
#define PLCRUNTIME_FFI_MAX_NAME_LEN 32
#endif

#ifndef PLCRUNTIME_FFI_MAX_SIG_LEN
#define PLCRUNTIME_FFI_MAX_SIG_LEN 32
#endif

#ifndef PLCRUNTIME_FFI_MAX_DESC_LEN
#define PLCRUNTIME_FFI_MAX_DESC_LEN 64
#endif

#ifndef PLCRUNTIME_FFI_MAX_PARAMS
#define PLCRUNTIME_FFI_MAX_PARAMS 8
#endif

// ============================================================================
// String Struct Types for FFI
// ============================================================================

// str8 - 8-bit capacity/length string (max 255 chars)
// Memory layout: [u8 capacity, u8 length, char data...]
struct str8 {
    u8* capacity;   // Pointer to capacity byte
    u8* length;     // Pointer to length byte
    char* data;     // Pointer to character data
    
    // Constructor from PLC memory address
    static str8 fromMemory(u8* memory, u16 addr) {
        return str8{ &memory[addr], &memory[addr + 1], (char*)&memory[addr + 2] };
    }
    
    // Get as null-terminated string (copies to provided buffer)
    void copyTo(char* buf, u8 maxLen) const {
        u8 len = *length;
        if (len > maxLen - 1) len = maxLen - 1;
        for (u8 i = 0; i < len; i++) buf[i] = data[i];
        buf[len] = '\0';
    }
};

// str16 - 16-bit capacity/length string (max 65535 chars)
// Memory layout: [u16 capacity, u16 length, char data...]
struct str16 {
    u16* capacity;  // Pointer to capacity (2 bytes)
    u16* length;    // Pointer to length (2 bytes)
    char* data;     // Pointer to character data
    
    // Constructor from PLC memory address
    static str16 fromMemory(u8* memory, u16 addr) {
        return str16{ (u16*)&memory[addr], (u16*)&memory[addr + 2], (char*)&memory[addr + 4] };
    }
    
    // Get as null-terminated string (copies to provided buffer)
    void copyTo(char* buf, u16 maxLen) const {
        u16 len = *length;
        if (len > maxLen - 1) len = maxLen - 1;
        for (u16 i = 0; i < len; i++) buf[i] = data[i];
        buf[len] = '\0';
    }
};

// ============================================================================
// Memory Marshalling Helpers
// ============================================================================

// Read typed value from PLC memory
template<typename T> inline T ffi_read(u8* memory, u16 addr);
template<> inline bool ffi_read<bool>(u8* memory, u16 addr) { return memory[addr] != 0; }
template<> inline u8 ffi_read<u8>(u8* memory, u16 addr) { return memory[addr]; }
template<> inline i8 ffi_read<i8>(u8* memory, u16 addr) { return (i8)memory[addr]; }
template<> inline u16 ffi_read<u16>(u8* memory, u16 addr) { return *(u16*)&memory[addr]; }
template<> inline i16 ffi_read<i16>(u8* memory, u16 addr) { return *(i16*)&memory[addr]; }
template<> inline u32 ffi_read<u32>(u8* memory, u16 addr) { return *(u32*)&memory[addr]; }
template<> inline i32 ffi_read<i32>(u8* memory, u16 addr) { return *(i32*)&memory[addr]; }
template<> inline u64 ffi_read<u64>(u8* memory, u16 addr) { return *(u64*)&memory[addr]; }
template<> inline i64 ffi_read<i64>(u8* memory, u16 addr) { return *(i64*)&memory[addr]; }
template<> inline f32 ffi_read<f32>(u8* memory, u16 addr) { return *(f32*)&memory[addr]; }
template<> inline f64 ffi_read<f64>(u8* memory, u16 addr) { return *(f64*)&memory[addr]; }
// String types: read u16 address value, return struct with direct pointers into memory
template<> inline str8 ffi_read<str8>(u8* memory, u16 addr) { 
    return str8::fromMemory(memory, *(u16*)&memory[addr]); 
}
template<> inline str16 ffi_read<str16>(u8* memory, u16 addr) { 
    return str16::fromMemory(memory, *(u16*)&memory[addr]); 
}

// Write typed value to PLC memory
template<typename T> inline void ffi_write(u8* memory, u16 addr, T value);
template<> inline void ffi_write<bool>(u8* memory, u16 addr, bool v) { memory[addr] = v ? 1 : 0; }
template<> inline void ffi_write<u8>(u8* memory, u16 addr, u8 v) { memory[addr] = v; }
template<> inline void ffi_write<i8>(u8* memory, u16 addr, i8 v) { memory[addr] = (u8)v; }
template<> inline void ffi_write<u16>(u8* memory, u16 addr, u16 v) { *(u16*)&memory[addr] = v; }
template<> inline void ffi_write<i16>(u8* memory, u16 addr, i16 v) { *(i16*)&memory[addr] = v; }
template<> inline void ffi_write<u32>(u8* memory, u16 addr, u32 v) { *(u32*)&memory[addr] = v; }
template<> inline void ffi_write<i32>(u8* memory, u16 addr, i32 v) { *(i32*)&memory[addr] = v; }
template<> inline void ffi_write<u64>(u8* memory, u16 addr, u64 v) { *(u64*)&memory[addr] = v; }
template<> inline void ffi_write<i64>(u8* memory, u16 addr, i64 v) { *(i64*)&memory[addr] = v; }
template<> inline void ffi_write<f32>(u8* memory, u16 addr, f32 v) { *(f32*)&memory[addr] = v; }
template<> inline void ffi_write<f64>(u8* memory, u16 addr, f64 v) { *(f64*)&memory[addr] = v; }
// String types: write the string address (u16) to memory - computed from capacity pointer
template<> inline void ffi_write<str8>(u8* memory, u16 addr, str8 v) { 
    *(u16*)&memory[addr] = (u16)(v.capacity - memory); 
}
template<> inline void ffi_write<str16>(u8* memory, u16 addr, str16 v) { 
    *(u16*)&memory[addr] = (u16)((u8*)v.capacity - memory); 
}

// ============================================================================
// Internal Handler Type (used for type-erased storage)
// ============================================================================

typedef RuntimeError (*PLCFFIInternalHandler)(u8* memory, u16* param_addrs, u8 param_count, u16 ret_addr, void* fn_ptr);

// Legacy handler type alias (for backward compatibility with WASM and advanced use)
// This is the raw handler signature for when you need direct memory access
typedef PLCFFIInternalHandler PLCFFIHandler;

// ============================================================================
// FFI Wrapper Templates - Generate marshalling code at compile time
// ============================================================================

// --- 0 parameters ---
template<typename Ret>
struct FFIWrapper0 {
    static RuntimeError call(u8* memory, u16* addrs, u8 cnt, u16 ret_addr, void* fn_ptr) {
        auto fn = reinterpret_cast<Ret(*)()>(fn_ptr);
        Ret result = fn();
        ffi_write<Ret>(memory, ret_addr, result);
        return STATUS_SUCCESS;
    }
};
template<>
struct FFIWrapper0<void> {
    static RuntimeError call(u8* memory, u16* addrs, u8 cnt, u16 ret_addr, void* fn_ptr) {
        auto fn = reinterpret_cast<void(*)()>(fn_ptr);
        fn();
        return STATUS_SUCCESS;
    }
};

// --- 1 parameter ---
template<typename Ret, typename A1>
struct FFIWrapper1 {
    static RuntimeError call(u8* memory, u16* addrs, u8 cnt, u16 ret_addr, void* fn_ptr) {
        auto fn = reinterpret_cast<Ret(*)(A1)>(fn_ptr);
        A1 a1 = ffi_read<A1>(memory, addrs[0]);
        Ret result = fn(a1);
        ffi_write<Ret>(memory, ret_addr, result);
        return STATUS_SUCCESS;
    }
};
template<typename A1>
struct FFIWrapper1<void, A1> {
    static RuntimeError call(u8* memory, u16* addrs, u8 cnt, u16 ret_addr, void* fn_ptr) {
        auto fn = reinterpret_cast<void(*)(A1)>(fn_ptr);
        A1 a1 = ffi_read<A1>(memory, addrs[0]);
        fn(a1);
        return STATUS_SUCCESS;
    }
};

// --- 2 parameters ---
template<typename Ret, typename A1, typename A2>
struct FFIWrapper2 {
    static RuntimeError call(u8* memory, u16* addrs, u8 cnt, u16 ret_addr, void* fn_ptr) {
        auto fn = reinterpret_cast<Ret(*)(A1, A2)>(fn_ptr);
        A1 a1 = ffi_read<A1>(memory, addrs[0]);
        A2 a2 = ffi_read<A2>(memory, addrs[1]);
        Ret result = fn(a1, a2);
        ffi_write<Ret>(memory, ret_addr, result);
        return STATUS_SUCCESS;
    }
};
template<typename A1, typename A2>
struct FFIWrapper2<void, A1, A2> {
    static RuntimeError call(u8* memory, u16* addrs, u8 cnt, u16 ret_addr, void* fn_ptr) {
        auto fn = reinterpret_cast<void(*)(A1, A2)>(fn_ptr);
        A1 a1 = ffi_read<A1>(memory, addrs[0]);
        A2 a2 = ffi_read<A2>(memory, addrs[1]);
        fn(a1, a2);
        return STATUS_SUCCESS;
    }
};

// --- 3 parameters ---
template<typename Ret, typename A1, typename A2, typename A3>
struct FFIWrapper3 {
    static RuntimeError call(u8* memory, u16* addrs, u8 cnt, u16 ret_addr, void* fn_ptr) {
        auto fn = reinterpret_cast<Ret(*)(A1, A2, A3)>(fn_ptr);
        A1 a1 = ffi_read<A1>(memory, addrs[0]);
        A2 a2 = ffi_read<A2>(memory, addrs[1]);
        A3 a3 = ffi_read<A3>(memory, addrs[2]);
        Ret result = fn(a1, a2, a3);
        ffi_write<Ret>(memory, ret_addr, result);
        return STATUS_SUCCESS;
    }
};
template<typename A1, typename A2, typename A3>
struct FFIWrapper3<void, A1, A2, A3> {
    static RuntimeError call(u8* memory, u16* addrs, u8 cnt, u16 ret_addr, void* fn_ptr) {
        auto fn = reinterpret_cast<void(*)(A1, A2, A3)>(fn_ptr);
        A1 a1 = ffi_read<A1>(memory, addrs[0]);
        A2 a2 = ffi_read<A2>(memory, addrs[1]);
        A3 a3 = ffi_read<A3>(memory, addrs[2]);
        fn(a1, a2, a3);
        return STATUS_SUCCESS;
    }
};

// --- 4 parameters ---
template<typename Ret, typename A1, typename A2, typename A3, typename A4>
struct FFIWrapper4 {
    static RuntimeError call(u8* memory, u16* addrs, u8 cnt, u16 ret_addr, void* fn_ptr) {
        auto fn = reinterpret_cast<Ret(*)(A1, A2, A3, A4)>(fn_ptr);
        A1 a1 = ffi_read<A1>(memory, addrs[0]);
        A2 a2 = ffi_read<A2>(memory, addrs[1]);
        A3 a3 = ffi_read<A3>(memory, addrs[2]);
        A4 a4 = ffi_read<A4>(memory, addrs[3]);
        Ret result = fn(a1, a2, a3, a4);
        ffi_write<Ret>(memory, ret_addr, result);
        return STATUS_SUCCESS;
    }
};
template<typename A1, typename A2, typename A3, typename A4>
struct FFIWrapper4<void, A1, A2, A3, A4> {
    static RuntimeError call(u8* memory, u16* addrs, u8 cnt, u16 ret_addr, void* fn_ptr) {
        auto fn = reinterpret_cast<void(*)(A1, A2, A3, A4)>(fn_ptr);
        A1 a1 = ffi_read<A1>(memory, addrs[0]);
        A2 a2 = ffi_read<A2>(memory, addrs[1]);
        A3 a3 = ffi_read<A3>(memory, addrs[2]);
        A4 a4 = ffi_read<A4>(memory, addrs[3]);
        fn(a1, a2, a3, a4);
        return STATUS_SUCCESS;
    }
};

// --- 5 parameters ---
template<typename Ret, typename A1, typename A2, typename A3, typename A4, typename A5>
struct FFIWrapper5 {
    static RuntimeError call(u8* memory, u16* addrs, u8 cnt, u16 ret_addr, void* fn_ptr) {
        auto fn = reinterpret_cast<Ret(*)(A1, A2, A3, A4, A5)>(fn_ptr);
        A1 a1 = ffi_read<A1>(memory, addrs[0]);
        A2 a2 = ffi_read<A2>(memory, addrs[1]);
        A3 a3 = ffi_read<A3>(memory, addrs[2]);
        A4 a4 = ffi_read<A4>(memory, addrs[3]);
        A5 a5 = ffi_read<A5>(memory, addrs[4]);
        Ret result = fn(a1, a2, a3, a4, a5);
        ffi_write<Ret>(memory, ret_addr, result);
        return STATUS_SUCCESS;
    }
};
template<typename A1, typename A2, typename A3, typename A4, typename A5>
struct FFIWrapper5<void, A1, A2, A3, A4, A5> {
    static RuntimeError call(u8* memory, u16* addrs, u8 cnt, u16 ret_addr, void* fn_ptr) {
        auto fn = reinterpret_cast<void(*)(A1, A2, A3, A4, A5)>(fn_ptr);
        A1 a1 = ffi_read<A1>(memory, addrs[0]);
        A2 a2 = ffi_read<A2>(memory, addrs[1]);
        A3 a3 = ffi_read<A3>(memory, addrs[2]);
        A4 a4 = ffi_read<A4>(memory, addrs[3]);
        A5 a5 = ffi_read<A5>(memory, addrs[4]);
        fn(a1, a2, a3, a4, a5);
        return STATUS_SUCCESS;
    }
};

// --- 6 parameters ---
template<typename Ret, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct FFIWrapper6 {
    static RuntimeError call(u8* memory, u16* addrs, u8 cnt, u16 ret_addr, void* fn_ptr) {
        auto fn = reinterpret_cast<Ret(*)(A1, A2, A3, A4, A5, A6)>(fn_ptr);
        A1 a1 = ffi_read<A1>(memory, addrs[0]);
        A2 a2 = ffi_read<A2>(memory, addrs[1]);
        A3 a3 = ffi_read<A3>(memory, addrs[2]);
        A4 a4 = ffi_read<A4>(memory, addrs[3]);
        A5 a5 = ffi_read<A5>(memory, addrs[4]);
        A6 a6 = ffi_read<A6>(memory, addrs[5]);
        Ret result = fn(a1, a2, a3, a4, a5, a6);
        ffi_write<Ret>(memory, ret_addr, result);
        return STATUS_SUCCESS;
    }
};
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct FFIWrapper6<void, A1, A2, A3, A4, A5, A6> {
    static RuntimeError call(u8* memory, u16* addrs, u8 cnt, u16 ret_addr, void* fn_ptr) {
        auto fn = reinterpret_cast<void(*)(A1, A2, A3, A4, A5, A6)>(fn_ptr);
        A1 a1 = ffi_read<A1>(memory, addrs[0]);
        A2 a2 = ffi_read<A2>(memory, addrs[1]);
        A3 a3 = ffi_read<A3>(memory, addrs[2]);
        A4 a4 = ffi_read<A4>(memory, addrs[3]);
        A5 a5 = ffi_read<A5>(memory, addrs[4]);
        A6 a6 = ffi_read<A6>(memory, addrs[5]);
        fn(a1, a2, a3, a4, a5, a6);
        return STATUS_SUCCESS;
    }
};

// ============================================================================
// Signature Parsing Helpers (for editor/transport API)
// ============================================================================

// Type codes: 0=void, 1=bool, 2=u8, 3=i8, 4=u16, 5=i16, 6=u32, 7=i32, 8=u64, 9=i64, 10=f32, 11=f64, 12=str8, 13=str16
inline u8 ffi_parseSignatureParams(const char* sig, u8* param_types, u8 max_params) {
    u8 count = 0;
    const char* p = sig;
    
    // Check for "void->"
    if (p[0] == 'v' && p[1] == 'o' && p[2] == 'i' && p[3] == 'd') {
        p += 4;
        if (*p == '-' && *(p+1) == '>') return 0;
    }
    
    while (*p && *p != '-' && count < max_params) {
        while (*p == ' ' || *p == ',') p++;
        if (*p == '-' || *p == '\0') break;
        
        if (p[0] == 'b' && p[1] == 'o' && p[2] == 'o' && p[3] == 'l') { param_types[count++] = 1; p += 4; }
        else if (p[0] == 's' && p[1] == 't' && p[2] == 'r' && p[3] == '1' && p[4] == '6') { param_types[count++] = 13; p += 5; }
        else if (p[0] == 's' && p[1] == 't' && p[2] == 'r' && p[3] == '8') { param_types[count++] = 12; p += 4; }
        else if (p[0] == 's' && p[1] == 't' && p[2] == 'r') { param_types[count++] = 12; p += 3; } // str alias for str8
        else if (p[0] == 'u' && p[1] == '8') { param_types[count++] = 2; p += 2; }
        else if (p[0] == 'i' && p[1] == '8') { param_types[count++] = 3; p += 2; }
        else if (p[0] == 'u' && p[1] == '1' && p[2] == '6') { param_types[count++] = 4; p += 3; }
        else if (p[0] == 'i' && p[1] == '1' && p[2] == '6') { param_types[count++] = 5; p += 3; }
        else if (p[0] == 'u' && p[1] == '3' && p[2] == '2') { param_types[count++] = 6; p += 3; }
        else if (p[0] == 'i' && p[1] == '3' && p[2] == '2') { param_types[count++] = 7; p += 3; }
        else if (p[0] == 'u' && p[1] == '6' && p[2] == '4') { param_types[count++] = 8; p += 3; }
        else if (p[0] == 'i' && p[1] == '6' && p[2] == '4') { param_types[count++] = 9; p += 3; }
        else if (p[0] == 'f' && p[1] == '3' && p[2] == '2') { param_types[count++] = 10; p += 3; }
        else if (p[0] == 'f' && p[1] == '6' && p[2] == '4') { param_types[count++] = 11; p += 3; }
        else { while (*p && *p != ',' && *p != '-') p++; }
    }
    return count;
}

inline u8 ffi_parseSignatureReturn(const char* sig) {
    const char* p = sig;
    while (*p) {
        if (*p == '-' && *(p+1) == '>') {
            p += 2;
            while (*p == ' ') p++;
            if (p[0] == 'v' && p[1] == 'o' && p[2] == 'i' && p[3] == 'd') return 0;
            if (p[0] == 'b' && p[1] == 'o' && p[2] == 'o' && p[3] == 'l') return 1;
            if (p[0] == 'u' && p[1] == '8') return 2;
            if (p[0] == 'i' && p[1] == '8') return 3;
            if (p[0] == 'u' && p[1] == '1' && p[2] == '6') return 4;
            if (p[0] == 'i' && p[1] == '1' && p[2] == '6') return 5;
            if (p[0] == 'u' && p[1] == '3' && p[2] == '2') return 6;
            if (p[0] == 'i' && p[1] == '3' && p[2] == '2') return 7;
            if (p[0] == 'u' && p[1] == '6' && p[2] == '4') return 8;
            if (p[0] == 'i' && p[1] == '6' && p[2] == '4') return 9;
            if (p[0] == 'f' && p[1] == '3' && p[2] == '2') return 10;
            if (p[0] == 'f' && p[1] == '6' && p[2] == '4') return 11;
            if (p[0] == 's' && p[1] == 't' && p[2] == 'r' && p[3] == '1' && p[4] == '6') return 13;
            if (p[0] == 's' && p[1] == 't' && p[2] == 'r' && p[3] == '8') return 12;
            if (p[0] == 's' && p[1] == 't' && p[2] == 'r') return 12; // str alias for str8
            return 0;
        }
        p++;
    }
    return 0;
}

inline u8 ffi_getTypeSize(u8 type_code) {
    switch (type_code) {
        case 0: return 0;   // void
        case 1: return 1;   // bool
        case 2: return 1;   // u8
        case 3: return 1;   // i8
        case 4: return 2;   // u16
        case 5: return 2;   // i16
        case 6: return 4;   // u32
        case 7: return 4;   // i32
        case 8: return 8;   // u64
        case 9: return 8;   // i64
        case 10: return 4;  // f32
        case 11: return 8;  // f64
        case 12: return 2;  // str8 (passed as u16 address)
        case 13: return 2;  // str16 (passed as u16 address)
        default: return 0;
    }
}

// ============================================================================
// FFI Entry Structure
// ============================================================================

struct PLCFFIEntry {
    char name[PLCRUNTIME_FFI_MAX_NAME_LEN];
    char signature[PLCRUNTIME_FFI_MAX_SIG_LEN];
    char description[PLCRUNTIME_FFI_MAX_DESC_LEN];
    PLCFFIInternalHandler wrapper;   // Generated wrapper function
    void* fn_ptr;                    // Original user function pointer
    bool active;
};

// ============================================================================
// Static Global FFI Entries (to survive WASM class reinitialization)
// ============================================================================
static PLCFFIEntry g_ffi_entries[PLCRUNTIME_MAX_FFI_FUNCTIONS] = {0};
static bool g_ffi_initialized = false;

// ============================================================================
// FFI Registry Class
// ============================================================================

class PLCFFIRegistry {
private:
    // Entries stored as static global to survive WASM reinitialization

    static void copyStr(char* dest, const char* src, u8 max_len) {
        u8 i = 0;
        while (src[i] && i < max_len - 1) { dest[i] = src[i]; i++; }
        dest[i] = '\0';
    }

    static bool strEqual(const char* a, const char* b) {
        while (*a && *b) { if (*a != *b) return false; a++; b++; }
        return *a == *b;
    }

public:
    PLCFFIRegistry() {
        // Only initialize once - static globals survive WASM reinitialization
        if (!g_ffi_initialized) {
            for (u8 i = 0; i < PLCRUNTIME_MAX_FFI_FUNCTIONS; i++) {
                g_ffi_entries[i].active = false;
                g_ffi_entries[i].wrapper = nullptr;
                g_ffi_entries[i].fn_ptr = nullptr;
                g_ffi_entries[i].name[0] = '\0';
            }
            g_ffi_initialized = true;
        }
    }

    // Internal registration (called by template helpers)
    i8 registerInternal(const char* name, const char* sig, const char* desc,
                        PLCFFIInternalHandler wrapper, void* fn_ptr) {
        // Check if already exists
        for (u8 i = 0; i < PLCRUNTIME_MAX_FFI_FUNCTIONS; i++) {
            if (g_ffi_entries[i].active && strEqual(g_ffi_entries[i].name, name)) {
                copyStr(g_ffi_entries[i].signature, sig, PLCRUNTIME_FFI_MAX_SIG_LEN);
                copyStr(g_ffi_entries[i].description, desc, PLCRUNTIME_FFI_MAX_DESC_LEN);
                g_ffi_entries[i].wrapper = wrapper;
                g_ffi_entries[i].fn_ptr = fn_ptr;
                return i;
            }
        }
        // Find empty slot
        for (u8 i = 0; i < PLCRUNTIME_MAX_FFI_FUNCTIONS; i++) {
            if (!g_ffi_entries[i].active) {
                copyStr(g_ffi_entries[i].name, name, PLCRUNTIME_FFI_MAX_NAME_LEN);
                copyStr(g_ffi_entries[i].signature, sig, PLCRUNTIME_FFI_MAX_SIG_LEN);
                copyStr(g_ffi_entries[i].description, desc, PLCRUNTIME_FFI_MAX_DESC_LEN);
                g_ffi_entries[i].wrapper = wrapper;
                g_ffi_entries[i].fn_ptr = fn_ptr;
                g_ffi_entries[i].active = true;
                return i;
            }
        }
        return -1;
    }

    // ========================================================================
    // Simple User-Facing Registration Templates
    // ========================================================================

    // 0 parameters
    template<typename Ret>
    i8 add(const char* name, const char* sig, const char* desc, Ret (*fn)()) {
        return registerInternal(name, sig, desc, FFIWrapper0<Ret>::call, (void*)fn);
    }

    // 1 parameter
    template<typename Ret, typename A1>
    i8 add(const char* name, const char* sig, const char* desc, Ret (*fn)(A1)) {
        return registerInternal(name, sig, desc, FFIWrapper1<Ret, A1>::call, (void*)fn);
    }

    // 2 parameters
    template<typename Ret, typename A1, typename A2>
    i8 add(const char* name, const char* sig, const char* desc, Ret (*fn)(A1, A2)) {
        return registerInternal(name, sig, desc, FFIWrapper2<Ret, A1, A2>::call, (void*)fn);
    }

    // 3 parameters
    template<typename Ret, typename A1, typename A2, typename A3>
    i8 add(const char* name, const char* sig, const char* desc, Ret (*fn)(A1, A2, A3)) {
        return registerInternal(name, sig, desc, FFIWrapper3<Ret, A1, A2, A3>::call, (void*)fn);
    }

    // 4 parameters
    template<typename Ret, typename A1, typename A2, typename A3, typename A4>
    i8 add(const char* name, const char* sig, const char* desc, Ret (*fn)(A1, A2, A3, A4)) {
        return registerInternal(name, sig, desc, FFIWrapper4<Ret, A1, A2, A3, A4>::call, (void*)fn);
    }

    // 5 parameters
    template<typename Ret, typename A1, typename A2, typename A3, typename A4, typename A5>
    i8 add(const char* name, const char* sig, const char* desc, Ret (*fn)(A1, A2, A3, A4, A5)) {
        return registerInternal(name, sig, desc, FFIWrapper5<Ret, A1, A2, A3, A4, A5>::call, (void*)fn);
    }

    // 6 parameters
    template<typename Ret, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    i8 add(const char* name, const char* sig, const char* desc, Ret (*fn)(A1, A2, A3, A4, A5, A6)) {
        return registerInternal(name, sig, desc, FFIWrapper6<Ret, A1, A2, A3, A4, A5, A6>::call, (void*)fn);
    }

    // ========================================================================
    // Registry Operations
    // ========================================================================

    // Legacy registration method (for raw handlers with memory access)
    // Used by WASM exports and advanced users who need direct memory access
    i8 registerFunction(const char* name, const char* sig, const char* desc,
                        PLCFFIHandler handler, void* user_data = nullptr) {
        return registerInternal(name, sig, desc, handler, user_data);
    }

    bool remove(u8 index) {
        if (index >= PLCRUNTIME_MAX_FFI_FUNCTIONS || !g_ffi_entries[index].active) return false;
        g_ffi_entries[index].active = false;
        g_ffi_entries[index].wrapper = nullptr;
        g_ffi_entries[index].fn_ptr = nullptr;
        g_ffi_entries[index].name[0] = '\0';
        return true;
    }

    // Alias for backward compatibility
    bool unregisterByIndex(u8 index) { return remove(index); }
    bool unregisterByName(const char* name) { return removeByName(name); }

    bool removeByName(const char* name) {
        i8 idx = findByName(name);
        return idx >= 0 ? remove(idx) : false;
    }

    i8 findByName(const char* name) const {
        for (u8 i = 0; i < PLCRUNTIME_MAX_FFI_FUNCTIONS; i++) {
            if (g_ffi_entries[i].active && strEqual(g_ffi_entries[i].name, name)) return i;
        }
        return -1;
    }

    const PLCFFIEntry* get(u8 index) const {
        if (index >= PLCRUNTIME_MAX_FFI_FUNCTIONS || !g_ffi_entries[index].active) return nullptr;
        return &g_ffi_entries[index];
    }

    // Alias for backward compatibility
    const PLCFFIEntry* getEntry(u8 index) const { return get(index); }

    u8 getCount() const {
        u8 c = 0;
        for (u8 i = 0; i < PLCRUNTIME_MAX_FFI_FUNCTIONS; i++) {
            if (g_ffi_entries[i].active) c++;
        }
        return c;
    }
    u8 getCapacity() const { return PLCRUNTIME_MAX_FFI_FUNCTIONS; }

    // Call FFI function by index
    RuntimeError call(u8 index, u8* memory, u16* param_addrs, u8 param_count, u16 ret_addr) {
        if (index >= PLCRUNTIME_MAX_FFI_FUNCTIONS) return EXECUTION_ERROR;
        if (!g_ffi_entries[index].active || !g_ffi_entries[index].wrapper) return EXECUTION_ERROR;
        return g_ffi_entries[index].wrapper(memory, param_addrs, param_count, ret_addr, g_ffi_entries[index].fn_ptr);
    }

    // Iterate all entries (for transport/serial API)
    template<typename Callback>
    void forEach(Callback cb) const {
        for (u8 i = 0; i < PLCRUNTIME_MAX_FFI_FUNCTIONS; i++) {
            if (g_ffi_entries[i].active) cb(i, g_ffi_entries[i]);
        }
    }

    void clear() {
        for (u8 i = 0; i < PLCRUNTIME_MAX_FFI_FUNCTIONS; i++) {
            g_ffi_entries[i].active = false;
            g_ffi_entries[i].wrapper = nullptr;
            g_ffi_entries[i].fn_ptr = nullptr;
            g_ffi_entries[i].name[0] = '\0';
        }
    }
};

// ============================================================================
// Global FFI Registry Instance (when FFI is enabled)
// ============================================================================

#ifdef PLCRUNTIME_FFI_ENABLED
static PLCFFIRegistry g_ffiRegistry;
#endif
