// stl-compiler.h - Siemens STL to PLCASM Transpiler
// Converts Siemens-style Statement List (STL) code to VovkPLCRuntime PLCASM assembly
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

#include "plcasm-compiler.h"

// STL Compiler - Transpiles Siemens STL to PLCASM
// 
// Supported Siemens STL Instructions:
// 
// Bit Logic:
//   A, AN, O, ON, X, XN   - AND, AND NOT, OR, OR NOT, XOR, XOR NOT
//   =                     - Assign RLO to bit
//   S, R                  - Set/Reset bit if RLO=1
//   SET, CLR, NOT         - Set RLO=1, Clear RLO=0, Negate RLO
//   A(, O(, X(, )         - Nesting (parentheses)
//
// Edge Detection:
//   FP, FN                - Positive/Negative edge detect
//
// Jumps:
//   JU, JC, JCN           - Jump unconditional, Jump if RLO=1, Jump if RLO=0
//
// Calls/Returns:
//   CALL                  - Call subroutine
//   BE, BEC, BEU          - Block End, Block End Conditional
//
// Timers:
//   TON, TOF, TP          - IEC timers (inline syntax)
//
// Counters:
//   CTU, CTD              - Counter Up/Down with syntax: CTU C0, #10, X0.1
//
// Load/Transfer:
//   L, T                  - Load to accumulator, Transfer from accumulator
//
// Math:
//   +I, -I, *I, /I, MOD   - Integer math
//   NEG, ABS              - Negate, Absolute value
//
// Compare:
//   ==I, <>I, >I, >=I, <I, <=I  - Integer comparisons

#define STL_MAX_OUTPUT_SIZE 32768
#define STL_MAX_NESTING_DEPTH 16
#define STL_MAX_LABELS 64

class STLCompiler {
public:
    // Input STL source
    char* stl_source = nullptr;
    int stl_length = 0;

    // Output PLCASM
    char output[STL_MAX_OUTPUT_SIZE];
    int output_length = 0;

    // Error handling
    char error_message[256];
    int error_line = 0;
    int error_column = 0;
    bool has_error = false;

    // Parsing state
    int current_line = 1;
    int current_column = 1;
    int pos = 0;

    // RLO tracking - tracks if we're at the start of a network (first boolean op)
    bool network_has_rlo = false;

    // Nesting stack for A(, O(, X( operations
    char nesting_ops[STL_MAX_NESTING_DEPTH]; // 'A', 'O', 'X'
    bool nesting_had_rlo[STL_MAX_NESTING_DEPTH]; // Track if outer had RLO before nesting
    int nesting_depth = 0;

    // Label counter for generated labels
    int label_counter = 0;

    STLCompiler() {
        reset();
    }

    void reset() {
        stl_source = nullptr;
        stl_length = 0;
        output_length = 0;
        output[0] = '\0';
        error_message[0] = '\0';
        error_line = 0;
        error_column = 0;
        has_error = false;
        current_line = 1;
        current_column = 1;
        pos = 0;
        network_has_rlo = false;
        nesting_depth = 0;
        label_counter = 0;
    }

    void setSource(char* source, int length) {
        reset();
        stl_source = source;
        stl_length = length;
    }

    // ============ Output helpers ============

    void emit(const char* str) {
        while (*str && output_length < STL_MAX_OUTPUT_SIZE - 1) {
            output[output_length++] = *str++;
        }
        output[output_length] = '\0';
    }

    void emitLine(const char* str) {
        emit(str);
        emit("\n");
    }

    void emitIndent() {
        emit("    ");
    }

    // Emit with format (simple implementation)
    void emitInt(int value) {
        char buf[16];
        int i = 0;
        bool negative = value < 0;
        if (negative) value = -value;
        if (value == 0) {
            buf[i++] = '0';
        } else {
            while (value > 0) {
                buf[i++] = '0' + (value % 10);
                value /= 10;
            }
        }
        if (negative) buf[i++] = '-';
        // Reverse
        for (int j = i - 1; j >= 0; j--) {
            char c = buf[j];
            if (output_length < STL_MAX_OUTPUT_SIZE - 1) {
                output[output_length++] = c;
            }
        }
        output[output_length] = '\0';
    }

    // Generate unique label
    void emitUniqueLabel(const char* prefix) {
        emit("__");
        emit(prefix);
        emit("_");
        emitInt(label_counter++);
    }

    // ============ Error handling ============

    // Virtual so STLLinter can override to capture multiple errors
    virtual void setError(const char* msg) {
        if (has_error) return; // Keep first error
        has_error = true;
        error_line = current_line;
        error_column = current_column;
        int i = 0;
        while (msg[i] && i < 255) {
            error_message[i] = msg[i];
            i++;
        }
        error_message[i] = '\0';
    }
    
    // Virtual destructor for proper cleanup in derived classes
    virtual ~STLCompiler() = default;

    // ============ Lexer helpers ============

    char peek(int offset = 0) {
        int p = pos + offset;
        if (p >= stl_length) return '\0';
        return stl_source[p];
    }

    char advance() {
        if (pos >= stl_length) return '\0';
        char c = stl_source[pos++];
        if (c == '\n') {
            current_line++;
            current_column = 1;
        } else {
            current_column++;
        }
        return c;
    }

    void skipWhitespace() {
        while (pos < stl_length) {
            char c = peek();
            if (c == ' ' || c == '\t' || c == '\r') {
                advance();
            } else {
                break;
            }
        }
    }

    void skipToEndOfLine() {
        while (pos < stl_length && peek() != '\n') {
            advance();
        }
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

    // Read an identifier/keyword
    virtual bool readIdentifier(char* buf, int maxLen) {
        int i = 0;
        while (i < maxLen - 1) {
            char c = peek();
            if (isAlphaNum(c) || c == '.' || c == '#' || c == '_') {
                buf[i++] = advance();
            } else {
                break;
            }
        }
        buf[i] = '\0';
        return i > 0;
    }

    // Read a number (including negative)
    bool readNumber(char* buf, int maxLen) {
        int i = 0;
        if (peek() == '-' || peek() == '+') {
            buf[i++] = advance();
        }
        while (i < maxLen - 1 && isDigit(peek())) {
            buf[i++] = advance();
        }
        buf[i] = '\0';
        return i > 0 && isDigit(buf[i-1]);
    }

    // Parse integer from string
    int parseInt(const char* str) {
        int result = 0;
        bool negative = false;
        int i = 0;
        if (str[0] == '-') {
            negative = true;
            i = 1;
        } else if (str[0] == '+') {
            i = 1;
        }
        while (str[i]) {
            if (isDigit(str[i])) {
                result = result * 10 + (str[i] - '0');
            }
            i++;
        }
        return negative ? -result : result;
    }

    // String comparison helpers
    bool strEq(const char* a, const char* b) {
        while (*a && *b) {
            // Case-insensitive compare
            char ca = *a >= 'a' && *a <= 'z' ? *a - 32 : *a;
            char cb = *b >= 'a' && *b <= 'z' ? *b - 32 : *b;
            if (ca != cb) return false;
            a++; b++;
        }
        return *a == *b;
    }

    bool startsWith(const char* str, const char* prefix) {
        while (*prefix) {
            char ca = *str >= 'a' && *str <= 'z' ? *str - 32 : *str;
            char cb = *prefix >= 'a' && *prefix <= 'z' ? *prefix - 32 : *prefix;
            if (ca != cb) return false;
            str++; prefix++;
        }
        return true;
    }

    // ============ Address conversion ============

    // Convert STL address to PLCASM address
    // STL uses: I0.0 (input), Q0.0 (output), M0.0 (marker), etc.
    // PLCASM uses: X0.0 (input), Y0.0 (output), M0.0 (marker), S0 (system), C0 (control)
    virtual void convertAddress(const char* stlAddr, char* plcasmAddr) {
        int i = 0, j = 0;
        
        // Map first character
        char first = stlAddr[0];
        if (first >= 'a' && first <= 'z') first -= 32; // Uppercase
        
        switch (first) {
            case 'I': plcasmAddr[j++] = 'X'; i++; break;  // Input
            case 'Q': plcasmAddr[j++] = 'Y'; i++; break;  // Output
            case 'M': plcasmAddr[j++] = 'M'; i++; break;  // Marker
            case 'S': plcasmAddr[j++] = 'S'; i++; break;  // System
            case 'C': plcasmAddr[j++] = 'C'; i++; break;  // Counter -> C (with struct size multiplier)
            case 'T': 
                // T could be timer or T# duration
                if (stlAddr[1] == '#') {
                    // Duration format T#... - copy as-is
                    while (stlAddr[i]) plcasmAddr[j++] = stlAddr[i++];
                    plcasmAddr[j] = '\0';
                    return;
                }
                plcasmAddr[j++] = 'T'; i++; break;  // Timer -> T (with struct size multiplier)
            case 'X': plcasmAddr[j++] = 'X'; i++; break;  // Already PLCASM format
            case 'Y': plcasmAddr[j++] = 'Y'; i++; break;
            case 'K': plcasmAddr[j++] = 'K'; i++; break;  // K maps to controls
            case 'P':
                // Special: P_1s or similar pulse macro
                while (stlAddr[i]) plcasmAddr[j++] = stlAddr[i++];
                plcasmAddr[j] = '\0';
                return;
            case '#':
                // Immediate value #123
                while (stlAddr[i]) plcasmAddr[j++] = stlAddr[i++];
                plcasmAddr[j] = '\0';
                return;
            default:
                // Unknown - copy as-is
                while (stlAddr[i]) plcasmAddr[j++] = stlAddr[i++];
                plcasmAddr[j] = '\0';
                return;
        }
        
        // Copy rest of address
        while (stlAddr[i]) {
            plcasmAddr[j++] = stlAddr[i++];
        }
        plcasmAddr[j] = '\0';
    }

    // ============ Instruction handlers ============

    // Emit boolean read with proper first-op handling
    void emitBoolRead(const char* addr, bool negate) {
        char plcAddr[64];
        convertAddress(addr, plcAddr);
        
        emit("u8.readBit ");
        emitLine(plcAddr);
        
        if (negate) {
            emitLine("u8.not");
        }
    }

    // Emit boolean combine operation
    void emitBoolCombine(char op) {
        switch (op) {
            case 'A': emitLine("u8.and"); break;
            case 'O': emitLine("u8.or"); break;
            case 'X': emitLine("u8.xor"); break;
        }
    }
    
    // Lookahead to check if the next non-comment instruction is an output (=, S, R)
    // This allows us to emit u8.copy only when needed for multiple coil outputs
    bool peekNextIsOutput() {
        int savedPos = pos;
        int savedLine = current_line;
        int savedCol = current_column;
        
        while (pos < stl_length) {
            // Skip whitespace
            while (pos < stl_length && (stl_source[pos] == ' ' || stl_source[pos] == '\t' || 
                   stl_source[pos] == '\r' || stl_source[pos] == '\n')) {
                if (stl_source[pos] == '\n') { current_line++; current_column = 1; }
                else { current_column++; }
                pos++;
            }
            
            if (pos >= stl_length) break;
            
            char c = stl_source[pos];
            
            // Skip comments
            if (c == '/' && pos + 1 < stl_length && stl_source[pos + 1] == '/') {
                while (pos < stl_length && stl_source[pos] != '\n') pos++;
                continue;
            }
            
            // Check for = (assign)
            if (c == '=') {
                // Make sure it's not ==I comparison
                if (pos + 1 < stl_length && stl_source[pos + 1] == '=') {
                    // It's ==I, not an output
                    break;
                }
                // It's an assign output
                pos = savedPos; current_line = savedLine; current_column = savedCol;
                return true;
            }
            
            // Check for S or R (set/reset) - must be followed by whitespace and operand
            if (c == 'S' || c == 's' || c == 'R' || c == 'r') {
                // Check it's a standalone S/R, not SET, S1, etc.
                if (pos + 1 < stl_length) {
                    char next = stl_source[pos + 1];
                    if (next == ' ' || next == '\t') {
                        // It's S or R instruction (output)
                        pos = savedPos; current_line = savedLine; current_column = savedCol;
                        return true;
                    }
                }
            }
            
            // Any other instruction - not an output
            break;
        }
        
        pos = savedPos; current_line = savedLine; current_column = savedCol;
        return false;
    }

    // Handle A/AN/O/ON/X/XN instructions
    void handleBitLogic(char op, bool negate, const char* operand) {
        emitBoolRead(operand, negate);
        
        if (network_has_rlo) {
            // Combine with existing RLO
            emitBoolCombine(op);
        }
        
        network_has_rlo = true;
    }

    // Handle nesting open: A(, O(, X(
    void handleNestingOpen(char op) {
        if (nesting_depth >= STL_MAX_NESTING_DEPTH) {
            setError("Maximum nesting depth exceeded");
            return;
        }
        // Save whether there was an outer RLO before starting this nesting
        nesting_had_rlo[nesting_depth] = network_has_rlo;
        nesting_ops[nesting_depth] = op;
        nesting_depth++;
        // Reset RLO for the inner expression
        network_has_rlo = false;
    }

    // Handle nesting close: )
    void handleNestingClose() {
        if (nesting_depth == 0) {
            setError("Unmatched closing parenthesis");
            return;
        }
        nesting_depth--;
        char op = nesting_ops[nesting_depth];
        bool had_outer_rlo = nesting_had_rlo[nesting_depth];
        
        // Only combine if there was an outer RLO to combine with
        if (had_outer_rlo) {
            emitBoolCombine(op);
        }
        // After closing, we have an RLO (the result of the nested expression)
        network_has_rlo = true;
    }

    // Handle = (assign)
    void handleAssign(const char* operand) {
        char plcAddr[64];
        convertAddress(operand, plcAddr);
        // Only duplicate RLO if another output instruction follows
        if (peekNextIsOutput()) {
            emit("u8.copy\n");
        }
        emit("u8.writeBit ");
        emitLine(plcAddr);
        network_has_rlo = false; // Consumed (or preserved if we duped)
    }

    // Handle S (set if RLO=1)
    void handleSet(const char* operand) {
        char plcAddr[64];
        convertAddress(operand, plcAddr);
        
        int savedCounter = label_counter++;
        
        // Only duplicate RLO if another output instruction follows
        if (peekNextIsOutput()) {
            emit("u8.copy\n");
        }
        // Use relative jump for position-independent bytecode
        emit("jmp_if_not_rel __skip_set_");
        emitInt(savedCounter);
        emit("\n");
        
        emit("u8.writeBitOn ");
        emitLine(plcAddr);
        
        emit("__skip_set_");
        emitInt(savedCounter);
        emit(":\n");
        
        network_has_rlo = false;
    }

    // Handle R (reset if RLO=1)
    void handleReset(const char* operand) {
        char plcAddr[64];
        convertAddress(operand, plcAddr);
        
        int savedCounter = label_counter++;
        
        // Only duplicate RLO if another output instruction follows
        if (peekNextIsOutput()) {
            emit("u8.copy\n");
        }
        // Use relative jump for position-independent bytecode
        emit("jmp_if_not_rel __skip_reset_");
        emitInt(savedCounter);
        emit("\n");
        
        emit("u8.writeBitOff ");
        emitLine(plcAddr);
        
        emit("__skip_reset_");
        emitInt(savedCounter);
        emit(":\n");
        
        network_has_rlo = false;
    }

    // Handle FP (positive edge)
    void handleFP(const char* edgeBit) {
        char plcAddr[64];
        convertAddress(edgeBit, plcAddr);
        emit("u8.du ");
        emitLine(plcAddr);
    }

    // Handle FN (negative edge)
    void handleFN(const char* edgeBit) {
        char plcAddr[64];
        convertAddress(edgeBit, plcAddr);
        emit("u8.dd ");
        emitLine(plcAddr);
    }

    // Handle FX (any edge / change detect) - VovkPLCRuntime extension
    void handleFX(const char* edgeBit) {
        char plcAddr[64];
        convertAddress(edgeBit, plcAddr);
        emit("u8.dc ");
        emitLine(plcAddr);
    }

    // Handle TON/TOF/TP timers
    void handleTimer(const char* type, const char* timerAddr, const char* preset) {
        char plcTimer[64], plcPreset[64];
        convertAddress(timerAddr, plcTimer);
        convertAddress(preset, plcPreset);
        
        emit(type);
        emit(" ");
        emit(plcTimer);
        emit(" ");
        emitLine(plcPreset);
    }

    // Handle CTU counter: CTU C0, #10, X0.1
    void handleCTU(const char* counterAddr, const char* preset, const char* resetBit) {
        char plcCounter[64], plcPreset[64], plcReset[64];
        convertAddress(counterAddr, plcCounter);
        convertAddress(preset, plcPreset);
        convertAddress(resetBit, plcReset);
        
        // Stack needs [CU, R] with R on top
        // RLO is already CU, now push R
        emit("u8.readBit ");
        emitLine(plcReset);
        
        emit("ctu ");
        emit(plcCounter);
        emit(" ");
        emitLine(plcPreset);
    }

    // Handle CTD counter: CTD C1, #5, X0.3
    void handleCTD(const char* counterAddr, const char* preset, const char* loadBit) {
        char plcCounter[64], plcPreset[64], plcLoad[64];
        convertAddress(counterAddr, plcCounter);
        convertAddress(preset, plcPreset);
        convertAddress(loadBit, plcLoad);
        
        // Stack needs [CD, LD] with LD on top
        // RLO is already CD, now push LD
        emit("u8.readBit ");
        emitLine(plcLoad);
        
        emit("ctd ");
        emit(plcCounter);
        emit(" ");
        emitLine(plcPreset);
    }

    // Handle L (load)
    void handleLoad(const char* operand) {
        char plcAddr[64];
        convertAddress(operand, plcAddr);
        
        if (plcAddr[0] == '#') {
            // Immediate value
            emit("u8.const ");
            emitLine(plcAddr + 1); // Skip #
        } else {
            emit("u8.load_from ");
            emitLine(plcAddr);
        }
    }

    // Handle T (transfer)
    void handleTransfer(const char* operand) {
        char plcAddr[64];
        convertAddress(operand, plcAddr);
        emit("u8.move_to ");
        emitLine(plcAddr);
    }

    // Handle math operations
    void handleMath(const char* op) {
        if (strEq(op, "+I")) emitLine("u8.add");
        else if (strEq(op, "-I")) emitLine("u8.sub");
        else if (strEq(op, "*I")) emitLine("u8.mul");
        else if (strEq(op, "/I")) emitLine("u8.div");
        else if (strEq(op, "MOD")) emitLine("u8.mod");
        else if (strEq(op, "NEG")) emitLine("u8.neg");
        else if (strEq(op, "ABS")) emitLine("u8.abs");
    }

    // Handle compare operations
    void handleCompare(const char* op) {
        if (strEq(op, "==I")) emitLine("u8.cmp_eq");
        else if (strEq(op, "<>I")) emitLine("u8.cmp_neq");
        else if (strEq(op, ">I")) emitLine("u8.cmp_gt");
        else if (strEq(op, ">=I")) emitLine("u8.cmp_gte");
        else if (strEq(op, "<I")) emitLine("u8.cmp_lt");
        else if (strEq(op, "<=I")) emitLine("u8.cmp_lte");
        network_has_rlo = true;
    }

    // Handle jumps
    void handleJump(const char* type, const char* label) {
        if (strEq(type, "JU")) {
            emit("jmp ");
        } else if (strEq(type, "JC")) {
            emit("jmp_if ");
            network_has_rlo = false;
        } else if (strEq(type, "JCN")) {
            emit("jmp_if_not ");
            network_has_rlo = false;
        }
        emitLine(label);
    }

    // Handle CALL
    void handleCall(const char* label) {
        emit("call ");
        emitLine(label);
    }

    // Handle returns
    void handleReturn(const char* type) {
        if (strEq(type, "BE") || strEq(type, "RET")) {
            emitLine("ret");
        } else if (strEq(type, "BEC")) {
            emitLine("ret_if");
            network_has_rlo = false;
        } else if (strEq(type, "BEU")) {
            emitLine("ret_if_not");
            network_has_rlo = false;
        }
    }

    // ============ Main parser ============

    bool compile() {
        if (!stl_source || stl_length == 0) {
            setError("No source code provided");
            return false;
        }

        // Add header comment
        emitLine("// Generated from STL by VovkPLCRuntime STL Compiler");
        emitLine("");

        while (pos < stl_length && !has_error) {
            skipWhitespace();
            
            char c = peek();
            
            // End of input
            if (c == '\0') break;
            
            // Newline - just advance
            if (c == '\n') {
                advance();
                continue;
            }
            
            // Comment (// or (* *))
            if (c == '/' && peek(1) == '/') {
                // Copy comment to output
                emit("//");
                advance(); advance();
                while (peek() != '\n' && peek() != '\0') {
                    char cc = advance();
                    if (output_length < STL_MAX_OUTPUT_SIZE - 1) {
                        output[output_length++] = cc;
                    }
                }
                output[output_length] = '\0';
                emit("\n");
                continue;
            }
            
            // Label definition: LABEL:
            if (isAlpha(c)) {
                char token[64];
                readIdentifier(token, sizeof(token));
                skipWhitespace();
                
                if (peek() == ':' && peek(1) != '=') {
                    // It's a label
                    advance(); // consume :
                    emit(token);
                    emitLine(":");
                    continue;
                }
                
                // Not a label - it's an instruction
                // Process the instruction
                processInstruction(token);
                continue;
            }
            
            // = (assign) instruction OR ==I comparison
            if (c == '=') {
                advance();
                // Check if this is ==I comparison
                if (peek() == '=') {
                    advance(); // consume second =
                    if (peek() == 'I' || peek() == 'i') {
                        advance();
                        processInstruction("==I");
                        continue;
                    }
                    emit("// Unknown comparison: ==\n");
                    continue;
                }
                // It's the assign instruction
                skipWhitespace();
                char operand[64];
                readIdentifier(operand, sizeof(operand));
                handleAssign(operand);
                continue;
            }
            
            // Math operators (+I, -I, *I, /I)
            if (c == '+' || c == '-' || c == '*' || c == '/') {
                char op[4];
                op[0] = advance();
                if (peek() == 'I' || peek() == 'i') {
                    op[1] = advance();
                    op[2] = '\0';
                    // Convert to uppercase
                    if (op[1] == 'i') op[1] = 'I';
                    processInstruction(op);
                    continue;
                }
                // Not a math op - might be start of number, rewind
                // Actually, we can't easily rewind, so emit unknown
                emit("// Unknown operator: ");
                emitLine(op);
                continue;
            }
            
            // Comparison operators (==I, <>I, >I, >=I, <I, <=I)
            if (c == '<' || c == '>') {
                char op[4];
                op[0] = advance();
                int idx = 1;
                // Check for second char (=, >, or I)
                char c2 = peek();
                if (c2 == '=' || c2 == '>') {
                    op[idx++] = advance();
                }
                // Now expect I
                if (peek() == 'I' || peek() == 'i') {
                    op[idx++] = 'I';
                    advance();
                    op[idx] = '\0';
                    processInstruction(op);
                    continue;
                }
                op[idx] = '\0';
                emit("// Unknown comparison: ");
                emitLine(op);
                continue;
            }
            
            // Special characters
            if (c == '(') {
                advance();
                // Shouldn't happen standalone, but handle it
                continue;
            }
            
            if (c == ')') {
                advance();
                handleNestingClose();
                continue;
            }
            
            // Skip unknown characters
            advance();
        }

        return !has_error;
    }

    void processInstruction(const char* instr) {
        char operand1[64] = {0};
        char operand2[64] = {0};
        char operand3[64] = {0};
        
        // Convert instruction to uppercase for comparison
        char upperInstr[64];
        int i = 0;
        while (instr[i] && i < 63) {
            upperInstr[i] = (instr[i] >= 'a' && instr[i] <= 'z') ? instr[i] - 32 : instr[i];
            i++;
        }
        upperInstr[i] = '\0';

        // ============ Bit Logic ============
        
        // A, AN (AND)
        if (strEq(upperInstr, "A")) {
            skipWhitespace();
            if (peek() == '(') {
                advance();
                handleNestingOpen('A');
            } else {
                readIdentifier(operand1, sizeof(operand1));
                handleBitLogic('A', false, operand1);
            }
            return;
        }
        if (strEq(upperInstr, "AN")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleBitLogic('A', true, operand1);
            return;
        }
        
        // O, ON (OR)
        if (strEq(upperInstr, "O")) {
            skipWhitespace();
            if (peek() == '(') {
                advance();
                handleNestingOpen('O');
            } else if (peek() == '\n' || peek() == '\0' || peek() == '/') {
                // Bare O instruction (OR with next network) - rare, emit comment
                emitLine("// OR (network combine - not directly supported)");
            } else {
                readIdentifier(operand1, sizeof(operand1));
                handleBitLogic('O', false, operand1);
            }
            return;
        }
        if (strEq(upperInstr, "ON")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleBitLogic('O', true, operand1);
            return;
        }
        
        // X, XN (XOR)
        if (strEq(upperInstr, "X") && !isDigit(peek())) {
            skipWhitespace();
            if (peek() == '(') {
                advance();
                handleNestingOpen('X');
            } else {
                readIdentifier(operand1, sizeof(operand1));
                handleBitLogic('X', false, operand1);
            }
            return;
        }
        if (strEq(upperInstr, "XN")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleBitLogic('X', true, operand1);
            return;
        }
        
        // SET, CLR, NOT
        if (strEq(upperInstr, "SET")) {
            emitLine("u8.const 1");
            network_has_rlo = true;
            return;
        }
        if (strEq(upperInstr, "CLR") || strEq(upperInstr, "CLEAR")) {
            emitLine("u8.const 0");
            network_has_rlo = true;
            return;
        }
        if (strEq(upperInstr, "NOT")) {
            emitLine("u8.not");
            return;
        }
        
        // = (assign)
        if (upperInstr[0] == '=' && upperInstr[1] == '\0') {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleAssign(operand1);
            return;
        }
        
        // S (set), R (reset)
        if (strEq(upperInstr, "S")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleSet(operand1);
            return;
        }
        if (strEq(upperInstr, "R")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleReset(operand1);
            return;
        }
        
        // ============ Edge Detection ============
        
        if (strEq(upperInstr, "FP")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleFP(operand1);
            return;
        }
        if (strEq(upperInstr, "FN")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleFN(operand1);
            return;
        }
        if (strEq(upperInstr, "FX")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleFX(operand1);
            return;
        }
        
        // ============ Timers ============
        
        if (strEq(upperInstr, "TON") || strEq(upperInstr, "TOF") || strEq(upperInstr, "TP")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1)); // Timer address
            skipWhitespace();
            if (peek() == ',') advance();
            skipWhitespace();
            readIdentifier(operand2, sizeof(operand2)); // Preset
            
            const char* timerType = strEq(upperInstr, "TON") ? "ton" : 
                                    strEq(upperInstr, "TOF") ? "tof" : "tp";
            handleTimer(timerType, operand1, operand2);
            return;
        }
        
        // ============ Counters ============
        
        if (strEq(upperInstr, "CTU")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1)); // Counter address
            skipWhitespace();
            if (peek() == ',') advance();
            skipWhitespace();
            readIdentifier(operand2, sizeof(operand2)); // Preset
            skipWhitespace();
            if (peek() == ',') advance();
            skipWhitespace();
            readIdentifier(operand3, sizeof(operand3)); // Reset bit
            handleCTU(operand1, operand2, operand3);
            return;
        }
        
        if (strEq(upperInstr, "CTD")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1)); // Counter address
            skipWhitespace();
            if (peek() == ',') advance();
            skipWhitespace();
            readIdentifier(operand2, sizeof(operand2)); // Preset
            skipWhitespace();
            if (peek() == ',') advance();
            skipWhitespace();
            readIdentifier(operand3, sizeof(operand3)); // Load bit
            handleCTD(operand1, operand2, operand3);
            return;
        }
        
        // ============ Load/Transfer ============
        
        if (strEq(upperInstr, "L")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleLoad(operand1);
            return;
        }
        if (strEq(upperInstr, "T")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleTransfer(operand1);
            return;
        }
        
        // ============ Math Operations ============
        
        if (strEq(upperInstr, "+I") || strEq(upperInstr, "-I") || 
            strEq(upperInstr, "*I") || strEq(upperInstr, "/I") ||
            strEq(upperInstr, "MOD") || strEq(upperInstr, "NEG") || strEq(upperInstr, "ABS")) {
            handleMath(upperInstr);
            return;
        }
        
        // ============ Compare Operations ============
        
        if (strEq(upperInstr, "==I") || strEq(upperInstr, "<>I") ||
            strEq(upperInstr, ">I") || strEq(upperInstr, ">=I") ||
            strEq(upperInstr, "<I") || strEq(upperInstr, "<=I")) {
            handleCompare(upperInstr);
            return;
        }
        
        // ============ Jumps ============
        
        if (strEq(upperInstr, "JU") || strEq(upperInstr, "JC") || strEq(upperInstr, "JCN")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleJump(upperInstr, operand1);
            return;
        }
        
        // ============ Calls/Returns ============
        
        if (strEq(upperInstr, "CALL")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleCall(operand1);
            return;
        }
        
        if (strEq(upperInstr, "BE") || strEq(upperInstr, "BEC") || 
            strEq(upperInstr, "BEU") || strEq(upperInstr, "RET")) {
            handleReturn(upperInstr);
            return;
        }
        
        // ============ Network marker ============
        
        if (strEq(upperInstr, "NETWORK")) {
            // Network marker - emit clear and comment
            emitLine("");
            emit("// NETWORK");
            skipWhitespace();
            if (isDigit(peek())) {
                emit(" ");
                readIdentifier(operand1, sizeof(operand1));
                emit(operand1);
            }
            emitLine("");
            emitLine("clear");
            network_has_rlo = false;
            return;
        }
        
        // ============ NOP ============
        
        if (strEq(upperInstr, "NOP")) {
            emitLine("nop");
            return;
        }
        
        // ============ IEC IL Aliases ============
        
        // LD (Load) - same as first A
        if (strEq(upperInstr, "LD")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            emitBoolRead(operand1, false);
            network_has_rlo = true;
            return;
        }
        
        // LDN (Load Not)
        if (strEq(upperInstr, "LDN")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            emitBoolRead(operand1, true);
            network_has_rlo = true;
            return;
        }
        
        // ST (Store) - same as =
        if (strEq(upperInstr, "ST")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleAssign(operand1);
            return;
        }
        
        // AND, OR, XOR (IEC aliases)
        if (strEq(upperInstr, "AND")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleBitLogic('A', false, operand1);
            return;
        }
        if (strEq(upperInstr, "ANDN")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleBitLogic('A', true, operand1);
            return;
        }
        if (strEq(upperInstr, "OR")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleBitLogic('O', false, operand1);
            return;
        }
        if (strEq(upperInstr, "ORN")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleBitLogic('O', true, operand1);
            return;
        }
        if (strEq(upperInstr, "XOR")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleBitLogic('X', false, operand1);
            return;
        }
        if (strEq(upperInstr, "XORN")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleBitLogic('X', true, operand1);
            return;
        }
        
        // JMP, JMPC, JMPCN (IEC aliases)
        if (strEq(upperInstr, "JMP")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleJump("JU", operand1);
            return;
        }
        if (strEq(upperInstr, "JMPC")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleJump("JC", operand1);
            return;
        }
        if (strEq(upperInstr, "JMPCN") || strEq(upperInstr, "JMPNC")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleJump("JCN", operand1);
            return;
        }
        
        // CAL (IEC call)
        if (strEq(upperInstr, "CAL")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleCall(operand1);
            return;
        }
        
        // Unknown instruction - report error
        setError("Unknown instruction");
        emit("// Unknown instruction: ");
        emitLine(instr);
    }

    // Get compiled output
    const char* getOutput() {
        return output;
    }

    int getOutputLength() {
        return output_length;
    }

    bool hasError() {
        return has_error;
    }

    const char* getErrorMessage() {
        return error_message;
    }
};

// ============ Global Instance & WASM Exports ============

STLCompiler& getSTLCompiler() {
    static STLCompiler instance;
    return instance;
}
#define stlCompiler getSTLCompiler()

// Temporary buffer for STL source (loaded from stream)
static char stl_source_buffer[STL_MAX_OUTPUT_SIZE];
static int stl_source_length = 0;

extern "C" {

    // Load STL source from stream buffer (call after streaming in the STL code)
    WASM_EXPORT void stl_load_from_stream() {
        // Use streamRead to copy stream buffer to our STL buffer
        streamRead(stl_source_buffer, stl_source_length, STL_MAX_OUTPUT_SIZE);
        stlCompiler.setSource(stl_source_buffer, stl_source_length);
    }

    // Set STL source code directly (for internal use)
    WASM_EXPORT void stl_set_source(char* source, int length) {
        stlCompiler.setSource(source, length);
    }

    // Compile STL to PLCASM
    WASM_EXPORT bool stl_compile() {
        return stlCompiler.compile();
    }

    // Get output PLCASM pointer
    WASM_EXPORT const char* stl_get_output() {
        return stlCompiler.getOutput();
    }

    // Get output length
    WASM_EXPORT int stl_get_output_length() {
        return stlCompiler.getOutputLength();
    }

    // Check for errors
    WASM_EXPORT bool stl_has_error() {
        return stlCompiler.hasError();
    }

    // Get error message
    WASM_EXPORT const char* stl_get_error() {
        return stlCompiler.getErrorMessage();
    }

    // Get error line
    WASM_EXPORT int stl_get_error_line() {
        return stlCompiler.error_line;
    }

    // Get error column
    WASM_EXPORT int stl_get_error_column() {
        return stlCompiler.error_column;
    }

    // Write STL output to stream (for reading via readStream)
    WASM_EXPORT void stl_output_to_stream() {
        const char* output = stlCompiler.getOutput();
        int len = stlCompiler.getOutputLength();
        // Use streamOut to send to JS (this goes to stream_message in JS)
        for (int i = 0; i < len; i++) {
            streamOut(output[i]);
        }
    }

    // Convenience: compile STL and then compile resulting PLCASM to bytecode
    WASM_EXPORT bool stl_compile_full() {
        if (!stlCompiler.compile()) {
            return true; // Error in STL compilation
        }
        
        // Pass the PLCASM output to the main compiler
        defaultCompiler.set_assembly_string((char*)stlCompiler.getOutput());
        return defaultCompiler.compileAssembly(false, false);
    }
}

#endif // __WASM__
