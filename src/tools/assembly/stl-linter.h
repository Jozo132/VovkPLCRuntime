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
#define STL_MAX_SYMBOLS 128

// Simple symbol structure for STL linting
struct STLSymbol {
    char name[64];
    char type[16];
    u32 address;
    u8 bit;
    bool is_bit;
    int line;
    int column;
    u8 type_size;
};

// ### STL Linter Class Definition ###

class STLLinter : public STLCompiler {
public:
    LinterProblem problems[STL_MAX_LINT_PROBLEMS];
    int problem_count = 0;
    
    // Symbol table
    STLSymbol symbols[STL_MAX_SYMBOLS];
    int symbol_count = 0;
    
    // Track token info for error reporting
    int token_start_column = 1;
    int token_length = 0;
    
    // Track last operand position for address validation errors
    int last_operand_column = 1;
    int last_operand_length = 0;
    const char* last_operand_ptr = nullptr;  // Pointer into source for token text

    STLLinter() : STLCompiler() {
        problem_count = 0;
        symbol_count = 0;
    }

    void clearProblems() {
        problem_count = 0;
        symbol_count = 0;
        has_error = false;
        error_message[0] = '\0';
        error_line = 0;
        error_column = 0;
    }
    
    // Override readIdentifier to track operand position for error reporting
    bool readIdentifier(char* buf, int maxLen) override {
        last_operand_column = current_column;
        last_operand_ptr = stl_source + pos;  // Capture pointer into source
        bool result = STLCompiler::readIdentifier(buf, maxLen);
        last_operand_length = current_column - last_operand_column;
        return result;
    }
    
    // ============ Symbol Parsing ============
    
    // Get type size in bytes
    u8 getTypeSize(const char* type) {
        if (strEq(type, "i8") || strEq(type, "u8") || strEq(type, "byte")) return 1;
        if (strEq(type, "i16") || strEq(type, "u16")) return 2;
        if (strEq(type, "i32") || strEq(type, "u32") || strEq(type, "f32")) return 4;
        if (strEq(type, "i64") || strEq(type, "u64") || strEq(type, "f64")) return 8;
        if (strEq(type, "bit") || strEq(type, "bool")) return 0; // bit type has no size
        if (strEq(type, "ptr") || strEq(type, "pointer")) return 4;
        return 0; // unknown type
    }
    
    // Check if type is valid
    bool isValidType(const char* type) {
        const char* valid_types[] = { "i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", 
                                       "f32", "f64", "bool", "bit", "byte", "ptr", "pointer" };
        for (int i = 0; i < 15; i++) {
            if (strEq(type, valid_types[i])) return true;
        }
        return false;
    }
    
    // Check if symbol name is valid
    bool isValidSymbolName(const char* name) {
        if (!name || name[0] == '\0') return false;
        char first = name[0];
        if (!((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z') || first == '_')) {
            return false;
        }
        for (int i = 1; name[i]; i++) {
            char c = name[i];
            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
                  (c >= '0' && c <= '9') || c == '_')) {
                return false;
            }
        }
        return true;
    }
    
    // Find symbol by name
    STLSymbol* findSymbol(const char* name) {
        for (int i = 0; i < symbol_count; i++) {
            if (strEq(symbols[i].name, name)) {
                return &symbols[i];
            }
        }
        return nullptr;
    }
    
    // Parse symbol address (e.g., "M0", "X0.1", "130")
    bool parseSymbolAddress(const char* addr_str, u32& address, u8& bit, bool& is_bit) {
        is_bit = false;
        bit = 255;
        
        if (!addr_str || addr_str[0] == '\0') return false;
        
        // Find dot position for bit notation
        int dot_pos = -1;
        int len = 0;
        for (int i = 0; addr_str[i]; i++) {
            if (addr_str[i] == '.') dot_pos = i;
            len++;
        }
        
        char prefix = addr_str[0];
        if (prefix >= 'a' && prefix <= 'z') prefix -= 32; // Uppercase
        
        u32 base_offset = 0;
        int num_start = 0;
        
        // Determine memory area and offset
        switch (prefix) {
            case 'K': case 'C': base_offset = plcasm_control_offset; num_start = 1; break;
            case 'I': case 'X': base_offset = plcasm_input_offset; num_start = 1; break;
            case 'Q': case 'Y': base_offset = plcasm_output_offset; num_start = 1; break;
            case 'S': base_offset = plcasm_system_offset; num_start = 1; break;
            case 'M': base_offset = plcasm_marker_offset; num_start = 1; break;
            case 'T': base_offset = plcasm_timer_offset; num_start = 1; break;
            default:
                if (prefix >= '0' && prefix <= '9') {
                    num_start = 0;
                    base_offset = 0;
                } else {
                    return false;
                }
        }
        
        // Parse numeric part
        int byte_addr = 0;
        int end_pos = dot_pos >= 0 ? dot_pos : len;
        
        for (int i = num_start; i < end_pos; i++) {
            char c = addr_str[i];
            if (c < '0' || c > '9') return false;
            byte_addr = byte_addr * 10 + (c - '0');
        }
        
        address = base_offset + byte_addr;
        
        // Parse bit position if present
        if (dot_pos >= 0 && dot_pos + 1 < len) {
            is_bit = true;
            char bit_char = addr_str[dot_pos + 1];
            if (bit_char < '0' || bit_char > '7') return false;
            bit = bit_char - '0';
        }
        
        return true;
    }
    
    // Parse a symbol definition line: $$ name | type | address [| comment]
    void parseSymbolDefinition() {
        int start_line = current_line;
        int start_col = current_column;
        
        // Skip "$$"
        advance(); advance();
        skipWhitespace();
        
        // Read symbol name
        char name[64] = {0};
        int name_col = current_column;
        int ni = 0;
        while (ni < 63 && isAlphaNum(peek())) {
            name[ni++] = advance();
        }
        name[ni] = '\0';
        
        if (name[0] == '\0') {
            token_start_column = name_col;
            token_length = 1;
            setError("Expected symbol name after '$$'");
            skipToEndOfLine();
            return;
        }
        
        if (!isValidSymbolName(name)) {
            token_start_column = name_col;
            token_length = ni;
            setError("Invalid symbol name");
            skipToEndOfLine();
            return;
        }
        
        // Check for duplicate
        if (findSymbol(name)) {
            token_start_column = name_col;
            token_length = ni;
            setError("Duplicate symbol name");
            skipToEndOfLine();
            return;
        }
        
        skipWhitespace();
        
        // Expect '|'
        if (peek() != '|') {
            token_start_column = current_column;
            token_length = 1;
            setError("Expected '|' after symbol name");
            skipToEndOfLine();
            return;
        }
        advance();
        skipWhitespace();
        
        // Read type
        char type[16] = {0};
        int type_col = current_column;
        int ti = 0;
        while (ti < 15 && isAlphaNum(peek())) {
            type[ti++] = advance();
        }
        type[ti] = '\0';
        
        if (!isValidType(type)) {
            token_start_column = type_col;
            token_length = ti > 0 ? ti : 1;
            setError("Invalid symbol type");
            skipToEndOfLine();
            return;
        }
        
        skipWhitespace();
        
        // Expect '|'
        if (peek() != '|') {
            token_start_column = current_column;
            token_length = 1;
            setError("Expected '|' after type");
            skipToEndOfLine();
            return;
        }
        advance();
        skipWhitespace();
        
        // Read address
        char addr[32] = {0};
        int addr_col = current_column;
        int ai = 0;
        while (ai < 31 && (isAlphaNum(peek()) || peek() == '.')) {
            addr[ai++] = advance();
        }
        addr[ai] = '\0';
        
        u32 address;
        u8 bit;
        bool is_bit;
        
        if (!parseSymbolAddress(addr, address, bit, is_bit)) {
            token_start_column = addr_col;
            token_length = ai > 0 ? ai : 1;
            setError("Invalid symbol address");
            skipToEndOfLine();
            return;
        }
        
        // Validate bit/type consistency
        bool is_bit_type = strEq(type, "bit") || strEq(type, "bool");
        if (is_bit_type && !is_bit) {
            token_start_column = addr_col;
            token_length = ai;
            setError("bit/bool type requires bit address (e.g., X0.1)");
            skipToEndOfLine();
            return;
        }
        if (!is_bit_type && is_bit) {
            token_start_column = addr_col;
            token_length = ai;
            setError("Non-bit type cannot use bit address");
            skipToEndOfLine();
            return;
        }
        
        // Add symbol if we have space
        if (symbol_count >= STL_MAX_SYMBOLS) {
            addWarning("Maximum symbols reached", start_line, start_col);
            skipToEndOfLine();
            return;
        }
        
        STLSymbol& sym = symbols[symbol_count++];
        // Clear name and type arrays first
        for (int i = 0; i < 64; i++) sym.name[i] = '\0';
        for (int i = 0; i < 16; i++) sym.type[i] = '\0';
        // Copy name
        for (int i = 0; i < 63 && name[i]; i++) sym.name[i] = name[i];
        // Copy type
        for (int i = 0; i < 15 && type[i]; i++) sym.type[i] = type[i];
        sym.address = address;
        sym.bit = bit;
        sym.is_bit = is_bit;
        sym.line = start_line;
        sym.column = start_col;
        sym.type_size = getTypeSize(type);
        
        // Skip rest of line (comment)
        skipToEndOfLine();
    }
    
    // Check if an operand is a valid address/symbol
    bool validateOperand(const char* operand, int col, int len) {
        if (!operand || operand[0] == '\0') return false;
        
        char first = operand[0];
        if (first >= 'a' && first <= 'z') first -= 32;
        
        // Check if it's a known symbol
        STLSymbol* sym = findSymbol(operand);
        if (sym) return true;
        
        // Check if it's a valid address format
        // Valid prefixes: I, Q, M, S, X, Y, K, C, T, P (pulse), # (immediate)
        switch (first) {
            case 'I': case 'Q': case 'M': case 'S': case 'X': case 'Y':
            case 'K': case 'C': case 'T': case '#':
                return true;
            case 'P':
                // P_On, P_Off, P_1s etc.
                if (operand[1] == '_') return true;
                return false;
            default:
                // Could be a plain number or unknown identifier
                if (first >= '0' && first <= '9') return true;
                // Unknown identifier - report warning
                token_start_column = col;
                token_length = len;
                addWarning("Unknown identifier - may be undefined symbol", current_line, col, len);
                return true; // Don't block compilation
        }
    }
    
    // Validate address format (e.g., I0.0, Q1, M10.5, T#5s, #123)
    bool isValidAddressFormat(const char* addr) {
        if (!addr || addr[0] == '\0') return false;
        
        char first = addr[0];
        if (first >= 'a' && first <= 'z') first -= 32; // Uppercase
        
        // Special formats that are passed through
        if (first == '#') return true;  // Immediate value
        if (first == 'P' && addr[1] == '_') return true;  // P_On, P_Off, etc.
        if (first == 'T' && addr[1] == '#') return true;  // T#5s duration
        
        // Standard address prefixes
        bool hasPrefix = false;
        int i = 0;
        switch (first) {
            case 'I': case 'Q': case 'M': case 'S': case 'X': case 'Y':
            case 'K': case 'C': case 'T':
                hasPrefix = true;
                i = 1;
                break;
            default:
                // Could be a plain number or a symbol name
                if (first >= '0' && first <= '9') {
                    i = 0;
                } else if ((first >= 'A' && first <= 'Z') || first == '_') {
                    // Could be a symbol name - let it pass for now
                    return true;
                } else {
                    return false;
                }
        }
        
        // After prefix, expect digits
        bool hasDigits = false;
        while (addr[i] >= '0' && addr[i] <= '9') {
            hasDigits = true;
            i++;
        }
        
        if (!hasDigits && hasPrefix) return false;  // Must have at least one digit after prefix
        
        // Optional: dot followed by bit number (0-7)
        if (addr[i] == '.') {
            i++;
            if (addr[i] < '0' || addr[i] > '7') return false;  // Invalid bit number
            i++;
        }
        
        // Should be at end of string now
        if (addr[i] != '\0') return false;  // Extra characters after valid address
        
        return true;
    }
    
    // Override convertAddress to resolve symbols first and validate addresses
    void convertAddress(const char* stlAddr, char* plcasmAddr) override {
        // Check if it's a symbol name
        STLSymbol* sym = findSymbol(stlAddr);
        if (sym) {
            // Resolve to actual address
            // Build address string from symbol's stored address
            u32 addr = sym->address;
            u8 bit = sym->bit;
            bool is_bit = sym->is_bit;
            
            // Determine the memory prefix from the address
            char prefix = 'M'; // default
            u32 offset = addr;
            
            if (addr >= plcasm_timer_offset) {
                prefix = 'T';
                offset = (addr - plcasm_timer_offset) / 9; // Timer struct is 9 bytes
            } else if (addr >= plcasm_marker_offset) {
                prefix = 'M';
                offset = addr - plcasm_marker_offset;
            } else if (addr >= plcasm_system_offset) {
                prefix = 'S';
                offset = addr - plcasm_system_offset;
            } else if (addr >= plcasm_output_offset) {
                prefix = 'Y';
                offset = addr - plcasm_output_offset;
            } else if (addr >= plcasm_input_offset) {
                prefix = 'X';
                offset = addr - plcasm_input_offset;
            } else if (addr >= plcasm_control_offset) {
                prefix = 'K';
                offset = addr - plcasm_control_offset;
            }
            
            int j = 0;
            plcasmAddr[j++] = prefix;
            
            // Write offset as decimal
            char numBuf[16];
            int ni = 0;
            u32 tmp = offset;
            if (tmp == 0) {
                numBuf[ni++] = '0';
            } else {
                while (tmp > 0) {
                    numBuf[ni++] = '0' + (tmp % 10);
                    tmp /= 10;
                }
            }
            // Reverse
            for (int k = ni - 1; k >= 0; k--) {
                plcasmAddr[j++] = numBuf[k];
            }
            
            // Add bit if needed
            if (is_bit) {
                plcasmAddr[j++] = '.';
                plcasmAddr[j++] = '0' + bit;
            }
            
            plcasmAddr[j] = '\0';
            return;
        }
        
        // Validate address format before converting
        if (!isValidAddressFormat(stlAddr)) {
            // Report error for invalid address at the operand position
            int len = 0;
            while (stlAddr[len]) len++;
            token_start_column = last_operand_column;
            token_length = last_operand_length > 0 ? last_operand_length : len;
            setError("Invalid address format");
            // Still convert to avoid null output
            STLCompiler::convertAddress(stlAddr, plcasmAddr);
            return;
        }
        
        // Not a symbol, use base class implementation
        STLCompiler::convertAddress(stlAddr, plcasmAddr);
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
        
        // Set token_text pointer to point into the source at the last operand position
        p.token_text = (char*)last_operand_ptr;

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
        p.token_text = (char*)last_operand_ptr;

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
        p.token_text = (char*)last_operand_ptr;

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
            
            // Symbol definition: $$ name | type | address [| comment]
            if (c == '$' && peek(1) == '$') {
                parseSymbolDefinition();
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
