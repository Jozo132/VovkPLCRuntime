// plcasm-linter.h
// Linter for VovkPLCRuntime assembly, extends PLCASMCompiler.
#pragma once

#ifdef __WASM__

#include "plcasm-compiler.h"

// ### Linter Structure Definitions ###

enum LinterProblemType {
    LINT_INFO = 0,
    LINT_WARNING = 1,
    LINT_ERROR = 2
};

struct LinterProblem {
    uint32_t type;      // 0=Info, 1=Warning, 2=Error
    uint32_t line;      // Line number (1-based)
    uint32_t column;    // Column number (1-based)
    uint32_t length;    // Length of the token causing the issue
    char message[64]; // Text description
    char* token_text; // Text of the token (only useful for internal debugging)
};

#define MAX_LINT_PROBLEMS 100

// ### Linter Class Definition ###

class PLCASMLinter : public PLCASMCompiler {
public:
    LinterProblem problems[MAX_LINT_PROBLEMS];
    int problem_count = 0;

    PLCASMLinter() : PLCASMCompiler() {
        problem_count = 0;
    }

    void clearArray() {
        problem_count = 0;
    }

    // Override the reportError method to capture errors instead of printing to Serial
    // Returns true to indicate an error occurred (which stops the build in the current compiler logic)
    bool reportError(Token& token, const char* message) override {
        int replace_index = -1;
        int same_line_index = -1;
        for (int j = 0; j < problem_count; j++) {
            if (problems[j].line == token.line) {
                if (same_line_index < 0) same_line_index = j;
                if (problems[j].column == token.column) {
                    replace_index = j;
                    break;
                }
            }
        }

        if (replace_index < 0 && same_line_index >= 0) {
            if (token.column < problems[same_line_index].column) replace_index = same_line_index;
            else return false;
        }

        if (replace_index < 0 && problem_count >= MAX_LINT_PROBLEMS) return true;

        LinterProblem& p = replace_index >= 0 ? problems[replace_index] : problems[problem_count];
        p.type = LINT_ERROR;
        p.line = token.line;
        p.column = token.column;
        p.length = token.length;
        p.token_text = token.string.data;

        // Reset message buffer
        for(int k=0; k<64; k++) p.message[k] = 0;

        // Copy message safely (max 63 chars)
        const char * src = message;
        int i = 0;
        while (*src != '\0' && i < 63) {
            p.message[i] = *src;
            src++;
            i++;
        }
        p.message[i] = '\0';

        if (replace_index < 0) problem_count++;
        return problem_count >= MAX_LINT_PROBLEMS;
    }
    
    // Override logBytecode to do nothing during linting
    // void logBytecode() { } // Actually not virtual in base class yet, but compiler methods call defaultCompiler wrapper? No, compiler methods inside 'build' don't call logBytecode.

    // Override compileAssembly to return error if any problems found
    bool compileAssembly(bool debug = true, bool lintMode = false) {
        bool result = PLCASMCompiler::compileAssembly(debug, lintMode);
        if (problem_count > 0) return true;
        return result;
    }
};

// Global Linter Instance
// Singleton wrapper to prevent re-initialization
PLCASMLinter& getDefaultLinter() {
    static PLCASMLinter instance;
    return instance;
}
#define defaultLinter getDefaultLinter()

// PLCASMLinter defaultLinter;

// ### WASM Exports for Linter ###

extern "C" {
    
    // Sets the assembly string for the linter instance
    WASM_EXPORT void lint_set_assembly(char* s) {
        defaultLinter.set_assembly_string(s);
    }

    // Runs the linter (compilation with debug=false, using the linter instance)
    WASM_EXPORT void lint_run() {
        defaultLinter.clearArray();
        defaultLinter.compileAssembly(false, true); // false = no debug printing, true = lintMode
    }
    
    // Reads assembly from the stream buffer into the linter
    WASM_EXPORT void lint_load_assembly() {
        defaultLinter.loadAssembly();
    }

    WASM_EXPORT int lint_get_problem_count() {
        return defaultLinter.problem_count;
    }

    WASM_EXPORT LinterProblem* lint_get_problems_pointer() {
        return defaultLinter.problems;
    }

    // Deprecated or simplified wrapper if needed
    // WASM_EXPORT LinterProblem* getLinterProblems(int* count) { ... }
}

#endif // __WASM__
