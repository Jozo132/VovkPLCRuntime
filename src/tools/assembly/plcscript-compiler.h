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
#include "shared-symbols.h"

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
// Do-while loop:
//   do { statements } while (condition)
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
#define PLCSCRIPT_MAX_FUNCTIONS 64
#define PLCSCRIPT_MAX_FUNC_PARAMS 16

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
    PSTOK_TEMPLATE,     // `hello ${expr}` - JS-style template literal
    
    // Identifiers and keywords
    PSTOK_IDENTIFIER,   // variable/function names
    PSTOK_LET,          // let
    PSTOK_CONST,        // const
    PSTOK_IF,           // if
    PSTOK_ELSE,         // else
    PSTOK_WHILE,        // while
    PSTOK_DO,           // do
    PSTOK_FOR,          // for
    PSTOK_FUNCTION,     // function
    PSTOK_RETURN,       // return
    PSTOK_BREAK,        // break
    PSTOK_CONTINUE,     // continue
    PSTOK_TYPE,         // type (for struct definitions)
    PSTOK_STRUCT,       // struct
    
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
    PSTOK_TYPE_STR8,
    PSTOK_TYPE_STR16,
    
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
// String Support (str8 / str16)
// ============================================================================
//
// PLCScript supports two string types with JS-like syntax:
//   - str8:  Max 254 chars, 2-byte header (u8 capacity, u8 length) + chars
//   - str16: Max 65534 chars, 4-byte header (u16 capacity, u16 length) + chars
//
// Declaration:
//   let msg: str8[32] @ M0;      // 32-char capacity at memory address M0
//   let buf: str16[256] @ auto;  // 256-char capacity, auto-allocated
//   let greeting: str8[100];     // Capacity 100, auto memory
//
// Literals (with JS escape sequences):
//   msg = "Hello, World!";
//   buf = "Line1\nLine2\tTabbed";
//   // Supported escapes: \n \t \r \0 \\ \" \'
//
// Operators:
//   msg += " appended";         // Concatenate to msg
//   let result: str8[64] @ auto = msg + buf;  // Concatenate two strings
//   if (msg == buf) { ... }      // String equality comparison
//   if (msg != buf) { ... }      // String inequality comparison
//
// Properties:
//   let len: u16 = msg.length;   // Current string length (characters used)
//   let cap: u16 = msg.capacity; // Maximum capacity (characters available)
//
// Methods:
//   msg.clear();                 // Reset string to empty (length = 0)
//   let pos: i16 = msg.indexOf("World");   // Find substring, -1 if not found
//   let found: bool = msg.includes("ello"); // Check if substring exists
//   let ch: u8 = msg.charAt(0);  // Get character at index (0-based)
//   msg.concat(buf);             // Append another string
//
// Memory Layout Example (str8[32]):
//   Byte 0:    capacity (32)
//   Byte 1:    length (current string length)
//   Bytes 2-33: character data (capacity bytes)
//   Total size: 2 + 32 = 34 bytes
//
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
    PSTYPE_STR8,     // String8 type: [u8 capacity, u8 length, char[capacity]]
    PSTYPE_STR16,    // String16 type: [u16 capacity, u16 length, char[capacity]]
    PSTYPE_STRUCT,   // Custom struct type (uses structTypeIndex for lookup)
};

// ============================================================================
// Struct Field Definition
// ============================================================================

#define PLCSCRIPT_MAX_STRUCT_FIELDS 32
#define PLCSCRIPT_MAX_STRUCT_TYPES 64

struct PLCScriptStructField {
    char name[PLCSCRIPT_MAX_IDENTIFIER_LEN];
    PLCScriptVarType type;
    int structTypeIndex;  // For nested struct types (-1 if primitive)
    u32 offset;           // Byte offset within the struct
    u32 size;             // Size in bytes
    bool isBit;           // True if this is a bit field
    u8 bitIndex;          // Bit position (0-7) if isBit
    bool used;

    void reset() {
        name[0] = '\0';
        type = PSTYPE_VOID;
        structTypeIndex = -1;
        offset = 0;
        size = 0;
        isBit = false;
        bitIndex = 0;
        used = false;
    }
};

// ============================================================================
// Struct Type Definition
// ============================================================================

struct PLCScriptStructType {
    char name[PLCSCRIPT_MAX_IDENTIFIER_LEN];
    PLCScriptStructField fields[PLCSCRIPT_MAX_STRUCT_FIELDS];
    int fieldCount;
    u32 totalSize;        // Total size in bytes
    bool used;

    void reset() {
        name[0] = '\0';
        fieldCount = 0;
        totalSize = 0;
        used = false;
        for (int i = 0; i < PLCSCRIPT_MAX_STRUCT_FIELDS; i++) {
            fields[i].reset();
        }
    }

    // Find a field by name
    PLCScriptStructField* findField(const char* fieldName) {
        for (int i = 0; i < fieldCount; i++) {
            bool match = true;
            const char* a = fields[i].name;
            const char* b = fieldName;
            while (*a && *b) {
                if (*a != *b) { match = false; break; }
                a++; b++;
            }
            if (match && *a == *b) return &fields[i];
        }
        return nullptr;
    }
};

// ============================================================================
// Symbol Table Entry
// ============================================================================

struct PLCScriptSymbol {
    char name[PLCSCRIPT_MAX_IDENTIFIER_LEN];
    PLCScriptVarType type;
    int structTypeIndex;  // Index into structTypes array (-1 if primitive type)
    bool isConst;
    bool isBit;           // True if this is a bit address
    bool isLocal;         // True if this is a local stack variable (no PLC address)
    bool isAutoAddress;   // True if address was auto-assigned
    bool isParam;         // True if this is a function parameter
    
    // Array support
    MY_PTR_t array_size;       // Number of elements (0 = not an array, 1+ = array)
    
    // String support (for PSTYPE_STR8 and PSTYPE_STR16)
    u16 stringCapacity;        // Maximum string length (0 = not a string)
    
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
    
    // Check if this is an array type
    bool isArray() const { return array_size > 0; }
    
    // Check if this is a string type
    bool isStringType() const { return type == PSTYPE_STR8 || type == PSTYPE_STR16; }
    
    // Get total size in bytes for string (header + capacity)
    u32 stringSizeInBytes() const {
        if (type == PSTYPE_STR8) return 2 + stringCapacity;   // u8 cap + u8 len + chars
        if (type == PSTYPE_STR16) return 4 + stringCapacity;  // u16 cap + u16 len + chars
        return 0;
    }
    
    // Get element size in bytes - requires getTypeSize from compiler
    // Note: This returns 0 for struct types (needs compiler context)
    u8 elementSize() const {
        if (structTypeIndex >= 0) return 0; // Struct size determined at runtime
        if (isStringType()) return 0;       // String size determined by stringCapacity
        // Simple type size lookup (doesn't require compiler context)
        switch(type) {
            case PSTYPE_BOOL: return 1;
            case PSTYPE_U8: case PSTYPE_I8: return 1;
            case PSTYPE_U16: case PSTYPE_I16: return 2;
            case PSTYPE_U32: case PSTYPE_I32: case PSTYPE_F32: return 4;
            case PSTYPE_U64: case PSTYPE_I64: case PSTYPE_F64: return 8;
            default: return 4;
        }
    }
    
    // Get total size in bytes (element size * count for arrays, string size for strings)
    u32 totalSize() const {
        if (isStringType()) return stringSizeInBytes();
        u8 elem_size = elementSize();
        if (array_size == 0) return elem_size;
        return (u32)elem_size * (u32)array_size;
    }
};

// ============================================================================
// Function Parameter
// ============================================================================

struct PLCScriptFuncParam {
    char name[PLCSCRIPT_MAX_IDENTIFIER_LEN];
    PLCScriptVarType type;
    int structTypeIndex;  // For struct type parameters (-1 if primitive)
};

// ============================================================================
// Function Declaration
// ============================================================================

struct PLCScriptFunction {
    char name[PLCSCRIPT_MAX_IDENTIFIER_LEN];
    PLCScriptVarType returnType;
    
    // Parameters
    PLCScriptFuncParam params[PLCSCRIPT_MAX_FUNC_PARAMS];
    int paramCount;
    
    // Source location
    int declarationLine;
    int declarationColumn;
    int bodyStartPos;     // Position in source where body starts (for second pass)
    int bodyEndPos;       // Position where body ends
    int bodyStartLine;    // Line number at body start
    int bodyStartColumn;  // Column number at body start
    
    // Label for the function entry point
    char entryLabel[64];
    
    // Whether the function has been compiled
    bool isCompiled;
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
    
    // String literal pending for assignment (e.g., in msg = "hello")
    char pendingStringLiteral[PLCSCRIPT_MAX_IDENTIFIER_LEN];
    int pendingStringLiteralLen = 0;
    bool hasPendingStringLiteral = false;
    
    // Template literal pending for assignment (e.g., in msg = `Hello ${name}`)
    char pendingTemplateLiteral[PLCSCRIPT_MAX_IDENTIFIER_LEN];
    int pendingTemplateLiteralLen = 0;
    bool hasPendingTemplateLiteral = false;
    
    // String variable pending for type casting (e.g., in i32(strVar))
    bool hasPendingStringVar = false;
    PLCScriptVarType pendingStringVarType = PSTYPE_VOID;
    char pendingStringVarAddr[32];
    
    // String expression context - tracks last referenced string variable
    PLCScriptSymbol* lastStringSymbol = nullptr;
    
    // Substr context - tracks when a .substr() call was made for assignment
    PLCScriptSymbol* lastSubstrSymbol = nullptr;  // Source string for substr
    bool hasSubstrArgs = false;  // True if from/to args are on stack
    
    // Number-to-string context - tracks when .toString() or .toFixed() was called
    bool hasToStringCall = false;  // True if number is on stack for string conversion
    PLCScriptVarType toStringNumType = PSTYPE_VOID;  // Type of number on stack
    u8 toStringBase = 10;  // Base for integer conversion
    u8 toStringDecimals = 2;  // Decimal places for float conversion
    
    // Function table (for user-defined functions)
    PLCScriptFunction functions[PLCSCRIPT_MAX_FUNCTIONS];
    int functionCount = 0;
    
    // Struct type table (for custom datatypes)
    PLCScriptStructType structTypes[PLCSCRIPT_MAX_STRUCT_TYPES];
    int structTypeCount = 0;
    
    // Compilation pass (1 = scan for functions, 2 = generate code)
    int compilationPass = 1;
    
    // Current function being compiled (nullptr if in global scope)
    PLCScriptFunction* currentFunction = nullptr;
    
    // Tracking for unreachable code detection
    bool lastStatementWasTerminator = false;
    
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
        // Reset string context
        pendingStringLiteral[0] = '\0';
        pendingStringLiteralLen = 0;
        hasPendingStringLiteral = false;
        // Reset template literal context
        pendingTemplateLiteral[0] = '\0';
        pendingTemplateLiteralLen = 0;
        hasPendingTemplateLiteral = false;
        // Reset other string context
        hasPendingStringVar = false;
        pendingStringVarType = PSTYPE_VOID;
        pendingStringVarAddr[0] = '\0';
        lastStringSymbol = nullptr;
        lastSubstrSymbol = nullptr;
        hasSubstrArgs = false;
        hasToStringCall = false;
        toStringNumType = PSTYPE_VOID;
        toStringBase = 10;
        toStringDecimals = 2;
        // Reset auto-address counters
        autoAddrBit = 900;
        autoAddrByte = 950;
        autoAddrWord = 960;
        autoAddrDword = 980;
        // Reset stack slot management
        nextStackSlot = 0;
        maxStackSlots = 0;
        // Reset function table
        functionCount = 0;
        compilationPass = 1;
        currentFunction = nullptr;
        // Reset struct type table
        structTypeCount = 0;
        for (int i = 0; i < PLCSCRIPT_MAX_STRUCT_TYPES; i++) {
            structTypes[i].reset();
        }
        memset(&currentToken, 0, sizeof(currentToken));
        memset(&peekToken, 0, sizeof(peekToken));
        memset(symbols, 0, sizeof(symbols));
        memset(loopStack, 0, sizeof(loopStack));
        memset(functions, 0, sizeof(functions));
        lastStatementWasTerminator = false;
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
    
    // Emit a string literal with proper escaping for PLCASM cstr.lit instruction
    // The input string has already been processed by the tokenizer (escapes decoded)
    // We need to re-encode escapes when emitting to PLCASM
    // PLCASM uses single quotes for string literals
    void emitStringLiteral(const char* str, int len) {
        emit("'");
        for (int i = 0; i < len; i++) {
            char c = str[i];
            switch (c) {
                case '\n': emit("\\n"); break;
                case '\r': emit("\\r"); break;
                case '\t': emit("\\t"); break;
                case '\0': emit("\\0"); break;
                case '\\': emit("\\\\"); break;
                case '\'': emit("\\'"); break;
                default:
                    // Emit printable ASCII directly, escape control chars
                    if (c >= 32 && c < 127) {
                        char buf[2] = { c, '\0' };
                        emit(buf);
                    } else {
                        // Non-printable: emit as octal or just pass through
                        // For simplicity, just emit the byte directly
                        char buf[2] = { c, '\0' };
                        emit(buf);
                    }
                    break;
            }
        }
        emit("'");
    }
    
    // Helper to emit a string literal without quotes (raw content for concat)
    void emitStringLiteralRaw(const char* str, int len) {
        for (int i = 0; i < len; i++) {
            char c = str[i];
            switch (c) {
                case '\n': emit("\\n"); break;
                case '\r': emit("\\r"); break;
                case '\t': emit("\\t"); break;
                case '\0': emit("\\0"); break;
                case '\\': emit("\\\\"); break;
                case '\'': emit("\\'"); break;
                default:
                    if (c >= 32 && c < 127) {
                        char buf[2] = { c, '\0' };
                        emit(buf);
                    } else {
                        char buf[2] = { c, '\0' };
                        emit(buf);
                    }
                    break;
            }
        }
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
    
    virtual void setError(const char* msg) {
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
    
    virtual void setErrorAt(const char* msg, int line, int col) {
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
    
    // Warning hook - base compiler ignores warnings, linter overrides to capture them
    virtual void addWarning(const char* /*msg*/) { }
    virtual void addWarningAt(const char* /*msg*/, int /*line*/, int /*col*/, int /*length*/) { }
    virtual void addInfo(const char* /*msg*/) { }
    virtual void addInfoAt(const char* /*msg*/, int /*line*/, int /*col*/, int /*length*/) { }
    
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
        if (strEq(text, "do")) return PSTOK_DO;
        if (strEq(text, "for")) return PSTOK_FOR;
        if (strEq(text, "function")) return PSTOK_FUNCTION;
        if (strEq(text, "return")) return PSTOK_RETURN;
        if (strEq(text, "break")) return PSTOK_BREAK;
        if (strEq(text, "continue")) return PSTOK_CONTINUE;
        if (strEq(text, "true")) return PSTOK_BOOL_TRUE;
        if (strEq(text, "false")) return PSTOK_BOOL_FALSE;
        if (strEq(text, "auto")) return PSTOK_AUTO;
        if (strEq(text, "type")) return PSTOK_TYPE;
        if (strEq(text, "struct")) return PSTOK_STRUCT;
        
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
        if (strEq(text, "str8")) return PSTOK_TYPE_STR8;
        if (strEq(text, "str16")) return PSTOK_TYPE_STR16;
        if (strEq(text, "string")) return PSTOK_TYPE_STR8;  // alias: string = str8
        
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
        
        // String literals with JS-style escape sequences
        if (c == '"' || c == '\'') {
            char quote = advance();
            while (pos < sourceLength && peek() != quote) {
                if (peek() == '\\') {
                    advance(); // consume backslash
                    if (pos >= sourceLength) {
                        setError("Unterminated escape sequence");
                        currentToken.type = PSTOK_ERROR;
                        return;
                    }
                    char escaped = advance();
                    switch (escaped) {
                        case 'n': currentToken.text[currentToken.textLen++] = '\n'; break;
                        case 't': currentToken.text[currentToken.textLen++] = '\t'; break;
                        case 'r': currentToken.text[currentToken.textLen++] = '\r'; break;
                        case '0': currentToken.text[currentToken.textLen++] = '\0'; break;
                        case '\\': currentToken.text[currentToken.textLen++] = '\\'; break;
                        case '"': currentToken.text[currentToken.textLen++] = '"'; break;
                        case '\'': currentToken.text[currentToken.textLen++] = '\''; break;
                        default: currentToken.text[currentToken.textLen++] = escaped; break;
                    }
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
        
        // Template literals with backticks: `hello ${expr} world`
        // We store the raw template string (including ${...}) for later parsing
        if (c == '`') {
            advance(); // consume opening backtick
            int braceDepth = 0;
            while (pos < sourceLength) {
                char ch = peek();
                if (ch == '`' && braceDepth == 0) {
                    break; // End of template
                }
                if (ch == '\\') {
                    advance(); // consume backslash
                    if (pos >= sourceLength) {
                        setError("Unterminated escape in template literal");
                        currentToken.type = PSTOK_ERROR;
                        return;
                    }
                    char escaped = advance();
                    switch (escaped) {
                        case 'n': currentToken.text[currentToken.textLen++] = '\n'; break;
                        case 't': currentToken.text[currentToken.textLen++] = '\t'; break;
                        case 'r': currentToken.text[currentToken.textLen++] = '\r'; break;
                        case '0': currentToken.text[currentToken.textLen++] = '\0'; break;
                        case '\\': currentToken.text[currentToken.textLen++] = '\\'; break;
                        case '`': currentToken.text[currentToken.textLen++] = '`'; break;
                        case '$': currentToken.text[currentToken.textLen++] = '$'; break;
                        default: currentToken.text[currentToken.textLen++] = escaped; break;
                    }
                } else if (ch == '$' && pos + 1 < sourceLength && source[pos + 1] == '{') {
                    // Start of interpolation - store raw ${
                    currentToken.text[currentToken.textLen++] = advance(); // $
                    currentToken.text[currentToken.textLen++] = advance(); // {
                    braceDepth++;
                } else if (ch == '{' && braceDepth > 0) {
                    currentToken.text[currentToken.textLen++] = advance();
                    braceDepth++;
                } else if (ch == '}' && braceDepth > 0) {
                    currentToken.text[currentToken.textLen++] = advance();
                    braceDepth--;
                } else {
                    currentToken.text[currentToken.textLen++] = advance();
                }
                
                if (currentToken.textLen >= PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
                    setError("Template literal too long");
                    currentToken.type = PSTOK_ERROR;
                    return;
                }
            }
            if (pos >= sourceLength) {
                setError("Unterminated template literal");
                currentToken.type = PSTOK_ERROR;
                return;
            }
            advance(); // Closing backtick
            currentToken.text[currentToken.textLen] = '\0';
            currentToken.type = PSTOK_TEMPLATE;
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
        
        // Check for shadowed variable in outer scope (warning only)
        for (int i = symbolCount - 1; i >= 0; i--) {
            if (symbols[i].scopeLevel < currentScopeLevel && strEq(symbols[i].name, name)) {
                addWarning("Variable shadows outer declaration");
                break;
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
            case PSTOK_TYPE_STR8: return PSTYPE_STR8;
            case PSTOK_TYPE_STR16: return PSTYPE_STR16;
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
            case PSTYPE_STR8: return "str8";
            case PSTYPE_STR16: return "str16";
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
        return type >= PSTOK_TYPE_BOOL && type <= PSTOK_TYPE_STR16;
    }
    
    bool isStringType(PLCScriptVarType type) {
        return type == PSTYPE_STR8 || type == PSTYPE_STR16;
    }
    
    bool isNumericType(PLCScriptVarType type) {
        return type >= PSTYPE_BOOL && type <= PSTYPE_F64 && !isStringType(type);
    }
    
    bool isFloatType(PLCScriptVarType type) {
        return type == PSTYPE_F32 || type == PSTYPE_F64;
    }
    
    // ========================================================================
    // Struct Type helpers
    // ========================================================================
    
    // Find a struct type by name
    PLCScriptStructType* findStructType(const char* name) {
        for (int i = 0; i < structTypeCount; i++) {
            if (strEq(structTypes[i].name, name)) {
                return &structTypes[i];
            }
        }
        return nullptr;
    }
    
    // Find struct type index by name (-1 if not found)
    // Also checks global UserStructType registry and imports if found
    int findStructTypeIndex(const char* name) {
        // First check local struct types
        for (int i = 0; i < structTypeCount; i++) {
            if (strEq(structTypes[i].name, name)) {
                return i;
            }
        }
        
        // Check global UserStructType registry (types from TYPES section)
        UserStructType* globalType = findUserStructType(name);
        if (globalType) {
            // Import into local struct types array
            if (structTypeCount >= PLCSCRIPT_MAX_STRUCT_TYPES) {
                return -1; // Can't import, array full
            }
            
            PLCScriptStructType* localType = &structTypes[structTypeCount];
            localType->reset();
            
            // Copy name
            int ni = 0;
            while (globalType->name[ni] && ni < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
                localType->name[ni] = globalType->name[ni];
                ni++;
            }
            localType->name[ni] = '\0';
            
            // Copy fields
            for (int f = 0; f < globalType->field_count && f < PLCSCRIPT_MAX_STRUCT_FIELDS; f++) {
                const StructProperty& srcField = globalType->fields[f];
                PLCScriptStructField& dstField = localType->fields[f];
                
                // Copy field name
                int fi = 0;
                while (srcField.name[fi] && fi < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
                    dstField.name[fi] = srcField.name[fi];
                    fi++;
                }
                dstField.name[fi] = '\0';
                
                dstField.offset = srcField.offset;
                dstField.size = srcField.type_size > 0 ? srcField.type_size : 1;
                dstField.used = true;
                dstField.isBit = (srcField.bit_pos != 255);
                dstField.bitIndex = srcField.bit_pos;
                
                // Determine PLCScript type from size
                if (dstField.isBit || srcField.type_size == 0) {
                    dstField.type = PSTYPE_BOOL;
                } else if (srcField.type_size == 1) {
                    dstField.type = PSTYPE_I8;
                } else if (srcField.type_size == 2) {
                    dstField.type = PSTYPE_I16;
                } else if (srcField.type_size == 4) {
                    dstField.type = PSTYPE_I32;
                } else if (srcField.type_size == 8) {
                    dstField.type = PSTYPE_I64;
                } else {
                    dstField.type = PSTYPE_I32; // Default
                }
                
                localType->fieldCount++;
            }
            
            localType->totalSize = globalType->total_size;
            
            return structTypeCount++;
        }
        
        return -1;
    }
    
    // Find struct type index by exact name match (case-sensitive, no import)
    // Used for checking if a variable name conflicts with a type name
    int findStructTypeIndexExact(const char* name) {
        for (int i = 0; i < structTypeCount; i++) {
            if (strEq(structTypes[i].name, name)) {
                return i;
            }
        }
        return -1;
    }
    
    // Add a new struct type
    PLCScriptStructType* addStructType(const char* name) {
        if (structTypeCount >= PLCSCRIPT_MAX_STRUCT_TYPES) {
            setError("Too many struct types");
            return nullptr;
        }
        
        // Check for duplicate
        if (findStructType(name) != nullptr) {
            setError("Duplicate struct type definition");
            return nullptr;
        }
        
        PLCScriptStructType* st = &structTypes[structTypeCount++];
        st->reset();
        int i = 0;
        while (name[i] && i < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
            st->name[i] = name[i];
            i++;
        }
        st->name[i] = '\0';
        st->used = true;
        return st;
    }
    
    // Get the size of a type (including struct types)
    int getTypeSize(PLCScriptVarType type, int structTypeIndex) {
        if (type == PSTYPE_STRUCT && structTypeIndex >= 0 && structTypeIndex < structTypeCount) {
            return structTypes[structTypeIndex].totalSize;
        }
        return varTypeSize(type);
    }
    
    // Check if token is a type (primitive or struct)
    bool isType() {
        if (isTypeKeyword(currentToken.type)) return true;
        if (currentToken.type == PSTOK_IDENTIFIER) {
            // Check if it's a struct type name
            return findStructType(currentToken.text) != nullptr;
        }
        return false;
    }
    
    // Parse a type and return the var type and struct index
    bool parseType(PLCScriptVarType& outType, int& outStructIndex) {
        outStructIndex = -1;
        
        if (isTypeKeyword(currentToken.type)) {
            outType = tokenTypeToVarType(currentToken.type);
            nextToken();
            return true;
        }
        
        if (currentToken.type == PSTOK_IDENTIFIER) {
            int idx = findStructTypeIndex(currentToken.text);
            if (idx >= 0) {
                outType = PSTYPE_STRUCT;
                outStructIndex = idx;
                nextToken();
                return true;
            }
        }
        
        setError("Expected type name");
        return false;
    }
    
    // ========================================================================
    // Address parsing
    // ========================================================================
    
    bool parseAddress(PLCScriptSymbol* sym, const char* addr) {
        // First, check if this is a symbol name from the shared symbol table
        SharedSymbol* sharedSym = sharedSymbols.findSymbol(addr);
        if (sharedSym) {
            // Found a symbol - use its resolved address
            sym->memoryOffset = sharedSym->address;
            sym->isBit = sharedSym->is_bit;
            sym->bitIndex = sharedSym->is_bit ? sharedSym->bit : 0;
            
            // Build PLCASM address string from the shared symbol's address
            // Determine area prefix from address range
            char areaPrefix = 'M'; // Default
            u32 relativeAddr = sharedSym->address;
            
            if (sharedSym->address >= plcasm_input_offset && sharedSym->address < plcasm_output_offset) {
                areaPrefix = 'X';
                relativeAddr = sharedSym->address - plcasm_input_offset;
            } else if (sharedSym->address >= plcasm_output_offset && sharedSym->address < plcasm_marker_offset) {
                areaPrefix = 'Y';
                relativeAddr = sharedSym->address - plcasm_output_offset;
            } else if (sharedSym->address >= plcasm_marker_offset && sharedSym->address < plcasm_timer_offset) {
                areaPrefix = 'M';
                relativeAddr = sharedSym->address - plcasm_marker_offset;
            } else if (sharedSym->address >= plcasm_timer_offset && sharedSym->address < plcasm_counter_offset) {
                areaPrefix = 'T';
                relativeAddr = sharedSym->address - plcasm_timer_offset;
            } else if (sharedSym->address >= plcasm_counter_offset) {
                areaPrefix = 'C';
                relativeAddr = sharedSym->address - plcasm_counter_offset;
            }
            
            int outIdx = 0;
            sym->address[outIdx++] = areaPrefix;
            
            // Write byte number
            char numBuf[16];
            int numLen = 0;
            u32 n = relativeAddr;
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
            
            // Add bit position if this is a bit symbol
            if (sharedSym->is_bit) {
                sym->address[outIdx++] = '.';
                sym->address[outIdx++] = '0' + sharedSym->bit;
            }
            
            sym->address[outIdx] = '\0';
            return true;
        }
        
        // Not a symbol - parse as raw address
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
            case 'C': baseOffset = plcasm_counter_offset; break;
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

    // Helper to convert a SharedSymbol to a PLCScriptSymbol (for using project symbols in expressions)
    PLCScriptVarType sharedSymbolToPLCScriptType(const SharedSymbol* shared) {
        // Map SharedSymbol type names to PLCScript types
        // SharedSymbol types: bit, i8, u8, i16, u16, i32, u32, i64, u64, f32, f64
        const char* t = shared->type;
        if (strEqCI(t, "bit") || strEqCI(t, "bool")) return PSTYPE_BOOL;
        if (strEqCI(t, "i8")) return PSTYPE_I8;
        if (strEqCI(t, "u8") || strEqCI(t, "byte")) return PSTYPE_U8;
        if (strEqCI(t, "i16") || strEqCI(t, "int")) return PSTYPE_I16;
        if (strEqCI(t, "u16") || strEqCI(t, "uint") || strEqCI(t, "word")) return PSTYPE_U16;
        if (strEqCI(t, "i32") || strEqCI(t, "dint")) return PSTYPE_I32;
        if (strEqCI(t, "u32") || strEqCI(t, "udint") || strEqCI(t, "dword")) return PSTYPE_U32;
        if (strEqCI(t, "i64") || strEqCI(t, "lint")) return PSTYPE_I64;
        if (strEqCI(t, "u64") || strEqCI(t, "ulint") || strEqCI(t, "lword")) return PSTYPE_U64;
        if (strEqCI(t, "f32") || strEqCI(t, "real") || strEqCI(t, "float")) return PSTYPE_F32;
        if (strEqCI(t, "f64") || strEqCI(t, "lreal") || strEqCI(t, "double")) return PSTYPE_F64;
        return PSTYPE_I16; // Default to i16
    }

    bool populateFromSharedSymbol(PLCScriptSymbol* sym, const SharedSymbol* shared) {
        // Copy name
        int i = 0;
        while (shared->name[i] && i < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
            sym->name[i] = shared->name[i];
            i++;
        }
        sym->name[i] = '\0';

        // Set type
        sym->type = sharedSymbolToPLCScriptType(shared);

        // Set address info
        sym->memoryOffset = shared->address;
        sym->isBit = shared->is_bit;
        sym->bitIndex = shared->is_bit ? shared->bit : 0;

        // Build PLCASM address string
        // Determine area prefix from address range (S - System removed)
        char areaPrefix = 'M'; // Default
        u32 relativeAddr = shared->address;

        if (shared->address >= plcasm_input_offset && shared->address < plcasm_output_offset) {
            areaPrefix = 'X';
            relativeAddr = shared->address - plcasm_input_offset;
        } else if (shared->address >= plcasm_output_offset && shared->address < plcasm_marker_offset) {
            areaPrefix = 'Y';
            relativeAddr = shared->address - plcasm_output_offset;
        } else if (shared->address >= plcasm_marker_offset && shared->address < plcasm_timer_offset) {
            areaPrefix = 'M';
            relativeAddr = shared->address - plcasm_marker_offset;
        } else if (shared->address >= plcasm_timer_offset && shared->address < plcasm_counter_offset) {
            areaPrefix = 'T';
            relativeAddr = shared->address - plcasm_timer_offset;
        } else if (shared->address >= plcasm_counter_offset) {
            areaPrefix = 'C';
            relativeAddr = shared->address - plcasm_counter_offset;
        }

        int outIdx = 0;
        sym->address[outIdx++] = areaPrefix;

        // Write byte number
        char numBuf[16];
        int numLen = 0;
        u32 n = relativeAddr;
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

        // Add bit position if this is a bit symbol
        if (shared->is_bit) {
            sym->address[outIdx++] = '.';
            sym->address[outIdx++] = '0' + shared->bit;
        }

        sym->address[outIdx] = '\0';
        
        sym->isConst = false;
        sym->isLocal = false;
        sym->isAutoAddress = false;
        sym->isParam = false;
        sym->stackSlot = -1;
        sym->hasInitializer = false;
        sym->array_size = shared->array_size;
        sym->structTypeIndex = -1;
        
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
        // String types don't push values to stack, so nothing to drop
        if (type == PSTYPE_STR8 || type == PSTYPE_STR16) return;
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
        
        // Calculate how many elements to reserve (1 for scalar, array_size for arrays)
        MY_PTR_t count = sym->array_size > 0 ? sym->array_size : 1;
        
        if (sym->type == PSTYPE_BOOL) {
            // Bit address: M<byte>.<bit> - no conversion needed for PLCASM
            // Arrays of bits not supported (rejected earlier)
            int byteNum = autoAddrBit / 8;
            int bitNum = autoAddrBit % 8;
            // Set memoryOffset to absolute byte address for array element calculations
            sym->memoryOffset = plcasm_marker_offset + byteNum;
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
            // Set memoryOffset to absolute byte address for array element calculations
            sym->memoryOffset = plcasm_marker_offset + autoAddrByte;
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
            autoAddrByte += count;  // Reserve space for all elements
            // Copy directly - already in PLCASM format
            int j = 0;
            while (addr[j] && j < 31) {
                sym->address[j] = addr[j];
                j++;
            }
            sym->address[j] = '\0';
        } else if (sym->type == PSTYPE_I16 || sym->type == PSTYPE_U16) {
            // Word address: M<n> (PLCASM format, not MW<n>)
            // Set memoryOffset to absolute byte address for array element calculations
            sym->memoryOffset = plcasm_marker_offset + autoAddrWord;
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
            autoAddrWord += 2 * count;  // Words are 2 bytes apart, reserve for all elements
            // Copy directly - already in PLCASM format
            int j = 0;
            while (addr[j] && j < 31) {
                sym->address[j] = addr[j];
                j++;
            }
            sym->address[j] = '\0';
        } else if (sym->type == PSTYPE_STR8 || sym->type == PSTYPE_STR16) {
            // String address: M<n> - use dword alignment for strings
            // String size = header + capacity
            u32 strSize = sym->stringSizeInBytes();
            sym->memoryOffset = plcasm_marker_offset + autoAddrDword;
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
            autoAddrDword += strSize;  // Reserve space for string header + capacity
            // Copy directly - already in PLCASM format
            int j = 0;
            while (addr[j] && j < 31) {
                sym->address[j] = addr[j];
                j++;
            }
            sym->address[j] = '\0';
        } else {
            // Dword address: M<n> (PLCASM format, for i32, u32, f32, i64, u64, f64, struct)
            // Set memoryOffset to absolute byte address for array element calculations
            sym->memoryOffset = plcasm_marker_offset + autoAddrDword;
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
            autoAddrDword += getTypeSize(sym->type) * count;  // Reserve space for all elements
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
        // Two-pass compilation:
        // Pass 1: Scan for function declarations and register their signatures
        // Pass 2: Generate code (now knowing all function signatures for forward references)
        
        // ====== Pass 1: Scan for function declarations ======
        compilationPass = 1;
        pos = 0;
        currentLine = 1;
        currentColumn = 1;
        hasPeekToken = false;
        nextToken();
        
        while (!check(PSTOK_EOF) && !hasError) {
            if (check(PSTOK_FUNCTION)) {
                scanFunctionDeclaration();
            } else {
                // Skip other tokens in pass 1
                skipToNextStatement();
            }
        }
        
        if (hasError) return false;
        
        // ====== Pass 2: Generate code ======
        compilationPass = 2;
        pos = 0;
        currentLine = 1;
        currentColumn = 1;
        hasPeekToken = false;
        nextToken();
        
        // Emit header comment
        emitComment("Generated by PLCScript Compiler");
        emit("\n");
        
        // Jump over function definitions to main code
        if (functionCount > 0) {
            emit("jmp __main_start\n\n");
        }
        
        // First, emit all function bodies
        for (int i = 0; i < functionCount; i++) {
            compileFunctionBody(&functions[i]);
            if (hasError) return false;
        }
        
        // Reset tokenizer to start of source for main code parsing
        pos = 0;
        currentLine = 1;
        currentColumn = 1;
        hasPeekToken = false;
        nextToken();
        
        // Emit main code label
        if (functionCount > 0) {
            emit("__main_start:\n");
        }
        
        // Parse main program (list of statements, excluding function declarations)
        while (!check(PSTOK_EOF) && !hasError) {
            if (check(PSTOK_FUNCTION)) {
                // Skip function declarations in pass 2 (already compiled)
                skipFunctionDeclaration();
            } else {
                parseStatement();
            }
        }
        
        // Emit exit if not already
        emitLine("exit");
        
        return !hasError;
    }
    
    // Skip tokens until we reach the next top-level statement
    void skipToNextStatement() {
        // Handle different token types
        int braceDepth = 0;
        
        // If we see a brace, skip the entire block
        if (check(PSTOK_LBRACE)) {
            braceDepth = 1;
            nextToken();
            while (!check(PSTOK_EOF) && braceDepth > 0) {
                if (check(PSTOK_LBRACE)) braceDepth++;
                else if (check(PSTOK_RBRACE)) braceDepth--;
                nextToken();
            }
            return;
        }
        
        // Skip until semicolon or brace (for statements with blocks)
        while (!check(PSTOK_EOF)) {
            if (check(PSTOK_SEMICOLON)) {
                nextToken();
                return;
            }
            if (check(PSTOK_LBRACE)) {
                // Found a block, skip it
                braceDepth = 1;
                nextToken();
                while (!check(PSTOK_EOF) && braceDepth > 0) {
                    if (check(PSTOK_LBRACE)) braceDepth++;
                    else if (check(PSTOK_RBRACE)) braceDepth--;
                    nextToken();
                }
                return;
            }
            nextToken();
        }
    }
    
    // Scan a function declaration and record its signature (Pass 1)
    void scanFunctionDeclaration() {
        int declLine = currentLine;
        int declColumn = currentColumn;
        
        nextToken(); // consume 'function'
        
        // Function name
        if (!check(PSTOK_IDENTIFIER)) {
            setError("Expected function name");
            return;
        }
        
        // Check for duplicate function name
        for (int i = 0; i < functionCount; i++) {
            if (strEq(functions[i].name, currentToken.text)) {
                setError("Function already defined");
                return;
            }
        }
        
        // Check for conflict with built-in function names
        if (strEqCI(currentToken.text, "TON") || strEqCI(currentToken.text, "TOF") || strEqCI(currentToken.text, "TP") ||
            strEqCI(currentToken.text, "CTU") || strEqCI(currentToken.text, "CTD") ||
            strEqCI(currentToken.text, "risingEdge") || strEqCI(currentToken.text, "FP") ||
            strEqCI(currentToken.text, "fallingEdge") || strEqCI(currentToken.text, "FN")) {
            setError("Function name conflicts with built-in PLC function");
            return;
        }
        
        // Check function limit
        if (functionCount >= PLCSCRIPT_MAX_FUNCTIONS) {
            setError("Too many functions");
            return;
        }
        
        PLCScriptFunction* func = &functions[functionCount];
        int i = 0;
        while (currentToken.text[i] && i < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
            func->name[i] = currentToken.text[i];
            i++;
        }
        func->name[i] = '\0';
        func->declarationLine = declLine;
        func->declarationColumn = declColumn;
        func->paramCount = 0;
        func->returnType = PSTYPE_VOID;
        func->isCompiled = false;
        
        // Generate entry label
        snprintf(func->entryLabel, sizeof(func->entryLabel), "__func_%s", func->name);
        
        nextToken();
        
        // Parameters: (param1: type1, param2: type2, ...)
        if (!match(PSTOK_LPAREN)) {
            setError("Expected '(' after function name");
            return;
        }
        
        while (!check(PSTOK_RPAREN) && !check(PSTOK_EOF)) {
            if (func->paramCount >= PLCSCRIPT_MAX_FUNC_PARAMS) {
                setError("Too many function parameters");
                return;
            }
            
            // Parameter name
            if (!check(PSTOK_IDENTIFIER)) {
                setError("Expected parameter name");
                return;
            }
            
            PLCScriptFuncParam* param = &func->params[func->paramCount];
            i = 0;
            while (currentToken.text[i] && i < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
                param->name[i] = currentToken.text[i];
                i++;
            }
            param->name[i] = '\0';
            
            // Check for duplicate parameter names
            for (int p = 0; p < func->paramCount; p++) {
                if (strEq(func->params[p].name, param->name)) {
                    setError("Duplicate parameter name");
                    return;
                }
            }
            
            nextToken();
            
            // : type
            if (!match(PSTOK_COLON)) {
                setError("Expected ':' after parameter name");
                return;
            }
            
            if (!isTypeKeyword(currentToken.type)) {
                setError("Expected type name");
                return;
            }
            param->type = tokenTypeToVarType(currentToken.type);
            nextToken();
            
            func->paramCount++;
            
            // Comma or end of parameters
            if (!check(PSTOK_RPAREN)) {
                if (!match(PSTOK_COMMA)) {
                    setError("Expected ',' or ')' in parameter list");
                    return;
                }
            }
        }
        
        if (!match(PSTOK_RPAREN)) {
            setError("Expected ')' after parameters");
            return;
        }
        
        // Return type: : type (optional, defaults to void)
        if (match(PSTOK_COLON)) {
            if (!isTypeKeyword(currentToken.type)) {
                setError("Expected return type");
                return;
            }
            func->returnType = tokenTypeToVarType(currentToken.type);
            nextToken();
        }
        
        // Record body position for later compilation
        if (!check(PSTOK_LBRACE)) {
            setError("Expected '{' to start function body");
            return;
        }
        
        // Record position of the '{' token so we can restart from here
        // currentToken is '{', and when we call nextToken() later, it will scan the first body token
        func->bodyStartPos = pos - 1;  // Back up to include the '{' character
        func->bodyStartLine = currentLine;
        func->bodyStartColumn = currentColumn;
        
        // Skip the body (we'll compile it in pass 2)
        int braceDepth = 1;
        nextToken(); // consume '{'
        while (!check(PSTOK_EOF) && braceDepth > 0) {
            if (check(PSTOK_LBRACE)) braceDepth++;
            else if (check(PSTOK_RBRACE)) braceDepth--;
            nextToken();
        }
        
        func->bodyEndPos = pos;
        functionCount++;
    }
    
    // Skip a function declaration in pass 2 (already compiled)
    void skipFunctionDeclaration() {
        nextToken(); // consume 'function'
        
        // Skip until we find the opening brace
        while (!check(PSTOK_LBRACE) && !check(PSTOK_EOF)) {
            nextToken();
        }
        
        if (!check(PSTOK_LBRACE)) return;
        
        // Skip the body
        int braceDepth = 1;
        nextToken(); // consume '{'
        while (!check(PSTOK_EOF) && braceDepth > 0) {
            if (check(PSTOK_LBRACE)) braceDepth++;
            else if (check(PSTOK_RBRACE)) braceDepth--;
            nextToken();
        }
    }
    
    // Compile a function body (Pass 2)
    void compileFunctionBody(PLCScriptFunction* func) {
        if (func->isCompiled) return;
        func->isCompiled = true;
        
        // Save current state
        PLCScriptFunction* prevFunction = currentFunction;
        int prevScopeLevel = currentScopeLevel;
        int prevSymbolCount = symbolCount;
        
        currentFunction = func;
        
        // Emit function label
        emit(func->entryLabel);
        emit(":\n");
        
        // Enter function scope
        currentScopeLevel++;
        
        // Declare parameters as local variables (in reverse order since they're pushed left-to-right)
        // Parameters are passed on the stack, so we pop them into local variables
        for (int i = func->paramCount - 1; i >= 0; i--) {
            PLCScriptFuncParam* param = &func->params[i];
            
            // Create symbol for parameter
            PLCScriptSymbol* sym = addSymbol(param->name, param->type, false);
            if (!sym) return;
            sym->isParam = true;
            sym->isLocal = true;
            
            // Generate auto address for parameter storage
            generateAutoAddress(sym);
            
            // Pop parameter from stack into local variable address
            emit(varTypeToPlcasm(param->type));
            emit(".move_to ");
            emit(sym->address);
            emit("\n");
        }
        
        // Position tokenizer at body start (at the '{')
        pos = func->bodyStartPos;
        currentLine = func->bodyStartLine;
        currentColumn = func->bodyStartColumn;
        hasPeekToken = false;
        nextToken();  // This should give us '{'
        
        // Consume '{' and get first body token
        if (!match(PSTOK_LBRACE)) {
            setError("Expected '{' to start function body");
            return;
        }
        
        // Parse function body statements until '}'
        lastStatementWasTerminator = false;
        while (!check(PSTOK_RBRACE) && !check(PSTOK_EOF) && !hasError) {
            parseStatement();
        }
        
        if (!match(PSTOK_RBRACE)) {
            setError("Expected '}' to end function body");
            return;
        }
        
        // Emit fallback return (in case function doesn't explicitly return)
        // This is unreachable if all code paths have explicit return statements
        if (func->returnType == PSTYPE_VOID) {
            emit("ret\n");
        } else {
            // For non-void functions, warn if the last statement wasn't a return
            if (!lastStatementWasTerminator) {
                addWarningAt("Not all code paths return a value", func->declarationLine, func->declarationColumn, 0);
            }
            // Push a default value and return as fallback
            emitLoadConst(func->returnType, 0);
            emit("ret\n");
        }
        lastStatementWasTerminator = false;
        
        emit("\n");
        
        // Exit function scope - remove local symbols
        while (symbolCount > prevSymbolCount) {
            symbolCount--;
        }
        currentScopeLevel = prevScopeLevel;
        currentFunction = prevFunction;
    }
    
    // Find a user-defined function by name
    PLCScriptFunction* findFunction(const char* name) {
        for (int i = 0; i < functionCount; i++) {
            if (strEq(functions[i].name, name)) {
                return &functions[i];
            }
        }
        return nullptr;
    }
    
    // ========================================================================
    // Struct Type Declaration Parsing
    // ========================================================================
    // Syntax: type MyStruct = struct { field1: type1; field2: type2; ... }
    
    void parseStructDeclaration() {
        nextToken(); // consume 'type'
        
        // Get struct name
        if (!check(PSTOK_IDENTIFIER)) {
            setError("Expected struct type name after 'type'");
            return;
        }
        char typeName[PLCSCRIPT_MAX_IDENTIFIER_LEN];
        int i = 0;
        while (currentToken.text[i] && i < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
            typeName[i] = currentToken.text[i];
            i++;
        }
        typeName[i] = '\0';
        nextToken();
        
        // Expect '='
        if (!match(PSTOK_EQ)) {
            setError("Expected '=' after struct type name");
            return;
        }
        
        // Expect 'struct'
        if (!check(PSTOK_STRUCT)) {
            setError("Expected 'struct' keyword");
            return;
        }
        nextToken();
        
        // Expect '{'
        if (!match(PSTOK_LBRACE)) {
            setError("Expected '{' after 'struct'");
            return;
        }
        
        // Create the struct type
        PLCScriptStructType* structType = addStructType(typeName);
        if (!structType) return;
        
        u32 currentOffset = 0;
        
        // Parse fields
        while (!check(PSTOK_RBRACE) && !check(PSTOK_EOF) && !hasError) {
            // Field name
            if (!check(PSTOK_IDENTIFIER)) {
                setError("Expected field name");
                return;
            }
            
            if (structType->fieldCount >= PLCSCRIPT_MAX_STRUCT_FIELDS) {
                setError("Too many fields in struct");
                return;
            }
            
            PLCScriptStructField* field = &structType->fields[structType->fieldCount];
            field->reset();
            
            int j = 0;
            while (currentToken.text[j] && j < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
                field->name[j] = currentToken.text[j];
                j++;
            }
            field->name[j] = '\0';
            
            // Check for duplicate field name
            for (int f = 0; f < structType->fieldCount; f++) {
                if (strEq(structType->fields[f].name, field->name)) {
                    setError("Duplicate field name in struct");
                    return;
                }
            }
            
            nextToken();
            
            // Expect ':'
            if (!match(PSTOK_COLON)) {
                setError("Expected ':' after field name");
                return;
            }
            
            // Parse field type
            PLCScriptVarType fieldType;
            int fieldStructIdx;
            if (!parseType(fieldType, fieldStructIdx)) {
                return;
            }
            
            field->type = fieldType;
            field->structTypeIndex = fieldStructIdx;
            field->offset = currentOffset;
            field->size = getTypeSize(fieldType, fieldStructIdx);
            field->used = true;
            
            currentOffset += field->size;
            structType->fieldCount++;
            
            // Semicolon between/after fields
            match(PSTOK_SEMICOLON);
        }
        
        // Expect '}'
        if (!match(PSTOK_RBRACE)) {
            setError("Expected '}' at end of struct definition");
            return;
        }
        
        structType->totalSize = currentOffset;
        
        // Register this struct type in the global UserStructType registry
        // so PLCASM can also use property access on variables of this type
        // Check for existing type with same name
        UserStructType* existing = findUserStructType(typeName);
        
        // Build temp UserStructType for comparison
        UserStructType tempUST;
        tempUST.reset();
        int ni = 0;
        while (typeName[ni] && ni < 31) { tempUST.name[ni] = typeName[ni]; ni++; }
        tempUST.name[ni] = '\0';
        tempUST.total_size = structType->totalSize;
        
        for (int k = 0; k < structType->fieldCount && k < MAX_STRUCT_PROPERTIES; k++) {
            PLCScriptStructField* sf = &structType->fields[k];
            StructProperty* sp = &tempUST.fields[tempUST.field_count++];
            
            // Copy field name
            ni = 0;
            while (sf->name[ni] && ni < 15) {
                sp->name[ni] = sf->name[ni];
                ni++;
            }
            sp->name[ni] = '\0';
            
            sp->offset = sf->offset;
            sp->type_size = sf->isBit ? 0 : sf->size;
            sp->bit_pos = sf->isBit ? sf->bitIndex : 255;
            sp->readable = true;
            sp->writable = true;
        }
        
        if (existing) {
            // Check if definitions match
            StructCompareResult cmp = compareUserStructTypes(existing, &tempUST);
            if (cmp == STRUCT_COMPARE_IDENTICAL) {
                // Duplicate but identical - emit warning comment
                emit("// WARNING: Duplicate struct type declaration '");
                emit(typeName);
                emit("' (identical to previous definition)\n");
            } else {
                // Conflict - add error
                char err[128];
                int ei = 0;
                const char* msg = "Conflicting struct type '";
                while (*msg && ei < 60) err[ei++] = *msg++;
                ni = 0;
                while (typeName[ni] && ei < 90) err[ei++] = typeName[ni++];
                const char* suffix = "' - definitions differ";
                int si = 0;
                while (suffix[si] && ei < 126) err[ei++] = suffix[si++];
                err[ei] = '\0';
                setError(err);
                return;
            }
        } else {
            // New type - register it
            UserStructType* ust = addUserStructType(typeName);
            if (ust) {
                ust->total_size = tempUST.total_size;
                ust->field_count = tempUST.field_count;
                for (int k = 0; k < tempUST.field_count; k++) {
                    ust->fields[k] = tempUST.fields[k];
                }
            }
        }
        
        // Optional trailing semicolon
        match(PSTOK_SEMICOLON);
        
        // Emit comment about the struct definition
        emit("// type ");
        emit(typeName);
        emit(" = struct { ");
        for (int k = 0; k < structType->fieldCount; k++) {
            if (k > 0) emit(", ");
            emit(structType->fields[k].name);
            emit(": ");
            if (structType->fields[k].type == PSTYPE_STRUCT) {
                emit(structTypes[structType->fields[k].structTypeIndex].name);
            } else {
                emit(varTypeToPlcasm(structType->fields[k].type));
            }
        }
        emit(" } // size=");
        emitInt(structType->totalSize);
        emit(" bytes\n");
    }
    
    // Parse a function declaration
    // Called from parseStatement when 'function' keyword is encountered
    // In pass 2, function declarations at top level are skipped by the main loop
    // This handles nested function declarations (which are errors)
    void parseFunctionDeclaration() {
        if (currentFunction != nullptr) {
            setError("Nested functions are not supported");
            // Skip to end of function declaration
            skipFunctionDeclaration();
            return;
        }
        // This should not be reached in normal flow since pass 2 skips functions
        // But just in case, skip the function
        skipFunctionDeclaration();
    }
    
    void parseStatement() {
        if (hasError) return;
        
        // Check for unreachable code (after return/break/continue)
        if (lastStatementWasTerminator) {
            // Empty statements and closing braces don't count as unreachable code
            if (currentToken.type != PSTOK_SEMICOLON && currentToken.type != PSTOK_RBRACE && currentToken.type != PSTOK_EOF) {
                addWarning("Unreachable code after return/break/continue");
            }
            lastStatementWasTerminator = false;
        }
        
        switch (currentToken.type) {
            case PSTOK_TYPE:
                parseStructDeclaration();
                break;
            case PSTOK_FUNCTION:
                parseFunctionDeclaration();
                break;
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
            case PSTOK_DO:
                parseDoWhileStatement();
                break;
            case PSTOK_FOR:
                parseForStatement();
                break;
            case PSTOK_LBRACE:
                parseBlock();
                break;
            case PSTOK_BREAK:
                parseBreak();
                lastStatementWasTerminator = true;
                break;
            case PSTOK_CONTINUE:
                parseContinue();
                lastStatementWasTerminator = true;
                break;
            case PSTOK_RETURN:
                parseReturn();
                lastStatementWasTerminator = true;
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
        
        // Check if variable name conflicts with a struct type (exact case match only)
        // This prevents 'let Motor: i32' when type 'Motor' exists, but allows 'let motor: Motor'
        if (findStructTypeIndexExact(name) >= 0 || findUserStructTypeExact(name)) {
            setError("Variable name conflicts with a type name");
            return;
        }
        
        nextToken();
        
        // Type annotation: ": type" or ": type[size]"
        PLCScriptVarType varType = PSTYPE_I16; // Default type
        int structTypeIdx = -1;
        MY_PTR_t array_size = 0;  // 0 = not an array
        u16 string_capacity = 0;  // 0 = not a string, or string capacity
        
        if (match(PSTOK_COLON)) {
            // Check for struct type first
            if (currentToken.type == PSTOK_IDENTIFIER) {
                int idx = findStructTypeIndex(currentToken.text);
                if (idx >= 0) {
                    varType = PSTYPE_STRUCT;
                    structTypeIdx = idx;
                    nextToken();
                } else if (!isTypeKeyword(currentToken.type)) {
                    setError("Unknown type name");
                    return;
                } else {
                    varType = tokenTypeToVarType(currentToken.type);
                    nextToken();
                }
            } else if (isTypeKeyword(currentToken.type)) {
                varType = tokenTypeToVarType(currentToken.type);
                nextToken();
            } else {
                setError("Expected type name");
                return;
            }
            
            // Check for array/capacity syntax: type[size]
            if (match(PSTOK_LBRACKET)) {
                // Handle string capacity: str8[capacity] or str16[capacity]
                if (varType == PSTYPE_STR8 || varType == PSTYPE_STR16) {
                    // Expect a numeric literal for string capacity
                    if (!check(PSTOK_INTEGER)) {
                        setError("Expected string capacity (numeric literal)");
                        return;
                    }
                    
                    int64_t cap = currentToken.intValue;
                    u16 max_cap = (varType == PSTYPE_STR8) ? 254 : 65534;
                    if (cap < 1 || cap > max_cap) {
                        setError("String capacity out of range");
                        return;
                    }
                    string_capacity = (u16)cap;
                    nextToken();
                    
                    if (!match(PSTOK_RBRACKET)) {
                        setError("Expected ']' after string capacity");
                        return;
                    }
                } else {
                    // Arrays of bit types not supported
                    if (varType == PSTYPE_BOOL) {
                        setError("Arrays of bool types not supported");
                        return;
                    }
                    
                    // Expect a numeric literal for array size
                    if (!check(PSTOK_INTEGER)) {
                        setError("Expected array size (numeric literal)");
                        return;
                    }
                    
                    // Parse array size from token
                    int64_t size = currentToken.intValue;
                    if (size < 1 || size > MY_PTR_MAX) {
                        setError("Array size out of range");
                        return;
                    }
                    array_size = (MY_PTR_t)size;
                    nextToken();
                    
                    if (!match(PSTOK_RBRACKET)) {
                        setError("Expected ']' after array size");
                        return;
                    }
                }
            } else if (varType == PSTYPE_STR8 || varType == PSTYPE_STR16) {
                // String without capacity - require capacity specification
                setError("String type requires capacity: str8[capacity] or str16[capacity]");
                return;
            }
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
        
        sym->structTypeIndex = structTypeIdx;
        sym->isLocal = isLocal;
        sym->isAutoAddress = isAutoAddress;
        sym->stackSlot = -1;
        sym->array_size = array_size;
        sym->stringCapacity = string_capacity;
        
        if (isAutoAddress || isLocal) {
            // Generate auto address for both explicit "@ auto" and local variables
            // For arrays, generateAutoAddress needs to account for array size
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
        
        // Register struct variables in the shared symbol table so PLCASM can use property access
        if (hasAddress && varType == PSTYPE_STRUCT && structTypeIdx >= 0) {
            const char* structTypeName = structTypes[structTypeIdx].name;
            addSharedSymbol(name, structTypeName, sym->memoryOffset, 0, false, structTypes[structTypeIdx].totalSize, array_size);
        } else if (hasAddress && (varType == PSTYPE_STR8 || varType == PSTYPE_STR16)) {
            // Register string in shared symbol table (type_size = total string size)
            u32 str_size = sym->stringSizeInBytes();
            addSharedSymbol(name, varTypeToPlcasm(varType), sym->memoryOffset, 0, false, (u8)str_size, 0);
        } else if (hasAddress && array_size > 0) {
            // Register array in shared symbol table
            u8 elem_size = (u8)getTypeSize(varType);
            addSharedSymbol(name, varTypeToPlcasm(varType), sym->memoryOffset, 0, false, elem_size, array_size);
        }
        
        // Emit comment
        emit("// ");
        emit(isConst ? "const " : "let ");
        emit(name);
        emit(": ");
        emit(varTypeToPlcasm(varType));
        if (string_capacity > 0) {
            // Show string capacity
            emit("[");
            char sizeBuf[16];
            int si = 0;
            u16 s = string_capacity;
            if (s == 0) sizeBuf[si++] = '0';
            else {
                char temp[16];
                int ti = 0;
                while (s > 0) { temp[ti++] = '0' + (s % 10); s /= 10; }
                while (ti > 0) sizeBuf[si++] = temp[--ti];
            }
            sizeBuf[si] = '\0';
            emit(sizeBuf);
            emit("]");
        } else if (array_size > 0) {
            emit("[");
            char sizeBuf[16];
            int si = 0;
            MY_PTR_t s = array_size;
            if (s == 0) sizeBuf[si++] = '0';
            else {
                char temp[16];
                int ti = 0;
                while (s > 0) { temp[ti++] = '0' + (s % 10); s /= 10; }
                while (ti > 0) sizeBuf[si++] = temp[--ti];
            }
            sizeBuf[si] = '\0';
            emit(sizeBuf);
            emit("]");
        }
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
        
        // String initialization: emit str.init to set capacity
        if (varType == PSTYPE_STR8 || varType == PSTYPE_STR16) {
            const char* strType = (varType == PSTYPE_STR8) ? "str" : "str16";
            u16 cap = sym->stringCapacity;
            
            // Emit: u16.const <capacity>; str.init <addr>
            emit("u16.const ");
            char capBuf[16];
            int ci = 0;
            if (cap == 0) capBuf[ci++] = '0';
            else {
                char temp[16];
                int ti = 0;
                while (cap > 0) { temp[ti++] = '0' + (cap % 10); cap /= 10; }
                while (ti > 0) capBuf[ci++] = temp[--ti];
            }
            capBuf[ci] = '\0';
            emit(capBuf);
            emit("\n");
            emit(strType);
            emit(".init ");
            emit(sym->address);
            emit("\n");
        }
        
        // Initializer: "= expression"
        if (match(PSTOK_EQ)) {
            // Set target type hint for expression parsing
            PLCScriptVarType savedTarget = targetType;
            targetType = varType;
            
            // Reset string context tracking
            lastStringSymbol = nullptr;
            hasPendingStringLiteral = false;
            hasPendingTemplateLiteral = false;
            
            // Generate expression code
            PLCScriptVarType exprType = parseExpression();
            targetType = savedTarget;
            
            // Handle string type initialization specially
            if (varType == PSTYPE_STR8 || varType == PSTYPE_STR16) {
                if (exprType != PSTYPE_STR8 && exprType != PSTYPE_STR16) {
                    setError("Cannot initialize string with non-string value");
                    return;
                }
                
                const char* cstrType = (varType == PSTYPE_STR8) ? "cstr" : "cstr16";
                const char* strType = (varType == PSTYPE_STR8) ? "str" : "str16";
                
                // Check for template literal initialization (e.g., `Hello ${name}`)
                if (hasPendingTemplateLiteral) {
                    emitTemplateLiteral(sym->address, varType, pendingTemplateLiteral, pendingTemplateLiteralLen);
                    hasPendingTemplateLiteral = false;
                    sym->hasInitializer = true;
                }
                // Check for string literal initialization
                else if (hasPendingStringLiteral) {
                    // Use cstr.lit for efficient inline string literal copy
                    emit(cstrType);
                    emit(".lit ");
                    emit(sym->address);
                    emit(" ");
                    emitStringLiteral(pendingStringLiteral, pendingStringLiteralLen);
                    emit("\n");
                    hasPendingStringLiteral = false;
                    sym->hasInitializer = true;
                } else if (lastStringSymbol) {
                    // Copy from another string variable
                    emit(strType);
                    emit(".copy ");
                    emit(sym->address);
                    emit(" ");
                    emit(lastStringSymbol->address);
                    emit("\n");
                    sym->hasInitializer = true;
                } else {
                    setError("String initializer must be a literal or variable");
                    return;
                }

            } else {
                // Type conversion if needed (non-string types)
                if (exprType != varType && exprType != PSTYPE_VOID && !isStringType(exprType)) {
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
        } else if (isConst) {
            // const requires an initializer
            setError("Constant declaration requires an initializer");
            return;
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
    
    void parseDoWhileStatement() {
        nextToken(); // consume 'do'
        
        // Generate labels
        char startLabel[64], endLabel[64];
        generateLabel(startLabel, "dowhile");
        generateLabel(endLabel, "enddowhile");
        
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
        
        // Body (must be a block)
        parseStatement();
        
        // Expect 'while' after body
        if (!check(PSTOK_WHILE)) {
            setError("Expected 'while' after 'do' body");
            if (loopStackDepth > 0) loopStackDepth--;
            return;
        }
        nextToken(); // consume 'while'
        
        expect(PSTOK_LPAREN, "Expected '(' after 'while'");
        parseExpression();
        expect(PSTOK_RPAREN, "Expected ')' after condition");
        
        emitJumpIfTrue(startLabel);
        emitLabel(endLabel);
        
        // Pop loop context
        if (loopStackDepth > 0) loopStackDepth--;
        
        // Optional semicolon
        match(PSTOK_SEMICOLON);
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
        
        // Check for empty block
        if (check(PSTOK_RBRACE)) {
            addWarning("Empty block");
        }
        
        // Reset terminator tracking within block scope
        bool savedTerminator = lastStatementWasTerminator;
        lastStatementWasTerminator = false;
        
        while (!check(PSTOK_RBRACE) && !check(PSTOK_EOF) && !hasError) {
            parseStatement();
        }
        
        // Restore terminator state (don't leak block-level state)
        lastStatementWasTerminator = savedTerminator;
        
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
        
        if (currentFunction != nullptr) {
            // Inside a function
            if (!check(PSTOK_SEMICOLON) && !check(PSTOK_RBRACE)) {
                // Has return value
                PLCScriptVarType savedTarget = targetType;
                targetType = currentFunction->returnType;
                PLCScriptVarType retType = parseExpression();
                targetType = savedTarget;
                
                // Convert return value to function's return type if needed
                if (retType != currentFunction->returnType && currentFunction->returnType != PSTYPE_VOID) {
                    emit("cvt ");
                    emit(varTypeToPlcasm(retType));
                    emit(" ");
                    emitLine(varTypeToPlcasm(currentFunction->returnType));
                }
                // Value stays on stack for caller
            } else if (currentFunction->returnType != PSTYPE_VOID) {
                // No return value but function expects one
                setError("Non-void function must return a value");
                return;
            }
            emitLine("ret");
        } else {
            // At top level - return means exit
            if (!check(PSTOK_SEMICOLON)) {
                PLCScriptVarType retType = parseExpression();
                if (retType != PSTYPE_VOID) emitDrop(retType);
            }
            emitLine("exit");
        }
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
    // Template Literal Processing: `Hello ${name}!`
    // ========================================================================
    
    // Emit a template literal to a string destination
    // tmpl and tmplLen are the template content (without backticks)
    void emitTemplateLiteral(const char* destAddr, PLCScriptVarType destType, const char* tmpl, int tmplLen) {
        const char* strType = (destType == PSTYPE_STR8) ? "str" : "str16";
        const char* cstrType = (destType == PSTYPE_STR8) ? "cstr" : "cstr16";
        
        // First, clear the destination string
        emit(strType);
        emit(".clear ");
        emit(destAddr);
        emit("\n");
        
        // Parse through the template string
        int i = 0;
        char textBuf[PLCSCRIPT_MAX_IDENTIFIER_LEN];
        int textLen = 0;
        bool isFirstSegment = true;
        
        while (i < tmplLen) {
            // Check for interpolation start ${
            if (i + 1 < tmplLen && tmpl[i] == '$' && tmpl[i + 1] == '{') {
                // Emit accumulated text segment first
                if (textLen > 0) {
                    if (isFirstSegment) {
                        // Use cstr.lit for first segment
                        emit(cstrType);
                        emit(".lit ");
                        emit(destAddr);
                        emit(" ");
                        emitStringLiteral(textBuf, textLen);
                        emit("\n");
                        isFirstSegment = false;
                    } else {
                        // Use cstr.cat for subsequent text segments (efficient concatenation)
                        emit(cstrType);
                        emit(".cat ");
                        emit(destAddr);
                        emit(" ");
                        emitStringLiteral(textBuf, textLen);
                        emit("\n");
                    }
                    textLen = 0;
                }
                
                // Skip ${
                i += 2;
                
                // Extract the expression between ${ and }
                char exprBuf[PLCSCRIPT_MAX_IDENTIFIER_LEN];
                int exprLen = 0;
                int braceDepth = 1;
                
                while (i < tmplLen && braceDepth > 0) {
                    if (tmpl[i] == '{') braceDepth++;
                    else if (tmpl[i] == '}') {
                        braceDepth--;
                        if (braceDepth == 0) break;
                    }
                    if (exprLen < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
                        exprBuf[exprLen++] = tmpl[i];
                    }
                    i++;
                }
                exprBuf[exprLen] = '\0';
                i++; // Skip closing }
                
                // Now we need to parse and emit the expression
                // Save current tokenizer state
                int savedPos = pos;
                int savedLine = currentLine;
                int savedCol = currentColumn;
                PLCScriptToken savedToken = currentToken;
                bool savedHasPeek = hasPeekToken;
                PLCScriptToken savedPeek = peekToken;
                char* savedSource = source;
                int savedSourceLen = sourceLength;
                
                // Temporarily set source to the expression
                source = exprBuf;
                sourceLength = exprLen;
                pos = 0;
                currentLine = 1;
                currentColumn = 1;
                hasPeekToken = false;
                
                // Tokenize and parse the expression
                nextToken();
                PLCScriptVarType exprType = parseExpression();
                
                // Restore tokenizer state
                source = savedSource;
                sourceLength = savedSourceLen;
                pos = savedPos;
                currentLine = savedLine;
                currentColumn = savedCol;
                currentToken = savedToken;
                hasPeekToken = savedHasPeek;
                peekToken = savedPeek;
                
                // Emit code to concat the expression result to the string
                if (exprType == PSTYPE_STR8 || exprType == PSTYPE_STR16) {
                    // String expression - should have set lastStringSymbol
                    if (lastStringSymbol) {
                        emit(strType);
                        emit(".concat ");
                        emit(destAddr);
                        emit(" ");
                        emit(lastStringSymbol->address);
                        emit("\n");
                    }
                } else if (exprType != PSTYPE_VOID) {
                    // Numeric expression - convert to string and concat
                    // Stack has the numeric value, use str.from to append
                    emit(strType);
                    emit(".from.");
                    emit(varTypeToPlcasm(exprType));
                    emit(" ");
                    emit(destAddr);
                    emit(" ");
                    // Use default base (10) or decimals (2)
                    if (isFloatType(exprType)) {
                        emit("2"); // 2 decimal places for floats
                    } else {
                        emit("10"); // base 10 for integers
                    }
                    emit("\n");
                }
                
                isFirstSegment = false;
            } else {
                // Regular character - accumulate
                if (textLen < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
                    textBuf[textLen++] = tmpl[i];
                }
                i++;
            }
        }
        
        // Emit remaining text segment
        if (textLen > 0) {
            if (isFirstSegment) {
                // Use cstr.lit for first (and only) segment
                emit(cstrType);
                emit(".lit ");
                emit(destAddr);
                emit(" ");
                emitStringLiteral(textBuf, textLen);
                emit("\n");
            } else {
                // Use cstr.cat for subsequent text segments (efficient concatenation)
                emit(cstrType);
                emit(".cat ");
                emit(destAddr);
                emit(" ");
                emitStringLiteral(textBuf, textLen);
                emit("\n");
            }
        }
    }
    
    // Emit a template literal for concatenation (+=) - doesn't clear first
    void emitTemplateLiteralConcat(const char* destAddr, PLCScriptVarType destType, const char* tmpl, int tmplLen) {
        const char* strType = (destType == PSTYPE_STR8) ? "str" : "str16";
        const char* cstrType = (destType == PSTYPE_STR8) ? "cstr" : "cstr16";
        
        // Parse through the template string
        int i = 0;
        char textBuf[PLCSCRIPT_MAX_IDENTIFIER_LEN];
        int textLen = 0;
        
        while (i < tmplLen) {
            // Check for interpolation start ${
            if (i + 1 < tmplLen && tmpl[i] == '$' && tmpl[i + 1] == '{') {
                // Emit accumulated text segment using cstr.cat (efficient concatenation)
                if (textLen > 0) {
                    emit(cstrType);
                    emit(".cat ");
                    emit(destAddr);
                    emit(" ");
                    emitStringLiteral(textBuf, textLen);
                    emit("\n");
                    textLen = 0;
                }
                
                // Skip ${
                i += 2;
                
                // Extract the expression between ${ and }
                char exprBuf[PLCSCRIPT_MAX_IDENTIFIER_LEN];
                int exprLen = 0;
                int braceDepth = 1;
                
                while (i < tmplLen && braceDepth > 0) {
                    if (tmpl[i] == '{') braceDepth++;
                    else if (tmpl[i] == '}') {
                        braceDepth--;
                        if (braceDepth == 0) break;
                    }
                    if (exprLen < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
                        exprBuf[exprLen++] = tmpl[i];
                    }
                    i++;
                }
                exprBuf[exprLen] = '\0';
                i++; // Skip closing }
                
                // Parse and emit the expression
                int savedPos = pos;
                int savedLine = currentLine;
                int savedCol = currentColumn;
                PLCScriptToken savedToken = currentToken;
                bool savedHasPeek = hasPeekToken;
                PLCScriptToken savedPeek = peekToken;
                char* savedSource = source;
                int savedSourceLen = sourceLength;
                
                source = exprBuf;
                sourceLength = exprLen;
                pos = 0;
                currentLine = 1;
                currentColumn = 1;
                hasPeekToken = false;
                
                nextToken();
                PLCScriptVarType exprType = parseExpression();
                
                source = savedSource;
                sourceLength = savedSourceLen;
                pos = savedPos;
                currentLine = savedLine;
                currentColumn = savedCol;
                currentToken = savedToken;
                hasPeekToken = savedHasPeek;
                peekToken = savedPeek;
                
                // Emit code to concat the expression result to the string
                if (exprType == PSTYPE_STR8 || exprType == PSTYPE_STR16) {
                    if (lastStringSymbol) {
                        emit(strType);
                        emit(".concat ");
                        emit(destAddr);
                        emit(" ");
                        emit(lastStringSymbol->address);
                        emit("\n");
                    }
                } else if (exprType != PSTYPE_VOID) {
                    emit(strType);
                    emit(".from.");
                    emit(varTypeToPlcasm(exprType));
                    emit(" ");
                    emit(destAddr);
                    emit(" ");
                    if (isFloatType(exprType)) {
                        emit("2");
                    } else {
                        emit("10");
                    }
                    emit("\n");
                }
            } else {
                // Regular character - accumulate
                if (textLen < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
                    textBuf[textLen++] = tmpl[i];
                }
                i++;
            }
        }
        
        // Emit remaining text segment using cstr.cat (efficient concatenation)
        if (textLen > 0) {
            emit(cstrType);
            emit(".cat ");
            emit(destAddr);
            emit(" ");
            emitStringLiteral(textBuf, textLen);
            emit("\n");
        }
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
            
            // False branch - set targetType so literals match true branch type
            emitLabel(falseLabel);
            PLCScriptVarType savedTarget = targetType;
            targetType = trueType;
            (void)parseTernary(); // Right-associative, discard type (should match trueType)
            targetType = savedTarget;
            
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
            
            // If not found locally and not a direct address, check shared symbol table
            SharedSymbol* sharedSymForStruct = nullptr;  // Keep track of shared symbol for struct lookup
            if (!sym && !isDirectAddress) {
                SharedSymbol* sharedSym = sharedSymbols.findSymbol(varName);
                if (sharedSym) {
                    populateFromSharedSymbol(&tempSym, sharedSym);
                    sym = &tempSym;
                    sharedSymForStruct = sharedSym;  // May be a struct type
                }
            }
            
            nextToken();;
            
            // Check for struct property assignment (e.g., myStruct.field = value)
            // Check both local structs AND shared symbol user struct types
            bool isLocalStruct = (sym && sym->type == PSTYPE_STRUCT && sym->structTypeIndex >= 0);
            UserStructType* userStructType = nullptr;
            if (!isLocalStruct && sharedSymForStruct) {
                userStructType = findUserStructType(sharedSymForStruct->type);
            }
            
            if (check(PSTOK_DOT) && (isLocalStruct || userStructType)) {
                nextToken(); // consume dot
                
                if (!check(PSTOK_IDENTIFIER)) {
                    setError("Expected field name after '.'");
                    return PSTYPE_VOID;
                }
                
                char fieldName[32];
                int fi = 0;
                while (currentToken.text[fi] && fi < 31) {
                    fieldName[fi] = currentToken.text[fi];
                    fi++;
                }
                fieldName[fi] = '\0';
                nextToken();
                
                // Find the field - from local struct or user struct type
                PLCScriptStructField* localField = nullptr;
                const StructProperty* userField = nullptr;
                
                if (isLocalStruct) {
                    PLCScriptStructType* structType = &structTypes[sym->structTypeIndex];
                    localField = structType->findField(fieldName);
                } else if (userStructType) {
                    userField = userStructType->findField(fieldName);
                }
                
                if (!localField && !userField) {
                    setError("Unknown struct field");
                    return PSTYPE_VOID;
                }
                
                // Create a temporary symbol for the field to use with emitStoreToAddress
                PLCScriptSymbol fieldSym;
                memset(&fieldSym, 0, sizeof(fieldSym));
                fieldSym.isConst = false;
                
                // Determine field type and offset based on which kind of struct
                int fieldOffset = 0;
                bool fieldIsBit = false;
                int fieldBitIndex = 0;
                
                if (localField) {
                    fieldSym.type = localField->type;
                    fieldOffset = localField->offset;
                    fieldIsBit = localField->isBit;
                    fieldBitIndex = localField->bitIndex;
                } else if (userField) {
                    // type_size: 0=bit, 1=byte, 2=word, 4=dword, 8=qword
                    fieldIsBit = (userField->bit_pos < 8 || userField->type_size == 0);
                    fieldBitIndex = userField->bit_pos;
                    fieldOffset = userField->offset;
                    if (fieldIsBit) {
                        fieldSym.type = PSTYPE_BOOL;
                    } else {
                        switch (userField->type_size) {
                            case 1: fieldSym.type = PSTYPE_U8; break;
                            case 2: fieldSym.type = PSTYPE_U16; break;
                            case 4: fieldSym.type = PSTYPE_U32; break;
                            case 8: fieldSym.type = PSTYPE_U64; break;
                            default: fieldSym.type = PSTYPE_U16; break;
                        }
                    }
                }
                
                // Calculate field address: base_address + field_offset
                // NOTE: Don't add type modifiers (D/W) - PLCASM uses instruction type prefix
                int idx = 0;
                char area = sym->address[0];
                int baseNum = 0;
                int j = 1;
                // Skip type modifier if present in source address
                if (sym->address[1] == 'W' || sym->address[1] == 'D' ||
                    sym->address[1] == 'w' || sym->address[1] == 'd') {
                    j = 2;
                }
                while (sym->address[j] >= '0' && sym->address[j] <= '9') {
                    baseNum = baseNum * 10 + (sym->address[j] - '0');
                    j++;
                }
                
                // Build field address (no type modifier)
                fieldSym.address[idx++] = area;
                
                int newAddr = baseNum + fieldOffset;
                char numBuf[16];
                int ni = 0;
                if (newAddr == 0) {
                    numBuf[ni++] = '0';
                } else {
                    int temp = newAddr;
                    while (temp > 0) {
                        numBuf[ni++] = '0' + (temp % 10);
                        temp /= 10;
                    }
                }
                for (int r = ni - 1; r >= 0; r--) {
                    fieldSym.address[idx++] = numBuf[r];
                }
                
                // Handle bit fields
                if (fieldIsBit) {
                    fieldSym.address[idx++] = '.';
                    fieldSym.address[idx++] = '0' + fieldBitIndex;
                    fieldSym.type = PSTYPE_BOOL;
                }
                fieldSym.address[idx] = '\0';
                
                if (check(PSTOK_EQ)) {
                    // Simple assignment to struct field
                    nextToken(); // consume '='
                    
                    PLCScriptVarType savedTarget = targetType;
                    targetType = fieldSym.type;
                    PLCScriptVarType rhsType = parseAssignment();
                    targetType = savedTarget;
                    
                    // Type conversion if needed
                    if (rhsType != fieldSym.type && rhsType != PSTYPE_VOID) {
                        emit("cvt ");
                        emit(varTypeToPlcasm(rhsType));
                        emit(" ");
                        emit(varTypeToPlcasm(fieldSym.type));
                        emit("\n");
                    }
                    
                    emitCopy(fieldSym.type);
                    emitStoreToAddress(&fieldSym);
                    return fieldSym.type;
                }
                else if (check(PSTOK_PLUS_EQ) || check(PSTOK_MINUS_EQ) || 
                         check(PSTOK_STAR_EQ) || check(PSTOK_SLASH_EQ)) {
                    // Compound assignment to struct field
                    PLCScriptTokenType opType = currentToken.type;
                    nextToken();
                    
                    emitLoadFromAddress(&fieldSym);
                    
                    PLCScriptVarType savedTarget = targetType;
                    targetType = fieldSym.type;
                    PLCScriptVarType rhsType = parseAssignment();
                    targetType = savedTarget;
                    
                    if (rhsType != fieldSym.type && rhsType != PSTYPE_VOID) {
                        emit("cvt ");
                        emit(varTypeToPlcasm(rhsType));
                        emit(" ");
                        emit(varTypeToPlcasm(fieldSym.type));
                        emit("\n");
                    }
                    
                    const char* op = nullptr;
                    switch (opType) {
                        case PSTOK_PLUS_EQ: op = "add"; break;
                        case PSTOK_MINUS_EQ: op = "sub"; break;
                        case PSTOK_STAR_EQ: op = "mul"; break;
                        case PSTOK_SLASH_EQ: op = "div"; break;
                        default: break;
                    }
                    if (op) emitBinaryOp(op, fieldSym.type);
                    
                    emitCopy(fieldSym.type);
                    emitStoreToAddress(&fieldSym);
                    return fieldSym.type;
                }
                else if (check(PSTOK_PLUS_PLUS) || check(PSTOK_MINUS_MINUS)) {
                    // Post-increment/decrement on struct field
                    bool isIncr = check(PSTOK_PLUS_PLUS);
                    nextToken();
                    
                    emitLoadFromAddress(&fieldSym);
                    emitCopy(fieldSym.type);
                    emitLoadConst(fieldSym.type, 1);
                    emitBinaryOp(isIncr ? "add" : "sub", fieldSym.type);
                    emitStoreToAddress(&fieldSym);
                    
                    return fieldSym.type;
                }
                
                // Not an assignment - restore and let expression parsing handle it
                pos = savedPos;
                currentLine = savedLine;
                currentColumn = savedCol;
                currentToken = savedToken;
                hasPeekToken = savedHasPeek;
                peekToken = savedPeek;
                return parseTernary();
            }
            
            // Check for array element assignment (e.g., arr[0] = value, arr[i] = value)
            if (check(PSTOK_LBRACKET)) {
                nextToken(); // consume '['
                
                // Find the array symbol
                PLCScriptSymbol* arrSym = sym;
                PLCScriptSymbol tempArrSym;
                
                if (!arrSym) {
                    SharedSymbol* sharedSym = sharedSymbols.findSymbol(varName);
                    if (sharedSym) {
                        memset(&tempArrSym, 0, sizeof(tempArrSym));
                        populateFromSharedSymbol(&tempArrSym, sharedSym);
                        arrSym = &tempArrSym;
                    }
                }
                
                if (!arrSym) {
                    setError("Undefined array variable");
                    return PSTYPE_VOID;
                }
                
                if (!arrSym->isArray()) {
                    setError("Variable is not an array");
                    return PSTYPE_VOID;
                }
                
                // Parse index expression
                // Save output position in case we need to restore (for array reads, not writes)
                int savedOutputLen = outputLength;
                bool isStaticIndex = check(PSTOK_INTEGER);
                int64_t staticIndex = 0;
                
                if (isStaticIndex) {
                    staticIndex = currentToken.intValue;
                    nextToken();
                    
                    if (staticIndex < 0 || staticIndex >= arrSym->array_size) {
                        setError("Array index out of bounds");
                        return PSTYPE_VOID;
                    }
                } else {
                    // Dynamic index - need to save index for later use
                    PLCScriptVarType indexType = parseExpression();
                    
                    if (indexType != PSTYPE_U32 && indexType != PSTYPE_I32) {
                        emit("cvt ");
                        emit(varTypeToPlcasm(indexType));
                        emit(" u32\n");
                    }
                }
                
                if (!match(PSTOK_RBRACKET)) {
                    setError("Expected ']' after array index");
                    return PSTYPE_VOID;
                }
                
                PLCScriptVarType elemType = arrSym->type;
                u8 elemSize = (u8)getTypeSize(elemType);
                
                // Check for assignment operators
                if (check(PSTOK_EQ)) {
                    // This is an array element write - check const now
                    if (arrSym->isConst) {
                        setError("Cannot assign to const array");
                        return PSTYPE_VOID;
                    }
                    
                    nextToken(); // consume '='
                    
                    if (isStaticIndex) {
                        // Static index: compute address at compile time
                        u32 offset = (u32)staticIndex * elemSize;
                        
                        PLCScriptVarType savedTarget = targetType;
                        targetType = elemType;
                        PLCScriptVarType rhsType = parseAssignment();
                        targetType = savedTarget;
                        
                        if (rhsType != elemType && rhsType != PSTYPE_VOID) {
                            emit("cvt ");
                            emit(varTypeToPlcasm(rhsType));
                            emit(" ");
                            emit(varTypeToPlcasm(elemType));
                            emit("\n");
                        }
                        
                        emitCopy(elemType);
                        
                        // Build address and store using raw address prefix '#'
                        // memoryOffset contains absolute byte address, so we use # prefix
                        // to prevent PLCASM from adding area offset again
                        char addr[64];
                        int idx = 0;
                        addr[idx++] = '#';
                        u32 newAddr = arrSym->memoryOffset + offset;
                        char numBuf[16];
                        int ni = 0;
                        if (newAddr == 0) numBuf[ni++] = '0';
                        else {
                            u32 temp = newAddr;
                            while (temp > 0) { numBuf[ni++] = '0' + (temp % 10); temp /= 10; }
                        }
                        for (int r = ni - 1; r >= 0; r--) addr[idx++] = numBuf[r];
                        addr[idx] = '\0';
                        
                        emit(varTypeToPlcasm(elemType));
                        emit(".move_to ");
                        emit(addr);
                        emit("\n");
                    } else {
                        // Dynamic index: stack has index
                        // Need to: compute address, then parse RHS, then store using pointer
                        
                        // Convert index to ptr for pointer arithmetic
                        emit("cvt u32 ptr\n");
                        
                        // Multiply index by element size
                        if (elemSize > 1) {
                            emit("ptr.const ");
                            char sizeBuf[16];
                            int si = 0;
                            u8 s = elemSize;
                            if (s == 0) sizeBuf[si++] = '0';
                            else {
                                char temp[16];
                                int ti = 0;
                                while (s > 0) { temp[ti++] = '0' + (s % 10); s /= 10; }
                                while (ti > 0) sizeBuf[si++] = temp[--ti];
                            }
                            sizeBuf[si] = '\0';
                            emit(sizeBuf);
                            emit("\nptr.mul\n");
                        }
                        
                        // Add base address - now we have pointer on stack
                        emit("ptr.const ");
                        char baseBuf[16];
                        int bi = 0;
                        u32 base = arrSym->memoryOffset;
                        if (base == 0) baseBuf[bi++] = '0';
                        else {
                            char temp[16];
                            int ti = 0;
                            while (base > 0) { temp[ti++] = '0' + (base % 10); base /= 10; }
                            while (ti > 0) baseBuf[bi++] = temp[--ti];
                        }
                        baseBuf[bi] = '\0';
                        emit(baseBuf);
                        emit("\nptr.add\n");
                        
                        // Stack: [address (ptr)]
                        // Parse RHS expression
                        PLCScriptVarType savedTarget = targetType;
                        targetType = elemType;
                        PLCScriptVarType rhsType = parseAssignment();
                        targetType = savedTarget;
                        
                        if (rhsType != elemType && rhsType != PSTYPE_VOID) {
                            emit("cvt ");
                            emit(varTypeToPlcasm(rhsType));
                            emit(" ");
                            emit(varTypeToPlcasm(elemType));
                            emit("\n");
                        }
                        
                        // Stack: [address] [value]
                        // Store using pointer and keep value on stack
                        emit(varTypeToPlcasm(elemType));
                        emit(".move_copy\n");
                    }
                    
                    return elemType;
                }
                
                // Not an assignment - restore state AND output position, let expression parsing handle read
                pos = savedPos;
                currentLine = savedLine;
                currentColumn = savedCol;
                currentToken = savedToken;
                hasPeekToken = savedHasPeek;
                peekToken = savedPeek;
                outputLength = savedOutputLen;  // Restore emitted code too
                output[outputLength] = '\0';
                return parseTernary();
            }
            
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
                
                // Handle string assignment specially
                if (sym->type == PSTYPE_STR8 || sym->type == PSTYPE_STR16) {
                    const char* strType = (sym->type == PSTYPE_STR8) ? "str" : "str16";
                    
                    // Check if assigning from a string literal
                    if (check(PSTOK_STRING)) {
                        // String literal assignment - use cstr.lit for efficient inline copy
                        // The literal is in currentToken.text (WITHOUT quotes, escapes already processed by tokenizer)
                        const char* litText = currentToken.text;
                        int litLen = currentToken.textLen;
                        
                        const char* cstrType = (sym->type == PSTYPE_STR8) ? "cstr" : "cstr16";
                        emit(cstrType);
                        emit(".lit ");
                        emit(sym->address);
                        emit(" ");
                        emitStringLiteral(litText, litLen);
                        emit("\n");
                        
                        nextToken(); // consume string literal
                        return sym->type;
                    }
                    
                    // Check if assigning from a template literal: `Hello ${name}!`
                    if (check(PSTOK_TEMPLATE)) {
                        emitTemplateLiteral(sym->address, sym->type, currentToken.text, currentToken.textLen);
                        nextToken(); // consume template literal
                        return sym->type;
                    }
                    
                    // Otherwise, should be assigning from another string variable or toString()
                    lastStringSymbol = nullptr;
                    lastSubstrSymbol = nullptr;
                    hasSubstrArgs = false;
                    hasToStringCall = false;
                    PLCScriptVarType rhsType = parseAssignment();
                    
                    if (rhsType != PSTYPE_STR8 && rhsType != PSTYPE_STR16) {
                        setError("Cannot assign non-string to string variable");
                        return PSTYPE_VOID;
                    }
                    
                    // Check if this is a number.toString() or number.toFixed() operation
                    if (hasToStringCall) {
                        // Stack has the number value, emit str.from.<type>
                        emit(strType);
                        emit(".from.");
                        emit(varTypeToPlcasm(toStringNumType));
                        emit(" ");
                        emit(sym->address);
                        emit(" ");
                        // Emit base or decimals
                        char numBuf[8];
                        int nbi = 0;
                        u8 val = isFloatType(toStringNumType) ? toStringDecimals : toStringBase;
                        if (val == 0) numBuf[nbi++] = '0';
                        else {
                            char temp[8];
                            int ti = 0;
                            while (val > 0) { temp[ti++] = '0' + (val % 10); val /= 10; }
                            while (ti > 0) numBuf[nbi++] = temp[--ti];
                        }
                        numBuf[nbi] = '\0';
                        emit(numBuf);
                        emit("\n");
                        hasToStringCall = false;
                        return sym->type;
                    }
                    
                    if (!lastStringSymbol) {
                        setError("String assignment requires a string variable source");
                        return PSTYPE_VOID;
                    }
                    
                    // Check if this is a substr operation
                    if (lastSubstrSymbol && hasSubstrArgs) {
                        // str.substr dest src - stack has [start, length]
                        emit(strType);
                        emit(".substr ");
                        emit(sym->address);
                        emit(" ");
                        emit(lastSubstrSymbol->address);
                        emit("\n");
                        lastSubstrSymbol = nullptr;
                        hasSubstrArgs = false;
                    } else {
                        // Copy from source to destination
                        emit(strType);
                        emit(".copy ");
                        emit(sym->address);
                        emit(" ");
                        emit(lastStringSymbol->address);
                        emit("\n");
                    }
                    
                    return sym->type;
                }
                
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
                
                // Handle string concatenation with +=
                if ((sym->type == PSTYPE_STR8 || sym->type == PSTYPE_STR16) && opType == PSTOK_PLUS_EQ) {
                    const char* strType = (sym->type == PSTYPE_STR8) ? "str" : "str16";
                    
                    // Check if concatenating a string literal
                    if (check(PSTOK_STRING)) {
                        // The literal is in currentToken.text (WITHOUT quotes, escapes already processed)
                        const char* litText = currentToken.text;
                        int litLen = currentToken.textLen;
                        
                        // Append each character using str.char
                        for (int ci = 0; ci < litLen; ci++) {
                            char c = litText[ci];
                            emit("u8.const ");
                            char charBuf[8];
                            int cbi = 0;
                            u8 cv = (u8)c;
                            if (cv == 0) charBuf[cbi++] = '0';
                            else {
                                char temp[8];
                                int ti = 0;
                                while (cv > 0) { temp[ti++] = '0' + (cv % 10); cv /= 10; }
                                while (ti > 0) charBuf[cbi++] = temp[--ti];
                            }
                            charBuf[cbi] = '\0';
                            emit(charBuf);
                            emit("\n");
                            emit(strType);
                            emit(".char ");
                            emit(sym->address);
                            emit("\n");
                        }
                        
                        nextToken(); // consume string literal
                        return sym->type;
                    }
                    
                    // Check if concatenating a template literal: s += `Hello ${name}!`
                    if (check(PSTOK_TEMPLATE)) {
                        emitTemplateLiteralConcat(sym->address, sym->type, currentToken.text, currentToken.textLen);
                        nextToken(); // consume template literal
                        return sym->type;
                    }
                    
                    // Otherwise concatenating another string variable
                    lastStringSymbol = nullptr;
                    PLCScriptVarType rhsType = parseAssignment();
                    
                    if (rhsType != PSTYPE_STR8 && rhsType != PSTYPE_STR16) {
                        setError("Cannot concatenate non-string with +=");
                        return PSTYPE_VOID;
                    }
                    
                    if (!lastStringSymbol) {
                        setError("String += requires a string variable source");
                        return PSTYPE_VOID;
                    }
                    
                    emit(strType);
                    emit(".concat ");
                    emit(sym->address);
                    emit(" ");
                    emit(lastStringSymbol->address);
                    emit("\n");
                    
                    return sym->type;
                }
                
                // Non-string compound assignment
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
            // Set target type so literals are treated as bool
            PLCScriptVarType savedTarget = targetType;
            targetType = PSTYPE_BOOL;
            (void)parseLogicalAnd();
            targetType = savedTarget;
            emitLine("u8.or");
            left = PSTYPE_BOOL;
        }
        
        return left;
    }
    
    PLCScriptVarType parseLogicalAnd() {
        PLCScriptVarType left = parseBitwiseOr();
        
        while (check(PSTOK_AMP_AMP)) {
            nextToken();
            // Set target type so literals are treated as bool
            PLCScriptVarType savedTarget = targetType;
            targetType = PSTYPE_BOOL;
            (void)parseBitwiseOr();
            targetType = savedTarget;
            emitLine("u8.and");
            left = PSTYPE_BOOL;
        }
        
        return left;
    }
    
    PLCScriptVarType parseBitwiseOr() {
        PLCScriptVarType left = parseBitwiseXor();
        
        while (check(PSTOK_PIPE)) {
            nextToken();
            // Set target type so literals match the left operand's type
            PLCScriptVarType savedTarget = targetType;
            targetType = left;
            (void)parseBitwiseXor();
            targetType = savedTarget;
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
            // Set target type so literals match the left operand's type
            PLCScriptVarType savedTarget = targetType;
            targetType = left;
            (void)parseBitwiseAnd();
            targetType = savedTarget;
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
            // Set target type so literals match the left operand's type
            PLCScriptVarType savedTarget = targetType;
            targetType = left;
            (void)parseEquality();
            targetType = savedTarget;
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
        
        // String equality comparison
        if ((left == PSTYPE_STR8 || left == PSTYPE_STR16) && (check(PSTOK_EQ_EQ) || check(PSTOK_BANG_EQ))) {
            bool isEq = check(PSTOK_EQ_EQ);
            nextToken();
            
            PLCScriptSymbol* leftSym = lastStringSymbol;
            if (!leftSym) {
                setError("String == requires a string variable on the left side");
                return PSTYPE_BOOL;
            }
            const char* strType = (leftSym->type == PSTYPE_STR8) ? "str" : "str16";
            
            lastStringSymbol = nullptr;
            PLCScriptVarType right = parseComparison();
            
            if (right != PSTYPE_STR8 && right != PSTYPE_STR16) {
                setError("String == requires string operands");
                return PSTYPE_BOOL;
            }
            
            if (!lastStringSymbol) {
                setError("String == requires a string variable on the right side");
                return PSTYPE_BOOL;
            }
            
            // Emit str.eq left right -> pushes bool
            emit(strType);
            emit(".eq ");
            emit(leftSym->address);
            emit(" ");
            emit(lastStringSymbol->address);
            emit("\n");
            
            if (!isEq) {
                // For != , negate the result
                emit("u8.not\n");
            }
            
            return PSTYPE_BOOL;
        }
        
        while (check(PSTOK_EQ_EQ) || check(PSTOK_BANG_EQ)) {
            bool isEq = check(PSTOK_EQ_EQ);
            nextToken();
            // Set target type so literals match the left operand's type
            PLCScriptVarType savedTarget = targetType;
            targetType = left;
            (void)parseComparison();
            targetType = savedTarget;
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
            // Set target type so literals match the left operand's type
            PLCScriptVarType savedTarget = targetType;
            targetType = left;
            (void)parseShift();
            targetType = savedTarget;
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
        
        // String concatenation with + operator
        if ((left == PSTYPE_STR8 || left == PSTYPE_STR16) && check(PSTOK_PLUS)) {
            // For strings, we need to track the destination and concat sources
            // The left operand should have set lastStringSymbol
            if (!lastStringSymbol) {
                setError("String + requires a string variable on the left side");
                return left;
            }
            PLCScriptSymbol* destSym = lastStringSymbol;
            const char* strType = (destSym->type == PSTYPE_STR8) ? "str" : "str16";
            
            while (check(PSTOK_PLUS)) {
                nextToken();
                lastStringSymbol = nullptr;
                PLCScriptVarType right = parseMultiplicative();
                
                if (right == PSTYPE_STR8 || right == PSTYPE_STR16) {
                    // Concatenating another string variable
                    if (!lastStringSymbol) {
                        setError("String + requires string variables");
                        return left;
                    }
                    emit(strType);
                    emit(".concat ");
                    emit(destSym->address);
                    emit(" ");
                    emit(lastStringSymbol->address);
                    emit("\n");
                } else {
                    setError("Cannot concatenate non-string with string using +");
                    return left;
                }
            }
            return left;
        }
        
        while (check(PSTOK_PLUS) || check(PSTOK_MINUS)) {
            bool isAdd = check(PSTOK_PLUS);
            nextToken();
            // Set target type so literals match the left operand's type
            PLCScriptVarType savedTarget = targetType;
            targetType = left;
            (void)parseMultiplicative();
            targetType = savedTarget;
            emitBinaryOp(isAdd ? "add" : "sub", left);
        }
        
        return left;
    }
    
    PLCScriptVarType parseMultiplicative() {
        PLCScriptVarType left = parseUnary();
        
        while (check(PSTOK_STAR) || check(PSTOK_SLASH) || check(PSTOK_PERCENT)) {
            const char* op = nullptr;
            bool isDivOrMod = false;
            if (check(PSTOK_STAR)) op = "mul";
            else if (check(PSTOK_SLASH)) { op = "div"; isDivOrMod = true; }
            else { op = "mod"; isDivOrMod = true; }
            nextToken();
            
            // Check for constant division by zero
            if (isDivOrMod && check(PSTOK_INTEGER) && currentToken.intValue == 0) {
                addWarning("Division by constant zero");
            } else if (isDivOrMod && check(PSTOK_FLOAT) && currentToken.floatValue == 0.0) {
                addWarning("Division by constant zero");
            }
            
            // Set target type so literals match the left operand's type
            PLCScriptVarType savedTarget = targetType;
            targetType = left;
            (void)parseUnary();
            targetType = savedTarget;
            emitBinaryOp(op, left);
        }
        
        return left;
    }
    
    PLCScriptVarType parseUnary() {
        // Unary plus: +str parses string to f32 (like JavaScript)
        if (check(PSTOK_PLUS)) {
            nextToken();
            PLCScriptVarType type = parseUnary();
            // If string type, convert to number (default f32 like JS)
            if (isStringType(type) && hasPendingStringVar) {
                const char* strType = (pendingStringVarType == PSTYPE_STR8) ? "str" : "str16";
                emit(strType);
                emit(".to.f32 ");
                emit(pendingStringVarAddr);
                emit("\n");
                hasPendingStringVar = false;
                return PSTYPE_F32;
            }
            // For numeric types, unary + is a no-op
            return type;
        }
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
        
        // String literal - returns STR8 type, stores literal info for assignment
        if (check(PSTOK_STRING)) {
            // Save the string literal for later use in assignment
            int i = 0;
            while (currentToken.text[i] && i < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
                pendingStringLiteral[i] = currentToken.text[i];
                i++;
            }
            pendingStringLiteral[i] = '\0';
            pendingStringLiteralLen = currentToken.textLen;
            hasPendingStringLiteral = true;
            nextToken();
            return PSTYPE_STR8;  // String literals are str8 by default
        }
        
        // Template literal - returns STR8 type, stores literal info for assignment
        if (check(PSTOK_TEMPLATE)) {
            // Save the template literal for later use in assignment
            int i = 0;
            while (currentToken.text[i] && i < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
                pendingTemplateLiteral[i] = currentToken.text[i];
                i++;
            }
            pendingTemplateLiteral[i] = '\0';
            pendingTemplateLiteralLen = currentToken.textLen;
            hasPendingTemplateLiteral = true;
            nextToken();
            return PSTYPE_STR8;  // Template literals produce str8
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
        
        // Type casting: i32(expr), f32(str), etc.
        if (isTypeKeyword(currentToken.type)) {
            PLCScriptVarType castType = tokenTypeToVarType(currentToken.type);
            nextToken();
            
            if (!check(PSTOK_LPAREN)) {
                setError("Expected '(' after type name for type cast");
                return PSTYPE_VOID;
            }
            nextToken(); // consume '('
            
            // Parse the expression inside the cast
            PLCScriptVarType exprType = parseExpression();
            
            expect(PSTOK_RPAREN, "Expected ')' after type cast expression");
            
            // Check for string-to-number conversion
            if (isStringType(exprType) && isNumericType(castType)) {
                // String to number: need to use str.to.<type>
                // The expression should have left us with a string address context
                if (!hasPendingStringVar) {
                    setError("Cannot cast expression to number - expected string variable");
                    return PSTYPE_VOID;
                }
                
                const char* strType = (pendingStringVarType == PSTYPE_STR8) ? "str" : "str16";
                emit(strType);
                emit(".to.");
                emit(varTypeToPlcasm(castType));
                emit(" ");
                emit(pendingStringVarAddr);
                emit("\n");
                
                hasPendingStringVar = false;
                return castType;
            }
            
            // Regular numeric conversion using cvt
            if (exprType != castType && !isStringType(exprType) && !isStringType(castType)) {
                emit("cvt ");
                emit(varTypeToPlcasm(exprType));
                emit(" ");
                emit(varTypeToPlcasm(castType));
                emit("\n");
            }
            
            return castType;
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
            
            // Check for property access (e.g., my_timer.ET, my_counter.CV, myStruct.field)
            if (check(PSTOK_DOT)) {
                nextToken(); // consume dot
                
                // Expect property name
                if (!check(PSTOK_IDENTIFIER)) {
                    setError("Expected property name after '.'");
                    return PSTYPE_VOID;
                }
                
                char propName[32];
                int pi = 0;
                while (currentToken.text[pi] && pi < 31) {
                    propName[pi] = currentToken.text[pi];
                    pi++;
                }
                propName[pi] = '\0';
                nextToken();
                
                // Check for string property/method access (JS-style API)
                PLCScriptSymbol* strSym = findSymbol(name);
                if (strSym && (strSym->type == PSTYPE_STR8 || strSym->type == PSTYPE_STR16)) {
                    const char* strType = (strSym->type == PSTYPE_STR8) ? "str" : "str16";
                    
                    // Property: .length
                    if (strEq(propName, "length")) {
                        emit(strType);
                        emit(".len ");
                        emit(strSym->address);
                        emit("\n");
                        return PSTYPE_U16;
                    }
                    // Property: .capacity
                    if (strEq(propName, "capacity")) {
                        emit(strType);
                        emit(".cap ");
                        emit(strSym->address);
                        emit("\n");
                        return PSTYPE_U16;
                    }
                    
                    // Method: .clear()
                    if (strEq(propName, "clear")) {
                        if (!match(PSTOK_LPAREN)) {
                            setError("Expected '(' after clear");
                            return PSTYPE_VOID;
                        }
                        if (!match(PSTOK_RPAREN)) {
                            setError("Expected ')' after clear(");
                            return PSTYPE_VOID;
                        }
                        emit(strType);
                        emit(".clear ");
                        emit(strSym->address);
                        emit("\n");
                        return PSTYPE_VOID;
                    }
                    
                    // Method: .indexOf(needle) - returns i16 (-1 if not found)
                    if (strEq(propName, "indexOf")) {
                        if (!match(PSTOK_LPAREN)) {
                            setError("Expected '(' after indexOf");
                            return PSTYPE_VOID;
                        }
                        // Parse needle argument - must be a string variable
                        if (!check(PSTOK_IDENTIFIER)) {
                            setError("indexOf expects a string variable argument");
                            return PSTYPE_VOID;
                        }
                        char needleName[PLCSCRIPT_MAX_IDENTIFIER_LEN];
                        int ni = 0;
                        while (currentToken.text[ni] && ni < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
                            needleName[ni] = currentToken.text[ni];
                            ni++;
                        }
                        needleName[ni] = '\0';
                        nextToken();
                        
                        PLCScriptSymbol* needleSym = findSymbol(needleName);
                        if (!needleSym || (needleSym->type != PSTYPE_STR8 && needleSym->type != PSTYPE_STR16)) {
                            setError("indexOf argument must be a string variable");
                            return PSTYPE_VOID;
                        }
                        
                        if (!match(PSTOK_RPAREN)) {
                            setError("Expected ')' after indexOf argument");
                            return PSTYPE_VOID;
                        }
                        
                        // Emit str.find haystack needle -> pushes i16 index (-1 if not found)
                        emit(strType);
                        emit(".find ");
                        emit(strSym->address);
                        emit(" ");
                        emit(needleSym->address);
                        emit("\n");
                        return PSTYPE_I16;
                    }
                    
                    // Method: .includes(needle) - returns bool
                    if (strEq(propName, "includes")) {
                        if (!match(PSTOK_LPAREN)) {
                            setError("Expected '(' after includes");
                            return PSTYPE_VOID;
                        }
                        // Parse needle argument - must be a string variable
                        if (!check(PSTOK_IDENTIFIER)) {
                            setError("includes expects a string variable argument");
                            return PSTYPE_VOID;
                        }
                        char needleName[PLCSCRIPT_MAX_IDENTIFIER_LEN];
                        int ni = 0;
                        while (currentToken.text[ni] && ni < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
                            needleName[ni] = currentToken.text[ni];
                            ni++;
                        }
                        needleName[ni] = '\0';
                        nextToken();
                        
                        PLCScriptSymbol* needleSym = findSymbol(needleName);
                        if (!needleSym || (needleSym->type != PSTYPE_STR8 && needleSym->type != PSTYPE_STR16)) {
                            setError("includes argument must be a string variable");
                            return PSTYPE_VOID;
                        }
                        
                        if (!match(PSTOK_RPAREN)) {
                            setError("Expected ')' after includes argument");
                            return PSTYPE_VOID;
                        }
                        
                        // Emit str.find then compare >= 0 to get bool
                        emit(strType);
                        emit(".find ");
                        emit(strSym->address);
                        emit(" ");
                        emit(needleSym->address);
                        emit("\n");
                        // Convert result: -1 means not found (false), >= 0 means found (true)
                        emit("i16.const -1\n");
                        emit("i16.cmp_neq\n");
                        return PSTYPE_BOOL;
                    }
                    
                    // Method: .substr(from, to) or .substring(from, to) - returns string for assignment
                    // Usage: dest = src.substr(0, 5);
                    // This pushes from and (to-from) to stack and sets lastSubstrSymbol for assignment handling
                    if (strEq(propName, "substr") || strEq(propName, "substring")) {
                        if (!match(PSTOK_LPAREN)) {
                            setError("Expected '(' after substr");
                            return PSTYPE_VOID;
                        }
                        
                        // Parse 'from' argument
                        PLCScriptVarType fromType = parseExpression();
                        if (fromType == PSTYPE_VOID) {
                            setError("Invalid 'from' argument in substr");
                            return PSTYPE_VOID;
                        }
                        // Convert from to u16 if needed
                        if (fromType != PSTYPE_U16) {
                            emit("cvt ");
                            emit(varTypeToPlcasm(fromType));
                            emit(" u16\n");
                        }
                        
                        if (!match(PSTOK_COMMA)) {
                            setError("Expected ',' after 'from' argument in substr");
                            return PSTYPE_VOID;
                        }
                        
                        // Parse 'to' argument
                        PLCScriptVarType toType = parseExpression();
                        if (toType == PSTYPE_VOID) {
                            setError("Invalid 'to' argument in substr");
                            return PSTYPE_VOID;
                        }
                        // Convert to to u16 if needed
                        if (toType != PSTYPE_U16) {
                            emit("cvt ");
                            emit(varTypeToPlcasm(toType));
                            emit(" u16\n");
                        }
                        
                        if (!match(PSTOK_RPAREN)) {
                            setError("Expected ')' after substr arguments");
                            return PSTYPE_VOID;
                        }
                        
                        // Stack has: from, to
                        // STR_SUBSTR expects: start (u16), length (u16) where we pop length first, then start
                        // So we need to compute length = to - from
                        // Stack is: [from, to] (top of stack is 'to')
                        // We need to emit: swap, dup (copy from), rot up, sub (to - from)
                        // Result: [from, length] with length on top
                        
                        // Actually STR_SUBSTR pops: len first, then start
                        // So stack order should be: start, len (len on top)
                        // We have: from, to (to on top)
                        // Need: from, (to - from)
                        emit("u16.swap\n");    // to, from
                        emit("u16.copy\n");    // to, from, from
                        emit("u16.rot_up\n");  // from, from, to
                        emit("u16.swap\n");    // from, to, from
                        emit("u16.sub\n");     // from, (to - from) = from, length
                        
                        // Now stack has: from (start), length with length on top - correct for STR_SUBSTR
                        
                        // Set context for assignment handling
                        lastSubstrSymbol = strSym;
                        lastStringSymbol = strSym;  // Also set this for compatibility
                        hasSubstrArgs = true;
                        
                        return strSym->type;
                    }
                    
                    // Method: .charAt(index) - returns u8 char code
                    if (strEq(propName, "charAt")) {
                        if (!match(PSTOK_LPAREN)) {
                            setError("Expected '(' after charAt");
                            return PSTYPE_VOID;
                        }
                        // Parse index argument
                        PLCScriptVarType indexType = parseExpression();
                        if (!match(PSTOK_RPAREN)) {
                            setError("Expected ')' after charAt argument");
                            return PSTYPE_VOID;
                        }
                        // Convert index to u16 if needed
                        if (indexType != PSTYPE_U16) {
                            emit("cvt ");
                            emit(varTypeToPlcasm(indexType));
                            emit(" u16\n");
                        }
                        // Emit str.get - pops u16 index, pushes u8 char
                        emit(strType);
                        emit(".get ");
                        emit(strSym->address);
                        emit("\n");
                        return PSTYPE_U8;
                    }
                    
                    // Method: .concat(other) - appends other string to this one
                    if (strEq(propName, "concat")) {
                        if (!match(PSTOK_LPAREN)) {
                            setError("Expected '(' after concat");
                            return PSTYPE_VOID;
                        }
                        // Parse source argument - must be a string variable
                        if (!check(PSTOK_IDENTIFIER)) {
                            setError("concat expects a string variable argument");
                            return PSTYPE_VOID;
                        }
                        char srcName[PLCSCRIPT_MAX_IDENTIFIER_LEN];
                        int si = 0;
                        while (currentToken.text[si] && si < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
                            srcName[si] = currentToken.text[si];
                            si++;
                        }
                        srcName[si] = '\0';
                        nextToken();
                        
                        PLCScriptSymbol* srcSym = findSymbol(srcName);
                        if (!srcSym || (srcSym->type != PSTYPE_STR8 && srcSym->type != PSTYPE_STR16)) {
                            setError("concat argument must be a string variable");
                            return PSTYPE_VOID;
                        }
                        
                        if (!match(PSTOK_RPAREN)) {
                            setError("Expected ')' after concat argument");
                            return PSTYPE_VOID;
                        }
                        
                        // Emit str.concat dest src
                        emit(strType);
                        emit(".concat ");
                        emit(strSym->address);
                        emit(" ");
                        emit(srcSym->address);
                        emit("\n");
                        return PSTYPE_VOID;
                    }
                    
                    // Method: .startsWith(prefix) - returns bool
                    if (strEq(propName, "startsWith")) {
                        if (!match(PSTOK_LPAREN)) {
                            setError("Expected '(' after startsWith");
                            return PSTYPE_VOID;
                        }
                        // Parse prefix argument - must be a string variable
                        if (!check(PSTOK_IDENTIFIER)) {
                            setError("startsWith expects a string variable argument");
                            return PSTYPE_VOID;
                        }
                        char prefixName[PLCSCRIPT_MAX_IDENTIFIER_LEN];
                        int pi2 = 0;
                        while (currentToken.text[pi2] && pi2 < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
                            prefixName[pi2] = currentToken.text[pi2];
                            pi2++;
                        }
                        prefixName[pi2] = '\0';
                        nextToken();
                        
                        PLCScriptSymbol* prefixSym = findSymbol(prefixName);
                        if (!prefixSym || (prefixSym->type != PSTYPE_STR8 && prefixSym->type != PSTYPE_STR16)) {
                            setError("startsWith argument must be a string variable");
                            return PSTYPE_VOID;
                        }
                        
                        if (!match(PSTOK_RPAREN)) {
                            setError("Expected ')' after startsWith argument");
                            return PSTYPE_VOID;
                        }
                        
                        // startsWith: indexOf(prefix) == 0
                        emit(strType);
                        emit(".find ");
                        emit(strSym->address);
                        emit(" ");
                        emit(prefixSym->address);
                        emit("\n");
                        emit("i16.const 0\n");
                        emit("i16.cmp_eq\n");
                        return PSTYPE_BOOL;
                    }
                    
                    // Method: .endsWith(suffix) - returns bool
                    if (strEq(propName, "endsWith")) {
                        if (!match(PSTOK_LPAREN)) {
                            setError("Expected '(' after endsWith");
                            return PSTYPE_VOID;
                        }
                        // Parse suffix argument - must be a string variable
                        if (!check(PSTOK_IDENTIFIER)) {
                            setError("endsWith expects a string variable argument");
                            return PSTYPE_VOID;
                        }
                        char suffixName[PLCSCRIPT_MAX_IDENTIFIER_LEN];
                        int si2 = 0;
                        while (currentToken.text[si2] && si2 < PLCSCRIPT_MAX_IDENTIFIER_LEN - 1) {
                            suffixName[si2] = currentToken.text[si2];
                            si2++;
                        }
                        suffixName[si2] = '\0';
                        nextToken();
                        
                        PLCScriptSymbol* suffixSym = findSymbol(suffixName);
                        if (!suffixSym || (suffixSym->type != PSTYPE_STR8 && suffixSym->type != PSTYPE_STR16)) {
                            setError("endsWith argument must be a string variable");
                            return PSTYPE_VOID;
                        }
                        
                        if (!match(PSTOK_RPAREN)) {
                            setError("Expected ')' after endsWith argument");
                            return PSTYPE_VOID;
                        }
                        
                        // endsWith: indexOf(suffix) == (strLen - suffixLen)
                        // First get indexOf result
                        emit(strType);
                        emit(".find ");
                        emit(strSym->address);
                        emit(" ");
                        emit(suffixSym->address);
                        emit("\n");
                        // Get string length
                        emit(strType);
                        emit(".len ");
                        emit(strSym->address);
                        emit("\n");
                        // Get suffix length
                        const char* suffixType = (suffixSym->type == PSTYPE_STR8) ? "str" : "str16";
                        emit(suffixType);
                        emit(".len ");
                        emit(suffixSym->address);
                        emit("\n");
                        // Calculate expected position: strLen - suffixLen
                        emit("u16.sub\n");
                        // Convert to i16 for comparison
                        emit("cvt u16 i16\n");
                        // Compare: indexOf == expected position
                        emit("i16.cmp_eq\n");
                        return PSTYPE_BOOL;
                    }
                    
                    setError("Unknown string property or method");
                    return PSTYPE_VOID;
                }
                
                // Check for numeric variable methods (.toString, .toFixed)
                PLCScriptSymbol* numSym = findSymbol(name);
                if (numSym && isNumericType(numSym->type)) {
                    // Method: .toString(base = 10) - returns string for assignment
                    if (strEq(propName, "toString")) {
                        if (!match(PSTOK_LPAREN)) {
                            setError("Expected '(' after toString");
                            return PSTYPE_VOID;
                        }
                        
                        // Parse optional base argument (default 10)
                        u8 base = 10;
                        if (!check(PSTOK_RPAREN)) {
                            (void)parseExpression();  // Parse and discard base expression
                            // Base must be a constant, for now we evaluate at compile time
                            // For simplicity, we only support literal bases
                            // Pop the base value from the expression - since it's already emitted
                            // We need to drop it and use the literal value
                            emit("u8.drop\n");  // Drop the base value
                            // Use a fixed default for now - TODO: runtime base would need more work
                            base = 10;
                        }
                        
                        if (!match(PSTOK_RPAREN)) {
                            setError("Expected ')' after toString argument");
                            return PSTYPE_VOID;
                        }
                        
                        // Load the number value to stack
                        emitLoadFromAddress(numSym);
                        
                        // Set context for assignment to handle str.from.<type>
                        hasToStringCall = true;
                        toStringNumType = numSym->type;
                        toStringBase = base;
                        toStringDecimals = 0;  // Not used for integers
                        
                        // Return a string type to indicate this should be assigned to a string
                        return PSTYPE_STR8;
                    }
                    
                    // Method: .toFixed(decimals = 2) - for float types, returns string
                    if (strEq(propName, "toFixed")) {
                        if (numSym->type != PSTYPE_F32 && numSym->type != PSTYPE_F64) {
                            setError("toFixed() can only be used on float types");
                            return PSTYPE_VOID;
                        }
                        
                        if (!match(PSTOK_LPAREN)) {
                            setError("Expected '(' after toFixed");
                            return PSTYPE_VOID;
                        }
                        
                        // Parse optional decimals argument (default 2)
                        u8 decimals = 2;
                        if (!check(PSTOK_RPAREN)) {
                            // For now, require a literal for decimals
                            if (check(PSTOK_INTEGER)) {
                                decimals = (u8)currentToken.intValue;
                                if (decimals > 15) decimals = 15;
                                nextToken();
                            } else {
                                (void)parseExpression();  // Parse and discard decimals expression
                                emit("u8.drop\n");  // Drop - use default
                                decimals = 2;
                            }
                        }
                        
                        if (!match(PSTOK_RPAREN)) {
                            setError("Expected ')' after toFixed argument");
                            return PSTYPE_VOID;
                        }
                        
                        // Load the float value to stack
                        emitLoadFromAddress(numSym);
                        
                        // Set context for assignment to handle str.from.<type>
                        hasToStringCall = true;
                        toStringNumType = numSym->type;
                        toStringBase = 10;
                        toStringDecimals = decimals;
                        
                        return PSTYPE_STR8;
                    }
                }
                
                // First, check if this is a local struct variable
                PLCScriptSymbol* localSym = findSymbol(name);
                if (localSym && localSym->type == PSTYPE_STRUCT && localSym->structTypeIndex >= 0) {
                    // This is a struct variable - find the field
                    PLCScriptStructType* structType = &structTypes[localSym->structTypeIndex];
                    PLCScriptStructField* field = structType->findField(propName);
                    
                    if (!field) {
                        setError("Unknown struct field");
                        return PSTYPE_VOID;
                    }
                    
                    // Calculate field address: base_address + field_offset
                    // Build address string by combining symbol address with field offset
                    // NOTE: In PLCASM, addresses don't have type modifiers (D/W)
                    // The type is determined by the instruction (i32.load_from vs i16.load_from)
                    char fieldAddr[64];
                    int idx = 0;
                    
                    // Parse the base address to extract area and numeric part
                    char area = localSym->address[0];
                    int baseNum = 0;
                    int j = 1;
                    // Skip type modifier (W, D) if present in base address
                    if (localSym->address[1] == 'W' || localSym->address[1] == 'D' ||
                        localSym->address[1] == 'w' || localSym->address[1] == 'd') {
                        j = 2;
                    }
                    while (localSym->address[j] >= '0' && localSym->address[j] <= '9') {
                        baseNum = baseNum * 10 + (localSym->address[j] - '0');
                        j++;
                    }
                    
                    // Build the field address (no type modifier for PLCASM)
                    fieldAddr[idx++] = area;
                    
                    // Add the computed address (base + offset)
                    int newAddr = baseNum + field->offset;
                    char numBuf[16];
                    int ni = 0;
                    if (newAddr == 0) {
                        numBuf[ni++] = '0';
                    } else {
                        int temp = newAddr;
                        while (temp > 0) {
                            numBuf[ni++] = '0' + (temp % 10);
                            temp /= 10;
                        }
                    }
                    // Reverse the number
                    for (int r = ni - 1; r >= 0; r--) {
                        fieldAddr[idx++] = numBuf[r];
                    }
                    
                    // Handle bit fields
                    if (field->isBit) {
                        fieldAddr[idx++] = '.';
                        fieldAddr[idx++] = '0' + field->bitIndex;
                    }
                    fieldAddr[idx] = '\0';
                    
                    // Emit load instruction based on field type
                    if (field->isBit || field->type == PSTYPE_BOOL) {
                        emit("u8.readBit ");
                        emit(fieldAddr);
                        emit("\n");
                        return PSTYPE_BOOL;
                    } else {
                        emit(varTypeToPlcasm(field->type));
                        emit(".load_from ");
                        emit(fieldAddr);
                        emit("\n");
                        return field->type;
                    }
                }
                
                // Check if this is a shared symbol with a user-defined struct type
                SharedSymbol* structSharedSym = sharedSymbols.findSymbol(name);
                if (structSharedSym) {
                    UserStructType* userStruct = findUserStructType(structSharedSym->type);
                    if (userStruct) {
                        // Found a user-defined struct from shared symbols
                        const StructProperty* userField = userStruct->findField(propName);
                        
                        if (!userField) {
                            setError("Unknown struct field");
                            return PSTYPE_VOID;
                        }
                        
                        // Calculate field address: base_address + field_offset
                        char fieldAddr[64];
                        int idx = 0;
                        
                        // Determine area prefix from absolute address
                        u32 baseAddr = structSharedSym->address;
                        char area = 'M';
                        u32 relativeAddr = baseAddr;
                        
                        if (baseAddr >= plcasm_input_offset && baseAddr < plcasm_output_offset) {
                            area = 'X';
                            relativeAddr = baseAddr - plcasm_input_offset;
                        } else if (baseAddr >= plcasm_output_offset && baseAddr < plcasm_marker_offset) {
                            area = 'Y';
                            relativeAddr = baseAddr - plcasm_output_offset;
                        } else if (baseAddr >= plcasm_marker_offset && baseAddr < plcasm_timer_offset) {
                            area = 'M';
                            relativeAddr = baseAddr - plcasm_marker_offset;
                        } else if (baseAddr >= plcasm_timer_offset && baseAddr < plcasm_counter_offset) {
                            area = 'T';
                            relativeAddr = baseAddr - plcasm_timer_offset;
                        } else if (baseAddr >= plcasm_counter_offset) {
                            area = 'C';
                            relativeAddr = baseAddr - plcasm_counter_offset;
                        }
                        
                        fieldAddr[idx++] = area;
                        
                        // Add the computed address (relative + field offset)
                        u32 newAddr = relativeAddr + userField->offset;
                        char numBuf[16];
                        int ni = 0;
                        if (newAddr == 0) {
                            numBuf[ni++] = '0';
                        } else {
                            u32 temp = newAddr;
                            while (temp > 0) {
                                numBuf[ni++] = '0' + (temp % 10);
                                temp /= 10;
                            }
                        }
                        // Reverse the number
                        for (int r = ni - 1; r >= 0; r--) {
                            fieldAddr[idx++] = numBuf[r];
                        }
                        
                        // Handle bit fields (bit_pos 0-7 means bit type, 255 means non-bit)
                        bool isBitField = (userField->bit_pos < 8);
                        if (isBitField) {
                            fieldAddr[idx++] = '.';
                            fieldAddr[idx++] = '0' + userField->bit_pos;
                        }
                        fieldAddr[idx] = '\0';
                        
                        // Emit load instruction based on field type
                        if (isBitField || userField->type_size == 0) {
                            emit("u8.readBit ");
                            emit(fieldAddr);
                            emit("\n");
                            return PSTYPE_BOOL;
                        } else {
                            // type_size: 1=byte, 2=word, 4=dword, 8=qword
                            const char* typeStr = "u16";
                            PLCScriptVarType retType = PSTYPE_U16;
                            switch (userField->type_size) {
                                case 1: typeStr = "u8"; retType = PSTYPE_U8; break;
                                case 2: typeStr = "u16"; retType = PSTYPE_U16; break;
                                case 4: typeStr = "u32"; retType = PSTYPE_U32; break;
                                case 8: typeStr = "u64"; retType = PSTYPE_U64; break;
                            }
                            emit(typeStr);
                            emit(".load_from ");
                            emit(fieldAddr);
                            emit("\n");
                            return retType;
                        }
                    }
                }
                
                // Look up the symbol to determine if it's a timer or counter
                SharedSymbol* sharedSym = sharedSymbols.findSymbol(name);
                bool isTimer = false;
                bool isCounter = false;
                
                if (sharedSym) {
                    isTimer = SharedSymbolTable::isTimerType(sharedSym->type);
                    isCounter = SharedSymbolTable::isCounterType(sharedSym->type);
                }
                
                // Try to resolve as timer/counter property
                PLCScriptVarType resultType = PSTYPE_VOID;
                bool foundProperty = false;
                
                if (isTimer) {
                    for (int p = 0; p < timerPropertyCount; p++) {
                        if (strEqCI(propName, timerProperties[p].name)) {
                            foundProperty = true;
                            // Build address: symbol_addr.property
                            char fullAddr[64];
                            int idx = 0;
                            for (int k = 0; name[k] && idx < 60; k++) fullAddr[idx++] = name[k];
                            fullAddr[idx++] = '.';
                            for (int k = 0; propName[k] && idx < 63; k++) fullAddr[idx++] = propName[k];
                            fullAddr[idx] = '\0';
                            
                            // Determine type based on property
                            if (timerProperties[p].bit_pos != 0xFF) {
                                // Bit property (Q, IN, RUN)
                                emit("u8.readBit ");
                                emit(fullAddr);
                                emit("\n");
                                resultType = PSTYPE_BOOL;
                            } else {
                                // Value property (ET)
                                u8 ts = timerProperties[p].type_size;
                                if (ts == 4) {
                                    emit("u32.load_from ");
                                    resultType = PSTYPE_U32;
                                } else {
                                    emit("u8.load_from ");
                                    resultType = PSTYPE_U8;
                                }
                                emit(fullAddr);
                                emit("\n");
                            }
                            break;
                        }
                    }
                } else if (isCounter) {
                    for (int p = 0; p < counterPropertyCount; p++) {
                        if (strEqCI(propName, counterProperties[p].name)) {
                            foundProperty = true;
                            // Build address: symbol_addr.property
                            char fullAddr[64];
                            int idx = 0;
                            for (int k = 0; name[k] && idx < 60; k++) fullAddr[idx++] = name[k];
                            fullAddr[idx++] = '.';
                            for (int k = 0; propName[k] && idx < 63; k++) fullAddr[idx++] = propName[k];
                            fullAddr[idx] = '\0';
                            
                            // Determine type based on property
                            if (counterProperties[p].bit_pos != 0xFF) {
                                // Bit property (Q, IN)
                                emit("u8.readBit ");
                                emit(fullAddr);
                                emit("\n");
                                resultType = PSTYPE_BOOL;
                            } else {
                                // Value property (CV)
                                u8 ts = counterProperties[p].type_size;
                                if (ts == 4) {
                                    emit("u32.load_from ");
                                    resultType = PSTYPE_U32;
                                } else {
                                    emit("u8.load_from ");
                                    resultType = PSTYPE_U8;
                                }
                                emit(fullAddr);
                                emit("\n");
                            }
                            break;
                        }
                    }
                }
                
                if (!foundProperty) {
                    setError("Unknown property");
                    return PSTYPE_VOID;
                }
                
                return resultType;
            }
            
            // Check for array element access (e.g., arr[0], arr[i])
            if (check(PSTOK_LBRACKET)) {
                nextToken(); // consume '['
                
                // Find the array symbol
                PLCScriptSymbol* arrSym = findSymbol(name);
                PLCScriptSymbol tempArrSym;
                
                if (!arrSym) {
                    SharedSymbol* sharedSym = sharedSymbols.findSymbol(name);
                    if (sharedSym) {
                        memset(&tempArrSym, 0, sizeof(tempArrSym));
                        populateFromSharedSymbol(&tempArrSym, sharedSym);
                        arrSym = &tempArrSym;
                    }
                }
                
                if (!arrSym) {
                    setError("Undefined array variable");
                    return PSTYPE_VOID;
                }
                
                if (!arrSym->isArray()) {
                    setError("Variable is not an array");
                    return PSTYPE_VOID;
                }
                
                // Parse index expression
                // Check if it's a constant index (static access) or variable (dynamic access)
                bool isStaticIndex = check(PSTOK_INTEGER);
                int64_t staticIndex = 0;
                
                if (isStaticIndex) {
                    staticIndex = currentToken.intValue;
                    nextToken();
                    
                    // Bounds checking for static index
                    if (staticIndex < 0 || staticIndex >= arrSym->array_size) {
                        setError("Array index out of bounds");
                        return PSTYPE_VOID;
                    }
                } else {
                    // Dynamic index - parse expression and use it for address calculation
                    PLCScriptVarType indexType = parseExpression();
                    
                    // Convert to u32 for address calculation if needed
                    if (indexType != PSTYPE_U32 && indexType != PSTYPE_I32) {
                        emit("cvt ");
                        emit(varTypeToPlcasm(indexType));
                        emit(" u32\n");
                    }
                }
                
                if (!match(PSTOK_RBRACKET)) {
                    setError("Expected ']' after array index");
                    return PSTYPE_VOID;
                }
                
                // Get element type and size
                PLCScriptVarType elemType = arrSym->type;
                u8 elemSize = (u8)getTypeSize(elemType);
                
                if (isStaticIndex) {
                    // Static index: compute address at compile time
                    u32 offset = (u32)staticIndex * elemSize;
                    char addr[64];
                    int idx = 0;
                    
                    // Use raw address prefix '#' since memoryOffset is absolute byte address
                    // This prevents PLCASM from adding area offset again
                    addr[idx++] = '#';
                    u32 newAddr = arrSym->memoryOffset + offset;
                    char numBuf[16];
                    int ni = 0;
                    if (newAddr == 0) numBuf[ni++] = '0';
                    else {
                        u32 temp = newAddr;
                        while (temp > 0) { numBuf[ni++] = '0' + (temp % 10); temp /= 10; }
                    }
                    for (int r = ni - 1; r >= 0; r--) addr[idx++] = numBuf[r];
                    addr[idx] = '\0';
                    
                    // Emit load instruction
                    emit(varTypeToPlcasm(elemType));
                    emit(".load_from ");
                    emit(addr);
                    emit("\n");
                } else {
                    // Dynamic index: emit instructions to compute address at runtime
                    // Stack now has: index (u32)
                    // Need to: base_address + (index * elem_size)
                    
                    // Convert index to ptr for pointer arithmetic
                    emit("cvt u32 ptr\n");
                    
                    // Multiply index by element size
                    if (elemSize > 1) {
                        emit("ptr.const ");
                        char sizeBuf[16];
                        int si = 0;
                        u8 s = elemSize;
                        if (s == 0) sizeBuf[si++] = '0';
                        else {
                            char temp[16];
                            int ti = 0;
                            while (s > 0) { temp[ti++] = '0' + (s % 10); s /= 10; }
                            while (ti > 0) sizeBuf[si++] = temp[--ti];
                        }
                        sizeBuf[si] = '\0';
                        emit(sizeBuf);
                        emit("\nptr.mul\n");
                    }
                    
                    // Add base address
                    emit("ptr.const ");
                    char baseBuf[16];
                    int bi = 0;
                    u32 base = arrSym->memoryOffset;
                    if (base == 0) baseBuf[bi++] = '0';
                    else {
                        char temp[16];
                        int ti = 0;
                        while (base > 0) { temp[ti++] = '0' + (base % 10); base /= 10; }
                        while (ti > 0) baseBuf[bi++] = temp[--ti];
                    }
                    baseBuf[bi] = '\0';
                    emit(baseBuf);
                    emit("\nptr.add\n");
                    
                    // Use dynamic load instruction with ptr address on stack
                    emit(varTypeToPlcasm(elemType));
                    emit(".load\n");
                }
                
                return elemType;
            }
            
            // Variable reference - first check local symbol table
            PLCScriptSymbol* sym = findSymbol(name);
            
            // If not found locally, check shared symbol table (project symbols)
            PLCScriptSymbol tempSym;
            if (!sym) {
                SharedSymbol* sharedSym = sharedSymbols.findSymbol(name);
                if (sharedSym) {
                    memset(&tempSym, 0, sizeof(tempSym));
                    populateFromSharedSymbol(&tempSym, sharedSym);
                    sym = &tempSym;
                }
            }
            
            if (!sym) {
                setError("Undefined variable");
                return PSTYPE_VOID;
            }
            
            // For string types, track the symbol for string operations (no value to load)
            if (sym->type == PSTYPE_STR8 || sym->type == PSTYPE_STR16) {
                lastStringSymbol = sym;
                // For local symbols we need to store a reference, for tempSym we need to copy
                if (sym == &tempSym) {
                    // Copy the temp symbol to a more permanent location
                    static PLCScriptSymbol stringSymbolStorage;
                    memcpy(&stringSymbolStorage, &tempSym, sizeof(PLCScriptSymbol));
                    lastStringSymbol = &stringSymbolStorage;
                }
                // Also set pending string var context for type casting like i32(str)
                hasPendingStringVar = true;
                pendingStringVarType = sym->type;
                int ai = 0;
                while (sym->address[ai] && ai < 31) {
                    pendingStringVarAddr[ai] = sym->address[ai];
                    ai++;
                }
                pendingStringVarAddr[ai] = '\0';
                return sym->type;
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
        
        // Check for user-defined function
        PLCScriptFunction* func = findFunction(name);
        if (func) {
            return parseUserFunctionCall(func);
        }
        
        // Unknown function
        setError("Unknown function");
        return PSTYPE_VOID;
    }
    
    // Parse and emit a call to a user-defined function
    PLCScriptVarType parseUserFunctionCall(PLCScriptFunction* func) {
        // Parse arguments and push them onto the stack (left to right)
        int argCount = 0;
        while (!check(PSTOK_RPAREN) && !check(PSTOK_EOF) && !hasError) {
            if (argCount >= func->paramCount) {
                setError("Too many arguments");
                return PSTYPE_VOID;
            }
            
            // Set target type hint for argument
            PLCScriptVarType savedTarget = targetType;
            targetType = func->params[argCount].type;
            
            PLCScriptVarType argType = parseExpression();
            
            targetType = savedTarget;
            
            // Convert argument type if necessary
            PLCScriptVarType expectedType = func->params[argCount].type;
            if (argType != expectedType) {
                emit("cvt ");
                emit(varTypeToPlcasm(argType));
                emit(" ");
                emitLine(varTypeToPlcasm(expectedType));
            }
            
            argCount++;
            
            if (!check(PSTOK_RPAREN)) {
                if (!match(PSTOK_COMMA)) {
                    setError("Expected ',' or ')' in argument list");
                    return PSTYPE_VOID;
                }
            }
        }
        
        if (argCount < func->paramCount) {
            setError("Too few arguments");
            return PSTYPE_VOID;
        }
        
        if (!match(PSTOK_RPAREN)) {
            setError("Expected ')' after arguments");
            return PSTYPE_VOID;
        }
        
        // Call the function
        emit("call ");
        emit(func->entryLabel);
        emit("\n");
        
        return func->returnType;
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
