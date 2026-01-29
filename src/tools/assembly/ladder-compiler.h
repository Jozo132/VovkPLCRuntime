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

class LadderGraphCompiler {
public:
    const char* source;
    int length;
    int pos;

    char error_msg[256];
    bool has_error;

    // Output STL buffer
    static const int MAX_OUTPUT = 32768;
    char output[MAX_OUTPUT];
    volatile int output_len;

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

    // Virtual so LadderLinter can override to capture multiple errors
    virtual void setError(const char* msg) {
        if (has_error) return;
        has_error = true;
        int i = 0;
        while (msg[i] && i < 255) {
            error_msg[i] = msg[i];
            i++;
        }
        error_msg[i] = '\0';
    }

    // Get/Set edge memory counter for cross-block continuity
    int getEdgeMemCounter() const { return edge_mem_counter; }
    void setEdgeMemCounter(int value) { edge_mem_counter = value; }

    // ============ Output Helpers ============

    void emitIndent() {
        for (int i = 0; i < indent_level; i++) {
            emit("    ");  // 4 spaces per indent level
        }
    }

    void emit(const char* s) {
        while (*s && output_len < MAX_OUTPUT - 1) {
            output[output_len++] = *s++;
        }
        output[output_len] = '\0';
    }

    void emitChar(char c) {
        if (output_len < MAX_OUTPUT - 1) {
            output[output_len++] = c;
            output[output_len] = '\0';
        }
    }

    void emitLine(const char* s) {
        emitIndent();
        emit(s);
        emitChar('\n');
    }

    void emitInt(int val) {
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
        while (i > 0) {
            emitChar(buf[--i]);
        }
    }

    // ============ JSON Lexer Helpers ============

    char peek(int offset = 0) {
        int p = pos + offset;
        return (p < length) ? source[p] : '\0';
    }

    char advance() {
        if (pos >= length) return '\0';
        return source[pos++];
    }

    void skipWhitespace() {
        while (pos < length) {
            char c = peek();
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') advance();
            else break;
        }
    }

    bool expect(char c) {
        skipWhitespace();
        if (peek() == c) {
            advance();
            return true;
        }
        return false;
    }

    bool strMatch(const char* s) {
        skipWhitespace();
        int i = 0;
        while (s[i]) {
            if (peek(i) != s[i]) return false;
            i++;
        }
        for (int j = 0; j < i; j++) advance();
        return true;
    }

    bool strEqI(const char* a, const char* b) {
        while (*a && *b) {
            char ca = (*a >= 'A' && *a <= 'Z') ? *a + 32 : *a;
            char cb = (*b >= 'A' && *b <= 'Z') ? *b + 32 : *b;
            if (ca != cb) return false;
            a++; b++;
        }
        return *a == *b;
    }

    bool readString(char* buf, int maxLen) {
        skipWhitespace();
        if (peek() != '"') {
            setError("Expected string");
            return false;
        }
        advance();

        int i = 0;
        while (i < maxLen - 1) {
            char c = peek();
            if (c == '\0') {
                setError("Unterminated string");
                return false;
            }
            if (c == '"') {
                advance();
                break;
            }
            if (c == '\\') {
                advance();
                c = peek();
                if (c == 'n') c = '\n';
                else if (c == 't') c = '\t';
                else if (c == 'r') c = '\r';
            }
            buf[i++] = advance();
        }
        buf[i] = '\0';
        return true;
    }

    bool readBool(bool& value) {
        skipWhitespace();
        if (strMatch("true")) { value = true; return true; }
        if (strMatch("false")) { value = false; return true; }
        setError("Expected boolean");
        return false;
    }

    bool readU32(uint32_t& value) {
        skipWhitespace();
        if (peek() < '0' || peek() > '9') {
            setError("Expected number");
            return false;
        }
        value = 0;
        while (peek() >= '0' && peek() <= '9') {
            value = value * 10 + (advance() - '0');
        }
        return true;
    }

    bool readInt(int& value) {
        skipWhitespace();
        bool neg = false;
        if (peek() == '-') {
            neg = true;
            advance();
        }
        if (peek() < '0' || peek() > '9') {
            setError("Expected number");
            return false;
        }
        value = 0;
        while (peek() >= '0' && peek() <= '9') {
            value = value * 10 + (advance() - '0');
        }
        if (neg) value = -value;
        return true;
    }

    void skipValue() {
        skipWhitespace();
        char c = peek();

        if (c == '"') {
            advance();
            while (peek() && peek() != '"') {
                if (peek() == '\\') advance();
                advance();
            }
            if (peek() == '"') advance();
        } else if (c == '{') {
            advance();
            int depth = 1;
            while (peek() && depth > 0) {
                if (peek() == '{') depth++;
                else if (peek() == '}') depth--;
                else if (peek() == '"') {
                    advance();
                    while (peek() && peek() != '"') {
                        if (peek() == '\\') advance();
                        advance();
                    }
                }
                advance();
            }
        } else if (c == '[') {
            advance();
            int depth = 1;
            while (peek() && depth > 0) {
                if (peek() == '[') depth++;
                else if (peek() == ']') depth--;
                else if (peek() == '"') {
                    advance();
                    while (peek() && peek() != '"') {
                        if (peek() == '\\') advance();
                        advance();
                    }
                }
                advance();
            }
        } else if ((c >= '0' && c <= '9') || c == '-') {
            while (peek() && ((peek() >= '0' && peek() <= '9') || peek() == '.' || peek() == '-' || peek() == 'e' || peek() == 'E')) {
                advance();
            }
        } else if (c == 't' || c == 'f' || c == 'n') {
            while (peek() && peek() >= 'a' && peek() <= 'z') advance();
        }
    }

    // ============ Graph Data Structures ============

    static const int MAX_NODES = 256;
    static const int MAX_CONNECTIONS = 256;
    static const int MAX_CONN_ENDPOINTS = 16;  // Max sources/destinations per connection

    struct GraphNode {
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

    struct Connection {
        char sources[MAX_CONN_ENDPOINTS][32];
        int source_count;
        char destinations[MAX_CONN_ENDPOINTS][32];
        int dest_count;

        void clear() {
            source_count = 0;
            dest_count = 0;
        }
    };

    GraphNode nodes[MAX_NODES];
    int node_count;

    Connection connections[MAX_CONNECTIONS];
    int connection_count;

    // ============ Validation Helpers ============

    // Find a node by its ID, returns index or -1 if not found
    int findNodeById(const char* nodeId) {
        for (int i = 0; i < node_count; i++) {
            if (strEqI(nodes[i].id, nodeId)) return i;
        }
        return -1;
    }

    // Check if a node type is valid (known)
    bool isKnownNodeType(const char* type) {
        return isContact(type) || isCoil(type) || isTimer(type) ||
               isCounter(type) || isOperationBlock(type) || isComparator(type) ||
               strEqI(type, "tap");
    }

    // Check if node type is a tap node
    bool isTapNode(const char* type) {
        return strEqI(type, "tap");
    }

    // Check if address is a raw memory address (starts with M, X, Y, K, S, T, C, or number)
    // Raw addresses like "X0.0", "M10", "T0" don't need symbol lookup
    // Also handles literal values prefixed with # (e.g., #100, #-50, #3.14)
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

    // Validate a symbol name exists in the shared symbol table
    // Returns true if address is raw OR if symbol exists
    bool validateSymbolExists(const char* addr, int nodeIdx) {
        if (!addr || addr[0] == '\0') return true;  // Empty handled elsewhere
        if (isRawAddress(addr)) return true;  // Raw addresses don't need lookup

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
                reportNodeError(msg, nodeIdx);
                return false;
            }
        }
        return true;
    }

    // Check if a node has any incoming connections
    bool nodeHasInputs(int nodeIdx) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return false;
        for (int c = 0; c < connection_count; c++) {
            for (int d = 0; d < connections[c].dest_count; d++) {
                if (strEqI(connections[c].destinations[d], nodes[nodeIdx].id)) {
                    return true;
                }
            }
        }
        return false;
    }

    // Check if a node has any outgoing connections
    bool nodeHasOutputs(int nodeIdx) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return false;
        for (int c = 0; c < connection_count; c++) {
            for (int s = 0; s < connections[c].source_count; s++) {
                if (strEqI(connections[c].sources[s], nodes[nodeIdx].id)) {
                    return true;
                }
            }
        }
        return false;
    }

    // Virtual method to report a validation error with node position
    // Derived class (linter) can override to collect multiple errors
    virtual void reportNodeError(const char* msg, int nodeIdx) {
        if (has_error) return; // Base class stops at first error
        char fullMsg[256];
        int mi = 0;
        // Add node position info
        if (nodeIdx >= 0 && nodeIdx < node_count) {
            GraphNode& node = nodes[nodeIdx];
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
    // connIdx is the index of the connection in the connections array
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

    // ============ Node Type Helpers ============

    bool isContact(const char* type) {
        return strEqI(type, "contact_no") || strEqI(type, "contact_nc") ||
            strEqI(type, "contact_pos") || strEqI(type, "contact_neg") ||
            strEqI(type, "contact");
    }

    bool isCoil(const char* type) {
        return strEqI(type, "coil") || strEqI(type, "coil_set") ||
            strEqI(type, "coil_rset") || strEqI(type, "coil_neg");
    }

    bool isTimer(const char* type) {
        return strEqI(type, "timer_on") || strEqI(type, "timer_off") ||
            strEqI(type, "timer_pulse") || strEqI(type, "timer_ton") ||
            strEqI(type, "timer_tof") || strEqI(type, "timer_tp");
    }

    bool isCounter(const char* type) {
        return strEqI(type, "counter_up") || strEqI(type, "counter_down") ||
            strEqI(type, "counter_u") || strEqI(type, "counter_d");
    }

    // Operation blocks: math and memory operations that execute conditionally
    bool isOperationBlock(const char* type) {
        // Math operations
        if (strEqI(type, "fb_add") || strEqI(type, "fb_sub") ||
            strEqI(type, "fb_mul") || strEqI(type, "fb_div") ||
            strEqI(type, "fb_mod")) return true;
        // Increment/decrement
        if (strEqI(type, "fb_inc") || strEqI(type, "fb_dec")) return true;
        // Bitwise operations
        if (strEqI(type, "fb_and") || strEqI(type, "fb_or") ||
            strEqI(type, "fb_xor") || strEqI(type, "fb_not") ||
            strEqI(type, "fb_shl") || strEqI(type, "fb_shr")) return true;
        // Move/copy operations
        if (strEqI(type, "fb_move") || strEqI(type, "fb_copy")) return true;
        return false;
    }

    // Comparator blocks: compare values and produce boolean result (like contacts)
    bool isComparator(const char* type) {
        return strEqI(type, "cmp_eq") || strEqI(type, "cmp_ne") ||
            strEqI(type, "cmp_gt") || strEqI(type, "cmp_lt") ||
            strEqI(type, "cmp_ge") || strEqI(type, "cmp_le") ||
            strEqI(type, "cmp_gte") || strEqI(type, "cmp_lte") ||
            strEqI(type, "fb_cmp_eq") || strEqI(type, "fb_cmp_ne") ||
            strEqI(type, "fb_cmp_gt") || strEqI(type, "fb_cmp_lt") ||
            strEqI(type, "fb_cmp_ge") || strEqI(type, "fb_cmp_le") ||
            strEqI(type, "fb_cmp_gte") || strEqI(type, "fb_cmp_lte");
    }

    // isTerminationNode: nodes that write output and can optionally pass through RLO
    bool isTerminationNode(const char* type) {
        return isCoil(type) || isTimer(type) || isCounter(type) || isOperationBlock(type);
    }

    // isOutputNode: nodes that are actual outputs (coils, operation blocks)
    // Operation blocks are like coils - they execute conditionally and can pass through RLO
    bool isOutputNode(const char* type) {
        return isCoil(type) || isOperationBlock(type);
    }

    bool isInputNode(const char* type) {
        return isContact(type) || isComparator(type);
    }

    bool isFunctionBlock(const char* type) {
        return isTimer(type) || isCounter(type);
    }

    // Check if a termination node should emit TAP (has outgoing connections to continuation nodes)
    // Only COILS need TAP because they consume the RLO
    // Timers and counters push their Q output as the new RLO, so they don't need TAP
    // TAP is needed when the coil output feeds into ANY downstream node (contact OR coil)
    // because the coil consumes the RLO value from the stack
    bool nodeNeedsTap(int nodeIdx) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return false;
        GraphNode& node = nodes[nodeIdx];

        // Only COILS need TAP - timers/counters push their Q output to the stack
        if (!isCoil(node.type)) return false;

        // Check if node has outgoing connections to any downstream nodes
        // TAP is needed if there's any downstream node that needs the RLO
        int destIndices[MAX_NODES];
        int destCount;
        getDestNodes(node.id, destIndices, destCount, MAX_NODES);

        for (int d = 0; d < destCount; d++) {
            GraphNode& dest = nodes[destIndices[d]];
            // If destination is a contact (input), we need TAP
            if (isInputNode(dest.type)) return true;
            // If destination is another coil/output, we also need TAP
            // because the current coil consumes the RLO
            if (isOutputNode(dest.type)) return true;
        }

        return false;
    }

    // Emit any coils that are connected as passthrough from the given node
    void emitPassthroughChain(int nodeIdx, bool* outputProcessed) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return;

        // Find destination coils connected via passthrough
        int destIndices[MAX_NODES];
        int destCount;
        getDestNodes(nodes[nodeIdx].id, destIndices, destCount, MAX_NODES);

        for (int d = 0; d < destCount; d++) {
            int destIdx = destIndices[d];
            GraphNode& destNode = nodes[destIdx];

            // Only process coils (not contacts used as passthrough sensing)
            if (!isCoil(destNode.type)) continue;

            // Skip if already processed
            if (outputProcessed[destIdx]) continue;

            // Emit this coil
            bool needTap = nodeNeedsTap(destIdx);
            emitCoil(destNode, needTap);
            outputProcessed[destIdx] = true;
            if (needTap) destNode.emitted = true;

            // Recursively emit any further passthrough chain
            if (needTap) {
                emitPassthroughChain(destIdx, outputProcessed);
            }
        }
    }

    // Check if a node has a downstream chain (contact → output sequences after it)
    bool hasDownstreamChain(int nodeIdx) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return false;

        // Get destinations from this node
        int destIndices[MAX_NODES];
        int destCount;
        getDestNodes(nodes[nodeIdx].id, destIndices, destCount, MAX_NODES);

        for (int d = 0; d < destCount; d++) {
            GraphNode& dest = nodes[destIndices[d]];
            // If any destination is a contact/comparator (input node), there's a downstream chain
            if (isInputNode(dest.type)) return true;
            // If destination is an output (coil/operation block), it's a direct passthrough
            if (isOutputNode(dest.type)) return true;
            // Timers and counters pass through RLO like contacts, so they're part of the chain
            if (isTimer(dest.type) || isCounter(dest.type)) return true;
        }
        return false;
    }

    // Check if a node has a downstream contact (input node) - used to determine if TAP is needed
    // TAP is only needed when the RLO will be read by a contact, not for output-to-output passthrough
    bool hasDownstreamContact(int nodeIdx) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return false;

        // Get destinations from this node
        int destIndices[MAX_NODES];
        int destCount;
        getDestNodes(nodes[nodeIdx].id, destIndices, destCount, MAX_NODES);

        for (int d = 0; d < destCount; d++) {
            GraphNode& dest = nodes[destIndices[d]];
            // Only return true if any destination is a contact/comparator (input node)
            if (isInputNode(dest.type)) return true;
        }
        return false;
    }

    // Check if a node has downstream output nodes (operations, coils, etc.)
    // Used to determine if TAP is needed before an operation's JCN
    bool hasDownstreamOutput(int nodeIdx) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return false;

        int destIndices[MAX_NODES];
        int destCount;
        getDestNodes(nodes[nodeIdx].id, destIndices, destCount, MAX_NODES);

        for (int d = 0; d < destCount; d++) {
            GraphNode& dest = nodes[destIndices[d]];
            if (isOutputNode(dest.type)) return true;
            // Also check if there's a contact that leads to outputs
            if (isInputNode(dest.type)) {
                if (hasDownstreamChain(destIndices[d])) return true;
            }
        }
        return false;
    }

    // Emit downstream chain from a node (handles contact → coil sequences)
    void emitDownstreamChain(int nodeIdx, bool* outputProcessed) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return;

        // Get destinations from this node
        int destIndices[MAX_NODES];
        int destCount;
        getDestNodes(nodes[nodeIdx].id, destIndices, destCount, MAX_NODES);

        for (int d = 0; d < destCount; d++) {
            int destIdx = destIndices[d];
            GraphNode& dest = nodes[destIdx];

            if (isInputNode(dest.type)) {
                // This is a contact/comparator - emit it and follow to its destinations
                emitContact(dest, true);  // useAnd = true (first in sub-chain)

                // Find output nodes that this contact feeds into
                int outputIndices[MAX_NODES];
                int outputCount;
                getDestNodes(dest.id, outputIndices, outputCount, MAX_NODES);

                for (int c = 0; c < outputCount; c++) {
                    int outIdx = outputIndices[c];
                    GraphNode& outNode = nodes[outIdx];

                    if (isOutputNode(outNode.type) && !outNode.emitted) {
                        // Check if this output has further downstream contacts (for TAP after)
                        bool needsTapAfter = hasDownstreamContact(outIdx);
                        // Check if there are more outputs after this one (for TAP before operation's JCN)
                        bool hasMoreOutputs = (c < outputCount - 1) || hasDownstreamOutput(outIdx);

                        if (isCoil(outNode.type)) {
                            // Coils need TAP if they have ANY downstream nodes (contacts OR other coils)
                            emitCoil(outNode, needsTapAfter || hasMoreOutputs);
                        } else if (isOperationBlock(outNode.type)) {
                            emitOperationBlock(outNode, hasMoreOutputs, needsTapAfter);
                        }
                        outputProcessed[outIdx] = true;
                        outNode.emitted = true;

                        // Recurse if there's more downstream (contacts or outputs)
                        if (hasDownstreamChain(outIdx)) {
                            emitDownstreamChain(outIdx, outputProcessed);
                        }
                    }
                }
            } else if (isOutputNode(dest.type) && !dest.emitted) {
                // Direct output-to-output passthrough
                bool needsTapAfter = hasDownstreamContact(destIdx);
                bool hasMoreOutputs = hasDownstreamOutput(destIdx);

                if (isCoil(dest.type)) {
                    // Coils need TAP if they have ANY downstream nodes (contacts OR other coils)
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
                // Timers and counters pass through RLO like contacts
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

    // ============ Connection Helpers ============

    // Find all connections where this node is a destination (inputs to this node)
    void getInputConnections(const char* nodeId, int* connIndices, int& count, int maxCount) {
        count = 0;
        for (int c = 0; c < connection_count && count < maxCount; c++) {
            for (int d = 0; d < connections[c].dest_count; d++) {
                if (strEqI(connections[c].destinations[d], nodeId)) {
                    connIndices[count++] = c;
                    break;
                }
            }
        }
    }

    // Find all connections where this node is a source (outputs from this node)
    void getOutputConnections(const char* nodeId, int* connIndices, int& count, int maxCount) {
        count = 0;
        for (int c = 0; c < connection_count && count < maxCount; c++) {
            for (int s = 0; s < connections[c].source_count; s++) {
                if (strEqI(connections[c].sources[s], nodeId)) {
                    connIndices[count++] = c;
                    break;
                }
            }
        }
    }

    // Get all source node IDs feeding into a given node
    void getSourceNodes(const char* nodeId, int* nodeIndices, int& count, int maxCount) {
        count = 0;
        int connIndices[MAX_CONNECTIONS];
        int connCount;
        getInputConnections(nodeId, connIndices, connCount, MAX_CONNECTIONS);

        for (int c = 0; c < connCount; c++) {
            Connection& conn = connections[connIndices[c]];
            for (int s = 0; s < conn.source_count && count < maxCount; s++) {
                int idx = findNodeById(conn.sources[s]);
                if (idx >= 0) {
                    // Check if already added
                    bool found = false;
                    for (int i = 0; i < count; i++) {
                        if (nodeIndices[i] == idx) { found = true; break; }
                    }
                    if (!found) nodeIndices[count++] = idx;
                }
            }
        }
    }

    // Get all destination node IDs this node feeds into
    void getDestNodes(const char* nodeId, int* nodeIndices, int& count, int maxCount) {
        count = 0;
        int connIndices[MAX_CONNECTIONS];
        int connCount;
        getOutputConnections(nodeId, connIndices, connCount, MAX_CONNECTIONS);

        for (int c = 0; c < connCount; c++) {
            Connection& conn = connections[connIndices[c]];
            for (int d = 0; d < conn.dest_count && count < maxCount; d++) {
                int idx = findNodeById(conn.destinations[d]);
                if (idx >= 0) {
                    bool found = false;
                    for (int i = 0; i < count; i++) {
                        if (nodeIndices[i] == idx) { found = true; break; }
                    }
                    if (!found) nodeIndices[count++] = idx;
                }
            }
        }
    }

    // ============ JSON Parsing ============

    bool parseNode(GraphNode& node) {
        node.clear();

        if (!expect('{')) {
            setError("Expected node object");
            return false;
        }

        while (peek() != '}' && peek() != '\0') {
            char key[32];
            if (!readString(key, sizeof(key))) return false;
            if (!expect(':')) { setError("Expected ':'"); return false; }

            if (strEqI(key, "id")) {
                if (!readString(node.id, sizeof(node.id))) return false;
            } else if (strEqI(key, "type")) {
                if (!readString(node.type, sizeof(node.type))) return false;
            } else if (strEqI(key, "address") || strEqI(key, "symbol")) {
                if (!readString(node.address, sizeof(node.address))) return false;
            } else if (strEqI(key, "x")) {
                if (!readInt(node.x)) return false;
            } else if (strEqI(key, "y")) {
                if (!readInt(node.y)) return false;
            } else if (strEqI(key, "inverted")) {
                if (!readBool(node.inverted)) return false;
            } else if (strEqI(key, "preset")) {
                skipWhitespace();
                if (peek() == '"') {
                    if (!readString(node.preset_str, sizeof(node.preset_str))) return false;
                } else {
                    if (!readU32(node.preset)) return false;
                }
            } else if (strEqI(key, "trigger")) {
                if (!readString(node.trigger, sizeof(node.trigger))) return false;
            } else if (strEqI(key, "dataType") || strEqI(key, "data_type")) {
                if (!readString(node.data_type, sizeof(node.data_type))) return false;
            } else if (strEqI(key, "in1") || strEqI(key, "input1") || strEqI(key, "in")) {
                if (!readString(node.in1, sizeof(node.in1))) return false;
            } else if (strEqI(key, "in2") || strEqI(key, "input2")) {
                if (!readString(node.in2, sizeof(node.in2))) return false;
            } else if (strEqI(key, "out") || strEqI(key, "output")) {
                if (!readString(node.out, sizeof(node.out))) return false;
            } else if (strEqI(key, "passThrough") || strEqI(key, "pass_through")) {
                if (!readBool(node.passThrough)) return false;
            } else {
                skipValue();
            }

            skipWhitespace();
            if (peek() == ',') advance();
        }

        if (!expect('}')) {
            setError("Expected '}' to close node");
            return false;
        }

        return true;
    }

    bool parseConnection(Connection& conn) {
        conn.clear();

        if (!expect('{')) {
            setError("Expected connection object");
            return false;
        }

        bool has_sources = false;
        bool has_destinations = false;

        while (peek() != '}' && peek() != '\0') {
            char key[32];
            if (!readString(key, sizeof(key))) return false;
            if (!expect(':')) { setError("Expected ':'"); return false; }

            if (strEqI(key, "sources")) {
                has_sources = true;
                if (!expect('[')) { setError("Expected sources array"); return false; }
                while (peek() != ']' && peek() != '\0') {
                    if (conn.source_count >= MAX_CONN_ENDPOINTS) {
                        setError("Too many sources");
                        return false;
                    }
                    if (!readString(conn.sources[conn.source_count], 32)) return false;
                    conn.source_count++;
                    skipWhitespace();
                    if (peek() == ',') advance();
                }
                if (!expect(']')) { setError("Expected ']'"); return false; }
            } else if (strEqI(key, "destinations")) {
                has_destinations = true;
                if (!expect('[')) { setError("Expected destinations array"); return false; }
                while (peek() != ']' && peek() != '\0') {
                    if (conn.dest_count >= MAX_CONN_ENDPOINTS) {
                        setError("Too many destinations");
                        return false;
                    }
                    if (!readString(conn.destinations[conn.dest_count], 32)) return false;
                    conn.dest_count++;
                    skipWhitespace();
                    if (peek() == ',') advance();
                }
                if (!expect(']')) { setError("Expected ']'"); return false; }
            } else {
                skipValue();
            }

            skipWhitespace();
            if (peek() == ',') advance();
        }

        if (!expect('}')) {
            setError("Expected '}' to close connection");
            return false;
        }

        // Validate required keys
        if (!has_sources) {
            setError("Connection missing required 'sources' array");
            return false;
        }
        if (!has_destinations) {
            setError("Connection missing required 'destinations' array");
            return false;
        }
        if (conn.source_count == 0) {
            setError("Connection 'sources' array is empty");
            return false;
        }
        if (conn.dest_count == 0) {
            setError("Connection 'destinations' array is empty");
            return false;
        }

        return true;
    }

    bool parseGraph() {
        if (!expect('{')) {
            setError("Expected JSON object");
            return false;
        }

        bool has_nodes = false;
        bool has_connections = false;

        while (peek() != '}' && peek() != '\0') {
            char key[32];
            if (!readString(key, sizeof(key))) return false;
            if (!expect(':')) { setError("Expected ':'"); return false; }

            if (strEqI(key, "nodes")) {
                has_nodes = true;
                if (!expect('[')) { setError("Expected nodes array"); return false; }
                while (peek() != ']' && peek() != '\0') {
                    if (node_count >= MAX_NODES) {
                        setError("Too many nodes");
                        return false;
                    }
                    if (!parseNode(nodes[node_count])) return false;
                    node_count++;
                    skipWhitespace();
                    if (peek() == ',') advance();
                }
                if (!expect(']')) { setError("Expected ']'"); return false; }
            } else if (strEqI(key, "connections")) {
                has_connections = true;
                if (!expect('[')) { setError("Expected connections array"); return false; }
                while (peek() != ']' && peek() != '\0') {
                    if (connection_count >= MAX_CONNECTIONS) {
                        setError("Too many connections");
                        return false;
                    }
                    if (!parseConnection(connections[connection_count])) return false;
                    connection_count++;
                    skipWhitespace();
                    if (peek() == ',') advance();
                }
                if (!expect(']')) { setError("Expected ']'"); return false; }
            } else {
                skipValue();
            }

            skipWhitespace();
            if (peek() == ',') advance();
        }

        if (!expect('}')) {
            setError("Expected '}'");
            return false;
        }

        // Validate required arrays
        if (!has_nodes) {
            setError("Ladder Graph JSON missing required 'nodes' array");
            return false;
        }
        if (!has_connections) {
            setError("Ladder Graph JSON missing required 'connections' array");
            return false;
        }

        return true;
    }

    // ============ Connection Normalization ============

    // Helper: Check if a string is already in an array
    bool stringInArray(const char* str, char arr[][32], int count) {
        for (int i = 0; i < count; i++) {
            if (strEqI(arr[i], str)) return true;
        }
        return false;
    }

    // Helper: Check if two connections share any source or destination
    bool connectionsShareEndpoints(const Connection& a, const Connection& b) {
        // Check if they share any sources
        for (int i = 0; i < a.source_count; i++) {
            for (int j = 0; j < b.source_count; j++) {
                if (strEqI(a.sources[i], b.sources[j])) return true;
            }
        }
        // Check if they share any destinations
        for (int i = 0; i < a.dest_count; i++) {
            for (int j = 0; j < b.dest_count; j++) {
                if (strEqI(a.destinations[i], b.destinations[j])) return true;
            }
        }
        return false;
    }

    // Helper: Merge connection b into connection a (adding unique sources/destinations)
    void mergeConnections(Connection& a, const Connection& b) {
        // Merge sources
        for (int i = 0; i < b.source_count; i++) {
            if (!stringInArray(b.sources[i], a.sources, a.source_count)) {
                if (a.source_count < MAX_CONN_ENDPOINTS) {
                    strcpy(a.sources[a.source_count], b.sources[i]);
                    a.source_count++;
                }
            }
        }
        // Merge destinations
        for (int i = 0; i < b.dest_count; i++) {
            if (!stringInArray(b.destinations[i], a.destinations, a.dest_count)) {
                if (a.dest_count < MAX_CONN_ENDPOINTS) {
                    strcpy(a.destinations[a.dest_count], b.destinations[i]);
                    a.dest_count++;
                }
            }
        }
    }

    // Normalize connections: merge all connections that share sources or destinations
    // This creates many-to-many relationships for parallel power transfer detection
    void normalizeConnections() {
        bool merged;
        do {
            merged = false;
            for (int i = 0; i < connection_count && !merged; i++) {
                for (int j = i + 1; j < connection_count && !merged; j++) {
                    if (connectionsShareEndpoints(connections[i], connections[j])) {
                        // Merge j into i
                        mergeConnections(connections[i], connections[j]);
                        // Remove j by shifting remaining connections down
                        for (int k = j; k < connection_count - 1; k++) {
                            connections[k] = connections[k + 1];
                        }
                        connection_count--;
                        merged = true; // Restart the loop
                    }
                }
            }
        } while (merged);
    }

    // ============ STL Generation ============

    // Helper to generate unique edge memory address
    void generateEdgeAddress(char* edge_mem) {
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

    // Emit STL for a contact node (including comparators)
    // useOr: if true, emit O/ON instead of A/AN
    void emitContact(GraphNode& node, bool isFirst, bool useOr = false) {
        // Handle comparator blocks separately
        if (isComparator(node.type)) {
            emitComparator(node, isFirst);
            return;
        }

        // Determine instruction: A (and), AN (and not), O (or), ON (or not)
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

        // Handle edge detection
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
    // If emitTapAfter is true, emit TAP instruction after the coil (for passthrough)
    void emitCoil(GraphNode& node, bool emitTapAfter = false) {
        const char* instr = "= ";
        bool isInverted = node.inverted || strEqI(node.type, "coil_neg");

        if (strEqI(node.type, "coil_set")) instr = "S ";
        else if (strEqI(node.type, "coil_rset")) instr = "R ";
        else if (isInverted) {
            // Inverted coil: use =N instruction which negates only for the write,
            // preserving the original RLO for continuation
            instr = "=N ";
        }

        emitIndent();
        emit(instr);
        emit(node.address);
        emitChar('\n');

        // For inverted coils, =N preserves the RLO so no TAP needed
        // For non-inverted coils, TAP after to preserve RLO for continuation
        if (emitTapAfter && !isInverted) {
            emitLine("TAP");
        }
    }

    // Emit STL for a timer node
    // If emitTapAfter is true, emit TAP instruction after the timer (for passthrough)
    void emitTimer(GraphNode& node, bool emitTapAfter = false) {
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
    // If emitTapAfter is true, emit TAP instruction after the counter (for passthrough)
    void emitCounter(GraphNode& node, bool emitTapAfter = false) {
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

    // Get Siemens type suffix (I=16-bit int, D=32-bit int, R=32-bit float, B=8-bit)
    char getSiemensTypeSuffix(const char* data_type) {
        if (strEqI(data_type, "i16") || strEqI(data_type, "int16") ||
            strEqI(data_type, "u16") || strEqI(data_type, "uint16")) return 'I';
        if (strEqI(data_type, "i32") || strEqI(data_type, "int32") ||
            strEqI(data_type, "u32") || strEqI(data_type, "uint32")) return 'D';
        if (strEqI(data_type, "f32") || strEqI(data_type, "float")) return 'R';
        if (strEqI(data_type, "i8") || strEqI(data_type, "int8") ||
            strEqI(data_type, "u8") || strEqI(data_type, "uint8")) return 'B';
        return 'I';  // Default to 16-bit integer
    }

    // Get Siemens address size prefix (W=word/16-bit, D=dword/32-bit, B=byte/8-bit)
    // For memory addresses like M14 -> MW14 (word), MD14 (dword), MB14 (byte)
    char getSiemensAddressSizePrefix(char typeSuffix) {
        switch (typeSuffix) {
            case 'I': return 'W';  // Word (16-bit)
            case 'D': return 'D';  // Double word (32-bit)
            case 'R': return 'D';  // Real stored in double word
            case 'B': return 'B';  // Byte (8-bit)
            default: return 'W';
        }
    }

    // Emit address with proper Siemens type prefix
    // Converts M14 -> MW14 (for word), MD14 (for dword), MB14 (for byte)
    // Handles: M (memory), preserves #literals and already-typed addresses
    void emitTypedAddress(const char* addr, char typeSuffix) {
        if (!addr || addr[0] == '\0') return;

        // If it's a literal (starts with #), emit as-is
        if (addr[0] == '#') {
            emit(addr);
            return;
        }

        // If address already has a size prefix (MW, MD, MB), emit as-is
        if (addr[0] == 'M' && (addr[1] == 'W' || addr[1] == 'D' || addr[1] == 'B')) {
            emit(addr);
            return;
        }

        // For M addresses, insert the size prefix after M
        if (addr[0] == 'M' && addr[1] >= '0' && addr[1] <= '9') {
            char sizePrefix = getSiemensAddressSizePrefix(typeSuffix);
            emitChar('M');
            emitChar(sizePrefix);
            emit(addr + 1);  // Rest of address after 'M'
            return;
        }

        // For other addresses (X, Y, etc.), emit as-is
        emit(addr);
    }

    // Emit STL for an operation block (math/memory operations)
    // Uses standard Siemens STL syntax: L, T, +I/-I/*I//I, INCI/DECI, etc.
    // Operations are wrapped in JCN to only execute when RLO is true
    // emitTapBefore: if true, TAP is emitted before JCN to preserve RLO for subsequent operations
    void emitOperationBlock(GraphNode& node, bool emitTapBefore = false, bool emitTapAfter = false) {
        char typeSuffix = getSiemensTypeSuffix(node.data_type);

        // Generate unique label for this operation's skip point
        int skipLabel = label_counter++;

        // TAP to preserve RLO if there are more operations after this one
        if (emitTapBefore) {
            emitIndent();
            emit("TAP\n");
        }

        // Emit JCN to skip operation if RLO is false (consumes RLO)
        emitIndent();
        emit("JCN _op");
        emitInt(skipLabel);
        emitChar('\n');

        // INC/DEC - use INCI/DECI/INCD/DECD/INCB/DECB/INCR/DECR
        if (strEqI(node.type, "fb_inc") || strEqI(node.type, "fb_dec")) {
            bool isInc = strEqI(node.type, "fb_inc");
            emitIndent();
            emit(isInc ? "INC" : "DEC");
            emitChar(typeSuffix);
            emit(" ");
            emitTypedAddress(node.address, typeSuffix);
            emitChar('\n');
        }
        // MOVE - use L and T
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
        // Binary math operations: L in1, op in2, T out
        else if (strEqI(node.type, "fb_add") || strEqI(node.type, "fb_sub") ||
            strEqI(node.type, "fb_mul") || strEqI(node.type, "fb_div")) {
            const char* mathOp = nullptr;
            if (strEqI(node.type, "fb_add")) mathOp = "+";
            else if (strEqI(node.type, "fb_sub")) mathOp = "-";
            else if (strEqI(node.type, "fb_mul")) mathOp = "*";
            else if (strEqI(node.type, "fb_div")) mathOp = "/";

            // L in1
            emitIndent();
            emit("L ");
            emitTypedAddress(node.in1, typeSuffix);
            emitChar('\n');

            // +I in2 (or -I, *I, /I, +D, -D, etc.)
            emitIndent();
            emit(mathOp);
            emitChar(typeSuffix);
            emit(" ");
            emitTypedAddress(node.in2, typeSuffix);
            emitChar('\n');

            // T out
            emitIndent();
            emit("T ");
            emitTypedAddress(node.out, typeSuffix);
            emitChar('\n');
        }
        // MOD - use extended syntax
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
        // Bitwise operations
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

        // Emit the skip label
        emit("_op");
        emitInt(skipLabel);
        emit(":\n");

        if (emitTapAfter) {
            emitLine("TAP");
        }
    }

    // Emit STL for a comparator block (produces boolean result like a contact)
    // Uses Siemens-style comparisons: L value, <I #literal or L val1, L val2, <I
    void emitComparator(GraphNode& node, bool asFirstCondition) {
        char typeSuffix = getSiemensTypeSuffix(node.data_type);

        // Determine comparison operator
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

        // Check if in2 is a literal (number)
        bool in2IsLiteral = true;
        const char* p = node.in2;
        if (*p == '-') p++;  // skip negative sign
        while (*p) {
            if (*p < '0' || *p > '9') {
                in2IsLiteral = false;
                break;
            }
            p++;
        }

        // Siemens STL comparison format:
        // If in2 is literal: L in1, <I #literal
        // If in2 is address: L in1, L in2, <I
        emitIndent();
        emit("L ");
        emitTypedAddress(node.in1, typeSuffix);
        emitChar('\n');

        if (in2IsLiteral) {
            // Comparison with immediate value
            emitIndent();
            emit(op);
            emitChar(typeSuffix);
            emit(" #");
            emit(node.in2);
            emitChar('\n');
        } else {
            // Comparison with memory address
            emitIndent();
            emit("L ");
            emitTypedAddress(node.in2, typeSuffix);
            emitChar('\n');

            emitIndent();
            emit(op);
            emitChar(typeSuffix);
            emitChar('\n');
        }
    }

    // ============ Graph Traversal and Code Generation ============

    // Sort nodes by x,y coordinates (left-to-right, top-to-bottom)
    void sortNodesByPosition() {
        // Simple bubble sort for embedded compatibility
        for (int i = 0; i < node_count - 1; i++) {
            for (int j = 0; j < node_count - i - 1; j++) {
                bool swap = false;
                if (nodes[j].x > nodes[j + 1].x) {
                    swap = true;
                } else if (nodes[j].x == nodes[j + 1].x && nodes[j].y > nodes[j + 1].y) {
                    swap = true;
                }
                if (swap) {
                    GraphNode tmp = nodes[j];
                    nodes[j] = nodes[j + 1];
                    nodes[j + 1] = tmp;
                }
            }
        }

        // Assign order indices
        for (int i = 0; i < node_count; i++) {
            nodes[i].order_index = i;
        }
    }

    // Find starting nodes (nodes with no input connections)
    void findStartNodes(int* startNodes, int& count, int maxCount) {
        count = 0;
        for (int i = 0; i < node_count && count < maxCount; i++) {
            int inputConns[MAX_CONNECTIONS];
            int inputCount;
            getInputConnections(nodes[i].id, inputConns, inputCount, MAX_CONNECTIONS);
            if (inputCount == 0) {
                startNodes[count++] = i;
            }
        }
    }

    // Find ending nodes (nodes with no output connections)
    void findEndNodes(int* endNodes, int& count, int maxCount) {
        count = 0;
        for (int i = 0; i < node_count && count < maxCount; i++) {
            int outputConns[MAX_CONNECTIONS];
            int outputCount;
            getOutputConnections(nodes[i].id, outputConns, outputCount, MAX_CONNECTIONS);
            if (outputCount == 0) {
                endNodes[count++] = i;
            }
        }
    }

    // Recursively emit condition logic leading to a node
    // Returns true if something was emitted
    // stopAtEmittedOutputs: if true, stop recursion at output nodes that have been emitted (with TAP)
    bool emitConditionForNode(int nodeIdx, int depth, bool isOrBranch, bool stopAtEmittedOutputs = true) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return false;
        GraphNode& node = nodes[nodeIdx];

        // Prevent infinite recursion
        if (node.visited) return false;
        node.visited = true;

        // If this node has been marked as emitted (part of common chain), stop here
        // This is used when emitting divergent OR branches to avoid re-emitting shared ancestors
        if (node.emitted) {
            node.visited = false;
            return false;  // Don't emit anything - already emitted
        }

        // Get all source nodes for this node
        int sourceIndices[MAX_NODES];
        int sourceCount;
        getSourceNodes(node.id, sourceIndices, sourceCount, MAX_NODES);

        // If this is an input (contact), emit it
        if (isContact(node.type)) {
            // First, emit conditions from sources (if any)
            if (sourceCount > 0) {
                if (sourceCount > 1) {
                    // Multiple sources = OR logic - use smart OR emission
                    emitOrSources(sourceIndices, sourceCount, depth, stopAtEmittedOutputs);
                } else {
                    emitConditionForNode(sourceIndices[0], depth + 1, isOrBranch, stopAtEmittedOutputs);
                }
            }

            // Emit this contact
            emitContact(node, sourceCount == 0);
            node.visited = false;
            return true;
        }

        // If this is a function block (timer/counter)
        if (isFunctionBlock(node.type)) {
            // Emit input conditions
            if (sourceCount > 1) {
                // Multiple sources = OR logic - use smart OR emission
                emitOrSources(sourceIndices, sourceCount, depth, stopAtEmittedOutputs);
            } else if (sourceCount == 1) {
                emitConditionForNode(sourceIndices[0], depth + 1, isOrBranch, stopAtEmittedOutputs);
            }

            // Emit the function block
            if (isTimer(node.type)) {
                emitTimer(node);
            } else if (isCounter(node.type)) {
                emitCounter(node);
            }
            node.visited = false;
            return true;
        }

        // If this is a coil used as source, we need to:
        // 1. Emit its input conditions
        // 2. Execute the coil instruction
        // 3. TAP to pass the RLO to downstream logic
        if (isCoil(node.type)) {
            // If already emitted, just read its address state
            if (node.emitted) {
                emitContact(node, depth == 0 && !isOrBranch);
                node.visited = false;
                return true;
            }

            // Emit input conditions for this coil
            if (sourceCount > 1) {
                emitOrSources(sourceIndices, sourceCount, depth, stopAtEmittedOutputs);
            } else if (sourceCount == 1) {
                emitConditionForNode(sourceIndices[0], depth + 1, isOrBranch, stopAtEmittedOutputs);
            }

            // Emit the coil - only TAP if it has downstream nodes that need the RLO
            bool needsTap = hasDownstreamContact(nodeIdx) || hasDownstreamOutput(nodeIdx);
            emitCoil(node, needsTap);
            node.emitted = true;
            node.visited = false;
            return true;
        }

        // For other nodes, just emit sources using OR pattern
        if (sourceCount > 1) {
            // Multiple sources = OR logic - use smart OR emission
            emitOrSources(sourceIndices, sourceCount, depth, stopAtEmittedOutputs);
        } else if (sourceCount == 1) {
            emitConditionForNode(sourceIndices[0], depth + 1, isOrBranch, stopAtEmittedOutputs);
        }

        node.visited = false;
        return sourceCount > 0;
    }

    // Check if all sources are simple contacts (no chains behind them)
    bool allSourcesAreSimpleContacts(int* sourceIndices, int sourceCount) {
        for (int i = 0; i < sourceCount; i++) {
            GraphNode& node = nodes[sourceIndices[i]];
            if (!isContact(node.type)) return false;

            // Check if this contact has sources of its own
            int srcIndices[MAX_NODES];
            int srcCount;
            getSourceNodes(node.id, srcIndices, srcCount, MAX_NODES);
            if (srcCount > 0) return false;  // Has chain behind it
        }
        return true;
    }

    // Sort source indices by y position (top to bottom)
    void sortSourcesByPosition(int* sourceIndices, int sourceCount) {
        // Simple bubble sort
        for (int i = 0; i < sourceCount - 1; i++) {
            for (int j = 0; j < sourceCount - i - 1; j++) {
                int yA = nodes[sourceIndices[j]].y;
                int yB = nodes[sourceIndices[j + 1]].y;
                // If same y, sort by x
                if (yA > yB || (yA == yB && nodes[sourceIndices[j]].x > nodes[sourceIndices[j + 1]].x)) {
                    int tmp = sourceIndices[j];
                    sourceIndices[j] = sourceIndices[j + 1];
                    sourceIndices[j + 1] = tmp;
                }
            }
        }
    }

    // Check if a source node is a "simple" source that can be directly OR'd
    // A source is simple for OR if:
    // - It's a contact with no chain behind it, OR
    // - It's a contact whose entire chain was already emitted (shared ancestor)
    bool isSimpleOrSource(int nodeIdx, int* alreadyEmittedChain, int emittedCount) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return false;
        GraphNode& node = nodes[nodeIdx];

        if (!isContact(node.type)) return false;

        // Check if this contact has sources
        int srcIndices[MAX_NODES];
        int srcCount;
        getSourceNodes(node.id, srcIndices, srcCount, MAX_NODES);

        // No sources = simple contact
        if (srcCount == 0) return true;

        // Check if all sources were already emitted in a previous branch
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

    // Collect all nodes in a condition chain (for tracking what was emitted)
    void collectChainNodes(int nodeIdx, int* chainNodes, int& count, int maxCount) {
        if (nodeIdx < 0 || nodeIdx >= node_count || count >= maxCount) return;
        GraphNode& node = nodes[nodeIdx];
        if (node.visited) return;
        node.visited = true;

        // Add this node
        chainNodes[count++] = nodeIdx;

        // Recurse to sources
        int srcIndices[MAX_NODES];
        int srcCount;
        getSourceNodes(node.id, srcIndices, srcCount, MAX_NODES);
        for (int i = 0; i < srcCount && count < maxCount; i++) {
            collectChainNodes(srcIndices[i], chainNodes, count, maxCount);
        }

        node.visited = false;
    }

    // Find the common ancestor of multiple source nodes
    // Returns the index of the common ancestor, or -1 if none found
    int findCommonAncestor(int* sourceIndices, int sourceCount) {
        if (sourceCount < 2) return -1;

        // Collect ancestors of the first source
        int ancestors0[MAX_NODES];
        int ancestorCount0 = 0;
        for (int i = 0; i < node_count; i++) nodes[i].visited = false;
        collectChainNodes(sourceIndices[0], ancestors0, ancestorCount0, MAX_NODES);
        for (int i = 0; i < node_count; i++) nodes[i].visited = false;

        // Find common ancestors with all other sources
        for (int a = 0; a < ancestorCount0; a++) {
            int candidateIdx = ancestors0[a];
            // Skip the source node itself
            if (candidateIdx == sourceIndices[0]) continue;

            bool isCommonToAll = true;
            for (int s = 1; s < sourceCount && isCommonToAll; s++) {
                // Check if this candidate is an ancestor of source s
                int ancestorsS[MAX_NODES];
                int ancestorCountS = 0;
                for (int i = 0; i < node_count; i++) nodes[i].visited = false;
                collectChainNodes(sourceIndices[s], ancestorsS, ancestorCountS, MAX_NODES);
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

    // Emit OR sources - handles both simple contacts and complex chains
    void emitOrSources(int* sourceIndices, int sourceCount, int depth, bool stopAtEmittedOutputs) {
        if (sourceCount == 0) return;

        // Sort sources by y position (top to bottom)
        sortSourcesByPosition(sourceIndices, sourceCount);

        // If all sources are simple contacts (no chains), use inline OR format
        if (allSourcesAreSimpleContacts(sourceIndices, sourceCount)) {
            // First source uses A/AN
            emitContact(nodes[sourceIndices[0]], true, false);
            // Remaining sources use O/ON
            for (int s = 1; s < sourceCount; s++) {
                emitContact(nodes[sourceIndices[s]], false, true);
            }
            return;
        }

        // Check for common ancestor pattern:
        // If all sources share a common ancestor, emit:
        //   <common chain> SAVE A( O( L BR <first unique part> ) O( L BR <second unique part> ) ) DROP BR
        int commonAncestorIdx = findCommonAncestor(sourceIndices, sourceCount);

        if (commonAncestorIdx >= 0) {
            // Emit the common ancestor chain first
            for (int i = 0; i < node_count; i++) nodes[i].visited = false;
            emitConditionForNode(commonAncestorIdx, depth + 1, false, stopAtEmittedOutputs);

            // Mark the common ancestor and its chain as emitted so they won't be re-emitted
            int emittedChain[MAX_NODES];
            int emittedCount = 0;
            for (int i = 0; i < node_count; i++) nodes[i].visited = false;
            collectChainNodes(commonAncestorIdx, emittedChain, emittedCount, MAX_NODES);
            for (int i = 0; i < node_count; i++) nodes[i].visited = false;

            // Mark all nodes in common chain as emitted
            for (int i = 0; i < emittedCount; i++) {
                nodes[emittedChain[i]].emitted = true;
            }

            // Check if all sources are simple contacts with all their sources already emitted
            // If so, we can use standard nesting without SAVE
            bool allSimpleContacts = true;
            for (int s = 0; s < sourceCount && allSimpleContacts; s++) {
                if (!isContact(nodes[sourceIndices[s]].type)) {
                    allSimpleContacts = false;
                    break;
                }
                // Check if all sources of this contact are in the emitted chain
                int srcIndices[MAX_NODES];
                int srcCount;
                getSourceNodes(nodes[sourceIndices[s]].id, srcIndices, srcCount, MAX_NODES);
                for (int i = 0; i < srcCount && allSimpleContacts; i++) {
                    bool found = false;
                    for (int j = 0; j < emittedCount; j++) {
                        if (srcIndices[i] == emittedChain[j]) { found = true; break; }
                    }
                    if (!found) allSimpleContacts = false;
                }
            }

            if (allSimpleContacts) {
                // All sources are simple contacts directly connected to the common chain
                // Use standard nesting without SAVE: A( A M0.1 O M0.2 O M0.3 )
                emitLine("A(");
                indent_level++;
                for (int s = 0; s < sourceCount; s++) {
                    emitContact(nodes[sourceIndices[s]], s == 0, s > 0);
                }
                indent_level--;
                emitLine(")");

                // Unmark emitted nodes
                for (int i = 0; i < emittedCount; i++) {
                    nodes[emittedChain[i]].emitted = false;
                }
                return;
            }

            // Save RLO before branching so each branch can access it with L BR
            emitLine("SAVE");

            // In SAVE pattern, all branches need O( L BR ... ) to be properly gated
            // Emit each source's unique part wrapped in O( L BR ... )
            for (int s = 0; s < sourceCount; s++) {
                for (int i = 0; i < node_count; i++) nodes[i].visited = false;

                // Check if this source is just a simple contact with all sources emitted
                bool isSimpleEmittedContact = false;
                if (isContact(nodes[sourceIndices[s]].type)) {
                    int srcIndices[MAX_NODES];
                    int srcCount;
                    getSourceNodes(nodes[sourceIndices[s]].id, srcIndices, srcCount, MAX_NODES);

                    isSimpleEmittedContact = true;
                    for (int i = 0; i < srcCount && isSimpleEmittedContact; i++) {
                        bool found = false;
                        for (int j = 0; j < emittedCount; j++) {
                            if (srcIndices[i] == emittedChain[j]) { found = true; break; }
                        }
                        if (!found) isSimpleEmittedContact = false;
                    }
                }

                // All branches wrapped in O( L BR ... ) for proper gating with common ancestor
                if (s == 0) {
                    // First branch doesn't need O( wrapper, just L BR
                    emitLine("L BR");
                    if (isSimpleEmittedContact) {
                        emitContact(nodes[sourceIndices[s]], true, false);  // A/AN contact
                    } else {
                        emitConditionForNode(sourceIndices[s], depth + 1, false, stopAtEmittedOutputs);
                    }
                } else {
                    // Subsequent branches wrapped in O( L BR ... )
                    emitLine("O(");
                    indent_level++;
                    emitLine("L BR");
                    if (isSimpleEmittedContact) {
                        emitContact(nodes[sourceIndices[s]], true, false);  // A/AN contact
                    } else {
                        emitConditionForNode(sourceIndices[s], depth + 2, false, stopAtEmittedOutputs);
                    }
                    indent_level--;
                    emitLine(")");
                }
            }

            emitLine("DROP BR");  // Clean up the saved RLO

            // Unmark emitted nodes
            for (int i = 0; i < emittedCount; i++) {
                nodes[emittedChain[i]].emitted = false;
            }
        } else {
            // No common ancestor - fall back to standard OR handling
            // First branch emits normally
            emitConditionForNode(sourceIndices[0], depth + 1, false, stopAtEmittedOutputs);

            // Track what was emitted in the first branch
            int emittedChain[MAX_NODES];
            int emittedCount = 0;
            for (int i = 0; i < node_count; i++) nodes[i].visited = false;
            collectChainNodes(sourceIndices[0], emittedChain, emittedCount, MAX_NODES);
            for (int i = 0; i < node_count; i++) nodes[i].visited = false;

            // Subsequent branches: wrap in O(...)
            for (int s = 1; s < sourceCount; s++) {
                int srcIdx = sourceIndices[s];

                // Check if this source is a contact whose chain was already emitted
                if (isSimpleOrSource(srcIdx, emittedChain, emittedCount)) {
                    emitContact(nodes[srcIdx], false, true);  // O contact
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

    // Emit a simple contact as OR (O address instead of A address)
    // Used for inline OR: A X0.0 / O X0.1 instead of O( A X0.0 O A X0.1 )
    void emitConditionForNodeAsOr(int nodeIdx, int depth) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return;
        GraphNode& node = nodes[nodeIdx];

        // Only handles simple contacts for now
        if (isContact(node.type)) {
            emitContact(node, false, true);  // useOr = true
        }
        // For non-contacts, fall back to regular emission (should not happen in simple OR cases)
    }

    // Find all output nodes (coils, taps) that are destinations
    void findOutputNodes(int* outputNodes, int& count, int maxCount) {
        count = 0;
        for (int i = 0; i < node_count && count < maxCount; i++) {
            if (isOutputNode(nodes[i].type)) {
                outputNodes[count++] = i;
            }
        }
    }

    // Check if a node is a terminal (has no outgoing connections or only non-input destinations)
    bool isTerminalOutput(int nodeIdx) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return false;
        GraphNode& node = nodes[nodeIdx];

        // Must be an output/termination type
        if (!isTerminationNode(node.type)) return false;

        // Check outgoing connections
        int destIndices[MAX_NODES];
        int destCount;
        getDestNodes(node.id, destIndices, destCount, MAX_NODES);

        // If no outgoing connections, it's terminal
        if (destCount == 0) return true;

        // If has outgoing connections to inputs/contacts, it's NOT terminal (it's passthrough)
        for (int d = 0; d < destCount; d++) {
            if (isInputNode(nodes[destIndices[d]].type)) return false;
            if (isTerminationNode(nodes[destIndices[d]].type)) return false;
        }

        return true;
    }

    // Find all terminal output nodes (outputs with no outgoing connections that continue the chain)
    void findTerminalOutputNodes(int* outputNodes, int& count, int maxCount) {
        count = 0;
        for (int i = 0; i < node_count && count < maxCount; i++) {
            if (isTerminalOutput(i)) {
                outputNodes[count++] = i;
            }
        }
    }

    // Group output nodes by their shared input condition
    // Returns groups where each group shares the same source chain
    struct OutputGroup {
        int nodeIndices[MAX_NODES];
        int count;
        int conditionSourceIdx;  // -1 if multiple sources

        void clear() {
            count = 0;
            conditionSourceIdx = -1;
        }
    };

    // Main compilation function
    bool compile() {

        // Sort nodes by position (left-to-right, top-to-bottom)
        sortNodesByPosition();

        // Find all output nodes (coils, timers, counters)
        int outputNodes[MAX_NODES];
        int outputCount;
        findOutputNodes(outputNodes, outputCount, MAX_NODES);

        if (outputCount == 0) {
            // No outputs - nothing to compile
            return true;
        }

        // Track which outputs have been processed
        bool outputProcessed[MAX_NODES] = { false };

        // First pass: handle parallel branches (any node that feeds multiple termination nodes)
        // Termination nodes include coils, timers, counters, comparators and operation blocks
        // This handles both single-connection multiple destinations AND multiple connections from same source
        for (int n = 0; n < node_count; n++) {
            // Skip if this node type can't branch (outputs don't branch, they receive)
            GraphNode& srcNode = nodes[n];
            if (!isInputNode(srcNode.type) && !isTerminationNode(srcNode.type)) continue;

            // Get all destinations from this node (across ALL connections)
            int allDestIndices[MAX_NODES];
            int allDestCount;
            getDestNodes(srcNode.id, allDestIndices, allDestCount, MAX_NODES);

            // Filter to termination nodes only (coils, timers, counters, comparators, ops)
            int termDests[MAX_NODES];
            int termDestCount = 0;
            for (int d = 0; d < allDestCount; d++) {
                int destIdx = allDestIndices[d];
                GraphNode& dest = nodes[destIdx];
                // Include termination nodes AND comparators (which act like contacts)
                if ((isTerminationNode(dest.type) || isComparator(dest.type)) && !outputProcessed[destIdx] && !dest.emitted) {
                    termDests[termDestCount++] = destIdx;
                }
            }

            // Skip if not multiple termination destinations
            if (termDestCount < 2) continue;

            // Mark the source node as visited to avoid re-processing
            srcNode.visited = true;

            // Emit condition chain for the source
            for (int i = 0; i < node_count; i++) nodes[i].visited = false;
            emitConditionForNode(n, 0, false);

            // Sort destinations by Y position (top to bottom)
            for (int i = 0; i < termDestCount - 1; i++) {
                for (int j = i + 1; j < termDestCount; j++) {
                    if (nodes[termDests[j]].y < nodes[termDests[i]].y) {
                        int tmp = termDests[i];
                        termDests[i] = termDests[j];
                        termDests[j] = tmp;
                    }
                }
            }

            // Use branch stack for parallel outputs
            emitLine("SAVE");
            indent_level++;
            for (int d = 0; d < termDestCount; d++) {
                int destIdx = termDests[d];
                emitLine("L BR");

                GraphNode& destNode = nodes[destIdx];
                bool hasContinuation = hasDownstreamContact(destIdx);
                bool hasMoreOutputs = hasDownstreamOutput(destIdx);

                // Handle comparators first (they're like contacts, emit and continue)
                if (isComparator(destNode.type)) {
                    // Wrap comparator in A(...) block so it gets AND'ed with L BR
                    emitLine("A(");
                    indent_level++;
                    emitComparator(destNode, true);  // isFirst = true
                    indent_level--;
                    emitLine(")");
                    // Follow the chain from this comparator
                    int compDestIndices[MAX_NODES];
                    int compDestCount;
                    getDestNodes(destNode.id, compDestIndices, compDestCount, MAX_NODES);
                    for (int cd = 0; cd < compDestCount; cd++) {
                        int compDestIdx = compDestIndices[cd];
                        GraphNode& compDest = nodes[compDestIdx];
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
                    // Coils need TAP if they have ANY downstream nodes (contacts OR other coils)
                    // because the coil consumes the RLO from the stack
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

        // Second pass: process remaining outputs in position order
        for (int o = 0; o < outputCount; o++) {
            int destIdx = outputNodes[o];
            if (outputProcessed[destIdx]) continue;

            GraphNode& destNode = nodes[destIdx];

            // Skip if already emitted (e.g., as passthrough in OR branch)
            if (destNode.emitted) continue;

            // Get source nodes for this output
            int sourceIndices[MAX_NODES];
            int sourceCount;
            getSourceNodes(destNode.id, sourceIndices, sourceCount, MAX_NODES);

            // Check if source is an already-executed output node (coil)
            // If so, just read its address instead of re-emitting the entire chain
            bool sourceIsEmittedCoil = false;
            if (sourceCount == 1) {
                GraphNode& srcNode = nodes[sourceIndices[0]];
                if (isCoil(srcNode.type) && srcNode.emitted) {
                    sourceIsEmittedCoil = true;
                    // Just read the coil's address
                    emitContact(srcNode, true);
                }
            }

            if (!sourceIsEmittedCoil) {
                // Emit condition chain
                if (sourceCount > 1) {
                    // Multiple sources = OR logic - use emitOrSources for proper A(/O()) pattern
                    emitOrSources(sourceIndices, sourceCount, 0, true);
                } else if (sourceCount == 1) {
                    for (int i = 0; i < node_count; i++) nodes[i].visited = false;
                    emitConditionForNode(sourceIndices[0], 0, false);
                }
            }

            // Emit output - check if TAP is needed (downstream contact exists)
            // TAP is only needed if the RLO will be read by a downstream contact
            bool needTapAfter = hasDownstreamContact(destIdx);
            // For operations: TAP before JCN if there are more outputs downstream
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
            destNode.emitted = true;  // Mark as emitted

            // Follow and emit any downstream chain (contact → coil sequences)
            if (hasDownstreamChain(destIdx)) {
                emitDownstreamChain(destIdx, outputProcessed);
            }

            emitChar('\n');
        }

        return !has_error;
    }

    // ============ Main Entry Point ============

    // ============ Graph Validation ============

    // Validate a single node
    virtual bool validateNode(int nodeIdx) {
        if (nodeIdx < 0 || nodeIdx >= node_count) return true;
        GraphNode& node = nodes[nodeIdx];

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
            reportNodeError(msg, nodeIdx);
            return false;
        }

        // Check address/symbol for most node types (tap nodes don't need addresses)
        // Comparators and operation blocks use in1/in2/out fields instead of address
        if (!isTapNode(node.type) && !isComparator(node.type) && !isOperationBlock(node.type)) {
            if (node.address[0] == '\0') {
                reportNodeError("Missing address/symbol", nodeIdx);
                return false;
            }
            // Validate symbol exists in shared symbol table
            if (!validateSymbolExists(node.address, nodeIdx)) {
                return false;
            }
        }

        // Validate node-type-specific parameters
        if (isTimer(node.type) || isCounter(node.type)) {
            // Timers and counters need a preset value
            if (node.preset == 0 && node.preset_str[0] == '\0') {
                reportNodeWarning("Timer/counter has no preset value", nodeIdx);
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
                
                // Must have at least one way to specify the address
                if (!hasAddress && !hasIn1 && !hasOut) {
                    reportNodeError("INC/DEC operation block missing address (use 'address', 'in1', or 'out')", nodeIdx);
                    return false;
                }
                
                // Validate the symbol that was provided
                const char* addrToValidate = hasAddress ? node.address : (hasIn1 ? node.in1 : node.out);
                if (!validateSymbolExists(addrToValidate, nodeIdx)) {
                    return false;
                }
            } else {
                // Other operation blocks use in1/in2/out fields
                bool needsIn1 = true;
                bool needsIn2 = !strEqI(node.type, "fb_not") && !strEqI(node.type, "fb_move");
                bool needsOut = true;

                if (needsIn1 && node.in1[0] == '\0') {
                    reportNodeError("Operation block missing input 1 (in1)", nodeIdx);
                    return false;
                }
                // Validate in1 symbol
                if (needsIn1 && node.in1[0] != '\0' && !validateSymbolExists(node.in1, nodeIdx)) {
                    return false;
                }
                if (needsIn2 && node.in2[0] == '\0') {
                    reportNodeError("Operation block missing input 2 (in2)", nodeIdx);
                    return false;
                }
                // Validate in2 symbol
                if (needsIn2 && node.in2[0] != '\0' && !validateSymbolExists(node.in2, nodeIdx)) {
                    return false;
                }
                if (needsOut && node.out[0] == '\0') {
                    reportNodeError("Operation block missing output (out)", nodeIdx);
                    return false;
                }
                // Validate out symbol
                if (needsOut && node.out[0] != '\0' && !validateSymbolExists(node.out, nodeIdx)) {
                    return false;
                }
            }
        }

        if (isComparator(node.type)) {
            // Comparators need two inputs
            if (node.in1[0] == '\0') {
                reportNodeError("Comparator missing input 1 (in1)", nodeIdx);
                return false;
            }
            // Validate in1 symbol
            if (!validateSymbolExists(node.in1, nodeIdx)) {
                return false;
            }
            if (node.in2[0] == '\0') {
                reportNodeError("Comparator missing input 2 (in2)", nodeIdx);
                return false;
            }
            // Validate in2 symbol
            if (!validateSymbolExists(node.in2, nodeIdx)) {
                return false;
            }
        }

        return true;
    }

    // Validate a single connection
    virtual bool validateConnection(int connIdx) {
        if (connIdx < 0 || connIdx >= connection_count) return true;
        Connection& conn = connections[connIdx];

        // Check that all sources exist and validate flow direction
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
                // Use first destination node for error position if available
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

            // Check that destinations have x > source x (left-to-right flow)
            GraphNode& srcNode = nodes[srcIdx];
            for (int d = 0; d < conn.dest_count; d++) {
                int destIdx = findNodeById(conn.destinations[d]);
                if (destIdx >= 0) {
                    GraphNode& destNode = nodes[destIdx];
                    if (destNode.x <= srcNode.x) {
                        reportConnectionError("Connection flows backward (destination must be right of source)", connIdx, destIdx);
                        return false;
                    }
                }
            }
        }

        // Check that all destinations exist
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
                // Use first source node for error position
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

    // Validate entire graph structure
    virtual bool validateGraph() {
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
                    reportNodeError(msg, i);
                    if (has_error) return false;
                }
            }
        }

        // Validate all nodes
        for (int i = 0; i < node_count; i++) {
            if (!validateNode(i)) {
                if (has_error) return false;
            }
        }

        // Validate all connections
        for (int i = 0; i < connection_count; i++) {
            if (!validateConnection(i)) {
                if (has_error) return false;
            }
        }

        // Check for dead nodes (nodes with no connections at all)
        for (int i = 0; i < node_count; i++) {
            // Tap nodes are allowed to be connection points without their own logic
            if (isTapNode(nodes[i].type)) continue;

            bool hasInputs = nodeHasInputs(i);
            bool hasOutputs = nodeHasOutputs(i);

            // Input nodes (contacts) should have outputs
            if (isInputNode(nodes[i].type) && !hasOutputs) {
                reportNodeWarning("Contact not connected to any output", i);
            }

            // Output nodes (coils) should have inputs
            if (isTerminationNode(nodes[i].type) && !hasInputs) {
                reportNodeWarning("Output has no inputs", i);
            }

            // Completely isolated nodes are errors
            if (!hasInputs && !hasOutputs) {
                reportNodeError("Node is completely disconnected", i);
                if (has_error) return false;
            }

            // Starting nodes (no input connections) should be at x=0
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

        // Parse the graph JSON
        if (!parseGraph()) {
            return false;
        }

        // Normalize connections: merge connections that share sources/destinations
        // This creates many-to-many relationships for parallel power transfer detection
        normalizeConnections();

        // Validate the graph structure
        if (!validateGraph()) {
            return false;
        }

        // Compile to STL
        if (!compile()) {
            return false;
        }

        // Check for empty output - this is an error
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

// Global output buffer for WASM export (avoids reinitialization issues)
char ladder_output_buffer[65536];
int ladder_output_length = 0;

extern "C" {

    // ============ Standalone Ladder Compiler: Ladder Graph JSON -> STL ============
    // NOTE: These functions use a SEPARATE instance (ladderGraphCompiler) from
    // the project compiler's internal ladder_compiler (LadderLinter).
    // For project compilation, use project_compile() which handles ladder internally.

    WASM_EXPORT void ladder_standalone_clear() {
        ladderGraphCompiler.clearState();
        ladder_input_length = 0;
        ladder_output_length = 0;
    }

    WASM_EXPORT void ladder_standalone_load_stream() {
        streamRead(ladder_input_buffer, ladder_input_length, sizeof(ladder_input_buffer));
    }

    WASM_EXPORT int ladder_standalone_compile() {
        // Parse and compile Ladder Graph JSON directly to STL
        bool result = ladderGraphCompiler.parse(ladder_input_buffer, ladder_input_length);
        if (result) {
            // Copy output to global buffer to avoid reinitialization issues
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
        return ladderGraphCompiler.has_error;
    }

    WASM_EXPORT const char* ladder_standalone_get_error() {
        return ladderGraphCompiler.error_msg;
    }

    WASM_EXPORT void ladder_standalone_error_to_stream() {
        const char* err = ladderGraphCompiler.error_msg;
        for (int i = 0; err[i] != '\0'; i++) {
            streamOut(err[i]);
        }
    }

    // ============ Full Pipeline: Ladder Graph -> STL -> PLCASM -> Bytecode ============

    WASM_EXPORT bool ladder_standalone_compile_full() {
        // Step 1: Parse Ladder Graph to STL
        if (!ladderGraphCompiler.parse(ladder_input_buffer, ladder_input_length)) {
            return false;
        }

        // Step 2: Compile STL to PLCASM
        stlCompiler.setSource(ladderGraphCompiler.output, ladderGraphCompiler.output_len);
        if (!stlCompiler.compile()) {
            return false;
        }

        // Step 3: Compile PLCASM to bytecode
        defaultCompiler.set_assembly_string((char*) stlCompiler.getOutput());
        return !defaultCompiler.compileAssembly(false, false);
    }

} // extern "C"

#endif // __WASM__
