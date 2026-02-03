// ladder-linter.h - Linter for Ladder Graph JSON
// Extends LadderGraphCompiler to capture multiple errors for IDE integration.
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

#include "ladder-compiler.h"
#include "plcasm-linter.h" // Reuse LinterProblem struct and enums

#define LADDER_MAX_LINT_PROBLEMS 100
#define LADDER_MAX_SYMBOLS 128

// Simple symbol structure for ladder linting
struct LadderSymbol {
    char name[64];
    char type[16];
    u32 address;
    u8 bit;
    bool is_bit;
    u8 type_size;
};

// ### Ladder Linter Class Definition ###

class LadderLinter : public LadderGraphCompiler {
public:
    LinterProblem problems[LADDER_MAX_LINT_PROBLEMS];
    int problem_count = 0;

    // Symbol table for validating addresses
    LadderSymbol symbols[LADDER_MAX_SYMBOLS];
    int symbol_count = 0;

    LadderLinter() : LadderGraphCompiler() {
        problem_count = 0;
        symbol_count = 0;
    }

    void clearProblems() {
        problem_count = 0;
        symbol_count = 0;
        has_error = false;
        error_msg[0] = '\0';
    }

    void clearState() override {
        LadderGraphCompiler::clearState();
        clearProblems();
    }

    // ============ Symbol Management ============

    void addSymbol(const char* name, const char* type, u32 address, u8 bit, bool is_bit, u8 type_size) {
        if (symbol_count >= LADDER_MAX_SYMBOLS) return;
        LadderSymbol& sym = symbols[symbol_count++];
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
    }

    LadderSymbol* findSymbol(const char* name) {
        for (int i = 0; i < symbol_count; i++) {
            if (strEqI(symbols[i].name, name)) return &symbols[i];
        }
        return nullptr;
    }

    // Check if a symbol/address is valid (handles struct.property access)
    bool isValidSymbol(const char* addr) {
        if (!addr || addr[0] == '\0') return false;
        if (isRawAddress(addr)) return true;

        // Check local symbol table
        if (findSymbol(addr)) return true;
        
        // Check shared symbol table  
        if (sharedSymbols.findSymbol(addr)) return true;
        
        // Check for struct property access (e.g., "myStruct.field")
        int dotPos = -1;
        for (int i = 0; addr[i]; i++) {
            if (addr[i] == '.') {
                dotPos = i;
                break;
            }
        }
        
        if (dotPos > 0) {
            // Extract base symbol name
            char baseName[64];
            for (int i = 0; i < dotPos && i < 63; i++) {
                baseName[i] = addr[i];
            }
            baseName[dotPos] = '\0';
            
            // Check if base symbol exists in shared symbols
            SharedSymbol* baseSym = sharedSymbols.findSymbol(baseName);
            if (baseSym) {
                // Check if it's a user struct type
                UserStructType* userStruct = findUserStructType(baseSym->type);
                if (userStruct) {
                    // Verify the property exists
                    const char* propName = addr + dotPos + 1;
                    if (userStruct->findField(propName)) {
                        return true;  // Valid struct.property
                    }
                }
            }
        }
        
        return false;
    }

    // ============ Problem Reporting ============

    // Add a problem using node's x,y position as line,column
    void addProblemAtNode(LinterProblemType type, const char* message, int nodeIdx, const char* token = nullptr) {
        if (problem_count >= LADDER_MAX_LINT_PROBLEMS) return;
        if (nodeIdx < 0 || nodeIdx >= node_count) return;

        GraphNode& node = nodes[nodeIdx];
        LinterProblem& p = problems[problem_count];
        p.type = type;
        // Use node's x,y position as line (y+1) and column (x+1) for 1-based indexing
        p.line = node.y + 1;
        p.column = node.x + 1;
        p.length = 1;
        p.lang = 3;  // LADDER language ID

        // Copy token into stable buffer
        p.token_buf[0] = '\0';
        if (token) {
            int ti = 0;
            while (token[ti] && ti < 63) { p.token_buf[ti] = token[ti]; ti++; }
            p.token_buf[ti] = '\0';
        } else {
            // Use node id as token
            int ti = 0;
            while (node.id[ti] && ti < 63) { p.token_buf[ti] = node.id[ti]; ti++; }
            p.token_buf[ti] = '\0';
        }
        p.token_text = p.token_buf;

        // Copy message safely (max 127 chars)
        int i = 0;
        while (message[i] && i < 127) {
            p.message[i] = message[i];
            i++;
        }
        p.message[i] = '\0';

        problem_count++;
    }

    void addError(const char* message, int nodeIdx, const char* token = nullptr) {
        addProblemAtNode(LINT_ERROR, message, nodeIdx, token);
        has_error = true;
    }

    void addWarning(const char* message, int nodeIdx, const char* token = nullptr) {
        addProblemAtNode(LINT_WARNING, message, nodeIdx, token);
    }

    void addInfo(const char* message, int nodeIdx, const char* token = nullptr) {
        addProblemAtNode(LINT_INFO, message, nodeIdx, token);
    }

    // Add a connection-related error with c[<index>] token format
    void addConnectionError(const char* message, int connIdx, int nodeIdx = -1) {
        if (problem_count >= LADDER_MAX_LINT_PROBLEMS) return;

        LinterProblem& p = problems[problem_count];
        p.type = LINT_ERROR;
        p.lang = 3;  // LADDER language ID

        // Use node position if available, otherwise default to 1,1
        if (nodeIdx >= 0 && nodeIdx < node_count) {
            GraphNode& node = nodes[nodeIdx];
            p.line = node.y + 1;
            p.column = node.x + 1;
        } else {
            p.line = 1;
            p.column = 1;
        }
        p.length = 1;

        // Build c[<index>] token
        p.token_buf[0] = 'c';
        p.token_buf[1] = '[';
        int ti = 2;
        char numBuf[16];
        int idx = connIdx;
        int nlen = 0;
        if (idx < 0) { numBuf[nlen++] = '-'; idx = -idx; }
        if (idx == 0) numBuf[nlen++] = '0';
        else {
            int start = nlen;
            while (idx > 0) { numBuf[nlen++] = '0' + (idx % 10); idx /= 10; }
            for (int r = start; r < start + (nlen - start) / 2; r++) {
                char tmp = numBuf[r];
                numBuf[r] = numBuf[nlen - 1 - (r - start)];
                numBuf[nlen - 1 - (r - start)] = tmp;
            }
        }
        for (int ni = 0; ni < nlen && ti < 60; ni++) p.token_buf[ti++] = numBuf[ni];
        p.token_buf[ti++] = ']';
        p.token_buf[ti] = '\0';
        p.token_text = p.token_buf;

        // Copy message
        int mi = 0;
        while (message[mi] && mi < 127) { p.message[mi] = message[mi]; mi++; }
        p.message[mi] = '\0';

        problem_count++;
        has_error = true;
    }

    // Override setError to add to problems list but continue linting
    void setError(const char* msg) override {
        // Don't set has_error - allow linting to continue
        // Just record the error
        int i = 0;
        while (msg[i] && i < 255) {
            error_msg[i] = msg[i];
            i++;
        }
        error_msg[i] = '\0';

        // Add as a general problem (line 1, col 1 since we don't have node context)
        if (problem_count < LADDER_MAX_LINT_PROBLEMS) {
            LinterProblem& p = problems[problem_count++];
            p.type = LINT_ERROR;
            p.line = 1;
            p.column = 1;
            p.length = 1;
            p.lang = 3;
            p.token_buf[0] = '\0';
            p.token_text = p.token_buf;
            int mi = 0;
            while (msg[mi] && mi < 127) { p.message[mi] = msg[mi]; mi++; }
            p.message[mi] = '\0';
        }
    }

    // Override reportNodeError to collect multiple errors
    void reportNodeError(const char* msg, int nodeIdx) override {
        addError(msg, nodeIdx);
    }

    // Override reportNodeWarning to collect warnings
    void reportNodeWarning(const char* msg, int nodeIdx) override {
        addWarning(msg, nodeIdx);
    }

    // Override reportConnectionError to collect connection errors
    void reportConnectionError(const char* msg, int connIdx, int nodeIdx = -1) override {
        addConnectionError(msg, connIdx, nodeIdx);
    }

    // ============ Extended Validation ============

    // Override validateNode to add symbol checking
    bool validateNode(int nodeIdx) override {
        if (nodeIdx < 0 || nodeIdx >= node_count) return true;
        GraphNode& node = nodes[nodeIdx];

        // Run base validation first
        // Check for unknown node type
        if (!isKnownNodeType(node.type)) {
            char msg[64];
            int mi = 0;
            const char* prefix = "Unknown node type '";
            while (*prefix && mi < 50) msg[mi++] = *prefix++;
            int ti = 0;
            while (node.type[ti] && mi < 60) msg[mi++] = node.type[ti++];
            msg[mi++] = '\'';
            msg[mi] = '\0';
            addError(msg, nodeIdx, node.type);
        }

        // Check address/symbol for most node types (tap nodes don't need addresses)
        // Comparators and operation blocks use in1/in2 instead of address
        if (!isTapNode(node.type) && !isComparator(node.type) && !isOperationBlock(node.type) && isKnownNodeType(node.type)) {
            if (node.address[0] == '\0') {
                addError("Missing address/symbol", nodeIdx);
            } else if (symbol_count > 0 || sharedSymbols.symbol_count > 0) {
                // Validate symbol exists in symbol table (local or shared)
                if (!isValidSymbol(node.address)) {
                    char msg[64];
                    int mi = 0;
                    const char* prefix = "Unknown symbol '";
                    while (*prefix && mi < 40) msg[mi++] = *prefix++;
                    int ai = 0;
                    while (node.address[ai] && mi < 58) msg[mi++] = node.address[ai++];
                    msg[mi++] = '\'';
                    msg[mi] = '\0';
                    addError(msg, nodeIdx, node.address);
                }
            }
        }

        // Validate node-type-specific parameters
        if (isTimer(node.type) || isCounter(node.type)) {
            if (node.preset == 0 && node.preset_str[0] == '\0') {
                addWarning("Timer/counter has no preset value", nodeIdx);
            }
        }

        if (isOperationBlock(node.type)) {
            // fb_inc/fb_dec can use either 'address' field or 'in1'/'out' fields
            bool isIncDec = strEqI(node.type, "fb_inc") || strEqI(node.type, "fb_dec");
            
            if (isIncDec) {
                // INC/DEC operations: accept 'address' OR 'in1'/'out', but check for conflicts
                bool hasAddress = node.address[0] != '\0';
                bool hasIn1 = node.in1[0] != '\0';
                bool hasOut = node.out[0] != '\0';
                
                // If both address and in1/out are provided, they must match
                if (hasAddress && hasIn1 && !strEqI(node.address, node.in1)) {
                    addError("INC/DEC: address and in1 fields don't match", nodeIdx);
                }
                if (hasAddress && hasOut && !strEqI(node.address, node.out)) {
                    addError("INC/DEC: address and out fields don't match", nodeIdx);
                }
                if (hasIn1 && hasOut && !strEqI(node.in1, node.out)) {
                    addError("INC/DEC: in1 and out fields don't match", nodeIdx);
                }
                
                // Must have at least one way to specify the address
                if (!hasAddress && !hasIn1 && !hasOut) {
                    addError("INC/DEC operation block missing address (use 'address', 'in1', or 'out')", nodeIdx);
                } else {
                    // Validate the symbol that was provided
                    const char* addrToValidate = hasAddress ? node.address : (hasIn1 ? node.in1 : node.out);
                    if ((symbol_count > 0 || sharedSymbols.symbol_count > 0) && !isValidSymbol(addrToValidate)) {
                        char msg[64];
                        int mi = 0;
                        const char* prefix = "Unknown symbol '";
                        while (*prefix && mi < 40) msg[mi++] = *prefix++;
                        int ai = 0;
                        while (addrToValidate[ai] && mi < 58) msg[mi++] = addrToValidate[ai++];
                        msg[mi++] = '\'';
                        msg[mi] = '\0';
                        addError(msg, nodeIdx, addrToValidate);
                    }
                }
            } else {
                // Other operation blocks use in1/in2/out fields
                bool needsIn1 = true;
                bool needsIn2 = !strEqI(node.type, "fb_not") && !strEqI(node.type, "fb_move");
                bool needsOut = true;

                if (needsIn1 && node.in1[0] == '\0') {
                    addError("Operation block missing input 1 (in1)", nodeIdx);
                } else if (needsIn1 && (symbol_count > 0 || sharedSymbols.symbol_count > 0) && !isValidSymbol(node.in1)) {
                    char msg[64];
                    int mi = 0;
                    const char* prefix = "Unknown symbol '";
                    while (*prefix && mi < 40) msg[mi++] = *prefix++;
                    int ai = 0;
                    while (node.in1[ai] && mi < 58) msg[mi++] = node.in1[ai++];
                    msg[mi++] = '\'';
                    msg[mi] = '\0';
                    addError(msg, nodeIdx, node.in1);
                }

                if (needsIn2 && node.in2[0] == '\0') {
                    addError("Operation block missing input 2 (in2)", nodeIdx);
                } else if (needsIn2 && (symbol_count > 0 || sharedSymbols.symbol_count > 0) && !isValidSymbol(node.in2)) {
                    char msg[64];
                    int mi = 0;
                    const char* prefix = "Unknown symbol '";
                    while (*prefix && mi < 40) msg[mi++] = *prefix++;
                    int ai = 0;
                    while (node.in2[ai] && mi < 58) msg[mi++] = node.in2[ai++];
                    msg[mi++] = '\'';
                    msg[mi] = '\0';
                    addError(msg, nodeIdx, node.in2);
                }

                if (needsOut && node.out[0] == '\0') {
                    addError("Operation block missing output (out)", nodeIdx);
                } else if (needsOut && (symbol_count > 0 || sharedSymbols.symbol_count > 0) && !isValidSymbol(node.out)) {
                    char msg[64];
                    int mi = 0;
                    const char* prefix = "Unknown symbol '";
                    while (*prefix && mi < 40) msg[mi++] = *prefix++;
                    int ai = 0;
                    while (node.out[ai] && mi < 58) msg[mi++] = node.out[ai++];
                    msg[mi++] = '\'';
                    msg[mi] = '\0';
                    addError(msg, nodeIdx, node.out);
                }
            }
        }

        if (isComparator(node.type)) {
            if (node.in1[0] == '\0') {
                addError("Comparator missing input 1 (in1)", nodeIdx);
            } else if ((symbol_count > 0 || sharedSymbols.symbol_count > 0) && !isValidSymbol(node.in1)) {
                char msg[64];
                int mi = 0;
                const char* prefix = "Unknown symbol '";
                while (*prefix && mi < 40) msg[mi++] = *prefix++;
                int ai = 0;
                while (node.in1[ai] && mi < 58) msg[mi++] = node.in1[ai++];
                msg[mi++] = '\'';
                msg[mi] = '\0';
                addError(msg, nodeIdx, node.in1);
            }

            if (node.in2[0] == '\0') {
                addError("Comparator missing input 2 (in2)", nodeIdx);
            } else if ((symbol_count > 0 || sharedSymbols.symbol_count > 0) && !isValidSymbol(node.in2)) {
                char msg[64];
                int mi = 0;
                const char* prefix = "Unknown symbol '";
                while (*prefix && mi < 40) msg[mi++] = *prefix++;
                int ai = 0;
                while (node.in2[ai] && mi < 58) msg[mi++] = node.in2[ai++];
                msg[mi++] = '\'';
                msg[mi] = '\0';
                addError(msg, nodeIdx, node.in2);
            }
        }

        return true;  // Always return true to continue linting
    }

    // Check if address is a raw memory address (starts with M, X, Y, K, S, T, C, or number)
    // Also handles literal values prefixed with # (e.g., #100)
    bool isRawAddress(const char* addr) {
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

    // Override validateConnection to continue collecting errors
    bool validateConnection(int connIdx) override {
        if (connIdx < 0 || connIdx >= connection_count) return true;
        Connection& conn = connections[connIdx];

        // Check that all sources exist
        for (int s = 0; s < conn.source_count; s++) {
            int srcIdx = findNodeById(conn.sources[s]);
            if (srcIdx < 0) {
                // Add error at position 1,1 since we don't have a node
                if (problem_count < LADDER_MAX_LINT_PROBLEMS) {
                    LinterProblem& p = problems[problem_count++];
                    p.type = LINT_ERROR;
                    p.line = 1;
                    p.column = 1;
                    p.length = 1;
                    p.lang = 3;
                    int ti = 0;
                    while (conn.sources[s][ti] && ti < 63) { p.token_buf[ti] = conn.sources[s][ti]; ti++; }
                    p.token_buf[ti] = '\0';
                    p.token_text = p.token_buf;
                    char msg[64];
                    int mi = 0;
                    const char* prefix = "Connection source '";
                    while (*prefix && mi < 40) msg[mi++] = *prefix++;
                    int si = 0;
                    while (conn.sources[s][si] && mi < 55) msg[mi++] = conn.sources[s][si++];
                    const char* suffix = "' not found";
                    while (*suffix && mi < 63) msg[mi++] = *suffix++;
                    msg[mi] = '\0';
                    int mj = 0;
                    while (msg[mj] && mj < 63) { p.message[mj] = msg[mj]; mj++; }
                    p.message[mj] = '\0';
                    has_error = true;
                }
                continue;
            }

            // Check that destinations have x > source x
            GraphNode& srcNode = nodes[srcIdx];
            for (int d = 0; d < conn.dest_count; d++) {
                int destIdx = findNodeById(conn.destinations[d]);
                if (destIdx >= 0) {
                    GraphNode& destNode = nodes[destIdx];
                    if (destNode.x <= srcNode.x) {
                        addConnectionError("Connection flows backward (destination must be right of source)", connIdx, destIdx);
                    }
                }
            }
        }

        // Check that all destinations exist
        for (int d = 0; d < conn.dest_count; d++) {
            int destIdx = findNodeById(conn.destinations[d]);
            if (destIdx < 0) {
                if (problem_count < LADDER_MAX_LINT_PROBLEMS) {
                    LinterProblem& p = problems[problem_count++];
                    p.type = LINT_ERROR;
                    p.line = 1;
                    p.column = 1;
                    p.length = 1;
                    p.lang = 3;
                    int ti = 0;
                    while (conn.destinations[d][ti] && ti < 63) { p.token_buf[ti] = conn.destinations[d][ti]; ti++; }
                    p.token_buf[ti] = '\0';
                    p.token_text = p.token_buf;
                    char msg[64];
                    int mi = 0;
                    const char* prefix = "Connection destination '";
                    while (*prefix && mi < 40) msg[mi++] = *prefix++;
                    int di = 0;
                    while (conn.destinations[d][di] && mi < 55) msg[mi++] = conn.destinations[d][di++];
                    const char* suffix = "' not found";
                    while (*suffix && mi < 63) msg[mi++] = *suffix++;
                    msg[mi] = '\0';
                    int mj = 0;
                    while (msg[mj] && mj < 63) { p.message[mj] = msg[mj]; mj++; }
                    p.message[mj] = '\0';
                    has_error = true;
                }
            }
        }

        return true;  // Always return true to continue linting
    }

    // Check if two nodes are connected (either direction)
    bool nodesAreConnected(int nodeIdxA, int nodeIdxB) {
        if (nodeIdxA < 0 || nodeIdxA >= node_count) return false;
        if (nodeIdxB < 0 || nodeIdxB >= node_count) return false;
        
        const char* idA = nodes[nodeIdxA].id;
        const char* idB = nodes[nodeIdxB].id;
        
        for (int c = 0; c < connection_count; c++) {
            Connection& conn = connections[c];
            bool hasA = false, hasB = false;
            
            // Check if A is a source and B is a destination
            for (int s = 0; s < conn.source_count; s++) {
                if (strEqI(conn.sources[s], idA)) hasA = true;
                if (strEqI(conn.sources[s], idB)) hasB = true;
            }
            for (int d = 0; d < conn.dest_count; d++) {
                if (strEqI(conn.destinations[d], idA)) hasA = true;
                if (strEqI(conn.destinations[d], idB)) hasB = true;
            }
            
            // If both nodes are in the same connection (one as source, one as dest)
            if (hasA && hasB) {
                // Verify it's actually a source->dest relationship
                bool aIsSource = false, bIsDest = false;
                bool bIsSource = false, aIsDest = false;
                for (int s = 0; s < conn.source_count; s++) {
                    if (strEqI(conn.sources[s], idA)) aIsSource = true;
                    if (strEqI(conn.sources[s], idB)) bIsSource = true;
                }
                for (int d = 0; d < conn.dest_count; d++) {
                    if (strEqI(conn.destinations[d], idA)) aIsDest = true;
                    if (strEqI(conn.destinations[d], idB)) bIsDest = true;
                }
                if ((aIsSource && bIsDest) || (bIsSource && aIsDest)) {
                    return true;
                }
            }
        }
        return false;
    }

    // Check if two nodes are touching horizontally (adjacent on X axis only)
    // Adjacent means they are on the same row (same Y) and X differs by exactly 1
    bool nodesAreTouching(int nodeIdxA, int nodeIdxB) {
        if (nodeIdxA < 0 || nodeIdxA >= node_count) return false;
        if (nodeIdxB < 0 || nodeIdxB >= node_count) return false;
        
        int dx = nodes[nodeIdxA].x - nodes[nodeIdxB].x;
        int dy = nodes[nodeIdxA].y - nodes[nodeIdxB].y;
        
        // Make absolute
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;
        
        // Adjacent horizontally only (same row, columns differ by 1)
        if (dy == 0 && dx == 1) return true;
        
        return false;
    }

    // Override validateGraph to continue collecting all errors
    bool validateGraph() override {
        // Check for duplicate nodes at same position
        for (int i = 0; i < node_count; i++) {
            for (int j = i + 1; j < node_count; j++) {
                if (nodes[i].x == nodes[j].x && nodes[i].y == nodes[j].y) {
                    char msg[64];
                    int mi = 0;
                    const char* prefix = "Overlaps with node '";
                    while (*prefix && mi < 40) msg[mi++] = *prefix++;
                    int ni = 0;
                    while (nodes[j].id[ni] && mi < 60) msg[mi++] = nodes[j].id[ni++];
                    msg[mi++] = '\'';
                    msg[mi] = '\0';
                    addError(msg, i, nodes[j].id);
                }
            }
        }

        // Check for touching nodes that are NOT connected (warning for each)
        for (int i = 0; i < node_count; i++) {
            for (int j = i + 1; j < node_count; j++) {
                if (nodesAreTouching(i, j) && !nodesAreConnected(i, j)) {
                    // Add warning for first node
                    char msg1[96];
                    int mi = 0;
                    const char* prefix = "Adjacent to '";
                    while (*prefix && mi < 40) msg1[mi++] = *prefix++;
                    int ni = 0;
                    while (nodes[j].id[ni] && mi < 70) msg1[mi++] = nodes[j].id[ni++];
                    const char* suffix = "' but not connected";
                    while (*suffix && mi < 95) msg1[mi++] = *suffix++;
                    msg1[mi] = '\0';
                    addWarning(msg1, i, nodes[j].id);
                    
                    // Add warning for second node
                    char msg2[96];
                    mi = 0;
                    prefix = "Adjacent to '";
                    while (*prefix && mi < 40) msg2[mi++] = *prefix++;
                    ni = 0;
                    while (nodes[i].id[ni] && mi < 70) msg2[mi++] = nodes[i].id[ni++];
                    suffix = "' but not connected";
                    while (*suffix && mi < 95) msg2[mi++] = *suffix++;
                    msg2[mi] = '\0';
                    addWarning(msg2, j, nodes[i].id);
                }
            }
        }

        // Validate all nodes
        for (int i = 0; i < node_count; i++) {
            validateNode(i);
        }

        // Validate all connections
        for (int i = 0; i < connection_count; i++) {
            validateConnection(i);
        }

        // Check for dead nodes (nodes with no connections at all)
        for (int i = 0; i < node_count; i++) {
            if (isTapNode(nodes[i].type)) continue;

            bool hasInputs = nodeHasInputs(i);
            bool hasOutputs = nodeHasOutputs(i);

            if (isInputNode(nodes[i].type) && !hasOutputs) {
                addWarning("Contact not connected to any output", i);
            }

            if (isTerminationNode(nodes[i].type) && !hasInputs) {
                addWarning("Output has no inputs", i);
            }

            if (!hasInputs && !hasOutputs) {
                addError("Node is completely disconnected", i);
            }

            // Check if input nodes (contacts, comparators) or operation blocks 
            // without input connections are not at the power rail (x=0)
            // Grid is 0-based internally, so x=0 is the power rail (user sees it as x=1)
            if (!hasInputs && nodes[i].x != 0) {
                if (isInputNode(nodes[i].type)) {
                    addWarning("Input node has no connection and is not at the power rail", i);
                } else if (isOperationBlock(nodes[i].type)) {
                    addWarning("Operation block has no input connection and is not at the power rail", i);
                }
            }
        }

        // Count errors
        int errorCount = 0;
        for (int i = 0; i < problem_count; i++) {
            if (problems[i].type == LINT_ERROR) errorCount++;
        }

        return errorCount == 0;
    }
    
    // ============ Main Entry Points ============
    
    // Lint-only mode: parse, validate, but don't compile to STL
    bool lint(const char* src, int len) {
        source = src;
        length = len;
        
        // Use base class parseGraph to parse the JSON
        if (!parseGraph()) {
            // Parser error already added via setError override
            return false;
        }
        
        // Validate the graph structure (collects all problems)
        validateGraph();
        
        // Count actual errors
        int errorCount = 0;
        for (int i = 0; i < problem_count; i++) {
            if (problems[i].type == LINT_ERROR) errorCount++;
        }
        
        return errorCount == 0;
    }
    
    // Full compile mode: parse, validate, compile to STL
    bool parse(const char* src, int len) {
        source = src;
        length = len;
        
        // Parse the graph JSON
        if (!parseGraph()) {
            return false;
        }
        
        // Validate the graph structure (collects all problems)
        validateGraph();
        
        // Count actual errors
        int errorCount = 0;
        for (int i = 0; i < problem_count; i++) {
            if (problems[i].type == LINT_ERROR) {
                errorCount++;
            }
        }
        
        // Only compile if no errors
        if (errorCount == 0 && !compile()) {
            return false;
        }
        
        return errorCount == 0;
    }

    // Add a symbol to the symbol table for validation
    void loadSymbol(const char* name, const char* type, u32 address, u8 bit, bool is_bit, u8 type_size) {
        addSymbol(name, type, address, bit, is_bit, type_size);
    }
};


// ============================================================================
// Global Instance and WASM Exports
// ============================================================================

LadderLinter ladderLinter = LadderLinter();

extern "C" {

WASM_EXPORT void ladder_lint_load_from_stream() {
    streamRead(ladder_input_buffer, ladder_input_length, sizeof(ladder_input_buffer));
}

// Returns problem_count (to workaround WASM static reinit issue)
// Access output_len and problems via the accessor functions BEFORE calling this again
WASM_EXPORT int ladder_lint_run() {
    ladderLinter.lint(ladder_input_buffer, ladder_input_length);
    return ladderLinter.problem_count;
}

// Returns problem_count (to workaround WASM static reinit issue)
// Access output_len and problems via the accessor functions BEFORE calling this again
WASM_EXPORT int ladder_lint_compile() {
    ladderLinter.parse(ladder_input_buffer, ladder_input_length);
    return ladderLinter.problem_count;
}

WASM_EXPORT int ladder_lint_problem_count() {
    return ladderLinter.problem_count;
}

// Get pointer to problems array - use this to read all problems from memory in one call
// to avoid reinit issues with individual accessor functions
WASM_EXPORT LinterProblem* ladder_lint_get_problems_pointer() {
    return ladderLinter.problems;
}

WASM_EXPORT int ladder_lint_get_problem_type(int index) {
    if (index < 0 || index >= ladderLinter.problem_count) return -1;
    return ladderLinter.problems[index].type;
}

WASM_EXPORT int ladder_lint_get_problem_line(int index) {
    if (index < 0 || index >= ladderLinter.problem_count) return -1;
    return ladderLinter.problems[index].line;
}

WASM_EXPORT int ladder_lint_get_problem_column(int index) {
    if (index < 0 || index >= ladderLinter.problem_count) return -1;
    return ladderLinter.problems[index].column;
}

WASM_EXPORT int ladder_lint_get_problem_length(int index) {
    if (index < 0 || index >= ladderLinter.problem_count) return -1;
    return ladderLinter.problems[index].length;
}

WASM_EXPORT const char* ladder_lint_get_problem_message(int index) {
    if (index < 0 || index >= ladderLinter.problem_count) return "";
    return ladderLinter.problems[index].message;
}

WASM_EXPORT const char* ladder_lint_get_output() {
    return ladderLinter.output;
}

WASM_EXPORT int ladder_lint_get_output_length() {
    return ladderLinter.output_len;
}

WASM_EXPORT bool ladder_lint_has_error() {
    return ladderLinter.hasError();
}

WASM_EXPORT const char* ladder_lint_get_error() {
    return ladderLinter.getErrorMsg();
}

WASM_EXPORT void ladder_lint_clear() {
    ladderLinter.clearProblems();
}

// Symbol table management
WASM_EXPORT void ladder_lint_add_symbol(const char* name, const char* type, u32 address, u8 bit, bool is_bit, u8 type_size) {
    ladderLinter.addSymbol(name, type, address, bit, is_bit, type_size);
}

WASM_EXPORT void ladder_lint_clear_symbols() {
    ladderLinter.symbol_count = 0;
}

WASM_EXPORT int ladder_lint_get_problem_lang(int index) {
    if (index < 0 || index >= ladderLinter.problem_count) return -1;
    return ladderLinter.problems[index].lang;
}

WASM_EXPORT const char* ladder_lint_get_problem_token(int index) {
    if (index < 0 || index >= ladderLinter.problem_count) return "";
    return ladderLinter.problems[index].token_text ? ladderLinter.problems[index].token_text : "";
}

} // extern "C"

#endif // __WASM__
