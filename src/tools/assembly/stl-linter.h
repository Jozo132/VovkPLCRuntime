// stl-linter.h - Linter for STL (Statement List) code
// Extends STLCompiler to capture multiple errors for IDE integration.
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
#include "plcasm-linter.h" // Reuse LinterProblem struct and enums

#define STL_MAX_LINT_PROBLEMS 100

// ### STL Linter Class Definition ###

class STLLinter : public STLCompiler {
public:
    LinterProblem problems[STL_MAX_LINT_PROBLEMS];
    int problem_count = 0;
    
    // Track token info for error reporting
    int token_start_column = 1;
    int token_length = 0;

    STLLinter() : STLCompiler() {
        problem_count = 0;
    }

    void clearProblems() {
        problem_count = 0;
        has_error = false;
        error_message[0] = '\0';
        error_line = 0;
        error_column = 0;
    }

    // Override setError to capture errors instead of stopping at the first one
    void setError(const char* msg) override {
        // Find if we already have an error on this line
        int replace_index = -1;
        int same_line_index = -1;
        
        for (int j = 0; j < problem_count; j++) {
            if (problems[j].line == (uint32_t)current_line) {
                if (same_line_index < 0) same_line_index = j;
                if (problems[j].column == (uint32_t)token_start_column) {
                    replace_index = j;
                    break;
                }
            }
        }

        // If same line but earlier column exists, replace it; otherwise skip duplicate
        if (replace_index < 0 && same_line_index >= 0) {
            if ((uint32_t)token_start_column < problems[same_line_index].column) {
                replace_index = same_line_index;
            } else {
                // Don't add duplicate error on same line with later column
                has_error = true;
                return;
            }
        }

        // Check if we've reached max problems
        if (replace_index < 0 && problem_count >= STL_MAX_LINT_PROBLEMS) {
            has_error = true;
            return;
        }

        // Add or replace the problem
        LinterProblem& p = replace_index >= 0 ? problems[replace_index] : problems[problem_count];
        p.type = LINT_ERROR;
        p.line = current_line;
        p.column = token_start_column > 0 ? token_start_column : current_column;
        p.length = token_length > 0 ? token_length : 1;
        p.token_text = nullptr; // STL doesn't have Token struct

        // Reset message buffer
        for (int k = 0; k < 64; k++) p.message[k] = 0;

        // Copy message safely (max 63 chars)
        int i = 0;
        while (msg[i] && i < 63) {
            p.message[i] = msg[i];
            i++;
        }
        p.message[i] = '\0';

        if (replace_index < 0) problem_count++;
        
        // Set the error flag but don't stop compilation in lint mode
        has_error = true;
        
        // Also store in base class error fields for compatibility
        if (error_line == 0) {
            error_line = current_line;
            error_column = token_start_column > 0 ? token_start_column : current_column;
            int j = 0;
            while (msg[j] && j < 255) {
                error_message[j] = msg[j];
                j++;
            }
            error_message[j] = '\0';
        }
    }

    // Add a warning (non-fatal)
    void addWarning(const char* msg, int line = -1, int column = -1, int length = 1) {
        if (problem_count >= STL_MAX_LINT_PROBLEMS) return;
        
        LinterProblem& p = problems[problem_count];
        p.type = LINT_WARNING;
        p.line = line >= 0 ? line : current_line;
        p.column = column >= 0 ? column : current_column;
        p.length = length;
        p.token_text = nullptr;

        for (int k = 0; k < 64; k++) p.message[k] = 0;
        int i = 0;
        while (msg[i] && i < 63) {
            p.message[i] = msg[i];
            i++;
        }
        p.message[i] = '\0';

        problem_count++;
    }

    // Add an info message
    void addInfo(const char* msg, int line = -1, int column = -1, int length = 1) {
        if (problem_count >= STL_MAX_LINT_PROBLEMS) return;
        
        LinterProblem& p = problems[problem_count];
        p.type = LINT_INFO;
        p.line = line >= 0 ? line : current_line;
        p.column = column >= 0 ? column : current_column;
        p.length = length;
        p.token_text = nullptr;

        for (int k = 0; k < 64; k++) p.message[k] = 0;
        int i = 0;
        while (msg[i] && i < 63) {
            p.message[i] = msg[i];
            i++;
        }
        p.message[i] = '\0';

        problem_count++;
    }

    // Lint mode compile - continues past errors to find more issues
    bool lint() {
        clearProblems();
        
        if (!stl_source || stl_length == 0) {
            setError("No source code provided");
            return false;
        }

        // Reset parsing state
        pos = 0;
        current_line = 1;
        current_column = 1;
        output_length = 0;
        output[0] = '\0';
        network_has_rlo = false;
        nesting_depth = 0;
        label_counter = 0;
        
        // Add header comment
        emitLine("// Generated from STL by VovkPLCRuntime STL Compiler");
        emitLine("");

        while (pos < stl_length) {
            skipWhitespace();
            
            char c = peek();
            
            // End of input
            if (c == '\0') break;
            
            // Newline - just advance
            if (c == '\n') {
                advance();
                network_has_rlo = false;
                continue;
            }
            
            // Comment (// or (* *))
            if (c == '/' && peek(1) == '/') {
                // Copy comment to output
                emit("//");
                advance(); advance();
                while (pos < stl_length && peek() != '\n') {
                    char cc = advance();
                    char buf[2] = {cc, '\0'};
                    emit(buf);
                }
                emit("\n");
                continue;
            }
            
            // Track token start for error reporting
            token_start_column = current_column;
            
            // Label definition: LABEL:
            if (isAlpha(c)) {
                char identifier[64];
                int start_col = current_column;
                readIdentifier(identifier, sizeof(identifier));
                token_length = current_column - start_col;
                
                skipWhitespace();
                
                if (peek() == ':') {
                    // It's a label
                    advance();
                    emit(identifier);
                    emitLine(":");
                } else {
                    // It's an instruction
                    processInstruction(identifier);
                }
                continue;
            }
            
            // = (assign) instruction OR ==I comparison
            if (c == '=') {
                token_start_column = current_column;
                advance();
                if (peek() == '=') {
                    advance();
                    if (peek() == 'I' || peek() == 'i') {
                        advance();
                        token_length = 3;
                        handleCompare("==I");
                    } else {
                        token_length = 2;
                        setError("Expected '==I' for integer comparison");
                    }
                } else {
                    // Assignment
                    token_length = 1;
                    skipWhitespace();
                    char operand[64];
                    readIdentifier(operand, sizeof(operand));
                    if (operand[0] == '\0') {
                        setError("Expected operand after '='");
                    } else {
                        handleAssign(operand);
                    }
                }
                continue;
            }
            
            // Math operators (+I, -I, *I, /I)
            if (c == '+' || c == '-' || c == '*' || c == '/') {
                token_start_column = current_column;
                char op[4] = {0};
                op[0] = advance();
                if (peek() == 'I' || peek() == 'i') {
                    op[1] = advance();
                    token_length = 2;
                    handleMath(op);
                } else {
                    token_length = 1;
                    setError("Expected 'I' after math operator");
                }
                continue;
            }
            
            // Comparison operators (<>I, >I, >=I, <I, <=I)
            if (c == '<' || c == '>') {
                token_start_column = current_column;
                char op[4] = {0};
                op[0] = advance();
                if (peek() == '>' || peek() == '=') {
                    op[1] = advance();
                }
                if (peek() == 'I' || peek() == 'i') {
                    op[strlen(op)] = advance();
                    token_length = strlen(op);
                    handleCompare(op);
                } else {
                    token_length = strlen(op);
                    setError("Expected 'I' after comparison operator");
                }
                continue;
            }
            
            // Special characters
            if (c == '(') {
                token_start_column = current_column;
                token_length = 1;
                advance();
                // Standalone ( is an error - should be A(, O(, X(
                setError("Unexpected '(' - use A(, O(, or X( for nesting");
                continue;
            }
            
            if (c == ')') {
                token_start_column = current_column;
                token_length = 1;
                advance();
                handleNestingClose();
                continue;
            }
            
            // Skip unknown characters
            token_start_column = current_column;
            token_length = 1;
            advance();
        }

        // Check for unclosed nesting
        if (nesting_depth > 0) {
            addWarning("Unclosed parenthesis - missing ')'");
        }

        return problem_count == 0;
    }
};

// Global STL Linter Instance
STLLinter& getSTLLinter() {
    static STLLinter instance;
    return instance;
}
#define stlLinter getSTLLinter()

// Temporary buffer for STL lint source (loaded from stream)
static char stl_lint_source_buffer[STL_MAX_OUTPUT_SIZE];
static int stl_lint_source_length = 0;

// ### WASM Exports for STL Linter ###

extern "C" {

    // Load STL source from stream buffer into linter
    WASM_EXPORT void stl_lint_load_from_stream() {
        streamRead(stl_lint_source_buffer, stl_lint_source_length, STL_MAX_OUTPUT_SIZE);
        stlLinter.setSource(stl_lint_source_buffer, stl_lint_source_length);
    }

    // Set STL source directly for linting
    WASM_EXPORT void stl_lint_set_source(char* source, int length) {
        stlLinter.setSource(source, length);
    }

    // Run the STL linter
    WASM_EXPORT bool stl_lint_run() {
        return stlLinter.lint();
    }

    // Get number of problems found
    WASM_EXPORT int stl_lint_get_problem_count() {
        return stlLinter.problem_count;
    }

    // Get pointer to problems array (for direct memory access)
    WASM_EXPORT LinterProblem* stl_lint_get_problems_pointer() {
        return stlLinter.problems;
    }

    // Get a specific problem's type (0=Info, 1=Warning, 2=Error)
    WASM_EXPORT int stl_lint_get_problem_type(int index) {
        if (index < 0 || index >= stlLinter.problem_count) return -1;
        return stlLinter.problems[index].type;
    }

    // Get a specific problem's line number
    WASM_EXPORT int stl_lint_get_problem_line(int index) {
        if (index < 0 || index >= stlLinter.problem_count) return -1;
        return stlLinter.problems[index].line;
    }

    // Get a specific problem's column number
    WASM_EXPORT int stl_lint_get_problem_column(int index) {
        if (index < 0 || index >= stlLinter.problem_count) return -1;
        return stlLinter.problems[index].column;
    }

    // Get a specific problem's length
    WASM_EXPORT int stl_lint_get_problem_length(int index) {
        if (index < 0 || index >= stlLinter.problem_count) return -1;
        return stlLinter.problems[index].length;
    }

    // Get a specific problem's message (writes to stream)
    WASM_EXPORT void stl_lint_get_problem_message(int index) {
        if (index < 0 || index >= stlLinter.problem_count) return;
        const char* msg = stlLinter.problems[index].message;
        while (*msg) {
            streamOut(*msg++);
        }
    }

    // Get the generated PLCASM output from linting (writes to stream)
    WASM_EXPORT void stl_lint_get_output() {
        const char* output = stlLinter.getOutput();
        int len = stlLinter.getOutputLength();
        for (int i = 0; i < len; i++) {
            streamOut(output[i]);
        }
    }

    // Get the generated PLCASM output length
    WASM_EXPORT int stl_lint_get_output_length() {
        return stlLinter.getOutputLength();
    }

    // Clear all linter problems
    WASM_EXPORT void stl_lint_clear() {
        stlLinter.clearProblems();
    }
}

#endif // __WASM__
