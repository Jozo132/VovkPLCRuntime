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
    u8 type_size;           // Size in bytes (0 for bit type)

    void reset() {
        name[0] = '\0';
        type[0] = '\0';
        address = 0;
        bit = 255;
        is_bit = false;
        type_size = 0;
    }

    // Check if this symbol matches another exactly (for duplicate detection)
    bool exactMatch(const SharedSymbol& other) const {
        if (!sharedStrEq(name, other.name)) return false;
        if (!sharedStrEq(type, other.type)) return false;
        if (address != other.address) return false;
        if (bit != other.bit) return false;
        if (is_bit != other.is_bit) return false;
        if (type_size != other.type_size) return false;
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
    int addSymbol(const char* name, const char* type, u32 address, u8 bit, bool is_bit, u8 type_size) {
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

        symbol_count++;
        return 0;
    }

    // Convenience method to add symbol with full parameters
    int addSymbol(const SharedSymbol& sym) {
        return addSymbol(sym.name, sym.type, sym.address, sym.bit, sym.is_bit, sym.type_size);
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
};

// ============================================================================
// Global Shared Symbol Table Instance
// ============================================================================

static SharedSymbolTable sharedSymbols;

// Convenience functions for global access
inline void resetSharedSymbols() {
    sharedSymbols.reset();
}

inline SharedSymbol* findSharedSymbol(const char* name) {
    return sharedSymbols.findSymbol(name);
}

inline int addSharedSymbol(const char* name, const char* type, u32 address, u8 bit, bool is_bit, u8 type_size) {
    return sharedSymbols.addSymbol(name, type, address, bit, is_bit, type_size);
}

inline bool hasSharedSymbol(const char* name) {
    return sharedSymbols.hasSymbol(name);
}

#endif // __WASM__
