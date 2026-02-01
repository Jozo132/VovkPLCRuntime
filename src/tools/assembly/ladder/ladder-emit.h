// ladder-emit.h - STL Code Emission Functions
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

#include "ladder-types.h"
#include "ladder-node-types.h"

// ============================================================================
// STL Emission Mixin
// These functions are designed to be used as part of the LadderGraphCompiler class
// They provide STL code generation for various ladder elements
// ============================================================================

// Helper to emit an integer value to the output buffer
inline void ladderEmitInt(char* output, int& output_len, int max_size, int val) {
    char buf[16];
    int i = 0;
    bool neg = val < 0;
    if (neg) val = -val;
    if (val == 0) buf[i++] = '0';
    else {
        while (val > 0) {
            buf[i++] = '0' + (val % 10);
            val /= 10;
        }
    }
    if (neg) buf[i++] = '-';
    while (i > 0 && output_len < max_size - 1) {
        output[output_len++] = buf[--i];
    }
    output[output_len] = '\0';
}

// Helper to emit a string to the output buffer
inline void ladderEmit(char* output, int& output_len, int max_size, const char* s) {
    while (*s && output_len < max_size - 1) {
        output[output_len++] = *s++;
    }
    output[output_len] = '\0';
}

// Helper to emit a single character to the output buffer
inline void ladderEmitChar(char* output, int& output_len, int max_size, char c) {
    if (output_len < max_size - 1) {
        output[output_len++] = c;
        output[output_len] = '\0';
    }
}

// Helper to emit indentation
inline void ladderEmitIndent(char* output, int& output_len, int max_size, int indent_level) {
    for (int i = 0; i < indent_level; i++) {
        ladderEmit(output, output_len, max_size, "    ");  // 4 spaces per indent level
    }
}

// Helper to emit a line with indentation
inline void ladderEmitLine(char* output, int& output_len, int max_size, int indent_level, const char* s) {
    ladderEmitIndent(output, output_len, max_size, indent_level);
    ladderEmit(output, output_len, max_size, s);
    ladderEmitChar(output, output_len, max_size, '\n');
}

// Emit address with proper Siemens type prefix
// Converts M14 -> MW14 (for word), MD14 (for dword), MB14 (for byte)
// Handles: M (memory), preserves #literals and already-typed addresses
inline void ladderEmitTypedAddress(char* output, int& output_len, int max_size, const char* addr, char typeSuffix) {
    if (!addr || addr[0] == '\0') return;

    // If it's a literal (starts with #), emit as-is
    if (addr[0] == '#') {
        ladderEmit(output, output_len, max_size, addr);
        return;
    }

    // If address already has a size prefix (MW, MD, MB), emit as-is
    if (addr[0] == 'M' && (addr[1] == 'W' || addr[1] == 'D' || addr[1] == 'B')) {
        ladderEmit(output, output_len, max_size, addr);
        return;
    }

    // For M addresses, insert the size prefix after M
    if (addr[0] == 'M' && addr[1] >= '0' && addr[1] <= '9') {
        char sizePrefix = ladderGetSiemensAddressSizePrefix(typeSuffix);
        ladderEmitChar(output, output_len, max_size, 'M');
        ladderEmitChar(output, output_len, max_size, sizePrefix);
        ladderEmit(output, output_len, max_size, addr + 1);  // Rest of address after 'M'
        return;
    }

    // For other addresses (X, Y, etc.), emit as-is
    ladderEmit(output, output_len, max_size, addr);
}

// Generate unique edge memory address
inline void ladderGenerateEdgeAddress(char* edge_mem, int& edge_mem_counter) {
    int byte_addr = edge_mem_counter / 8;
    int bit_addr = edge_mem_counter % 8;
    edge_mem_counter++;

    int idx = 0;
    edge_mem[idx++] = 'M';
    if (byte_addr >= 100) edge_mem[idx++] = '0' + (byte_addr / 100);
    if (byte_addr >= 10) edge_mem[idx++] = '0' + ((byte_addr / 10) % 10);
    edge_mem[idx++] = '0' + (byte_addr % 10);
    edge_mem[idx++] = '.';
    edge_mem[idx++] = '0' + bit_addr;
    edge_mem[idx] = '\0';
}

#endif // __WASM__
