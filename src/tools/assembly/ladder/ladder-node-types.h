// ladder-node-types.h - Node Type Classification and Helper Functions
// Compiles graph-based ladder logic (nodes + connections) to Siemens-style STL
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

// ============================================================================
// String Comparison Helper (case-insensitive)
// ============================================================================

inline bool ladderStrEqI(const char* a, const char* b) {
    while (*a && *b) {
        char ca = (*a >= 'A' && *a <= 'Z') ? *a + 32 : *a;
        char cb = (*b >= 'A' && *b <= 'Z') ? *b + 32 : *b;
        if (ca != cb) return false;
        a++; b++;
    }
    return *a == *b;
}

// ============================================================================
// Node Type Classification
// ============================================================================

inline bool ladderIsContact(const char* type) {
    return ladderStrEqI(type, "contact_no") || ladderStrEqI(type, "contact_nc") ||
        ladderStrEqI(type, "contact_pos") || ladderStrEqI(type, "contact_neg") ||
        ladderStrEqI(type, "contact");
}

inline bool ladderIsCoil(const char* type) {
    return ladderStrEqI(type, "coil") || ladderStrEqI(type, "coil_set") ||
        ladderStrEqI(type, "coil_rset") || ladderStrEqI(type, "coil_neg");
}

inline bool ladderIsTimer(const char* type) {
    return ladderStrEqI(type, "timer_on") || ladderStrEqI(type, "timer_off") ||
        ladderStrEqI(type, "timer_pulse") || ladderStrEqI(type, "timer_ton") ||
        ladderStrEqI(type, "timer_tof") || ladderStrEqI(type, "timer_tp");
}

inline bool ladderIsCounter(const char* type) {
    return ladderStrEqI(type, "counter_up") || ladderStrEqI(type, "counter_down") ||
        ladderStrEqI(type, "counter_u") || ladderStrEqI(type, "counter_d");
}

// Operation blocks: math and memory operations that execute conditionally
inline bool ladderIsOperationBlock(const char* type) {
    // Math operations
    if (ladderStrEqI(type, "fb_add") || ladderStrEqI(type, "fb_sub") ||
        ladderStrEqI(type, "fb_mul") || ladderStrEqI(type, "fb_div") ||
        ladderStrEqI(type, "fb_mod")) return true;
    // Increment/decrement
    if (ladderStrEqI(type, "fb_inc") || ladderStrEqI(type, "fb_dec")) return true;
    // Bitwise operations
    if (ladderStrEqI(type, "fb_and") || ladderStrEqI(type, "fb_or") ||
        ladderStrEqI(type, "fb_xor") || ladderStrEqI(type, "fb_not") ||
        ladderStrEqI(type, "fb_shl") || ladderStrEqI(type, "fb_shr")) return true;
    // Move/copy operations
    if (ladderStrEqI(type, "fb_move") || ladderStrEqI(type, "fb_copy")) return true;
    return false;
}

// Comparator blocks: compare values and produce boolean result (like contacts)
inline bool ladderIsComparator(const char* type) {
    return ladderStrEqI(type, "cmp_eq") || ladderStrEqI(type, "cmp_ne") ||
        ladderStrEqI(type, "cmp_gt") || ladderStrEqI(type, "cmp_lt") ||
        ladderStrEqI(type, "cmp_ge") || ladderStrEqI(type, "cmp_le") ||
        ladderStrEqI(type, "cmp_gte") || ladderStrEqI(type, "cmp_lte") ||
        ladderStrEqI(type, "fb_cmp_eq") || ladderStrEqI(type, "fb_cmp_ne") ||
        ladderStrEqI(type, "fb_cmp_gt") || ladderStrEqI(type, "fb_cmp_lt") ||
        ladderStrEqI(type, "fb_cmp_ge") || ladderStrEqI(type, "fb_cmp_le") ||
        ladderStrEqI(type, "fb_cmp_gte") || ladderStrEqI(type, "fb_cmp_lte");
}

// isTerminationNode: nodes that write output and can optionally pass through RLO
inline bool ladderIsTerminationNode(const char* type) {
    return ladderIsCoil(type) || ladderIsTimer(type) || ladderIsCounter(type) || ladderIsOperationBlock(type);
}

// isOutputNode: nodes that are actual outputs (coils, operation blocks)
// Operation blocks are like coils - they execute conditionally and can pass through RLO
inline bool ladderIsOutputNode(const char* type) {
    return ladderIsCoil(type) || ladderIsOperationBlock(type);
}

// isInputNode: nodes that produce boolean conditions
inline bool ladderIsInputNode(const char* type) {
    return ladderIsContact(type) || ladderIsComparator(type);
}

// isFunctionBlock: timers and counters (special handling)
inline bool ladderIsFunctionBlock(const char* type) {
    return ladderIsTimer(type) || ladderIsCounter(type);
}

// isTapNode: tap nodes for RLO passthrough
inline bool ladderIsTapNode(const char* type) {
    return ladderStrEqI(type, "tap");
}

// isKnownNodeType: check if a node type is valid (known)
inline bool ladderIsKnownNodeType(const char* type) {
    return ladderIsContact(type) || ladderIsCoil(type) || ladderIsTimer(type) ||
           ladderIsCounter(type) || ladderIsOperationBlock(type) || ladderIsComparator(type) ||
           ladderStrEqI(type, "tap");
}

// ============================================================================
// Data Type Helpers
// ============================================================================

// Get Siemens type suffix (I=16-bit int, D=32-bit int, R=32-bit float, B=8-bit)
inline char ladderGetSiemensTypeSuffix(const char* data_type) {
    if (ladderStrEqI(data_type, "i16") || ladderStrEqI(data_type, "int16") ||
        ladderStrEqI(data_type, "u16") || ladderStrEqI(data_type, "uint16")) return 'I';
    if (ladderStrEqI(data_type, "i32") || ladderStrEqI(data_type, "int32") ||
        ladderStrEqI(data_type, "u32") || ladderStrEqI(data_type, "uint32")) return 'D';
    if (ladderStrEqI(data_type, "f32") || ladderStrEqI(data_type, "float")) return 'R';
    if (ladderStrEqI(data_type, "i8") || ladderStrEqI(data_type, "int8") ||
        ladderStrEqI(data_type, "u8") || ladderStrEqI(data_type, "uint8")) return 'B';
    return 'I';  // Default to 16-bit integer
}

// Get Siemens address size prefix (W=word/16-bit, D=dword/32-bit, B=byte/8-bit)
// For memory addresses like M14 -> MW14 (word), MD14 (dword), MB14 (byte)
inline char ladderGetSiemensAddressSizePrefix(char typeSuffix) {
    switch (typeSuffix) {
        case 'I': return 'W';  // Word (16-bit)
        case 'D': return 'D';  // Double word (32-bit)
        case 'R': return 'D';  // Real stored in double word
        case 'B': return 'B';  // Byte (8-bit)
        default: return 'W';
    }
}

// Check if address is a raw memory address (starts with M, X, Y, K, S, T, C, or number)
// Raw addresses like "X0.0", "M10", "T0" don't need symbol lookup
// Also handles literal values prefixed with # (e.g., #100, #-50, #3.14)
inline bool ladderIsRawAddress(const char* addr) {
    if (!addr || addr[0] == '\0') return false;
    char first = addr[0];
    // Literal value prefix (e.g., #100, #-50, #3.14)
    if (first == '#') {
        // Must have at least one character after #
        if (addr[1] == '\0') return false;
        // Can be a number or negative number
        char second = addr[1];
        if (second >= '0' && second <= '9') return true;
        if (second == '-' && addr[2] >= '0' && addr[2] <= '9') return true;
        return false;
    }
    // Memory area prefixes
    if (first == 'M' || first == 'm' || first == 'X' || first == 'x' ||
        first == 'Y' || first == 'y' || first == 'K' || first == 'k' ||
        first == 'S' || first == 's' || first == 'T' || first == 't' ||
        first == 'C' || first == 'c') {
        // Check if next char is a digit (raw address) or not (could be symbol)
        if (addr[1] >= '0' && addr[1] <= '9') return true;
    }
    // Pure numeric address
    if (first >= '0' && first <= '9') return true;
    return false;
}

#endif // __WASM__
