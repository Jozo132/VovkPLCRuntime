// ladder-validation.h - Graph Validation Functions
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
#include "../shared-symbols.h"

// ============================================================================
// Validation Helper Mixin
// These are designed to be part of the LadderGraphCompiler class
// ============================================================================

// Validate a symbol name exists in the shared symbol table
// Returns true if address is raw OR if symbol exists
// reportNodeError is a function pointer that will be called on error
inline bool ladderValidateSymbolExists(
    const char* addr, 
    int nodeIdx,
    void (*reportNodeError)(const char* msg, int nodeIdx, void* ctx),
    void* ctx
) {
    if (!addr || addr[0] == '\0') return true;  // Empty handled elsewhere
    if (ladderIsRawAddress(addr)) return true;  // Raw addresses don't need lookup

    // Check shared symbol table (if populated)
    if (sharedSymbols.symbol_count > 0) {
        if (!sharedSymbols.findSymbol(addr)) {
            char msg[64];
            int mi = 0;
            const char* prefix = "Unknown symbol '";
            while (*prefix && mi < 40) msg[mi++] = *prefix++;
            int ai = 0;
            while (addr[ai] && mi < 58) msg[mi++] = addr[ai++];
            msg[mi++] = '\'';
            msg[mi] = '\0';
            if (reportNodeError) {
                reportNodeError(msg, nodeIdx, ctx);
            }
            return false;
        }
    }
    return true;
}

#endif // __WASM__
