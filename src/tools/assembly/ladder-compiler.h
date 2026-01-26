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

#include "network-ir.h"
#include "stl-compiler.h"


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
    int output_len;
    
    // Edge memory counter for auto-generated edge state bits
    int edge_mem_counter;
    
    // Label counter for conditional jumps
    int label_counter;
    
    // Indentation level for nested structures
    int indent_level;
    
    LadderGraphCompiler() { reset(); }
    
    // Virtual destructor for proper cleanup in derived classes
    virtual ~LadderGraphCompiler() = default;
    
    virtual void reset() {
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
    
    // ============ Node Lookup ============
    
    int findNodeById(const char* id) {
        for (int i = 0; i < node_count; i++) {
            if (strEqI(nodes[i].id, id)) return i;
        }
        return -1;
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
        
        while (peek() != '}' && peek() != '\0') {
            char key[32];
            if (!readString(key, sizeof(key))) return false;
            if (!expect(':')) { setError("Expected ':'"); return false; }
            
            if (strEqI(key, "sources")) {
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
        
        return true;
    }
    
    bool parseGraph() {
        if (!expect('{')) {
            setError("Expected JSON object");
            return false;
        }
        
        while (peek() != '}' && peek() != '\0') {
            char key[32];
            if (!readString(key, sizeof(key))) return false;
            if (!expect(':')) { setError("Expected ':'"); return false; }
            
            if (strEqI(key, "nodes")) {
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
        
        return true;
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
        }
        else {
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
        emit("// Generated from Ladder Graph by VovkPLCRuntime\n\n");
        
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
    
    bool parse(const char* src, int len) {
        reset();
        source = src;
        length = len;
        
        // Parse the graph JSON
        if (!parseGraph()) {
            return false;
        }
        
        // Compile to STL
        if (!compile()) {
            return false;
        }
        
        return !has_error;
    }
};


// ============================================================================
// STL to Network IR Parser
// ============================================================================
//
// Parses Siemens-style STL into Network IR representation.
// Supports: A, AN, O, ON, X, XN, NOT, SET, CLR, =, S, R, FP, FN
//           TON, TOF, TP, CTU, CTD and nesting with O( ... )
//
// ============================================================================

class STLToIRParser {
public:
    const char* source;
    int length;
    int pos;
    int line;
    int col;
    
    // Output buffer for error messages
    char error_msg[256];
    bool has_error;
    
    // RLO expression stack for nested logic
    static const int MAX_EXPR_STACK = 64;
    uint32_t expr_stack[MAX_EXPR_STACK];
    int expr_stack_top;
    
    // OR branch tracking
    static const int MAX_OR_BRANCHES = 32;
    uint32_t or_branches[MAX_OR_BRANCHES];
    int or_branch_count;
    bool in_or_block;
    
    // Current network actions
    static const int MAX_PENDING_ACTIONS = 32;
    uint32_t pending_actions[MAX_PENDING_ACTIONS];
    int pending_action_count;
    uint32_t action_start_idx;
    
    STLToIRParser() { reset(); }
    
    void reset() {
        source = nullptr;
        length = 0;
        pos = 0;
        line = 1;
        col = 1;
        error_msg[0] = '\0';
        has_error = false;
        expr_stack_top = 0;
        or_branch_count = 0;
        in_or_block = false;
        pending_action_count = 0;
        action_start_idx = 0;
    }
    
    void setError(const char* msg) {
        if (has_error) return;
        has_error = true;
        int i = 0;
        while (msg[i] && i < 255) {
            error_msg[i] = msg[i];
            i++;
        }
        error_msg[i] = '\0';
    }
    
    // ============ Lexer Helpers ============
    
    char peek(int offset = 0) {
        int p = pos + offset;
        return (p < length) ? source[p] : '\0';
    }
    
    char advance() {
        if (pos >= length) return '\0';
        char c = source[pos++];
        if (c == '\n') { line++; col = 1; }
        else col++;
        return c;
    }
    
    void skipWhitespace() {
        while (pos < length) {
            char c = peek();
            if (c == ' ' || c == '\t' || c == '\r') advance();
            else break;
        }
    }
    
    void skipLine() {
        while (pos < length && peek() != '\n') advance();
    }
    
    bool isAlpha(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }
    
    bool isDigit(char c) {
        return c >= '0' && c <= '9';
    }
    
    bool isAlphaNum(char c) {
        return isAlpha(c) || isDigit(c);
    }
    
    bool readToken(char* buf, int maxLen) {
        skipWhitespace();
        int i = 0;
        
        // Special handling for single-character operators: = S R
        char c = peek();
        if (c == '=' || c == ')') {
            buf[0] = advance();
            buf[1] = '\0';
            return true;
        }
        
        while (i < maxLen - 1) {
            char c = peek();
            if (isAlphaNum(c) || c == '.' || c == '#' || c == '_' || c == ':') {
                buf[i++] = advance();
            } else {
                break;
            }
        }
        buf[i] = '\0';
        return i > 0;
    }
    
    bool strEqI(const char* a, const char* b) {
        while (*a && *b) {
            char ca = (*a >= 'a' && *a <= 'z') ? *a - 32 : *a;
            char cb = (*b >= 'a' && *b <= 'z') ? *b - 32 : *b;
            if (ca != cb) return false;
            a++; b++;
        }
        return *a == *b;
    }
    
    // ============ Expression Stack ============
    
    void pushExpr(uint32_t expr) {
        if (expr_stack_top < MAX_EXPR_STACK) {
            expr_stack[expr_stack_top++] = expr;
        }
    }
    
    uint32_t popExpr() {
        if (expr_stack_top > 0) {
            return expr_stack[--expr_stack_top];
        }
        return nir::NIR_NONE;
    }
    
    uint32_t peekExpr() {
        if (expr_stack_top > 0) {
            return expr_stack[expr_stack_top - 1];
        }
        return nir::NIR_NONE;
    }
    
    // Combine top two expressions with AND
    void combineAnd() {
        if (expr_stack_top >= 2) {
            uint32_t b = popExpr();
            uint32_t a = popExpr();
            uint32_t children[2] = { a, b };
            uint32_t and_expr = nir::g_store.addAnd(children, 2);
            pushExpr(and_expr);
        }
    }
    
    // ============ Control Flow Detection ============
    
    bool hasControlFlow() {
        const char* p = source;
        char word[32];
        
        while (*p) {
            // Skip whitespace
            while (*p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) p++;
            if (!*p) break;
            
            // Skip comments
            if (*p == '/' && *(p+1) == '/') {
                while (*p && *p != '\n') p++;
                continue;
            }
            
            // Read word
            int i = 0;
            while (*p && i < 31 && (isAlpha(*p) || isDigit(*p) || *p == '_')) {
                word[i++] = (*p >= 'a' && *p <= 'z') ? *p - 32 : *p;
                p++;
            }
            word[i] = '\0';
            
            if (i == 0) { p++; continue; }
            
            // Check for control flow
            if (strEqI(word, "JU") || strEqI(word, "JC") || strEqI(word, "JCN") ||
                strEqI(word, "JMP") || strEqI(word, "JUMP") ||
                strEqI(word, "CALL") || strEqI(word, "LOOP") ||
                strEqI(word, "BE") || strEqI(word, "BEC") || strEqI(word, "BEU") ||
                strEqI(word, "RET") || strEqI(word, "RETURN")) {
                return true;
            }
            
            while (*p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') p++;
        }
        return false;
    }
    
    // ============ Main Parser ============
    
    bool parse(const char* src, int len) {
        reset();
        source = src;
        length = len;
        
        nir::g_store.reset();
        
        // Check for control flow first
        if (hasControlFlow()) {
            setError("Cannot convert to Network IR: STL contains control flow (JU/JC/JCN/CALL) not representable in LADDER");
            return false;
        }
        
        action_start_idx = 0;
        
        char token[64];
        char operand[64];
        
        while (pos < length) {
            skipWhitespace();
            if (pos >= length) break;
            
            // Handle newline - potential network boundary
            if (peek() == '\n') {
                advance();
                // Check if we have pending actions -> finalize network
                if (pending_action_count > 0 && expr_stack_top > 0) {
                    finalizeNetwork();
                }
                continue;
            }
            
            // Skip comments
            if (peek() == '/' && peek(1) == '/') {
                skipLine();
                continue;
            }
            
            // Skip labels (name followed by colon)
            int save_pos = pos;
            if (readToken(token, sizeof(token))) {
                skipWhitespace();
                if (peek() == ':') {
                    advance(); // Skip label
                    continue;
                }
                // Not a label, restore position
                pos = save_pos;
            }
            
            // Read instruction
            if (!readToken(token, sizeof(token))) {
                advance();
                continue;
            }
            
            // Uppercase token
            for (int i = 0; token[i]; i++) {
                if (token[i] >= 'a' && token[i] <= 'z') token[i] -= 32;
            }
            
            // Read operand if any
            operand[0] = '\0';
            skipWhitespace();
            
            // Check for opening parenthesis (nesting)
            if (peek() == '(') {
                advance();
                operand[0] = '(';
                operand[1] = '\0';
            } else if (peek() != '\n' && peek() != '/' && peek() != '\0') {
                readToken(operand, sizeof(operand));
            }
            
            // Process instruction
            if (!processInstruction(token, operand)) {
                return false;
            }
        }
        
        // Finalize any remaining network
        if (pending_action_count > 0 || expr_stack_top > 0) {
            finalizeNetwork();
        }
        
        return !has_error;
    }
    
    bool processInstruction(const char* instr, const char* operand) {
        // ============ Load instructions (contacts) ============
        if (strEqI(instr, "A") || strEqI(instr, "AN") ||
            strEqI(instr, "O") || strEqI(instr, "ON") ||
            strEqI(instr, "X") || strEqI(instr, "XN")) {
            
            bool inverted = (instr[1] == 'N' || instr[1] == 'n');
            bool is_or = (instr[0] == 'O' || instr[0] == 'o');
            bool is_xor = (instr[0] == 'X' || instr[0] == 'x');
            
            // Check for nesting start: O( or A(
            if (operand[0] == '(') {
                if (is_or) {
                    // Start OR block - save current expression
                    if (expr_stack_top > 0) {
                        or_branches[or_branch_count++] = popExpr();
                    }
                    in_or_block = true;
                }
                // A( starts AND nesting - we'll combine on )
                return true;
            }
            
            if (operand[0] == '\0') {
                setError("Missing operand for load instruction");
                return false;
            }
            
            // Create leaf expression
            uint32_t sym_idx = nir::g_store.addSymbol(operand);
            if (sym_idx == nir::NIR_NONE) return false;
            
            uint32_t leaf = nir::g_store.addLeaf(sym_idx, inverted);
            if (leaf == nir::NIR_NONE) return false;
            
            if (is_or && expr_stack_top > 0) {
                // OR with previous - collect as branch
                or_branches[or_branch_count++] = popExpr();
                or_branches[or_branch_count++] = leaf;
                
                // Create OR expression
                uint32_t or_expr = nir::g_store.addOr(or_branches, or_branch_count);
                or_branch_count = 0;
                pushExpr(or_expr);
            } else if (is_xor && expr_stack_top > 0) {
                // XOR - for now treat similar to OR
                uint32_t prev = popExpr();
                uint32_t children[2] = { prev, leaf };
                // TODO: Add proper XOR support
                uint32_t xor_expr = nir::g_store.addOr(children, 2);
                pushExpr(xor_expr);
            } else if (expr_stack_top > 0 && !in_or_block) {
                // AND with previous (series)
                pushExpr(leaf);
                combineAnd();
            } else {
                pushExpr(leaf);
            }
            
            return true;
        }
        
        // ============ Closing parenthesis ============
        if (strEqI(instr, ")") || operand[0] == ')') {
            if (in_or_block && or_branch_count > 0) {
                // Close OR block
                if (expr_stack_top > 0) {
                    or_branches[or_branch_count++] = popExpr();
                }
                uint32_t or_expr = nir::g_store.addOr(or_branches, or_branch_count);
                or_branch_count = 0;
                in_or_block = false;
                pushExpr(or_expr);
            }
            return true;
        }
        
        // ============ NOT instruction ============
        if (strEqI(instr, "NOT")) {
            if (expr_stack_top > 0) {
                uint32_t top = popExpr();
                uint32_t not_expr = nir::g_store.addNot(top);
                pushExpr(not_expr);
            }
            return true;
        }
        
        // ============ Edge detection ============
        if (strEqI(instr, "FP") || strEqI(instr, "FN")) {
            bool is_falling = (instr[1] == 'N' || instr[1] == 'n');
            nir::CallOp op = is_falling ? nir::CALL_FN : nir::CALL_FP;
            
            // FP/FN uses operand as edge state memory
            uint32_t state_sym = nir::g_store.addSymbol(operand, nir::SYM_EDGE);
            
            if (expr_stack_top > 0) {
                uint32_t in_expr = popExpr();
                uint32_t call_expr = nir::g_store.addCallBool(op, in_expr, state_sym);
                pushExpr(call_expr);
            }
            return true;
        }
        
        // ============ Timer instructions ============
        if (strEqI(instr, "TON") || strEqI(instr, "TOF") || strEqI(instr, "TP")) {
            nir::CallOp op = strEqI(instr, "TON") ? nir::CALL_TON :
                             strEqI(instr, "TOF") ? nir::CALL_TOF : nir::CALL_TP;
            
            // Parse: TON T0, T#50ms or TON T0, #50
            char instance[32];
            uint32_t preset_ms = 0;
            
            // operand contains instance name (T0)
            int i = 0;
            while (operand[i] && operand[i] != ',' && i < 31) {
                instance[i] = operand[i];
                i++;
            }
            instance[i] = '\0';
            
            // Skip comma and whitespace, read preset
            if (operand[i] == ',') {
                i++;
                while (operand[i] == ' ') i++;
                // Parse preset (skip T# prefix if present)
                if (operand[i] == 'T' || operand[i] == 't') i++;
                if (operand[i] == '#') i++;
                preset_ms = 0;
                while (operand[i] >= '0' && operand[i] <= '9') {
                    preset_ms = preset_ms * 10 + (operand[i] - '0');
                    i++;
                }
                // Check for ms/s suffix
                if ((operand[i] == 'm' || operand[i] == 'M') &&
                    (operand[i+1] == 's' || operand[i+1] == 'S')) {
                    // Already in ms
                } else if (operand[i] == 's' || operand[i] == 'S') {
                    preset_ms *= 1000; // Convert to ms
                }
            }
            
            uint32_t inst_sym = nir::g_store.addSymbol(instance, nir::SYM_TIMER);
            
            if (expr_stack_top > 0) {
                uint32_t in_expr = popExpr();
                uint32_t call_expr = nir::g_store.addCallBool(op, in_expr, inst_sym, preset_ms);
                pushExpr(call_expr);
            }
            return true;
        }
        
        // ============ Counter instructions ============
        if (strEqI(instr, "CTU") || strEqI(instr, "CTD")) {
            nir::CallOp op = strEqI(instr, "CTU") ? nir::CALL_CTU : nir::CALL_CTD;
            
            // Parse: CTU C0, #10, X0.1 (instance, PV, reset/load)
            char instance[32];
            uint32_t pv = 0;
            char aux_operand[32];
            aux_operand[0] = '\0';
            
            int i = 0;
            // Instance
            while (operand[i] && operand[i] != ',' && i < 31) {
                instance[i] = operand[i];
                i++;
            }
            instance[i] = '\0';
            
            // PV
            if (operand[i] == ',') {
                i++;
                while (operand[i] == ' ') i++;
                if (operand[i] == '#') i++;
                while (operand[i] >= '0' && operand[i] <= '9') {
                    pv = pv * 10 + (operand[i] - '0');
                    i++;
                }
            }
            
            // Aux input (reset for CTU, load for CTD)
            if (operand[i] == ',') {
                i++;
                while (operand[i] == ' ') i++;
                int j = 0;
                while (operand[i] && j < 31) {
                    aux_operand[j++] = operand[i++];
                }
                aux_operand[j] = '\0';
            }
            
            uint32_t inst_sym = nir::g_store.addSymbol(instance, nir::SYM_COUNTER);
            uint32_t aux_expr = nir::NIR_NONE;
            
            if (aux_operand[0]) {
                uint32_t aux_sym = nir::g_store.addSymbol(aux_operand);
                aux_expr = nir::g_store.addLeaf(aux_sym);
            }
            
            if (expr_stack_top > 0) {
                uint32_t in_expr = popExpr();
                uint32_t call_expr = nir::g_store.addCallBool(op, in_expr, inst_sym, pv, aux_expr);
                pushExpr(call_expr);
            }
            return true;
        }
        
        // ============ Output instructions (coils) ============
        if (strEqI(instr, "=") || strEqI(instr, "S") || strEqI(instr, "R")) {
            nir::ActionKind kind = strEqI(instr, "=") ? nir::ACT_ASSIGN :
                                   strEqI(instr, "S") ? nir::ACT_SET : nir::ACT_RESET;
            
            uint32_t target_sym = nir::g_store.addSymbol(operand);
            uint32_t action_idx = nir::g_store.addAction(kind, target_sym);
            
            if (action_idx != nir::NIR_NONE) {
                if (pending_action_count == 0) {
                    action_start_idx = action_idx;
                }
                pending_actions[pending_action_count++] = action_idx;
            }
            return true;
        }
        
        // ============ TAP (VovkPLCRuntime extension) ============
        if (strEqI(instr, "TAP")) {
            // TAP - passthrough RLO to next network
            uint32_t action_idx = nir::g_store.addAction(nir::ACT_TAP, nir::NIR_NONE);
            
            if (action_idx != nir::NIR_NONE) {
                if (pending_action_count == 0) {
                    action_start_idx = action_idx;
                }
                pending_actions[pending_action_count++] = action_idx;
            }
            return true;
        }
        
        // ============ RLO manipulation ============
        if (strEqI(instr, "SET") || strEqI(instr, "CLR")) {
            // SET = force RLO to 1, CLR = force RLO to 0
            // For IR: create a constant true/false leaf
            // Not commonly used in LADDER, skip for now
            return true;
        }
        
        // Unknown instruction - skip
        return true;
    }
    
    void finalizeNetwork() {
        uint32_t condition = (expr_stack_top > 0) ? popExpr() : nir::NIR_NONE;
        
        if (pending_action_count > 0 || condition != nir::NIR_NONE) {
            nir::g_store.addNetwork(condition, action_start_idx, pending_action_count);
        }
        
        // Reset for next network
        pending_action_count = 0;
        action_start_idx = nir::g_store.action_count;
        expr_stack_top = 0;
        or_branch_count = 0;
        in_or_block = false;
    }
};

// ============================================================================
// Network IR to STL Emitter
// ============================================================================

class IRToSTLEmitter {
public:
    static const int MAX_OUTPUT = 32768;
    char output[MAX_OUTPUT];
    int output_len;
    
    char error_msg[256];
    bool has_error;
    
    int indent_level;  // Track nesting depth for indentation
    int skip_label_counter;  // Counter for unique skip labels
    
    IRToSTLEmitter() { reset(); }
    
    void reset() {
        output_len = 0;
        output[0] = '\0';
        error_msg[0] = '\0';
        has_error = false;
        indent_level = 0;
        skip_label_counter = 0;
    }
    
    void emit(const char* s) {
        while (*s && output_len < MAX_OUTPUT - 1) {
            output[output_len++] = *s++;
        }
        output[output_len] = '\0';
    }
    
    void emitLine(const char* s) {
        emit(s);
        emit("\n");
    }
    
    void emitIndent() {
        for (int i = 0; i < indent_level * 4; i++) {
            if (output_len < MAX_OUTPUT - 1) {
                output[output_len++] = ' ';
            }
        }
        output[output_len] = '\0';
    }
    
    void emitIndentedLine(const char* s) {
        emitIndent();
        emit(s);
        emit("\n");
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
            output[output_len++] = buf[--i];
        }
        output[output_len] = '\0';
    }
    
    void setError(const char* msg) {
        if (has_error) return;
        has_error = true;
        int i = 0;
        while (msg[i] && i < 255) {
            error_msg[i] = msg[i];
            i++;
        }
        error_msg[i] = '\0';
    }
    
    // ============ Expression Emission ============
    
    void emitExpr(uint32_t expr_idx, bool first_in_and = true) {
        if (expr_idx == nir::NIR_NONE) return;
        
        nir::Expr& expr = nir::g_store.exprs[expr_idx];
        
        switch (expr.kind) {
            case nir::EXPR_LEAF:
                emitLeaf(expr, first_in_and);
                break;
                
            case nir::EXPR_NOT:
                emitIndentedLine("NOT");
                emitExpr(expr.a, true);
                break;
                
            case nir::EXPR_AND:
                for (uint16_t i = 0; i < expr.child_cnt; i++) {
                    uint32_t child_idx = nir::g_store.children[expr.child_ofs + i].expr_index;
                    nir::Expr& child = nir::g_store.exprs[child_idx];
                    
                    // If child is an OR or COMPOUND expression, wrap it with A( ... )
                    if (child.kind == nir::EXPR_OR || child.kind == nir::EXPR_COMPOUND) {
                        emitIndentedLine("A(");
                        indent_level++;
                        emitExpr(child_idx, true);
                        indent_level--;
                        emitIndentedLine(")");
                    } else {
                        emitExpr(child_idx, i == 0);
                    }
                }
                break;
                
            case nir::EXPR_OR:
                // First branch - emit directly (no wrapper needed for first branch)
                // The first branch establishes the RLO, subsequent branches OR with it
                if (expr.child_cnt > 0) {
                    uint32_t first = nir::g_store.children[expr.child_ofs].expr_index;
                    emitExpr(first, true);  // First branch emits its own A() if needed
                }
                // Remaining branches with O ... (simple) or O( ... ) (complex)
                for (uint16_t i = 1; i < expr.child_cnt; i++) {
                    uint32_t child_idx = nir::g_store.children[expr.child_ofs + i].expr_index;
                    nir::Expr& child = nir::g_store.exprs[child_idx];
                    
                    // Simple leaf can use plain O, complex needs O( ... )
                    if (child.kind == nir::EXPR_LEAF) {
                        bool inverted = (child.flags & nir::EXPR_FLAG_INVERTED) != 0;
                        emitIndent();
                        emit(inverted ? "ON " : "O ");
                        emit(nir::g_store.symbols[child.a].name);
                        emit("\n");
                    } else {
                        emitIndentedLine("O(");
                        indent_level++;
                        emitExpr(child_idx, true);
                        indent_level--;
                        emitIndentedLine(")");
                    }
                }
                break;
                
            case nir::EXPR_CALL_BOOL:
                emitCallBool(expr);
                break;
                
            case nir::EXPR_COMPOUND:
                emitCompoundBranch(expr);
                break;
                
            case nir::EXPR_MATH:
                emitMathExpr(expr);
                break;
                
            case nir::EXPR_COMPARE:
                emitCompareExpr(expr);
                break;
                
            case nir::EXPR_LOAD:
                emitLoadExpr(expr);
                break;
                
            case nir::EXPR_CONST:
                emitConstExpr(expr);
                break;
        }
    }
    
    // Emit a compound branch (condition + inline actions + optional TAP)
    void emitCompoundBranch(nir::Expr& expr) {
        uint32_t comp_idx = expr.a;
        nir::CompoundBranch& comp = nir::g_store.compounds[comp_idx];
        
        // Emit the condition expression
        emitExpr(comp.condition_expr, true);
        
        // Emit inline actions
        for (int i = 0; i < comp.action_count; i++) {
            nir::CompoundBranch::InlineAction& act = comp.actions[i];
            const char* instr = act.kind == nir::ACT_ASSIGN ? "=" :
                                act.kind == nir::ACT_SET ? "S" : "R";
            emitIndent();
            emit(instr);
            emit(" ");
            emit(nir::g_store.symbols[act.target_sym].name);
            emit("\n");
        }
        
        // Emit TAP if present
        if (comp.has_tap) {
            emitIndentedLine("TAP");
        }
    }
    
    void emitLeaf(nir::Expr& expr, bool first_in_and) {
        bool inverted = (expr.flags & nir::EXPR_FLAG_INVERTED) != 0;
        
        // Determine instruction
        const char* instr = first_in_and ? (inverted ? "AN" : "A") : (inverted ? "AN" : "A");
        
        emitIndent();
        emit(instr);
        emit(" ");
        emit(nir::g_store.symbols[expr.a].name);
        emit("\n");
    }
    
    void emitCallBool(nir::Expr& expr) {
        uint32_t call_idx = expr.a;
        nir::CallBool& call = nir::g_store.calls[call_idx];
        
        // First emit the input expression
        emitExpr(call.in_expr, true);
        
        // Then emit the call instruction
        switch (call.op) {
            case nir::CALL_FP:
                emitIndent();
                emit("FP ");
                emit(nir::g_store.symbols[call.instance_sym].name);
                emit("\n");
                break;
                
            case nir::CALL_FN:
                emitIndent();
                emit("FN ");
                emit(nir::g_store.symbols[call.instance_sym].name);
                emit("\n");
                break;
                
            case nir::CALL_FX:
                emitIndent();
                emit("FX ");
                emit(nir::g_store.symbols[call.instance_sym].name);
                emit("\n");
                break;
                
            case nir::CALL_TON:
            case nir::CALL_TOF:
            case nir::CALL_TP: {
                const char* op = call.op == nir::CALL_TON ? "TON" :
                                 call.op == nir::CALL_TOF ? "TOF" : "TP";
                emitIndent();
                emit(op);
                emit(" ");
                emit(nir::g_store.symbols[call.instance_sym].name);
                emit(", ");
                if (call.preset_kind == nir::PRESET_LITERAL) {
                    emit(&nir::g_store.preset_pool[call.preset_str_ofs]);
                } else {
                    emit("T#");
                    emitInt(call.preset_u32);
                    emit("ms");
                }
                emit("\n");
                break;
            }
                
            case nir::CALL_CTU:
            case nir::CALL_CTD: {
                const char* op = call.op == nir::CALL_CTU ? "CTU" : "CTD";
                emitIndent();
                emit(op);
                emit(" ");
                emit(nir::g_store.symbols[call.instance_sym].name);
                emit(", #");
                emitInt(call.preset_u32);
                if (call.aux_expr != nir::NIR_NONE) {
                    emit(", ");
                    // Get the aux symbol name
                    nir::Expr& aux = nir::g_store.exprs[call.aux_expr];
                    if (aux.kind == nir::EXPR_LEAF) {
                        emit(nir::g_store.symbols[aux.a].name);
                    }
                }
                emit("\n");
                break;
            }
            
            default:
                break;
        }
    }
    
    // ============ Data Type Helpers ============
    
    const char* dataTypeToPrefix(nir::DataType dt) {
        switch (dt) {
            case nir::DT_U8:  return "u8";
            case nir::DT_U16: return "u16";
            case nir::DT_U32: return "u32";
            case nir::DT_U64: return "u64";
            case nir::DT_I8:  return "i8";
            case nir::DT_I16: return "i16";
            case nir::DT_I32: return "i32";
            case nir::DT_I64: return "i64";
            case nir::DT_F32: return "f32";
            case nir::DT_F64: return "f64";
            default:         return "i16";
        }
    }
    
    char dataTypeToSuffix(nir::DataType dt) {
        switch (dt) {
            case nir::DT_U8:
            case nir::DT_I8:
            case nir::DT_U16:
            case nir::DT_I16: return 'I';  // INT (Word operations)
            case nir::DT_U32:
            case nir::DT_I32: return 'D';  // DINT (DWord operations)
            case nir::DT_F32: return 'R';  // REAL
            case nir::DT_U64:
            case nir::DT_I64:
            case nir::DT_F64:
            default:          return '\0'; // Use extended form for 64-bit
        }
    }
    
    // Strip type prefix from address (MW10 -> M10, MD20 -> M20)
    void stripTypePrefix(const char* addr, char* out) {
        if (!addr || !addr[0]) { out[0] = '\0'; return; }
        
        char c0 = addr[0];
        char c1 = addr[1];
        if (c0 >= 'a' && c0 <= 'z') c0 -= 32;
        if (c1 >= 'a' && c1 <= 'z') c1 -= 32;
        
        int start = 0;
        // Check two-letter prefix
        if (c1 == 'B' || c1 == 'W' || c1 == 'D' || c1 == 'L' || c1 == 'R') {
            out[0] = addr[0]; // Keep the first letter (M, I, Q, etc.)
            start = 2;
            int j = 1;
            while (addr[start]) {
                out[j++] = addr[start++];
            }
            out[j] = '\0';
        } else {
            // No type prefix to strip, copy as-is
            int i = 0;
            while (addr[i]) {
                out[i] = addr[i];
                i++;
            }
            out[i] = '\0';
        }
    }
    
    // ============ Math Expression Emission ============
    
    // Emit a single operand (for use within math operations)
    // Returns: the operand string or empty if complex expression
    void emitOperandToSTL(uint32_t expr_idx, char* operand_out, int max_len) {
        operand_out[0] = '\0';
        if (expr_idx == nir::NIR_NONE) return;
        
        nir::Expr& expr = nir::g_store.exprs[expr_idx];
        
        if (expr.kind == nir::EXPR_LOAD) {
            // Memory address - the symbol already contains the full address (MW0, MD4, etc.)
            const char* sym_name = nir::g_store.symbols[expr.a].name;
            
            // Just copy the symbol name as-is (it already has the prefix)
            int i = 0;
            while (sym_name[i] && i < max_len - 1) {
                operand_out[i] = sym_name[i];
                i++;
            }
            operand_out[i] = '\0';
        }
        else if (expr.kind == nir::EXPR_CONST) {
            // Constant value - prefix with # for STL format
            uint32_t const_idx = expr.a;
            nir::ConstExpr& c = nir::g_store.consts[const_idx];
            
            int i = 0;
            operand_out[i++] = '#';  // STL constant prefix
            
            if (c.data_type == nir::DT_F32) {
                // Format float
                float v = c.f32_val;
                if (v < 0) { operand_out[i++] = '-'; v = -v; }
                int intPart = (int)v;
                float fracPart = v - intPart;
                
                // Integer part
                char ibuf[16];
                int ilen = 0;
                if (intPart == 0) ibuf[ilen++] = '0';
                else while (intPart > 0 && ilen < 15) {
                    ibuf[ilen++] = '0' + (intPart % 10);
                    intPart /= 10;
                }
                while (ilen > 0) operand_out[i++] = ibuf[--ilen];
                
                operand_out[i++] = '.';
                
                // Fraction part (6 digits)
                for (int d = 0; d < 6 && i < max_len - 1; d++) {
                    fracPart *= 10;
                    int digit = (int)fracPart;
                    operand_out[i++] = '0' + digit;
                    fracPart -= digit;
                }
                operand_out[i] = '\0';
            } else {
                // Format integer
                int64_t val = c.i64_val;
                if (val < 0) { operand_out[i++] = '-'; val = -val; }
                
                char ibuf[24];
                int ilen = 0;
                if (val == 0) ibuf[ilen++] = '0';
                else while (val > 0 && ilen < 23) {
                    ibuf[ilen++] = '0' + (val % 10);
                    val /= 10;
                }
                while (ilen > 0 && i < max_len - 1) operand_out[i++] = ibuf[--ilen];
                operand_out[i] = '\0';
            }
        }
    }
    
    void emitMathExpr(nir::Expr& expr) {
        uint32_t math_idx = expr.a;
        nir::MathExpr& math = nir::g_store.maths[math_idx];
        
        nir::DataType dt = (nir::DataType)math.data_type;
        char suffix = dataTypeToSuffix(dt);
        
        // Get operand strings
        char op1[64], op2[64];
        emitOperandToSTL(math.left_expr, op1, sizeof(op1));
        emitOperandToSTL(math.right_expr, op2, sizeof(op2));
        
        bool is_unary = (math.op == nir::MATH_NEG || math.op == nir::MATH_ABS);
        
        // Emit L first_operand
        emitIndent();
        emit("L ");
        emit(op1);
        emit("\n");
        
        // Emit operation with second operand (if binary)
        emitIndent();
        if (suffix != '\0' && !is_unary) {
            // Siemens short form: +I operand, -D operand, etc.
            switch (math.op) {
                case nir::MATH_ADD: emit("+"); break;
                case nir::MATH_SUB: emit("-"); break;
                case nir::MATH_MUL: emit("*"); break;
                case nir::MATH_DIV: emit("/"); break;
                default: break;
            }
            char buf[2] = { suffix, '\0' };
            emit(buf);
            emit(" ");
            emit(op2);
        } else {
            // Extended form or unary
            const char* prefix = dataTypeToPrefix(dt);
            emit(prefix);
            emit(".");
            switch (math.op) {
                case nir::MATH_ADD: emit("add "); emit(op2); break;
                case nir::MATH_SUB: emit("sub "); emit(op2); break;
                case nir::MATH_MUL: emit("mul "); emit(op2); break;
                case nir::MATH_DIV: emit("div "); emit(op2); break;
                case nir::MATH_MOD: emit("mod "); emit(op2); break;
                case nir::MATH_NEG: emit("neg"); break;
                case nir::MATH_ABS: emit("abs"); break;
                default: break;
            }
        }
        emit("\n");
    }
    
    void emit(const char* s, int len) {
        for (int i = 0; i < len && *s && output_len < MAX_OUTPUT - 1; i++) {
            output[output_len++] = *s++;
        }
        output[output_len] = '\0';
    }
    
    // ============ Compare Expression Emission ============
    
    void emitCompareExpr(nir::Expr& expr) {
        uint32_t cmp_idx = expr.a;
        nir::CompareExpr& cmp = nir::g_store.compares[cmp_idx];
        
        nir::DataType dt = (nir::DataType)cmp.data_type;
        char suffix = dataTypeToSuffix(dt);
        
        // Get operand strings
        char op1[64], op2[64];
        emitOperandToSTL(cmp.left_expr, op1, sizeof(op1));
        emitOperandToSTL(cmp.right_expr, op2, sizeof(op2));
        
        // Emit L first_operand
        emitIndent();
        emit("L ");
        emit(op1);
        emit("\n");
        
        // Emit comparison with second operand
        emitIndent();
        if (suffix != '\0') {
            // Siemens short form: ==I op, <>D op, etc.
            switch (cmp.op) {
                case nir::CMP_EQ:  emit("=="); break;
                case nir::CMP_NEQ: emit("<>"); break;
                case nir::CMP_GT:  emit(">"); break;
                case nir::CMP_LT:  emit("<"); break;
                case nir::CMP_GTE: emit(">="); break;
                case nir::CMP_LTE: emit("<="); break;
                default: break;
            }
            char buf[2] = { suffix, '\0' };
            emit(buf);
            emit(" ");
            emit(op2);
        } else {
            // Extended form
            const char* prefix = dataTypeToPrefix(dt);
            emit(prefix);
            emit(".");
            switch (cmp.op) {
                case nir::CMP_EQ:  emit("cmp_eq "); break;
                case nir::CMP_NEQ: emit("cmp_neq "); break;
                case nir::CMP_GT:  emit("cmp_gt "); break;
                case nir::CMP_LT:  emit("cmp_lt "); break;
                case nir::CMP_GTE: emit("cmp_gte "); break;
                case nir::CMP_LTE: emit("cmp_lte "); break;
                default: break;
            }
            emit(op2);
        }
        emit("\n");
    }
    
    // ============ Load Expression Emission ============
    
    void emitLoadExpr(nir::Expr& expr) {
        // The symbol already contains the full address (MW0, MD4, etc.)
        const char* sym_name = nir::g_store.symbols[expr.a].name;
        
        emitIndent();
        emit("L ");
        emit(sym_name);
        emit("\n");
    }
    
    // ============ Const Expression Emission ============
    
    void emitConstExpr(nir::Expr& expr) {
        uint32_t const_idx = expr.a;
        nir::ConstExpr& c = nir::g_store.consts[const_idx];
        
        emitIndent();
        emit("L #");  // STL constant prefix
        
        if (c.data_type == nir::DT_F32) {
            emitFloat(c.f32_val);
        } else if (c.data_type == nir::DT_F64) {
            emitDouble(c.f64_val);
        } else {
            emitInt64(c.i64_val);
        }
        emit("\n");
    }
    
    void emitInt64(int64_t val) {
        char buf[24];
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
            output[output_len++] = buf[--i];
        }
        output[output_len] = '\0';
    }
    
    void emitFloat(float val) {
        // Simple float to string
        if (val < 0) { emit("-"); val = -val; }
        int whole = (int)val;
        emitInt(whole);
        emit(".");
        val -= whole;
        val *= 1000000; // 6 decimal places
        int frac = (int)val;
        char buf[8];
        for (int i = 5; i >= 0; i--) {
            buf[i] = '0' + (frac % 10);
            frac /= 10;
        }
        buf[6] = '\0';
        // Trim trailing zeros
        for (int i = 5; i > 0; i--) {
            if (buf[i] == '0') buf[i] = '\0';
            else break;
        }
        emit(buf);
    }
    
    void emitDouble(double val) {
        emitFloat((float)val); // Simplify for now
    }
    
    // ============ Action Emission ============
    
    // Helper: Check if a MOVE action is an optimizable INC/DEC operation
    // Returns: 1 for increment, -1 for decrement, 0 for not optimizable
    int isOptimizableIncDec(nir::Action& action) {
        if (action.kind != nir::ACT_MOVE) return 0;
        if (action.value_expr == nir::NIR_NONE) return 0;
        
        nir::Expr& value_expr = nir::g_store.exprs[action.value_expr];
        if (value_expr.kind != nir::EXPR_MATH) return 0;
        
        nir::MathExpr& math = nir::g_store.maths[value_expr.a];
        
        // Only ADD and SUB can be optimized
        if (math.op != nir::MATH_ADD && math.op != nir::MATH_SUB) return 0;
        
        // Check if right operand is constant 1
        if (math.right_expr == nir::NIR_NONE) return 0;
        nir::Expr& right_expr = nir::g_store.exprs[math.right_expr];
        if (right_expr.kind != nir::EXPR_CONST) return 0;
        
        nir::ConstExpr& const_val = nir::g_store.consts[right_expr.a];
        // Check if the constant is 1 (for any supported type)
        bool is_one = false;
        switch (const_val.data_type) {
            case nir::DT_I8:  case nir::DT_U8:
            case nir::DT_I16: case nir::DT_U16:
            case nir::DT_I32: case nir::DT_U32:
                is_one = (const_val.i32_val == 1);
                break;
            case nir::DT_I64: case nir::DT_U64:
                is_one = (const_val.i64_val == 1);
                break;
            case nir::DT_F32:
                is_one = (const_val.f32_val == 1.0f);
                break;
            case nir::DT_F64:
                is_one = (const_val.f64_val == 1.0);
                break;
            default: break;
        }
        if (!is_one) return 0;
        
        // Check if left operand is a LOAD from the same target symbol
        if (math.left_expr == nir::NIR_NONE) return 0;
        nir::Expr& left_expr = nir::g_store.exprs[math.left_expr];
        if (left_expr.kind != nir::EXPR_LOAD) return 0;
        
        // Compare symbol indices - must be the same target
        if (left_expr.a != action.target_sym) return 0;
        
        // All conditions met - return increment or decrement
        return (math.op == nir::MATH_ADD) ? 1 : -1;
    }
    
    // Build destination address with type prefix for a memory symbol
    void buildDestAddress(char* dest, int max_len, uint32_t sym_idx, nir::DataType dt) {
        const char* sym_name = nir::g_store.symbols[sym_idx].name;
        int i = 0;
        if (sym_name[0] == 'M' || sym_name[0] == 'm' || 
            (sym_name[0] >= '0' && sym_name[0] <= '9')) {
            // Memory address - add type prefix
            dest[i++] = 'M';
            switch (dt) {
                case nir::DT_U8:  dest[i++] = 'B'; break;
                case nir::DT_I16: case nir::DT_U16: dest[i++] = 'W'; break;
                case nir::DT_I32: case nir::DT_U32: dest[i++] = 'D'; break;
                case nir::DT_I64: case nir::DT_U64: dest[i++] = 'L'; break;
                case nir::DT_F32: case nir::DT_F64: dest[i++] = 'R'; break;
                default: dest[i++] = 'W'; break;
            }
            // Copy the number part
            const char* num = sym_name;
            if (sym_name[0] == 'M' || sym_name[0] == 'm') num++;
            while (*num && i < max_len - 1) {
                dest[i++] = *num++;
            }
        } else {
            // Other address, copy as-is
            const char* p = sym_name;
            while (*p && i < max_len - 1) {
                dest[i++] = *p++;
            }
        }
        dest[i] = '\0';
    }
    
    void emitAction(nir::Action& action) {
        if (action.kind == nir::ACT_TAP) {
            // TAP - passthrough RLO to next network (no operand)
            emitIndentedLine("TAP");
            return;
        }
        
        // BR stack actions for parallel output branches
        if (action.kind == nir::ACT_BR_SAVE) {
            emitIndentedLine("SAVE");
            return;
        }
        if (action.kind == nir::ACT_BR_READ) {
            emitIndentedLine("L BR");
            return;
        }
        if (action.kind == nir::ACT_BR_DROP) {
            emitIndentedLine("DROP BR");
            return;
        }
        if (action.kind == nir::ACT_BR_CLR) {
            emitIndentedLine("CLR BR");
            return;
        }
        
        if (action.kind == nir::ACT_MOVE) {
            nir::DataType dt = (nir::DataType)action.data_type;
            
            // Check for optimizable INC/DEC operations
            int inc_dec = isOptimizableIncDec(action);
            if (inc_dec != 0) {
                // Emit optimized INCI/DECI instruction instead of L+op+T sequence
                char dest[64];
                buildDestAddress(dest, sizeof(dest), action.target_sym, dt);
                
                emitIndent();
                emit(inc_dec > 0 ? "INC" : "DEC");
                // Add type suffix: I (word), D (dword), B (byte), R (real)
                switch (dt) {
                    case nir::DT_U8:  emit("B"); break;
                    case nir::DT_I16: case nir::DT_U16: emit("I"); break;  // I for word (Siemens style)
                    case nir::DT_I32: case nir::DT_U32: emit("D"); break;  // D for double-word
                    case nir::DT_I64: case nir::DT_U64: emit("D"); break;  // Use D for 64-bit too
                    case nir::DT_F32: case nir::DT_F64: emit("R"); break;  // R for real
                    default: emit("I"); break;
                }
                emit(" ");
                emit(dest);
                emit("\n");
                return;
            }
            
            // MOVE - emit value expression, then transfer to target
            if (action.value_expr != nir::NIR_NONE) {
                emitExpr(action.value_expr, true);
            }
            
            // Build destination address with type prefix (MW4, MD8, etc.)
            char dest[64];
            buildDestAddress(dest, sizeof(dest), action.target_sym, dt);
            
            // Emit T destination (Transfer)
            emitIndent();
            emit("T ");
            emit(dest);
            emit("\n");
            return;
        }
        
        const char* instr = action.kind == nir::ACT_ASSIGN ? "=" :
                            action.kind == nir::ACT_SET ? "S" : "R";
        emitIndent();
        emit(instr);
        emit(" ");
        emit(nir::g_store.symbols[action.target_sym].name);
        emit("\n");
    }
    
    // ============ Network Emission ============
    
    // Helper: check if any action in the network is a MOVE (math/transfer)
    bool hasMoveAction(nir::Network& net) {
        for (uint16_t i = 0; i < net.action_cnt; i++) {
            if (nir::g_store.actions[net.action_ofs + i].kind == nir::ACT_MOVE) {
                return true;
            }
        }
        return false;
    }
    
    // Helper: check if we have only coil actions (no MOVE actions)
    // Coil-only networks can be emitted without JCN since coils consume RLO
    bool hasOnlyCoilActions(nir::Network& net) {
        for (uint16_t i = 0; i < net.action_cnt; i++) {
            nir::Action& action = nir::g_store.actions[net.action_ofs + i];
            if (action.kind != nir::ACT_ASSIGN && action.kind != nir::ACT_SET && 
                action.kind != nir::ACT_RESET && action.kind != nir::ACT_TAP) {
                return false;  // Has non-coil action
            }
        }
        return true;
    }
    
    void emitNetwork(nir::Network& net) {
        // Emit condition expression
        if (net.condition_expr != nir::NIR_NONE) {
            emitExpr(net.condition_expr, true);
        }
        
        // Determine if we need a JCN skip block
        // JCN is needed when:
        // - We have a condition AND
        // - We have any MOVE actions (math/transfer) since they don't use RLO
        // 
        // Coil-only networks can use native RLO consumption without JCN
        bool needsConditionalSkip = (net.condition_expr != nir::NIR_NONE) && hasMoveAction(net);
        int skipLabel = 0;
        
        if (needsConditionalSkip) {
            skipLabel = skip_label_counter++;
            emitIndent();
            emit("JCN skip_");
            emitInt(skipLabel);
            emit("\n");
        }
        
        // Emit actions
        for (uint16_t i = 0; i < net.action_cnt; i++) {
            emitAction(nir::g_store.actions[net.action_ofs + i]);
        }
        
        // Emit skip label if we added JCN
        if (needsConditionalSkip) {
            emit("skip_");
            emitInt(skipLabel);
            emit(":\n");
        }
        
        emit("\n"); // Blank line between networks
    }
    
    // ============ Full Program Emission ============
    
    bool emitAll() {
        reset();
        
        emit("// Generated from Network IR by VovkPLCRuntime\n\n");
        
        for (uint32_t i = 0; i < nir::g_store.network_count; i++) {
            emitNetwork(nir::g_store.networks[i]);
        }
        
        return !has_error;
    }
};

// ============================================================================
// Global Instances and WASM Exports
// ============================================================================

static LadderGraphCompiler ladderGraphCompiler;
static STLToIRParser stlToIRParser;
static IRToSTLEmitter irToSTLEmitter;
static char ladder_input_buffer[32768];
static int ladder_input_length = 0;

extern "C" {

// ============ Ladder Graph to STL ============

WASM_EXPORT void ladder_graph_load_from_stream() {
    streamRead(ladder_input_buffer, ladder_input_length, sizeof(ladder_input_buffer));
}

WASM_EXPORT bool ladder_graph_compile() {
    // Parse and compile Ladder Graph JSON directly to STL
    return ladderGraphCompiler.parse(ladder_input_buffer, ladder_input_length);
}

WASM_EXPORT const char* ladder_graph_get_output() {
    return ladderGraphCompiler.output;
}

WASM_EXPORT int ladder_graph_get_output_length() {
    return ladderGraphCompiler.output_len;
}

WASM_EXPORT void ladder_graph_output_to_stream() {
    for (int i = 0; i < ladderGraphCompiler.output_len; i++) {
        streamOut(ladderGraphCompiler.output[i]);
    }
}

WASM_EXPORT bool ladder_graph_has_error() {
    return ladderGraphCompiler.has_error;
}

WASM_EXPORT const char* ladder_graph_get_error() {
    return ladderGraphCompiler.error_msg;
}

WASM_EXPORT void ladder_graph_error_to_stream() {
    const char* err = ladderGraphCompiler.error_msg;
    for (int i = 0; err[i] != '\0'; i++) {
        streamOut(err[i]);
    }
}

// ============ Full Pipeline: Ladder Graph -> STL -> PLCASM -> Bytecode ============

WASM_EXPORT bool ladder_graph_compile_full() {
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
    defaultCompiler.set_assembly_string((char*)stlCompiler.getOutput());
    return !defaultCompiler.compileAssembly(false, false);
}

// ============ STL to Network IR ============

WASM_EXPORT void stl_to_ir_load_from_stream() {
    streamRead(ladder_input_buffer, ladder_input_length, sizeof(ladder_input_buffer));
}

WASM_EXPORT bool stl_to_ir_parse() {
    return stlToIRParser.parse(ladder_input_buffer, ladder_input_length);
}

WASM_EXPORT bool stl_to_ir_has_error() {
    return stlToIRParser.has_error;
}

WASM_EXPORT const char* stl_to_ir_get_error() {
    return stlToIRParser.error_msg;
}

// ============ Network IR to STL ============

WASM_EXPORT bool ir_to_stl_emit() {
    return irToSTLEmitter.emitAll();
}

WASM_EXPORT const char* ir_to_stl_get_output() {
    return irToSTLEmitter.output;
}

WASM_EXPORT int ir_to_stl_get_output_length() {
    return irToSTLEmitter.output_len;
}

WASM_EXPORT void ir_to_stl_output_to_stream() {
    for (int i = 0; i < irToSTLEmitter.output_len; i++) {
        streamOut(irToSTLEmitter.output[i]);
    }
}

WASM_EXPORT bool ir_to_stl_has_error() {
    return irToSTLEmitter.has_error;
}

WASM_EXPORT const char* ir_to_stl_get_error() {
    return irToSTLEmitter.error_msg;
}

// ============ Round-trip: STL -> IR -> STL ============

WASM_EXPORT bool stl_roundtrip() {
    // Parse STL to IR
    if (!stlToIRParser.parse(ladder_input_buffer, ladder_input_length)) {
        return false;
    }
    // Emit IR to STL
    return irToSTLEmitter.emitAll();
}

// ============ Full Pipeline: STL -> IR -> STL -> PLCASM -> Bytecode ============

WASM_EXPORT bool stl_to_ir_compile_full() {
    // Step 1: Parse STL to Network IR
    if (!stlToIRParser.parse(ladder_input_buffer, ladder_input_length)) {
        return false;
    }
    
    // Step 2: Emit Network IR back to canonical STL
    if (!irToSTLEmitter.emitAll()) {
        return false;
    }
    
    // Step 3: Compile STL to PLCASM
    stlCompiler.setSource(irToSTLEmitter.output, irToSTLEmitter.output_len);
    if (!stlCompiler.compile()) {
        return false;
    }
    
    // Step 4: Compile PLCASM to bytecode
    defaultCompiler.set_assembly_string((char*)stlCompiler.getOutput());
    return !defaultCompiler.compileAssembly(false, false);
}

// ============ STL to Ladder (via IR) ============

WASM_EXPORT bool stl_to_ladder_convert() {
    // Parse STL into Network IR
    if (!stlToIRParser.parse(stl_source_buffer, stl_source_length)) {
        return false;
    }
    // The IR itself IS the "ladder" representation
    // Emit back to STL for now (visual ladder rendering is a JS concern)
    return irToSTLEmitter.emitAll();
}

WASM_EXPORT bool stl_to_ladder_has_error() {
    return stlToIRParser.has_error;
}

WASM_EXPORT const char* stl_to_ladder_get_error() {
    return stlToIRParser.error_msg;
}

WASM_EXPORT const char* stl_to_ladder_get_output() {
    return irToSTLEmitter.output;
}

WASM_EXPORT int stl_to_ladder_get_output_length() {
    return irToSTLEmitter.output_len;
}

WASM_EXPORT void stl_to_ladder_output_to_stream() {
    ir_to_stl_output_to_stream();
}

} // extern "C"

#endif // __WASM__
