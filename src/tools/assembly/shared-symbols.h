// shared-symbols.h - Shared Symbol Table for all compilers
// Provides a common symbol lookup that can be shared across PLCASM, STL, and Ladder compilers
//
// Copyright (c) 2026 J.Vovk
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

#ifdef __WASM__

#include "../runtime-lib.h"

#define SHARED_MAX_SYMBOLS 256

// ============================================================================
// Local string utilities (duplicated to avoid include order issues)
// ============================================================================

inline bool sharedStrEq(const char* a, const char* b) {
    if (!a || !b) return a == b;
    while (*a && *b) {
        if (*a != *b) return false;
        a++; b++;
    }
    return *a == *b;
}

inline char sharedToLower(char c) {
    return (c >= 'A' && c <= 'Z') ? (c + 32) : c;
}

inline bool sharedStrEqI(const char* a, const char* b) {
    if (!a || !b) return a == b;
    while (*a && *b) {
        if (sharedToLower(*a) != sharedToLower(*b)) return false;
        a++; b++;
    }
    return *a == *b;
}

// ============================================================================
// Shared Symbol Structure
// ============================================================================
// This structure is used by all compilers for symbol lookup.
// It contains all fields needed for validation and code generation.

struct SharedSymbol {
    char name[64];          // Symbol name
    char type[16];          // Type name (bit, i8, i16, i32, u8, u16, u32, f32, f64, etc.)
    u32 address;            // Memory address or byte offset
    u8 bit;                 // Bit position (0-7) or 255 if not a bit type
    bool is_bit;            // True if this is a bit address
    u8 type_size;           // Size in bytes (0 for bit type, or element size for arrays)
    
    // Array support
    u16 array_size;         // Number of elements (0 = not an array, 1+ = array)

    void reset() {
        name[0] = '\0';
        type[0] = '\0';
        address = 0;
        bit = 255;
        is_bit = false;
        type_size = 0;
        array_size = 0;
    }
    
    // Check if this is an array type
    bool isArray() const { return array_size > 0; }
    
    // Get total size in bytes (element size * count for arrays)
    u32 totalSize() const {
        if (array_size == 0) return type_size;
        return (u32)type_size * (u32)array_size;
    }

    // Check if this symbol matches another exactly (for duplicate detection)
    bool exactMatch(const SharedSymbol& other) const {
        if (!sharedStrEq(name, other.name)) return false;
        if (!sharedStrEq(type, other.type)) return false;
        if (address != other.address) return false;
        if (bit != other.bit) return false;
        if (is_bit != other.is_bit) return false;
        if (type_size != other.type_size) return false;
        if (array_size != other.array_size) return false;
        return true;
    }
};

// ============================================================================
// Shared Symbol Table
// ============================================================================
// Global symbol table that all compilers can access and share.

class SharedSymbolTable {
public:
    SharedSymbol symbols[SHARED_MAX_SYMBOLS];
    int symbol_count = 0;

    SharedSymbolTable() {
        reset();
    }

    // Reset/clear all symbols
    void reset() {
        symbol_count = 0;
        for (int i = 0; i < SHARED_MAX_SYMBOLS; i++) {
            symbols[i].reset();
        }
    }

    // Find symbol by name (case-insensitive)
    SharedSymbol* findSymbol(const char* name) {
        if (!name || name[0] == '\0') return nullptr;
        for (int i = 0; i < symbol_count; i++) {
            if (sharedStrEqI(symbols[i].name, name)) {
                return &symbols[i];
            }
        }
        return nullptr;
    }

    // Add a symbol to the table
    // Returns: 0 = success, 1 = duplicate exact match (silently ignored), -1 = table full, -2 = duplicate with different definition
    int addSymbol(const char* name, const char* type, u32 address, u8 bit, bool is_bit, u8 type_size, u16 array_size = 0) {
        if (symbol_count >= SHARED_MAX_SYMBOLS) return -1;

        // Check for existing symbol with same name
        SharedSymbol* existing = findSymbol(name);
        if (existing) {
            // Build a temporary symbol for comparison
            SharedSymbol temp;
            int i = 0;
            while (name[i] && i < 63) { temp.name[i] = name[i]; i++; }
            temp.name[i] = '\0';
            i = 0;
            while (type[i] && i < 15) { temp.type[i] = type[i]; i++; }
            temp.type[i] = '\0';
            temp.address = address;
            temp.bit = bit;
            temp.is_bit = is_bit;
            temp.type_size = type_size;
            temp.array_size = array_size;

            // If exact match, silently ignore (return 1 for success-duplicate)
            if (existing->exactMatch(temp)) {
                return 1;
            }
            // Different definition with same name - conflict
            return -2;
        }

        // Add new symbol
        SharedSymbol& sym = symbols[symbol_count];
        int i = 0;
        while (name[i] && i < 63) { sym.name[i] = name[i]; i++; }
        sym.name[i] = '\0';
        i = 0;
        while (type[i] && i < 15) { sym.type[i] = type[i]; i++; }
        sym.type[i] = '\0';
        sym.address = address;
        sym.bit = bit;
        sym.is_bit = is_bit;
        sym.type_size = type_size;
        sym.array_size = array_size;

        symbol_count++;
        return 0;
    }

    // Convenience method to add symbol with full parameters
    int addSymbol(const SharedSymbol& sym) {
        return addSymbol(sym.name, sym.type, sym.address, sym.bit, sym.is_bit, sym.type_size, sym.array_size);
    }

    // Check if a name is a known symbol
    bool hasSymbol(const char* name) {
        return findSymbol(name) != nullptr;
    }

    // Get type size in bytes for a type name
    static u8 getTypeSize(const char* type) {
        if (sharedStrEqI(type, "i8") || sharedStrEqI(type, "u8") || sharedStrEqI(type, "byte")) return 1;
        if (sharedStrEqI(type, "i16") || sharedStrEqI(type, "u16")) return 2;
        if (sharedStrEqI(type, "i32") || sharedStrEqI(type, "u32") || sharedStrEqI(type, "f32")) return 4;
        if (sharedStrEqI(type, "i64") || sharedStrEqI(type, "u64") || sharedStrEqI(type, "f64")) return 8;
        if (sharedStrEqI(type, "bit") || sharedStrEqI(type, "bool")) return 0; // bit type has no byte size
        if (sharedStrEqI(type, "ptr") || sharedStrEqI(type, "pointer")) return 2; // 16-bit pointer
        return 0; // unknown type
    }

    // Check if a type name is valid
    static bool isValidType(const char* type) {
        const char* valid_types[] = { "i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64",
                                      "f32", "f64", "bool", "bit", "byte", "ptr", "pointer" };
        for (int i = 0; i < 15; i++) {
            if (sharedStrEqI(type, valid_types[i])) return true;
        }
        return false;
    }

    // Check if type is a timer type
    static bool isTimerType(const char* type) {
        return sharedStrEqI(type, "timer") || sharedStrEqI(type, "ton") || 
               sharedStrEqI(type, "tof") || sharedStrEqI(type, "tp");
    }

    // Check if type is a counter type
    static bool isCounterType(const char* type) {
        return sharedStrEqI(type, "counter") || sharedStrEqI(type, "ctu") || 
               sharedStrEqI(type, "ctd") || sharedStrEqI(type, "ctud");
    }
};

// ============================================================================
// Structure Property Definitions
// ============================================================================
// Defines the accessible properties for built-in structured types (Timer, Counter)
// and provides infrastructure for user-defined structures.

struct StructProperty {
    char name[16];          // Property name (e.g., "Q", "ET", "CV")
    u8 offset;              // Byte offset within the struct
    u8 type_size;           // Size in bytes (0 = bit type)
    u8 bit_pos;             // Bit position (0-7) for bit types, 255 for non-bit
    bool readable;          // Can be read
    bool writable;          // Can be written
};

// Maximum number of properties per struct type
#define MAX_STRUCT_PROPERTIES 8

// Built-in Timer properties (matches TIMER_OFFSET_* in methods-timer.h)
// Timer struct: [ET:u32][StartTime:u32][Flags:u8] = 9 bytes
//   Flags: bit0=Q, bit1=RUNNING, bit2=IN_OLD
static const StructProperty timerProperties[] = {
    { "Q",   8, 0, 0, true, false },   // Q output - FLAGS byte, bit 0 (read-only)
    { "ET",  0, 4, 255, true, false }, // Elapsed Time - offset 0, u32 (read-only)
    { "IN",  8, 0, 2, true, false },   // Previous IN - FLAGS byte, bit 2 (read-only, internal)
    { "RUN", 8, 0, 1, true, false },   // Running flag - FLAGS byte, bit 1 (read-only, internal)
};
static const int timerPropertyCount = 4;

// Built-in Counter properties (matches COUNTER_OFFSET_* in methods-counter.h)
// Counter struct: [CV:u32][Flags:u8] = 5 bytes
//   Flags: bit0=Q, bit1=INPUT_OLD
static const StructProperty counterProperties[] = {
    { "Q",  4, 0, 0, true, false },   // Q output - FLAGS byte, bit 0 (read-only)
    { "CV", 0, 4, 255, true, true },  // Current Value - offset 0, u32 (read/write)
    { "IN", 4, 0, 1, true, false },   // Previous input - FLAGS byte, bit 1 (read-only, internal)
};
static const int counterPropertyCount = 3;

// ============================================================================
// User-Defined Structure Support
// ============================================================================
// Allows users to define custom structure types with named fields

#define MAX_USER_STRUCT_TYPES 16

struct UserStructType {
    char name[32];                              // Type name (e.g., "MyStruct")
    StructProperty fields[MAX_STRUCT_PROPERTIES]; // Field definitions
    int field_count;                            // Number of fields
    u8 total_size;                              // Total size in bytes

    void reset() {
        name[0] = '\0';
        field_count = 0;
        total_size = 0;
        for (int i = 0; i < MAX_STRUCT_PROPERTIES; i++) {
            fields[i].name[0] = '\0';
            fields[i].offset = 0;
            fields[i].type_size = 0;
            fields[i].bit_pos = 255;
            fields[i].readable = true;
            fields[i].writable = true;
        }
    }

    // Find a field by name (case-insensitive)
    const StructProperty* findField(const char* fieldName) const {
        for (int i = 0; i < field_count; i++) {
            if (sharedStrEqI(fields[i].name, fieldName)) {
                return &fields[i];
            }
        }
        return nullptr;
    }
};

// Global user struct type registry
static UserStructType userStructTypes[MAX_USER_STRUCT_TYPES];
static int userStructTypeCount = 0;

inline void resetUserStructTypes() {
    userStructTypeCount = 0;
    for (int i = 0; i < MAX_USER_STRUCT_TYPES; i++) {
        userStructTypes[i].reset();
    }
}

inline UserStructType* findUserStructType(const char* typeName) {
    for (int i = 0; i < userStructTypeCount; i++) {
        if (sharedStrEqI(userStructTypes[i].name, typeName)) {
            return &userStructTypes[i];
        }
    }
    return nullptr;
}

// Case-sensitive version for checking exact name matches
inline UserStructType* findUserStructTypeExact(const char* typeName) {
    for (int i = 0; i < userStructTypeCount; i++) {
        if (sharedStrEq(userStructTypes[i].name, typeName)) {
            return &userStructTypes[i];
        }
    }
    return nullptr;
}

inline UserStructType* addUserStructType(const char* typeName) {
    if (userStructTypeCount >= MAX_USER_STRUCT_TYPES) return nullptr;
    UserStructType& st = userStructTypes[userStructTypeCount++];
    st.reset();
    int i = 0;
    while (typeName[i] && i < 31) { st.name[i] = typeName[i]; i++; }
    st.name[i] = '\0';
    return &st;
}

// Result of comparing two struct type definitions
enum StructCompareResult {
    STRUCT_COMPARE_IDENTICAL = 0,    // Definitions are exactly the same
    STRUCT_COMPARE_DIFFERENT = 1,    // Definitions differ
    STRUCT_COMPARE_NOT_FOUND = 2     // Existing struct not found
};

// Compare two UserStructType definitions for equality
// Returns STRUCT_COMPARE_IDENTICAL if they match, STRUCT_COMPARE_DIFFERENT otherwise
inline StructCompareResult compareUserStructTypes(const UserStructType* a, const UserStructType* b) {
    if (!a || !b) return STRUCT_COMPARE_NOT_FOUND;
    
    // Compare field count
    if (a->field_count != b->field_count) return STRUCT_COMPARE_DIFFERENT;
    
    // Compare total size
    if (a->total_size != b->total_size) return STRUCT_COMPARE_DIFFERENT;
    
    // Compare each field
    for (int i = 0; i < a->field_count; i++) {
        const StructProperty& fa = a->fields[i];
        const StructProperty& fb = b->fields[i];
        
        // Compare field name (case-insensitive)
        if (!sharedStrEqI(fa.name, fb.name)) return STRUCT_COMPARE_DIFFERENT;
        
        // Compare field properties
        if (fa.offset != fb.offset) return STRUCT_COMPARE_DIFFERENT;
        if (fa.type_size != fb.type_size) return STRUCT_COMPARE_DIFFERENT;
        if (fa.bit_pos != fb.bit_pos) return STRUCT_COMPARE_DIFFERENT;
    }
    
    return STRUCT_COMPARE_IDENTICAL;
}

// Try to add a user struct type, checking for collisions
// Returns: pointer to the struct type (new or existing identical), nullptr on error
// Sets isDuplicate to true if an identical definition already exists
// Sets isConflict to true if a conflicting definition exists
inline UserStructType* addUserStructTypeWithCheck(const char* typeName, bool* isDuplicate, bool* isConflict) {
    *isDuplicate = false;
    *isConflict = false;
    
    // Check if a type with this name already exists
    UserStructType* existing = findUserStructType(typeName);
    if (existing) {
        // Will compare after the new type is populated - return existing for now
        // Caller must populate temp and compare
        *isDuplicate = true;  // Signal that we found an existing type
        return existing;
    }
    
    return addUserStructType(typeName);
}

// ============================================================================
// Property Resolution Result
// ============================================================================
// Result structure for resolving a property access expression

struct PropertyResolution {
    bool success;           // True if resolution succeeded
    u32 address;            // Final memory address (base + property offset)
    u8 type_size;           // Size of the property in bytes (0 = bit)
    u8 bit_pos;             // Bit position for bit types, 255 otherwise
    bool is_bit;            // True if this is a bit access
    bool readable;          // Can be read
    bool writable;          // Can be written
    char error[64];         // Error message if resolution failed
};

// ============================================================================
// Property Resolution Functions
// ============================================================================

// Resolve a timer property access (e.g., "Q", "ET")
inline PropertyResolution resolveTimerProperty(u32 timerBaseAddress, const char* propName) {
    PropertyResolution result = {};
    for (int i = 0; i < timerPropertyCount; i++) {
        if (sharedStrEqI(timerProperties[i].name, propName)) {
            result.success = true;
            result.address = timerBaseAddress + timerProperties[i].offset;
            result.type_size = timerProperties[i].type_size;
            result.bit_pos = timerProperties[i].bit_pos;
            result.is_bit = (timerProperties[i].bit_pos != 255);
            result.readable = timerProperties[i].readable;
            result.writable = timerProperties[i].writable;
            return result;
        }
    }
    result.success = false;
    // Build error message
    int j = 0;
    const char* err = "Unknown timer property: ";
    while (err[j] && j < 40) { result.error[j] = err[j]; j++; }
    int k = 0;
    while (propName[k] && j < 63) { result.error[j++] = propName[k++]; }
    result.error[j] = '\0';
    return result;
}

// Resolve a counter property access (e.g., "Q", "CV")
inline PropertyResolution resolveCounterProperty(u32 counterBaseAddress, const char* propName) {
    PropertyResolution result = {};
    for (int i = 0; i < counterPropertyCount; i++) {
        if (sharedStrEqI(counterProperties[i].name, propName)) {
            result.success = true;
            result.address = counterBaseAddress + counterProperties[i].offset;
            result.type_size = counterProperties[i].type_size;
            result.bit_pos = counterProperties[i].bit_pos;
            result.is_bit = (counterProperties[i].bit_pos != 255);
            result.readable = counterProperties[i].readable;
            result.writable = counterProperties[i].writable;
            return result;
        }
    }
    result.success = false;
    int j = 0;
    const char* err = "Unknown counter property: ";
    while (err[j] && j < 40) { result.error[j] = err[j]; j++; }
    int k = 0;
    while (propName[k] && j < 63) { result.error[j++] = propName[k++]; }
    result.error[j] = '\0';
    return result;
}

// Resolve a user-defined struct property access
inline PropertyResolution resolveUserStructProperty(const UserStructType* structType, u32 baseAddress, const char* propName) {
    PropertyResolution result = {};
    const StructProperty* field = structType->findField(propName);
    if (field) {
        result.success = true;
        result.address = baseAddress + field->offset;
        result.type_size = field->type_size;
        result.bit_pos = field->bit_pos;
        result.is_bit = (field->bit_pos != 255);
        result.readable = field->readable;
        result.writable = field->writable;
        return result;
    }
    result.success = false;
    int j = 0;
    const char* err = "Unknown field '";
    while (err[j] && j < 20) { result.error[j] = err[j]; j++; }
    int k = 0;
    while (propName[k] && j < 40) { result.error[j++] = propName[k++]; }
    const char* in = "' in struct '";
    k = 0;
    while (in[k] && j < 55) { result.error[j++] = in[k++]; }
    k = 0;
    while (structType->name[k] && j < 62) { result.error[j++] = structType->name[k++]; }
    if (j < 63) result.error[j++] = '\'';
    result.error[j] = '\0';
    return result;
}

// ============================================================================
// Global Shared Symbol Table Instance
// ============================================================================

SharedSymbolTable sharedSymbols = SharedSymbolTable();

// Convenience functions for global access
inline void resetSharedSymbols() {
    sharedSymbols.reset();
}

inline SharedSymbol* findSharedSymbol(const char* name) {
    return sharedSymbols.findSymbol(name);
}

inline int addSharedSymbol(const char* name, const char* type, u32 address, u8 bit, bool is_bit, u8 type_size, u16 array_size = 0) {
    return sharedSymbols.addSymbol(name, type, address, bit, is_bit, type_size, array_size);
}

inline bool hasSharedSymbol(const char* name) {
    return sharedSymbols.hasSymbol(name);
}

#endif // __WASM__
