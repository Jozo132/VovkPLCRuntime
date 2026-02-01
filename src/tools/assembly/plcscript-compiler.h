// plcscript-compiler.h - PLCScript to PLCASM Transpiler
// A simple JavaScript subset language with PLC-specific variable declarations
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

// ============================================================================
// PLCScript Language Specification
// ============================================================================
//
// PLCScript is a simple JavaScript subset language with PLC-specific extensions for
// variable declarations with memory location and address.
//
// ============================================================================
// Variable Declarations (PLC Extension)
// ============================================================================
//
// Syntax variants:
//   let <name>: <type> @ <address> = <initial_value>?;   // Explicit PLC address
//   let <name>: <type> @ auto = <initial_value>?;        // Auto-assigned address
//   let <name>: <type> = <initial_value>?;               // Local variable (auto-assigned)
//
// Types:
//   bool        - Boolean (1 bit)
//   i8, i16, i32, i64  - Signed integers
//   u8, u16, u32, u64  - Unsigned integers
//   f32, f64    - Floating point
//
// Locations (PLC memory areas - just prefix + number, type from declaration):
//   X<n>        - Input byte/word/dword (e.g., X0, X1)
//   X<n>.<b>    - Input bit (e.g., X0.0, X1.7)
//   Y<n>        - Output byte/word/dword (e.g., Y0)
//   Y<n>.<b>    - Output bit (e.g., Y0.0)
//   M<n>        - Marker byte/word/dword (e.g., M10, M20)
//   M<n>.<b>    - Marker bit (e.g., M0.0)
//   T<n>        - Timer (e.g., T0)
//   C<n>        - Counter (e.g., C0)
//   S<n>        - System area (e.g., S0)
//   auto        - Compiler auto-assigns address from reserved area
//
// Note: The type determines memory width, address is just location:
//   let x: u8 @ M10      // 1 byte at M10
//   let y: i16 @ M10     // 2 bytes at M10
//   let z: i32 @ M10     // 4 bytes at M10
//   let b: bool @ M0.0   // bit 0 of M0
//
// Legacy MW/MD syntax still accepted but not recommended:
//   MW10 = M10 (word), MD10 = M10 (dword)
//
// Auto-addressing ranges (default):
//   Bits:   M112.0+ (M900+ in bit numbering)
//   Bytes:  M950+
//   Words:  MW960+
//   DWords: MD980+
//
// Examples:
//   let startButton: bool @ X0.0          // Input bit
//   let motorRunning: bool @ Y0.0         // Output bit
//   let counter: i16 @ M10 = 0            // Word at M10
//   let temperature: f32 @ M20            // Float (4 bytes) at M20
//   let flag: bool @ auto                 // Auto-assigned bit address
//   let tempValue: i16 @ auto = 100       // Auto-assigned word address
//   let localVar: i32 = 0                 // Local variable (auto-assigned)
//   const MAX_TEMP: f32 = 100.0           // Compile-time constant
//
// ============================================================================
// Statements (semicolons are optional)
// ============================================================================
//
// Assignment:
//   variable = expression
//   Y0.0 = X0.0 && X0.1       // Direct PLC address assignment
//
// If/Else (braces optional for single statements):
//   if (condition) statement
//   if (condition) { statements }
//   if (condition) { statements } else { statements }
//   if (condition) { statements } else if (condition) { statements }
//
// While loop (braces optional for single statements):
//   while (condition) statement
//   while (condition) { statements }
//
// For loop (braces optional for single statements):
//   for (let i: i16 @ MW100 = 0; i < 10; i++) statement
//   for (let i: i16 @ MW100 = 0; i < 10; i++) { statements }
//
// ============================================================================
// Expressions
// ============================================================================
//
// Arithmetic: +, -, *, /, %
// Comparison: ==, !=, <, >, <=, >=
// Logical: &&, ||, !
// Bitwise: &, |, ^, ~, <<, >>
// Ternary: condition ? trueExpr : falseExpr
//
// ============================================================================
// Built-in PLC Functions
// ============================================================================
//
// Timers:
//   TON(timerAddr, condition, preset)  - On-delay timer
//   TOF(timerAddr, condition, preset)  - Off-delay timer
//   TP(timerAddr, condition, preset)   - Pulse timer
//
// Counters:
//   CTU(counterAddr, countUp, reset, preset)  - Count up
//   CTD(counterAddr, countDown, load, preset) - Count down
//
// Edge detection:
//   risingEdge(input, memoryBit)   - Detect rising edge
//   fallingEdge(input, memoryBit) - Detect falling edge
//
// ============================================================================

#define PLCSCRIPT_MAX_OUTPUT_SIZE 65536
#define PLCSCRIPT_MAX_SYMBOLS 256
#define PLCSCRIPT_MAX_IDENTIFIER_LEN 64
#define PLCSCRIPT_MAX_SCOPES 32
#define PLCSCRIPT_MAX_TEMP_VARS 64

// ============================================================================
// Token Types
// ============================================================================

enum PLCScriptTokenType {
    PSTOK_EOF = 0,
    PSTOK_ERROR,
    
    // Literals
    PSTOK_INTEGER,      // 123, -456, 0xFF
    PSTOK_FLOAT,        // 1.5, -3.14, 1e10
    PSTOK_BOOL_TRUE,    // true
    PSTOK_BOOL_FALSE,   // false
    PSTOK_STRING,       // "hello"
    
    // Identifiers and keywords
    PSTOK_IDENTIFIER,   // variable/function names
    PSTOK_LET,          // let
    PSTOK_CONST,        // const
    PSTOK_IF,           // if
    PSTOK_ELSE,         // else
    PSTOK_WHILE,        // while
    PSTOK_FOR,          // for
    PSTOK_FUNCTION,     // function
    PSTOK_RETURN,       // return
    PSTOK_BREAK,        // break
    PSTOK_CONTINUE,     // continue
    
    // Type keywords
    PSTOK_TYPE_BOOL,
    PSTOK_TYPE_I8,
    PSTOK_TYPE_I16,
    PSTOK_TYPE_I32,
    PSTOK_TYPE_I64,
    PSTOK_TYPE_U8,
    PSTOK_TYPE_U16,
    PSTOK_TYPE_U32,
    PSTOK_TYPE_U64,
    PSTOK_TYPE_F32,
    PSTOK_TYPE_F64,
    
    // Operators
    PSTOK_PLUS,         // +
    PSTOK_MINUS,        // -
    PSTOK_STAR,         // *
    PSTOK_SLASH,        // /
    PSTOK_PERCENT,      // %
    PSTOK_AMPERSAND,    // &
    PSTOK_PIPE,         // |
    PSTOK_CARET,        // ^
    PSTOK_TILDE,        // ~
    PSTOK_BANG,         // !
    PSTOK_LT,           // <
    PSTOK_GT,           // >
    PSTOK_EQ,           // =
    PSTOK_DOT,          // .
    PSTOK_AT,           // @ (PLC address specifier)
    PSTOK_HASH,         // # (constant prefix)
    PSTOK_COLON,        // :
    PSTOK_SEMICOLON,    // ;
    PSTOK_COMMA,        // ,
    PSTOK_QUESTION,     // ?
    
    // Compound operators
    PSTOK_PLUS_PLUS,    // ++
    PSTOK_MINUS_MINUS,  // --
    PSTOK_PLUS_EQ,      // +=
    PSTOK_MINUS_EQ,     // -=
    PSTOK_STAR_EQ,      // *=
    PSTOK_SLASH_EQ,     // /=
    PSTOK_EQ_EQ,        // ==
    PSTOK_BANG_EQ,      // !=
    PSTOK_LT_EQ,        // <=
    PSTOK_GT_EQ,        // >=
    PSTOK_AMP_AMP,      // &&
    PSTOK_PIPE_PIPE,    // ||
    PSTOK_LT_LT,        // <<
    PSTOK_GT_GT,        // >>
    PSTOK_ARROW,        // =>
    
    // Delimiters
    PSTOK_LPAREN,       // (
    PSTOK_RPAREN,       // )
    PSTOK_LBRACE,       // {
    PSTOK_RBRACE,       // }
    PSTOK_LBRACKET,     // [
    PSTOK_RBRACKET,     // ]
    
    // PLC-specific
    PSTOK_ADDRESS,      // X0.0, Y0.0, M10, MW10, MD10, T0, C0
    PSTOK_AUTO,         // auto (auto-address assignment)
};

// ============================================================================
// Token Structure
// ============================================================================

struct PLCScriptToken {
    PLCScriptTokenType type;
    int line;
    int column;
    
    // Value storage
    char text[PLCSCRIPT_MAX_IDENTIFIER_LEN];
    int textLen;
    
    // Parsed values
    union {
        int64_t intValue;
        double floatValue;
        bool boolValue;
    };
};

// ============================================================================
// Symbol Types
// ============================================================================

enum PLCScriptVarType {
    PSTYPE_VOID = 0,
    PSTYPE_BOOL,
    PSTYPE_I8,
    PSTYPE_I16,
    PSTYPE_I32,
    PSTYPE_I64,
    PSTYPE_U8,
    PSTYPE_U16,
    PSTYPE_U32,
    PSTYPE_U64,
    PSTYPE_F32,
    PSTYPE_F64,
};

// ============================================================================
// Symbol Table Entry
// ============================================================================

struct PLCScriptSymbol {
    char name[PLCSCRIPT_MAX_IDENTIFIER_LEN];
    PLCScriptVarType type;
    bool isConst;
    bool isBit;           // True if this is a bit address
    bool isLocal;         // True if this is a local stack variable (no PLC address)
    bool isAutoAddress;   // True if address was auto-assigned
    
    // Memory location (PLC address)
    char address[32];     // Original address string (e.g., "X0.0", "MW10")
    u32 memoryOffset;     // Computed memory offset
    u8 bitIndex;          // Bit position (0-7) if isBit
    
    // Stack location (for local variables)
    int stackSlot;        // Stack slot index for local variables (-1 if not local)
    
    // Initial value
    bool hasInitializer;
    union {
        int64_t initInt;
        double initFloat;
        bool initBool;
    };
    
    // Source location
    int declarationLine;
    int declarationColumn;
    
    // Scope level (for local variables)
    int scopeLevel;
};

// ============================================================================
// PLCScript Compiler Class
// ============================================================================

class PLCScriptCompiler {
public:
    // Input source
    char* source = nullptr;
    int sourceLength = 0;
    
    // Output PLCASM
    char output[PLCSCRIPT_MAX_OUTPUT_SIZE];
    int outputLength = 0;
    
    // Error handling
    char errorMessage[512];
    int errorLine = 0;
    int errorColumn = 0;
    bool hasError = false;
    
    // Lexer state
    int pos = 0;
    int currentLine = 1;
    int currentColumn = 1;
    PLCScriptToken currentToken;
    PLCScriptToken peekToken;
    bool hasPeekToken = false;
    
    // Symbol table
    PLCScriptSymbol symbols[PLCSCRIPT_MAX_SYMBOLS];
    int symbolCount = 0;
    
    // Scope management
    int currentScopeLevel = 0;
    
    // Label generation
    int labelCounter = 0;
    uint32_t compilationHash = 0;
    
    // Temporary variable counter (for intermediate calculations)
    int tempVarCounter = 0;
    
    // Auto-address counters (for auto-assigned PLC addresses)
    int autoAddrBit = 900;    // M900.0+ for auto-assigned bits
    int autoAddrByte = 950;   // M950+ for auto-assigned bytes
    int autoAddrWord = 960;   // MW960+ for auto-assigned words
    int autoAddrDword = 980;  // MD980+ for auto-assigned dwords
    
    // Stack slot management (for local variables)
    int nextStackSlot = 0;    // Next available stack slot
    int maxStackSlots = 0;    // Maximum stack slots used
    
    // Loop context for break/continue
    struct LoopContext {
        char breakLabel[64];
        char continueLabel[64];
    };
    LoopContext loopStack[PLCSCRIPT_MAX_SCOPES];
    int loopStackDepth = 0;
    
    // Type inference hint for expressions
    // When set, integer literals will be emitted as this type instead of i32
    PLCScriptVarType targetType = PSTYPE_VOID;
    
    PLCScriptCompiler() {
        reset();
    }
    
    void reset() {
        source = nullptr;
        sourceLength = 0;
        outputLength = 0;
        output[0] = '\0';
        errorMessage[0] = '\0';
        errorLine = 0;
        errorColumn = 0;
        hasError = false;
        pos = 0;
        currentLine = 1;
        currentColumn = 1;
        hasPeekToken = false;
        symbolCount = 0;
        currentScopeLevel = 0;
        labelCounter = 0;
        tempVarCounter = 0;
        loopStackDepth = 0;
        // Reset type inference hint
        targetType = PSTYPE_VOID;
        // Reset auto-address counters
        autoAddrBit = 900;
        autoAddrByte = 950;
        autoAddrWord = 960;
        autoAddrDword = 980;
        // Reset stack slot management
        nextStackSlot = 0;
        maxStackSlots = 0;
        memset(&currentToken, 0, sizeof(currentToken));
        memset(&peekToken, 0, sizeof(peekToken));
        memset(symbols, 0, sizeof(symbols));
        memset(loopStack, 0, sizeof(loopStack));
    }
    
    void setSource(char* src, int length) {
        reset();
        source = src;
        sourceLength = length;
        compilationHash = computeHash(src, length);
    }
    
    uint32_t computeHash(const char* data, int length) {
        uint32_t hash = 0x811c9dc5;
        for (int i = 0; i < length; i++) {
            hash ^= (uint8_t)data[i];
            hash *= 0x01000193;
        }
        return hash;
    }
    
    // ========================================================================
    // Output helpers
    // ========================================================================
    
    void emit(const char* str) {
        while (*str && outputLength < PLCSCRIPT_MAX_OUTPUT_SIZE - 1) {
            output[outputLength++] = *str++;
        }
        output[outputLength] = '\0';
    }
    
    void emitLine(const char* str) {
        emit(str);
        emit("\n");
    }
    
    void emitIndent(int level = 1) {
        for (int i = 0; i < level; i++) {
            emit("    ");
        }
    }
    
    void emitInt(int64_t value) {
        char buf[32];
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
        for (int j = i - 1; j >= 0; j--) {
            if (outputLength < PLCSCRIPT_MAX_OUTPUT_SIZE - 1) {
                output[outputLength++] = buf[j];
            }
        }
        output[outputLength] = '\0';
    }
    
    void emitFloat(double value) {
        // Simple float to string conversion
        if (value < 0) {
            emit("-");
            value = -value;
        }
        int64_t intPart = (int64_t)value;
        double fracPart = value - intPart;
        emitInt(intPart);
        emit(".");
        // 6 decimal places
        for (int i = 0; i < 6; i++) {
            fracPart *= 10;
            int digit = (int)fracPart;
            char c = '0' + digit;
            if (outputLength < PLCSCRIPT_MAX_OUTPUT_SIZE - 1) {
                output[outputLength++] = c;
            }
            fracPart -= digit;
        }
        output[outputLength] = '\0';
    }
    
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
    
    void generateLabel(char* buf, const char* prefix) {
        int i = 0;
        buf[i++] = '_'; buf[i++] = '_';
        const char* p = prefix;
        while (*p && i < 48) buf[i++] = *p++;
        buf[i++] = '_';
        // Add counter
        int cnt = labelCounter++;
        char numBuf[16];
        int ni = 0;
        if (cnt == 0) {
            numBuf[ni++] = '0';
        } else {
            while (cnt > 0) {
                numBuf[ni++] = '0' + (cnt % 10);
                cnt /= 10;
            }
        }
        for (int j = ni - 1; j >= 0 && i < 56; j--) {
            buf[i++] = numBuf[j];
        }
        buf[i++] = '_';
        // Add hash
        const char hexChars[] = "0123456789abcdef";
        uint32_t h = compilationHash;
        for (int j = 0; j < 8 && i < 63; j++) {
            buf[i++] = hexChars[(h >> 28) & 0xF];
            h <<= 4;
        }
        buf[i] = '\0';
    }
    
    void emitLabel(const char* label) {
        emit(label);
        emitLine(":");
    }
    
    // ========================================================================
    // Error handling
    // ========================================================================
    
    void setError(const char* msg) {
        if (hasError) return;
        hasError = true;
        errorLine = currentLine;
        errorColumn = currentColumn;
        int i = 0;
        while (msg[i] && i < 511) {
            errorMessage[i] = msg[i];
            i++;
        }
        errorMessage[i] = '\0';
    }
    
    void setErrorAt(const char* msg, int line, int col) {
        if (hasError) return;
        hasError = true;
        errorLine = line;
        errorColumn = col;
        int i = 0;
        while (msg[i] && i < 511) {
            errorMessage[i] = msg[i];
            i++;
        }
        errorMessage[i] = '\0';
    }
    
    // ========================================================================
    // Lexer
    // ========================================================================
    
    char peek(int offset = 0) {
        int p = pos + offset;
        if (p >= sourceLength) return '\0';
        return source[p];
    }
    
    char advance() {
        if (pos >= sourceLength) return '\0';
        char c = source[pos++];
        if (c == '\n') {
            currentLine++;
            currentColumn = 1;
        } else {
            currentColumn++;
        }
        return c;
    }
    
    void skipWhitespaceAndComments() {
        while (pos < sourceLength) {
            char c = peek();
            
            // Whitespace
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                advance();
                continue;
            }
            
            // Single-line comment
            if (c == '/' && peek(1) == '/') {
                advance(); advance();
                while (pos < sourceLength && peek() != '\n') {
                    advance();
                }
                continue;
            }
            
            // Multi-line comment
            if (c == '/' && peek(1) == '*') {
                advance(); advance();
                while (pos < sourceLength) {
                    if (peek() == '*' && peek(1) == '/') {
                        advance(); advance();
                        break;
                    }
                    advance();
                }
                continue;
            }
            
            break;
        }
    }
    
    bool isAlpha(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }
    
    bool isDigit(char c) {
        return c >= '0' && c <= '9';
    }
    
    bool isHexDigit(char c) {
        return isDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }
    
    bool isAlphaNum(char c) {
        return isAlpha(c) || isDigit(c);
    }
    
    bool strEq(const char* a, const char* b) {
        while (*a && *b) {
            if (*a != *b) return false;
            a++; b++;
        }
        return *a == *b;
    }
    
    bool strEqCI(const char* a, const char* b) {
        while (*a && *b) {
            char ca = (*a >= 'A' && *a <= 'Z') ? *a + 32 : *a;
            char cb = (*b >= 'A' && *b <= 'Z') ? *b + 32 : *b;
            if (ca != cb) return false;
            a++; b++;
        }
        return *a == *b;
    }
    
    PLCScriptTokenType checkKeyword(const char* text) {
        if (strEq(text, "let")) return PSTOK_LET;
        if (strEq(text, "const")) return PSTOK_CONST;
        if (strEq(text, "if")) return PSTOK_IF;
        if (strEq(text, "else")) return PSTOK_ELSE;
        if (strEq(text, "while")) return PSTOK_WHILE;
        if (strEq(text, "for")) return PSTOK_FOR;
        if (strEq(text, "function")) return PSTOK_FUNCTION;
        if (strEq(text, "return")) return PSTOK_RETURN;
        if (strEq(text, "break")) return PSTOK_BREAK;
        if (strEq(text, "continue")) return PSTOK_CONTINUE;
        if (strEq(text, "true")) return PSTOK_BOOL_TRUE;
        if (strEq(text, "false")) return PSTOK_BOOL_FALSE;
        if (strEq(text, "auto")) return PSTOK_AUTO;
        
        // Type keywords
        if (strEq(text, "bool")) return PSTOK_TYPE_BOOL;
        if (strEq(text, "i8")) return PSTOK_TYPE_I8;
        if (strEq(text, "i16")) return PSTOK_TYPE_I16;
        if (strEq(text, "i32")) return PSTOK_TYPE_I32;
        if (strEq(text, "i64")) return PSTOK_TYPE_I64;
        if (strEq(text, "u8")) return PSTOK_TYPE_U8;
        if (strEq(text, "u16")) return PSTOK_TYPE_U16;
        if (strEq(text, "u32")) return PSTOK_TYPE_U32;
        if (strEq(text, "u64")) return PSTOK_TYPE_U64;
        if (strEq(text, "f32")) return PSTOK_TYPE_F32;
        if (strEq(text, "f64")) return PSTOK_TYPE_F64;
        
        return PSTOK_IDENTIFIER;
    }
    
    bool isPLCAddressStart(char c) {
        return c == 'X' || c == 'x' || c == 'Y' || c == 'y' || 
               c == 'M' || c == 'm' || c == 'T' || c == 't' || 
               c == 'C' || c == 'c' || c == 'S' || c == 's';
    }
    
    bool isPLCAddress(const char* text) {
        // PLC addresses are: X0, Y0, M0, T0, C0, S0 (and M0.0 for bits)
        // Legacy MW/MD/XW/XD etc. still accepted for compatibility
        if (!text || !text[0]) return false;
        char first = text[0];
        if (first >= 'a' && first <= 'z') first -= 32;
        
        // Valid prefixes: X, Y, M, T, C, S
        if (first != 'X' && first != 'Y' && first != 'M' && 
            first != 'T' && first != 'C' && first != 'S') {
            return false;
        }
        
        // Next char should be digit, or W/D/B for legacy syntax
        if (text[1] >= '0' && text[1] <= '9') return true;
        
        // Check for legacy W/D/B suffix (MW10, MD20, etc.)
        char second = text[1];
        if (second >= 'a' && second <= 'z') second -= 32;
        if ((second == 'W' || second == 'D' || second == 'B') && 
            text[2] >= '0' && text[2] <= '9') return true;
        
        return false;
    }
    
    void scanToken() {
        skipWhitespaceAndComments();
        
        currentToken.line = currentLine;
        currentToken.column = currentColumn;
        currentToken.textLen = 0;
        currentToken.text[0] = '\0';
        
        if (pos >= sourceLength) {
            currentToken.type = PSTOK_EOF;
            return;
        }
        
        char c = peek();
        
        // Number (integer or float)
        if (isDigit(c) || (c == '-' && isDigit(peek(1)))) {
            bool isNegative = false;
            if (c == '-') {
                isNegative = true;
                currentToken.text[currentToken.textLen++] = advance();
            }
            
            // Check for hex
            if (peek() == '0' && (peek(1) == 'x' || peek(1) == 'X')) {
                currentToken.text[currentToken.textLen++] = advance(); // 0
                currentToken.text[currentToken.textLen++] = advance(); // x
                while (isHexDigit(peek())) {
                    currentToken.text[currentToken.textLen++] = advance();
                }
                currentToken.text[currentToken.textLen] = '\0';
                // Parse hex
                int64_t val = 0;
                for (int i = (isNegative ? 3 : 2); i < currentToken.textLen; i++) {
                    char hc = currentToken.text[i];
                    int digit;
                    if (hc >= '0' && hc <= '9') digit = hc - '0';
                    else if (hc >= 'a' && hc <= 'f') digit = 10 + hc - 'a';
                    else digit = 10 + hc - 'A';
                    val = val * 16 + digit;
                }
                currentToken.intValue = isNegative ? -val : val;
                currentToken.type = PSTOK_INTEGER;
                return;
            }
            
            // Decimal number
            bool hasDecimal = false;
            while (isDigit(peek()) || peek() == '.') {
                if (peek() == '.') {
                    if (hasDecimal) break; // Second dot ends number
                    // Check if it's actually a decimal point (not bit notation)
                    if (!isDigit(peek(1))) break;
                    hasDecimal = true;
                }
                currentToken.text[currentToken.textLen++] = advance();
            }
            
            // Check for exponent
            if (peek() == 'e' || peek() == 'E') {
                hasDecimal = true; // Treat as float
                currentToken.text[currentToken.textLen++] = advance();
                if (peek() == '+' || peek() == '-') {
                    currentToken.text[currentToken.textLen++] = advance();
                }
                while (isDigit(peek())) {
                    currentToken.text[currentToken.textLen++] = advance();
                }
            }
            
            currentToken.text[currentToken.textLen] = '\0';
            
            if (hasDecimal) {
                // Parse float
                double val = 0.0;
                double frac = 0.0;
                double div = 1.0;
                bool inFrac = false;
                bool neg = false;
                int expVal = 0;
                bool negExp = false;
                bool inExp = false;
                
                for (int i = 0; i < currentToken.textLen; i++) {
                    char fc = currentToken.text[i];
                    if (fc == '-' && i == 0) { neg = true; continue; }
                    if (fc == '.') { inFrac = true; continue; }
                    if (fc == 'e' || fc == 'E') { inExp = true; continue; }
                    if (inExp) {
                        if (fc == '-') { negExp = true; continue; }
                        if (fc == '+') continue;
                        expVal = expVal * 10 + (fc - '0');
                    } else if (inFrac) {
                        div *= 10.0;
                        frac += (fc - '0') / div;
                    } else {
                        val = val * 10 + (fc - '0');
                    }
                }
                val = val + frac;
                if (inExp) {
                    double mult = 1.0;
                    for (int i = 0; i < expVal; i++) mult *= 10.0;
                    if (negExp) val /= mult;
                    else val *= mult;
                }
                if (neg) val = -val;
                currentToken.floatValue = val;
                currentToken.type = PSTOK_FLOAT;
            } else {
                // Parse integer
                int64_t val = 0;
                bool neg = false;
                for (int i = 0; i < currentToken.textLen; i++) {
                    if (currentToken.text[i] == '-') { neg = true; continue; }
                    val = val * 10 + (currentToken.text[i] - '0');
                }
                currentToken.intValue = neg ? -val : val;
                currentToken.type = PSTOK_INTEGER;
            }
            return;
        }
        
        // Identifier or keyword or PLC address
        if (isAlpha(c)) {
            while (isAlphaNum(peek()) || peek() == '.') {
                if (peek() == '.') {
                    // Only include dot if followed by digit (bit notation)
                    if (!isDigit(peek(1))) break;
                }
                currentToken.text[currentToken.textLen++] = advance();
            }
            currentToken.text[currentToken.textLen] = '\0';
            
            // Check if it's a PLC address
            if (isPLCAddress(currentToken.text)) {
                currentToken.type = PSTOK_ADDRESS;
            } else {
                currentToken.type = checkKeyword(currentToken.text);
            }
            return;
        }
        
        // Hash for constant literals
        if (c == '#') {
            currentToken.text[currentToken.textLen++] = advance();
            // Read the number after #
            bool isNeg = false;
            if (peek() == '-') {
                currentToken.text[currentToken.textLen++] = advance();
                isNeg = true;
            }
            while (isDigit(peek()) || peek() == '.') {
                currentToken.text[currentToken.textLen++] = advance();
            }
            currentToken.text[currentToken.textLen] = '\0';
            currentToken.type = PSTOK_INTEGER; // Treat as integer literal
            // Parse value
            int64_t val = 0;
            bool hasDecimal = false;
            double fval = 0.0;
            double div = 1.0;
            for (int i = 1; i < currentToken.textLen; i++) {
                char nc = currentToken.text[i];
                if (nc == '-') continue;
                if (nc == '.') { hasDecimal = true; continue; }
                if (hasDecimal) {
                    div *= 10.0;
                    fval += (nc - '0') / div;
                } else {
                    val = val * 10 + (nc - '0');
                }
            }
            if (hasDecimal) {
                currentToken.floatValue = (double)val + fval;
                if (isNeg) currentToken.floatValue = -currentToken.floatValue;
                currentToken.type = PSTOK_FLOAT;
            } else {
                currentToken.intValue = isNeg ? -val : val;
                currentToken.type = PSTOK_INTEGER;
            }
            return;
        }
        
        // String literals
        if (c == '"' || c == '\'') {
            char quote = advance();
            while (pos < sourceLength && peek() != quote) {
                if (peek() == '\\' && peek(1) == quote) {
                    advance();
                    currentToken.text[currentToken.textLen++] = advance();
                } else if (peek() == '\n') {
                    setError("Unterminated string literal");
                    currentToken.type = PSTOK_ERROR;
                    return;
                } else {
                    currentToken.text[currentToken.textLen++] = advance();
                }
            }
            if (pos >= sourceLength) {
                setError("Unterminated string literal");
                currentToken.type = PSTOK_ERROR;
                return;
            }
            advance(); // Closing quote
            currentToken.text[currentToken.textLen] = '\0';
            currentToken.type = PSTOK_STRING;
            return;
        }
        
        // Operators and punctuation
        currentToken.text[0] = c;
        currentToken.text[1] = '\0';
        currentToken.textLen = 1;
        
        switch (c) {
            case '+':
                advance();
                if (peek() == '+') { advance(); currentToken.type = PSTOK_PLUS_PLUS; currentToken.text[1] = '+'; currentToken.text[2] = '\0'; currentToken.textLen = 2; }
                else if (peek() == '=') { advance(); currentToken.type = PSTOK_PLUS_EQ; currentToken.text[1] = '='; currentToken.text[2] = '\0'; currentToken.textLen = 2; }
                else currentToken.type = PSTOK_PLUS;
                break;
            case '-':
                advance();
                if (peek() == '-') { advance(); currentToken.type = PSTOK_MINUS_MINUS; currentToken.text[1] = '-'; currentToken.text[2] = '\0'; currentToken.textLen = 2; }
                else if (peek() == '=') { advance(); currentToken.type = PSTOK_MINUS_EQ; currentToken.text[1] = '='; currentToken.text[2] = '\0'; currentToken.textLen = 2; }
                else currentToken.type = PSTOK_MINUS;
                break;
            case '*':
                advance();
                if (peek() == '=') { advance(); currentToken.type = PSTOK_STAR_EQ; currentToken.text[1] = '='; currentToken.text[2] = '\0'; currentToken.textLen = 2; }
                else currentToken.type = PSTOK_STAR;
                break;
            case '/':
                advance();
                if (peek() == '=') { advance(); currentToken.type = PSTOK_SLASH_EQ; currentToken.text[1] = '='; currentToken.text[2] = '\0'; currentToken.textLen = 2; }
                else currentToken.type = PSTOK_SLASH;
                break;
            case '%':
                advance();
                currentToken.type = PSTOK_PERCENT;
                break;
            case '&':
                advance();
                if (peek() == '&') { advance(); currentToken.type = PSTOK_AMP_AMP; currentToken.text[1] = '&'; currentToken.text[2] = '\0'; currentToken.textLen = 2; }
                else currentToken.type = PSTOK_AMPERSAND;
                break;
            case '|':
                advance();
                if (peek() == '|') { advance(); currentToken.type = PSTOK_PIPE_PIPE; currentToken.text[1] = '|'; currentToken.text[2] = '\0'; currentToken.textLen = 2; }
                else currentToken.type = PSTOK_PIPE;
                break;
            case '^':
                advance();
                currentToken.type = PSTOK_CARET;
                break;
            case '~':
                advance();
                currentToken.type = PSTOK_TILDE;
                break;
            case '!':
                advance();
                if (peek() == '=') { advance(); currentToken.type = PSTOK_BANG_EQ; currentToken.text[1] = '='; currentToken.text[2] = '\0'; currentToken.textLen = 2; }
                else currentToken.type = PSTOK_BANG;
                break;
            case '<':
                advance();
                if (peek() == '=') { advance(); currentToken.type = PSTOK_LT_EQ; currentToken.text[1] = '='; currentToken.text[2] = '\0'; currentToken.textLen = 2; }
                else if (peek() == '<') { advance(); currentToken.type = PSTOK_LT_LT; currentToken.text[1] = '<'; currentToken.text[2] = '\0'; currentToken.textLen = 2; }
                else currentToken.type = PSTOK_LT;
                break;
            case '>':
                advance();
                if (peek() == '=') { advance(); currentToken.type = PSTOK_GT_EQ; currentToken.text[1] = '='; currentToken.text[2] = '\0'; currentToken.textLen = 2; }
                else if (peek() == '>') { advance(); currentToken.type = PSTOK_GT_GT; currentToken.text[1] = '>'; currentToken.text[2] = '\0'; currentToken.textLen = 2; }
                else currentToken.type = PSTOK_GT;
                break;
            case '=':
                advance();
                if (peek() == '=') { advance(); currentToken.type = PSTOK_EQ_EQ; currentToken.text[1] = '='; currentToken.text[2] = '\0'; currentToken.textLen = 2; }
                else if (peek() == '>') { advance(); currentToken.type = PSTOK_ARROW; currentToken.text[1] = '>'; currentToken.text[2] = '\0'; currentToken.textLen = 2; }
                else currentToken.type = PSTOK_EQ;
                break;
            case '.': advance(); currentToken.type = PSTOK_DOT; break;
            case '@': advance(); currentToken.type = PSTOK_AT; break;
            case ':': advance(); currentToken.type = PSTOK_COLON; break;
            case ';': advance(); currentToken.type = PSTOK_SEMICOLON; break;
            case ',': advance(); currentToken.type = PSTOK_COMMA; break;
            case '?': advance(); currentToken.type = PSTOK_QUESTION; break;
            case '(': advance(); currentToken.type = PSTOK_LPAREN; break;
            case ')': advance(); currentToken.type = PSTOK_RPAREN; break;
            case '{': advance(); currentToken.type = PSTOK_LBRACE; break;
            case '}': advance(); currentToken.type = PSTOK_RBRACE; break;
            case '[': advance(); currentToken.type = PSTOK_LBRACKET; break;
            case ']': advance(); currentToken.type = PSTOK_RBRACKET; break;
            default:
                advance();
                currentToken.type = PSTOK_ERROR;
                setError("Unexpected character");
                break;
        }
    }
    
    void nextToken() {
        if (hasPeekToken) {
            currentToken = peekToken;
            hasPeekToken = false;
        } else {
            scanToken();
        }
    }
    
    PLCScriptToken& peekNextToken() {
        if (!hasPeekToken) {
            PLCScriptToken saved = currentToken;
            int savedPos = pos;
            int savedLine = currentLine;
            int savedCol = currentColumn;
            scanToken();
            peekToken = currentToken;
            currentToken = saved;
            pos = savedPos;
            currentLine = savedLine;
            currentColumn = savedCol;
            hasPeekToken = true;
        }
        return peekToken;
    }
    
    bool check(PLCScriptTokenType type) {
        return currentToken.type == type;
    }
    
    bool match(PLCScriptTokenType type) {
        if (check(type)) {
            nextToken();
            return true;
        }
        return false;
    }
    
    void expect(PLCScriptTokenType type, const char* msg) {
        if (!check(type)) {
            setError(msg);
            return;
        }
        nextToken();
    }
    
    // ========================================================================
    // Symbol Table
    // ========================================================================
    
    PLCScriptSymbol* findSymbol(const char* name) {
        for (int i = symbolCount - 1; i >= 0; i--) {
            if (strEq(symbols[i].name, name)) {
                return &symbols[i];
            }
        }
        return nullptr;
    }
    
    PLCScriptSymbol* addSymbol(const char* name, PLCScriptVarType type, bool isConst) {
        if (symbolCount >= PLCSCRIPT_MAX_SYMBOLS) {
            setError("Too many symbols");
            return nullptr;
        }
        
        // Check for duplicate in current scope
        for (int i = symbolCount - 1; i >= 0 && symbols[i].scopeLevel == currentScopeLevel; i--) {
            if (strEq(symbols[i].name, name)) {
                setError("Duplicate symbol declaration");
                return nullptr;
            }
        }
        
        PLCScriptSymbol* sym = &symbols[symbolCount++];
        int i = 0;
        while (name[i] && i < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
            sym->name[i] = name[i];
            i++;
        }
        sym->name[i] = '\0';
        sym->type = type;
        sym->isConst = isConst;
        sym->scopeLevel = currentScopeLevel;
        sym->declarationLine = currentToken.line;
        sym->declarationColumn = currentToken.column;
        sym->hasInitializer = false;
        sym->isBit = false;
        sym->address[0] = '\0';
        sym->memoryOffset = 0;
        sym->bitIndex = 0;
        
        return sym;
    }
    
    void enterScope() {
        currentScopeLevel++;
    }
    
    void exitScope() {
        // Remove symbols from current scope
        while (symbolCount > 0 && symbols[symbolCount - 1].scopeLevel == currentScopeLevel) {
            symbolCount--;
        }
        currentScopeLevel--;
    }
    
    // ========================================================================
    // Type helpers
    // ========================================================================
    
    PLCScriptVarType tokenTypeToVarType(PLCScriptTokenType tokType) {
        switch (tokType) {
            case PSTOK_TYPE_BOOL: return PSTYPE_BOOL;
            case PSTOK_TYPE_I8: return PSTYPE_I8;
            case PSTOK_TYPE_I16: return PSTYPE_I16;
            case PSTOK_TYPE_I32: return PSTYPE_I32;
            case PSTOK_TYPE_I64: return PSTYPE_I64;
            case PSTOK_TYPE_U8: return PSTYPE_U8;
            case PSTOK_TYPE_U16: return PSTYPE_U16;
            case PSTOK_TYPE_U32: return PSTYPE_U32;
            case PSTOK_TYPE_U64: return PSTYPE_U64;
            case PSTOK_TYPE_F32: return PSTYPE_F32;
            case PSTOK_TYPE_F64: return PSTYPE_F64;
            default: return PSTYPE_VOID;
        }
    }
    
    const char* varTypeToPlcasm(PLCScriptVarType type) {
        switch (type) {
            case PSTYPE_BOOL: return "bool";
            case PSTYPE_I8: return "i8";
            case PSTYPE_I16: return "i16";
            case PSTYPE_I32: return "i32";
            case PSTYPE_I64: return "i64";
            case PSTYPE_U8: return "u8";
            case PSTYPE_U16: return "u16";
            case PSTYPE_U32: return "u32";
            case PSTYPE_U64: return "u64";
            case PSTYPE_F32: return "f32";
            case PSTYPE_F64: return "f64";
            default: return "u8";
        }
    }
    
    int varTypeSize(PLCScriptVarType type) {
        switch (type) {
            case PSTYPE_BOOL:
            case PSTYPE_I8:
            case PSTYPE_U8: return 1;
            case PSTYPE_I16:
            case PSTYPE_U16: return 2;
            case PSTYPE_I32:
            case PSTYPE_U32:
            case PSTYPE_F32: return 4;
            case PSTYPE_I64:
            case PSTYPE_U64:
            case PSTYPE_F64: return 8;
            default: return 1;
        }
    }
    
    bool isTypeKeyword(PLCScriptTokenType type) {
        return type >= PSTOK_TYPE_BOOL && type <= PSTOK_TYPE_F64;
    }
    
    // ========================================================================
    // Address parsing
    // ========================================================================
    
    bool parseAddress(PLCScriptSymbol* sym, const char* addr) {
        // Parse the address - just prefix + number (type determines width)
        // Examples: M10, X0, Y0.0, T0, C0
        // Legacy MW10/MD20 still accepted (W/D/B suffix ignored)
        
        char first = addr[0];
        if (first >= 'a' && first <= 'z') first -= 32;
        
        int numStart = 1;
        u32 baseOffset = 0;
        char areaPrefix = first;
        
        // Determine memory area
        switch (first) {
            case 'X': baseOffset = plcasm_input_offset; break;
            case 'Y': baseOffset = plcasm_output_offset; break;
            case 'M': baseOffset = plcasm_marker_offset; break;
            case 'S': baseOffset = plcasm_system_offset; break;
            case 'C': baseOffset = plcasm_control_offset; break;
            case 'T': baseOffset = plcasm_timer_offset; break;
            default:
                setError("Invalid address prefix");
                return false;
        }
        
        // Check for type suffix (W, D, B) - skip it in PLCASM address
        char second = addr[1];
        if (second >= 'a' && second <= 'z') second -= 32;
        if (second == 'W' || second == 'D' || second == 'B') {
            numStart = 2;
        }
        
        // Parse byte number
        int byteNum = 0;
        int dotPos = -1;
        for (int j = numStart; addr[j]; j++) {
            if (addr[j] == '.') {
                dotPos = j;
                break;
            }
            if (addr[j] >= '0' && addr[j] <= '9') {
                byteNum = byteNum * 10 + (addr[j] - '0');
            } else {
                setError("Invalid address format");
                return false;
            }
        }
        
        sym->memoryOffset = baseOffset + byteNum;
        
        // Build PLCASM-style address string (without type suffix)
        int outIdx = 0;
        sym->address[outIdx++] = areaPrefix;
        
        // Write byte number
        char numBuf[16];
        int numLen = 0;
        int n = byteNum;
        if (n == 0) {
            numBuf[numLen++] = '0';
        } else {
            while (n > 0) {
                numBuf[numLen++] = '0' + (n % 10);
                n /= 10;
            }
        }
        for (int k = numLen - 1; k >= 0; k--) {
            sym->address[outIdx++] = numBuf[k];
        }
        
        // Parse bit position if present
        if (dotPos >= 0) {
            sym->isBit = true;
            sym->address[outIdx++] = '.';
            if (addr[dotPos + 1] >= '0' && addr[dotPos + 1] <= '7') {
                sym->bitIndex = addr[dotPos + 1] - '0';
                sym->address[outIdx++] = addr[dotPos + 1];
            } else {
                setError("Bit position must be 0-7");
                return false;
            }
        } else {
            sym->isBit = false;
            sym->bitIndex = 0;
        }
        
        sym->address[outIdx] = '\0';
        return true;
    }
    
    // ========================================================================
    // Code generation helpers
    // ========================================================================
    
    void emitComment(const char* comment) {
        emit("// ");
        emitLine(comment);
    }
    
    void emitLoadConst(PLCScriptVarType type, int64_t value) {
        emit(varTypeToPlcasm(type));
        emit(".const ");
        emitInt(value);
        emit("\n");
    }
    
    void emitLoadConstFloat(PLCScriptVarType type, double value) {
        emit(varTypeToPlcasm(type));
        emit(".const ");
        emitFloat(value);
        emit("\n");
    }
    
    void emitDrop(PLCScriptVarType type) {
        emit(varTypeToPlcasm(type));
        emitLine(".drop");
    }
    
    void emitCopy(PLCScriptVarType type) {
        emit(varTypeToPlcasm(type));
        emitLine(".copy");
    }
    
    // Get size in bytes for a variable type
    int getTypeSize(PLCScriptVarType type) {
        switch (type) {
            case PSTYPE_BOOL:
            case PSTYPE_I8:
            case PSTYPE_U8: return 1;
            case PSTYPE_I16:
            case PSTYPE_U16: return 2;
            case PSTYPE_I32:
            case PSTYPE_U32:
            case PSTYPE_F32: return 4;
            case PSTYPE_I64:
            case PSTYPE_U64:
            case PSTYPE_F64: return 8;
            default: return 4;
        }
    }
    
    // Generate auto-address for a variable based on its type
    void generateAutoAddress(PLCScriptSymbol* sym) {
        char addr[32];
        
        if (sym->type == PSTYPE_BOOL) {
            // Bit address: M<byte>.<bit> - no conversion needed for PLCASM
            int byteNum = autoAddrBit / 8;
            int bitNum = autoAddrBit % 8;
            // Format: M<byte>.<bit>
            int len = 0;
            addr[len++] = 'M';
            // Write byte number
            char numBuf[16];
            int numLen = 0;
            int n = byteNum;
            if (n == 0) {
                numBuf[numLen++] = '0';
            } else {
                while (n > 0) {
                    numBuf[numLen++] = '0' + (n % 10);
                    n /= 10;
                }
            }
            for (int i = numLen - 1; i >= 0; i--) addr[len++] = numBuf[i];
            addr[len++] = '.';
            addr[len++] = '0' + bitNum;
            addr[len] = '\0';
            autoAddrBit++;
            sym->isBit = true;
            sym->bitIndex = bitNum;
            // Copy directly - already in PLCASM format
            int j = 0;
            while (addr[j] && j < 31) {
                sym->address[j] = addr[j];
                j++;
            }
            sym->address[j] = '\0';
        } else if (sym->type == PSTYPE_I8 || sym->type == PSTYPE_U8) {
            // Byte address: M<n> - direct PLCASM format
            int len = 0;
            addr[len++] = 'M';
            char numBuf[16];
            int numLen = 0;
            int n = autoAddrByte;
            if (n == 0) {
                numBuf[numLen++] = '0';
            } else {
                while (n > 0) {
                    numBuf[numLen++] = '0' + (n % 10);
                    n /= 10;
                }
            }
            for (int i = numLen - 1; i >= 0; i--) addr[len++] = numBuf[i];
            addr[len] = '\0';
            autoAddrByte++;
            // Copy directly - already in PLCASM format
            int j = 0;
            while (addr[j] && j < 31) {
                sym->address[j] = addr[j];
                j++;
            }
            sym->address[j] = '\0';
        } else if (sym->type == PSTYPE_I16 || sym->type == PSTYPE_U16) {
            // Word address: M<n> (PLCASM format, not MW<n>)
            int len = 0;
            addr[len++] = 'M';
            char numBuf[16];
            int numLen = 0;
            int n = autoAddrWord;
            if (n == 0) {
                numBuf[numLen++] = '0';
            } else {
                while (n > 0) {
                    numBuf[numLen++] = '0' + (n % 10);
                    n /= 10;
                }
            }
            for (int i = numLen - 1; i >= 0; i--) addr[len++] = numBuf[i];
            addr[len] = '\0';
            autoAddrWord += 2;  // Words are 2 bytes apart
            // Copy directly - already in PLCASM format
            int j = 0;
            while (addr[j] && j < 31) {
                sym->address[j] = addr[j];
                j++;
            }
            sym->address[j] = '\0';
        } else {
            // Dword address: M<n> (PLCASM format, for i32, u32, f32, i64, u64, f64)
            int len = 0;
            addr[len++] = 'M';
            char numBuf[16];
            int numLen = 0;
            int n = autoAddrDword;
            if (n == 0) {
                numBuf[numLen++] = '0';
            } else {
                while (n > 0) {
                    numBuf[numLen++] = '0' + (n % 10);
                    n /= 10;
                }
            }
            for (int i = numLen - 1; i >= 0; i--) addr[len++] = numBuf[i];
            addr[len] = '\0';
            autoAddrDword += getTypeSize(sym->type);
            // Copy directly - already in PLCASM format
            int j = 0;
            while (addr[j] && j < 31) {
                sym->address[j] = addr[j];
                j++;
            }
            sym->address[j] = '\0';
        }
        
        sym->isAutoAddress = true;
    }
    
    void emitLoadFromAddress(PLCScriptSymbol* sym) {
        if (sym->isLocal) {
            // Local variable uses auto-assigned memory address
            // The address was generated when the variable was declared
        }
        if (sym->isBit) {
            emit("u8.readBit ");
            emit(sym->address);
            emit("\n");
        } else {
            emit(varTypeToPlcasm(sym->type));
            emit(".load_from ");
            emit(sym->address);
            emit("\n");
        }
    }
    
    void emitStoreToAddress(PLCScriptSymbol* sym) {
        if (sym->isLocal) {
            // Local variable uses auto-assigned memory address
            // The address was generated when the variable was declared
        }
        if (sym->isBit) {
            emit("u8.writeBit ");
            emit(sym->address);
            emit("\n");
        } else {
            emit(varTypeToPlcasm(sym->type));
            emit(".move_to ");
            emit(sym->address);
            emit("\n");
        }
    }
    
    void emitBinaryOp(const char* op, PLCScriptVarType type) {
        emit(varTypeToPlcasm(type));
        emit(".");
        emit(op);
        emit("\n");
    }
    
    void emitCompareOp(const char* op, PLCScriptVarType type) {
        emit(varTypeToPlcasm(type));
        emit(".cmp_");
        emit(op);
        emit("\n");
    }
    
    void emitJump(const char* label) {
        emit("jmp ");
        emit(label);
        emit("\n");
    }
    
    void emitJumpIfFalse(const char* label) {
        emit("jmp_if_not ");
        emit(label);
        emit("\n");
    }
    
    void emitJumpIfTrue(const char* label) {
        emit("jmp_if ");
        emit(label);
        emit("\n");
    }
    
    // ========================================================================
    // Parser & Code Generator
    // ========================================================================
    
    bool compile() {
        // Initialize
        nextToken();
        
        // Emit header comment
        emitComment("Generated by PLCScript Compiler");
        emit("\n");
        
        // Parse program (list of statements)
        while (!check(PSTOK_EOF) && !hasError) {
            parseStatement();
        }
        
        // Emit exit if not already
        emitLine("exit");
        
        return !hasError;
    }
    
    void parseStatement() {
        if (hasError) return;
        
        switch (currentToken.type) {
            case PSTOK_LET:
            case PSTOK_CONST:
                parseVariableDeclaration();
                break;
            case PSTOK_IF:
                parseIfStatement();
                break;
            case PSTOK_WHILE:
                parseWhileStatement();
                break;
            case PSTOK_FOR:
                parseForStatement();
                break;
            case PSTOK_LBRACE:
                parseBlock();
                break;
            case PSTOK_BREAK:
                parseBreak();
                break;
            case PSTOK_CONTINUE:
                parseContinue();
                break;
            case PSTOK_RETURN:
                parseReturn();
                break;
            case PSTOK_SEMICOLON:
                // Empty statement
                nextToken();
                break;
            default:
                // Expression statement (assignment, function call, etc.)
                parseExpressionStatement();
                break;
        }
    }
    
    void parseVariableDeclaration() {
        bool isConst = check(PSTOK_CONST);
        nextToken(); // consume 'let' or 'const'
        
        // Variable name
        if (!check(PSTOK_IDENTIFIER)) {
            setError("Expected variable name");
            return;
        }
        char name[PLCSCRIPT_MAX_IDENTIFIER_LEN];
        int i = 0;
        while (currentToken.text[i] && i < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
            name[i] = currentToken.text[i];
            i++;
        }
        name[i] = '\0';
        nextToken();
        
        // Type annotation: ": type"
        PLCScriptVarType varType = PSTYPE_I16; // Default type
        if (match(PSTOK_COLON)) {
            if (!isTypeKeyword(currentToken.type)) {
                setError("Expected type name");
                return;
            }
            varType = tokenTypeToVarType(currentToken.type);
            nextToken();
        }
        
        // Address annotation: "@ address" or "@ auto" or none (local variable)
        char address[32] = {0};
        bool hasAddress = false;
        bool isAutoAddress = false;
        bool isLocal = false;
        
        if (match(PSTOK_AT)) {
            // Check for 'auto' keyword for auto-addressing
            if (check(PSTOK_AUTO)) {
                isAutoAddress = true;
                hasAddress = true;
                nextToken();
            } else if (check(PSTOK_ADDRESS) || check(PSTOK_IDENTIFIER)) {
                int j = 0;
                while (currentToken.text[j] && j < 31) {
                    address[j] = currentToken.text[j];
                    j++;
                }
                address[j] = '\0';
                hasAddress = true;
                nextToken();
            } else {
                setError("Expected PLC address or 'auto' after @");
                return;
            }
        } else {
            // No @ means local variable - auto-assign a temporary memory address
            isLocal = !isConst;  // Constants without address are compile-time only
        }
        
        // Create symbol
        PLCScriptSymbol* sym = addSymbol(name, varType, isConst);
        if (!sym) return;
        
        sym->isLocal = isLocal;
        sym->isAutoAddress = isAutoAddress;
        sym->stackSlot = -1;
        
        if (isAutoAddress || isLocal) {
            // Generate auto address for both explicit "@ auto" and local variables
            generateAutoAddress(sym);
            hasAddress = true;
            isAutoAddress = true;
            sym->isAutoAddress = true;
            // Copy generated address to local for comment
            int j = 0;
            while (sym->address[j] && j < 31) {
                address[j] = sym->address[j];
                j++;
            }
            address[j] = '\0';
        } else if (hasAddress) {
            if (!parseAddress(sym, address)) return;
        }
        
        // Emit comment
        emit("// ");
        emit(isConst ? "const " : "let ");
        emit(name);
        emit(": ");
        emit(varTypeToPlcasm(varType));
        if (hasAddress) {
            emit(" @ ");
            emit(address);
            if (isLocal) {
                emit(" (local)");
            } else if (isAutoAddress) {
                emit(" (auto)");
            }
        }
        emit("\n");
        
        // Initializer: "= expression"
        if (match(PSTOK_EQ)) {
            // Set target type hint for expression parsing
            PLCScriptVarType savedTarget = targetType;
            targetType = varType;
            // Generate expression code
            PLCScriptVarType exprType = parseExpression();
            targetType = savedTarget;
            
            // Type conversion if needed
            if (exprType != varType && exprType != PSTYPE_VOID) {
                emit("cvt ");
                emit(varTypeToPlcasm(exprType));
                emit(" ");
                emit(varTypeToPlcasm(varType));
                emit("\n");
            }
            
            // Store to memory
            if (hasAddress) {
                emitStoreToAddress(sym);
            } else {
                // For constants without address, just drop (value is compile-time only)
                if (!isConst) {
                    setError("Variable without @ address cannot be stored");
                    return;
                }
                emitDrop(varType);
            }
            
            sym->hasInitializer = true;
        }
        
        // Semicolon is optional
        match(PSTOK_SEMICOLON);
    }
    
    void parseIfStatement() {
        nextToken(); // consume 'if'
        
        expect(PSTOK_LPAREN, "Expected '(' after 'if'");
        
        // Generate condition
        parseExpression();
        
        expect(PSTOK_RPAREN, "Expected ')' after condition");
        
        // Generate labels
        char elseLabel[64], endLabel[64];
        generateLabel(elseLabel, "else");
        generateLabel(endLabel, "endif");
        
        // Jump to else if false
        emitJumpIfFalse(elseLabel);
        
        // Then branch
        parseStatement();
        
        // Check for else
        if (check(PSTOK_ELSE)) {
            emitJump(endLabel);
            emitLabel(elseLabel);
            nextToken(); // consume 'else'
            parseStatement();
            emitLabel(endLabel);
        } else {
            emitLabel(elseLabel);
        }
    }
    
    void parseWhileStatement() {
        nextToken(); // consume 'while'
        
        // Generate labels
        char startLabel[64], endLabel[64];
        generateLabel(startLabel, "while");
        generateLabel(endLabel, "endwhile");
        
        // Push loop context
        if (loopStackDepth < PLCSCRIPT_MAX_SCOPES) {
            int j = 0;
            while (endLabel[j]) { loopStack[loopStackDepth].breakLabel[j] = endLabel[j]; j++; }
            loopStack[loopStackDepth].breakLabel[j] = '\0';
            j = 0;
            while (startLabel[j]) { loopStack[loopStackDepth].continueLabel[j] = startLabel[j]; j++; }
            loopStack[loopStackDepth].continueLabel[j] = '\0';
            loopStackDepth++;
        }
        
        emitLabel(startLabel);
        
        expect(PSTOK_LPAREN, "Expected '(' after 'while'");
        parseExpression();
        expect(PSTOK_RPAREN, "Expected ')' after condition");
        
        emitJumpIfFalse(endLabel);
        
        parseStatement();
        
        emitJump(startLabel);
        emitLabel(endLabel);
        
        // Pop loop context
        if (loopStackDepth > 0) loopStackDepth--;
    }
    
    void parseForStatement() {
        nextToken(); // consume 'for'
        
        expect(PSTOK_LPAREN, "Expected '(' after 'for'");
        
        // Enter scope for loop variable
        enterScope();
        
        // Initializer
        if (!check(PSTOK_SEMICOLON)) {
            if (check(PSTOK_LET)) {
                parseVariableDeclaration();
            } else {
                parseExpressionStatement();
            }
        } else {
            nextToken(); // consume ';'
        }
        
        // Generate labels
        char condLabel[64], bodyLabel[64], updateLabel[64], endLabel[64];
        generateLabel(condLabel, "forcond");
        generateLabel(bodyLabel, "forbody");
        generateLabel(updateLabel, "forupd");
        generateLabel(endLabel, "endfor");
        
        // Push loop context
        if (loopStackDepth < PLCSCRIPT_MAX_SCOPES) {
            int j = 0;
            while (endLabel[j]) { loopStack[loopStackDepth].breakLabel[j] = endLabel[j]; j++; }
            loopStack[loopStackDepth].breakLabel[j] = '\0';
            j = 0;
            while (updateLabel[j]) { loopStack[loopStackDepth].continueLabel[j] = updateLabel[j]; j++; }
            loopStack[loopStackDepth].continueLabel[j] = '\0';
            loopStackDepth++;
        }
        
        emitLabel(condLabel);
        
        // Condition
        if (!check(PSTOK_SEMICOLON)) {
            parseExpression();
            emitJumpIfFalse(endLabel);
        }
        expect(PSTOK_SEMICOLON, "Expected ';' after for condition");
        
        // Jump to body (we'll emit update after body)
        emitJump(bodyLabel);
        
        // Update expression label
        emitLabel(updateLabel);
        
        // Update expression
        if (!check(PSTOK_RPAREN)) {
            PLCScriptVarType updateType = parseExpression();
            if (updateType != PSTYPE_VOID) emitDrop(updateType); // Discard result
        }
        emitJump(condLabel);
        
        expect(PSTOK_RPAREN, "Expected ')' after for clauses");
        
        // Body
        emitLabel(bodyLabel);
        parseStatement();
        emitJump(updateLabel);
        
        emitLabel(endLabel);
        
        // Pop loop context
        if (loopStackDepth > 0) loopStackDepth--;
        
        exitScope();
    }
    
    void parseBlock() {
        expect(PSTOK_LBRACE, "Expected '{'");
        enterScope();
        
        while (!check(PSTOK_RBRACE) && !check(PSTOK_EOF) && !hasError) {
            parseStatement();
        }
        
        exitScope();
        expect(PSTOK_RBRACE, "Expected '}'");
    }
    
    void parseBreak() {
        nextToken(); // consume 'break'
        if (loopStackDepth == 0) {
            setError("'break' outside of loop");
            return;
        }
        emitJump(loopStack[loopStackDepth - 1].breakLabel);
        // Semicolon is optional
        match(PSTOK_SEMICOLON);
    }
    
    void parseContinue() {
        nextToken(); // consume 'continue'
        if (loopStackDepth == 0) {
            setError("'continue' outside of loop");
            return;
        }
        emitJump(loopStack[loopStackDepth - 1].continueLabel);
        // Semicolon is optional
        match(PSTOK_SEMICOLON);
    }
    
    void parseReturn() {
        nextToken(); // consume 'return'
        // For now, just emit exit
        if (!check(PSTOK_SEMICOLON)) {
            PLCScriptVarType retType = parseExpression();
            if (retType != PSTYPE_VOID) emitDrop(retType);
        }
        emitLine("exit");
        // Semicolon is optional
        match(PSTOK_SEMICOLON);
    }
    
    void parseExpressionStatement() {
        PLCScriptVarType type = parseExpression();
        
        // Drop the result if not void
        if (type != PSTYPE_VOID) {
            // Check if this was an assignment (result already stored)
            // For now, always drop
            emitDrop(type);
        }
        
        // Semicolon is optional
        match(PSTOK_SEMICOLON);
    }
    
    // ========================================================================
    // Expression parsing (precedence climbing)
    // ========================================================================
    
    PLCScriptVarType parseExpression() {
        return parseAssignment();
    }
    
    PLCScriptVarType parseTernary() {
        PLCScriptVarType condType = parseLogicalOr();
        
        if (check(PSTOK_QUESTION)) {
            nextToken(); // consume '?'
            
            // Generate labels for ternary
            char falseLabel[64], endLabel[64];
            generateLabel(falseLabel, "ternary_false");
            generateLabel(endLabel, "ternary_end");
            
            // Jump to false branch if condition is false
            emitJumpIfFalse(falseLabel);
            
            // True branch
            PLCScriptVarType trueType = parseExpression();
            emitJump(endLabel);
            
            // Expect ':'
            expect(PSTOK_COLON, "Expected ':' in ternary expression");
            
            // False branch
            emitLabel(falseLabel);
            (void)parseTernary(); // Right-associative, discard type (should match trueType)
            
            emitLabel(endLabel);
            
            // Return the type (prefer the more specific type, or trueType)
            // For simplicity, just return trueType - both branches should have same type
            return trueType;
        }
        
        return condType;
    }
    
    PLCScriptVarType parseAssignment() {
        // For assignment, we need to check if LHS is assignable
        // Save state in case we need to backtrack
        int savedPos = pos;
        int savedLine = currentLine;
        int savedCol = currentColumn;
        PLCScriptToken savedToken = currentToken;
        bool savedHasPeek = hasPeekToken;
        PLCScriptToken savedPeek = peekToken;
        
        // Try to parse as identifier followed by assignment operator
        if (check(PSTOK_IDENTIFIER) || check(PSTOK_ADDRESS)) {
            // Check if it's a direct PLC address assignment (e.g., Y0.0 = expr)
            bool isDirectAddress = check(PSTOK_ADDRESS);
            PLCScriptSymbol tempSym;
            memset(&tempSym, 0, sizeof(tempSym));
            
            char varName[PLCSCRIPT_MAX_IDENTIFIER_LEN];
            int i = 0;
            while (currentToken.text[i] && i < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
                varName[i] = currentToken.text[i];
                i++;
            }
            varName[i] = '\0';
            
            PLCScriptSymbol* sym = findSymbol(varName);
            
            // For direct PLC address, create temporary symbol
            if (isDirectAddress && !sym) {
                if (!parseAddress(&tempSym, varName)) {
                    return PSTYPE_VOID;
                }
                // Determine type from address
                char second = varName[1];
                if (second >= 'a' && second <= 'z') second -= 32;
                if (second == 'W') tempSym.type = PSTYPE_I16;
                else if (second == 'D') tempSym.type = PSTYPE_I32;
                else tempSym.type = PSTYPE_U8;
                // Check if bit address
                for (int j = 0; varName[j]; j++) {
                    if (varName[j] == '.') { tempSym.type = PSTYPE_BOOL; break; }
                }
                sym = &tempSym;
            }
            
            nextToken();
            
            if (check(PSTOK_EQ)) {
                // Simple assignment
                if (!sym) {
                    setError("Undefined variable");
                    return PSTYPE_VOID;
                }
                if (sym->isConst) {
                    setError("Cannot assign to const");
                    return PSTYPE_VOID;
                }
                
                nextToken(); // consume '='
                // Set target type hint for expression parsing
                PLCScriptVarType savedTarget = targetType;
                targetType = sym->type;
                PLCScriptVarType rhsType = parseAssignment();
                targetType = savedTarget;
                
                // Type conversion if needed
                if (rhsType != sym->type && rhsType != PSTYPE_VOID) {
                    emit("cvt ");
                    emit(varTypeToPlcasm(rhsType));
                    emit(" ");
                    emit(varTypeToPlcasm(sym->type));
                    emit("\n");
                }
                
                // Duplicate value for the expression result
                emitCopy(sym->type);
                
                // Store to address
                emitStoreToAddress(sym);
                
                return sym->type;
            }
            else if (check(PSTOK_PLUS_EQ) || check(PSTOK_MINUS_EQ) || 
                     check(PSTOK_STAR_EQ) || check(PSTOK_SLASH_EQ)) {
                // Compound assignment
                if (!sym) {
                    setError("Undefined variable");
                    return PSTYPE_VOID;
                }
                if (sym->isConst) {
                    setError("Cannot assign to const");
                    return PSTYPE_VOID;
                }
                
                PLCScriptTokenType opType = currentToken.type;
                nextToken();
                
                // Load current value
                emitLoadFromAddress(sym);
                
                // Parse RHS with target type hint
                PLCScriptVarType savedTarget = targetType;
                targetType = sym->type;
                PLCScriptVarType rhsType = parseAssignment();
                targetType = savedTarget;
                
                // Type conversion if needed
                if (rhsType != sym->type && rhsType != PSTYPE_VOID) {
                    emit("cvt ");
                    emit(varTypeToPlcasm(rhsType));
                    emit(" ");
                    emit(varTypeToPlcasm(sym->type));
                    emit("\n");
                }
                
                // Emit operation
                const char* op = nullptr;
                switch (opType) {
                    case PSTOK_PLUS_EQ: op = "add"; break;
                    case PSTOK_MINUS_EQ: op = "sub"; break;
                    case PSTOK_STAR_EQ: op = "mul"; break;
                    case PSTOK_SLASH_EQ: op = "div"; break;
                    default: break;
                }
                if (op) emitBinaryOp(op, sym->type);
                
                // Duplicate and store
                emitCopy(sym->type);
                emitStoreToAddress(sym);
                
                return sym->type;
            }
            else if (check(PSTOK_PLUS_PLUS) || check(PSTOK_MINUS_MINUS)) {
                // Post-increment/decrement
                if (!sym) {
                    setError("Undefined variable");
                    return PSTYPE_VOID;
                }
                if (sym->isConst) {
                    setError("Cannot modify const");
                    return PSTYPE_VOID;
                }
                
                bool isIncr = check(PSTOK_PLUS_PLUS);
                nextToken();
                
                // Load current value (this is the result)
                emitLoadFromAddress(sym);
                
                // Duplicate, modify, store
                emitCopy(sym->type);
                emitLoadConst(sym->type, 1);
                emitBinaryOp(isIncr ? "add" : "sub", sym->type);
                emitStoreToAddress(sym);
                
                return sym->type;
            }
        }
        
        // Restore state and parse as regular expression
        pos = savedPos;
        currentLine = savedLine;
        currentColumn = savedCol;
        currentToken = savedToken;
        hasPeekToken = savedHasPeek;
        peekToken = savedPeek;
        
        return parseTernary();
    }
    
    PLCScriptVarType parseLogicalOr() {
        PLCScriptVarType left = parseLogicalAnd();
        
        while (check(PSTOK_PIPE_PIPE)) {
            nextToken();
            (void)parseLogicalAnd();
            emitLine("u8.or");
            left = PSTYPE_BOOL;
        }
        
        return left;
    }
    
    PLCScriptVarType parseLogicalAnd() {
        PLCScriptVarType left = parseBitwiseOr();
        
        while (check(PSTOK_AMP_AMP)) {
            nextToken();
            (void)parseBitwiseOr();
            emitLine("u8.and");
            left = PSTYPE_BOOL;
        }
        
        return left;
    }
    
    PLCScriptVarType parseBitwiseOr() {
        PLCScriptVarType left = parseBitwiseXor();
        
        while (check(PSTOK_PIPE)) {
            nextToken();
            (void)parseBitwiseXor();
            emit("bw.or.");
            if (left == PSTYPE_U8 || left == PSTYPE_I8 || left == PSTYPE_BOOL) emitLine("x8");
            else if (left == PSTYPE_U16 || left == PSTYPE_I16) emitLine("x16");
            else if (left == PSTYPE_U64 || left == PSTYPE_I64) emitLine("x64");
            else emitLine("x32");
        }
        
        return left;
    }
    
    PLCScriptVarType parseBitwiseXor() {
        PLCScriptVarType left = parseBitwiseAnd();
        
        while (check(PSTOK_CARET)) {
            nextToken();
            (void)parseBitwiseAnd();
            emit("bw.xor.");
            if (left == PSTYPE_U8 || left == PSTYPE_I8 || left == PSTYPE_BOOL) emitLine("x8");
            else if (left == PSTYPE_U16 || left == PSTYPE_I16) emitLine("x16");
            else if (left == PSTYPE_U64 || left == PSTYPE_I64) emitLine("x64");
            else emitLine("x32");
        }
        
        return left;
    }
    
    PLCScriptVarType parseBitwiseAnd() {
        PLCScriptVarType left = parseEquality();
        
        while (check(PSTOK_AMPERSAND)) {
            nextToken();
            (void)parseEquality();
            emit("bw.and.");
            if (left == PSTYPE_U8 || left == PSTYPE_I8 || left == PSTYPE_BOOL) emitLine("x8");
            else if (left == PSTYPE_U16 || left == PSTYPE_I16) emitLine("x16");
            else if (left == PSTYPE_U64 || left == PSTYPE_I64) emitLine("x64");
            else emitLine("x32");
        }
        
        return left;
    }
    
    PLCScriptVarType parseEquality() {
        PLCScriptVarType left = parseComparison();
        
        while (check(PSTOK_EQ_EQ) || check(PSTOK_BANG_EQ)) {
            bool isEq = check(PSTOK_EQ_EQ);
            nextToken();
            (void)parseComparison();
            emitCompareOp(isEq ? "eq" : "neq", left);
            left = PSTYPE_BOOL;
        }
        
        return left;
    }
    
    PLCScriptVarType parseComparison() {
        PLCScriptVarType left = parseShift();
        
        while (check(PSTOK_LT) || check(PSTOK_GT) || check(PSTOK_LT_EQ) || check(PSTOK_GT_EQ)) {
            const char* op = nullptr;
            if (check(PSTOK_LT)) op = "lt";
            else if (check(PSTOK_GT)) op = "gt";
            else if (check(PSTOK_LT_EQ)) op = "lte";
            else if (check(PSTOK_GT_EQ)) op = "gte";
            nextToken();
            (void)parseShift();
            emitCompareOp(op, left);
            left = PSTYPE_BOOL;
        }
        
        return left;
    }
    
    PLCScriptVarType parseShift() {
        PLCScriptVarType left = parseAdditive();
        
        while (check(PSTOK_LT_LT) || check(PSTOK_GT_GT)) {
            bool isLeft = check(PSTOK_LT_LT);
            nextToken();
            // Set target type to u8 for shift amount (shifts never need more than 6 bits)
            PLCScriptVarType savedTarget = targetType;
            targetType = PSTYPE_U8;
            PLCScriptVarType shiftType = parseAdditive();
            targetType = savedTarget;
            // Convert shift amount to u8 if it wasn't already emitted as u8
            if (shiftType != PSTYPE_U8) {
                emit("cvt ");
                emit(varTypeToPlcasm(shiftType));
                emit(" ");
                emitLine(varTypeToPlcasm(PSTYPE_U8));
            }
            emit("bw.");
            emit(isLeft ? "shl" : "shr");
            emit(".");
            if (left == PSTYPE_U8 || left == PSTYPE_I8) emitLine("x8");
            else if (left == PSTYPE_U16 || left == PSTYPE_I16) emitLine("x16");
            else if (left == PSTYPE_U64 || left == PSTYPE_I64) emitLine("x64");
            else emitLine("x32");
        }
        
        return left;
    }
    
    PLCScriptVarType parseAdditive() {
        PLCScriptVarType left = parseMultiplicative();
        
        while (check(PSTOK_PLUS) || check(PSTOK_MINUS)) {
            bool isAdd = check(PSTOK_PLUS);
            nextToken();
            (void)parseMultiplicative();
            emitBinaryOp(isAdd ? "add" : "sub", left);
        }
        
        return left;
    }
    
    PLCScriptVarType parseMultiplicative() {
        PLCScriptVarType left = parseUnary();
        
        while (check(PSTOK_STAR) || check(PSTOK_SLASH) || check(PSTOK_PERCENT)) {
            const char* op = nullptr;
            if (check(PSTOK_STAR)) op = "mul";
            else if (check(PSTOK_SLASH)) op = "div";
            else op = "mod";
            nextToken();
            (void)parseUnary();
            emitBinaryOp(op, left);
        }
        
        return left;
    }
    
    PLCScriptVarType parseUnary() {
        if (check(PSTOK_BANG)) {
            nextToken();
            (void)parseUnary();
            emitLine("u8.not");
            return PSTYPE_BOOL;
        }
        if (check(PSTOK_TILDE)) {
            nextToken();
            PLCScriptVarType type = parseUnary();
            emit("bw.not.");
            if (type == PSTYPE_U8 || type == PSTYPE_I8) emitLine("x8");
            else if (type == PSTYPE_U16 || type == PSTYPE_I16) emitLine("x16");
            else if (type == PSTYPE_U64 || type == PSTYPE_I64) emitLine("x64");
            else emitLine("x32");
            return type;
        }
        if (check(PSTOK_MINUS)) {
            nextToken();
            PLCScriptVarType type = parseUnary();
            emit(varTypeToPlcasm(type));
            emitLine(".neg");
            return type;
        }
        if (check(PSTOK_PLUS_PLUS) || check(PSTOK_MINUS_MINUS)) {
            // Pre-increment/decrement
            bool isIncr = check(PSTOK_PLUS_PLUS);
            nextToken();
            
            if (!check(PSTOK_IDENTIFIER)) {
                setError("Expected variable after ++/--");
                return PSTYPE_VOID;
            }
            
            PLCScriptSymbol* sym = findSymbol(currentToken.text);
            if (!sym) {
                setError("Undefined variable");
                return PSTYPE_VOID;
            }
            if (sym->isConst) {
                setError("Cannot modify const");
                return PSTYPE_VOID;
            }
            nextToken();
            
            // Load, modify, store, result is new value
            emitLoadFromAddress(sym);
            emitLoadConst(sym->type, 1);
            emitBinaryOp(isIncr ? "add" : "sub", sym->type);
            emitCopy(sym->type);
            emitStoreToAddress(sym);
            
            return sym->type;
        }
        
        return parsePrimary();
    }
    
    PLCScriptVarType parsePrimary() {
        // Integer literal
        if (check(PSTOK_INTEGER)) {
            int64_t val = currentToken.intValue;
            nextToken();
            // Use targetType hint if set, otherwise default to i32
            PLCScriptVarType constType = (targetType != PSTYPE_VOID && targetType != PSTYPE_F32 && targetType != PSTYPE_F64) 
                                         ? targetType : PSTYPE_I32;
            // Check if value fits in target type
            bool fits = true;
            switch (constType) {
                case PSTYPE_BOOL: fits = (val == 0 || val == 1); break;
                case PSTYPE_U8:  fits = (val >= 0 && val <= 255); break;
                case PSTYPE_I8:  fits = (val >= -128 && val <= 127); break;
                case PSTYPE_U16: fits = (val >= 0 && val <= 65535); break;
                case PSTYPE_I16: fits = (val >= -32768 && val <= 32767); break;
                case PSTYPE_U32: fits = (val >= 0 && val <= 0xFFFFFFFF); break;
                case PSTYPE_I32: fits = (val >= -2147483648LL && val <= 2147483647LL); break;
                default: break; // u64/i64 always fit
            }
            if (!fits) constType = PSTYPE_I32; // Fall back to i32 if value doesn't fit
            emitLoadConst(constType, val);
            return constType;
        }
        
        // Float literal
        if (check(PSTOK_FLOAT)) {
            double val = currentToken.floatValue;
            nextToken();
            emitLoadConstFloat(PSTYPE_F32, val);
            return PSTYPE_F32;
        }
        
        // Boolean literals
        if (check(PSTOK_BOOL_TRUE)) {
            nextToken();
            emitLoadConst(PSTYPE_BOOL, 1);
            return PSTYPE_BOOL;
        }
        if (check(PSTOK_BOOL_FALSE)) {
            nextToken();
            emitLoadConst(PSTYPE_BOOL, 0);
            return PSTYPE_BOOL;
        }
        
        // Parenthesized expression
        if (check(PSTOK_LPAREN)) {
            nextToken();
            PLCScriptVarType type = parseExpression();
            expect(PSTOK_RPAREN, "Expected ')'");
            return type;
        }
        
        // PLC Address direct access
        if (check(PSTOK_ADDRESS)) {
            char addr[32];
            int i = 0;
            while (currentToken.text[i] && i < 31) {
                addr[i] = currentToken.text[i];
                i++;
            }
            addr[i] = '\0';
            nextToken();
            
            // Determine type from address
            PLCScriptVarType type = PSTYPE_U8;
            char second = addr[1];
            if (second >= 'a' && second <= 'z') second -= 32;
            if (second == 'W') type = PSTYPE_I16;
            else if (second == 'D') type = PSTYPE_I32;
            
            // Check if bit address
            bool isBit = false;
            for (int j = 0; addr[j]; j++) {
                if (addr[j] == '.') { isBit = true; break; }
            }
            
            if (isBit) {
                emit("u8.readBit ");
                emit(addr);
                emit("\n");
                return PSTYPE_BOOL;
            } else {
                emit(varTypeToPlcasm(type));
                emit(".load_from ");
                emit(addr);
                emit("\n");
                return type;
            }
        }
        
        // Identifier (variable or function call)
        if (check(PSTOK_IDENTIFIER)) {
            char name[PLCSCRIPT_MAX_IDENTIFIER_LEN];
            int i = 0;
            while (currentToken.text[i] && i < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
                name[i] = currentToken.text[i];
                i++;
            }
            name[i] = '\0';
            nextToken();
            
            // Check for function call
            if (check(PSTOK_LPAREN)) {
                return parseFunctionCall(name);
            }
            
            // Variable reference
            PLCScriptSymbol* sym = findSymbol(name);
            if (!sym) {
                setError("Undefined variable");
                return PSTYPE_VOID;
            }
            
            emitLoadFromAddress(sym);
            return sym->type;
        }
        
        setError("Expected expression");
        return PSTYPE_VOID;
    }
    
    PLCScriptVarType parseFunctionCall(const char* name) {
        nextToken(); // consume '('
        
        // Built-in PLC functions
        if (strEqCI(name, "TON") || strEqCI(name, "TOF") || strEqCI(name, "TP")) {
            // Timer: TON(T0, condition, preset)
            // First arg: timer address
            if (!check(PSTOK_ADDRESS) && !check(PSTOK_IDENTIFIER)) {
                setError("Expected timer address");
                return PSTYPE_VOID;
            }
            char timerAddr[32];
            int i = 0;
            while (currentToken.text[i] && i < 31) {
                timerAddr[i] = currentToken.text[i];
                i++;
            }
            timerAddr[i] = '\0';
            nextToken();
            
            expect(PSTOK_COMMA, "Expected ','");
            
            // Second arg: condition (bool expression)
            parseExpression();
            
            expect(PSTOK_COMMA, "Expected ','");
            
            // Third arg: preset time (u32)
            parseExpression();
            
            expect(PSTOK_RPAREN, "Expected ')'");
            
            // Emit timer instruction
            if (strEqCI(name, "TON")) emit("ton.const ");
            else if (strEqCI(name, "TOF")) emit("tof.const ");
            else emit("tp.const ");
            emit(timerAddr);
            emit("\n");
            
            return PSTYPE_BOOL;
        }
        
        if (strEqCI(name, "CTU") || strEqCI(name, "CTD")) {
            // Counter: CTU(C0, countUp, reset, preset)
            if (!check(PSTOK_ADDRESS) && !check(PSTOK_IDENTIFIER)) {
                setError("Expected counter address");
                return PSTYPE_VOID;
            }
            char counterAddr[32];
            int i = 0;
            while (currentToken.text[i] && i < 31) {
                counterAddr[i] = currentToken.text[i];
                i++;
            }
            counterAddr[i] = '\0';
            nextToken();
            
            expect(PSTOK_COMMA, "Expected ','");
            parseExpression(); // count signal
            expect(PSTOK_COMMA, "Expected ','");
            parseExpression(); // reset/load signal
            expect(PSTOK_COMMA, "Expected ','");
            parseExpression(); // preset value
            expect(PSTOK_RPAREN, "Expected ')'");
            
            if (strEqCI(name, "CTU")) emit("ctu.const ");
            else emit("ctd.const ");
            emit(counterAddr);
            emit("\n");
            
            return PSTYPE_BOOL;
        }
        
        if (strEqCI(name, "risingEdge") || strEqCI(name, "FP")) {
            // risingEdge(input, memoryBit)
            parseExpression(); // input signal
            expect(PSTOK_COMMA, "Expected ','");
            
            if (!check(PSTOK_ADDRESS) && !check(PSTOK_IDENTIFIER)) {
                setError("Expected memory address");
                return PSTYPE_VOID;
            }
            char memAddr[32];
            int i = 0;
            while (currentToken.text[i] && i < 31) {
                memAddr[i] = currentToken.text[i];
                i++;
            }
            memAddr[i] = '\0';
            nextToken();
            
            expect(PSTOK_RPAREN, "Expected ')'");
            
            emit("stack.du ");
            emit(memAddr);
            emit("\n");
            
            return PSTYPE_BOOL;
        }
        
        if (strEqCI(name, "fallingEdge") || strEqCI(name, "FN")) {
            parseExpression();
            expect(PSTOK_COMMA, "Expected ','");
            
            if (!check(PSTOK_ADDRESS) && !check(PSTOK_IDENTIFIER)) {
                setError("Expected memory address");
                return PSTYPE_VOID;
            }
            char memAddr[32];
            int i = 0;
            while (currentToken.text[i] && i < 31) {
                memAddr[i] = currentToken.text[i];
                i++;
            }
            memAddr[i] = '\0';
            nextToken();
            
            expect(PSTOK_RPAREN, "Expected ')'");
            
            emit("stack.dd ");
            emit(memAddr);
            emit("\n");
            
            return PSTYPE_BOOL;
        }
        
        // Unknown function
        setError("Unknown function");
        return PSTYPE_VOID;
    }
};

// ============================================================================
// WASM Exports
// ============================================================================

// ============ Global Instance & WASM Exports ============

PLCScriptCompiler& getStandalonePLCScriptCompiler() {
    static PLCScriptCompiler instance;
    return instance;
}
#define __standalone_plcscript_compiler__ getStandalonePLCScriptCompiler()

static char plcscript_source_buffer[PLCSCRIPT_MAX_OUTPUT_SIZE];
static int plcscript_source_length = 0;

// Load PLCScript source from stream buffer (call after streaming in the PLCScript code)
WASM_EXPORT void plcscript_load_from_stream() {
    streamRead(plcscript_source_buffer, plcscript_source_length, PLCSCRIPT_MAX_OUTPUT_SIZE);
    __standalone_plcscript_compiler__.setSource(plcscript_source_buffer, plcscript_source_length);
}

// Compile PLCScript to PLCASM (stream-based API)
WASM_EXPORT bool plcscript_compile() {
    return __standalone_plcscript_compiler__.compile();
}

// Check for errors
WASM_EXPORT bool plcscript_hasError() {
    return __standalone_plcscript_compiler__.hasError;
}

// Get error message pointer
WASM_EXPORT const char* plcscript_getError() {
    return __standalone_plcscript_compiler__.errorMessage;
}

// Get error line
WASM_EXPORT int plcscript_getErrorLine() {
    return __standalone_plcscript_compiler__.errorLine;
}

// Get error column
WASM_EXPORT int plcscript_getErrorColumn() {
    return __standalone_plcscript_compiler__.errorColumn;
}

// Get error token pointer (returns the portion of errorMessage containing the token, or empty)
WASM_EXPORT const char* plcscript_getErrorToken() {
    // PLCScript compiler doesn't have a separate error token, return empty string
    static const char empty[] = "";
    return empty;
}

// Get error source line pointer (returns the current source line where error occurred)
WASM_EXPORT const char* plcscript_getErrorSourceLine() {
    // PLCScript compiler doesn't store the source line separately
    // Return empty string for now
    static const char empty[] = "";
    return empty;
}

// Write PLCScript output to stream (for reading via readStream/readOutBuffer)
WASM_EXPORT void plcscript_output_to_stream() {
    const char* output = __standalone_plcscript_compiler__.output;
    int len = __standalone_plcscript_compiler__.outputLength;
    for (int i = 0; i < len; i++) {
        streamOut(output[i]);
    }
}

// Get output PLCASM pointer
WASM_EXPORT const char* plcscript_getOutput() {
    return __standalone_plcscript_compiler__.output;
}

// Get output length
WASM_EXPORT int plcscript_getOutputLength() {
    return __standalone_plcscript_compiler__.outputLength;
}

// Reset the PLCScript compiler
WASM_EXPORT void plcscript_reset() {
    __standalone_plcscript_compiler__.reset();
}

// ============================================================================
// Legacy WASM Exports (PascalCase naming for backward compatibility)
// ============================================================================

WASM_EXPORT void PLCScript_setSource(const char* source, int length) {
    if (length >= PLCSCRIPT_MAX_OUTPUT_SIZE) {
        length = PLCSCRIPT_MAX_OUTPUT_SIZE - 1;
    }
    for (int i = 0; i < length; i++) {
        plcscript_source_buffer[i] = source[i];
    }
    plcscript_source_buffer[length] = '\0';
    __standalone_plcscript_compiler__.setSource(plcscript_source_buffer, length);
}

WASM_EXPORT bool PLCScript_compile() {
    return __standalone_plcscript_compiler__.compile();
}

WASM_EXPORT const char* PLCScript_getOutput() {
    return __standalone_plcscript_compiler__.output;
}

WASM_EXPORT int PLCScript_getOutputLength() {
    return __standalone_plcscript_compiler__.outputLength;
}

WASM_EXPORT bool PLCScript_hasError() {
    return __standalone_plcscript_compiler__.hasError;
}

WASM_EXPORT const char* PLCScript_getErrorMessage() {
    return __standalone_plcscript_compiler__.errorMessage;
}

WASM_EXPORT int PLCScript_getErrorLine() {
    return __standalone_plcscript_compiler__.errorLine;
}

WASM_EXPORT int PLCScript_getErrorColumn() {
    return __standalone_plcscript_compiler__.errorColumn;
}

#endif // __WASM__
