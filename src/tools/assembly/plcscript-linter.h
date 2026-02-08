// plcscript-linter.h - Linter for PLCScript code
// Extends PLCScriptCompiler to capture multiple errors/warnings for IDE integration.
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

#include "plcscript-compiler.h"
#include "plcasm-linter.h" // Reuse LinterProblem struct and enums

#define PLCSCRIPT_MAX_LINT_PROBLEMS 100

// ### PLCScript Linter Class Definition ###

class PLCScriptLinter : public PLCScriptCompiler {
public:
    LinterProblem problems[PLCSCRIPT_MAX_LINT_PROBLEMS];
    int problem_count = 0;
    
    // Track if we're in lint-only mode (no code generation)
    bool lintOnly = false;

    PLCScriptLinter() : PLCScriptCompiler() {
        problem_count = 0;
        lintOnly = false;
    }

    void clearProblems() {
        problem_count = 0;
        hasError = false;
        errorMessage[0] = '\0';
        errorLine = 0;
        errorColumn = 0;
    }

    // Add a problem to the list
    void addProblem(int line, int column, int length, LinterProblemType type, const char* message) {
        if (problem_count >= PLCSCRIPT_MAX_LINT_PROBLEMS) return;
        
        // Check for duplicate problems at the same location with the same message
        for (int i = 0; i < problem_count; i++) {
            if (problems[i].line == line && problems[i].column == column && problems[i].type == type) {
                // Check if same message
                bool same = true;
                const char* a = problems[i].message;
                const char* b = message;
                while (*a && *b) {
                    if (*a != *b) { same = false; break; }
                    a++; b++;
                }
                if (same && *a == *b) return; // Skip duplicate
            }
        }
        
        LinterProblem& p = problems[problem_count++];
        p.line = line;
        p.column = column;
        p.length = length > 0 ? length : 1;
        p.type = type;
        
        // Copy message
        int i = 0;
        while (message[i] && i < 127) {
            p.message[i] = message[i];
            i++;
        }
        p.message[i] = '\0';
    }

    // Override setError to add problems instead of stopping
    void setError(const char* msg) override {
        // Add as error problem
        int length = currentToken.textLen > 0 ? currentToken.textLen : 1;
        addProblem(currentLine, currentColumn, length, LINT_ERROR, msg);
        
        // Also set the base class error for backwards compatibility
        PLCScriptCompiler::setError(msg);
    }

    // Override warning methods to capture warnings as lint problems
    void addWarning(const char* msg) override {
        int length = currentToken.textLen > 0 ? currentToken.textLen : 1;
        addProblem(currentLine, currentColumn, length, LINT_WARNING, msg);
    }

    void addWarningAt(const char* msg, int line, int col, int length) override {
        addProblem(line, col, length > 0 ? length : 1, LINT_WARNING, msg);
    }

    void addInfo(const char* msg) override {
        int length = currentToken.textLen > 0 ? currentToken.textLen : 1;
        addProblem(currentLine, currentColumn, length, LINT_INFO, msg);
    }

    void addInfoAt(const char* msg, int line, int col, int length) override {
        addProblem(line, col, length > 0 ? length : 1, LINT_INFO, msg);
    }

    // Lint the source code (compile to collect errors but optionally don't generate code)
    bool lint() {
        // Reset state
        clearProblems();
        reset();
        
        // Set source if not already set
        if (!source || sourceLength == 0) {
            return true; // Empty source is valid
        }

        // Perform full compilation - this will collect all errors via setError overrides
        // and warnings via addWarning overrides
        compile();
        
        // If there were errors captured, also add them as problems if not already
        if (hasError && problem_count == 0) {
            addProblem(errorLine, errorColumn, 1, LINT_ERROR, errorMessage);
        }
        
        return problem_count == 0;
    }

    // Get problem count
    int getProblemCount() const { return problem_count; }
    
    // Get problem by index
    const LinterProblem* getProblem(int index) const {
        if (index < 0 || index >= problem_count) return nullptr;
        return &problems[index];
    }
};

// ============================================================================
// WASM Exports for PLCScript Linter
// ============================================================================

PLCScriptLinter& getStandalonePLCScriptLinter() {
    static PLCScriptLinter instance;
    return instance;
}
#define __standalone_plcscript_linter__ getStandalonePLCScriptLinter()

static char plcscript_linter_source_buffer[PLCSCRIPT_MAX_OUTPUT_SIZE];
static int plcscript_linter_source_length = 0;

// Load PLCScript source from stream for linting
WASM_EXPORT void plcscript_linter_load_from_stream() {
    streamRead(plcscript_linter_source_buffer, plcscript_linter_source_length, PLCSCRIPT_MAX_OUTPUT_SIZE);
    __standalone_plcscript_linter__.setSource(plcscript_linter_source_buffer, plcscript_linter_source_length);
}

// Run the linter
WASM_EXPORT bool plcscript_linter_lint() {
    return __standalone_plcscript_linter__.lint();
}

// Get problem count
WASM_EXPORT int plcscript_linter_getProblemCount() {
    return __standalone_plcscript_linter__.getProblemCount();
}

// Get problem line by index
WASM_EXPORT int plcscript_linter_getProblemLine(int index) {
    const LinterProblem* p = __standalone_plcscript_linter__.getProblem(index);
    return p ? p->line : 0;
}

// Get problem column by index
WASM_EXPORT int plcscript_linter_getProblemColumn(int index) {
    const LinterProblem* p = __standalone_plcscript_linter__.getProblem(index);
    return p ? p->column : 0;
}

// Get problem length by index
WASM_EXPORT int plcscript_linter_getProblemLength(int index) {
    const LinterProblem* p = __standalone_plcscript_linter__.getProblem(index);
    return p ? p->length : 0;
}

// Get problem severity by index (0=info, 1=warning, 2=error)
WASM_EXPORT int plcscript_linter_getProblemSeverity(int index) {
    const LinterProblem* p = __standalone_plcscript_linter__.getProblem(index);
    return p ? (int)p->type : 0;
}

// Get problem message by index
WASM_EXPORT const char* plcscript_linter_getProblemMessage(int index) {
    const LinterProblem* p = __standalone_plcscript_linter__.getProblem(index);
    return p ? p->message : "";
}

// Reset the linter
WASM_EXPORT void plcscript_linter_reset() {
    __standalone_plcscript_linter__.reset();
    __standalone_plcscript_linter__.clearProblems();
}

// Clear problems only
WASM_EXPORT void plcscript_linter_clearProblems() {
    __standalone_plcscript_linter__.clearProblems();
}

// Get output (if also compiling)
WASM_EXPORT const char* plcscript_linter_getOutput() {
    return __standalone_plcscript_linter__.output;
}

// Get output length
WASM_EXPORT int plcscript_linter_getOutputLength() {
    return __standalone_plcscript_linter__.outputLength;
}

#endif // __WASM__
