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
    int type;      // 0=Info, 1=Warning, 2=Error
    int line;      // Line number (1-based)
    int column;    // Column number (1-based)
    int length;    // Length of the token causing the issue
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
        // Stop if we reached the limit
        if (problem_count >= MAX_LINT_PROBLEMS) return true;

        // Check for duplicates (same line, column, and message) before adding
        for (int j = 0; j < problem_count; j++) {
             if (problems[j].line == token.line && problems[j].column == token.column) {
                 const char* s1 = problems[j].message;
                 const char* s2 = message;
                 bool same = true;
                 // Compare messages (up to 63 chars as stored)
                 for(int k=0; k<63; k++) {
                     if (s1[k] != s2[k]) { same = false; break; }
                     if (s1[k] == 0) break; // End of string
                 }
                 if (same) return false; // Duplicate: Don't add, continue building
             }
        }

        LinterProblem& p = problems[problem_count];
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

        problem_count++;
        return problem_count >= MAX_LINT_PROBLEMS;
    }
    
    // Override logBytecode to do nothing during linting
    // void logBytecode() { } // Actually not virtual in base class yet, but compiler methods call defaultCompiler wrapper? No, compiler methods inside 'build' don't call logBytecode.
};

// Global Linter Instance
PLCASMLinter defaultLinter;

// ### WASM Exports for Linter ###

extern "C" {
    
    // Sets the assembly string for the linter instance
    WASM_EXPORT void lint_set_assembly(char* s) {
        defaultLinter.set_assembly_string(s);
    }

    // Runs the linter (compilation with debug=false, using the linter instance)
    WASM_EXPORT void lint_run() {
        defaultLinter.clearArray();
        defaultLinter.compileAssembly(false, true); // false = no debug printing (though reportError is overridden anyway)
    }

    // Returns a pointer to the problems array. JS reads 'count' from the int pointer argument.
    WASM_EXPORT LinterProblem* getLinterProblems(int* count) {
        *count = defaultLinter.problem_count;
        return defaultLinter.problems;
    }
}

#endif // __WASM__
