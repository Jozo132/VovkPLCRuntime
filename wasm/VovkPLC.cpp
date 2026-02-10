// VovkPLC.cpp - 2022-12-11
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

#define __WASM_TIME__
#define __SIMULATOR__
#define __RUNTIME_DEBUG__
#define __RUNTIME_FULL_UNIT_TEST___
#define USE_X64_OPS
#define PLCRUNTIME_FFI_ENABLED

#define VOVKPLC_DEVICE_NAME "Simulator"

#include "../src/VovkPLCRuntime.h"

#ifndef WASM_EXPORT
#define WASM_EXPORT
#endif // WASM_EXPORT

bool startup = true;

void custom_test() {
    // Blink the LED to indicate that the tests are done
    digitalWrite(LED_BUILTIN, LOW);
    delay(2);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);

    // Custom program test
    if (startup) {
        runtime.initialize();
        Serial.println(F("Custom program test"));
        Serial.println(F("Variables  = { a: 1, b: 2, c: 3, d: 4, e: 5, f: 6 }"));
        Serial.println(F("Function   = 10 * (1 - 'a' * ('b' + 'c' * ('c' + 'd' * ('d'-'e' *('e'-'f')))) / 'd')"));
        Serial.println(F("RPN        = 10|1|'a'|'b'|'c'|'c'|'d'|'d'|'e'|'e'|'f'|-|*|-|*|+|*|+|'d'|/|*|-|*"));
        Serial.println(F("Result     = -287.5"));
        Serial.println();

        float a = 1; float b = 2; float c = 3; float d = 4; float e = 5; float f = 6;
        float expected = 10 * (1 - a * (b + c * (c + d * (d - e * (e - f)))) / d);


        // Hand-coded RPN instructions:
        runtime.program.format();
        runtime.program.push_f32(10);
        runtime.program.push_f32(1);
        runtime.program.push_f32(a);
        runtime.program.push_f32(b);
        runtime.program.push_f32(c);
        runtime.program.push_f32(c);
        runtime.program.push_f32(d);
        runtime.program.push_f32(d);
        runtime.program.push_f32(e);
        runtime.program.push_f32(e);
        runtime.program.push_f32(f);
        runtime.program.push(SUB, type_f32);
        runtime.program.push(MUL, type_f32);
        runtime.program.push(SUB, type_f32);
        runtime.program.push(MUL, type_f32);
        runtime.program.push(ADD, type_f32);
        runtime.program.push(MUL, type_f32);
        runtime.program.push(ADD, type_f32);
        runtime.program.push_f32(d);
        runtime.program.push(DIV, type_f32);
        runtime.program.push(MUL, type_f32);
        runtime.program.push(SUB, type_f32);
        runtime.program.push(MUL, type_f32);


        /*
          // Compiled RPN bytecode:
          static const u8 bytecode [] = { 0x0A,0x41,0x20,0x00,0x00,0x0A,0x3F,0x80,0x00,0x00,0x0A,0x3F,0x80,0x00,0x00,0x0A,0x40,0x00,0x00,0x00,0x0A,0x40,0x40,0x00,0x00,0x0A,0x40,0x40,0x00,0x00,0x0A,0x40,0x80,0x00,0x00,0x0A,0x40,0x80,0x00,0x00,0x0A,0x40,0xA0,0x00,0x00,0x0A,0x40,0xA0,0x00,0x00,0x0A,0x40,0xC0,0x00,0x00,0x21,0x0A,0x22,0x0A,0x21,0x0A,0x22,0x0A,0x20,0x0A,0x22,0x0A,0x20,0x0A,0x0A,0x40,0x80,0x00,0x00,0x23,0x0A,0x22,0x0A,0x21,0x0A,0x22,0x0A };
          static const u16 size = 82;
          static const u32 checksum = 2677;
          runtime.loadProgram(bytecode, size, checksum);
          // runtime.loadProgramUnsafe(bytecode, size);
        */

        runtime.clear();

        RuntimeError status = UnitTest::fullProgramDebug(runtime);

        float output = runtime.read<float>();

        const char* status_name = RUNTIME_ERROR_NAME(status);
        Serial.print(F("Runtime status: ")); Serial.println(status_name);

        Serial.print(F("Result: ")); Serial.println(output);
        Serial.print(F("Expected result: ")); Serial.println(expected);
        Serial.print(F("Test passed: ")); Serial.println(expected == output ? F("YES") : F("NO - TEST DID NOT PASS !!!"));
        Serial.println();
        startup = false;
    }

    const int cycles = 1000;
    float result = -1;
    long t = micros();
    for (int i = 0; i < cycles; i++) {
        runtime.run();
        result = runtime.read<float>();
    }
    t = micros() - t;
    float ms = t / 1000.0;
    int mem = freeMemory();
    Serial.print(F("Program executed for ")); Serial.print(cycles); Serial.print(F(" cycles in ")); Serial.print(ms, 3); Serial.print(F(" ms. Result: ")); Serial.print(result); Serial.print(F("  Free memory: ")); Serial.print(mem); Serial.println(F(" bytes."));
}

WASM_EXPORT int run() {
    IntervalGlobalLoopCheck();
    return runtime.run();
}

WASM_EXPORT int runDirty() {
    IntervalGlobalLoopCheck();
    return runtime.runDirty();
}

WASM_EXPORT int runExplain() {
    IntervalGlobalLoopCheck();
    return UnitTest::fullProgramDebug(runtime);
}

WASM_EXPORT void run_unit_test() {
    IntervalGlobalLoopCheck();
    runtime_unit_test(runtime);
}

WASM_EXPORT void run_custom_test() {
    custom_test();
}

WASM_EXPORT void doNothing() {
    // Do nothing
}

WASM_EXPORT void doSomething() {
    runtime_unit_test(runtime);
    custom_test();
}

// ============================================================================
// Static String Buffers for JS Interop (no dynamic allocation)
// ============================================================================
// These are reusable static buffers for passing strings from JS to WASM.
// JS writes to these buffers, then calls functions that read from them.
// This avoids memory leaks from dynamic string allocation.

#define FFI_STRING_BUFFER_SIZE 128
#define FFI_STRING_BUFFER_COUNT 4

static char ffi_string_buffers[FFI_STRING_BUFFER_COUNT][FFI_STRING_BUFFER_SIZE] = {{}};

// Get pointer to a string buffer (0-3)
WASM_EXPORT u32 ffi_getStringBuffer(u8 index) {
    if (index >= FFI_STRING_BUFFER_COUNT) return 0;
    return (u32)ffi_string_buffers[index];
}

// Get string buffer size
WASM_EXPORT u32 ffi_getStringBufferSize() {
    return FFI_STRING_BUFFER_SIZE;
}

// Clear a string buffer
WASM_EXPORT void ffi_clearStringBuffer(u8 index) {
    if (index >= FFI_STRING_BUFFER_COUNT) return;
    memset(ffi_string_buffers[index], 0, FFI_STRING_BUFFER_SIZE);
}

// ============================================================================
// Hex Download Buffer
// ============================================================================
// Static buffer for downloadProgramHex - JS writes hex string here,
// then calls downloadProgramHex with the pointer. Max program = 64535 bytes
// = 129070 hex chars + null terminator.
#define HEX_DOWNLOAD_BUFFER_SIZE (64535 * 2 + 1)
static char hex_download_buffer[HEX_DOWNLOAD_BUFFER_SIZE] = {};

WASM_EXPORT u32 getHexDownloadBuffer() {
    return (u32)hex_download_buffer;
}

WASM_EXPORT u32 getHexDownloadBufferSize() {
    return HEX_DOWNLOAD_BUFFER_SIZE;
}

WASM_EXPORT u32 getMemoryLocation() {
    return (u32) runtime.memory;
}

WASM_EXPORT u32 getLastInstructionCount() {
    return runtime.last_instruction_count;
}

WASM_EXPORT u32 getMillis() {
    return millis();
}

WASM_EXPORT u32 getMicros() {
    return micros();
}

WASM_EXPORT void setMillis(u32 ms) {
    // const millis = () => +performance.now().toFixed(0);
    // millis is imported from JS and cannot be set
}

WASM_EXPORT void setMicros(u32 us) {
    // const micros = () => +(performance.now() * 1000).toFixed(0);
    // micros is imported from JS and cannot be set
}

// Get pointer to device health structure (efficient single-call access to all stats)
WASM_EXPORT u32 getDeviceHealthPtr() {
    static DeviceHealth health;
    runtime.getDeviceHealth(health);
    return (u32) &health;
}

// Legacy individual getters for backward compatibility
WASM_EXPORT u32 getLastCycleTimeUs() {
    return runtime.getLastCycleTimeUs();
}

WASM_EXPORT u32 getMinCycleTimeUs() {
    return runtime.getMinCycleTimeUs();
}

WASM_EXPORT u32 getMaxCycleTimeUs() {
    return runtime.getMaxCycleTimeUs();
}

WASM_EXPORT u32 getRamFree() {
    return runtime.getRamFree();
}

WASM_EXPORT u32 getMinRamFree() {
    return runtime.getMinRamFree();
}

WASM_EXPORT u32 getMaxRamFree() {
    return runtime.getMaxRamFree();
}

WASM_EXPORT u32 getLastPeriodUs() {
    return runtime.getLastPeriodUs();
}

WASM_EXPORT u32 getMinPeriodUs() {
    return runtime.getMinPeriodUs();
}

WASM_EXPORT u32 getMaxPeriodUs() {
    return runtime.getMaxPeriodUs();
}

WASM_EXPORT u32 getLastJitterUs() {
    return runtime.getLastJitterUs();
}

WASM_EXPORT u32 getMinJitterUs() {
    return runtime.getMinJitterUs();
}

WASM_EXPORT u32 getMaxJitterUs() {
    return runtime.getMaxJitterUs();
}

WASM_EXPORT u32 getTotalRam() {
    return runtime.getTotalRam();
}

WASM_EXPORT void resetDeviceHealth() {
    runtime.resetDeviceHealth();
}

WASM_EXPORT u32 getStackSize() {
    return runtime.stack.size();
}

WASM_EXPORT void clearStack() {
    runtime.clear();
}


// template <typename T> struct OutputBuffer {
//     T* data = nullptr;
//     u32 type_size = sizeof(T);
//     u32 length = 0;
//     u32 max_size = 0;
//     OutputBuffer(u32 size = 0) { init(size); }
//     void init(u32 size = 0) {
//         if (!size) return;
//         if (data == nullptr) {
//             data = new T[size];
//             length = 0;
//             max_size = size;
//         }
//     }
//     void push(T value) {
//         if (length < max_size) {
//             data[length++] = value;
//         }
//     }
//     T pop() {
//         if (length > 0) {
//             return data[--length];
//         }
//         return 0;
//     }
//     T shift() {
//         if (length > 0) {
//             T value = data[0];
//             for (u32 i = 1; i < length; i++) {
//                 data[i - 1] = data[i];
//             }
//             length--;
//             return value;
//         }
//         return 0;
//     }
//     // Array operator getter
//     T& operator[](u32 index) {
//         if (index < length)
//             return data[index];
//         return data[0];
//     }
//     // Array operator setter (avoid overloaded 'operator[]' cannot have more than one parameter before C++2b)
//     void set(u32 index, T value) {
//         if (index < length)
//             data[index] = value;
//     }

// };

// OutputBuffer<unsigned char> myBuffer(10);

// int export_request_count = 0;
// WASM_EXPORT void* export_output() {
//     export_request_count++;
//     if (myBuffer.length > 9)
//         myBuffer.shift();
//     myBuffer.push(export_request_count);
//     //Print buffer contents
//     Serial.print(F("Buffer <"));
//     for (u32 i = 0; i < myBuffer.length; i++) {
//         Serial.print(myBuffer[i], HEX);
//         if (i < myBuffer.length - 1) Serial.print(F(", "));
//     }
//     Serial.println(F(">"));
//     return (void*) (&myBuffer);
// }



// // Export program definition API
// WASM_EXPORT int API_resizeProgram(int new_size) {
//     initRuntime();
//     runtime.program.format(new_size);
//     return 0;
// }
// WASM_EXPORT int API_getProgramSize() {
//     initRuntime();
//     return runtime.program.size();
// }
// WASM_EXPORT int API_getProgramSizeMax() {
//     initRuntime();
//     return runtime.program.MAX_PROGRAM_SIZE;
// }
// WASM_EXPORT void API_printProgram() {
//     initRuntime();
//     runtime.program.println();
// }

// WASM_EXPORT void printErrorMessage() {
//     Error.println("Error: This is a test error message.");
// }
// WASM_EXPORT void printStreamTest() {
//     Stream.print("{ \"test\": \"This is a test stream message.\" }");
// }

WASM_EXPORT int project_loadToRuntime() {
    return (int)runtime.program.load(project_compiler.getBytecode(), project_compiler.getBytecodeLength(), project_compiler.getChecksum());
}

WASM_EXPORT void memoryReset() {
    runtime.formatMemory();
}

WASM_EXPORT void resetFirstCycle() {
    runtime.resetFirstCycle();
}

// ============================================================================
// DataBlock WASM Exports
// ============================================================================

WASM_EXPORT u16 db_getSlotCount() {
    return runtime.dataBlocks.num_slots;
}

WASM_EXPORT u16 db_getTableOffset() {
    return runtime.dataBlocks.table_offset;
}

WASM_EXPORT u16 db_getActiveCount() {
    return runtime.dataBlocks.activeCount();
}

WASM_EXPORT u16 db_getFreeSpace() {
    return runtime.dataBlocks.freeSpace();
}

// Get entry data by slot index. Returns db_number (0 = unused).
// offset and size are written to out_offset and out_size pointers.
WASM_EXPORT u16 db_getEntryDB(u16 slot) {
    u16 db, offset, size;
    if (!runtime.dataBlocks.getEntry(slot, db, offset, size)) return 0;
    return db;
}

WASM_EXPORT u16 db_getEntryOffset(u16 slot) {
    u16 db, offset, size;
    if (!runtime.dataBlocks.getEntry(slot, db, offset, size)) return 0;
    return offset;
}

WASM_EXPORT u16 db_getEntrySize(u16 slot) {
    u16 db, offset, size;
    if (!runtime.dataBlocks.getEntry(slot, db, offset, size)) return 0;
    return size;
}

// Declare a new DataBlock. Returns slot index or -1 on failure.
WASM_EXPORT i16 db_declare(u16 db_number, u16 size) {
    return runtime.dataBlocks.declare(db_number, size);
}

// Remove a DataBlock by DB number. Returns 1 on success, 0 on failure.
WASM_EXPORT u8 db_remove(u16 db_number) {
    return runtime.dataBlocks.remove(db_number) ? 1 : 0;
}

// Migrate a DataBlock to a new offset. Returns 1 on success, 0 on failure.
WASM_EXPORT u8 db_migrate(u16 db_number, u16 target_offset) {
    return runtime.dataBlocks.migrate(db_number, target_offset) ? 1 : 0;
}

// Compact all DataBlocks. Returns the new lowest allocated address.
WASM_EXPORT u16 db_compact() {
    return runtime.dataBlocks.compact();
}

// Resolve a DB-relative address to an absolute address. Returns 0xFFFF on error.
WASM_EXPORT u16 db_resolveAddress(u16 db_number, u16 db_offset) {
    return runtime.dataBlocks.resolveAddress(db_number, db_offset);
}

// Format (clear) all DataBlock entries.
WASM_EXPORT void db_format() {
    runtime.dataBlocks.format();
}

// Set the number of DB slots (re-initializes the DB manager).
// Must be called before any db_declare calls.
// This allows the WASM compiler to configure the DB slot count
// to match the target embedded device's configuration.
WASM_EXPORT void db_setSlotCount(u16 count) {
    if (count == 0 || count > 256) return; // sanity check
    u16 db_area_end = (u16)(runtime.counter_offset + PLCRUNTIME_NUM_OF_COUNTERS * PLCRUNTIME_COUNTER_STRUCT_SIZE);
    runtime.dataBlocks.init(runtime.memory, PLCRUNTIME_MAX_MEMORY_SIZE, count, db_area_end);
    runtime.dataBlocks.format();
}

// ============================================================================
// FFI (Foreign Function Interface) WASM Exports
// ============================================================================

// ============================================================================
// JS FFI Import - Called FROM WASM to invoke a JS-registered function
// ============================================================================
// This function is IMPORTED from JavaScript and called when the runtime
// needs to execute a JS-registered FFI function.
//
// Parameters:
//   ffi_index: Index of the FFI entry in the registry
//   param_types_ptr: Pointer to array of parameter type codes
//   param_addrs_ptr: Pointer to array of parameter memory addresses
//   param_count: Number of parameters
//   ret_addr: Memory address to write return value
//   ret_type: Type code for return value
//
// Returns: 0 on success, non-zero on error
// ============================================================================
extern "C" {
    // WASM import - implemented in JavaScript
    __attribute__((import_module("env"), import_name("js_ffi_invoke")))
    RuntimeError js_ffi_invoke(u8 ffi_index, u8* param_types, u16* param_addrs, 
                                u8 param_count, u16 ret_addr, u8 ret_type);
}

// Flag to track which FFI entries are JS-backed (bit field, max 64 entries)
static u64 js_ffi_flags = 0;

// Internal handler that routes FFI calls to JavaScript via the import
RuntimeError js_ffi_router(u8* memory, u16* param_addrs, u8 param_count, u16 ret_addr, void* user_data) {
    (void)memory; // Memory is accessed via runtime's global memory in JS
    
    u8 ffi_index = (u8)(uintptr_t)user_data;
    const PLCFFIEntry* entry = runtime.getFFI(ffi_index);
    if (!entry) return FFI_NOT_FOUND;
    
    // Parse signature to get parameter types and return type
    const char* sig = entry->signature;
    u8 param_types[8] = {0};
    u8 ret_type = 0;
    u8 p_idx = 0;
    
    // Helper lambda-like function to match type prefix
    auto matchType = [](const char* s, const char* t, int len) -> bool {
        for (int i = 0; i < len; i++) {
            if (s[i] != t[i]) return false;
        }
        return true;
    };
    
    // Parse parameter types (before "->")
    const char* ptr = sig;
    while (*ptr && !(ptr[0] == '-' && ptr[1] == '>')) {
        if (*ptr == ',') { ptr++; continue; }
        
        // Match type names
        if (matchType(ptr, "void", 4)) { param_types[p_idx++] = 0; ptr += 4; }
        else if (matchType(ptr, "bool", 4)) { param_types[p_idx++] = 1; ptr += 4; }
        else if (matchType(ptr, "u8", 2) && ptr[2] != '6') { param_types[p_idx++] = 2; ptr += 2; }
        else if (matchType(ptr, "i8", 2)) { param_types[p_idx++] = 3; ptr += 2; }
        else if (matchType(ptr, "u16", 3)) { param_types[p_idx++] = 4; ptr += 3; }
        else if (matchType(ptr, "i16", 3)) { param_types[p_idx++] = 5; ptr += 3; }
        else if (matchType(ptr, "u32", 3)) { param_types[p_idx++] = 6; ptr += 3; }
        else if (matchType(ptr, "i32", 3)) { param_types[p_idx++] = 7; ptr += 3; }
        else if (matchType(ptr, "u64", 3)) { param_types[p_idx++] = 8; ptr += 3; }
        else if (matchType(ptr, "i64", 3)) { param_types[p_idx++] = 9; ptr += 3; }
        else if (matchType(ptr, "f32", 3)) { param_types[p_idx++] = 10; ptr += 3; }
        else if (matchType(ptr, "f64", 3)) { param_types[p_idx++] = 11; ptr += 3; }
        else ptr++;
    }
    
    // Skip "->"
    if (ptr[0] == '-' && ptr[1] == '>') ptr += 2;
    
    // Parse return type
    if (matchType(ptr, "void", 4)) ret_type = 0;
    else if (matchType(ptr, "bool", 4)) ret_type = 1;
    else if (matchType(ptr, "u8", 2) && ptr[2] != '6') ret_type = 2;
    else if (matchType(ptr, "i8", 2)) ret_type = 3;
    else if (matchType(ptr, "u16", 3)) ret_type = 4;
    else if (matchType(ptr, "i16", 3)) ret_type = 5;
    else if (matchType(ptr, "u32", 3)) ret_type = 6;
    else if (matchType(ptr, "i32", 3)) ret_type = 7;
    else if (matchType(ptr, "u64", 3)) ret_type = 8;
    else if (matchType(ptr, "i64", 3)) ret_type = 9;
    else if (matchType(ptr, "f32", 3)) ret_type = 10;
    else if (matchType(ptr, "f64", 3)) ret_type = 11;
    
    // Call the JS import
    return js_ffi_invoke(ffi_index, param_types, param_addrs, param_count, ret_addr, ret_type);
}

// Register a JS-backed FFI function
// This creates an entry that routes through js_ffi_router -> js_ffi_invoke (JS import)
WASM_EXPORT i32 ffi_registerJS(u32 name_ptr, u32 signature_ptr, u32 description_ptr) {
    const char* name = (const char*)name_ptr;
    const char* signature = (const char*)signature_ptr;
    const char* description = (const char*)description_ptr;
    
    // Get the index this entry will have
    u8 next_index = runtime.getFFICount();
    
    // Register with js_ffi_router as the handler, index stored in user_data
    i8 result = runtime.registerFFI(name, signature, description, 
                                     js_ffi_router, (void*)(uintptr_t)next_index);
    
    if (result >= 0 && result < 64) {
        // Mark this index as JS-backed
        js_ffi_flags |= (1ULL << result);
    }
    
    return result;
}

// Check if an FFI entry is JS-backed
WASM_EXPORT u8 ffi_isJSBacked(u8 index) {
    if (index >= 64) return 0;
    return (js_ffi_flags & (1ULL << index)) ? 1 : 0;
}

// ============================================================================
// Legacy JS Callback System (for backward compatibility)
// ============================================================================

// JS callback function pointer - set by JS to receive FFI calls
typedef RuntimeError (*JSFFICallback)(u8 ffi_index, u8* memory, u16* param_addrs, u8 param_count, u16 ret_addr);
static JSFFICallback js_ffi_callback = nullptr;

// FFI handler that routes to JavaScript
RuntimeError js_ffi_handler(u8* memory, u16* param_addrs, u8 param_count, u16 ret_addr, void* user_data) {
    if (!js_ffi_callback) return FFI_EXECUTION_ERROR;
    u8 ffi_index = (u8)(uintptr_t)user_data;
    return js_ffi_callback(ffi_index, memory, param_addrs, param_count, ret_addr);
}

// Set the JS callback function
WASM_EXPORT void ffi_setJSCallback(JSFFICallback callback) {
    js_ffi_callback = callback;
}

// Register an FFI function (returns index or -1 on failure)
// For JS-backed FFI, handler will route through js_ffi_callback
WASM_EXPORT i32 ffi_register(u32 name_ptr, u32 signature_ptr, u32 description_ptr, u8 use_js_callback) {
    const char* name = (const char*)name_ptr;
    const char* signature = (const char*)signature_ptr;
    const char* description = (const char*)description_ptr;
    
    PLCFFIHandler handler = nullptr;
    void* user_data = nullptr;
    
    if (use_js_callback) {
        handler = js_ffi_handler;
        // Store the current count as the index for this handler
        user_data = (void*)(uintptr_t)g_ffiRegistry.getCount();
    }
    
    return runtime.registerFFI(name, signature, description, handler, user_data);
}

// Unregister an FFI function by index
WASM_EXPORT u8 ffi_unregister(u8 index) {
    return runtime.unregisterFFI(index) ? 1 : 0;
}

// Get FFI count
WASM_EXPORT u8 ffi_getCount() {
    return runtime.getFFICount();
}

// Get FFI capacity
WASM_EXPORT u8 ffi_getCapacity() {
    return runtime.getFFICapacity();
}

// Get FFI entry name (returns pointer to string)
WASM_EXPORT u32 ffi_getName(u8 index) {
    const PLCFFIEntry* entry = runtime.getFFI(index);
    if (!entry) return 0;
    return (u32)entry->name;
}

// Get FFI entry signature (returns pointer to string)
WASM_EXPORT u32 ffi_getSignature(u8 index) {
    const PLCFFIEntry* entry = runtime.getFFI(index);
    if (!entry) return 0;
    return (u32)entry->signature;
}

// Get FFI entry description (returns pointer to string)
WASM_EXPORT u32 ffi_getDescription(u8 index) {
    const PLCFFIEntry* entry = runtime.getFFI(index);
    if (!entry) return 0;
    return (u32)entry->description;
}

// Clear all FFI registrations
WASM_EXPORT void ffi_clear() {
    runtime.clearFFI();
}

// Find FFI by name (returns index or -1)
WASM_EXPORT i32 ffi_findByName(u32 name_ptr) {
    const char* name = (const char*)name_ptr;
    return runtime.findFFI(name);
}

// ============================================================================
// Demo FFI Functions for Testing
// ============================================================================
// These demos showcase the SIMPLE FFI API - just write normal C++ functions
// and register them. The runtime handles all memory marshalling automatically.
// ============================================================================

// Demo: Add two i32 values - simple function signature!
i32 demo_add_i32(i32 a, i32 b) {
    return a + b;
}

// Demo: Multiply two f32 values
f32 demo_mul_f32(f32 a, f32 b) {
    return a * b;
}

// Demo: Motor-like function - clamp position within bounds
// This shows a 3-parameter function
i32 demo_motor_clamp(i32 pos, i32 min_val, i32 max_val) {
    i32 result = pos;
    if (result < min_val) result = min_val;
    if (result > max_val) result = max_val;
    return result;
}

// Demo: Increment counter - takes and returns u16
u16 demo_increment(u16 value) {
    return value + 1;
}

// Demo: Boolean function - check if in range
bool demo_in_range(i32 value, i32 min_val, i32 max_val) {
    return value >= min_val && value <= max_val;
}

// Demo: Void function - doesn't return anything (but still can take params)
// This shows void return type handling
static i32 demo_counter = 0;
void demo_void_increment() {
    demo_counter++;
}

// Demo: Get the counter value (no parameters, returns i32)
i32 demo_get_counter() {
    return demo_counter;
}

// Register demo FFI functions using the SIMPLE API
// Note how we just pass the function pointer - no wrapper needed!
WASM_EXPORT void ffi_registerDemos() {
    // 2-parameter functions
    runtime.registerFFI("F_add_i32", "i32,i32->i32", "Add two i32 values", demo_add_i32);
    runtime.registerFFI("F_mul_f32", "f32,f32->f32", "Multiply two f32 values", demo_mul_f32);
    
    // 3-parameter function
    runtime.registerFFI("F_motor_clamp", "i32,i32,i32->i32", "Clamp position within bounds", demo_motor_clamp);
    
    // 1-parameter function
    runtime.registerFFI("F_increment", "u16->u16", "Increment a u16 value", demo_increment);
    
    // Boolean return type
    runtime.registerFFI("F_in_range", "i32,i32,i32->bool", "Check if value is in range", demo_in_range);
    
    // Void return type (0 parameters)
    runtime.registerFFI("F_void_inc", "void->void", "Increment internal counter", demo_void_increment);
    
    // No parameters, returns value
    runtime.registerFFI("F_get_counter", "void->i32", "Get internal counter value", demo_get_counter);
}

// ============================================================================
// Legacy Demo: Raw Handler API
// ============================================================================
// This shows the RAW handler API for advanced users who need direct memory access.
// Most users should use the simple API above instead.
// ============================================================================

RuntimeError demo_raw_handler(u8* memory, u16* param_addrs, u8 param_count, u16 ret_addr, void* user_data) {
    (void)user_data;
    if (param_count != 2) return FFI_INVALID_PARAMS;
    i32 a = *(i32*)&memory[param_addrs[0]];
    i32 b = *(i32*)&memory[param_addrs[1]];
    *(i32*)&memory[ret_addr] = a + b;
    return STATUS_SUCCESS;
}

// Register a raw handler for testing
WASM_EXPORT void ffi_registerRawDemo() {
    runtime.registerFFI("F_raw_add", "i32,i32->i32", "Raw handler demo", demo_raw_handler);}