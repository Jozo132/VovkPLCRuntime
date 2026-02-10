// st-linter.h - Linter for Structured Text (ST) code
// Extends STCompiler to capture multiple errors for IDE integration.
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

#include "st-compiler.h"
#include "plcasm-linter.h" // Reuse LinterProblem struct and enums

#define ST_MAX_LINT_PROBLEMS 100

// ============================================================================
// ST Linter Class Definition
// ============================================================================

class STLinter {
public:
    LinterProblem problems[ST_MAX_LINT_PROBLEMS];
    int problem_count = 0;
    
    // Own shared symbol table for standalone use
    SharedSymbolTable ownedSymbols;
    
    // The underlying compiler (owns its own instance)
    STCompiler compiler;
    
    // Track token info for error reporting
    int token_start_column = 1;
    int token_length = 0;
    
    // Error state (mirrored from compiler for easier access)
    bool has_error = false;
    char error_message[256];
    int error_line = 0;
    int error_column = 0;
    
    STLinter() : compiler(ownedSymbols) {
        problem_count = 0;
        has_error = false;
        error_message[0] = '\0';
    }
    
    void reset() {
        compiler.reset();
        clearProblems();
    }
    
    void clearProblems() {
        problem_count = 0;
        has_error = false;
        error_message[0] = '\0';
        error_line = 0;
        error_column = 0;
    }
    
    // Set source code for compilation
    void setSource(const char* src, int len) {
        compiler.setSource(src, len);
    }
    
    // Get the shared symbol table (for project compiler to populate)
    SharedSymbolTable& getSharedSymbols() {
        return ownedSymbols;
    }
    
    // Add an error
    void addError(const char* msg, int line, int column, int length = 1) {
        if (problem_count >= ST_MAX_LINT_PROBLEMS) return;
        
        // Check for duplicate on same line/column
        for (int j = 0; j < problem_count; j++) {
            if (problems[j].line == (uint32_t)line && 
                problems[j].column == (uint32_t)column &&
                problems[j].type == LINT_ERROR) {
                return; // Skip duplicate
            }
        }
        
        LinterProblem& p = problems[problem_count];
        p.type = LINT_ERROR;
        p.line = line;
        p.column = column;
        p.length = length > 0 ? length : 1;
        p.token_text = nullptr;
        p.lang = LANG_ST; // ST language ID = 6
        p.block[0] = '\0';
        p.program[0] = '\0';
        
        // Reset and copy message
        for (int k = 0; k < 128; k++) p.message[k] = 0;
        int i = 0;
        while (msg[i] && i < 127) {
            p.message[i] = msg[i];
            i++;
        }
        p.message[i] = '\0';
        
        problem_count++;
    }
    
    // Add a warning
    void addWarning(const char* msg, int line, int column, int length = 1) {
        if (problem_count >= ST_MAX_LINT_PROBLEMS) return;
        
        LinterProblem& p = problems[problem_count];
        p.type = LINT_WARNING;
        p.line = line;
        p.column = column;
        p.length = length > 0 ? length : 1;
        p.token_text = nullptr;
        p.lang = LANG_ST; // ST language ID = 6
        p.block[0] = '\0';
        p.program[0] = '\0';
        
        for (int k = 0; k < 128; k++) p.message[k] = 0;
        int i = 0;
        while (msg[i] && i < 127) {
            p.message[i] = msg[i];
            i++;
        }
        p.message[i] = '\0';
        
        problem_count++;
    }
    
    // Add an info message
    void addInfo(const char* msg, int line, int column, int length = 1) {
        if (problem_count >= ST_MAX_LINT_PROBLEMS) return;
        
        LinterProblem& p = problems[problem_count];
        p.type = LINT_INFO;
        p.line = line;
        p.column = column;
        p.length = length > 0 ? length : 1;
        p.token_text = nullptr;
        p.lang = LANG_ST; // ST language ID = 6
        p.block[0] = '\0';
        p.program[0] = '\0';
        
        for (int k = 0; k < 128; k++) p.message[k] = 0;
        int i = 0;
        while (msg[i] && i < 127) {
            p.message[i] = msg[i];
            i++;
        }
        p.message[i] = '\0';
        
        problem_count++;
    }
    
    // Run the linter - compiles and captures errors
    bool lint() {
        clearProblems();
        
        // Run compilation (returns true on success, false on error)
        bool success = compiler.compile();
        
        // Sync error state from compiler
        has_error = compiler.has_error;
        error_line = compiler.error_line;
        error_column = compiler.error_column;
        
        // Copy error message
        int i = 0;
        while (compiler.error_message[i] && i < 255) {
            error_message[i] = compiler.error_message[i];
            i++;
        }
        error_message[i] = '\0';
        
        // If there was an error, capture it as a linter problem
        if (!success && compiler.has_error) {
            // Calculate token length from error token text
            int len = 0;
            while (compiler.error_token_text[len] && len < 63) len++;
            if (len == 0) len = 1;
            
            addError(compiler.error_message, 
                     compiler.error_line, 
                     compiler.error_column,
                     len);
            
            // Copy error token text into the problem's token_buf
            if (problem_count > 0 && compiler.error_token_text[0]) {
                LinterProblem& p = problems[problem_count - 1];
                int k = 0;
                while (compiler.error_token_text[k] && k < 63) {
                    p.token_buf[k] = compiler.error_token_text[k];
                    k++;
                }
                p.token_buf[k] = '\0';
                p.token_text = p.token_buf;
            }
        }
        
        // Copy warnings from compiler
        for (int w = 0; w < compiler.warning_count && problem_count < ST_MAX_LINT_PROBLEMS; w++) {
            STWarning& warn = compiler.warnings[w];
            if (warn.severity == 1) {
                addWarning(warn.message, warn.line, warn.column, warn.length);
            } else if (warn.severity == 0) {
                addInfo(warn.message, warn.line, warn.column, warn.length);
            }
        }
        
        return problem_count == 0;
    }
    
    // Compile (returns true if compilation succeeded, even if warnings exist)
    bool compile() {
        lint();
        return !has_error;
    }
    
    // Get the PLCScript output (if compilation succeeded)
    const char* getOutput() const {
        return compiler.output;
    }
    
    // Alias for consistency with other compilers
    char* output = nullptr;  // Will point to compiler.output after compile
    int output_len = 0;
    
    int getOutputLength() const {
        return compiler.output_len;
    }
};

// ============================================================================
// Global ST Linter Instance
// ============================================================================

STLinter& getSTLinter() {
    static STLinter instance;
    return instance;
}
#define stLinter getSTLinter()

// Temporary buffer for ST lint source (loaded from stream)
static char st_lint_source_buffer[ST_MAX_OUTPUT_SIZE];
static int st_lint_source_length = 0;

// ============================================================================
// WASM Exports for ST Linter
// ============================================================================

extern "C" {

    // Load ST source from stream buffer into linter
    WASM_EXPORT void st_lint_load_from_stream() {
        streamRead(st_lint_source_buffer, st_lint_source_length, ST_MAX_OUTPUT_SIZE);
        stLinter.reset();
        stLinter.setSource(st_lint_source_buffer, st_lint_source_length);
    }

    // Set ST source directly for linting
    WASM_EXPORT void st_lint_set_source(char* source, int length) {
        stLinter.reset();
        stLinter.setSource(source, length);
    }

    // Run the ST linter
    WASM_EXPORT bool st_lint_run() {
        return stLinter.lint();
    }

    // Get number of problems found
    WASM_EXPORT int st_lint_get_problem_count() {
        return stLinter.problem_count;
    }

    // Get pointer to problems array (for direct memory access)
    WASM_EXPORT LinterProblem* st_lint_get_problems_pointer() {
        return stLinter.problems;
    }

    // Get a specific problem's type (0=Info, 1=Warning, 2=Error)
    WASM_EXPORT int st_lint_get_problem_type(int index) {
        if (index < 0 || index >= stLinter.problem_count) return -1;
        return stLinter.problems[index].type;
    }

    // Get a specific problem's line number
    WASM_EXPORT int st_lint_get_problem_line(int index) {
        if (index < 0 || index >= stLinter.problem_count) return -1;
        return stLinter.problems[index].line;
    }

    // Get a specific problem's column number
    WASM_EXPORT int st_lint_get_problem_column(int index) {
        if (index < 0 || index >= stLinter.problem_count) return -1;
        return stLinter.problems[index].column;
    }

    // Get a specific problem's length
    WASM_EXPORT int st_lint_get_problem_length(int index) {
        if (index < 0 || index >= stLinter.problem_count) return -1;
        return stLinter.problems[index].length;
    }

    // Get a specific problem's message (writes to stream)
    WASM_EXPORT void st_lint_get_problem_message(int index) {
        if (index < 0 || index >= stLinter.problem_count) return;
        const char* msg = stLinter.problems[index].message;
        while (*msg) {
            streamOut(*msg++);
        }
    }

    // Get the generated PLCScript output from linting (writes to stream)
    WASM_EXPORT void st_lint_get_output() {
        const char* output = stLinter.getOutput();
        int len = stLinter.getOutputLength();
        for (int i = 0; i < len; i++) {
            streamOut(output[i]);
        }
    }

    // Get the generated PLCScript output length
    WASM_EXPORT int st_lint_get_output_length() {
        return stLinter.getOutputLength();
    }

    // Clear all linter problems
    WASM_EXPORT void st_lint_clear() {
        stLinter.clearProblems();
    }

    // ========================================================================
    // WASM Exports for ST Symbol Table
    // ========================================================================

    // Get number of symbols declared in the ST code
    WASM_EXPORT int st_lint_getSymbolCount() {
        return stLinter.compiler.symbol_count;
    }

    // Get symbol name by index
    WASM_EXPORT const char* st_lint_getSymbolName(int index) {
        if (index < 0 || index >= stLinter.compiler.symbol_count) return "";
        return stLinter.compiler.symbols[index].name;
    }

    // Get symbol type string by index (e.g. "i32", "u8", "bool", "f32")
    WASM_EXPORT const char* st_lint_getSymbolType(int index) {
        if (index < 0 || index >= stLinter.compiler.symbol_count) return "";
        return stLinter.compiler.symbols[index].plcscriptType;
    }

    // Get symbol PLC address string (e.g. "M0", "MW10", "auto")
    WASM_EXPORT const char* st_lint_getSymbolAddress(int index) {
        if (index < 0 || index >= stLinter.compiler.symbol_count) return "";
        return stLinter.compiler.symbols[index].address;
    }

    // Get symbol declaration line
    WASM_EXPORT int st_lint_getSymbolLine(int index) {
        if (index < 0 || index >= stLinter.compiler.symbol_count) return 0;
        return stLinter.compiler.symbols[index].declarationLine;
    }

    // Get symbol declaration column
    WASM_EXPORT int st_lint_getSymbolColumn(int index) {
        if (index < 0 || index >= stLinter.compiler.symbol_count) return 0;
        return stLinter.compiler.symbols[index].declarationColumn;
    }

    // Check if symbol is a constant (VAR CONSTANT)
    WASM_EXPORT bool st_lint_getSymbolIsConst(int index) {
        if (index < 0 || index >= stLinter.compiler.symbol_count) return false;
        return stLinter.compiler.symbols[index].isConstant;
    }

    // Check if symbol is used after declaration
    WASM_EXPORT bool st_lint_getSymbolIsUsed(int index) {
        if (index < 0 || index >= stLinter.compiler.symbol_count) return false;
        return stLinter.compiler.symbols[index].used;
    }
}

#endif // __WASM__
