// ladder-compiler.h - Ladder Graph to STL Compiler
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

#include "stl-compiler.h"
#include "shared-symbols.h"

// Include modular ladder compiler components
#include "ladder/ladder-types.h"
#include "ladder/ladder-node-types.h"
#include "ladder/ladder-emit.h"
#include "ladder/ladder-json-parser.h"
#include "ladder/ladder-graph-helpers.h"
#include "ladder/ladder-validation.h"


// ============================================================================
// Ladder Graph to STL Compiler
// ============================================================================
//
// Compiles ladder logic represented as a graph structure (nodes + connections)
// directly to STL code. This is the new format used by VovkPLC Editor.
//
// JSON format:
//   {
//     "nodes": [
//       { "id": "n1", "type": "contact_no", "address": "X0.0", "x": 0, "y": 0 },
//       { "id": "n2", "type": "coil", "address": "Y0.0", "x": 100, "y": 0 }
//     ],
//     "connections": [
//       { "sources": ["n1"], "destinations": ["n2"] }
//     ]
//   }
//
// Node types:
//   Contacts: contact_no, contact_nc, contact_pos, contact_neg
//   Coils: coil, coil_set, coil_rset, coil_neg
//   Timers: timer_on, timer_off, timer_pulse
//   Counters: counter_up, counter_down
//   Special: tap
//
// Connection semantics:
//   - Multiple sources = OR logic (sources are ORed together)
//   - Multiple destinations = Parallel branches (use SAVE/L BR/DROP BR)
//
// ============================================================================

class LadderGraphCompiler : public LadderJsonParser {
public:
    // Output STL buffer
    char output[LADDER_MAX_OUTPUT];
    int output_len;

    // Edge memory counter for auto-generated edge state bits
    int edge_mem_counter;

    // Label counter for conditional jumps
    int label_counter;

    // Indentation level for nested structures
    int indent_level;

    LadderGraphCompiler() { clearState(); }

    // Virtual destructor for proper cleanup in derived classes
    virtual ~LadderGraphCompiler() = default;

    virtual void clearState() {
        source = nullptr;
        length = 0;
        pos = 0;
        error_msg[0] = '\0';
        has_error = false;
        output[0] = '\0';
        output_len = 0;
        edge_mem_counter = 800;
        label_counter = 0;
        node_count = 0;
        connection_count = 0;
        indent_level = 0;
    }

    // Get/Set edge memory counter for cross-block continuity
    int getEdgeMemCounter() const { return edge_mem_counter; }
    void setEdgeMemCounter(int value) { edge_mem_counter = value; }

    // Error state accessors (public interface for WASM exports)
    bool hasError() const { return has_error; }
    const char* getErrorMsg() const { return error_msg; }

    // ============ Output Helpers ============

    void emitIndent() {
        ladderEmitIndent(output, output_len, LADDER_MAX_OUTPUT, indent_level);
    }

    void emit(const char* s) {
        ladderEmit(output, output_len, LADDER_MAX_OUTPUT, s);
    }

    void emitChar(char c) {
        ladderEmitChar(output, output_len, LADDER_MAX_OUTPUT, c);
    }

    void emitLine(const char* s) {
        ladderEmitLine(output, output_len, LADDER_MAX_OUTPUT, indent_level, s);
    }

    void emitInt(int val) {
        ladderEmitInt(output, output_len, LADDER_MAX_OUTPUT, val);
    }

    void emitTypedAddress(const char* addr, char typeSuffix) {
        ladderEmitTypedAddress(output, output_len, LADDER_MAX_OUTPUT, addr, typeSuffix);
    }

    // ============ Node Type Wrappers ============
    // These wrap the free functions from ladder-node-types.h for class-based usage

    bool strEqI(const char* a, const char* b) { return ladderStrEqI(a, b); }
    bool isContact(const char* type) { return ladderIsContact(type); }
    bool isCoil(const char* type) { return ladderIsCoil(type); }
    bool isTimer(const char* type) { return ladderIsTimer(type); }
    bool isCounter(const char* type) { return ladderIsCounter(type); }
    bool isOperationBlock(const char* type) { return ladderIsOperationBlock(type); }
    bool isComparator(const char* type) { return ladderIsComparator(type); }
    bool isTerminationNode(const char* type) { return ladderIsTerminationNode(type); }
    bool isOutputNode(const char* type) { return ladderIsOutputNode(type); }
    bool isInputNode(const char* type) { return ladderIsInputNode(type); }
    bool isFunctionBlock(const char* type) { return ladderIsFunctionBlock(type); }
    bool isTapNode(const char* type) { return ladderIsTapNode(type); }
    bool isKnownNodeType(const char* type) { return ladderIsKnownNodeType(type); }
    bool isRawAddress(const char* addr) { return ladderIsRawAddress(addr); }
    char getSiemensTypeSuffix(const char* data_type) { return ladderGetSiemensTypeSuffix(data_type); }
    char getSiemensAddressSizePrefix(char typeSuffix) { return ladderGetSiemensAddressSizePrefix(typeSuffix); }

    // ============ Graph Helper Wrappers ============

    int findNodeById(const char* nodeId) {
        return ladderFindNodeById(nodes, node_count, nodeId);
    }

    bool nodeHasInputs(int nodeIdx) {
        return ladderNodeHasInputs(nodes, node_count, connections, connection_count, nodeIdx);
    }

    bool nodeHasOutputs(int nodeIdx) {
        return ladderNodeHasOutputs(nodes, node_count, connections, connection_count, nodeIdx);
    }

    void getInputConnections(const char* nodeId, int* connIndices, int& count, int maxCount) {
        ladderGetInputConnections(connections, connection_count, nodeId, connIndices, count, maxCount);
    }

    void getOutputConnections(const char* nodeId, int* connIndices, int& count, int maxCount) {
        ladderGetOutputConnections(connections, connection_count, nodeId, connIndices, count, maxCount);
    }

    void getSourceNodes(const char* nodeId, int* nodeIndices, int& count, int maxCount) {
        ladderGetSourceNodes(nodes, node_count, connections, connection_count, nodeId, nodeIndices, count, maxCount);
    }

    void getDestNodes(const char* nodeId, int* nodeIndices, int& count, int maxCount) {
        ladderGetDestNodes(nodes, node_count, connections, connection_count, nodeId, nodeIndices, count, maxCount);
    }

    void sortNodesByPosition() {
        ladderSortNodesByPosition(nodes, node_count);
    }

    void findStartNodes(int* startNodes, int& count, int maxCount) {
        ladderFindStartNodes(nodes, node_count, connections, connection_count, startNodes, count, maxCount);
    }

    void findEndNodes(int* endNodes, int& count, int maxCount) {
        ladderFindEndNodes(nodes, node_count, connections, connection_count, endNodes, count, maxCount);
    }

    void findOutputNodes(int* outputNodes, int& count, int maxCount) {
        ladderFindOutputNodes(nodes, node_count, outputNodes, count, maxCount);
    }

    void sortSourcesByPosition(int* sourceIndices, int sourceCount) {
        ladderSortSourcesByPosition(nodes, sourceIndices, sourceCount);
    }

    // ============ Validation Helpers ============

    // Validate a symbol name exists in the shared symbol table
    bool validateSymbolExists(const char* addr, int nodeIdx) {
        if (!addr || addr[0] == '\0') return true;  // Empty handled elsewhere
        if (isRawAddress(addr)) return true;  // Raw addresses don't need lookup

        // Check shared symbol table (if populated)
        if (sharedSymbols.symbol_count > 0) {
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
                
                // Check if base symbol exists
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
                        // Property not found in struct
                        char msg[64];
                        int mi = 0;
                        const char* prefix = "Unknown property '";
                        while (*prefix && mi < 40) msg[mi++] = *prefix++;
                        int ai = 0;
                        while (propName[ai] && mi < 55) msg[mi++] = propName[ai++];
                        msg[mi++] = '\'';
                        msg[mi] = '\0';
                        reportNodeError(msg, nodeIdx);
                        return false;
                    }
                    // baseSym exists but is not a struct type - this address has a dot but the base isn't a struct
                    // Fall through to treat as regular symbol lookup (might be a typo)
                } else {
                    // Base symbol not found in SharedSymbol table.
                    // Check if it's a known UserStructType (e.g., DB alias like "Motor" or DB number like "DB1")
                    UserStructType* directStruct = findUserStructType(baseName);
                    if (directStruct) {
                        // Verify the property exists in the struct type
                        const char* propName = addr + dotPos + 1;
                        if (directStruct->findField(propName)) {
                            return true;  // Valid DB/struct property access
                        }
                        char msg[64];
                        int mi = 0;
                        const char* prefix = "Unknown property '";
                        while (*prefix && mi < 40) msg[mi++] = *prefix++;
                        int ai = 0;
                        while (propName[ai] && mi < 55) msg[mi++] = propName[ai++];
                        msg[mi++] = '\'';
                        msg[mi] = '\0';
                        reportNodeError(msg, nodeIdx);
                        return false;
                    }
                    // Truly unknown base symbol
                    char msg[64];
                    int mi = 0;
                    const char* prefix = "Unknown symbol '";
                    while (*prefix && mi < 40) msg[mi++] = *prefix++;
                    int ai = 0;
                    while (baseName[ai] && mi < 58) msg[mi++] = baseName[ai++];
                    msg[mi++] = '\'';
                    msg[mi] = '\0';
                    reportNodeError(msg, nodeIdx);
                    return false;
                }
            }
            
            // Check direct symbol
            if (!sharedSymbols.findSymbol(addr)) {
                char msg[64];
                int mi = 0;
                const char* prefix = "Unknown symbol '";
                while (*prefix && mi < 40) msg[mi++] = *prefix++;
                int ai = 0;
                while (addr[ai] && mi < 58) msg[mi++] = addr[ai++];
                msg[mi++] = '\'';
                msg[mi] = '\0';
                reportNodeError(msg, nodeIdx);
                return false;
            }
        }
        return true;
    }

    // Virtual method to report a validation error with node position
    // Derived class (linter) can override to collect multiple errors
    virtual void reportNodeError(const char* msg, int nodeIdx) {
        if (has_error) return; // Base class stops at first error
        char fullMsg[256];
        int mi = 0;
        // Add node position info
        if (nodeIdx >= 0 && nodeIdx < node_count) {
            LadderGraphNode& node = nodes[nodeIdx];
            const char* prefix = "Node '";
            while (*prefix && mi < 200) fullMsg[mi++] = *prefix++;
            int ni = 0;
            while (node.id[ni] && mi < 200) fullMsg[mi++] = node.id[ni++];
            const char* mid = "' at (";
            while (*mid && mi < 200) fullMsg[mi++] = *mid++;
            // Convert x to string
            char numBuf[16];
            int x = node.x;
            int nlen = 0;
            if (x < 0) { numBuf[nlen++] = '-'; x = -x; }
            if (x == 0) numBuf[nlen++] = '0';
            else {
                int start = nlen;
                while (x > 0) { numBuf[nlen++] = '0' + (x % 10); x /= 10; }
                for (int r = start; r < start + (nlen - start) / 2; r++) {
                    char tmp = numBuf[r];
                    numBuf[r] = numBuf[nlen - 1 - (r - start)];
                    numBuf[nlen - 1 - (r - start)] = tmp;
                }
            }
            for (int ni2 = 0; ni2 < nlen && mi < 200; ni2++) fullMsg[mi++] = numBuf[ni2];
            fullMsg[mi++] = ',';
            // Convert y to string
            int y = node.y;
            nlen = 0;
            if (y < 0) { numBuf[nlen++] = '-'; y = -y; }
            if (y == 0) numBuf[nlen++] = '0';
            else {
                int start = nlen;
                while (y > 0) { numBuf[nlen++] = '0' + (y % 10); y /= 10; }
                for (int r = start; r < start + (nlen - start) / 2; r++) {
                    char tmp = numBuf[r];
                    numBuf[r] = numBuf[nlen - 1 - (r - start)];
                    numBuf[nlen - 1 - (r - start)] = tmp;
                }
            }
            for (int ni2 = 0; ni2 < nlen && mi < 200; ni2++) fullMsg[mi++] = numBuf[ni2];
            const char* end = "): ";
            while (*end && mi < 200) fullMsg[mi++] = *end++;
        }
        while (*msg && mi < 255) fullMsg[mi++] = *msg++;
        fullMsg[mi] = '\0';
        setError(fullMsg);
    }

    // Virtual method to report a validation warning (non-fatal)
    virtual void reportNodeWarning(const char* msg, int nodeIdx) {
        // Base class ignores warnings during compilation
        (void)msg; (void)nodeIdx;
    }

    // Virtual method to report a connection validation error
    virtual void reportConnectionError(const char* msg, int connIdx, int nodeIdx = -1) {
        if (has_error) return; // Base class stops at first error
        char fullMsg[256];
        int mi = 0;
        const char* prefix = "Connection c[";
        while (*prefix && mi < 200) fullMsg[mi++] = *prefix++;
        // Convert connIdx to string
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
        for (int ni = 0; ni < nlen && mi < 200; ni++) fullMsg[mi++] = numBuf[ni];
        const char* mid = "]: ";
        while (*mid && mi < 200) fullMsg[mi++] = *mid++;
        while (*msg && mi < 255) fullMsg[mi++] = *msg++;
        fullMsg[mi] = '\0';
        setError(fullMsg);
        (void)nodeIdx; // nodeIdx available for derived classes
    }

    // ============ TAP and Downstream Chain Helpers ============

    // Check if a termination node should emit TAP (has outgoing connections to continuation nodes)
    bool nodeNeedsTap(int nodeIdx) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return false;
        LadderGraphNode& node = nodes[nodeIdx];

        // Only COILS need TAP - timers/counters push their Q output to the stack
        if (!isCoil(node.type)) return false;

        // Check if node has outgoing connections to any downstream nodes
        int destIndices[LADDER_MAX_NODES];
        int destCount;
        getDestNodes(node.id, destIndices, destCount, LADDER_MAX_NODES);

        for (int d = 0; d < destCount; d++) {
            LadderGraphNode& dest = nodes[destIndices[d]];
            if (isInputNode(dest.type)) return true;
            if (isOutputNode(dest.type)) return true;
        }

        return false;
    }

    // Check if a node has a downstream chain (contact → output sequences after it)
    bool hasDownstreamChain(int nodeIdx) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return false;

        int destIndices[LADDER_MAX_NODES];
        int destCount;
        getDestNodes(nodes[nodeIdx].id, destIndices, destCount, LADDER_MAX_NODES);

        for (int d = 0; d < destCount; d++) {
            LadderGraphNode& dest = nodes[destIndices[d]];
            if (isInputNode(dest.type)) return true;
            if (isOutputNode(dest.type)) return true;
            if (isTimer(dest.type) || isCounter(dest.type)) return true;
        }
        return false;
    }

    // Check if a node has a downstream contact (input node)
    bool hasDownstreamContact(int nodeIdx) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return false;

        int destIndices[LADDER_MAX_NODES];
        int destCount;
        getDestNodes(nodes[nodeIdx].id, destIndices, destCount, LADDER_MAX_NODES);

        for (int d = 0; d < destCount; d++) {
            LadderGraphNode& dest = nodes[destIndices[d]];
            if (isInputNode(dest.type)) return true;
        }
        return false;
    }

    // Check if a node has downstream output nodes
    bool hasDownstreamOutput(int nodeIdx) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return false;

        int destIndices[LADDER_MAX_NODES];
        int destCount;
        getDestNodes(nodes[nodeIdx].id, destIndices, destCount, LADDER_MAX_NODES);

        for (int d = 0; d < destCount; d++) {
            LadderGraphNode& dest = nodes[destIndices[d]];
            if (isOutputNode(dest.type)) return true;
            if (isInputNode(dest.type)) {
                if (hasDownstreamChain(destIndices[d])) return true;
            }
        }
        return false;
    }

    // ============ STL Generation ============

    void generateEdgeAddress(char* edge_mem) {
        ladderGenerateEdgeAddress(edge_mem, edge_mem_counter);
    }

    // Emit STL for a contact node (including comparators)
    void emitContact(LadderGraphNode& node, bool isFirst, bool useOr = false) {
        if (isComparator(node.type)) {
            emitComparator(node, isFirst);
            return;
        }

        bool inverted = node.inverted || strEqI(node.type, "contact_nc");
        bool isPositive = strEqI(node.type, "contact_pos") || strEqI(node.trigger, "rising") || strEqI(node.trigger, "positive");
        bool isNegative = strEqI(node.type, "contact_neg") || strEqI(node.trigger, "falling") || strEqI(node.trigger, "negative");

        const char* instr;
        if (useOr) {
            instr = inverted ? "ON " : "O ";
        } else {
            instr = inverted ? "AN " : "A ";
        }

        emitIndent();
        emit(instr);
        emit(node.address);
        emitChar('\n');

        if (isPositive) {
            char edge_mem[16];
            generateEdgeAddress(edge_mem);
            emitIndent();
            emit("FP ");
            emit(edge_mem);
            emitChar('\n');
        } else if (isNegative) {
            char edge_mem[16];
            generateEdgeAddress(edge_mem);
            emitIndent();
            emit("FN ");
            emit(edge_mem);
            emitChar('\n');
        }
    }

    // Emit STL for a coil node
    void emitCoil(LadderGraphNode& node, bool emitTapAfter = false) {
        const char* instr = "= ";
        bool isInverted = node.inverted || strEqI(node.type, "coil_neg");

        if (strEqI(node.type, "coil_set")) instr = "S ";
        else if (strEqI(node.type, "coil_rset")) instr = "R ";
        else if (isInverted) {
            instr = "=N ";
        }

        emitIndent();
        emit(instr);
        emit(node.address);
        emitChar('\n');

        if (emitTapAfter && !isInverted) {
            emitLine("TAP");
        }
    }

    // Emit STL for a timer node
    void emitTimer(LadderGraphNode& node, bool emitTapAfter = false) {
        const char* instr = "TON";
        if (strEqI(node.type, "timer_off") || strEqI(node.type, "timer_tof")) instr = "TOF";
        else if (strEqI(node.type, "timer_pulse") || strEqI(node.type, "timer_tp")) instr = "TP";

        emitIndent();
        emit(instr);
        emit(" ");
        emit(node.address);
        emit(", ");
        if (node.preset_str[0] != '\0') {
            emit(node.preset_str);
        } else {
            emitInt(node.preset);
        }
        emitChar('\n');

        if (emitTapAfter) {
            emitLine("TAP");
        }
    }

    // Emit STL for a counter node
    void emitCounter(LadderGraphNode& node, bool emitTapAfter = false) {
        const char* instr = "CTU";
        if (strEqI(node.type, "counter_down") || strEqI(node.type, "counter_d")) instr = "CTD";

        emitIndent();
        emit(instr);
        emit(" ");
        emit(node.address);
        emit(", ");
        emitInt(node.preset);
        emitChar('\n');

        if (emitTapAfter) {
            emitLine("TAP");
        }
    }

    // Emit STL for an operation block
    void emitOperationBlock(LadderGraphNode& node, bool emitTapBefore = false, bool emitTapAfter = false) {
        char typeSuffix = getSiemensTypeSuffix(node.data_type);
        int skipLabel = label_counter++;

        if (emitTapBefore) {
            emitIndent();
            emit("TAP\n");
        }

        emitIndent();
        emit("JCN _op");
        emitInt(skipLabel);
        emitChar('\n');

        if (strEqI(node.type, "fb_inc") || strEqI(node.type, "fb_dec")) {
            bool isInc = strEqI(node.type, "fb_inc");
            emitIndent();
            emit(isInc ? "INC" : "DEC");
            emitChar(typeSuffix);
            emit(" ");
            emitTypedAddress(node.address, typeSuffix);
            emitChar('\n');
        }
        else if (strEqI(node.type, "fb_move") || strEqI(node.type, "fb_copy")) {
            emitIndent();
            emit("L ");
            emitTypedAddress(node.in1, typeSuffix);
            emitChar('\n');

            emitIndent();
            emit("T ");
            emitTypedAddress(node.out, typeSuffix);
            emitChar('\n');
        }
        else if (strEqI(node.type, "fb_add") || strEqI(node.type, "fb_sub") ||
            strEqI(node.type, "fb_mul") || strEqI(node.type, "fb_div")) {
            const char* mathOp = nullptr;
            if (strEqI(node.type, "fb_add")) mathOp = "+";
            else if (strEqI(node.type, "fb_sub")) mathOp = "-";
            else if (strEqI(node.type, "fb_mul")) mathOp = "*";
            else if (strEqI(node.type, "fb_div")) mathOp = "/";

            emitIndent();
            emit("L ");
            emitTypedAddress(node.in1, typeSuffix);
            emitChar('\n');

            emitIndent();
            emit(mathOp);
            emitChar(typeSuffix);
            emit(" ");
            emitTypedAddress(node.in2, typeSuffix);
            emitChar('\n');

            emitIndent();
            emit("T ");
            emitTypedAddress(node.out, typeSuffix);
            emitChar('\n');
        }
        else if (strEqI(node.type, "fb_mod")) {
            emitIndent();
            emit("L ");
            emitTypedAddress(node.in1, typeSuffix);
            emitChar('\n');

            emitIndent();
            emit("MOD ");
            emitTypedAddress(node.in2, typeSuffix);
            emitChar('\n');

            emitIndent();
            emit("T ");
            emitTypedAddress(node.out, typeSuffix);
            emitChar('\n');
        }
        else if (strEqI(node.type, "fb_and") || strEqI(node.type, "fb_or") ||
            strEqI(node.type, "fb_xor") || strEqI(node.type, "fb_shl") ||
            strEqI(node.type, "fb_shr") || strEqI(node.type, "fb_not")) {
            const char* bitwiseOp = nullptr;
            if (strEqI(node.type, "fb_and")) bitwiseOp = "AW";
            else if (strEqI(node.type, "fb_or")) bitwiseOp = "OW";
            else if (strEqI(node.type, "fb_xor")) bitwiseOp = "XOW";
            else if (strEqI(node.type, "fb_shl")) bitwiseOp = "SLW";
            else if (strEqI(node.type, "fb_shr")) bitwiseOp = "SRW";
            else if (strEqI(node.type, "fb_not")) bitwiseOp = "INVW";

            emitIndent();
            emit("L ");
            emitTypedAddress(node.in1, typeSuffix);
            emitChar('\n');

            if (!strEqI(node.type, "fb_not")) {
                emitIndent();
                emit(bitwiseOp);
                emit(" ");
                emitTypedAddress(node.in2, typeSuffix);
                emitChar('\n');
            } else {
                emitIndent();
                emit(bitwiseOp);
                emitChar('\n');
            }

            emitIndent();
            emit("T ");
            emitTypedAddress(node.out, typeSuffix);
            emitChar('\n');
        } else {
            setError("Unknown operation block type");
            return;
        }

        emit("_op");
        emitInt(skipLabel);
        emit(":\n");

        if (emitTapAfter) {
            emitLine("TAP");
        }
    }

    // Emit STL for a comparator block
    void emitComparator(LadderGraphNode& node, bool asFirstCondition) {
        char typeSuffix = getSiemensTypeSuffix(node.data_type);

        if (!asFirstCondition) {
            emitLine("A(");
            indent_level++;
        }

        const char* op = nullptr;
        if (strEqI(node.type, "cmp_eq") || strEqI(node.type, "fb_cmp_eq")) op = "==";
        else if (strEqI(node.type, "cmp_ne") || strEqI(node.type, "fb_cmp_ne")) op = "<>";
        else if (strEqI(node.type, "cmp_gt") || strEqI(node.type, "fb_cmp_gt")) op = ">";
        else if (strEqI(node.type, "cmp_lt") || strEqI(node.type, "fb_cmp_lt")) op = "<";
        else if (strEqI(node.type, "cmp_ge") || strEqI(node.type, "fb_cmp_ge") ||
            strEqI(node.type, "cmp_gte") || strEqI(node.type, "fb_cmp_gte")) op = ">=";
        else if (strEqI(node.type, "cmp_le") || strEqI(node.type, "fb_cmp_le") ||
            strEqI(node.type, "cmp_lte") || strEqI(node.type, "fb_cmp_lte")) op = "<=";

        if (!op) {
            setError("Unknown comparator type");
            return;
        }

        bool in2IsLiteral = true;
        const char* p = node.in2;
        if (*p == '-') p++;
        while (*p) {
            if (*p < '0' || *p > '9') {
                in2IsLiteral = false;
                break;
            }
            p++;
        }

        emitIndent();
        emit("L ");
        emitTypedAddress(node.in1, typeSuffix);
        emitChar('\n');

        if (in2IsLiteral) {
            emitIndent();
            emit(op);
            emitChar(typeSuffix);
            emit(" #");
            emit(node.in2);
            emitChar('\n');
        } else {
            emitIndent();
            emit("L ");
            emitTypedAddress(node.in2, typeSuffix);
            emitChar('\n');

            emitIndent();
            emit(op);
            emitChar(typeSuffix);
            emitChar('\n');
        }

        if (!asFirstCondition) {
            indent_level--;
            emitLine(")");
        }
    }

    // ============ Graph Traversal and Code Generation ============

    // Emit downstream chain from a node
    void emitDownstreamChain(int nodeIdx, bool* outputProcessed) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return;

        int destIndices[LADDER_MAX_NODES];
        int destCount;
        getDestNodes(nodes[nodeIdx].id, destIndices, destCount, LADDER_MAX_NODES);

        for (int d = 0; d < destCount; d++) {
            int destIdx = destIndices[d];
            LadderGraphNode& dest = nodes[destIdx];

            if (isInputNode(dest.type)) {
                emitContact(dest, true);

                int outputIndices[LADDER_MAX_NODES];
                int outputCount;
                getDestNodes(dest.id, outputIndices, outputCount, LADDER_MAX_NODES);

                for (int c = 0; c < outputCount; c++) {
                    int outIdx = outputIndices[c];
                    LadderGraphNode& outNode = nodes[outIdx];

                    if (isOutputNode(outNode.type) && !outNode.emitted) {
                        bool needsTapAfter = hasDownstreamContact(outIdx);
                        bool hasMoreOutputs = (c < outputCount - 1) || hasDownstreamOutput(outIdx);

                        if (isCoil(outNode.type)) {
                            emitCoil(outNode, needsTapAfter || hasMoreOutputs);
                        } else if (isOperationBlock(outNode.type)) {
                            emitOperationBlock(outNode, hasMoreOutputs, needsTapAfter);
                        }
                        outputProcessed[outIdx] = true;
                        outNode.emitted = true;

                        if (hasDownstreamChain(outIdx)) {
                            emitDownstreamChain(outIdx, outputProcessed);
                        }
                    }
                }
            } else if (isOutputNode(dest.type) && !dest.emitted) {
                bool needsTapAfter = hasDownstreamContact(destIdx);
                bool hasMoreOutputs = hasDownstreamOutput(destIdx);

                if (isCoil(dest.type)) {
                    emitCoil(dest, needsTapAfter || hasMoreOutputs);
                } else if (isOperationBlock(dest.type)) {
                    emitOperationBlock(dest, hasMoreOutputs, needsTapAfter);
                }
                outputProcessed[destIdx] = true;
                dest.emitted = true;

                if (hasDownstreamChain(destIdx)) {
                    emitDownstreamChain(destIdx, outputProcessed);
                }
            } else if ((isTimer(dest.type) || isCounter(dest.type)) && !dest.emitted) {
                bool needsTapAfter = hasDownstreamContact(destIdx);

                if (isTimer(dest.type)) {
                    emitTimer(dest, needsTapAfter);
                } else {
                    emitCounter(dest, needsTapAfter);
                }
                outputProcessed[destIdx] = true;
                dest.emitted = true;

                if (hasDownstreamChain(destIdx)) {
                    emitDownstreamChain(destIdx, outputProcessed);
                }
            }
        }
    }

    // Emit passthrough chain
    void emitPassthroughChain(int nodeIdx, bool* outputProcessed) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return;

        int destIndices[LADDER_MAX_NODES];
        int destCount;
        getDestNodes(nodes[nodeIdx].id, destIndices, destCount, LADDER_MAX_NODES);

        for (int d = 0; d < destCount; d++) {
            int destIdx = destIndices[d];
            LadderGraphNode& destNode = nodes[destIdx];

            if (!isCoil(destNode.type)) continue;
            if (outputProcessed[destIdx]) continue;

            bool needTap = nodeNeedsTap(destIdx);
            emitCoil(destNode, needTap);
            outputProcessed[destIdx] = true;
            if (needTap) destNode.emitted = true;

            if (needTap) {
                emitPassthroughChain(destIdx, outputProcessed);
            }
        }
    }

    // Check if all sources are simple contacts
    bool allSourcesAreSimpleContacts(int* sourceIndices, int sourceCount) {
        for (int i = 0; i < sourceCount; i++) {
            LadderGraphNode& node = nodes[sourceIndices[i]];
            if (!isContact(node.type)) return false;

            int srcIndices[LADDER_MAX_NODES];
            int srcCount;
            getSourceNodes(node.id, srcIndices, srcCount, LADDER_MAX_NODES);
            if (srcCount > 0) return false;
        }
        return true;
    }

    // Collect all nodes in a condition chain
    void collectChainNodes(int nodeIdx, int* chainNodes, int& count, int maxCount) {
        if (nodeIdx < 0 || nodeIdx >= node_count || count >= maxCount) return;
        LadderGraphNode& node = nodes[nodeIdx];
        if (node.visited) return;
        node.visited = true;

        chainNodes[count++] = nodeIdx;

        int srcIndices[LADDER_MAX_NODES];
        int srcCount;
        getSourceNodes(node.id, srcIndices, srcCount, LADDER_MAX_NODES);
        for (int i = 0; i < srcCount && count < maxCount; i++) {
            collectChainNodes(srcIndices[i], chainNodes, count, maxCount);
        }

        node.visited = false;
    }

    // Find common ancestor
    int findCommonAncestor(int* sourceIndices, int sourceCount) {
        if (sourceCount < 2) return -1;

        int ancestors0[LADDER_MAX_NODES];
        int ancestorCount0 = 0;
        for (int i = 0; i < node_count; i++) nodes[i].visited = false;
        collectChainNodes(sourceIndices[0], ancestors0, ancestorCount0, LADDER_MAX_NODES);
        for (int i = 0; i < node_count; i++) nodes[i].visited = false;

        for (int a = 0; a < ancestorCount0; a++) {
            int candidateIdx = ancestors0[a];
            if (candidateIdx == sourceIndices[0]) continue;

            bool isCommonToAll = true;
            for (int s = 1; s < sourceCount && isCommonToAll; s++) {
                int ancestorsS[LADDER_MAX_NODES];
                int ancestorCountS = 0;
                for (int i = 0; i < node_count; i++) nodes[i].visited = false;
                collectChainNodes(sourceIndices[s], ancestorsS, ancestorCountS, LADDER_MAX_NODES);
                for (int i = 0; i < node_count; i++) nodes[i].visited = false;

                bool found = false;
                for (int as = 0; as < ancestorCountS; as++) {
                    if (ancestorsS[as] == candidateIdx) {
                        found = true;
                        break;
                    }
                }
                if (!found) isCommonToAll = false;
            }

            if (isCommonToAll) return candidateIdx;
        }

        return -1;
    }

    // Check if source is simple for OR
    bool isSimpleOrSource(int nodeIdx, int* alreadyEmittedChain, int emittedCount) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return false;
        LadderGraphNode& node = nodes[nodeIdx];

        if (!isContact(node.type)) return false;

        int srcIndices[LADDER_MAX_NODES];
        int srcCount;
        getSourceNodes(node.id, srcIndices, srcCount, LADDER_MAX_NODES);

        if (srcCount == 0) return true;

        for (int i = 0; i < srcCount; i++) {
            bool found = false;
            for (int j = 0; j < emittedCount; j++) {
                if (srcIndices[i] == alreadyEmittedChain[j]) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        return true;
    }

    // Recursively emit condition logic leading to a node
    bool emitConditionForNode(int nodeIdx, int depth, bool isOrBranch, bool stopAtEmittedOutputs = true) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return false;
        LadderGraphNode& node = nodes[nodeIdx];

        if (node.visited) return false;
        node.visited = true;

        if (node.emitted) {
            node.visited = false;
            return false;
        }

        int sourceIndices[LADDER_MAX_NODES];
        int sourceCount;
        getSourceNodes(node.id, sourceIndices, sourceCount, LADDER_MAX_NODES);

        if (isContact(node.type)) {
            if (sourceCount > 0) {
                if (sourceCount > 1) {
                    emitOrSources(sourceIndices, sourceCount, depth, stopAtEmittedOutputs);
                } else {
                    emitConditionForNode(sourceIndices[0], depth + 1, isOrBranch, stopAtEmittedOutputs);
                }
            }

            emitContact(node, sourceCount == 0);
            node.visited = false;
            return true;
        }

        if (isComparator(node.type)) {
            if (sourceCount > 0) {
                if (sourceCount > 1) {
                    emitOrSources(sourceIndices, sourceCount, depth, stopAtEmittedOutputs);
                } else {
                    emitConditionForNode(sourceIndices[0], depth + 1, isOrBranch, stopAtEmittedOutputs);
                }
            }

            emitComparator(node, sourceCount == 0);
            node.visited = false;
            return true;
        }

        if (isFunctionBlock(node.type)) {
            if (sourceCount > 1) {
                emitOrSources(sourceIndices, sourceCount, depth, stopAtEmittedOutputs);
            } else if (sourceCount == 1) {
                emitConditionForNode(sourceIndices[0], depth + 1, isOrBranch, stopAtEmittedOutputs);
            }

            if (isTimer(node.type)) {
                emitTimer(node);
            } else if (isCounter(node.type)) {
                emitCounter(node);
            }
            node.visited = false;
            return true;
        }

        if (isCoil(node.type)) {
            if (node.emitted) {
                emitContact(node, depth == 0 && !isOrBranch);
                node.visited = false;
                return true;
            }

            if (sourceCount > 1) {
                emitOrSources(sourceIndices, sourceCount, depth, stopAtEmittedOutputs);
            } else if (sourceCount == 1) {
                emitConditionForNode(sourceIndices[0], depth + 1, isOrBranch, stopAtEmittedOutputs);
            }

            bool needsTap = hasDownstreamContact(nodeIdx) || hasDownstreamOutput(nodeIdx);
            emitCoil(node, needsTap);
            node.emitted = true;
            node.visited = false;
            return true;
        }

        if (sourceCount > 1) {
            emitOrSources(sourceIndices, sourceCount, depth, stopAtEmittedOutputs);
        } else if (sourceCount == 1) {
            emitConditionForNode(sourceIndices[0], depth + 1, isOrBranch, stopAtEmittedOutputs);
        }

        node.visited = false;
        return sourceCount > 0;
    }

    // Emit OR sources
    void emitOrSources(int* sourceIndices, int sourceCount, int depth, bool stopAtEmittedOutputs) {
        if (sourceCount == 0) return;

        sortSourcesByPosition(sourceIndices, sourceCount);

        if (allSourcesAreSimpleContacts(sourceIndices, sourceCount)) {
            emitContact(nodes[sourceIndices[0]], true, false);
            for (int s = 1; s < sourceCount; s++) {
                emitContact(nodes[sourceIndices[s]], false, true);
            }
            return;
        }

        int commonAncestorIdx = findCommonAncestor(sourceIndices, sourceCount);

        if (commonAncestorIdx >= 0) {
            for (int i = 0; i < node_count; i++) nodes[i].visited = false;
            emitConditionForNode(commonAncestorIdx, depth + 1, false, stopAtEmittedOutputs);

            int emittedChain[LADDER_MAX_NODES];
            int emittedCount = 0;
            for (int i = 0; i < node_count; i++) nodes[i].visited = false;
            collectChainNodes(commonAncestorIdx, emittedChain, emittedCount, LADDER_MAX_NODES);
            for (int i = 0; i < node_count; i++) nodes[i].visited = false;

            for (int i = 0; i < emittedCount; i++) {
                nodes[emittedChain[i]].emitted = true;
            }

            bool allSimpleContacts = true;
            for (int s = 0; s < sourceCount && allSimpleContacts; s++) {
                if (!isContact(nodes[sourceIndices[s]].type)) {
                    allSimpleContacts = false;
                    break;
                }
                int srcIndices[LADDER_MAX_NODES];
                int srcCount;
                getSourceNodes(nodes[sourceIndices[s]].id, srcIndices, srcCount, LADDER_MAX_NODES);
                for (int i = 0; i < srcCount && allSimpleContacts; i++) {
                    bool found = false;
                    for (int j = 0; j < emittedCount; j++) {
                        if (srcIndices[i] == emittedChain[j]) { found = true; break; }
                    }
                    if (!found) allSimpleContacts = false;
                }
            }

            if (allSimpleContacts) {
                emitLine("A(");
                indent_level++;
                for (int s = 0; s < sourceCount; s++) {
                    emitContact(nodes[sourceIndices[s]], s == 0, s > 0);
                }
                indent_level--;
                emitLine(")");

                for (int i = 0; i < emittedCount; i++) {
                    nodes[emittedChain[i]].emitted = false;
                }
                return;
            }

            emitLine("SAVE");

            for (int s = 0; s < sourceCount; s++) {
                for (int i = 0; i < node_count; i++) nodes[i].visited = false;

                bool isSimpleEmittedContact = false;
                if (isContact(nodes[sourceIndices[s]].type)) {
                    int srcIndices[LADDER_MAX_NODES];
                    int srcCount;
                    getSourceNodes(nodes[sourceIndices[s]].id, srcIndices, srcCount, LADDER_MAX_NODES);

                    isSimpleEmittedContact = true;
                    for (int i = 0; i < srcCount && isSimpleEmittedContact; i++) {
                        bool found = false;
                        for (int j = 0; j < emittedCount; j++) {
                            if (srcIndices[i] == emittedChain[j]) { found = true; break; }
                        }
                        if (!found) isSimpleEmittedContact = false;
                    }
                }

                if (s == 0) {
                    emitLine("L BR");
                    if (isSimpleEmittedContact) {
                        emitContact(nodes[sourceIndices[s]], true, false);
                    } else {
                        emitConditionForNode(sourceIndices[s], depth + 1, false, stopAtEmittedOutputs);
                    }
                } else {
                    emitLine("O(");
                    indent_level++;
                    emitLine("L BR");
                    if (isSimpleEmittedContact) {
                        emitContact(nodes[sourceIndices[s]], true, false);
                    } else {
                        emitConditionForNode(sourceIndices[s], depth + 2, false, stopAtEmittedOutputs);
                    }
                    indent_level--;
                    emitLine(")");
                }
            }

            emitLine("DROP BR");

            for (int i = 0; i < emittedCount; i++) {
                nodes[emittedChain[i]].emitted = false;
            }
        } else {
            emitConditionForNode(sourceIndices[0], depth + 1, false, stopAtEmittedOutputs);

            int emittedChain[LADDER_MAX_NODES];
            int emittedCount = 0;
            for (int i = 0; i < node_count; i++) nodes[i].visited = false;
            collectChainNodes(sourceIndices[0], emittedChain, emittedCount, LADDER_MAX_NODES);
            for (int i = 0; i < node_count; i++) nodes[i].visited = false;

            for (int s = 1; s < sourceCount; s++) {
                int srcIdx = sourceIndices[s];

                if (isSimpleOrSource(srcIdx, emittedChain, emittedCount)) {
                    emitContact(nodes[srcIdx], false, true);
                } else {
                    emitLine("O(");
                    indent_level++;
                    emitConditionForNode(srcIdx, depth + 1, false, stopAtEmittedOutputs);
                    indent_level--;
                    emitLine(")");
                }
            }
        }
    }

    // Check if terminal output
    bool isTerminalOutput(int nodeIdx) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return false;
        LadderGraphNode& node = nodes[nodeIdx];

        if (!isTerminationNode(node.type)) return false;

        int destIndices[LADDER_MAX_NODES];
        int destCount;
        getDestNodes(node.id, destIndices, destCount, LADDER_MAX_NODES);

        if (destCount == 0) return true;

        for (int d = 0; d < destCount; d++) {
            if (isInputNode(nodes[destIndices[d]].type)) return false;
            if (isTerminationNode(nodes[destIndices[d]].type)) return false;
        }

        return true;
    }

    // Find terminal output nodes
    void findTerminalOutputNodes(int* outputNodes, int& count, int maxCount) {
        count = 0;
        for (int i = 0; i < node_count && count < maxCount; i++) {
            if (isTerminalOutput(i)) {
                outputNodes[count++] = i;
            }
        }
    }

    // ============ Main compilation function ============

    bool compile() {
        sortNodesByPosition();

        int outputNodes[LADDER_MAX_NODES];
        int outputCount;
        findOutputNodes(outputNodes, outputCount, LADDER_MAX_NODES);

        if (outputCount == 0) {
            return true;
        }

        bool outputProcessed[LADDER_MAX_NODES] = { false };

        // First pass: handle parallel branches
        for (int n = 0; n < node_count; n++) {
            LadderGraphNode& srcNode = nodes[n];
            if (!isInputNode(srcNode.type) && !isTerminationNode(srcNode.type)) continue;

            int allDestIndices[LADDER_MAX_NODES];
            int allDestCount;
            getDestNodes(srcNode.id, allDestIndices, allDestCount, LADDER_MAX_NODES);

            int termDests[LADDER_MAX_NODES];
            int termDestCount = 0;
            for (int d = 0; d < allDestCount; d++) {
                int destIdx = allDestIndices[d];
                LadderGraphNode& dest = nodes[destIdx];
                if ((isTerminationNode(dest.type) || isComparator(dest.type)) && !outputProcessed[destIdx] && !dest.emitted) {
                    termDests[termDestCount++] = destIdx;
                }
            }

            if (termDestCount < 2) continue;

            srcNode.visited = true;

            for (int i = 0; i < node_count; i++) nodes[i].visited = false;
            emitConditionForNode(n, 0, false);

            for (int i = 0; i < termDestCount - 1; i++) {
                for (int j = i + 1; j < termDestCount; j++) {
                    if (nodes[termDests[j]].y < nodes[termDests[i]].y) {
                        int tmp = termDests[i];
                        termDests[i] = termDests[j];
                        termDests[j] = tmp;
                    }
                }
            }

            emitLine("SAVE");
            indent_level++;
            for (int d = 0; d < termDestCount; d++) {
                int destIdx = termDests[d];
                emitLine("L BR");

                LadderGraphNode& destNode = nodes[destIdx];
                bool hasContinuation = hasDownstreamContact(destIdx);
                bool hasMoreOutputs = hasDownstreamOutput(destIdx);

                if (isComparator(destNode.type)) {
                    emitLine("A(");
                    indent_level++;
                    emitComparator(destNode, true);
                    indent_level--;
                    emitLine(")");
                    int compDestIndices[LADDER_MAX_NODES];
                    int compDestCount;
                    getDestNodes(destNode.id, compDestIndices, compDestCount, LADDER_MAX_NODES);
                    for (int cd = 0; cd < compDestCount; cd++) {
                        int compDestIdx = compDestIndices[cd];
                        LadderGraphNode& compDest = nodes[compDestIdx];
                        if (isOperationBlock(compDest.type) && !compDest.emitted) {
                            emitOperationBlock(compDest, hasDownstreamOutput(compDestIdx), hasDownstreamContact(compDestIdx));
                            outputProcessed[compDestIdx] = true;
                            compDest.emitted = true;
                            if (hasDownstreamChain(compDestIdx)) {
                                emitDownstreamChain(compDestIdx, outputProcessed);
                            }
                        } else if (isCoil(compDest.type) && !compDest.emitted) {
                            emitCoil(compDest, hasDownstreamContact(compDestIdx));
                            outputProcessed[compDestIdx] = true;
                            compDest.emitted = true;
                            if (hasDownstreamChain(compDestIdx)) {
                                emitDownstreamChain(compDestIdx, outputProcessed);
                            }
                        }
                    }
                    destNode.emitted = true;
                } else if (isCoil(destNode.type)) {
                    emitCoil(destNode, hasContinuation || hasMoreOutputs);
                    outputProcessed[destIdx] = true;
                    destNode.emitted = true;
                    if (hasDownstreamChain(destIdx)) {
                        emitDownstreamChain(destIdx, outputProcessed);
                    }
                } else if (isTimer(destNode.type)) {
                    emitTimer(destNode, hasContinuation);
                    outputProcessed[destIdx] = true;
                    destNode.emitted = true;
                    if (hasDownstreamChain(destIdx)) {
                        emitDownstreamChain(destIdx, outputProcessed);
                    }
                } else if (isCounter(destNode.type)) {
                    emitCounter(destNode, hasContinuation);
                    outputProcessed[destIdx] = true;
                    destNode.emitted = true;
                    if (hasDownstreamChain(destIdx)) {
                        emitDownstreamChain(destIdx, outputProcessed);
                    }
                } else if (isOperationBlock(destNode.type)) {
                    emitOperationBlock(destNode, hasMoreOutputs, hasContinuation);
                    outputProcessed[destIdx] = true;
                    destNode.emitted = true;
                    if (hasDownstreamChain(destIdx)) {
                        emitDownstreamChain(destIdx, outputProcessed);
                    }
                }
            }
            indent_level--;
            emitLine("DROP BR");
            emitChar('\n');
        }

        // Second pass: process remaining outputs
        for (int o = 0; o < outputCount; o++) {
            int destIdx = outputNodes[o];
            if (outputProcessed[destIdx]) continue;

            LadderGraphNode& destNode = nodes[destIdx];

            if (destNode.emitted) continue;

            int sourceIndices[LADDER_MAX_NODES];
            int sourceCount;
            getSourceNodes(destNode.id, sourceIndices, sourceCount, LADDER_MAX_NODES);

            bool sourceIsEmittedCoil = false;
            if (sourceCount == 1) {
                LadderGraphNode& srcNode = nodes[sourceIndices[0]];
                if (isCoil(srcNode.type) && srcNode.emitted) {
                    sourceIsEmittedCoil = true;
                    emitContact(srcNode, true);
                }
            }

            if (!sourceIsEmittedCoil) {
                if (sourceCount > 1) {
                    emitOrSources(sourceIndices, sourceCount, 0, true);
                } else if (sourceCount == 1) {
                    for (int i = 0; i < node_count; i++) nodes[i].visited = false;
                    emitConditionForNode(sourceIndices[0], 0, false);
                }
            }

            bool needTapAfter = hasDownstreamContact(destIdx);
            bool hasMoreOutputs = hasDownstreamOutput(destIdx);

            if (isCoil(destNode.type)) {
                emitCoil(destNode, needTapAfter);
            } else if (isTimer(destNode.type)) {
                emitTimer(destNode, needTapAfter);
            } else if (isCounter(destNode.type)) {
                emitCounter(destNode, needTapAfter);
            } else if (isOperationBlock(destNode.type)) {
                emitOperationBlock(destNode, hasMoreOutputs, needTapAfter);
            }
            outputProcessed[destIdx] = true;
            destNode.emitted = true;

            if (hasDownstreamChain(destIdx)) {
                emitDownstreamChain(destIdx, outputProcessed);
            }

            emitChar('\n');
        }

        return !has_error;
    }

    // ============ Graph Validation ============

    virtual bool validateNode(int nodeIdx) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return true;
        LadderGraphNode& node = nodes[nodeIdx];

        if (!isKnownNodeType(node.type)) {
            char msg[64];
            int mi = 0;
            const char* prefix = "Unknown node type '";
            while (*prefix && mi < 50) msg[mi++] = *prefix++;
            int ti = 0;
            while (node.type[ti] && mi < 60) msg[mi++] = node.type[ti++];
            msg[mi++] = '\'';
            msg[mi] = '\0';
            reportNodeError(msg, nodeIdx);
            return false;
        }

        if (!isTapNode(node.type) && !isComparator(node.type) && !isOperationBlock(node.type)) {
            if (node.address[0] == '\0') {
                reportNodeError("Missing address/symbol", nodeIdx);
                return false;
            }
            if (!validateSymbolExists(node.address, nodeIdx)) {
                return false;
            }
        }

        if (isTimer(node.type) || isCounter(node.type)) {
            if (node.preset == 0 && node.preset_str[0] == '\0') {
                reportNodeWarning("Timer/counter has no preset value", nodeIdx);
            }
        }

        if (isOperationBlock(node.type)) {
            bool isIncDec = strEqI(node.type, "fb_inc") || strEqI(node.type, "fb_dec");
            
            if (isIncDec) {
                bool hasAddress = node.address[0] != '\0';
                bool hasIn1 = node.in1[0] != '\0';
                bool hasOut = node.out[0] != '\0';
                
                if (hasAddress && hasIn1 && !strEqI(node.address, node.in1)) {
                    reportNodeError("INC/DEC: address and in1 fields don't match", nodeIdx);
                    return false;
                }
                if (hasAddress && hasOut && !strEqI(node.address, node.out)) {
                    reportNodeError("INC/DEC: address and out fields don't match", nodeIdx);
                    return false;
                }
                if (hasIn1 && hasOut && !strEqI(node.in1, node.out)) {
                    reportNodeError("INC/DEC: in1 and out fields don't match", nodeIdx);
                    return false;
                }
                
                if (!hasAddress && !hasIn1 && !hasOut) {
                    reportNodeError("INC/DEC operation block missing address (use 'address', 'in1', or 'out')", nodeIdx);
                    return false;
                }
                
                const char* addrToValidate = hasAddress ? node.address : (hasIn1 ? node.in1 : node.out);
                if (!validateSymbolExists(addrToValidate, nodeIdx)) {
                    return false;
                }
            } else {
                bool needsIn1 = true;
                bool needsIn2 = !strEqI(node.type, "fb_not") && !strEqI(node.type, "fb_move");
                bool needsOut = true;

                if (needsIn1 && node.in1[0] == '\0') {
                    reportNodeError("Operation block missing input 1 (in1)", nodeIdx);
                    return false;
                }
                if (needsIn1 && node.in1[0] != '\0' && !validateSymbolExists(node.in1, nodeIdx)) {
                    return false;
                }
                if (needsIn2 && node.in2[0] == '\0') {
                    reportNodeError("Operation block missing input 2 (in2)", nodeIdx);
                    return false;
                }
                if (needsIn2 && node.in2[0] != '\0' && !validateSymbolExists(node.in2, nodeIdx)) {
                    return false;
                }
                if (needsOut && node.out[0] == '\0') {
                    reportNodeError("Operation block missing output (out)", nodeIdx);
                    return false;
                }
                if (needsOut && node.out[0] != '\0' && !validateSymbolExists(node.out, nodeIdx)) {
                    return false;
                }
            }
        }

        if (isComparator(node.type)) {
            if (node.in1[0] == '\0') {
                reportNodeError("Comparator missing input 1 (in1)", nodeIdx);
                return false;
            }
            if (!validateSymbolExists(node.in1, nodeIdx)) {
                return false;
            }
            if (node.in2[0] == '\0') {
                reportNodeError("Comparator missing input 2 (in2)", nodeIdx);
                return false;
            }
            if (!validateSymbolExists(node.in2, nodeIdx)) {
                return false;
            }
        }

        return true;
    }

    virtual bool validateConnection(int connIdx) {
        if (connIdx < 0 || connIdx >= connection_count) return true;
        LadderConnection& conn = connections[connIdx];

        for (int s = 0; s < conn.source_count; s++) {
            int srcIdx = findNodeById(conn.sources[s]);
            if (srcIdx < 0) {
                char msg[64];
                int mi = 0;
                const char* prefix = "Connection source '";
                while (*prefix && mi < 40) msg[mi++] = *prefix++;
                int si = 0;
                while (conn.sources[s][si] && mi < 55) msg[mi++] = conn.sources[s][si++];
                const char* suffix = "' not found";
                while (*suffix && mi < 63) msg[mi++] = *suffix++;
                msg[mi] = '\0';
                int errorNode = -1;
                for (int d = 0; d < conn.dest_count && errorNode < 0; d++) {
                    errorNode = findNodeById(conn.destinations[d]);
                }
                if (errorNode >= 0) {
                    reportNodeError(msg, errorNode);
                } else {
                    setError(msg);
                }
                return false;
            }

            LadderGraphNode& srcNode = nodes[srcIdx];
            for (int d = 0; d < conn.dest_count; d++) {
                int destIdx = findNodeById(conn.destinations[d]);
                if (destIdx >= 0) {
                    LadderGraphNode& destNode = nodes[destIdx];
                    if (destNode.x <= srcNode.x) {
                        reportConnectionError("Connection flows backward (destination must be right of source)", connIdx, destIdx);
                        return false;
                    }
                }
            }
        }

        for (int d = 0; d < conn.dest_count; d++) {
            int destIdx = findNodeById(conn.destinations[d]);
            if (destIdx < 0) {
                char msg[64];
                int mi = 0;
                const char* prefix = "Connection destination '";
                while (*prefix && mi < 40) msg[mi++] = *prefix++;
                int di = 0;
                while (conn.destinations[d][di] && mi < 55) msg[mi++] = conn.destinations[d][di++];
                const char* suffix = "' not found";
                while (*suffix && mi < 63) msg[mi++] = *suffix++;
                msg[mi] = '\0';
                int errorNode = -1;
                for (int si = 0; si < conn.source_count && errorNode < 0; si++) {
                    errorNode = findNodeById(conn.sources[si]);
                }
                if (errorNode >= 0) {
                    reportNodeError(msg, errorNode);
                } else {
                    setError(msg);
                }
                return false;
            }
        }

        return true;
    }

    virtual bool validateGraph() {
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
                    reportNodeError(msg, i);
                    if (has_error) return false;
                }
            }
        }

        for (int i = 0; i < node_count; i++) {
            if (!validateNode(i)) {
                if (has_error) return false;
            }
        }

        for (int i = 0; i < connection_count; i++) {
            if (!validateConnection(i)) {
                if (has_error) return false;
            }
        }

        for (int i = 0; i < node_count; i++) {
            if (isTapNode(nodes[i].type)) continue;

            bool hasInputs = nodeHasInputs(i);
            bool hasOutputs = nodeHasOutputs(i);

            if (isInputNode(nodes[i].type) && !hasOutputs) {
                reportNodeWarning("Contact not connected to any output", i);
            }

            if (isTerminationNode(nodes[i].type) && !hasInputs) {
                reportNodeWarning("Output has no inputs", i);
            }

            if (!hasInputs && !hasOutputs) {
                reportNodeError("Node is completely disconnected", i);
                if (has_error) return false;
            }

            if (!hasInputs && nodes[i].x != 0) {
                reportNodeWarning("Starting node should be at x=0", i);
            }
        }

        return !has_error;
    }

    // ============ Main Entry Point ============

    bool parse(const char* src, int len) {
        source = src;
        length = len;

        if (!parseGraph()) {
            return false;
        }

        normalizeConnections();

        if (!validateGraph()) {
            return false;
        }

        if (!compile()) {
            return false;
        }

        if (output_len == 0) {
            setError("Compilation produced no output");
            return false;
        }

        return !has_error;
    }
};

// ============================================================================
// Global Instances and WASM Exports
// ============================================================================

LadderGraphCompiler ladderGraphCompiler = LadderGraphCompiler();
char ladder_input_buffer[32768];
int ladder_input_length = 0;

char ladder_output_buffer[65536];
int ladder_output_length = 0;

extern "C" {

    WASM_EXPORT void ladder_standalone_clear() {
        ladderGraphCompiler.clearState();
        ladder_input_length = 0;
        ladder_output_length = 0;
    }

    WASM_EXPORT void ladder_standalone_load_stream() {
        streamRead(ladder_input_buffer, ladder_input_length, sizeof(ladder_input_buffer));
    }

    WASM_EXPORT int ladder_standalone_compile() {
        bool result = ladderGraphCompiler.parse(ladder_input_buffer, ladder_input_length);
        if (result) {
            ladder_output_length = ladderGraphCompiler.output_len;
            for (int i = 0; i < ladder_output_length && i < 65535; i++) {
                ladder_output_buffer[i] = ladderGraphCompiler.output[i];
            }
            ladder_output_buffer[ladder_output_length] = '\0';
            return ladder_output_length;
        }
        return -1;
    }

    WASM_EXPORT const char* ladder_standalone_get_output() {
        return ladder_output_buffer;
    }

    WASM_EXPORT int ladder_standalone_get_output_len() {
        return ladder_output_length;
    }

    WASM_EXPORT void ladder_standalone_output_to_stream() {
        for (int i = 0; i < ladder_output_length; i++) {
            streamOut(ladder_output_buffer[i]);
        }
    }

    WASM_EXPORT bool ladder_standalone_has_error() {
        return ladderGraphCompiler.hasError();
    }

    WASM_EXPORT const char* ladder_standalone_get_error() {
        return ladderGraphCompiler.getErrorMsg();
    }

    WASM_EXPORT void ladder_standalone_error_to_stream() {
        const char* err = ladderGraphCompiler.getErrorMsg();
        for (int i = 0; err[i] != '\0'; i++) {
            streamOut(err[i]);
        }
    }

    WASM_EXPORT bool ladder_standalone_compile_full() {
        if (!ladderGraphCompiler.parse(ladder_input_buffer, ladder_input_length)) {
            return false;
        }

        stlCompiler.setSource(ladderGraphCompiler.output, ladderGraphCompiler.output_len);
        if (!stlCompiler.compile()) {
            return false;
        }

        defaultCompiler.set_assembly_string((char*) stlCompiler.getOutput());
        return !defaultCompiler.compileAssembly(false, false);
    }

} // extern "C"

#endif // __WASM__