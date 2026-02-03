// st-compiler.h - Structured Text (ST) to PLCScript Transpiler
// IEC 61131-3 Structured Text compiler that outputs PLCScript
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

#include "shared-symbols.h"

// ============================================================================
// Structured Text (ST) Language Specification - IEC 61131-3
// ============================================================================
//
// Structured Text is a high-level Pascal-like language for PLC programming.
// This compiler transpiles ST to PLCScript for further compilation.
//
// ============================================================================
// Variable Declarations
// ============================================================================
//
// VAR
//     name : TYPE;                    // Local variable
//     name : TYPE := initial_value;   // With initialization
//     name AT %address : TYPE;        // At specific address
// END_VAR
//
// VAR_INPUT / VAR_OUTPUT / VAR_IN_OUT for function block parameters
//
// Types:
//   BOOL        - Boolean
//   SINT, INT, DINT, LINT  - Signed integers (8, 16, 32, 64 bit)
//   USINT, UINT, UDINT, ULINT - Unsigned integers
//   BYTE, WORD, DWORD, LWORD  - Bit strings
//   REAL, LREAL - Floating point (32, 64 bit)
//   TIME        - Duration
//
// ============================================================================
// Operators
// ============================================================================
//
// Assignment:   :=
// Arithmetic:   +, -, *, /, MOD
// Comparison:   =, <>, <, >, <=, >=
// Logical:      AND, OR, XOR, NOT, &, |
// Bitwise:      AND, OR, XOR, NOT, SHL, SHR
//
// ============================================================================
// Control Structures
// ============================================================================
//
// IF condition THEN
//     statements;
// ELSIF condition THEN
//     statements;
// ELSE
//     statements;
// END_IF;
//
// CASE selector OF
//     value1: statements;
//     value2, value3: statements;
//     ELSE statements;
// END_CASE;
//
// FOR counter := start TO end BY step DO
//     statements;
// END_FOR;
//
// WHILE condition DO
//     statements;
// END_WHILE;
//
// REPEAT
//     statements;
// UNTIL condition
// END_REPEAT;
//
// EXIT;    - Exit loop
// RETURN;  - Return from function
//
// ============================================================================
// Comments
// ============================================================================
//
// (* Block comment *)
// // Line comment (extension)
//
// ============================================================================

#define ST_MAX_OUTPUT_SIZE 65536
#define ST_MAX_IDENTIFIER_LEN 64
#define ST_MAX_SYMBOLS 256

// ============================================================================
// Token Types
// ============================================================================

enum STTokenType {
    // Literals
    STTOK_INTEGER,
    STTOK_FLOAT,
    STTOK_STRING,
    STTOK_BOOL_TRUE,
    STTOK_BOOL_FALSE,
    
    // Identifiers and addresses
    STTOK_IDENTIFIER,
    STTOK_ADDRESS,      // %IX0.0, %QX0.0, %MW10, etc.
    
    // Keywords - Declarations
    STTOK_VAR,
    STTOK_VAR_INPUT,
    STTOK_VAR_OUTPUT,
    STTOK_VAR_IN_OUT,
    STTOK_VAR_GLOBAL,
    STTOK_VAR_TEMP,
    STTOK_END_VAR,
    STTOK_CONSTANT,
    STTOK_AT,
    
    // Keywords - Types
    STTOK_TYPE_BOOL,
    STTOK_TYPE_SINT,
    STTOK_TYPE_INT,
    STTOK_TYPE_DINT,
    STTOK_TYPE_LINT,
    STTOK_TYPE_USINT,
    STTOK_TYPE_UINT,
    STTOK_TYPE_UDINT,
    STTOK_TYPE_ULINT,
    STTOK_TYPE_BYTE,
    STTOK_TYPE_WORD,
    STTOK_TYPE_DWORD,
    STTOK_TYPE_LWORD,
    STTOK_TYPE_REAL,
    STTOK_TYPE_LREAL,
    STTOK_TYPE_TIME,
    STTOK_TYPE_STRING,
    
    // Keywords - Control flow
    STTOK_IF,
    STTOK_THEN,
    STTOK_ELSIF,
    STTOK_ELSE,
    STTOK_END_IF,
    STTOK_CASE,
    STTOK_OF,
    STTOK_END_CASE,
    STTOK_FOR,
    STTOK_TO,
    STTOK_BY,
    STTOK_DO,
    STTOK_END_FOR,
    STTOK_WHILE,
    STTOK_END_WHILE,
    STTOK_REPEAT,
    STTOK_UNTIL,
    STTOK_END_REPEAT,
    STTOK_EXIT,
    STTOK_RETURN,
    
    // Keywords - Logical operators
    STTOK_AND,
    STTOK_OR,
    STTOK_XOR,
    STTOK_NOT,
    STTOK_MOD,
    STTOK_SHL,
    STTOK_SHR,
    
    // Keywords - PLC functions
    STTOK_TON,
    STTOK_TOF,
    STTOK_TP,
    STTOK_CTU,
    STTOK_CTD,
    STTOK_CTUD,
    STTOK_R_TRIG,
    STTOK_F_TRIG,
    
    // Punctuation
    STTOK_LPAREN,       // (
    STTOK_RPAREN,       // )
    STTOK_LBRACKET,     // [
    STTOK_RBRACKET,     // ]
    STTOK_SEMICOLON,    // ;
    STTOK_COLON,        // :
    STTOK_COMMA,        // ,
    STTOK_DOT,          // .
    STTOK_DOTDOT,       // ..
    STTOK_HASH,         // #
    
    // Operators
    STTOK_ASSIGN,       // :=
    STTOK_PLUS,         // +
    STTOK_MINUS,        // -
    STTOK_STAR,         // *
    STTOK_SLASH,        // /
    STTOK_EQ,           // =
    STTOK_NEQ,          // <>
    STTOK_LT,           // <
    STTOK_GT,           // >
    STTOK_LTE,          // <=
    STTOK_GTE,          // >=
    STTOK_AMPERSAND,    // & (AND)
    STTOK_PIPE,         // | (OR - extension)
    STTOK_CARET,        // ^ (XOR - extension)
    
    // Special
    STTOK_EOF,
    STTOK_ERROR
};

// ============================================================================
// Token Structure
// ============================================================================

struct STToken {
    STTokenType type;
    char text[ST_MAX_IDENTIFIER_LEN];
    int64_t intValue;
    double floatValue;
    int line;
    int column;
};

// ============================================================================
// Symbol for ST variables
// ============================================================================

struct STSymbol {
    char name[ST_MAX_IDENTIFIER_LEN];
    char plcscriptType[16];     // PLCScript type (u8, i16, u32, f32, etc.)
    char address[32];           // PLC address or "auto"
    bool hasInitializer;
    int64_t initInt;
    double initFloat;
    bool isConstant;
};

// ============================================================================
// ST to PLCScript Compiler
// ============================================================================

class STCompiler {
public:
    // Input
    const char* source;
    int source_length;
    int pos;
    int line;
    int column;
    
    // Output
    char output[ST_MAX_OUTPUT_SIZE];
    int output_len;
    
    // Current token
    STToken currentToken;
    STToken peekToken;
    bool hasPeekToken;
    
    // Symbol table
    STSymbol symbols[ST_MAX_SYMBOLS];
    int symbol_count;
    
    // Shared symbols (from project)
    SharedSymbolTable& sharedSymbols;
    
    // Error handling
    bool has_error;
    char error_message[256];
    char error_token_text[64];  // Debug: token text when error occurred
    int error_token_type;       // Debug: token type when error occurred
    int error_line;
    int error_column;
    
    // Indentation level for output
    int indent_level;
    
    // Auto address counter
    int auto_addr_counter;
    
    STCompiler(SharedSymbolTable& shared) : sharedSymbols(shared) {
        reset();
    }
    
    void reset() {
        source = nullptr;
        source_length = 0;
        pos = 0;
        line = 1;
        column = 1;
        output_len = 0;
        output[0] = '\0';
        symbol_count = 0;
        has_error = false;
        error_message[0] = '\0';
        error_token_text[0] = '\0';
        error_token_type = 0;
        error_line = 0;
        error_column = 0;
        hasPeekToken = false;
        indent_level = 0;
        auto_addr_counter = 100;  // Start auto addresses at M100
    }
    
    void setSource(const char* src, int len) {
        source = src;
        source_length = len;
        pos = 0;
        line = 1;
        column = 1;
    }
    
    // ========================================================================
    // Error handling
    // ========================================================================
    
    void setError(const char* msg) {
        if (has_error) return;
        has_error = true;
        error_line = currentToken.line;
        error_column = currentToken.column;
        error_token_type = (int)currentToken.type;
        // Copy token text for debug
        int j = 0;
        while (currentToken.text[j] && j < 63) {
            error_token_text[j] = currentToken.text[j];
            j++;
        }
        error_token_text[j] = '\0';
        int i = 0;
        while (msg[i] && i < 255) {
            error_message[i] = msg[i];
            i++;
        }
        error_message[i] = '\0';
    }
    
    // ========================================================================
    // Output emission
    // ========================================================================
    
    void emit(const char* str) {
        while (*str && output_len < ST_MAX_OUTPUT_SIZE - 1) {
            output[output_len++] = *str++;
        }
        output[output_len] = '\0';
    }
    
    void emitChar(char c) {
        if (output_len < ST_MAX_OUTPUT_SIZE - 1) {
            output[output_len++] = c;
            output[output_len] = '\0';
        }
    }
    
    void emitLine(const char* str) {
        emit(str);
        emitChar('\n');
    }
    
    void emitIndent() {
        for (int i = 0; i < indent_level; i++) {
            emit("    ");
        }
    }
    
    void emitInt(int64_t val) {
        char buf[24];
        int idx = 0;
        bool negative = val < 0;
        if (negative) val = -val;
        if (val == 0) {
            buf[idx++] = '0';
        } else {
            while (val > 0) {
                buf[idx++] = '0' + (val % 10);
                val /= 10;
            }
        }
        if (negative) emitChar('-');
        while (idx > 0) emitChar(buf[--idx]);
    }
    
    void emitFloat(double val) {
        // Simple float formatting
        if (val < 0) {
            emitChar('-');
            val = -val;
        }
        int64_t intPart = (int64_t)val;
        double fracPart = val - intPart;
        emitInt(intPart);
        emitChar('.');
        fracPart *= 1000000;
        int64_t fracInt = (int64_t)(fracPart + 0.5);
        // Pad with leading zeros
        if (fracInt < 100000) emitChar('0');
        if (fracInt < 10000) emitChar('0');
        if (fracInt < 1000) emitChar('0');
        if (fracInt < 100) emitChar('0');
        if (fracInt < 10) emitChar('0');
        emitInt(fracInt);
    }
    
    // ========================================================================
    // Lexer helpers
    // ========================================================================
    
    char peek() {
        if (pos >= source_length) return '\0';
        return source[pos];
    }
    
    char peekAhead(int offset) {
        if (pos + offset >= source_length) return '\0';
        return source[pos + offset];
    }
    
    bool peekString(const char* str) {
        int i = 0;
        while (str[i]) {
            if (pos + i >= source_length) return false;
            if (source[pos + i] != str[i]) return false;
            i++;
        }
        return true;
    }
    
    char advance() {
        if (pos >= source_length) return '\0';
        char c = source[pos++];
        if (c == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        return c;
    }
    
    void skipWhitespace() {
        while (pos < source_length) {
            char c = peek();
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                advance();
            } else if (c == '(' && peekAhead(1) == '*') {
                // Block comment (* ... *)
                advance(); advance();
                while (pos < source_length - 1) {
                    if (peek() == '*' && peekAhead(1) == ')') {
                        advance(); advance();
                        break;
                    }
                    advance();
                }
            } else if (c == '/' && peekAhead(1) == '/') {
                // Line comment
                while (pos < source_length && peek() != '\n') {
                    advance();
                }
            } else {
                break;
            }
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
    
    char toUpper(char c) {
        if (c >= 'a' && c <= 'z') return c - 32;
        return c;
    }
    
    bool strEqCI(const char* a, const char* b) {
        while (*a && *b) {
            if (toUpper(*a) != toUpper(*b)) return false;
            a++; b++;
        }
        return *a == *b;
    }
    
    // Check if a keyword matches at current position (case-insensitive) 
    // followed by a non-identifier character
    bool peekKeyword(const char* keyword) {
        int i = 0;
        while (keyword[i]) {
            if (pos + i >= source_length) return false;
            if (toUpper(source[pos + i]) != toUpper(keyword[i])) return false;
            i++;
        }
        // Make sure keyword is not followed by alphanumeric or underscore
        if (pos + i < source_length) {
            char next = source[pos + i];
            if (isAlphaNum(next) || next == '_') return false;
        }
        return true;
    }
    
    // Advance past a keyword (use after peekKeyword returns true)
    void advanceKeyword(const char* keyword, char* text) {
        int i = 0;
        while (keyword[i]) {
            if (i < ST_MAX_IDENTIFIER_LEN - 1) {
                text[i] = advance();
            } else {
                advance();
            }
            i++;
        }
        text[i] = '\0';
    }
    
    // ========================================================================
    // Token scanning
    // ========================================================================
    
    STToken scanToken() {
        skipWhitespace();
        
        STToken token;
        token.line = line;
        token.column = column;
        token.text[0] = '\0';
        token.intValue = 0;
        token.floatValue = 0.0;
        
        if (pos >= source_length) {
            token.type = STTOK_EOF;
            return token;
        }
        
        char c = peek();
        
        // IEC address %IX0.0, %QX0.0, %MW10, etc.
        if (c == '%') {
            int i = 0;
            token.text[i++] = advance();
            while (isAlphaNum(peek()) || peek() == '.') {
                if (i < ST_MAX_IDENTIFIER_LEN - 1) {
                    token.text[i++] = advance();
                } else {
                    advance();
                }
            }
            token.text[i] = '\0';
            token.type = STTOK_ADDRESS;
            return token;
        }
        
        // Number (integer or float)
        if (isDigit(c) || (c == '-' && isDigit(peekAhead(1)))) {
            bool negative = false;
            if (c == '-') {
                negative = true;
                advance();
            }
            
            int64_t intVal = 0;
            while (isDigit(peek())) {
                intVal = intVal * 10 + (advance() - '0');
            }
            
            if (peek() == '.' && isDigit(peekAhead(1))) {
                advance(); // skip '.'
                double fracVal = 0.0;
                double fracDiv = 10.0;
                while (isDigit(peek())) {
                    fracVal += (advance() - '0') / fracDiv;
                    fracDiv *= 10.0;
                }
                token.floatValue = (double)intVal + fracVal;
                if (negative) token.floatValue = -token.floatValue;
                token.type = STTOK_FLOAT;
            } else {
                token.intValue = negative ? -intVal : intVal;
                token.type = STTOK_INTEGER;
            }
            return token;
        }
        
        // Time literal T#... or TIME#...
        if ((c == 'T' || c == 't') && peekAhead(1) == '#') {
            advance(); advance(); // skip T#
            int i = 0;
            while (isAlphaNum(peek()) || peek() == '_') {
                if (i < ST_MAX_IDENTIFIER_LEN - 1) {
                    token.text[i++] = advance();
                } else {
                    advance();
                }
            }
            token.text[i] = '\0';
            // Parse time value (simplified: just extract ms)
            // Format: T#100ms, T#1s, T#1m, etc.
            token.intValue = parseTimeValue(token.text);
            token.type = STTOK_INTEGER;
            return token;
        }
        
        // String literal
        if (c == '\'') {
            advance();
            int i = 0;
            while (peek() != '\'' && peek() != '\0') {
                if (i < ST_MAX_IDENTIFIER_LEN - 1) {
                    token.text[i++] = advance();
                } else {
                    advance();
                }
            }
            token.text[i] = '\0';
            if (peek() == '\'') advance();
            token.type = STTOK_STRING;
            return token;
        }
        
        // Keywords and identifiers - check keywords with underscore first (longest match)
        if (isAlpha(c)) {
            // Check compound keywords first (with underscores) - longest match first
            if (peekKeyword("VAR_INPUT")) { advanceKeyword("VAR_INPUT", token.text); token.type = STTOK_VAR_INPUT; return token; }
            if (peekKeyword("VAR_OUTPUT")) { advanceKeyword("VAR_OUTPUT", token.text); token.type = STTOK_VAR_OUTPUT; return token; }
            if (peekKeyword("VAR_IN_OUT")) { advanceKeyword("VAR_IN_OUT", token.text); token.type = STTOK_VAR_IN_OUT; return token; }
            if (peekKeyword("VAR_GLOBAL")) { advanceKeyword("VAR_GLOBAL", token.text); token.type = STTOK_VAR_GLOBAL; return token; }
            if (peekKeyword("VAR_TEMP")) { advanceKeyword("VAR_TEMP", token.text); token.type = STTOK_VAR_TEMP; return token; }
            if (peekKeyword("END_VAR")) { advanceKeyword("END_VAR", token.text); token.type = STTOK_END_VAR; return token; }
            if (peekKeyword("END_IF")) { advanceKeyword("END_IF", token.text); token.type = STTOK_END_IF; return token; }
            if (peekKeyword("END_CASE")) { advanceKeyword("END_CASE", token.text); token.type = STTOK_END_CASE; return token; }
            if (peekKeyword("END_FOR")) { advanceKeyword("END_FOR", token.text); token.type = STTOK_END_FOR; return token; }
            if (peekKeyword("END_WHILE")) { advanceKeyword("END_WHILE", token.text); token.type = STTOK_END_WHILE; return token; }
            if (peekKeyword("END_REPEAT")) { advanceKeyword("END_REPEAT", token.text); token.type = STTOK_END_REPEAT; return token; }
            
            // Simple keywords (no underscore)
            if (peekKeyword("VAR")) { advanceKeyword("VAR", token.text); token.type = STTOK_VAR; return token; }
            if (peekKeyword("CONSTANT")) { advanceKeyword("CONSTANT", token.text); token.type = STTOK_CONSTANT; return token; }
            if (peekKeyword("AT")) { advanceKeyword("AT", token.text); token.type = STTOK_AT; return token; }
            
            // Types
            if (peekKeyword("BOOL")) { advanceKeyword("BOOL", token.text); token.type = STTOK_TYPE_BOOL; return token; }
            if (peekKeyword("SINT")) { advanceKeyword("SINT", token.text); token.type = STTOK_TYPE_SINT; return token; }
            if (peekKeyword("DINT")) { advanceKeyword("DINT", token.text); token.type = STTOK_TYPE_DINT; return token; }
            if (peekKeyword("LINT")) { advanceKeyword("LINT", token.text); token.type = STTOK_TYPE_LINT; return token; }
            if (peekKeyword("USINT")) { advanceKeyword("USINT", token.text); token.type = STTOK_TYPE_USINT; return token; }
            if (peekKeyword("UINT")) { advanceKeyword("UINT", token.text); token.type = STTOK_TYPE_UINT; return token; }
            if (peekKeyword("UDINT")) { advanceKeyword("UDINT", token.text); token.type = STTOK_TYPE_UDINT; return token; }
            if (peekKeyword("ULINT")) { advanceKeyword("ULINT", token.text); token.type = STTOK_TYPE_ULINT; return token; }
            if (peekKeyword("INT")) { advanceKeyword("INT", token.text); token.type = STTOK_TYPE_INT; return token; }
            if (peekKeyword("BYTE")) { advanceKeyword("BYTE", token.text); token.type = STTOK_TYPE_BYTE; return token; }
            if (peekKeyword("WORD")) { advanceKeyword("WORD", token.text); token.type = STTOK_TYPE_WORD; return token; }
            if (peekKeyword("DWORD")) { advanceKeyword("DWORD", token.text); token.type = STTOK_TYPE_DWORD; return token; }
            if (peekKeyword("LWORD")) { advanceKeyword("LWORD", token.text); token.type = STTOK_TYPE_LWORD; return token; }
            if (peekKeyword("LREAL")) { advanceKeyword("LREAL", token.text); token.type = STTOK_TYPE_LREAL; return token; }
            if (peekKeyword("REAL")) { advanceKeyword("REAL", token.text); token.type = STTOK_TYPE_REAL; return token; }
            if (peekKeyword("TIME")) { advanceKeyword("TIME", token.text); token.type = STTOK_TYPE_TIME; return token; }
            if (peekKeyword("STRING")) { advanceKeyword("STRING", token.text); token.type = STTOK_TYPE_STRING; return token; }
            
            // Control flow
            if (peekKeyword("ELSIF")) { advanceKeyword("ELSIF", token.text); token.type = STTOK_ELSIF; return token; }
            if (peekKeyword("ELSE")) { advanceKeyword("ELSE", token.text); token.type = STTOK_ELSE; return token; }
            if (peekKeyword("IF")) { advanceKeyword("IF", token.text); token.type = STTOK_IF; return token; }
            if (peekKeyword("THEN")) { advanceKeyword("THEN", token.text); token.type = STTOK_THEN; return token; }
            if (peekKeyword("CASE")) { advanceKeyword("CASE", token.text); token.type = STTOK_CASE; return token; }
            if (peekKeyword("OF")) { advanceKeyword("OF", token.text); token.type = STTOK_OF; return token; }
            if (peekKeyword("FOR")) { advanceKeyword("FOR", token.text); token.type = STTOK_FOR; return token; }
            if (peekKeyword("TO")) { advanceKeyword("TO", token.text); token.type = STTOK_TO; return token; }
            if (peekKeyword("BY")) { advanceKeyword("BY", token.text); token.type = STTOK_BY; return token; }
            if (peekKeyword("DO")) { advanceKeyword("DO", token.text); token.type = STTOK_DO; return token; }
            if (peekKeyword("WHILE")) { advanceKeyword("WHILE", token.text); token.type = STTOK_WHILE; return token; }
            if (peekKeyword("REPEAT")) { advanceKeyword("REPEAT", token.text); token.type = STTOK_REPEAT; return token; }
            if (peekKeyword("UNTIL")) { advanceKeyword("UNTIL", token.text); token.type = STTOK_UNTIL; return token; }
            if (peekKeyword("EXIT")) { advanceKeyword("EXIT", token.text); token.type = STTOK_EXIT; return token; }
            if (peekKeyword("RETURN")) { advanceKeyword("RETURN", token.text); token.type = STTOK_RETURN; return token; }
            
            // Logical/bitwise operators
            if (peekKeyword("AND")) { advanceKeyword("AND", token.text); token.type = STTOK_AND; return token; }
            if (peekKeyword("XOR")) { advanceKeyword("XOR", token.text); token.type = STTOK_XOR; return token; }
            if (peekKeyword("NOT")) { advanceKeyword("NOT", token.text); token.type = STTOK_NOT; return token; }
            if (peekKeyword("MOD")) { advanceKeyword("MOD", token.text); token.type = STTOK_MOD; return token; }
            if (peekKeyword("SHL")) { advanceKeyword("SHL", token.text); token.type = STTOK_SHL; return token; }
            if (peekKeyword("SHR")) { advanceKeyword("SHR", token.text); token.type = STTOK_SHR; return token; }
            if (peekKeyword("OR")) { advanceKeyword("OR", token.text); token.type = STTOK_OR; return token; }
            
            // Boolean literals
            if (peekKeyword("TRUE")) { advanceKeyword("TRUE", token.text); token.type = STTOK_BOOL_TRUE; return token; }
            if (peekKeyword("FALSE")) { advanceKeyword("FALSE", token.text); token.type = STTOK_BOOL_FALSE; return token; }
            
            // PLC function blocks
            if (peekKeyword("TON")) { advanceKeyword("TON", token.text); token.type = STTOK_TON; return token; }
            if (peekKeyword("TOF")) { advanceKeyword("TOF", token.text); token.type = STTOK_TOF; return token; }
            if (peekKeyword("TP")) { advanceKeyword("TP", token.text); token.type = STTOK_TP; return token; }
            if (peekKeyword("CTUD")) { advanceKeyword("CTUD", token.text); token.type = STTOK_CTUD; return token; }
            if (peekKeyword("CTU")) { advanceKeyword("CTU", token.text); token.type = STTOK_CTU; return token; }
            if (peekKeyword("CTD")) { advanceKeyword("CTD", token.text); token.type = STTOK_CTD; return token; }
            if (peekKeyword("R_TRIG")) { advanceKeyword("R_TRIG", token.text); token.type = STTOK_R_TRIG; return token; }
            if (peekKeyword("F_TRIG")) { advanceKeyword("F_TRIG", token.text); token.type = STTOK_F_TRIG; return token; }
            
            // Not a keyword, scan as identifier
            int i = 0;
            while (isAlphaNum(peek()) || peek() == '_') {
                if (i < ST_MAX_IDENTIFIER_LEN - 1) {
                    token.text[i++] = advance();
                } else {
                    advance();
                }
            }
            token.text[i] = '\0';
            token.type = STTOK_IDENTIFIER;
            return token;
        }
        
        // Identifier starting with underscore
        if (c == '_') {
            int i = 0;
            while (isAlphaNum(peek()) || peek() == '_') {
                if (i < ST_MAX_IDENTIFIER_LEN - 1) {
                    token.text[i++] = advance();
                } else {
                    advance();
                }
            }
            token.text[i] = '\0';
            token.type = STTOK_IDENTIFIER;
            return token;
        }
        
        // Two-character operators
        if (c == ':' && peekAhead(1) == '=') {
            advance(); advance();
            token.type = STTOK_ASSIGN;
            return token;
        }
        if (c == '<' && peekAhead(1) == '>') {
            advance(); advance();
            token.type = STTOK_NEQ;
            return token;
        }
        if (c == '<' && peekAhead(1) == '=') {
            advance(); advance();
            token.type = STTOK_LTE;
            return token;
        }
        if (c == '>' && peekAhead(1) == '=') {
            advance(); advance();
            token.type = STTOK_GTE;
            return token;
        }
        if (c == '.' && peekAhead(1) == '.') {
            advance(); advance();
            token.type = STTOK_DOTDOT;
            return token;
        }
        
        // Single-character tokens
        advance();
        switch (c) {
            case '(': token.type = STTOK_LPAREN; break;
            case ')': token.type = STTOK_RPAREN; break;
            case '[': token.type = STTOK_LBRACKET; break;
            case ']': token.type = STTOK_RBRACKET; break;
            case ';': token.type = STTOK_SEMICOLON; break;
            case ':': token.type = STTOK_COLON; break;
            case ',': token.type = STTOK_COMMA; break;
            case '.': token.type = STTOK_DOT; break;
            case '#': token.type = STTOK_HASH; break;
            case '+': token.type = STTOK_PLUS; break;
            case '-': token.type = STTOK_MINUS; break;
            case '*': token.type = STTOK_STAR; break;
            case '/': token.type = STTOK_SLASH; break;
            case '=': token.type = STTOK_EQ; break;
            case '<': token.type = STTOK_LT; break;
            case '>': token.type = STTOK_GT; break;
            case '&': token.type = STTOK_AMPERSAND; break;
            case '|': token.type = STTOK_PIPE; break;
            case '^': token.type = STTOK_CARET; break;
            default:
                token.type = STTOK_ERROR;
                token.text[0] = c;
                token.text[1] = '\0';
                break;
        }
        return token;
    }
    
    int64_t parseTimeValue(const char* text) {
        // Parse T#100ms, T#1s, T#1m, etc. to milliseconds
        int64_t value = 0;
        int64_t num = 0;
        const char* p = text;
        
        while (*p) {
            if (isDigit(*p)) {
                num = num * 10 + (*p - '0');
            } else if (*p == 'm' && *(p+1) == 's') {
                value += num;
                num = 0;
                p++;
            } else if (*p == 's' && *(p+1) != 'i') {
                value += num * 1000;
                num = 0;
            } else if (*p == 'm' && (*(p+1) == '\0' || *(p+1) == '_' || isDigit(*(p+1)))) {
                value += num * 60000;
                num = 0;
            } else if (*p == 'h') {
                value += num * 3600000;
                num = 0;
            }
            p++;
        }
        return value + num; // remaining number treated as ms
    }
    
    STTokenType identifyKeyword(const char* text) {
        // Keywords
        if (strEqCI(text, "VAR")) return STTOK_VAR;
        if (strEqCI(text, "VAR_INPUT")) return STTOK_VAR_INPUT;
        if (strEqCI(text, "VAR_OUTPUT")) return STTOK_VAR_OUTPUT;
        if (strEqCI(text, "VAR_IN_OUT")) return STTOK_VAR_IN_OUT;
        if (strEqCI(text, "VAR_GLOBAL")) return STTOK_VAR_GLOBAL;
        if (strEqCI(text, "VAR_TEMP")) return STTOK_VAR_TEMP;
        if (strEqCI(text, "END_VAR")) return STTOK_END_VAR;
        if (strEqCI(text, "CONSTANT")) return STTOK_CONSTANT;
        if (strEqCI(text, "AT")) return STTOK_AT;
        
        // Types
        if (strEqCI(text, "BOOL")) return STTOK_TYPE_BOOL;
        if (strEqCI(text, "SINT")) return STTOK_TYPE_SINT;
        if (strEqCI(text, "INT")) return STTOK_TYPE_INT;
        if (strEqCI(text, "DINT")) return STTOK_TYPE_DINT;
        if (strEqCI(text, "LINT")) return STTOK_TYPE_LINT;
        if (strEqCI(text, "USINT")) return STTOK_TYPE_USINT;
        if (strEqCI(text, "UINT")) return STTOK_TYPE_UINT;
        if (strEqCI(text, "UDINT")) return STTOK_TYPE_UDINT;
        if (strEqCI(text, "ULINT")) return STTOK_TYPE_ULINT;
        if (strEqCI(text, "BYTE")) return STTOK_TYPE_BYTE;
        if (strEqCI(text, "WORD")) return STTOK_TYPE_WORD;
        if (strEqCI(text, "DWORD")) return STTOK_TYPE_DWORD;
        if (strEqCI(text, "LWORD")) return STTOK_TYPE_LWORD;
        if (strEqCI(text, "REAL")) return STTOK_TYPE_REAL;
        if (strEqCI(text, "LREAL")) return STTOK_TYPE_LREAL;
        if (strEqCI(text, "TIME")) return STTOK_TYPE_TIME;
        if (strEqCI(text, "STRING")) return STTOK_TYPE_STRING;
        
        // Control flow
        if (strEqCI(text, "IF")) return STTOK_IF;
        if (strEqCI(text, "THEN")) return STTOK_THEN;
        if (strEqCI(text, "ELSIF")) return STTOK_ELSIF;
        if (strEqCI(text, "ELSE")) return STTOK_ELSE;
        if (strEqCI(text, "END_IF")) return STTOK_END_IF;
        if (strEqCI(text, "CASE")) return STTOK_CASE;
        if (strEqCI(text, "OF")) return STTOK_OF;
        if (strEqCI(text, "END_CASE")) return STTOK_END_CASE;
        if (strEqCI(text, "FOR")) return STTOK_FOR;
        if (strEqCI(text, "TO")) return STTOK_TO;
        if (strEqCI(text, "BY")) return STTOK_BY;
        if (strEqCI(text, "DO")) return STTOK_DO;
        if (strEqCI(text, "END_FOR")) return STTOK_END_FOR;
        if (strEqCI(text, "WHILE")) return STTOK_WHILE;
        if (strEqCI(text, "END_WHILE")) return STTOK_END_WHILE;
        if (strEqCI(text, "REPEAT")) return STTOK_REPEAT;
        if (strEqCI(text, "UNTIL")) return STTOK_UNTIL;
        if (strEqCI(text, "END_REPEAT")) return STTOK_END_REPEAT;
        if (strEqCI(text, "EXIT")) return STTOK_EXIT;
        if (strEqCI(text, "RETURN")) return STTOK_RETURN;
        
        // Logical operators
        if (strEqCI(text, "AND")) return STTOK_AND;
        if (strEqCI(text, "OR")) return STTOK_OR;
        if (strEqCI(text, "XOR")) return STTOK_XOR;
        if (strEqCI(text, "NOT")) return STTOK_NOT;
        if (strEqCI(text, "MOD")) return STTOK_MOD;
        if (strEqCI(text, "SHL")) return STTOK_SHL;
        if (strEqCI(text, "SHR")) return STTOK_SHR;
        
        // PLC function blocks
        if (strEqCI(text, "TON")) return STTOK_TON;
        if (strEqCI(text, "TOF")) return STTOK_TOF;
        if (strEqCI(text, "TP")) return STTOK_TP;
        if (strEqCI(text, "CTU")) return STTOK_CTU;
        if (strEqCI(text, "CTD")) return STTOK_CTD;
        if (strEqCI(text, "CTUD")) return STTOK_CTUD;
        if (strEqCI(text, "R_TRIG")) return STTOK_R_TRIG;
        if (strEqCI(text, "F_TRIG")) return STTOK_F_TRIG;
        
        // Boolean literals
        if (strEqCI(text, "TRUE")) return STTOK_BOOL_TRUE;
        if (strEqCI(text, "FALSE")) return STTOK_BOOL_FALSE;
        
        return STTOK_IDENTIFIER;
    }
    
    // ========================================================================
    // Token handling
    // ========================================================================
    
    void nextToken() {
        if (hasPeekToken) {
            currentToken = peekToken;
            hasPeekToken = false;
        } else {
            currentToken = scanToken();
        }
    }
    
    STToken& peekNextToken() {
        if (!hasPeekToken) {
            peekToken = scanToken();
            hasPeekToken = true;
        }
        return peekToken;
    }
    
    bool check(STTokenType type) {
        return currentToken.type == type;
    }
    
    bool match(STTokenType type) {
        if (check(type)) {
            nextToken();
            return true;
        }
        return false;
    }
    
    void expect(STTokenType type, const char* msg) {
        if (!match(type)) {
            setError(msg);
        }
    }
    
    bool isTypeToken() {
        return currentToken.type >= STTOK_TYPE_BOOL && currentToken.type <= STTOK_TYPE_STRING;
    }
    
    // ========================================================================
    // Type conversion helpers
    // ========================================================================
    
    const char* stTypeToPLCScript(STTokenType type) {
        switch (type) {
            case STTOK_TYPE_BOOL: return "bool";
            case STTOK_TYPE_SINT: return "i8";
            case STTOK_TYPE_INT: return "i16";
            case STTOK_TYPE_DINT: return "i32";
            case STTOK_TYPE_LINT: return "i64";
            case STTOK_TYPE_USINT: return "u8";
            case STTOK_TYPE_UINT: return "u16";
            case STTOK_TYPE_UDINT: return "u32";
            case STTOK_TYPE_ULINT: return "u64";
            case STTOK_TYPE_BYTE: return "u8";
            case STTOK_TYPE_WORD: return "u16";
            case STTOK_TYPE_DWORD: return "u32";
            case STTOK_TYPE_LWORD: return "u64";
            case STTOK_TYPE_REAL: return "f32";
            case STTOK_TYPE_LREAL: return "f64";
            case STTOK_TYPE_TIME: return "u32";
            default: return "i16";
        }
    }
    
    const char* iecAddressToPLCScript(const char* addr) {
        // Convert %IX0.0 -> X0.0, %QW10 -> YW10, %MW10 -> MW10
        static char buf[32];
        int i = 0, o = 0;
        
        if (addr[i] == '%') i++;
        
        // Map I->X, Q->Y, M->M
        char area = toUpper(addr[i]);
        if (area == 'I') buf[o++] = 'X';
        else if (area == 'Q') buf[o++] = 'Y';
        else buf[o++] = area;
        i++;
        
        // Copy rest (size indicator and number)
        while (addr[i] && o < 30) {
            buf[o++] = addr[i++];
        }
        buf[o] = '\0';
        return buf;
    }
    
    // ========================================================================
    // Symbol table
    // ========================================================================
    
    STSymbol* findSymbol(const char* name) {
        for (int i = 0; i < symbol_count; i++) {
            if (strEqCI(symbols[i].name, name)) {
                return &symbols[i];
            }
        }
        return nullptr;
    }
    
    STSymbol* addSymbol(const char* name, const char* plcType, const char* addr) {
        if (symbol_count >= ST_MAX_SYMBOLS) {
            setError("Too many symbols");
            return nullptr;
        }
        
        STSymbol* sym = &symbols[symbol_count++];
        int i = 0;
        while (name[i] && i < ST_MAX_IDENTIFIER_LEN - 1) {
            sym->name[i] = name[i];
            i++;
        }
        sym->name[i] = '\0';
        
        i = 0;
        while (plcType[i] && i < 15) {
            sym->plcscriptType[i] = plcType[i];
            i++;
        }
        sym->plcscriptType[i] = '\0';
        
        i = 0;
        while (addr[i] && i < 31) {
            sym->address[i] = addr[i];
            i++;
        }
        sym->address[i] = '\0';
        
        sym->hasInitializer = false;
        sym->initInt = 0;
        sym->initFloat = 0.0;
        sym->isConstant = false;
        
        return sym;
    }
    
    // ========================================================================
    // Parsing - Declarations
    // ========================================================================
    
    bool parseVarBlock() {
        bool isConstant = false;
        
        // VAR [CONSTANT]
        if (!match(STTOK_VAR)) {
            // Could be VAR_INPUT, etc.
            if (match(STTOK_VAR_INPUT) || match(STTOK_VAR_OUTPUT) || 
                match(STTOK_VAR_IN_OUT) || match(STTOK_VAR_GLOBAL) || 
                match(STTOK_VAR_TEMP)) {
                // Accept these variants
            } else {
                return false;
            }
        }
        
        if (match(STTOK_CONSTANT)) {
            isConstant = true;
        }
        
        // Parse variable declarations until END_VAR
        while (!check(STTOK_END_VAR) && !check(STTOK_EOF) && !has_error) {
            parseVarDeclaration(isConstant);
        }
        
        expect(STTOK_END_VAR, "Expected END_VAR");
        
        return !has_error;
    }
    
    void parseVarDeclaration(bool isConstant) {
        // name : TYPE [:= initial] ;
        // name AT %address : TYPE [:= initial] ;
        
        if (!check(STTOK_IDENTIFIER)) {
            setError("Expected variable name");
            return;
        }
        
        char name[ST_MAX_IDENTIFIER_LEN];
        int i = 0;
        while (currentToken.text[i] && i < ST_MAX_IDENTIFIER_LEN - 1) {
            name[i] = currentToken.text[i];
            i++;
        }
        name[i] = '\0';
        
        // Check if variable name conflicts with a struct type (exact case match only)
        if (findUserStructTypeExact(name)) {
            setError("Variable name conflicts with a type name");
            return;
        }
        
        nextToken();
        
        // Check for AT %address
        char address[32] = "auto";
        if (match(STTOK_AT)) {
            if (check(STTOK_ADDRESS)) {
                const char* converted = iecAddressToPLCScript(currentToken.text);
                i = 0;
                while (converted[i] && i < 31) {
                    address[i] = converted[i];
                    i++;
                }
                address[i] = '\0';
                nextToken();
            } else {
                setError("Expected address after AT");
                return;
            }
        }
        
        expect(STTOK_COLON, "Expected ':'");
        
        // Type
        if (!isTypeToken()) {
            setError("Expected type");
            return;
        }
        const char* plcType = stTypeToPLCScript(currentToken.type);
        nextToken();
        
        // Create symbol
        STSymbol* sym = addSymbol(name, plcType, address);
        if (!sym) return;
        sym->isConstant = isConstant;
        
        // Check for initialization
        if (match(STTOK_ASSIGN)) {
            sym->hasInitializer = true;
            if (check(STTOK_INTEGER)) {
                sym->initInt = currentToken.intValue;
                nextToken();
            } else if (check(STTOK_FLOAT)) {
                sym->initFloat = currentToken.floatValue;
                nextToken();
            } else if (check(STTOK_BOOL_TRUE)) {
                sym->initInt = 1;
                nextToken();
            } else if (check(STTOK_BOOL_FALSE)) {
                sym->initInt = 0;
                nextToken();
            } else {
                setError("Expected initial value");
                return;
            }
        }
        
        expect(STTOK_SEMICOLON, "Expected ';'");
        
        // Emit PLCScript declaration
        emitIndent();
        emit(isConstant ? "const " : "let ");
        emit(name);
        emit(": ");
        emit(plcType);
        emit(" @ ");
        emit(address);
        if (sym->hasInitializer) {
            emit(" = ");
            if (strEqCI(plcType, "f32") || strEqCI(plcType, "f64")) {
                emitFloat(sym->initFloat);
            } else if (strEqCI(plcType, "bool")) {
                emit(sym->initInt ? "true" : "false");
            } else {
                emitInt(sym->initInt);
            }
        }
        emitLine(";");
    }
    
    // ========================================================================
    // Parsing - Statements
    // ========================================================================
    
    void parseStatements() {
        while (!check(STTOK_EOF) && !check(STTOK_END_IF) && !check(STTOK_ELSIF) &&
               !check(STTOK_ELSE) && !check(STTOK_END_FOR) && !check(STTOK_END_WHILE) &&
               !check(STTOK_END_REPEAT) && !check(STTOK_UNTIL) && !check(STTOK_END_CASE) &&
               !has_error) {
            parseStatement();
        }
    }
    
    void parseStatement() {
        if (check(STTOK_IF)) {
            parseIfStatement();
        } else if (check(STTOK_FOR)) {
            parseForStatement();
        } else if (check(STTOK_WHILE)) {
            parseWhileStatement();
        } else if (check(STTOK_REPEAT)) {
            parseRepeatStatement();
        } else if (check(STTOK_CASE)) {
            parseCaseStatement();
        } else if (check(STTOK_EXIT)) {
            nextToken();
            expect(STTOK_SEMICOLON, "Expected ';'");
            emitIndent();
            emitLine("break;");
        } else if (check(STTOK_RETURN)) {
            nextToken();
            expect(STTOK_SEMICOLON, "Expected ';'");
            emitIndent();
            emitLine("return;");
        } else if (check(STTOK_IDENTIFIER) || check(STTOK_ADDRESS)) {
            parseAssignmentOrCall();
        } else if (check(STTOK_SEMICOLON)) {
            nextToken(); // Empty statement
        } else {
            setError("Unexpected token in statement");
        }
    }
    
    void parseIfStatement() {
        expect(STTOK_IF, "Expected IF");
        
        emitIndent();
        emit("if (");
        parseExpression();
        emitLine(") {");
        
        expect(STTOK_THEN, "Expected THEN");
        
        indent_level++;
        parseStatements();
        indent_level--;
        
        while (check(STTOK_ELSIF)) {
            nextToken();
            emitIndent();
            emit("} else if (");
            parseExpression();
            emitLine(") {");
            
            expect(STTOK_THEN, "Expected THEN");
            
            indent_level++;
            parseStatements();
            indent_level--;
        }
        
        if (check(STTOK_ELSE)) {
            nextToken();
            emitIndent();
            emitLine("} else {");
            indent_level++;
            parseStatements();
            indent_level--;
        }
        
        expect(STTOK_END_IF, "Expected END_IF");
        match(STTOK_SEMICOLON); // Optional semicolon
        
        emitIndent();
        emitLine("}");
    }
    
    void parseForStatement() {
        expect(STTOK_FOR, "Expected FOR");
        
        // FOR counter := start TO end [BY step] DO
        if (!check(STTOK_IDENTIFIER)) {
            setError("Expected loop variable");
            return;
        }
        
        char loopVar[ST_MAX_IDENTIFIER_LEN];
        int i = 0;
        while (currentToken.text[i] && i < ST_MAX_IDENTIFIER_LEN - 1) {
            loopVar[i] = currentToken.text[i];
            i++;
        }
        loopVar[i] = '\0';
        nextToken();
        
        expect(STTOK_ASSIGN, "Expected ':='");
        
        emitIndent();
        emit("for (");
        emit(loopVar);
        emit(" = ");
        parseExpression();
        emit("; ");
        
        expect(STTOK_TO, "Expected TO");
        
        emit(loopVar);
        emit(" <= ");
        parseExpression();
        emit("; ");
        
        // BY step (optional)
        if (match(STTOK_BY)) {
            emit(loopVar);
            emit(" += ");
            parseExpression();
        } else {
            emit(loopVar);
            emit("++");
        }
        
        expect(STTOK_DO, "Expected DO");
        
        emitLine(") {");
        
        indent_level++;
        parseStatements();
        indent_level--;
        
        expect(STTOK_END_FOR, "Expected END_FOR");
        match(STTOK_SEMICOLON);
        
        emitIndent();
        emitLine("}");
    }
    
    void parseWhileStatement() {
        expect(STTOK_WHILE, "Expected WHILE");
        
        emitIndent();
        emit("while (");
        parseExpression();
        
        expect(STTOK_DO, "Expected DO");
        
        emitLine(") {");
        
        indent_level++;
        parseStatements();
        indent_level--;
        
        expect(STTOK_END_WHILE, "Expected END_WHILE");
        match(STTOK_SEMICOLON);
        
        emitIndent();
        emitLine("}");
    }
    
    void parseRepeatStatement() {
        expect(STTOK_REPEAT, "Expected REPEAT");
        
        emitIndent();
        emitLine("do {");
        
        indent_level++;
        parseStatements();
        indent_level--;
        
        expect(STTOK_UNTIL, "Expected UNTIL");
        
        emitIndent();
        emit("} while (!(");
        parseExpression();
        emitLine("));");
        
        expect(STTOK_END_REPEAT, "Expected END_REPEAT");
        match(STTOK_SEMICOLON);
    }
    
    void parseCaseStatement() {
        expect(STTOK_CASE, "Expected CASE");
        
        // Save selector expression
        emitIndent();
        emit("// CASE on ");
        
        // We need to save this for each case branch
        char selector[128];
        int selLen = 0;
        int startOut = output_len;
        
        parseExpression();
        
        // Copy emitted expression back
        selLen = output_len - startOut;
        if (selLen > 127) selLen = 127;
        for (int i = 0; i < selLen; i++) {
            selector[i] = output[startOut + i];
        }
        selector[selLen] = '\0';
        
        emitLine("");
        
        expect(STTOK_OF, "Expected OF");
        
        bool firstCase = true;
        
        while (!check(STTOK_END_CASE) && !check(STTOK_ELSE) && !check(STTOK_EOF) && !has_error) {
            // Case values: value1, value2: statements;
            emitIndent();
            if (firstCase) {
                emit("if (");
                firstCase = false;
            } else {
                emit("} else if (");
            }
            
            // Parse case values (could be multiple separated by commas)
            bool firstVal = true;
            while (!check(STTOK_COLON) && !check(STTOK_EOF) && !has_error) {
                if (!firstVal) {
                    emit(" || ");
                }
                firstVal = false;
                
                emit(selector);
                emit(" == ");
                parseExpression();
                
                if (!match(STTOK_COMMA)) break;
            }
            
            expect(STTOK_COLON, "Expected ':'");
            
            emitLine(") {");
            
            indent_level++;
            // Parse statements until next case value or END_CASE/ELSE
            while (!check(STTOK_END_CASE) && !check(STTOK_ELSE) && 
                   !check(STTOK_EOF) && !has_error) {
                // Peek ahead to see if this is a case value
                if (check(STTOK_INTEGER) || check(STTOK_IDENTIFIER)) {
                    STToken& next = peekNextToken();
                    if (next.type == STTOK_COLON || next.type == STTOK_COMMA) {
                        break; // This is a new case
                    }
                }
                parseStatement();
            }
            indent_level--;
        }
        
        if (check(STTOK_ELSE)) {
            nextToken();
            emitIndent();
            emitLine("} else {");
            indent_level++;
            parseStatements();
            indent_level--;
        }
        
        expect(STTOK_END_CASE, "Expected END_CASE");
        match(STTOK_SEMICOLON);
        
        emitIndent();
        emitLine("}");
    }
    
    void parseAssignmentOrCall() {
        // identifier := expression;
        // identifier.property := expression;
        // identifier(params);
        
        char target[128];
        int targetLen = 0;
        
        // Get the target (could be identifier or address)
        if (check(STTOK_ADDRESS)) {
            const char* converted = iecAddressToPLCScript(currentToken.text);
            while (converted[targetLen] && targetLen < 127) {
                target[targetLen] = converted[targetLen];
                targetLen++;
            }
            nextToken();
        } else {
            while (currentToken.text[targetLen] && targetLen < 127) {
                target[targetLen] = currentToken.text[targetLen];
                targetLen++;
            }
            nextToken();
        }
        target[targetLen] = '\0';
        
        // Check for property access
        while (check(STTOK_DOT)) {
            nextToken();
            if (!check(STTOK_IDENTIFIER)) {
                setError("Expected property name");
                return;
            }
            target[targetLen++] = '.';
            int i = 0;
            while (currentToken.text[i] && targetLen < 127) {
                target[targetLen++] = currentToken.text[i++];
            }
            target[targetLen] = '\0';
            nextToken();
        }
        
        // Check for function call (timer/counter invocation)
        if (check(STTOK_LPAREN)) {
            // Function block call: myTimer(IN := condition, PT := T#1s)
            nextToken();
            
            emitIndent();
            // Detect if this is a timer/counter based on type or known names
            // For now, emit as a generic function call
            emit(target);
            emit("(");
            
            bool first = true;
            while (!check(STTOK_RPAREN) && !check(STTOK_EOF) && !has_error) {
                if (!first) emit(", ");
                first = false;
                
                // Named parameter: name := value
                if (check(STTOK_IDENTIFIER)) {
                    // Skip parameter name for PLCScript
                    nextToken();
                    expect(STTOK_ASSIGN, "Expected ':='");
                }
                parseExpression();
                
                if (!match(STTOK_COMMA)) break;
            }
            
            expect(STTOK_RPAREN, "Expected ')'");
            emitLine(");");
        } else {
            // Assignment
            expect(STTOK_ASSIGN, "Expected ':='");
            
            emitIndent();
            emit(target);
            emit(" = ");
            parseExpression();
            emitLine(";");
        }
        
        expect(STTOK_SEMICOLON, "Expected ';'");
    }
    
    // ========================================================================
    // Parsing - Expressions
    // ========================================================================
    
    void parseExpression() {
        parseOrExpression();
    }
    
    void parseOrExpression() {
        parseXorExpression();
        
        while (check(STTOK_OR) || check(STTOK_PIPE)) {
            nextToken();
            emit(" || ");
            parseXorExpression();
        }
    }
    
    void parseXorExpression() {
        parseAndExpression();
        
        while (check(STTOK_XOR) || check(STTOK_CARET)) {
            nextToken();
            emit(" ^ ");
            parseAndExpression();
        }
    }
    
    void parseAndExpression() {
        parseComparisonExpression();
        
        while (check(STTOK_AND) || check(STTOK_AMPERSAND)) {
            nextToken();
            emit(" && ");
            parseComparisonExpression();
        }
    }
    
    void parseComparisonExpression() {
        parseShiftExpression();
        
        while (true) {
            if (check(STTOK_EQ)) {
                nextToken();
                emit(" == ");
                parseShiftExpression();
            } else if (check(STTOK_NEQ)) {
                nextToken();
                emit(" != ");
                parseShiftExpression();
            } else if (check(STTOK_LT)) {
                nextToken();
                emit(" < ");
                parseShiftExpression();
            } else if (check(STTOK_GT)) {
                nextToken();
                emit(" > ");
                parseShiftExpression();
            } else if (check(STTOK_LTE)) {
                nextToken();
                emit(" <= ");
                parseShiftExpression();
            } else if (check(STTOK_GTE)) {
                nextToken();
                emit(" >= ");
                parseShiftExpression();
            } else {
                break;
            }
        }
    }
    
    void parseShiftExpression() {
        parseAdditiveExpression();
        
        while (check(STTOK_SHL) || check(STTOK_SHR)) {
            bool isLeft = check(STTOK_SHL);
            nextToken();
            emit(isLeft ? " << " : " >> ");
            parseAdditiveExpression();
        }
    }
    
    void parseAdditiveExpression() {
        parseMultiplicativeExpression();
        
        while (check(STTOK_PLUS) || check(STTOK_MINUS)) {
            bool isPlus = check(STTOK_PLUS);
            nextToken();
            emit(isPlus ? " + " : " - ");
            parseMultiplicativeExpression();
        }
    }
    
    void parseMultiplicativeExpression() {
        parseUnaryExpression();
        
        while (check(STTOK_STAR) || check(STTOK_SLASH) || check(STTOK_MOD)) {
            if (check(STTOK_STAR)) {
                nextToken();
                emit(" * ");
            } else if (check(STTOK_SLASH)) {
                nextToken();
                emit(" / ");
            } else {
                nextToken();
                emit(" % ");
            }
            parseUnaryExpression();
        }
    }
    
    void parseUnaryExpression() {
        if (check(STTOK_NOT)) {
            nextToken();
            emit("!");
            parseUnaryExpression();
        } else if (check(STTOK_MINUS)) {
            nextToken();
            emit("-");
            parseUnaryExpression();
        } else {
            parsePrimaryExpression();
        }
    }
    
    void parsePrimaryExpression() {
        if (check(STTOK_INTEGER)) {
            emitInt(currentToken.intValue);
            nextToken();
        } else if (check(STTOK_FLOAT)) {
            emitFloat(currentToken.floatValue);
            nextToken();
        } else if (check(STTOK_BOOL_TRUE)) {
            emit("true");
            nextToken();
        } else if (check(STTOK_BOOL_FALSE)) {
            emit("false");
            nextToken();
        } else if (check(STTOK_LPAREN)) {
            nextToken();
            emit("(");
            parseExpression();
            emit(")");
            expect(STTOK_RPAREN, "Expected ')'");
        } else if (check(STTOK_ADDRESS)) {
            emit(iecAddressToPLCScript(currentToken.text));
            nextToken();
            
            // Property access
            while (check(STTOK_DOT)) {
                emitChar('.');
                nextToken();
                if (check(STTOK_IDENTIFIER)) {
                    emit(currentToken.text);
                    nextToken();
                }
            }
        } else if (check(STTOK_IDENTIFIER)) {
            emit(currentToken.text);
            nextToken();
            
            // Property access
            while (check(STTOK_DOT)) {
                emitChar('.');
                nextToken();
                if (check(STTOK_IDENTIFIER)) {
                    emit(currentToken.text);
                    nextToken();
                }
            }
            
            // Function call
            if (check(STTOK_LPAREN)) {
                nextToken();
                emit("(");
                
                bool first = true;
                while (!check(STTOK_RPAREN) && !check(STTOK_EOF) && !has_error) {
                    if (!first) emit(", ");
                    first = false;
                    
                    // Handle named parameters: skip name :=
                    if (check(STTOK_IDENTIFIER)) {
                        STToken& next = peekNextToken();
                        if (next.type == STTOK_ASSIGN) {
                            nextToken(); // skip name
                            nextToken(); // skip :=
                        }
                    }
                    parseExpression();
                    
                    if (!match(STTOK_COMMA)) break;
                }
                
                expect(STTOK_RPAREN, "Expected ')'");
                emit(")");
            }
        } else {
            setError("Expected expression");
        }
    }
    
    // ========================================================================
    // Main compilation entry point
    // ========================================================================
    
    bool compile() {
        if (!source || source_length == 0) {
            setError("No source code");
            return false;
        }
        
        output_len = 0;
        output[0] = '\0';
        symbol_count = 0;
        indent_level = 0;
        
        // Emit header comment
        emitLine("// Generated by ST to PLCScript Compiler");
        
        nextToken();
        
        // Parse VAR blocks first
        while (check(STTOK_VAR) || check(STTOK_VAR_INPUT) || check(STTOK_VAR_OUTPUT) ||
               check(STTOK_VAR_IN_OUT) || check(STTOK_VAR_GLOBAL) || check(STTOK_VAR_TEMP)) {
            if (!parseVarBlock()) {
                return false;
            }
        }
        
        // Parse statements
        parseStatements();
        
        return !has_error;
    }
};

// ============================================================================
// WASM Exports
// ============================================================================

// Use getter function pattern (like PLCScript) for stable instance across WASM calls
static SharedSymbolTable& getSTSharedSymbols() {
    static SharedSymbolTable instance;
    return instance;
}

static STCompiler& getSTCompiler() {
    static STCompiler instance(getSTSharedSymbols());
    return instance;
}

#define __st_compiler__ getSTCompiler()

// Static buffers for input/output that persist across WASM calls
static char st_input_buffer[65536];
static int st_input_length = 0;

// Static buffers to cache results after compile (since class state may not persist)
static char st_output_buffer[65536];
static int st_output_length = 0;
static char st_error_buffer[256];
static char st_error_token_buffer[64];
static int st_error_token_type = 0;
static bool st_has_error = false;
static int st_error_line = 0;
static int st_error_column = 0;

extern "C" {

WASM_EXPORT void st_compiler_load_from_stream() {
    streamRead(st_input_buffer, st_input_length, sizeof(st_input_buffer));
    // Also set source immediately (like PLCScript does)
    __st_compiler__.reset();
    __st_compiler__.setSource(st_input_buffer, st_input_length);
}

WASM_EXPORT int st_compiler_get_input_length() {
    return st_input_length;
}

WASM_EXPORT int st_compiler_compile() {
    // Compile and cache all results to static buffers
    bool success = __st_compiler__.compile();
    
    // Cache output
    st_output_length = __st_compiler__.output_len;
    for (int i = 0; i < st_output_length && i < 65535; i++) {
        st_output_buffer[i] = __st_compiler__.output[i];
    }
    st_output_buffer[st_output_length] = '\0';
    
    // Cache error info
    st_has_error = __st_compiler__.has_error;
    st_error_line = __st_compiler__.error_line;
    st_error_column = __st_compiler__.error_column;
    st_error_token_type = __st_compiler__.error_token_type;
    
    // Copy error token text
    const char* tokText = __st_compiler__.error_token_text;
    int j = 0;
    while (tokText[j] && j < 63) {
        st_error_token_buffer[j] = tokText[j];
        j++;
    }
    st_error_token_buffer[j] = '\0';
    
    // Copy error message
    const char* err = __st_compiler__.error_message;
    int i = 0;
    while (err[i] && i < 255) {
        st_error_buffer[i] = err[i];
        i++;
    }
    st_error_buffer[i] = '\0';
    
    return success ? 0 : 1;
}

WASM_EXPORT int st_compiler_get_output_length() {
    return st_output_length;
}

WASM_EXPORT char* st_compiler_get_output() {
    return st_output_buffer;
}

WASM_EXPORT void st_compiler_output_to_stream() {
    for (int i = 0; i < st_output_length; i++) {
        streamOut(st_output_buffer[i]);
    }
}

WASM_EXPORT int st_compiler_has_error() {
    return st_has_error ? 1 : 0;
}

WASM_EXPORT char* st_compiler_get_error() {
    return st_error_buffer;
}

WASM_EXPORT int st_compiler_get_error_line() {
    return st_error_line;
}

WASM_EXPORT int st_compiler_get_error_column() {
    return st_error_column;
}

WASM_EXPORT char* st_compiler_get_error_token_text() {
    return st_error_token_buffer;
}

WASM_EXPORT int st_compiler_get_error_token_type() {
    return st_error_token_type;
}

WASM_EXPORT void st_compiler_error_to_stream() {
    const char* msg = st_error_buffer;
    while (*msg) {
        streamOut(*msg++);
    }
}

// Debug exports (use cached values)
WASM_EXPORT const char* st_compiler_debug_get_source() {
    return st_input_buffer;
}

WASM_EXPORT int st_compiler_debug_get_source_length() {
    return st_input_length;
}

WASM_EXPORT int st_compiler_debug_get_token_type() {
    return (int)__st_compiler__.currentToken.type;
}

WASM_EXPORT int st_compiler_debug_get_eof_value() {
    return (int)STTOK_EOF;
}

WASM_EXPORT int st_compiler_debug_get_end_var_value() {
    return (int)STTOK_END_VAR;
}

WASM_EXPORT const char* st_compiler_debug_get_token_text() {
    return __st_compiler__.currentToken.text;
}

WASM_EXPORT const char* st_compiler_debug_get_input_preview() {
    static char preview[101];
    int len = st_input_length > 100 ? 100 : st_input_length;
    for (int i = 0; i < len; i++) {
        preview[i] = st_input_buffer[i];
    }
    preview[len] = '\0';
    return preview;
}

} // extern "C"

#endif // __WASM__
