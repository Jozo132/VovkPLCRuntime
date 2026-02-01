// ladder-types.h - Ladder Graph Data Types and Structures
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

// Use types from parent includes - don't include stdint.h directly
// uint32_t etc. are provided by wasm/jvint.h via parent files

// ============================================================================
// Constants
// ============================================================================

static const int LADDER_MAX_NODES = 256;
static const int LADDER_MAX_CONNECTIONS = 256;
static const int LADDER_MAX_CONN_ENDPOINTS = 16;  // Max sources/destinations per connection
static const int LADDER_MAX_OUTPUT = 32768;

// ============================================================================
// Graph Node Structure
// ============================================================================

struct LadderGraphNode {
    char id[32];
    char type[32];
    char address[32];
    int x, y;
    bool inverted;
    uint32_t preset;
    char preset_str[32];
    char trigger[16];
    char data_type[8];
    char in1[32];
    char in2[32];
    char out[32];
    bool passThrough;

    // Runtime: flags for processing
    bool visited;
    bool emitted;
    int order_index;  // Processing order

    void clear() {
        id[0] = '\0';
        type[0] = '\0';
        address[0] = '\0';
        x = 0; y = 0;
        inverted = false;
        preset = 0;
        preset_str[0] = '\0';
        trigger[0] = 'n'; trigger[1] = 'o'; trigger[2] = 'r'; trigger[3] = 'm';
        trigger[4] = 'a'; trigger[5] = 'l'; trigger[6] = '\0';
        data_type[0] = '\0';
        in1[0] = '\0';
        in2[0] = '\0';
        out[0] = '\0';
        passThrough = false;
        visited = false;
        emitted = false;
        order_index = -1;
    }
};

// ============================================================================
// Connection Structure
// ============================================================================

struct LadderConnection {
    char sources[LADDER_MAX_CONN_ENDPOINTS][32];
    int source_count;
    char destinations[LADDER_MAX_CONN_ENDPOINTS][32];
    int dest_count;

    void clear() {
        source_count = 0;
        dest_count = 0;
    }
};

// ============================================================================
// Output Group Structure (for parallel branch handling)
// ============================================================================

struct LadderOutputGroup {
    int nodeIndices[LADDER_MAX_NODES];
    int count;
    int conditionSourceIdx;  // -1 if multiple sources

    void clear() {
        count = 0;
        conditionSourceIdx = -1;
    }
};

// ============================================================================
// Type Aliases for backwards compatibility (used by ladder-linter.h)
// ============================================================================

typedef LadderGraphNode GraphNode;
typedef LadderConnection Connection;
typedef LadderOutputGroup OutputGroup;

#endif // __WASM__
