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
#include "shared-symbols.h"

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
// VovkPLCRuntime Extensions (non-standard):
//   TAP                   - Tap/passthrough RLO to next rung (preserves RLO after output)
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
// Increment/Decrement:
//   INC, DEC              - Increment/Decrement stack value (i16 default)
//   INCI addr, DECI addr  - Increment/Decrement 16-bit integer at address
//   INCD addr, DECD addr  - Increment/Decrement 32-bit integer at address
//   INCB addr, DECB addr  - Increment/Decrement 8-bit byte at address
//   INCR addr, DECR addr  - Increment/Decrement 32-bit float at address
//
// Math (Siemens STL style):
//   +I, -I, *I, /I        - 16-bit signed integer math (maps to i16)
//   +D, -D, *D, /D        - 32-bit signed integer math (maps to i32)
//   +R, -R, *R, /R        - 32-bit floating point math (maps to f32)
//   MOD                   - Modulo (i16 default)
//   NEG, ABS              - Negate, Absolute value
//
// Math (Extended VovkPLCRuntime types):
//   ADD_U8..ADD_I64       - Typed addition (u8, u16, u32, u64, i8, i16, i32, i64, f32, f64)
//   SUB_U8..SUB_I64       - Typed subtraction
//   MUL_U8..MUL_I64       - Typed multiplication
//   DIV_U8..DIV_I64       - Typed division
//   MOD_U8..MOD_I64       - Typed modulo
//   NEG_I8..NEG_F64       - Typed negate (signed types only)
//   ABS_I8..ABS_F64       - Typed absolute value (signed types only)
//
// Compare:
//   ==I, <>I, >I, >=I, <I, <=I  - Integer comparisons (i16 default)
//   ==D, <>D, >D, >=D, <D, <=D  - 32-bit comparisons (i32)
//   ==R, <>R, >R, >=R, <R, <=R  - Float comparisons (f32)

#define STL_MAX_OUTPUT_SIZE 32768
#define STL_MAX_NESTING_DEPTH 16
#define STL_MAX_LABELS 64
#define STL_MAX_TYPE_STACK 64

// Type IDs for stack tracking (must match PLCASM type codes)
enum STLType {
    STL_TYPE_UNKNOWN = 0,
    STL_TYPE_BOOL = 1,
    STL_TYPE_U8 = 2,
    STL_TYPE_U16 = 3,
    STL_TYPE_U32 = 4,
    STL_TYPE_U64 = 5,
    STL_TYPE_I8 = 6,
    STL_TYPE_I16 = 7,
    STL_TYPE_I32 = 8,
    STL_TYPE_I64 = 9,
    STL_TYPE_F32 = 10,
    STL_TYPE_F64 = 11,
};

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
    
    // Track when BR is loaded to stack (L BR) for combining with comparison results
    bool br_loaded_for_comparison = false;
    
    // Track when TAP has been pre-emitted by SET/RESET handling (to avoid double emission)
    bool tap_consumed = false;

    // Nesting stack for A(, O(, X( operations
    char nesting_ops[STL_MAX_NESTING_DEPTH]; // 'A', 'O', 'X'
    bool nesting_had_rlo[STL_MAX_NESTING_DEPTH]; // Track if outer had RLO before nesting
    int nesting_depth = 0;

    // Label counter for generated labels
    int label_counter = 0;
    
    // Unique hash for this compilation unit (to avoid label collisions when concatenating)
    uint32_t compilation_hash = 0;
    
    // Type stack for tracking what's on the runtime stack
    STLType type_stack[STL_MAX_TYPE_STACK];
    int type_stack_depth = 0;
    
    // Current expression type (type of the last value pushed/computed)
    const char* currentExprType = nullptr;
    
    // Static counter that increments with each compilation to ensure uniqueness
    // even for identical source code compiled multiple times
    static uint32_t& getGlobalCompilationCounter() {
        static uint32_t counter = 0;
        return counter;
    }

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
        br_loaded_for_comparison = false;
        tap_consumed = false;
        nesting_depth = 0;
        label_counter = 0;
        compilation_hash = 0;
        currentExprType = nullptr;
    }

    void setSource(char* source, int length) {
        reset();
        stl_source = source;
        stl_length = length;
        // Combine content hash with global counter for guaranteed uniqueness
        uint32_t content_hash = computeSourceHash(source, length);
        uint32_t counter = getGlobalCompilationCounter()++;
        compilation_hash = content_hash ^ (counter * 0x9e3779b9); // Mix with golden ratio
    }
    
    // Simple hash function for generating unique compilation unit ID
    uint32_t computeSourceHash(const char* source, int length) {
        uint32_t hash = 0x811c9dc5; // FNV-1a offset basis
        for (int i = 0; i < length; i++) {
            hash ^= (uint8_t)source[i];
            hash *= 0x01000193; // FNV-1a prime
        }
        return hash;
    }
    
    // Emit hex value (for hash suffix)
    void emitHex(uint32_t value) {
        const char hex[] = "0123456789abcdef";
        char buf[9];
        for (int i = 7; i >= 0; i--) {
            buf[i] = hex[value & 0xF];
            value >>= 4;
        }
        buf[8] = '\0';
        emit(buf);
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

    // Generate unique label with hash postfix to avoid collisions across compilation units
    void emitUniqueLabel(const char* prefix) {
        emit("__");
        emit(prefix);
        emit("_");
        emitInt(label_counter++);
        emit("_");
        emitHex(compilation_hash);
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

    // Helper to check if a string is a valid direct address (starts with I, Q, M, T, C, X, Y, S, P, or #)
    // Returns true for direct addresses like M0, T5, I0.0, MW10, T#5s
    // Returns false for symbol names like my_timer2, counter1 (even if they start with M, T, C, etc.)
    bool isDirectAddress(const char* addr) {
        if (!addr || !addr[0]) return false;
        char first = addr[0];
        if (first >= 'a' && first <= 'z') first -= 32; // Uppercase
        
        // Immediate value
        if (first == '#') return true;
        
        // Check for valid address prefix
        bool hasPrefix = (first == 'I' || first == 'Q' || first == 'M' || first == 'T' || 
                         first == 'C' || first == 'X' || first == 'Y' || first == 'S' || first == 'P');
        
        if (!hasPrefix) return false;
        
        // For P, check for P_On, P_Off patterns
        if (first == 'P') return (addr[1] == '_');
        
        // For T, check for T# duration format
        if (first == 'T' && addr[1] == '#') return true;
        
        // After a valid prefix letter, the next character must be:
        // - A digit (M0, T5, I0, etc.)
        // - A type suffix B/W/D/L/R followed by digit (MW10, IW0, etc.)
        char second = addr[1];
        
        // Check for digit directly after prefix
        if (second >= '0' && second <= '9') return true;
        
        // Check for type suffix (B=byte, W=word, D=dword, L=long, R=real)
        if (second >= 'a' && second <= 'z') second -= 32; // Uppercase
        if (second == 'B' || second == 'W' || second == 'D' || second == 'L' || second == 'R') {
            // Must be followed by a digit
            char third = addr[2];
            return (third >= '0' && third <= '9');
        }
        
        // If second char is another letter or underscore, it's a symbol name
        return false;
    }

    // Helper to check if a type is a timer type
    bool isTimerType(const char* type) {
        if (!type) return false;
        return strEq(type, "timer") || strEq(type, "ton") || strEq(type, "tof") || strEq(type, "tp");
    }

    // Helper to check if a type is a counter type
    bool isCounterType(const char* type) {
        if (!type) return false;
        return strEq(type, "counter") || strEq(type, "ctu") || strEq(type, "ctd") || strEq(type, "ctud");
    }

    // Helper to check if address contains property access (e.g., T0.Q, my_timer.ET)
    bool hasPropertyAccess(const char* addr) {
        if (!addr) return false;
        for (int i = 0; addr[i] && addr[i + 1]; i++) {
            if (addr[i] == '.') {
                char next = addr[i + 1];
                // Property access if next char is a letter (not a digit for bit access)
                if ((next >= 'A' && next <= 'Z') || (next >= 'a' && next <= 'z')) {
                    return true;
                }
            }
        }
        return false;
    }

    // Convert STL address to PLCASM address
    // STL uses: I0.0 (input), Q0.0 (output), M0.0 (marker), etc.
    // PLCASM uses: X0.0 (input), Y0.0 (output), M0.0 (marker), S0 (system), C0 (control)
    // Now also handles property access like T0.Q, my_timer.ET
    virtual void convertAddress(const char* stlAddr, char* plcasmAddr) {
        int i = 0, j = 0;
        
        // Check for property access (e.g., T0.Q, my_timer.ET)
        // These are passed through as-is to PLCASM which will resolve them
        if (hasPropertyAccess(stlAddr)) {
            // Check if it's a symbol-based property access
            // Find the dot position
            int dot_pos = -1;
            for (int k = 0; stlAddr[k]; k++) {
                if (stlAddr[k] == '.') {
                    dot_pos = k;
                    break;
                }
            }
            
            if (dot_pos > 0) {
                // Extract base name before dot
                char base[64] = {0};
                for (int k = 0; k < dot_pos && k < 63; k++) {
                    base[k] = stlAddr[k];
                }
                
                // Check if base is a symbol
                if (!isDirectAddress(base)) {
                    SharedSymbol* sym = findSharedSymbol(base);
                    if (sym && (isTimerType(sym->type) || isCounterType(sym->type))) {
                        // Symbol-based property access on timer/counter
                        // Convert: my_timer.Q -> T<idx>.Q
                        char prefix = isTimerType(sym->type) ? 'T' : 'C';
                        plcasmAddr[j++] = prefix;
                        u32 idx = sym->address;
                        if (idx >= 100) { plcasmAddr[j++] = '0' + (idx / 100); idx %= 100; }
                        if (idx >= 10 || sym->address >= 100) { plcasmAddr[j++] = '0' + (idx / 10); idx %= 10; }
                        plcasmAddr[j++] = '0' + idx;
                        // Copy the property part (including dot)
                        for (int k = dot_pos; stlAddr[k]; k++) {
                            plcasmAddr[j++] = stlAddr[k];
                        }
                        plcasmAddr[j] = '\0';
                        return;
                    }
                }
            }
            
            // Direct address with property or unknown - pass through as-is
            while (stlAddr[i]) plcasmAddr[j++] = stlAddr[i++];
            plcasmAddr[j] = '\0';
            return;
        }
        
        // First, check if this is a symbol name (not a direct address)
        if (!isDirectAddress(stlAddr)) {
            SharedSymbol* sym = findSharedSymbol(stlAddr);
            if (sym) {
                // Found a symbol - check if it's a timer or counter
                if (isTimerType(sym->type)) {
                    // Timer symbol - output T<index>
                    plcasmAddr[j++] = 'T';
                    u32 idx = sym->address;
                    if (idx >= 100) { plcasmAddr[j++] = '0' + (idx / 100); idx %= 100; }
                    if (idx >= 10 || sym->address >= 100) { plcasmAddr[j++] = '0' + (idx / 10); idx %= 10; }
                    plcasmAddr[j++] = '0' + idx;
                    plcasmAddr[j] = '\0';
                    return;
                } else if (isCounterType(sym->type)) {
                    // Counter symbol - output C<index>
                    plcasmAddr[j++] = 'C';
                    u32 idx = sym->address;
                    if (idx >= 100) { plcasmAddr[j++] = '0' + (idx / 100); idx %= 100; }
                    if (idx >= 10 || sym->address >= 100) { plcasmAddr[j++] = '0' + (idx / 10); idx %= 10; }
                    plcasmAddr[j++] = '0' + idx;
                    plcasmAddr[j] = '\0';
                    return;
                } else if (sym->is_bit) {
                    // Bit symbol - output M<address>.<bit>
                    plcasmAddr[j++] = 'M';
                    u32 addr = sym->address;
                    if (addr >= 100) { plcasmAddr[j++] = '0' + (addr / 100); addr %= 100; }
                    if (addr >= 10 || sym->address >= 100) { plcasmAddr[j++] = '0' + (addr / 10); addr %= 10; }
                    plcasmAddr[j++] = '0' + addr;
                    plcasmAddr[j++] = '.';
                    plcasmAddr[j++] = '0' + sym->bit;
                    plcasmAddr[j] = '\0';
                    return;
                } else {
                    // Non-bit symbol - output M<address>
                    plcasmAddr[j++] = 'M';
                    u32 addr = sym->address;
                    if (addr >= 100) { plcasmAddr[j++] = '0' + (addr / 100); addr %= 100; }
                    if (addr >= 10 || sym->address >= 100) { plcasmAddr[j++] = '0' + (addr / 10); addr %= 10; }
                    plcasmAddr[j++] = '0' + addr;
                    plcasmAddr[j] = '\0';
                    return;
                }
            }
            // Symbol not found - fall through to copy as-is
        }
        
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

    // ============ Type inference helpers ============
    
    // Get PLCASM type from STL address prefix (MW, MD, MB, etc.)
    // Returns the type string and advances the index past the type prefix
    // MW = Memory Word (16-bit signed) → i16
    // MD = Memory Double word (32-bit signed) → i32
    // MB = Memory Byte (8-bit unsigned) → u8
    // Similar for I (input), Q (output)
    const char* getTypeFromAddressPrefix(const char* addr, int* skipChars = nullptr) {
        if (!addr || !addr[0]) { if (skipChars) *skipChars = 0; return "u8"; }
        
        char first = addr[0];
        if (first >= 'a' && first <= 'z') first -= 32; // Uppercase
        
        char second = addr[1];
        if (second >= 'a' && second <= 'z') second -= 32; // Uppercase
        
        // Check for type suffix after area letter (M, I, Q, etc.)
        // Format: <area><type><number> e.g., MW10, MD20, IW0, QD4
        if (first == 'M' || first == 'I' || first == 'Q' || first == 'X' || first == 'Y') {
            switch (second) {
                case 'B': if (skipChars) *skipChars = 2; return "u8";   // Byte (8-bit)
                case 'W': if (skipChars) *skipChars = 2; return "i16";  // Word (16-bit signed)
                case 'D': if (skipChars) *skipChars = 2; return "i32";  // Double word (32-bit signed)
                case 'L': if (skipChars) *skipChars = 2; return "i64";  // Long (64-bit signed)
                case 'R': if (skipChars) *skipChars = 2; return "f32";  // Real (32-bit float)
                default:
                    // No type suffix - check if it's a bit address (has dot)
                    for (int i = 1; addr[i]; i++) {
                        if (addr[i] == '.') { if (skipChars) *skipChars = 1; return "u8"; } // Bit address
                    }
                    if (skipChars) *skipChars = 1;
                    return "u8"; // Default to byte
            }
        }
        
        if (skipChars) *skipChars = 0;
        return "u8"; // Default
    }
    
    // Convert STL typed address to PLCASM address (strips type prefix)
    // MW10 → M10, IW0 → X0, QD4 → Y4
    void convertTypedAddress(const char* stlAddr, char* plcasmAddr, const char** outType = nullptr) {
        int i = 0, j = 0;
        
        // Check for property access first (e.g., my_timer.ET, T0.Q)
        // Property access is detected by a dot followed by a letter
        if (hasPropertyAccess(stlAddr)) {
            // Find the dot position
            int dotPos = 0;
            while (stlAddr[dotPos] && stlAddr[dotPos] != '.') dotPos++;
            
            if (stlAddr[dotPos] == '.') {
                // Extract base name and property
                char baseName[64];
                int k = 0;
                for (int p = 0; p < dotPos && p < 63; p++) {
                    baseName[k++] = stlAddr[p];
                }
                baseName[k] = '\0';
                const char* propName = &stlAddr[dotPos + 1];
                
                // Check if base is a timer (T0, T1, etc.) or counter (C0, C1, etc.)
                char baseUpper = baseName[0];
                if (baseUpper >= 'a' && baseUpper <= 'z') baseUpper -= 32;
                
                bool isTimer = (baseUpper == 'T' && baseName[1] >= '0' && baseName[1] <= '9');
                bool isCounter = (baseUpper == 'C' && baseName[1] >= '0' && baseName[1] <= '9');
                
                // Note: For symbolic names (my_timer.ET), we can't determine the type here
                // since STL compiler doesn't have access to the symbol table.
                // In that case, we default to u8 and let PLCASM handle the resolution.
                // For direct T/C references, we can look up the property type.
                
                if (isTimer) {
                    // Look up timer property type
                    for (int p = 0; p < timerPropertyCount; p++) {
                        // Case-insensitive compare
                        bool match = true;
                        const char* pn = timerProperties[p].name;
                        const char* pp = propName;
                        while (*pn && *pp) {
                            char c1 = *pn, c2 = *pp;
                            if (c1 >= 'a' && c1 <= 'z') c1 -= 32;
                            if (c2 >= 'a' && c2 <= 'z') c2 -= 32;
                            if (c1 != c2) { match = false; break; }
                            pn++; pp++;
                        }
                        if (match && !*pn && !*pp) {
                            // Found property - set type
                            if (outType) {
                                u8 ts = timerProperties[p].type_size;
                                if (timerProperties[p].bit_pos != 0xFF) {
                                    *outType = "u8"; // Bit properties use u8 for bit operations
                                } else if (ts == 1) {
                                    *outType = "u8";
                                } else if (ts == 2) {
                                    *outType = "u16";
                                } else if (ts == 4) {
                                    *outType = "u32";
                                } else {
                                    *outType = "u32";
                                }
                            }
                            break;
                        }
                    }
                } else if (isCounter) {
                    // Look up counter property type
                    for (int p = 0; p < counterPropertyCount; p++) {
                        // Case-insensitive compare
                        bool match = true;
                        const char* pn = counterProperties[p].name;
                        const char* pp = propName;
                        while (*pn && *pp) {
                            char c1 = *pn, c2 = *pp;
                            if (c1 >= 'a' && c1 <= 'z') c1 -= 32;
                            if (c2 >= 'a' && c2 <= 'z') c2 -= 32;
                            if (c1 != c2) { match = false; break; }
                            pn++; pp++;
                        }
                        if (match && !*pn && !*pp) {
                            // Found property - set type
                            if (outType) {
                                u8 ts = counterProperties[p].type_size;
                                if (counterProperties[p].bit_pos != 0xFF) {
                                    *outType = "u8"; // Bit properties use u8 for bit operations
                                } else if (ts == 1) {
                                    *outType = "u8";
                                } else if (ts == 2) {
                                    *outType = "u16";
                                } else if (ts == 4) {
                                    *outType = "u32";
                                } else {
                                    *outType = "u32";
                                }
                            }
                            break;
                        }
                    }
                }
                
                // For symbolic names without direct type info, we need to guess based on property name
                // Since we know common timer/counter property names, we can infer the type
                if (outType && !isTimer && !isCounter) {
                    // Try to match known timer property names
                    bool foundType = false;
                    for (int p = 0; p < timerPropertyCount && !foundType; p++) {
                        bool match = true;
                        const char* pn = timerProperties[p].name;
                        const char* pp = propName;
                        while (*pn && *pp) {
                            char c1 = *pn, c2 = *pp;
                            if (c1 >= 'a' && c1 <= 'z') c1 -= 32;
                            if (c2 >= 'a' && c2 <= 'z') c2 -= 32;
                            if (c1 != c2) { match = false; break; }
                            pn++; pp++;
                        }
                        if (match && !*pn && !*pp) {
                            u8 ts = timerProperties[p].type_size;
                            if (timerProperties[p].bit_pos != 0xFF) {
                                *outType = "u8";
                            } else if (ts == 4) {
                                *outType = "u32";
                            } else {
                                *outType = "u8";
                            }
                            foundType = true;
                        }
                    }
                    // Try counter properties
                    for (int p = 0; p < counterPropertyCount && !foundType; p++) {
                        bool match = true;
                        const char* pn = counterProperties[p].name;
                        const char* pp = propName;
                        while (*pn && *pp) {
                            char c1 = *pn, c2 = *pp;
                            if (c1 >= 'a' && c1 <= 'z') c1 -= 32;
                            if (c2 >= 'a' && c2 <= 'z') c2 -= 32;
                            if (c1 != c2) { match = false; break; }
                            pn++; pp++;
                        }
                        if (match && !*pn && !*pp) {
                            u8 ts = counterProperties[p].type_size;
                            if (counterProperties[p].bit_pos != 0xFF) {
                                *outType = "u8";
                            } else if (ts == 4) {
                                *outType = "u32";
                            } else {
                                *outType = "u8";
                            }
                            foundType = true;
                        }
                    }
                    // If still not found, default to u32 (common for ET, CV)
                    if (!foundType) {
                        *outType = "u32";
                    }
                }
                
                // Copy address as-is for PLCASM to handle
                while (stlAddr[i]) plcasmAddr[j++] = stlAddr[i++];
                plcasmAddr[j] = '\0';
                return;
            }
        }
        
        // Check if this is a symbol name (not a direct address)
        if (!isDirectAddress(stlAddr)) {
            SharedSymbol* sym = findSharedSymbol(stlAddr);
            if (sym) {
                // Found a symbol - set the type from symbol definition
                if (outType) {
                    if (sym->is_bit) {
                        *outType = "u8";
                    } else {
                        // Use the symbol's declared type
                        *outType = sym->type;
                    }
                }
                // Copy address as-is for PLCASM to resolve
                while (stlAddr[i]) plcasmAddr[j++] = stlAddr[i++];
                plcasmAddr[j] = '\0';
                return;
            }
        }
        
        char first = stlAddr[0];
        if (first >= 'a' && first <= 'z') first -= 32; // Uppercase
        
        // Map area letter
        switch (first) {
            case 'I': plcasmAddr[j++] = 'X'; i++; break;  // Input
            case 'Q': plcasmAddr[j++] = 'Y'; i++; break;  // Output
            case 'M': plcasmAddr[j++] = 'M'; i++; break;  // Marker
            case 'X': plcasmAddr[j++] = 'X'; i++; break;  // Already PLCASM
            case 'Y': plcasmAddr[j++] = 'Y'; i++; break;
            default:
                // Unknown - copy as-is, get type from prefix
                int skip = 0;
                if (outType) *outType = getTypeFromAddressPrefix(stlAddr, &skip);
                while (stlAddr[i]) plcasmAddr[j++] = stlAddr[i++];
                plcasmAddr[j] = '\0';
                return;
        }
        
        // Check for type suffix (B, W, D, L, R)
        char second = stlAddr[i];
        if (second >= 'a' && second <= 'z') second -= 32;
        
        const char* type = "u8";
        if (second == 'B' || second == 'W' || second == 'D' || second == 'L' || second == 'R') {
            switch (second) {
                case 'B': type = "u8"; break;
                case 'W': type = "i16"; break;
                case 'D': type = "i32"; break;
                case 'L': type = "i64"; break;
                case 'R': type = "f32"; break;
            }
            i++; // Skip type suffix
        }
        
        if (outType) *outType = type;
        
        // Copy rest of address (the number part)
        while (stlAddr[i]) {
            plcasmAddr[j++] = stlAddr[i++];
        }
        plcasmAddr[j] = '\0';
    }

    // Get PLCASM type name from STL type suffix (I, D, R) or extended type (U8, I16, etc.)
    // Returns the type prefix for PLCASM (e.g., "i16", "i32", "f32", "u8")
    const char* getTypeFromSuffix(const char* suffix) {
        if (!suffix || !suffix[0]) return "u8"; // Default
        
        // Single character suffixes (Siemens style)
        if (suffix[1] == '\0' || suffix[1] == ' ' || suffix[1] == '\n' || suffix[1] == '\r') {
            char c = suffix[0];
            if (c >= 'a' && c <= 'z') c -= 32; // Uppercase
            switch (c) {
                case 'I': return "i16";  // Integer (16-bit signed)
                case 'D': return "i32";  // Double word (32-bit signed)
                case 'R': return "f32";  // Real (32-bit float)
            }
        }
        
        // Extended type suffixes (VovkPLCRuntime style: _U8, _I16, etc.)
        if (suffix[0] == '_') suffix++; // Skip underscore
        
        if (strEq(suffix, "U8"))  return "u8";
        if (strEq(suffix, "U16")) return "u16";
        if (strEq(suffix, "U32")) return "u32";
        if (strEq(suffix, "U64")) return "u64";
        if (strEq(suffix, "I8"))  return "i8";
        if (strEq(suffix, "I16")) return "i16";
        if (strEq(suffix, "I32")) return "i32";
        if (strEq(suffix, "I64")) return "i64";
        if (strEq(suffix, "F32")) return "f32";
        if (strEq(suffix, "F64")) return "f64";
        
        return "u8"; // Default fallback
    }
    
    // Check if two type strings are the same
    bool typesMatch(const char* t1, const char* t2) {
        if (!t1 || !t2) return false;
        return strEq(t1, t2);
    }
    
    // Emit CVT instruction if source and destination types differ
    // Updates currentExprType to destType
    void emitCvtIfNeeded(const char* destType) {
        if (!currentExprType || !destType) return;
        if (typesMatch(currentExprType, destType)) return;
        
        // Emit conversion: cvt <from> <to>
        emit("cvt ");
        emit(currentExprType);
        emit(" ");
        emitLine(destType);
        currentExprType = destType;
    }
    
    // Set current expression type (after load or math operation)
    void setExprType(const char* type) {
        currentExprType = type;
    }
    
    // Lookahead to find the next math/compare operation and return its expected type
    // This allows L (load) to emit the correct type directly without needing CVT
    const char* peekNextMathType() {
        int savedPos = pos;
        int savedLine = current_line;
        int savedCol = current_column;
        const char* result = "u8"; // Default type
        
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
            
            // Check for math operators: +I, -I, *I, /I, +D, -D, *D, /D, +R, -R, *R, /R
            if (c == '+' || c == '-' || c == '*' || c == '/') {
                if (pos + 1 < stl_length) {
                    char typeSuffix = stl_source[pos + 1];
                    if (typeSuffix == 'I' || typeSuffix == 'i') { result = "i16"; break; }
                    if (typeSuffix == 'D' || typeSuffix == 'd') { result = "i32"; break; }
                    if (typeSuffix == 'R' || typeSuffix == 'r') { result = "f32"; break; }
                }
            }
            
            // Check for comparison operators: ==I, <>I, >I, etc.
            if (c == '=' && pos + 2 < stl_length && stl_source[pos + 1] == '=') {
                char typeSuffix = stl_source[pos + 2];
                if (typeSuffix == 'I' || typeSuffix == 'i') { result = "i16"; break; }
                if (typeSuffix == 'D' || typeSuffix == 'd') { result = "i32"; break; }
                if (typeSuffix == 'R' || typeSuffix == 'r') { result = "f32"; break; }
            }
            if (c == '<' || c == '>') {
                int offset = 1;
                if (pos + offset < stl_length && (stl_source[pos + offset] == '=' || stl_source[pos + offset] == '>')) {
                    offset++;
                }
                if (pos + offset < stl_length) {
                    char typeSuffix = stl_source[pos + offset];
                    if (typeSuffix == 'I' || typeSuffix == 'i') { result = "i16"; break; }
                    if (typeSuffix == 'D' || typeSuffix == 'd') { result = "i32"; break; }
                    if (typeSuffix == 'R' || typeSuffix == 'r') { result = "f32"; break; }
                }
            }
            
            // Check for extended typed operations (ADD_U8, SUB_I32, etc.)
            if (isAlpha(c)) {
                char token[64];
                int i = 0;
                while (i < 63 && pos + i < stl_length && (isAlphaNum(stl_source[pos + i]) || stl_source[pos + i] == '_')) {
                    token[i] = stl_source[pos + i];
                    if (token[i] >= 'a' && token[i] <= 'z') token[i] -= 32; // Uppercase
                    i++;
                }
                token[i] = '\0';
                
                // Check for extended math ops: ADD_*, SUB_*, MUL_*, DIV_*, MOD_*, NEG_*, ABS_*
                if (startsWith(token, "ADD_") || startsWith(token, "SUB_") ||
                    startsWith(token, "MUL_") || startsWith(token, "DIV_") ||
                    startsWith(token, "MOD_") || startsWith(token, "NEG_") ||
                    startsWith(token, "ABS_")) {
                    result = getTypeFromSuffix(token + 3); // Skip "ADD" etc, keep "_U8"
                    break;
                }
                
                // Check for standard MOD, NEG, ABS (default to i16)
                if (strEq(token, "MOD") || strEq(token, "NEG") || strEq(token, "ABS")) {
                    result = "i16";
                    break;
                }
                
                // T (Transfer) instruction - get type from destination address
                if (strEq(token, "T")) {
                    pos += i;
                    // Skip whitespace to get to operand
                    while (pos < stl_length && (stl_source[pos] == ' ' || stl_source[pos] == '\t')) pos++;
                    // Read the address operand
                    char addr[64];
                    int ai = 0;
                    while (ai < 63 && pos < stl_length && stl_source[pos] != ' ' && stl_source[pos] != '\t' && 
                           stl_source[pos] != '\r' && stl_source[pos] != '\n') {
                        addr[ai++] = stl_source[pos++];
                    }
                    addr[ai] = '\0';
                    // Get type from address prefix (MW → i16, MD → i32, etc.)
                    result = getTypeFromAddressPrefix(addr);
                    break;
                }
                
                // L instruction - skip and continue looking (it's another load)
                if (strEq(token, "L")) {
                    pos += i;
                    // Skip the operand
                    while (pos < stl_length && (stl_source[pos] == ' ' || stl_source[pos] == '\t')) pos++;
                    while (pos < stl_length && stl_source[pos] != ' ' && stl_source[pos] != '\t' && 
                           stl_source[pos] != '\r' && stl_source[pos] != '\n') pos++;
                    continue;
                }
                
                // Any other instruction - stop looking (might be boolean logic, jumps, etc.)
                break;
            }
            
            // Skip unknown characters
            pos++;
        }
        
        pos = savedPos; current_line = savedLine; current_column = savedCol;
        return result;
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
    // or TAP. Returns true if yes, and sets foundTap to true if TAP specifically.
    // This allows us to emit u8.copy only when needed for multiple coil outputs.
    bool peekNextIsOutput(bool* foundTap = nullptr) {
        if (foundTap) *foundTap = false;
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
            
            // Check for TAP instruction (RLO passthrough) - also needs RLO preserved
            if (c == 'T' || c == 't') {
                if (pos + 2 < stl_length) {
                    char c2 = stl_source[pos + 1];
                    char c3 = stl_source[pos + 2];
                    if ((c2 == 'A' || c2 == 'a') && (c3 == 'P' || c3 == 'p')) {
                        // Check it's standalone TAP, not TAPE or similar
                        if (pos + 3 >= stl_length || !isAlphaNum(stl_source[pos + 3])) {
                            pos = savedPos; current_line = savedLine; current_column = savedCol;
                            if (foundTap) *foundTap = true;
                            return true;
                        }
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
        // Clear br_loaded_for_comparison - the nesting mechanism handles combining
        br_loaded_for_comparison = false;
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
        // If the next instruction is TAP, mark it as consumed since we emit the copy here
        bool foundTap = false;
        if (peekNextIsOutput(&foundTap)) {
            emit("u8.copy\n");
            if (foundTap) tap_consumed = true;
        }
        emit("u8.writeBit ");
        emitLine(plcAddr);
        network_has_rlo = false; // Consumed (or preserved if we duped)
    }

    // Handle =N (negated assign) - writes NOT(RLO) but preserves original RLO
    void handleAssignNegated(const char* operand) {
        char plcAddr[64];
        convertAddress(operand, plcAddr);
        // Always duplicate RLO first, then invert the copy, write, leaving original on stack
        emit("u8.copy\n");
        emit("u8.not\n");
        emit("u8.writeBit ");
        emitLine(plcAddr);
        // Original RLO is still on stack
        network_has_rlo = true;
    }

    // Handle S (set if RLO=1)
    void handleSet(const char* operand) {
        char plcAddr[64];
        convertAddress(operand, plcAddr);
        
        int savedCounter = label_counter++;
        
        // Only duplicate RLO if another output instruction follows
        // If the next instruction is TAP, mark it as consumed since we emit the copy here
        bool foundTap = false;
        if (peekNextIsOutput(&foundTap)) {
            emit("u8.copy\n");
            if (foundTap) tap_consumed = true;  // TAP will be consumed, don't emit again
        }
        // Use relative jump for position-independent bytecode
        emit("jmp_if_not_rel __skip_set_");
        emitInt(savedCounter);
        emit("_");
        emitHex(compilation_hash);
        emit("\n");
        
        emit("u8.writeBitOn ");
        emitLine(plcAddr);
        
        emit("__skip_set_");
        emitInt(savedCounter);
        emit("_");
        emitHex(compilation_hash);
        emit(":\n");
        
        network_has_rlo = false;
    }

    // Handle R (reset if RLO=1)
    void handleReset(const char* operand) {
        char plcAddr[64];
        convertAddress(operand, plcAddr);
        
        int savedCounter = label_counter++;
        
        // Only duplicate RLO if another output instruction follows
        // If the next instruction is TAP, mark it as consumed since we emit the copy here
        bool foundTap = false;
        if (peekNextIsOutput(&foundTap)) {
            emit("u8.copy\n");
            if (foundTap) tap_consumed = true;  // TAP will be consumed, don't emit again
        }
        // Use relative jump for position-independent bytecode
        emit("jmp_if_not_rel __skip_reset_");
        emitInt(savedCounter);
        emit("_");
        emitHex(compilation_hash);
        emit("\n");
        
        emit("u8.writeBitOff ");
        emitLine(plcAddr);
        
        emit("__skip_reset_");
        emitInt(savedCounter);
        emit("_");
        emitHex(compilation_hash);
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
        if (plcReset[0] != '\0') {
            emit("u8.readBit ");
            emitLine(plcReset);
        } else {
            // No reset bit provided, push 0 (false) as default
            emitLine("u8.const 0");
        }
        
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
        if (plcLoad[0] != '\0') {
            emit("u8.readBit ");
            emitLine(plcLoad);
        } else {
            // No load bit provided, push 0 (false) as default
            emitLine("u8.const 0");
        }
        
        emit("ctd ");
        emit(plcCounter);
        emit(" ");
        emitLine(plcPreset);
    }

    // Emit a load instruction for an operand with explicit type
    // Used by math operations like +I operand where we know the type from the operator
    void emitLoadForType(const char* operand, const char* type) {
        char plcAddr[64];
        
        // Check if it's an immediate value
        if (operand[0] == '#') {
            // Immediate value
            emit(type);
            emit(".const ");
            emitLine(operand + 1); // Skip #
        } else {
            // Memory address - convert but use the type we were given
            const char* addrType = nullptr;
            convertTypedAddress(operand, plcAddr, &addrType);
            emit(type);
            emit(".load_from ");
            emitLine(plcAddr);
        }
        // Track the type we just loaded
        setExprType(type);
    }

    // Handle L (load)
    void handleLoad(const char* operand) {
        char plcAddr[64];
        const char* type = nullptr;
        
        // Check if it's an immediate value
        if (operand[0] == '#') {
            // Immediate value - use lookahead to determine correct type
            type = peekNextMathType();
            emit(type);
            emit(".const ");
            emitLine(operand + 1); // Skip #
        } else {
            // Memory address - check for typed address (MW, MD, etc.)
            convertTypedAddress(operand, plcAddr, &type);
            emit(type);
            emit(".load_from ");
            emitLine(plcAddr);
        }
        // Track the type we just loaded
        setExprType(type);
    }

    // Handle T (transfer)
    void handleTransfer(const char* operand) {
        char plcAddr[64];
        const char* destType = nullptr;
        convertTypedAddress(operand, plcAddr, &destType);
        
        // If current expression type differs from destination, emit CVT
        emitCvtIfNeeded(destType);
        
        emit(destType);
        emit(".move_to ");
        emitLine(plcAddr);
        
        // Clear expression type after transfer (value consumed)
        currentExprType = nullptr;
    }

    // Handle math operations
    // Siemens STL: +I/-I/*I//I (16-bit), +D/-D/*D//D (32-bit), +R/-R/*R//R (float)
    // Extended: ADD_U8, SUB_I32, MUL_F64, etc.
    void handleMath(const char* op) {
        const char* resultType = nullptr;
        
        // Standard Siemens STL operations (16-bit signed integer default)
        if (strEq(op, "+I")) { emitLine("i16.add"); resultType = "i16"; }
        else if (strEq(op, "-I")) { emitLine("i16.sub"); resultType = "i16"; }
        else if (strEq(op, "*I")) { emitLine("i16.mul"); resultType = "i16"; }
        else if (strEq(op, "/I")) { emitLine("i16.div"); resultType = "i16"; }
        
        // Siemens STL 32-bit operations (Double word)
        else if (strEq(op, "+D")) { emitLine("i32.add"); resultType = "i32"; }
        else if (strEq(op, "-D")) { emitLine("i32.sub"); resultType = "i32"; }
        else if (strEq(op, "*D")) { emitLine("i32.mul"); resultType = "i32"; }
        else if (strEq(op, "/D")) { emitLine("i32.div"); resultType = "i32"; }
        
        // Siemens STL Real (float) operations
        else if (strEq(op, "+R")) { emitLine("f32.add"); resultType = "f32"; }
        else if (strEq(op, "-R")) { emitLine("f32.sub"); resultType = "f32"; }
        else if (strEq(op, "*R")) { emitLine("f32.mul"); resultType = "f32"; }
        else if (strEq(op, "/R")) { emitLine("f32.div"); resultType = "f32"; }
        
        // MOD variants
        else if (strEq(op, "MOD")) { emitLine("i16.mod"); resultType = "i16"; }
        else if (strEq(op, "MOD_U8"))  { emitLine("u8.mod");  resultType = "u8"; }
        else if (strEq(op, "MOD_U16")) { emitLine("u16.mod"); resultType = "u16"; }
        else if (strEq(op, "MOD_U32")) { emitLine("u32.mod"); resultType = "u32"; }
        else if (strEq(op, "MOD_U64")) { emitLine("u64.mod"); resultType = "u64"; }
        else if (strEq(op, "MOD_I8"))  { emitLine("i8.mod");  resultType = "i8"; }
        else if (strEq(op, "MOD_I16")) { emitLine("i16.mod"); resultType = "i16"; }
        else if (strEq(op, "MOD_I32")) { emitLine("i32.mod"); resultType = "i32"; }
        else if (strEq(op, "MOD_I64")) { emitLine("i64.mod"); resultType = "i64"; }
        
        // NEG variants (signed types only)
        else if (strEq(op, "NEG")) { emitLine("i16.neg"); resultType = "i16"; }
        else if (strEq(op, "NEG_I8"))  { emitLine("i8.neg");  resultType = "i8"; }
        else if (strEq(op, "NEG_I16")) { emitLine("i16.neg"); resultType = "i16"; }
        else if (strEq(op, "NEG_I32")) { emitLine("i32.neg"); resultType = "i32"; }
        else if (strEq(op, "NEG_I64")) { emitLine("i64.neg"); resultType = "i64"; }
        else if (strEq(op, "NEG_F32")) { emitLine("f32.neg"); resultType = "f32"; }
        else if (strEq(op, "NEG_F64")) { emitLine("f64.neg"); resultType = "f64"; }
        
        // ABS variants (signed types only)
        else if (strEq(op, "ABS")) { emitLine("i16.abs"); resultType = "i16"; }
        else if (strEq(op, "ABS_I8"))  { emitLine("i8.abs");  resultType = "i8"; }
        else if (strEq(op, "ABS_I16")) { emitLine("i16.abs"); resultType = "i16"; }
        else if (strEq(op, "ABS_I32")) { emitLine("i32.abs"); resultType = "i32"; }
        else if (strEq(op, "ABS_I64")) { emitLine("i64.abs"); resultType = "i64"; }
        else if (strEq(op, "ABS_F32")) { emitLine("f32.abs"); resultType = "f32"; }
        else if (strEq(op, "ABS_F64")) { emitLine("f64.abs"); resultType = "f64"; }
        
        // Extended typed ADD operations
        else if (strEq(op, "ADD_U8"))  { emitLine("u8.add");  resultType = "u8"; }
        else if (strEq(op, "ADD_U16")) { emitLine("u16.add"); resultType = "u16"; }
        else if (strEq(op, "ADD_U32")) { emitLine("u32.add"); resultType = "u32"; }
        else if (strEq(op, "ADD_U64")) { emitLine("u64.add"); resultType = "u64"; }
        else if (strEq(op, "ADD_I8"))  { emitLine("i8.add");  resultType = "i8"; }
        else if (strEq(op, "ADD_I16")) { emitLine("i16.add"); resultType = "i16"; }
        else if (strEq(op, "ADD_I32")) { emitLine("i32.add"); resultType = "i32"; }
        else if (strEq(op, "ADD_I64")) { emitLine("i64.add"); resultType = "i64"; }
        else if (strEq(op, "ADD_F32")) { emitLine("f32.add"); resultType = "f32"; }
        else if (strEq(op, "ADD_F64")) { emitLine("f64.add"); resultType = "f64"; }
        
        // Extended typed SUB operations
        else if (strEq(op, "SUB_U8"))  { emitLine("u8.sub");  resultType = "u8"; }
        else if (strEq(op, "SUB_U16")) { emitLine("u16.sub"); resultType = "u16"; }
        else if (strEq(op, "SUB_U32")) { emitLine("u32.sub"); resultType = "u32"; }
        else if (strEq(op, "SUB_U64")) { emitLine("u64.sub"); resultType = "u64"; }
        else if (strEq(op, "SUB_I8"))  { emitLine("i8.sub");  resultType = "i8"; }
        else if (strEq(op, "SUB_I16")) { emitLine("i16.sub"); resultType = "i16"; }
        else if (strEq(op, "SUB_I32")) { emitLine("i32.sub"); resultType = "i32"; }
        else if (strEq(op, "SUB_I64")) { emitLine("i64.sub"); resultType = "i64"; }
        else if (strEq(op, "SUB_F32")) { emitLine("f32.sub"); resultType = "f32"; }
        else if (strEq(op, "SUB_F64")) { emitLine("f64.sub"); resultType = "f64"; }
        
        // Extended typed MUL operations
        else if (strEq(op, "MUL_U8"))  { emitLine("u8.mul");  resultType = "u8"; }
        else if (strEq(op, "MUL_U16")) { emitLine("u16.mul"); resultType = "u16"; }
        else if (strEq(op, "MUL_U32")) { emitLine("u32.mul"); resultType = "u32"; }
        else if (strEq(op, "MUL_U64")) { emitLine("u64.mul"); resultType = "u64"; }
        else if (strEq(op, "MUL_I8"))  { emitLine("i8.mul");  resultType = "i8"; }
        else if (strEq(op, "MUL_I16")) { emitLine("i16.mul"); resultType = "i16"; }
        else if (strEq(op, "MUL_I32")) { emitLine("i32.mul"); resultType = "i32"; }
        else if (strEq(op, "MUL_I64")) { emitLine("i64.mul"); resultType = "i64"; }
        else if (strEq(op, "MUL_F32")) { emitLine("f32.mul"); resultType = "f32"; }
        else if (strEq(op, "MUL_F64")) { emitLine("f64.mul"); resultType = "f64"; }
        
        // Extended typed DIV operations
        else if (strEq(op, "DIV_U8"))  { emitLine("u8.div");  resultType = "u8"; }
        else if (strEq(op, "DIV_U16")) { emitLine("u16.div"); resultType = "u16"; }
        else if (strEq(op, "DIV_U32")) { emitLine("u32.div"); resultType = "u32"; }
        else if (strEq(op, "DIV_U64")) { emitLine("u64.div"); resultType = "u64"; }
        else if (strEq(op, "DIV_I8"))  { emitLine("i8.div");  resultType = "i8"; }
        else if (strEq(op, "DIV_I16")) { emitLine("i16.div"); resultType = "i16"; }
        else if (strEq(op, "DIV_I32")) { emitLine("i32.div"); resultType = "i32"; }
        else if (strEq(op, "DIV_I64")) { emitLine("i64.div"); resultType = "i64"; }
        else if (strEq(op, "DIV_F32")) { emitLine("f32.div"); resultType = "f32"; }
        else if (strEq(op, "DIV_F64")) { emitLine("f64.div"); resultType = "f64"; }
        
        // Track result type after math operation
        if (resultType) {
            setExprType(resultType);
        }
    }

    // Handle compare operations
    // Siemens STL: ==I/<>I/>I etc (16-bit), ==D etc (32-bit), ==R etc (float)
    void handleCompare(const char* op) {
        // Standard Siemens STL 16-bit integer comparisons
        if (strEq(op, "==I")) { emitLine("i16.cmp_eq"); }
        else if (strEq(op, "<>I")) { emitLine("i16.cmp_neq"); }
        else if (strEq(op, ">I"))  { emitLine("i16.cmp_gt"); }
        else if (strEq(op, ">=I")) { emitLine("i16.cmp_gte"); }
        else if (strEq(op, "<I"))  { emitLine("i16.cmp_lt"); }
        else if (strEq(op, "<=I")) { emitLine("i16.cmp_lte"); }
        // Siemens STL 32-bit integer comparisons (Double word)
        else if (strEq(op, "==D")) { emitLine("i32.cmp_eq"); }
        else if (strEq(op, "<>D")) { emitLine("i32.cmp_neq"); }
        else if (strEq(op, ">D"))  { emitLine("i32.cmp_gt"); }
        else if (strEq(op, ">=D")) { emitLine("i32.cmp_gte"); }
        else if (strEq(op, "<D"))  { emitLine("i32.cmp_lt"); }
        else if (strEq(op, "<=D")) { emitLine("i32.cmp_lte"); }
        // Siemens STL Real (float) comparisons
        else if (strEq(op, "==R")) { emitLine("f32.cmp_eq"); }
        else if (strEq(op, "<>R")) { emitLine("f32.cmp_neq"); }
        else if (strEq(op, ">R"))  { emitLine("f32.cmp_gt"); }
        else if (strEq(op, ">=R")) { emitLine("f32.cmp_gte"); }
        else if (strEq(op, "<R"))  { emitLine("f32.cmp_lt"); }
        else if (strEq(op, "<=R")) { emitLine("f32.cmp_lte"); }
        
        // If BR was loaded for comparison (L BR pattern), AND result with BR
        if (br_loaded_for_comparison) {
            emitLine("u8.and");
            br_loaded_for_comparison = false;
        }
        
        network_has_rlo = true;
        setExprType("u8"); // Comparisons return boolean
    }

    // Handle INC/DEC operations
    // Siemens STL: INC, DEC (operate on accumulator)
    // Extended: INCI addr, INCD addr, DECI addr, DECD addr (increment/decrement memory location)
    void handleIncDec(const char* op, const char* operand) {
        bool isInc = (op[0] == 'I');
        char suffix = '\0';
        
        // Determine type suffix (I=16-bit, D=32-bit, B=8-bit, R=32-bit float)
        int len = 0;
        while (op[len]) len++;
        if (len >= 4) suffix = op[3]; // INCI, INCD, DECB, DECD, etc.
        
        const char* typePrefix = "i16"; // Default
        if (suffix == 'B') typePrefix = "u8";
        else if (suffix == 'I') typePrefix = "i16";
        else if (suffix == 'D') typePrefix = "i32";
        else if (suffix == 'R') typePrefix = "f32";
        
        if (operand && operand[0]) {
            // Memory location operation: use optimized single-instruction INC_MEM/DEC_MEM
            char plcAddr[64];
            const char* addrType = nullptr;
            convertTypedAddress(operand, plcAddr, &addrType);
            
            // Use specified type from instruction suffix, or infer from address
            if (suffix != '\0') {
                // Use the type from the instruction
            } else if (addrType) {
                typePrefix = addrType;
            }
            
            // Emit optimized single instruction: <type>.inc <addr> or <type>.dec <addr>
            emit(typePrefix);
            emit(isInc ? ".inc " : ".dec ");
            emitLine(plcAddr);
        } else {
            // Stack-only operation: push 1, add/sub
            emit(typePrefix);
            emitLine(".const 1");
            emit(typePrefix);
            emitLine(isInc ? ".add" : ".sub");
        }
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
            
            // = (assign) instruction OR ==I/==D/==R comparison OR =N (negated assign)
            if (c == '=') {
                advance();
                // Check if this is ==I/==D/==R comparison
                if (peek() == '=') {
                    advance(); // consume second =
                    char typeSuffix = peek();
                    if (typeSuffix == 'I' || typeSuffix == 'i') {
                        advance();
                        processInstruction("==I");
                        continue;
                    }
                    if (typeSuffix == 'D' || typeSuffix == 'd') {
                        advance();
                        processInstruction("==D");
                        continue;
                    }
                    if (typeSuffix == 'R' || typeSuffix == 'r') {
                        advance();
                        processInstruction("==R");
                        continue;
                    }
                    emit("// Unknown comparison: ==\n");
                    continue;
                }
                // Check for =N (negated assign) - writes NOT(RLO) but preserves original
                if (peek() == 'N' || peek() == 'n') {
                    advance(); // consume 'N'
                    skipWhitespace();
                    char operand[64];
                    readIdentifier(operand, sizeof(operand));
                    handleAssignNegated(operand);
                    continue;
                }
                // It's the regular assign instruction
                skipWhitespace();
                char operand[64];
                readIdentifier(operand, sizeof(operand));
                handleAssign(operand);
                continue;
            }
            
            // Math operators (+I/-I/*I//I, +D/-D/*D//D, +R/-R/*R//R)
            if (c == '+' || c == '-' || c == '*' || c == '/') {
                char op[4];
                op[0] = advance();
                char typeSuffix = peek();
                if (typeSuffix == 'I' || typeSuffix == 'i' ||
                    typeSuffix == 'D' || typeSuffix == 'd' ||
                    typeSuffix == 'R' || typeSuffix == 'r') {
                    op[1] = advance();
                    op[2] = '\0';
                    // Convert to uppercase
                    if (op[1] >= 'a' && op[1] <= 'z') op[1] -= 32;
                    processInstruction(op);
                    continue;
                }
                // Not a math op - might be start of number, rewind
                // Actually, we can't easily rewind, so emit unknown
                emit("// Unknown operator: ");
                emitLine(op);
                continue;
            }
            
            // Comparison operators (==I/==D/==R, <>I/<>D/<>R, >I/>D/>R, >=I/>=D/>=R, <I/<D/<R, <=I/<=D/<=R)
            if (c == '<' || c == '>') {
                char op[4];
                op[0] = advance();
                int idx = 1;
                // Check for second char (=, >, or I/D/R)
                char c2 = peek();
                if (c2 == '=' || c2 == '>') {
                    op[idx++] = advance();
                }
                // Now expect I, D, or R type suffix
                char typeSuffix = peek();
                if (typeSuffix == 'I' || typeSuffix == 'i' ||
                    typeSuffix == 'D' || typeSuffix == 'd' ||
                    typeSuffix == 'R' || typeSuffix == 'r') {
                    op[idx++] = (typeSuffix >= 'a' && typeSuffix <= 'z') ? typeSuffix - 32 : typeSuffix;
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
            // Check if next word is "BR" for CLR BR (clear BR stack)
            skipWhitespace();
            char nextWord[16] = "";
            int lookAheadPos = pos;
            readIdentifier(nextWord, sizeof(nextWord));
            toUpper(nextWord);
            if (strEq(nextWord, "BR")) {
                emitLine("br.clr");
                return;
            }
            // Not "CLR BR", restore position and handle as normal CLR (RLO=0)
            pos = lookAheadPos;
            emitLine("u8.const 0");
            network_has_rlo = true;
            return;
        }
        if (strEq(upperInstr, "NOT")) {
            emitLine("u8.not");
            return;
        }
        
        // TAP (VovkPLCRuntime extension) - duplicate RLO on stack to preserve it
        // This allows the next instruction to consume one copy while keeping one for further use
        if (strEq(upperInstr, "TAP")) {
            // Check if TAP was already consumed by a preceding S/R instruction
            if (tap_consumed) {
                tap_consumed = false;  // Reset the flag
                network_has_rlo = true;  // RLO is still on stack (copy was emitted by S/R)
                return;  // Skip emitting another copy
            }
            emitLine("u8.copy");  // Duplicate top of stack (RLO)
            network_has_rlo = true;
            return;
        }
        
        // ============ BR Stack Instructions ============
        // SAVE - Save RLO to BR stack
        // L BR - Load (read) RLO from BR stack (non-destructive)
        // DROP BR - Drop top of BR stack
        // CLR BR - Clear BR stack
        
        if (strEq(upperInstr, "SAVE")) {
            emitLine("br.save");
            network_has_rlo = false;  // SAVE moves RLO from stack to BR, so stack is now empty
            return;
        }
        
        // "L BR" - two tokens: "L" followed by "BR"
        if (strEq(upperInstr, "L")) {
            skipWhitespace();
            char nextWord[16] = "";
            // Check if next word is "BR"
            int lookAheadPos = pos;
            readIdentifier(nextWord, sizeof(nextWord));
            toUpper(nextWord);
            if (strEq(nextWord, "BR")) {
                emitLine("br.read");
                network_has_rlo = true;  // BR value is now on stack as RLO
                br_loaded_for_comparison = true;  // Track that BR was loaded for combining with comparison
                return;
            }
            // Not "L BR", restore position so normal L handler can process it
            pos = lookAheadPos;
            // Fall through to normal L handling below
        }
        
        // "DROP BR" - two tokens
        if (strEq(upperInstr, "DROP")) {
            skipWhitespace();
            char nextWord[16] = "";
            readIdentifier(nextWord, sizeof(nextWord));
            toUpper(nextWord);
            if (strEq(nextWord, "BR")) {
                emitLine("br.drop");
                return;
            }
            // Not "DROP BR", this is an error - DROP without BR is not valid STL
            setError("Expected 'BR' after 'DROP'");
            return;
        }
        
        // "CLR BR" - clear BR stack (note: "CLR" by itself is RLO=0, not BR)
        // We already handled plain "CLR" above, this handles "CLR BR"
        // Actually, need to check for this before the CLR handler above.
        // For now, just handle as a special case if we see "CLR" and next is "BR"
        // Let me move this check to before the CLR handler above
        // Actually, the architecture here means we've already handled "CLR" above.
        // We need to look ahead when we see "CLR" to check if "BR" follows.
        
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
        
        // Siemens STL standard operations - may have optional operand
        // +I operand means: load operand, then add
        if (strEq(upperInstr, "+I") || strEq(upperInstr, "-I") || 
            strEq(upperInstr, "*I") || strEq(upperInstr, "/I") ||
            strEq(upperInstr, "+D") || strEq(upperInstr, "-D") || 
            strEq(upperInstr, "*D") || strEq(upperInstr, "/D") ||
            strEq(upperInstr, "+R") || strEq(upperInstr, "-R") || 
            strEq(upperInstr, "*R") || strEq(upperInstr, "/R")) {
            skipWhitespace();
            if (peek() != '\0' && peek() != '\n' && peek() != '\r' && peek() != '/' && peek() != ';') {
                // Has operand - emit load first
                readIdentifier(operand1, sizeof(operand1));
                // Determine type from instruction suffix
                char suffix = upperInstr[1]; // I, D, or R
                const char* typePrefix = suffix == 'I' ? "i16" : suffix == 'D' ? "i32" : "f32";
                emitLoadForType(operand1, typePrefix);
            }
            handleMath(upperInstr);
            return;
        }
        if (strEq(upperInstr, "MOD") || strEq(upperInstr, "NEG") || strEq(upperInstr, "ABS")) {
            handleMath(upperInstr);
            return;
        }
        
        // ============ Increment/Decrement Operations ============
        
        // Siemens STL: INC, DEC (operate on stack/accumulator)
        // Extended: INCI addr, INCD addr, INCB addr, INCR addr (with memory address)
        //           DECI addr, DECD addr, DECB addr, DECR addr
        if (strEq(upperInstr, "INC") || strEq(upperInstr, "DEC")) {
            // Basic INC/DEC - operate on stack (i16 default)
            handleIncDec(upperInstr, nullptr);
            return;
        }
        if (strEq(upperInstr, "INCI") || strEq(upperInstr, "INCD") || 
            strEq(upperInstr, "INCB") || strEq(upperInstr, "INCR") ||
            strEq(upperInstr, "DECI") || strEq(upperInstr, "DECD") ||
            strEq(upperInstr, "DECB") || strEq(upperInstr, "DECR")) {
            skipWhitespace();
            readIdentifier(operand1, sizeof(operand1));
            handleIncDec(upperInstr, operand1);
            return;
        }
        
        // Extended typed MOD operations
        if (startsWith(upperInstr, "MOD_")) {
            handleMath(upperInstr);
            return;
        }
        
        // Extended typed NEG operations
        if (startsWith(upperInstr, "NEG_")) {
            handleMath(upperInstr);
            return;
        }
        
        // Extended typed ABS operations
        if (startsWith(upperInstr, "ABS_")) {
            handleMath(upperInstr);
            return;
        }
        
        // Extended typed ADD/SUB/MUL/DIV operations
        if (startsWith(upperInstr, "ADD_") || startsWith(upperInstr, "SUB_") ||
            startsWith(upperInstr, "MUL_") || startsWith(upperInstr, "DIV_")) {
            handleMath(upperInstr);
            return;
        }
        
        // ============ Compare Operations ============
        
        // Siemens STL standard comparisons - may have optional operand
        // ==I operand means: load operand, then compare
        if (strEq(upperInstr, "==I") || strEq(upperInstr, "<>I") ||
            strEq(upperInstr, ">I") || strEq(upperInstr, ">=I") ||
            strEq(upperInstr, "<I") || strEq(upperInstr, "<=I") ||
            strEq(upperInstr, "==D") || strEq(upperInstr, "<>D") ||
            strEq(upperInstr, ">D") || strEq(upperInstr, ">=D") ||
            strEq(upperInstr, "<D") || strEq(upperInstr, "<=D") ||
            strEq(upperInstr, "==R") || strEq(upperInstr, "<>R") ||
            strEq(upperInstr, ">R") || strEq(upperInstr, ">=R") ||
            strEq(upperInstr, "<R") || strEq(upperInstr, "<=R")) {
            skipWhitespace();
            if (peek() != '\0' && peek() != '\n' && peek() != '\r' && peek() != '/' && peek() != ';') {
                // Has operand - emit load first
                readIdentifier(operand1, sizeof(operand1));
                // Determine type from instruction suffix (last char)
                int len = 0;
                while (upperInstr[len]) len++;
                char suffix = upperInstr[len - 1]; // I, D, or R
                const char* typePrefix = suffix == 'I' ? "i16" : suffix == 'D' ? "i32" : "f32";
                emitLoadForType(operand1, typePrefix);
            }
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
