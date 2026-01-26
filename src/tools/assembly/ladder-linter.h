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

// ### Ladder Linter Class Definition ###

class LadderLinter : public LadderGraphCompiler {
public:
    LinterProblem problems[LADDER_MAX_LINT_PROBLEMS];
    int problem_count = 0;
    
    // Position tracking for JSON parsing
    int current_line = 1;
    int current_column = 1;
    
    LadderLinter() : LadderGraphCompiler() {
        problem_count = 0;
    }
    
    void clearProblems() {
        problem_count = 0;
        has_error = false;
        error_msg[0] = '\0';
        current_line = 1;
        current_column = 1;
    }
    
    void reset() override {
        LadderGraphCompiler::reset();
        clearProblems();
    }
    
    // ============ Problem Reporting ============
    
    void addProblem(LinterProblemType type, const char* message, int line, int column, int len) {
        if (problem_count >= LADDER_MAX_LINT_PROBLEMS) return;
        
        LinterProblem& p = problems[problem_count];
        p.type = type;
        p.line = line;
        p.column = column;
        p.length = len > 0 ? len : 1;
        p.token_text = nullptr;
        
        // Copy message safely (max 63 chars)
        int i = 0;
        while (message[i] && i < 63) {
            p.message[i] = message[i];
            i++;
        }
        p.message[i] = '\0';
        
        problem_count++;
    }
    
    void addError(const char* message, int line = 1, int column = 1, int len = 1) {
        addProblem(LINT_ERROR, message, line, column, len);
    }
    
    void addWarning(const char* message, int line = 1, int column = 1, int len = 1) {
        addProblem(LINT_WARNING, message, line, column, len);
    }
    
    void addInfo(const char* message, int line = 1, int column = 1, int len = 1) {
        addProblem(LINT_INFO, message, line, column, len);
    }
    
    // Override setError to also add to problems list
    void setError(const char* msg) override {
        if (has_error) return;
        has_error = true;
        int i = 0;
        while (msg[i] && i < 255) {
            error_msg[i] = msg[i];
            i++;
        }
        error_msg[i] = '\0';
        
        // Calculate approximate line/column from current position
        int line = 1, col = 1;
        for (int j = 0; j < pos && source && j < length; j++) {
            if (source[j] == '\n') {
                line++;
                col = 1;
            } else {
                col++;
            }
        }
        
        // Also add to problems list
        addProblem(LINT_ERROR, msg, line, col, 1);
    }
    
    // ============ Semantic Validation ============
    
    // Calculate line/column for a node based on searching for its ID in source
    void findNodePosition(const char* nodeId, int& line, int& column) {
        line = 1;
        column = 1;
        if (!source || !nodeId || nodeId[0] == '\0') return;
        
        // Search for the node ID in the source
        int idLen = 0;
        while (nodeId[idLen]) idLen++;
        
        for (int i = 0; i < length - idLen; i++) {
            // Track position
            if (source[i] == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            
            // Check for match (looking for "id":"nodeId")
            bool match = true;
            for (int j = 0; j < idLen && match; j++) {
                if (source[i + j] != nodeId[j]) match = false;
            }
            if (match) return;
        }
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
    
    // Validate node has required fields
    void validateNode(GraphNode& node, int nodeIndex) {
        int line = 1, col = 1;
        findNodePosition(node.id, line, col);
        
        // Check for unknown node type
        if (!isKnownNodeType(node.type)) {
            char msg[64];
            int mi = 0;
            const char* prefix = "Unknown node type '";
            while (prefix[mi]) { msg[mi] = prefix[mi]; mi++; }
            int ti = 0;
            while (node.type[ti] && mi < 50) { msg[mi++] = node.type[ti++]; }
            const char* suffix = "'";
            int si = 0;
            while (suffix[si] && mi < 63) { msg[mi++] = suffix[si++]; }
            msg[mi] = '\0';
            addError(msg, line, col, 1);
        }
        
        // Check address for most node types (tap nodes don't need addresses)
        if (!isTapNode(node.type) && isKnownNodeType(node.type)) {
            if (node.address[0] == '\0') {
                char msg[64];
                int mi = 0;
                const char* prefix = "Node '";
                while (prefix[mi]) { msg[mi] = prefix[mi]; mi++; }
                int ni = 0;
                while (node.id[ni] && mi < 40) { msg[mi++] = node.id[ni++]; }
                const char* suffix = "' missing address";
                int si = 0;
                while (suffix[si] && mi < 63) { msg[mi++] = suffix[si++]; }
                msg[mi] = '\0';
                addWarning(msg, line, col, 1);
            }
        }
    }
    
    // Validate connection references valid nodes
    void validateConnection(Connection& conn, int connIndex) {
        // Check sources exist
        for (int i = 0; i < conn.source_count; i++) {
            bool found = false;
            for (int j = 0; j < node_count; j++) {
                if (strEqI(conn.sources[i], nodes[j].id)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                char msg[64];
                int mi = 0;
                const char* prefix = "Source '";
                while (prefix[mi]) { msg[mi] = prefix[mi]; mi++; }
                int si = 0;
                while (conn.sources[i][si] && mi < 45) { msg[mi++] = conn.sources[i][si++]; }
                const char* suffix = "' not found";
                int ssi = 0;
                while (suffix[ssi] && mi < 63) { msg[mi++] = suffix[ssi++]; }
                msg[mi] = '\0';
                
                int line = 1, col = 1;
                findNodePosition(conn.sources[i], line, col);
                addError(msg, line, col, 1);
            }
        }
        
        // Check destinations exist
        for (int i = 0; i < conn.dest_count; i++) {
            bool found = false;
            for (int j = 0; j < node_count; j++) {
                if (strEqI(conn.destinations[i], nodes[j].id)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                char msg[64];
                int mi = 0;
                const char* prefix = "Destination '";
                while (prefix[mi]) { msg[mi] = prefix[mi]; mi++; }
                int di = 0;
                while (conn.destinations[i][di] && mi < 45) { msg[mi++] = conn.destinations[i][di++]; }
                const char* suffix = "' not found";
                int ssi = 0;
                while (suffix[ssi] && mi < 63) { msg[mi++] = suffix[ssi++]; }
                msg[mi] = '\0';
                
                int line = 1, col = 1;
                findNodePosition(conn.destinations[i], line, col);
                addError(msg, line, col, 1);
            }
        }
    }
    
    // Validate entire graph structure
    void validateGraph() {
        // Validate all nodes
        for (int i = 0; i < node_count; i++) {
            validateNode(nodes[i], i);
        }
        
        // Validate all connections
        for (int i = 0; i < connection_count; i++) {
            validateConnection(connections[i], i);
        }
        
        // Check for disconnected outputs (coils without inputs)
        for (int i = 0; i < node_count; i++) {
            if (isTerminationNode(nodes[i].type)) {
                bool hasInput = false;
                for (int j = 0; j < connection_count; j++) {
                    for (int k = 0; k < connections[j].dest_count; k++) {
                        if (strEqI(connections[j].destinations[k], nodes[i].id)) {
                            hasInput = true;
                            break;
                        }
                    }
                    if (hasInput) break;
                }
                if (!hasInput) {
                    char msg[64];
                    int mi = 0;
                    const char* prefix = "Output '";
                    while (prefix[mi]) { msg[mi] = prefix[mi]; mi++; }
                    int ni = 0;
                    while (nodes[i].id[ni] && mi < 45) { msg[mi++] = nodes[i].id[ni++]; }
                    const char* suffix = "' has no inputs";
                    int si = 0;
                    while (suffix[si] && mi < 63) { msg[mi++] = suffix[si++]; }
                    msg[mi] = '\0';
                    
                    int line = 1, col = 1;
                    findNodePosition(nodes[i].id, line, col);
                    addWarning(msg, line, col, 1);
                }
            }
        }
        
        // Check for orphaned contacts (contacts not connected to anything)
        for (int i = 0; i < node_count; i++) {
            if (isInputNode(nodes[i].type)) {
                bool hasOutput = false;
                for (int j = 0; j < connection_count; j++) {
                    for (int k = 0; k < connections[j].source_count; k++) {
                        if (strEqI(connections[j].sources[k], nodes[i].id)) {
                            hasOutput = true;
                            break;
                        }
                    }
                    if (hasOutput) break;
                }
                if (!hasOutput) {
                    char msg[64];
                    int mi = 0;
                    const char* prefix = "Contact '";
                    while (prefix[mi]) { msg[mi] = prefix[mi]; mi++; }
                    int ni = 0;
                    while (nodes[i].id[ni] && mi < 45) { msg[mi++] = nodes[i].id[ni++]; }
                    const char* suffix = "' not connected";
                    int si = 0;
                    while (suffix[si] && mi < 63) { msg[mi++] = suffix[si++]; }
                    msg[mi] = '\0';
                    
                    int line = 1, col = 1;
                    findNodePosition(nodes[i].id, line, col);
                    addWarning(msg, line, col, 1);
                }
            }
        }
    }
    
    // ============ Main Entry Points ============
    
    // Lint-only mode: parse, validate, but don't compile to STL
    bool lint(const char* src, int len) {
        reset();
        source = src;
        length = len;
        
        // Use base class parseGraph to parse the JSON
        if (!parseGraph()) {
            // Parser error already added via setError override
            return false;
        }
        
        // Validate the graph structure (adds warnings)
        validateGraph();
        
        // Return true unless we have hard errors
        return !has_error;
    }
    
    // Full compile mode: parse, validate, compile to STL
    bool parse(const char* src, int len) {
        reset();
        source = src;
        length = len;
        
        // Parse the graph JSON
        if (!parseGraph()) {
            return false;
        }
        
        // Validate the graph structure
        validateGraph();
        
        // Check if we have any errors (not just warnings)
        bool hasErrors = false;
        for (int i = 0; i < problem_count; i++) {
            if (problems[i].type == LINT_ERROR) {
                hasErrors = true;
                break;
            }
        }
        
        // Only compile if no errors
        if (!hasErrors && !compile()) {
            return false;
        }
        
        return !has_error && !hasErrors;
    }
};


// ============================================================================
// Global Instance and WASM Exports
// ============================================================================

static LadderLinter ladderLinter;

extern "C" {

WASM_EXPORT void ladder_lint_load_from_stream() {
    streamRead(ladder_input_buffer, ladder_input_length, sizeof(ladder_input_buffer));
}

WASM_EXPORT bool ladder_lint_run() {
    return ladderLinter.lint(ladder_input_buffer, ladder_input_length);
}

WASM_EXPORT bool ladder_lint_compile() {
    return ladderLinter.parse(ladder_input_buffer, ladder_input_length);
}

WASM_EXPORT int ladder_lint_problem_count() {
    return ladderLinter.problem_count;
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
    return ladderLinter.has_error;
}

WASM_EXPORT const char* ladder_lint_get_error() {
    return ladderLinter.error_msg;
}

WASM_EXPORT void ladder_lint_clear() {
    ladderLinter.clearProblems();
}

} // extern "C"

#endif // __WASM__
