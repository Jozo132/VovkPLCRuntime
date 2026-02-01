// ladder-json-parser.h - JSON Lexer and Graph Parsing Functions
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

// Note: strcpy is provided by wasm/jvmalloc.h via parent files

// ============================================================================
// JSON Parser Mixin - provides JSON lexer and parsing functions
// This is designed to be included via CRTP or as a mixin in LadderGraphCompiler
// ============================================================================

class LadderJsonParser {
protected:
    const char* source;
    int length;
    int pos;

    // Error handling - will be overridden by main compiler class
    char error_msg[256];
    bool has_error;

    // Graph data
    LadderGraphNode nodes[LADDER_MAX_NODES];
    int node_count;
    LadderConnection connections[LADDER_MAX_CONNECTIONS];
    int connection_count;

public:
    LadderJsonParser() : source(nullptr), length(0), pos(0), has_error(false), node_count(0), connection_count(0) {
        error_msg[0] = '\0';
    }

    virtual ~LadderJsonParser() = default;

    // Virtual error setter - can be overridden by derived classes
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

    // ============ JSON Parsing ============

    bool parseNode(LadderGraphNode& node) {
        node.clear();

        if (!expect('{')) {
            setError("Expected node object");
            return false;
        }

        while (peek() != '}' && peek() != '\0') {
            char key[32];
            if (!readString(key, sizeof(key))) return false;
            if (!expect(':')) { setError("Expected ':'"); return false; }

            if (ladderStrEqI(key, "id")) {
                if (!readString(node.id, sizeof(node.id))) return false;
            } else if (ladderStrEqI(key, "type")) {
                if (!readString(node.type, sizeof(node.type))) return false;
            } else if (ladderStrEqI(key, "address") || ladderStrEqI(key, "symbol")) {
                if (!readString(node.address, sizeof(node.address))) return false;
            } else if (ladderStrEqI(key, "x")) {
                if (!readInt(node.x)) return false;
            } else if (ladderStrEqI(key, "y")) {
                if (!readInt(node.y)) return false;
            } else if (ladderStrEqI(key, "inverted")) {
                if (!readBool(node.inverted)) return false;
            } else if (ladderStrEqI(key, "preset")) {
                skipWhitespace();
                if (peek() == '"') {
                    if (!readString(node.preset_str, sizeof(node.preset_str))) return false;
                } else {
                    if (!readU32(node.preset)) return false;
                }
            } else if (ladderStrEqI(key, "trigger")) {
                if (!readString(node.trigger, sizeof(node.trigger))) return false;
            } else if (ladderStrEqI(key, "dataType") || ladderStrEqI(key, "data_type")) {
                if (!readString(node.data_type, sizeof(node.data_type))) return false;
            } else if (ladderStrEqI(key, "in1") || ladderStrEqI(key, "input1") || ladderStrEqI(key, "in")) {
                if (!readString(node.in1, sizeof(node.in1))) return false;
            } else if (ladderStrEqI(key, "in2") || ladderStrEqI(key, "input2")) {
                if (!readString(node.in2, sizeof(node.in2))) return false;
            } else if (ladderStrEqI(key, "out") || ladderStrEqI(key, "output")) {
                if (!readString(node.out, sizeof(node.out))) return false;
            } else if (ladderStrEqI(key, "passThrough") || ladderStrEqI(key, "pass_through")) {
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

    bool parseConnection(LadderConnection& conn) {
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

            if (ladderStrEqI(key, "sources")) {
                has_sources = true;
                if (!expect('[')) { setError("Expected sources array"); return false; }
                while (peek() != ']' && peek() != '\0') {
                    if (conn.source_count >= LADDER_MAX_CONN_ENDPOINTS) {
                        setError("Too many sources");
                        return false;
                    }
                    if (!readString(conn.sources[conn.source_count], 32)) return false;
                    conn.source_count++;
                    skipWhitespace();
                    if (peek() == ',') advance();
                }
                if (!expect(']')) { setError("Expected ']'"); return false; }
            } else if (ladderStrEqI(key, "destinations")) {
                has_destinations = true;
                if (!expect('[')) { setError("Expected destinations array"); return false; }
                while (peek() != ']' && peek() != '\0') {
                    if (conn.dest_count >= LADDER_MAX_CONN_ENDPOINTS) {
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

            if (ladderStrEqI(key, "nodes")) {
                has_nodes = true;
                if (!expect('[')) { setError("Expected nodes array"); return false; }
                while (peek() != ']' && peek() != '\0') {
                    if (node_count >= LADDER_MAX_NODES) {
                        setError("Too many nodes");
                        return false;
                    }
                    if (!parseNode(nodes[node_count])) return false;
                    node_count++;
                    skipWhitespace();
                    if (peek() == ',') advance();
                }
                if (!expect(']')) { setError("Expected ']'"); return false; }
            } else if (ladderStrEqI(key, "connections")) {
                has_connections = true;
                if (!expect('[')) { setError("Expected connections array"); return false; }
                while (peek() != ']' && peek() != '\0') {
                    if (connection_count >= LADDER_MAX_CONNECTIONS) {
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
            if (ladderStrEqI(arr[i], str)) return true;
        }
        return false;
    }

    // Helper: Check if two connections share any source or destination
    bool connectionsShareEndpoints(const LadderConnection& a, const LadderConnection& b) {
        // Check if they share any sources
        for (int i = 0; i < a.source_count; i++) {
            for (int j = 0; j < b.source_count; j++) {
                if (ladderStrEqI(a.sources[i], b.sources[j])) return true;
            }
        }
        // Check if they share any destinations
        for (int i = 0; i < a.dest_count; i++) {
            for (int j = 0; j < b.dest_count; j++) {
                if (ladderStrEqI(a.destinations[i], b.destinations[j])) return true;
            }
        }
        return false;
    }

    // Helper: Merge connection b into connection a (adding unique sources/destinations)
    void mergeConnections(LadderConnection& a, const LadderConnection& b) {
        // Merge sources
        for (int i = 0; i < b.source_count; i++) {
            if (!stringInArray(b.sources[i], a.sources, a.source_count)) {
                if (a.source_count < LADDER_MAX_CONN_ENDPOINTS) {
                    strcpy(a.sources[a.source_count], b.sources[i]);
                    a.source_count++;
                }
            }
        }
        // Merge destinations
        for (int i = 0; i < b.dest_count; i++) {
            if (!stringInArray(b.destinations[i], a.destinations, a.dest_count)) {
                if (a.dest_count < LADDER_MAX_CONN_ENDPOINTS) {
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
};

#endif // __WASM__
