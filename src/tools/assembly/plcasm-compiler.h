// plcasm-compiler.h - 2022-12-11
//
// Copyright (c) 2022 J.Vovk
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

#include "wasm/wasm.h"
#include "shared-symbols.h"

#include "./../runtime-types.h"

#ifndef PLCRUNTIME_NUM_OF_SYSTEMS
#define PLCRUNTIME_NUM_OF_SYSTEMS 64
#endif // PLCRUNTIME_NUM_OF_SYSTEMS

#ifndef PLCRUNTIME_NUM_OF_INPUTS
#define PLCRUNTIME_NUM_OF_INPUTS 64
#endif // PLCRUNTIME_NUM_OF_INPUTS

#ifndef PLCRUNTIME_NUM_OF_OUTPUTS
#define PLCRUNTIME_NUM_OF_OUTPUTS 64
#endif // PLCRUNTIME_NUM_OF_OUTPUTS

#ifndef PLCRUNTIME_NUM_OF_MARKERS
#define PLCRUNTIME_NUM_OF_MARKERS 256
#endif // PLCRUNTIME_NUM_OF_MARKERS

#ifndef PLCRUNTIME_SYSTEM_OFFSET
#define PLCRUNTIME_SYSTEM_OFFSET 0
#endif // PLCRUNTIME_SYSTEM_OFFSET

#ifndef PLCRUNTIME_INPUT_OFFSET
#define PLCRUNTIME_INPUT_OFFSET (PLCRUNTIME_SYSTEM_OFFSET + PLCRUNTIME_NUM_OF_SYSTEMS)
#endif // PLCRUNTIME_INPUT_OFFSET

#ifndef PLCRUNTIME_OUTPUT_OFFSET
#define PLCRUNTIME_OUTPUT_OFFSET (PLCRUNTIME_INPUT_OFFSET + PLCRUNTIME_NUM_OF_INPUTS)
#endif // PLCRUNTIME_OUTPUT_OFFSET

#ifndef PLCRUNTIME_MARKER_OFFSET
#define PLCRUNTIME_MARKER_OFFSET (PLCRUNTIME_OUTPUT_OFFSET + PLCRUNTIME_NUM_OF_OUTPUTS)
#endif // PLCRUNTIME_MARKER_OFFSET

#ifndef PLCRUNTIME_TIMER_OFFSET
#define PLCRUNTIME_TIMER_OFFSET (PLCRUNTIME_MARKER_OFFSET + PLCRUNTIME_NUM_OF_MARKERS)
#endif // PLCRUNTIME_TIMER_OFFSET

#ifndef PLCRUNTIME_COUNTER_OFFSET
#define PLCRUNTIME_COUNTER_OFFSET (PLCRUNTIME_TIMER_OFFSET + (PLCRUNTIME_NUM_OF_TIMERS * PLCRUNTIME_TIMER_STRUCT_SIZE))
#endif // PLCRUNTIME_COUNTER_OFFSET

static u32 plcasm_system_offset = PLCRUNTIME_SYSTEM_OFFSET;
static u32 plcasm_input_offset = PLCRUNTIME_INPUT_OFFSET;
static u32 plcasm_output_offset = PLCRUNTIME_OUTPUT_OFFSET;
static u32 plcasm_marker_offset = PLCRUNTIME_MARKER_OFFSET;
static u32 plcasm_timer_offset = PLCRUNTIME_TIMER_OFFSET;
static u32 plcasm_counter_offset = PLCRUNTIME_COUNTER_OFFSET;

#define MAX_ASSEMBLY_STRING_SIZE 64535
#define MAX_NUM_OF_TOKENS 10000

// ################################################################################################
// ### Example (0.1 + 0.2) * -1 = -0.3
// ################################################################################################
// .plcasm file assembly example:
/*
    # This is a comment
    f32.const 0.1
    f32.const 0.2
    f32.add
    f32.const -1
    f32.mul
*/

// hard coded compilation process in C++ (part of the code)
/*
    // ...
    program.push_f32(0.1);
    program.push_f32(0.2);
    program.push(ADD, type_f32);
    program.push_f32(-1);
    program.push(MUL, type_f32);
    // ...
*/

// .hex file bytecode example: (formatted in HEX)
/*
0A 3D CC CC CD 0A 3E 4C CC CD 20 0A 0A BF 80 00 00 22 0A
*/

// ################################################################################################
// We are trying to compile the .plaasm file into the .hex file
// The .plaasm file is a text file that contains the assembly code of the program
// The .hex file is a binary file that contains the bytecode of the program

// ################################################################################################
// The following is the flow of the compilation process:
// - reading the assembly
// - tokenization
// - parsing and compilation
// - linking
// - writing the bytecode

// ################################################################################################

// Reading the assembly
// The assembly is read from the .plaasm file but in this case it is passed as a char array

// Tokenization
// The assembly is tokenized into tokens and the tokens are stored in a vector

// Parsing and compilation
// The tokens are parsed and compiled into bytecode and the bytecode is stored in a vector
// While parsing, a lookup table is created to store the labels and their addresses

// Linking
// The bytecode is linked to correct the addresses of the labels

// ################################################################################################

// Note: DO NOT USE 'strlen' IN GLOBAL SCOPE, IT CAN CAUSE MEMORY LEAKS IN THIS WASM ENVIRONMENT
size_t string_len(const char* s) {
    size_t len = 0;
    while (*s++ != '\0') len++;
    return len;
}

char* string_chr(const char* s, int c) {
    while (*s != (char) c)
        if (!*s++)
            return 0;
    return (char*) s;
}

char* string_copy(char* destination, const char* source) {
    char* start = destination;
    while (*source != '\0') {
        *destination = *source;
        destination++;
        source++;
    }
    *destination = '\0';
    return start;
}

void fill(char c, int count) {
    for (int i = 0; i < count; i++) Serial.print(c);
}

// ################################################################################################

// Part 2: Tokenization
struct StringView {
    char* data;
    int length;
    int print() const {
        for (int i = 0; i < length; i++) printf("%c", data[i]);
        return length;
    }
    // Array operator get/set
    char& operator[](int index) {
        return data[index];
    }
    char operator[](int index) const {
        return data[index];
    }
    bool equals(const char* b);
    bool endsWith(const char* b);
    bool startsWith(const char* b);

    // Equality operator
    bool operator==(const char* b) { return equals(b); }
};

enum TokenType {
    TOKEN_UNKNOWN = -1,
    TOKEN_BOOLEAN = 0,
    TOKEN_INTEGER,
    TOKEN_REAL,
    TOKEN_STRING,
    TOKEN_CONST_BOOLEAN,
    TOKEN_CONST_INTEGER,
    TOKEN_CONST_REAL,
    TOKEN_CONST_STRING,
    TOKEN_OPERATOR,
    TOKEN_KEYWORD,
    TOKEN_LABEL,
};

enum ValueType {
    VAL_UNKNOWN = -1,
    VAL_BOOLEAN = 0,
    VAL_INTEGER,
    VAL_REAL,
    VAL_STRING,
};

int printTokenType(TokenType type) {
    if (type == TOKEN_BOOLEAN) return printf("boolean");
    if (type == TOKEN_INTEGER) return printf("integer");
    if (type == TOKEN_REAL) return printf("real");
    if (type == TOKEN_STRING) return printf("string");
    if (type == TOKEN_CONST_BOOLEAN) return printf("const_boolean");
    if (type == TOKEN_CONST_INTEGER) return printf("const_integer");
    if (type == TOKEN_CONST_REAL) return printf("const_real");
    if (type == TOKEN_CONST_STRING) return printf("const_string");
    if (type == TOKEN_OPERATOR) return printf("operator");
    if (type == TOKEN_KEYWORD) return printf("keyword");
    if (type == TOKEN_LABEL) return printf("label");
    return printf("unknown");
}

struct Token {
    StringView string;
    int length;
    int line;
    int column;
    bool value_bool;
    int value_int;
    float value_float;
    TokenType type;
    // Array operator get/set
    char& operator[](int index) {
        return string[index];
    }
    char operator[](int index) const {
        return string[index];
    }
    int print() {
        if (type == TOKEN_BOOLEAN) return printf("%s", value_bool ? "true" : "false");
        if (type == TOKEN_INTEGER) return printf("%d", value_int);
        if (type == TOKEN_REAL) return printf("%f", value_float);
        if (type == TOKEN_STRING) return printf("'") + string.print() + printf("'");
        return printf("\"") + string.print() + printf("\"");
    }
    void highlight(const char* assembly_string) {
        // jump test
        //      ~~~~
        int row = 1;
        const char* c = assembly_string;
        while (*c != '\0') {
            if (*c == '\n') row++;
            c++;
            if (row == line) {
                fill(' ', 8);
                while (*c != '\n' && *c != '\0') {
                    Serial.print(*c);
                    length++;
                    c++;
                };
                Serial.println();
                fill(' ', 8);
                fill(' ', column - 1);
                fill('~', string.length);
                Serial.println();
            }
        }

    }
    void parse();
    bool equals(const char* b);
    bool endsWith(const char* b);
    bool startsWith(const char* b);
    bool includes(const char* b);
    // Equality operator
    bool operator==(const char* b) { return equals(b); }
};

bool str_cmp(StringView a, StringView b) {
    if (a.length != b.length) return false;
    for (int i = 0; i < a.length; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

bool str_cmp(StringView a, const char* b) {
    int b_length = string_len(b);
    if (a.length != b_length) return false;
    for (int i = 0; i < a.length; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

bool str_cmp(Token a, StringView b) {
    if (a.length != b.length) return false;
    for (int i = 0; i < a.length; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

bool str_cmp(Token a, const char* b) {
    int b_length = string_len(b);
    if (a.length != b_length) return false;
    for (int i = 0; i < a.length; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

bool str_cmp(Token a, Token b) {
    if (a.length != b.length) return false;
    for (int i = 0; i < a.length; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

bool str_includes(StringView a, const char* b) {
    int b_length = string_len(b);
    if (a.length < b_length) return false;
    int matching_chars = 0;
    for (int i = 0; i < a.length; i++) {
        if (a[i] == b[matching_chars]) matching_chars++;
        else matching_chars = 0;
        if (matching_chars == b_length) return true;
    }
    return false;
}
bool str_includes(Token a, const char* b) {
    int b_length = string_len(b);
    if (a.length < b_length) return false;
    int matching_chars = 0;
    for (int i = 0; i < a.length; i++) {
        if (a[i] == b[matching_chars]) matching_chars++;
        else matching_chars = 0;
        if (matching_chars == b_length) return true;
    }
    return false;
}

bool _endsWith(StringView a, StringView b) {
    if (a.length < b.length) return false;
    for (int i = 0; i < b.length; i++) {
        if (a[a.length - b.length + i] != b[i]) return false;
    }
    return true;
}

bool _endsWith(StringView a, const char* b) {
    int b_length = string_len(b);
    if (a.length < b_length) return false;
    for (int i = 0; i < b_length; i++) {
        if (a[a.length - b_length + i] != b[i]) return false;
    }
    return true;
}

bool _endsWith(Token a, StringView b) {
    if (a.length < b.length) return false;
    for (int i = 0; i < b.length; i++) {
        if (a[a.length - b.length + i] != b[i]) return false;
    }
    return true;
}

bool _endsWith(Token a, const char* b) {
    int b_length = string_len(b);
    if (a.length < b_length) return false;
    for (int i = 0; i < b_length; i++) {
        if (a[a.length - b_length + i] != b[i]) return false;
    }
    return true;
}

bool _startsWith(StringView a, StringView b) {
    if (a.length < b.length) return false;
    for (int i = 0; i < b.length; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

bool _startsWith(StringView a, const char* b) {
    int b_length = string_len(b);
    if (a.length < b_length) return false;
    for (int i = 0; i < b_length; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

bool _startsWith(Token a, StringView b) {
    if (a.length < b.length) return false;
    for (int i = 0; i < b.length; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

bool _startsWith(Token a, const char* b) {
    int b_length = string_len(b);
    if (a.length < b_length) return false;
    for (int i = 0; i < b_length; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}



bool StringView::equals(const char* b) { return str_cmp(*this, b); }
bool StringView::endsWith(const char* b) { return _endsWith(*this, b); }
bool StringView::startsWith(const char* b) { return _startsWith(*this, b); }

bool Token::equals(const char* b) { return str_cmp(*this, b); }
bool Token::endsWith(const char* b) { return _endsWith(*this, b); }
bool Token::startsWith(const char* b) { return _startsWith(*this, b); }
bool Token::includes(const char* b) { return str_includes(*this, b); }


bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isBoolean(StringView string, bool& output) {
    if (string.length == 0) return false;
    if (string == "true") {
        output = true;
        return true;
    }
    if (string == "false") {
        output = false;
        return true;
    }
    if (string.length == 1) {
        if (string[0] == '1') {
            output = true;
            return true;
        }
        if (string[0] == '0') {
            output = false;
            return true;
        }
    }
    return false;
}

bool isInteger(StringView string, int& output) {
    if (string.length == 0) return false;
    int number = 0;
    bool is_negative = false;
    int i = 0;
    if (string[0] == '-') {
        is_negative = true;
        i++;
    }
    // Check for hex prefix 0x or 0X
    if (string.length > i + 2 && string[i] == '0' && (string[i + 1] == 'x' || string[i + 1] == 'X')) {
        i += 2;
        for (; i < string.length && string[i] != '\0'; i++) {
            char c = string[i];
            int digit = 0;
            if (c >= '0' && c <= '9') digit = c - '0';
            else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
            else return false;
            number = number * 16 + digit;
        }
        if (is_negative) number = -number;
        output = number;
        return true;
    }
    // Check for binary prefix 0b or 0B
    if (string.length > i + 2 && string[i] == '0' && (string[i + 1] == 'b' || string[i + 1] == 'B')) {
        i += 2;
        for (; i < string.length && string[i] != '\0'; i++) {
            char c = string[i];
            if (c != '0' && c != '1') return false;
            number = number * 2 + (c - '0');
        }
        if (is_negative) number = -number;
        output = number;
        return true;
    }
    // Decimal number
    for (; i < string.length && string[i] != '\0'; i++) {
        if (!isDigit(string[i])) {
            return false;
        }
        number = number * 10 + (string[i] - '0');
    }
    if (is_negative) number = -number;
    output = number;
    return true;
}

bool isReal(StringView string, float& output) {
    if (string.length == 0) return false;
    float number = 0;
    bool is_negative = false;
    bool dot = false;
    int decimal_places = 0;
    int i = 0;
    if (string[0] == '-') {
        is_negative = true;
        i++;
    }
    for (; i < string.length && string[i] != '\0'; i++) {
        if (string[i] == '.') {
            if (dot) return false;
            dot = true;
            continue;
        }
        if (!isDigit(string[i])) return false;
        if (dot) decimal_places++;
        number = number * 10.0 + float(string[i] - '0');
    }
    for (int i = 0; i < decimal_places; i++) {
        number *= 0.1;
    }
    if (is_negative) number = -number;
    output = number;
    return true;
}

void Token::parse() {
    this->type = TOKEN_UNKNOWN;
    if (this->length == 0) return;
    if (this->length == 1) {
        char c = this->string[0];
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '&' || c == '|' || c == '^' || c == '~' || c == '!' || c == '<' || c == '>' || c == '?' || c == ':' || c == ',' || c == ';' || c == '[' || c == ']' || c == '{' || c == '}' || c == '\'' || c == '"' || c == '`' || c == '\\') {
            this->type = TOKEN_OPERATOR;
            return;
        }
        // Single letter identifiers (T, C, etc.) are valid keywords
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_') {
            this->type = TOKEN_KEYWORD;
            return;
        }
    }
    if (string == "==" || string == "!=" || string == "<>" || string == "<=" || string == ">=" || string == "&&" || string == "||") {
        this->type = TOKEN_OPERATOR;
        return;
    }
    if (isInteger(this->string, this->value_int)) {
        this->type = TOKEN_INTEGER;
        return;
    }
    if (isReal(this->string, this->value_float)) {
        this->type = TOKEN_REAL;
        return;
    }
    if (isBoolean(this->string, this->value_bool)) {
        this->type = TOKEN_BOOLEAN;
        return;
    }
    this->type = TOKEN_KEYWORD;
}

struct LUT_label {
    StringView string;
    int address;
};

struct LUT_const {
    int address;
    StringView string;
    ValueType type;
    bool value_bool;
    int value_int;
    float value_float;
    StringView value_string;
};

struct ProgramLine {
    u8 code[16];
    int index;
    int size;
    Token* refToken;
};

// ################################################################################################
// ### IR (Intermediate Representation) for Front-End Editor ###
// ################################################################################################
// This IR is exported after compilation for live monitoring and constant editing.
// Each IR entry corresponds to one compiled instruction.

// IR Entry Flags
#define IR_FLAG_NONE          0x00
#define IR_FLAG_READ          0x01  // Instruction reads from memory
#define IR_FLAG_WRITE         0x02  // Instruction writes to memory
#define IR_FLAG_CONST         0x04  // Instruction uses an embedded constant
#define IR_FLAG_JUMP          0x08  // Instruction is a jump/call
#define IR_FLAG_TIMER         0x10  // Instruction is a timer
#define IR_FLAG_LABEL_TARGET  0x20  // This address is a jump target (label)
#define IR_FLAG_EDITABLE      0x40  // Constant can be edited in-place

// IR Operand Types
enum IR_OperandType : u8 {
    IR_OP_NONE = 0,
    IR_OP_BOOL,
    IR_OP_I8,
    IR_OP_U8,
    IR_OP_I16,
    IR_OP_U16,
    IR_OP_I32,
    IR_OP_U32,
    IR_OP_I64,
    IR_OP_U64,
    IR_OP_F32,
    IR_OP_F64,
    IR_OP_PTR,       // Pointer/address
    IR_OP_LABEL,     // Jump target label
};

// Single IR entry (fixed 48 bytes for easy memory access)
struct IR_Entry {
    // Location info (12 bytes)
    u32 bytecode_offset;    // Offset in bytecode where this instruction starts
    u16 source_line;        // Source line number (1-based)
    u16 source_column;      // Source column number (1-based)
    u8  bytecode_size;      // Size of this instruction in bytes
    u8  opcode;             // The instruction opcode
    u8  flags;              // IR_FLAG_* combination
    u8  operand_count;      // Number of operands (0-3)

    // Operand info (32 bytes - up to 3 operands)
    // Each operand: type (1) + value (8) + bytecode_pos (1) = 10 bytes, padded to 10
    struct {
        u8  type;           // IR_OperandType
        u8  bytecode_pos;   // Position within instruction where this value starts
        u8  _pad[2];        // Padding for alignment
        union {
            u8  val_u8;
            i8  val_i8;
            u16 val_u16;
            i16 val_i16;
            u32 val_u32;
            i32 val_i32;
            u64 val_u64;
            i64 val_i64;
            float val_f32;
            double val_f64;
        };
    } operands[3];          // Max 3 operands per instruction

    // Reserved (4 bytes)
    u8 _reserved[4];
};

#define MAX_IR_ENTRIES 2048

// Global config lists
const char* illegal_keywords [] = { "const", "var", "i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f32", "f64", "bool", "string", "true", "false", "if", "else", "while", "for", "do", "break", "continue", "return", "function" };
const int illegal_keywords_count = sizeof(illegal_keywords) / sizeof(illegal_keywords[0]);

const char lex_ignored [] = { ' ', ';', ',', '\t', '\r', '\n', '\0' };
const char lex_dividers [] = { '(', ')', '=', '+', '-', '*', '/', '%', '&', '|', '^', '~', '!', '<', '>', '?', ':', ';', '[', ']', '{', '}', '\'', '"', '`', '\\', '\0' };

const char* data_type_keywords [] = { "i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f32", "f64", "bool", "string", "bit", "byte", "ptr", "pointer", "*" };
const int data_type_keywords_count = sizeof(data_type_keywords) / sizeof(data_type_keywords[0]);

struct Symbol {
    StringView name;
    StringView type;
    char address_str[32]; // Original address string (e.g. "M10.0") - Added for better debug output
    u32 address;        // Memory address or byte offset
    u8 bit;             // Bit position (0-7) or 255 if not a bit type
    bool is_bit;        // True if this is a bit address (X0.1, Y0.1, etc.)
    int line;           // Line number where symbol is defined
    int column;         // Column number where symbol is defined
    u8 type_size;       // Size in bytes (1 for u8/i8, 2 for u16/i16, 4 for u32/i32/f32, etc.)

    void print() const {
        printf("Symbol: '");
        name.print();
        printf("' type=");
        type.print();
        if (address_str[0] != '\0') {
             printf(" addr=%s", address_str);
        } else if (is_bit) {
            printf(" addr=%u.%u", address, bit);
        } else {
            printf(" addr=%u (size=%u)", address, type_size);
        }
        printf(" [line %d:%d]", line, column);
    }
};


class PLCASMCompiler {
public:
    int built_bytecode_length = 0;
    u8 built_bytecode_checksum = 0;

    char assembly_string[MAX_ASSEMBLY_STRING_SIZE] = { 0 };

    Token tokens[MAX_NUM_OF_TOKENS];
    int token_count = 0;
    int token_count_temp = 0;

    int line = 1;
    int column = 1;

    struct LUT_label LUT_labels[MAX_NUM_OF_TOKENS];
    int LUT_label_count = 0;

    struct LUT_const LUT_consts[MAX_NUM_OF_TOKENS];
    int LUT_const_count = 0;

    struct Symbol symbols[MAX_NUM_OF_TOKENS];
    int symbol_count = 0;
    int base_symbol_count = 0; // Symbols added externally (before compilation)

    u8 built_bytecode[PLCRUNTIME_MAX_PROGRAM_SIZE];

    ProgramLine programLines[PLCRUNTIME_MAX_PROGRAM_SIZE];
    int programLineCount = 0;

    bool last_token_is_exit = false;
    int num_of_compile_runs = 0;
    bool emit_warnings = false;

    // IR (Intermediate Representation) output
    IR_Entry ir_entries[MAX_IR_ENTRIES];
    int ir_entry_count = 0;

#define MAX_DOWNLOADED_PROGRAM_SIZE 64535
    u8 downloaded_program[MAX_DOWNLOADED_PROGRAM_SIZE];
    int downloaded_program_size = 0;

    PLCASMCompiler() {

        memset(tokens, 0, sizeof(tokens));
        memset(LUT_labels, 0, sizeof(LUT_labels));
        memset(LUT_consts, 0, sizeof(LUT_consts));
        memset(symbols, 0, sizeof(symbols));
        memset(built_bytecode, 0, sizeof(built_bytecode));
        memset(programLines, 0, sizeof(programLines));
        memset(downloaded_program, 0, sizeof(downloaded_program));
        memset(ir_entries, 0, sizeof(ir_entries));
        // Default assembly string
        char default_asm [] = "DEFAULT_ASM_STRING_MARKER";
        /*
                char default_asm[] = R"(
        // This is a comment
            f32.const 0.1
            f32.const 0.2
            f32.add
            f32.const -1
            f32.mul
        )";
        */
        set_assembly_string(default_asm);
    }

    void set_assembly_string(char* new_assembly_string) {
        const int size = string_len(new_assembly_string);
        if (size == 0) {
            printf("Error: input assembly is empty.\n");
            return;
        }
        if (size > MAX_ASSEMBLY_STRING_SIZE) {
            printf("Error: input assembly is too long (%d characters). Max size is %d characters.\n", size, MAX_ASSEMBLY_STRING_SIZE);
            return;
        }
        string_copy(assembly_string, new_assembly_string);
    }

    // Symbol validation and parsing functions
    bool isValidSymbolName(const StringView& name) {
        if (name.length == 0) return false;
        // First character must be letter or underscore
        char first = name[0];
        if (!((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z') || first == '_')) {
            return false;
        }
        // Rest can be alphanumeric or underscore
        for (int i = 1; i < name.length; i++) {
            char c = name[i];
            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') || c == '_')) {
                return false;
            }
        }
        // Check against illegal keywords
        for (int i = 0; i < illegal_keywords_count; i++) {
            if (str_cmp(name, illegal_keywords[i])) return false;
        }
        return true;
    }

    u8 getTypeSize(const StringView& type) {
        if (str_cmp(type, "i8") || str_cmp(type, "u8") || str_cmp(type, "byte")) return 1;
        if (str_cmp(type, "i16") || str_cmp(type, "u16")) return 2;
        if (str_cmp(type, "i32") || str_cmp(type, "u32") || str_cmp(type, "f32")) return 4;
        if (str_cmp(type, "i64") || str_cmp(type, "u64") || str_cmp(type, "f64")) return 8;
        if (str_cmp(type, "bit") || str_cmp(type, "bool")) return 0; // bit type has no size
        if (str_cmp(type, "ptr") || str_cmp(type, "pointer") || str_cmp(type, "*")) return 4;
        return 0; // unknown type
    }

    // ========================================================================
    // Structure Property Access Resolution
    // ========================================================================
    // Resolves property access like T0.Q, my_timer.ET, C0.CV into memory addresses.
    // Returns true on error, false on success.
    //
    // Input format: "T0.Q", "my_timer.ET", "C0.CV"
    // For direct T/C addresses: base + (index * struct_size) + property_offset
    // For symbol references: symbol_base_address + property_offset

    // Check if a string contains a property access (has a dot followed by a letter)
    bool hasPropertyAccess(const StringView& str) {
        for (int i = 0; i < str.length - 1; i++) {
            if (str.data[i] == '.') {
                char next = str.data[i + 1];
                // Property access if next char is a letter (not a digit for bit access)
                if ((next >= 'A' && next <= 'Z') || (next >= 'a' && next <= 'z')) {
                    return true;
                }
            }
        }
        return false;
    }

    // Parse property access from a string like "T0.Q" or "my_timer.ET"
    // Returns: false = success, true = error
    // On success: address, bit, is_bit, type_size are set
    bool parsePropertyAccess(const StringView& str, int& address, int& bit, bool& is_bit, u8& type_size) {
        // Find the dot
        int dot_pos = -1;
        for (int i = 0; i < str.length; i++) {
            if (str.data[i] == '.') {
                dot_pos = i;
                break;
            }
        }
        if (dot_pos < 0 || dot_pos >= str.length - 1) return true;

        // Extract base and property parts
        char base_buf[64] = {0};
        char prop_buf[16] = {0};
        
        for (int i = 0; i < dot_pos && i < 63; i++) {
            base_buf[i] = str.data[i];
        }
        base_buf[dot_pos] = '\0';
        
        int prop_len = str.length - dot_pos - 1;
        for (int i = 0; i < prop_len && i < 15; i++) {
            prop_buf[i] = str.data[dot_pos + 1 + i];
        }
        prop_buf[prop_len] = '\0';

        // Determine if base is a direct address (T0, C0) or a symbol name
        char prefix = base_buf[0];
        bool is_timer_prefix = (prefix == 'T' || prefix == 't');
        bool is_counter_prefix = (prefix == 'C' || prefix == 'c');
        
        u32 base_address = 0;
        bool is_timer = false;
        bool is_counter = false;

        if ((is_timer_prefix || is_counter_prefix) && base_buf[1] >= '0' && base_buf[1] <= '9') {
            // Direct address like T0, C5
            int index = 0;
            for (int i = 1; base_buf[i] >= '0' && base_buf[i] <= '9'; i++) {
                index = index * 10 + (base_buf[i] - '0');
            }
            if (is_timer_prefix) {
                base_address = plcasm_timer_offset + (index * PLCRUNTIME_TIMER_STRUCT_SIZE);
                is_timer = true;
            } else {
                base_address = plcasm_counter_offset + (index * PLCRUNTIME_COUNTER_STRUCT_SIZE);
                is_counter = true;
            }
        } else {
            // Symbol name - look it up
            StringView base_sv = { base_buf, (int)string_len(base_buf) };
            Symbol* sym = findSymbol(base_sv);
            if (!sym) {
                // Also check shared symbols
                SharedSymbol* shared = findSharedSymbol(base_buf);
                if (!shared) return true; // Symbol not found
                
                // Determine type from shared symbol
                if (SharedSymbolTable::isTimerType(shared->type)) {
                    is_timer = true;
                    // For timer symbols, address is the timer index
                    base_address = plcasm_timer_offset + (shared->address * PLCRUNTIME_TIMER_STRUCT_SIZE);
                } else if (SharedSymbolTable::isCounterType(shared->type)) {
                    is_counter = true;
                    base_address = plcasm_counter_offset + (shared->address * PLCRUNTIME_COUNTER_STRUCT_SIZE);
                } else {
                    // Check user-defined struct types
                    UserStructType* ust = findUserStructType(shared->type);
                    if (ust) {
                        PropertyResolution res = resolveUserStructProperty(ust, shared->address, prop_buf);
                        if (!res.success) return true;
                        address = res.address;
                        bit = res.is_bit ? res.bit_pos : 0;
                        is_bit = res.is_bit;
                        type_size = res.type_size;
                        return false;
                    }
                    return true; // Not a structured type
                }
            } else {
                // Check local symbol type
                // Timer/counter symbols store the index, not the memory address
                if (str_cmp(sym->type, "timer") || str_cmp(sym->type, "ton") || 
                    str_cmp(sym->type, "tof") || str_cmp(sym->type, "tp")) {
                    is_timer = true;
                    base_address = plcasm_timer_offset + (sym->address * PLCRUNTIME_TIMER_STRUCT_SIZE);
                } else if (str_cmp(sym->type, "counter") || str_cmp(sym->type, "ctu") || 
                           str_cmp(sym->type, "ctd") || str_cmp(sym->type, "ctud")) {
                    is_counter = true;
                    base_address = plcasm_counter_offset + (sym->address * PLCRUNTIME_COUNTER_STRUCT_SIZE);
                } else {
                    return true; // Not a timer/counter type
                }
            }
        }

        // Resolve the property
        PropertyResolution res;
        if (is_timer) {
            res = resolveTimerProperty(base_address, prop_buf);
        } else if (is_counter) {
            res = resolveCounterProperty(base_address, prop_buf);
        } else {
            return true;
        }

        if (!res.success) return true;

        address = res.address;
        bit = res.is_bit ? res.bit_pos : 0;
        is_bit = res.is_bit;
        type_size = res.type_size;
        return false;
    }

    bool parseSymbolAddress(const StringView& addr_str, u32& address, u8& bit, bool& is_bit, Token& error_token) {
        // Parse addresses like: 130, M4, X0.1, Y0.1, etc.
        is_bit = false;
        bit = 255;

        // Check for bit notation (e.g., X0.1, Y0.1, M4.5)
        int dot_pos = -1;
        for (int i = 0; i < addr_str.length; i++) {
            if (addr_str[i] == '.') {
                dot_pos = i;
                break;
            }
        }

        char prefix = addr_str[0];
        u32 base_offset = 0;
        int num_start = 0;

        // Determine memory area and offset
        if (prefix == 'S' || prefix == 's') {
            base_offset = plcasm_system_offset;
            num_start = 1;
        } else if (prefix == 'X' || prefix == 'x') {
            base_offset = plcasm_input_offset;
            num_start = 1;
        } else if (prefix == 'Y' || prefix == 'y') {
            base_offset = plcasm_output_offset;
            num_start = 1;
        } else if (prefix == 'M' || prefix == 'm') {
            base_offset = plcasm_marker_offset;
            num_start = 1;
        } else if (prefix == 'T' || prefix == 't') {
            base_offset = plcasm_timer_offset;
            num_start = 1;
        } else if (prefix == 'C' || prefix == 'c') {
            base_offset = plcasm_counter_offset;
            num_start = 1;
        } else if (prefix >= '0' && prefix <= '9') {
            // Direct numeric address
            num_start = 0;
            base_offset = 0;
        } else {
            Serial.print(F("Error: invalid address prefix '"));
            Serial.print(prefix);
            Serial.print(F("' in symbol at "));
            Serial.print(error_token.line);
            Serial.print(F(":"));
            Serial.println(error_token.column);
            return true;
        }

        // Parse the numeric part
        int byte_addr = 0;
        int end_pos = dot_pos >= 0 ? dot_pos : addr_str.length;

        for (int i = num_start; i < end_pos; i++) {
            char c = addr_str[i];
            if (c < '0' || c > '9') {
                Serial.print(F("Error: invalid digit in address at "));
                Serial.print(error_token.line);
                Serial.print(F(":"));
                Serial.println(error_token.column);
                return true;
            }
            byte_addr = byte_addr * 10 + (c - '0');
        }

        address = base_offset + byte_addr;

        // Parse bit position if present
        if (dot_pos >= 0) {
            is_bit = true;
            if (dot_pos + 1 >= addr_str.length) {
                Serial.print(F("Error: missing bit position after '.' at "));
                Serial.print(error_token.line);
                Serial.print(F(":"));
                Serial.println(error_token.column);
                return true;
            }
            char bit_char = addr_str[dot_pos + 1];
            if (bit_char < '0' || bit_char > '7') {
                Serial.print(F("Error: bit position must be 0-7 at "));
                Serial.print(error_token.line);
                Serial.print(F(":"));
                Serial.println(error_token.column);
                return true;
            }
            bit = bit_char - '0';
        }

        return false;
    }

    bool checkSymbolOverlap(int current_idx) {
        Symbol& sym = symbols[current_idx];
        bool found_overlap = false;

        for (int i = 0; i < current_idx; i++) {
            Symbol& other = symbols[i];

            // Skip if either is a bit type (bits can share the same byte)
            if (sym.is_bit && other.is_bit) {
                // Check if same byte and same bit
                if (sym.address == other.address && sym.bit == other.bit) {
                    if (emit_warnings) {
                        Serial.print(F("Warning: symbol '"));
                        sym.name.print();
                        Serial.print(F("' overlaps with '"));
                        other.name.print();
                        Serial.print(F("' at address "));
                        Serial.print(sym.address);
                        Serial.print(F("."));
                        Serial.print(sym.bit);
                        Serial.print(F(" [line "));
                        Serial.print(sym.line);
                        Serial.print(F(":"));
                        Serial.print(sym.column);
                        Serial.println(F("]"));
                    }
                    found_overlap = true;
                }
                continue;
            }

            // Check byte-level overlap
            u32 sym_start = sym.address;
            u32 sym_end = sym.address + (sym.type_size > 0 ? sym.type_size - 1 : 0);
            u32 other_start = other.address;
            u32 other_end = other.address + (other.type_size > 0 ? other.type_size - 1 : 0);

            bool overlaps = !(sym_end < other_start || sym_start > other_end);

            if (overlaps) {
                if (emit_warnings) {
                    Serial.print(F("Warning: symbol '"));
                    sym.name.print();
                    Serial.print(F("' (addr "));
                    Serial.print(sym_start);
                    if (sym.type_size > 1) {
                        Serial.print(F("-"));
                        Serial.print(sym_end);
                    }
                    Serial.print(F(") overlaps with '"));
                    other.name.print();
                    Serial.print(F("' (addr "));
                    Serial.print(other_start);
                    if (other.type_size > 1) {
                        Serial.print(F("-"));
                        Serial.print(other_end);
                    }
                    Serial.print(F(") [line "));
                    Serial.print(sym.line);
                    Serial.print(F(":"));
                    Serial.print(sym.column);
                    Serial.println(F("]"));
                }
                found_overlap = true;
            }
        }

        return found_overlap;
    }

    bool parseSymbolDefinition(int token_idx) {
        // Symbol format: $$ name | type | address [| comment]
        if (token_idx + 5 >= token_count) {
            Serial.print(F("Error: incomplete symbol definition at line "));
            Serial.print(tokens[token_idx].line);
            Serial.print(F(":"));
            Serial.println(tokens[token_idx].column);
            return true;
        }

        Token& dollars = tokens[token_idx];
        Token& name = tokens[token_idx + 1];
        Token& pipe1 = tokens[token_idx + 2];
        Token& type = tokens[token_idx + 3];
        Token& pipe2 = tokens[token_idx + 4];
        Token& address = tokens[token_idx + 5];

        // Validate syntax
        if (!str_cmp(dollars.string, "$$")) {
            Serial.print(F("Error: symbol definition must start with '$$' at line "));
            Serial.print(dollars.line);
            Serial.print(F(":"));
            Serial.println(dollars.column);
            return true;
        }

        if (!str_cmp(pipe1.string, "|") || !str_cmp(pipe2.string, "|")) {
            Serial.print(F("Error: symbol parts must be separated by '|' at line "));
            Serial.print(name.line);
            Serial.print(F(":"));
            Serial.println(name.column);
            return true;
        }

        // Validate symbol name
        if (!isValidSymbolName(name.string)) {
            Serial.print(F("Error: invalid symbol name '"));
            name.string.print();
            Serial.print(F("' at line "));
            Serial.print(name.line);
            Serial.print(F(":"));
            Serial.println(name.column);
            return true;
        }

        // Check for duplicate symbol names
        for (int i = 0; i < symbol_count; i++) {
            if (str_cmp(symbols[i].name, name.string)) {
                Serial.print(F("Error: duplicate symbol '"));
                name.string.print();
                Serial.print(F("' at line "));
                Serial.print(name.line);
                Serial.print(F(" (first defined at line "));
                Serial.print(symbols[i].line);
                Serial.println(F(")"));
                return true;
            }
        }

        // Validate type
        bool valid_type = false;
        for (int i = 0; i < data_type_keywords_count; i++) {
            if (str_cmp(type.string, data_type_keywords[i])) {
                valid_type = true;
                break;
            }
        }
        if (!valid_type) {
            Serial.print(F("Error: invalid type '"));
            type.string.print();
            Serial.print(F("' for symbol at line "));
            Serial.print(type.line);
            Serial.print(F(":"));
            Serial.println(type.column);
            return true;
        }

        // Parse address
        u32 addr;
        u8 bit_pos;
        bool is_bit;
        if (parseSymbolAddress(address.string, addr, bit_pos, is_bit, address)) {
            return true;
        }

        // Get type size
        u8 type_size = getTypeSize(type.string);

        // Validate bit type
        if ((str_cmp(type.string, "bit") || str_cmp(type.string, "bool")) && !is_bit) {
            Serial.print(F("Error: bit/bool type requires bit address (e.g., X0.1) at line "));
            Serial.print(address.line);
            Serial.print(F(":"));
            Serial.println(address.column);
            return true;
        }

        if (!(str_cmp(type.string, "bit") || str_cmp(type.string, "bool")) && is_bit) {
            Serial.print(F("Error: non-bit type cannot use bit address at line "));
            Serial.print(address.line);
            Serial.print(F(":"));
            Serial.println(address.column);
            return true;
        }

        // Check bounds
        u32 max_addr = PLCRUNTIME_MAX_MEMORY_SIZE;
        if (is_bit) {
            if (addr >= max_addr) {
                Serial.print(F("Error: address "));
                Serial.print(addr);
                Serial.print(F(" out of bounds (max "));
                Serial.print(max_addr - 1);
                Serial.print(F(") at line "));
                Serial.print(address.line);
                Serial.print(F(":"));
                Serial.println(address.column);
                return true;
            }
        } else {
            if (addr + type_size > max_addr) {
                Serial.print(F("Error: address range "));
                Serial.print(addr);
                Serial.print(F("-"));
                Serial.print(addr + type_size - 1);
                Serial.print(F(" out of bounds (max "));
                Serial.print(max_addr - 1);
                Serial.print(F(") at line "));
                Serial.print(address.line);
                Serial.print(F(":"));
                Serial.println(address.column);
                return true;
            }
        }

        // Add symbol to table
        if (symbol_count >= MAX_NUM_OF_TOKENS) {
            Serial.print(F("Error: too many symbols (max "));
            Serial.print(MAX_NUM_OF_TOKENS);
            Serial.println(F(")"));
            return true;
        }

        Symbol& sym = symbols[symbol_count];
        sym.name = name.string;
        sym.type = type.string;
        sym.address = addr;
        sym.bit = bit_pos;
        sym.is_bit = is_bit;
        sym.line = name.line;
        sym.column = name.column;
        sym.type_size = type_size;

        symbol_count++;

        return false;
    }

    Symbol* findSymbol(const StringView& name) {
        for (int i = 0; i < symbol_count; i++) {
            if (str_cmp(symbols[i].name, name)) {
                return &symbols[i];
            }
        }
        return nullptr;
    }

    bool add_label(Token& token, int address) {
        for (int i = 0; i < LUT_label_count; i++) {
            if (str_cmp(LUT_labels[i].string, token.string)) {
                Serial.print(F("Error: duplicate label. Label ")); token.print(); Serial.print(F(" already exists at ")); Serial.print(token.line); Serial.print(F(":")); Serial.println(token.column);
                return true;
            }
        }
        for (int i = 0; i < illegal_keywords_count; i++) {
            if (str_cmp(token.string, illegal_keywords[i])) {
                Serial.print(F("Error: illegal label. Label ")); token.print(); Serial.print(F(" is illegal at ")); Serial.print(token.line); Serial.print(F(":")); Serial.println(token.column);
                return true;
            }
        }
        if (LUT_label_count >= MAX_NUM_OF_TOKENS) {
            Serial.print(F("Error: too many labels. Max number of labels is")); Serial.println(MAX_NUM_OF_TOKENS);
            return true;
        }
        LUT_labels[LUT_label_count].string = token.string;
        LUT_labels[LUT_label_count].address = -1;
        LUT_label_count++;
        return false;
    }

    bool add_const(Token& keyword, Token& value, int address) {
        for (int i = 0; i < LUT_const_count; i++) {
            if (str_cmp(LUT_consts[i].string, keyword.string)) {
                Serial.print(F("Error: duplicate const. Const ")); keyword.print(); Serial.print(F(" already exists at ")); Serial.print(keyword.line); Serial.print(F(":")); Serial.println(keyword.column);
                return true;
            }
        }
        for (int i = 0; i < illegal_keywords_count; i++) {
            if (str_cmp(keyword.string, illegal_keywords[i])) {
                Serial.print(F("Error: illegal const. Const ")); keyword.print(); Serial.print(F(" is illegal at ")); Serial.print(keyword.line); Serial.print(F(":")); Serial.println(keyword.column);
                return true;
            }
        }
        if (LUT_const_count >= MAX_NUM_OF_TOKENS) {
            Serial.print(F("Error: too many consts. Max number of consts is")); Serial.println(MAX_NUM_OF_TOKENS);
            return true;
        }
        LUT_consts[LUT_const_count].string = keyword.string;
        LUT_consts[LUT_const_count].address = address;
        LUT_consts[LUT_const_count].value_string = value.string;
        if (value.type == TOKEN_BOOLEAN) {
            LUT_consts[LUT_const_count].type = VAL_BOOLEAN;
            LUT_consts[LUT_const_count].value_bool = value.value_bool;
        }
        if (value.type == TOKEN_INTEGER) {
            LUT_consts[LUT_const_count].type = VAL_INTEGER;
            LUT_consts[LUT_const_count].value_int = value.value_int;
        }
        if (value.type == TOKEN_REAL) {
            LUT_consts[LUT_const_count].type = VAL_REAL;
            LUT_consts[LUT_const_count].value_float = value.value_float;
        }
        if (value.type == TOKEN_STRING) {
            LUT_consts[LUT_const_count].type = VAL_STRING;
            LUT_consts[LUT_const_count].value_string = value.string;
        }
        LUT_const_count++;
        return false;
    }

    bool add_token(char* string, int length) {
        last_token_is_exit = false;
        if (token_count_temp >= MAX_NUM_OF_TOKENS) {
            Serial.print(F("Error: too many tokens. Max number of tokens is")); Serial.println(MAX_NUM_OF_TOKENS);
            return true;
        }
        Token& token = tokens[token_count_temp];
        token.string.data = string;
        token.string.length = length;
        token.length = length;
        token.line = line;
        token.column = column;
        token.parse();
        if (token.type == TOKEN_UNKNOWN) {
            Serial.print(F("Error: unknown token ")); token.print(); Serial.print(F(" at ")); Serial.print(line); Serial.print(F(":")); Serial.println(column);
            return true;
        }
        last_token_is_exit = token == "exit";
        bool isNumber = token.type == TOKEN_INTEGER || token.type == TOKEN_REAL;
        if (token_count_temp >= 2 && isNumber) { // handle negative numbers
            Token& p1_token = tokens[token_count_temp - 1];
            Token& p2_token = tokens[token_count_temp - 2];
            TokenType p1_type = p1_token.type;
            TokenType p2_type = p2_token.type;
            // If [keyword , - , number] then change to [keyword , -number]

            bool skip = false;
            if (p1_type == TOKEN_OPERATOR && p2_type == TOKEN_KEYWORD && p1_token == "-") {
                if (token.type == TOKEN_INTEGER) {
                    p1_token.value_int = -token.value_int;
                    p1_token.type = TOKEN_INTEGER;
                    skip = true;
                }
                if (token.type == TOKEN_REAL) {
                    p1_token.value_float = -token.value_float;
                    p1_token.type = TOKEN_REAL;
                    skip = true;
                }
            }
            // Check if [keyword="const" , keyword , bool|int|real] then change to []
            if (!skip) token_count_temp++; // Work one step back

            if (token_count_temp >= 3) {
                Token& p1_token = tokens[token_count_temp - 1];
                Token& p2_token = tokens[token_count_temp - 2];
                Token& p3_token = tokens[token_count_temp - 3];
                TokenType p1_type = p1_token.type;
                TokenType p2_type = p2_token.type;
                if (str_cmp(p3_token, "const") && p2_type == TOKEN_KEYWORD && (p1_type == TOKEN_BOOLEAN || p1_type == TOKEN_INTEGER || p1_type == TOKEN_REAL)) {
                    token_count_temp -= 3;
                    bool error = add_const(p2_token, p1_token, token_count_temp - 1);
                    if (error) return error;
                }
            }
            return false;
        }
        if (token_count_temp >= 2) {
            Token& p1_token = tokens[token_count_temp - 1];
            Token& p2_token = tokens[token_count_temp - 2];
            // If [' , * , '] then change to [string]
            // Before: tokens = [..., ', content, '] with token_count_temp pointing past content
            // After:  tokens = [..., content_as_STRING] with token_count_temp pointing past the STRING
            if (token == "'" && p2_token == "'") {
                p2_token.type = TOKEN_STRING;
                p2_token.string = p1_token.string;
                p2_token.length = p1_token.length;
                token_count_temp--;  // Remove content token, keep the converted STRING token
                return false;  // Don't add the closing quote
            }
        }
        if (token_count_temp >= 1) {
            Token& p1_token = tokens[token_count_temp - 1];
            TokenType p1_type = p1_token.type;
            // If [keyword , :] then change to [label]
            if (p1_type == TOKEN_KEYWORD) {
                if (token.type == TOKEN_OPERATOR && token == ":") {
                    p1_token.type = TOKEN_LABEL;
                    bool error = add_label(p1_token, token_count_temp - 1);
                    if (error) return error;
                    return false;
                }
            }
        }
        token_count_temp++;
        return false;
    }

    bool add_token_optional(char* string, int length) {
        if (length == 0) return false;
        return add_token(string, length);
    }

    bool tokenize() {
        token_count = 0; // Fix: Reset token count
        LUT_label_count = 0;
        LUT_const_count = 0;
        char* token_start = assembly_string;
        int token_length = 0;
        int assembly_string_length = string_len(assembly_string);

        bool in_string = false;
        bool error = false;
        for (int i = 0; i < assembly_string_length; i++) {
            char c = assembly_string[i];

            if (in_string && !(c == '\'' || c == '\n')) {
                if (token_length == 0) token_start = assembly_string + i;
                token_length++;
                continue;
            }


            // c == "/*"
            bool hasNext = i + 1 < assembly_string_length;
            char c1 = hasNext ? assembly_string[i + 1] : '\0';
            bool blockComment = hasNext && c == '/' && c1 == '*';
            if (blockComment) {
                error = add_token_optional(token_start, token_length);
                if (error) return error;
                bool found = false;
                i += 2;
                while (i < assembly_string_length - 1 && !found) {
                    i++;
                    c = assembly_string[i];
                    if (c == '\n') {
                        line++;
                        column = 1;
                    }
                    if (c != '*') continue;
                    hasNext = i + 1 < assembly_string_length;
                    c1 = hasNext ? assembly_string[i + 1] : '\0';
                    found = c == '*' && c1 == '/';
                }
                if (!found || i >= assembly_string_length - 1) {
                    Serial.print(F("Error: unexpected end of file. Expected '*/' at ")); Serial.print(line); Serial.print(F(":")); Serial.println(column);
                    return true;
                }
                i++;
                token_start = assembly_string + i + 1;
                token_length = 0;
                continue;
            }

            // c == //
            if (c == '/') {
                // Check for double slash
                bool hasNext = i + 1 < assembly_string_length;
                if (hasNext && assembly_string[i + 1] == '/') {
                    error = add_token_optional(token_start, token_length);
                    if (error) return error;
                    while (i < assembly_string_length && assembly_string[i] != '\n') i++;
                    token_start = assembly_string + i + 1;
                    token_length = 0;
                    line++;
                    column = 1;
                    continue;
                }
            }
            // c == $$ (symbol definition)
            if (c == '$' && hasNext && c1 == '$') {
                error = add_token_optional(token_start, token_length);
                if (error) return error;
                column += token_length;
                error = add_token(assembly_string + i, 2); // Add "$$" as single token
                if (error) return error;
                i++; // Skip the second $
                token_start = assembly_string + i + 1;
                token_length = 0;
                column += 2;
                continue;
            }
            // c == \n
            if (c == '\n') {
                error = add_token_optional(token_start, token_length);
                if (error) return error;
                token_start = assembly_string + i + 1;
                token_length = 0;
                line++;
                column = 1;
                continue;
            }
            // c == ' ' || c == '\t' || c == '\r'
            if (string_chr(lex_ignored, c) != NULL) {
                error = add_token_optional(token_start, token_length);
                if (error) return error;
                column += token_length + 1;
                token_length = 0;
                continue;
            }
            // c == '(' || c == ')' || c == '=' || c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '&' || c == '|' || c == '^' || c == '~' || c == '!' || c == '<' || c == '>' || c == '?' || c == ':' || c == ',' || c == ';' || c == '[' || c == ']' || c == '{' || c == '}' || c == '\'' || c == '"' || c == '`' || c == '\\'
            if (string_chr(lex_dividers, c) != NULL) {
                error = add_token_optional(token_start, token_length);
                if (error) return error;
                column += token_length;
                if (c == '\'') in_string = !in_string;
                if (c != '"' && c != '=') error = add_token(assembly_string + i, 1);
                if (error) return error;
                token_length = 0;
                column++;
                continue;
            }
            if (token_length == 0) token_start = assembly_string + i;
            token_length++;
        }
        error = add_token_optional(token_start, token_length);
        if (error) return error;
        if (!last_token_is_exit) {
            error = add_token((char*) "exit", 4);
            if (error) return error;
        }
        token_count = token_count_temp;
        token_count_temp = 0;
        line = 1;
        column = 1;
        return false;
    }

    bool typeFromToken(Token& token, u8& type) {
        if (token.type == TOKEN_KEYWORD) {
            // "u8" = type_u8
            for (int i = 0; i < data_type_keywords_count; i++) {
                if (token.startsWith(data_type_keywords[i])) {
                    switch (i) {
                        case 0:  type = type_i8; break;
                        case 1:  type = type_i16; break;
                        case 2:  type = type_i32; break;
                        case 3:  type = type_i64; break;
                        case 4:  type = type_u8; break;
                        case 5:  type = type_u16; break;
                        case 6:  type = type_u32; break;
                        case 7:  type = type_u64; break;
                        case 8:  type = type_f32; break;
                        case 9:  type = type_f64; break;
                        case 10: type = type_bool; break;
                        case 11: return false; // type = type_string; break; // TODO: Add support for strings
                        case 12: type = type_bool; break;
                        case 13: type = type_u8; break;
                        case 14: type = type_pointer; break;
                        case 15: type = type_pointer; break;
                        case 16: type = type_pointer; break;
                        default: return true;
                    }
                    return false;
                }
            }
        }
        return true;
    }

    int typeSize(u8& type) {
        switch ((PLCRuntimeInstructionSet) type) {
            case type_bool: case type_i8: case type_u8: return 8;
            case type_i16: case type_u16: return 16;
            case type_i32: case type_u32: case type_f32: return 32;
            case type_i64: case type_u64: case type_f64: return 64;
            case type_pointer: return sizeof(MY_PTR_t) * 8;
            default: return -1;
        }
    }

    bool boolFromToken(Token& token, bool& output) {
        if (token.type == TOKEN_INTEGER) {
            output = token.value_int != 0;
            return false;
        }
        if (token.type == TOKEN_REAL) {
            output = token.value_float != 0;
            return false;
        }
        if (token.type == TOKEN_BOOLEAN) {
            output = token.value_bool;
            return false;
        }
        if (token.type == TOKEN_KEYWORD) {
            for (int i = 0; i < LUT_const_count; i++) {
                LUT_const& c = LUT_consts[i];
                if (str_cmp(token, c.string)) {
                    if (c.type == VAL_BOOLEAN) {
                        token.string = c.string;
                        output = c.value_bool;
                        return false;
                    }
                    if (c.type == VAL_INTEGER) {
                        token.string = c.string;
                        output = c.value_int != 0;
                        return false;
                    }
                    if (c.type == VAL_REAL) {
                        token.string = c.string;
                        output = c.value_float != 0;
                        return false;
                    }
                    return true;
                }
            }
        }
        return true;
    }

    bool intFromToken(Token& token, int& output) {
        if (token.type == TOKEN_INTEGER) {
            output = token.value_int;
            return false;
        }
        if (token.type == TOKEN_REAL) {
            output = token.value_float;
            return false;
        }
        if (token.type == TOKEN_KEYWORD) {
            for (int i = 0; i < LUT_const_count; i++) {
                LUT_const& c = LUT_consts[i];
                if (str_cmp(token, c.string)) {
                    if (c.type == VAL_BOOLEAN) {
                        token.string = c.string;
                        output = c.value_bool;
                        return false;
                    }
                    if (c.type == VAL_INTEGER) {
                        token.string = c.string;
                        output = c.value_int;
                        return false;
                    }
                    if (c.type == VAL_REAL) {
                        token.string = c.string;
                        output = c.value_float;
                        return false;
                    }
                    return true;
                }
            }
        }
        return true;
    }

    bool memoryOffsetFromPrefix(char prefix, int& offset, int& multiplier) {
        multiplier = 1; // Default multiplier for byte-addressable areas
        switch (prefix) {
            case 'S': case 's': offset = (int) plcasm_system_offset; return true;
            case 'X': case 'x': offset = (int) plcasm_input_offset; return true;
            case 'Y': case 'y': offset = (int) plcasm_output_offset; return true;
            case 'M': case 'm': offset = (int) plcasm_marker_offset; return true;
            case 'T': case 't': offset = (int) plcasm_timer_offset; multiplier = PLCRUNTIME_TIMER_STRUCT_SIZE; return true;
            case 'C': case 'c': offset = (int) plcasm_counter_offset; multiplier = PLCRUNTIME_COUNTER_STRUCT_SIZE; return true;
            default: return false;
        }
    }
    
    // Legacy overload for backward compatibility
    bool memoryOffsetFromPrefix(char prefix, int& offset) {
        int multiplier = 1;
        return memoryOffsetFromPrefix(prefix, offset, multiplier);
    }

    bool memorySizeFromPrefix(char prefix, int& size, const char*& name) {
        switch (prefix) {
            case 'S': case 's': size = PLCRUNTIME_NUM_OF_SYSTEMS; name = "systems"; return true;
            case 'X': case 'x': size = PLCRUNTIME_NUM_OF_INPUTS; name = "inputs"; return true;
            case 'Y': case 'y': size = PLCRUNTIME_NUM_OF_OUTPUTS; name = "outputs"; return true;
            case 'M': case 'm': size = PLCRUNTIME_NUM_OF_MARKERS; name = "markers"; return true;
            case 'T': case 't': size = PLCRUNTIME_NUM_OF_TIMERS; name = "timers"; return true;
            case 'C': case 'c': size = PLCRUNTIME_NUM_OF_COUNTERS; name = "counters"; return true;
            default: return false;
        }
    }

    void warnPrefixedAddressOutOfRange(Token& token, char prefix, int address) {
        if (!emit_warnings) return;
        int size = 0;
        const char* name = "";
        if (!memorySizeFromPrefix(prefix, size, name)) return;
        if (address >= 0 && address < size) return;
        Serial.print(F(" WARNING: ")); Serial.print(name);
        Serial.print(F(" address ")); Serial.print(address);
        Serial.print(F(" is outside size ")); Serial.print(size);
        Serial.print(F(" -> ")); token.print();
        Serial.print(F(" at line ")); Serial.print(token.line); Serial.print(F(":")); Serial.println(token.column);
        token.highlight(assembly_string);
    }

    bool parsePrefixedAddressToken(Token& token, StringView& number, int& offset, int& multiplier) {
        if (token.length < 2) return false;
        if (!memoryOffsetFromPrefix(token.string[0], offset, multiplier)) return false;
        number.data = token.string.data + 1;
        number.length = token.length - 1;
        return true;
    }
    
    // Legacy overload for backward compatibility
    bool parsePrefixedAddressToken(Token& token, StringView& number, int& offset) {
        int multiplier = 1;
        return parsePrefixedAddressToken(token, number, offset, multiplier);
    }

    bool addressFromToken(Token& token, int& output) {
        // First check for property access (e.g., T0.ET, my_timer.ET)
        if (hasPropertyAccess(token.string)) {
            int address = 0, bit = 0;
            bool is_bit = false;
            u8 type_size = 0;
            bool err = parsePropertyAccess(token.string, address, bit, is_bit, type_size);
            if (!err) {
                output = address;
                return false;
            }
            // Fall through to other parsing methods if property access failed
        }

        int offset = 0;
        int multiplier = 1;
        StringView number;
        
        // Handle # prefix for immediate/direct values (e.g., #1000)
        if (token.length > 1 && token.string[0] == '#') {
            number.data = token.string.data + 1;
            number.length = token.length - 1;
            int value_int = 0;
            float value_float = 0;
            if (isInteger(number, value_int)) {
                output = value_int;
                return false;
            }
            if (isReal(number, value_float)) {
                output = (int) value_float;
                return false;
            }
            // Fall through to other parsing methods
        }
        
        if (parsePrefixedAddressToken(token, number, offset, multiplier)) {
            int value_int = 0;
            float value_float = 0;
            if (isInteger(number, value_int)) {
                warnPrefixedAddressOutOfRange(token, token.string[0], value_int);
                output = offset + (value_int * multiplier);
                return false;
            }
            if (isReal(number, value_float)) {
                int addr = (int) value_float;
                warnPrefixedAddressOutOfRange(token, token.string[0], addr);
                output = offset + (addr * multiplier);
                return false;
            }
            // If prefixed parsing fails, fall back to named constants or plain ints.
        }
        
        // Check symbol table for named symbols (e.g., variable names)
        if (token.type == TOKEN_KEYWORD) {
            StringView name = { token.string.data, token.length };
            Symbol* sym = findSymbol(name);
            if (sym != nullptr) {
                output = sym->address;
                return false;
            }
        }
        
        return intFromToken(token, output);
    }

    bool realFromToken(Token& token, float& output) {
        if (token.type == TOKEN_INTEGER) {
            output = token.value_int;
            return false;
        }
        if (token.type == TOKEN_REAL) {
            output = token.value_float;
            return false;
        }
        if (token.type == TOKEN_KEYWORD) {
            for (int i = 0; i < LUT_const_count; i++) {
                LUT_const& c = LUT_consts[i];
                if (str_cmp(token, c.string)) {
                    if (c.type == VAL_BOOLEAN) {
                        token.string = c.string;
                        output = c.value_bool;
                        return false;
                    }
                    if (c.type == VAL_INTEGER) {
                        token.string = c.string;
                        output = c.value_int;
                        return false;
                    }
                    if (c.type == VAL_REAL) {
                        token.string = c.string;
                        output = c.value_float;
                        return false;
                    }
                    return true;
                }
            }
        }
        return true;
    }

    // Parse "2.7" into address and bit, where we separate the two with a dot. The bits range from 0 to 7
    // Also resolves symbols like "button1" to their defined address
    // Now also supports property access like "T0.Q", "my_timer.ET"
    bool memoryBitFromToken(Token& token, int& address, int& bit) {
        // First check for property access (e.g., T0.Q, my_timer.ET)
        if (hasPropertyAccess(token.string)) {
            bool is_bit = false;
            u8 type_size = 0;
            bool err = parsePropertyAccess(token.string, address, bit, is_bit, type_size);
            if (!err) {
                return false; // Successfully parsed property access
            }
            // Fall through to other parsing methods if property access failed
        }

        int offset = 0;
        StringView number = token.string;
        bool has_prefix = parsePrefixedAddressToken(token, number, offset);
        if (has_prefix) {
            int value_int = 0;
            float value = 0;
            if (isInteger(number, value_int)) {
                warnPrefixedAddressOutOfRange(token, token.string[0], value_int);
                address = offset + value_int;
                bit = 0;
                return false;
            }
            if (isReal(number, value)) {
                float addr = (int) value;
                warnPrefixedAddressOutOfRange(token, token.string[0], (int) addr);
                address = (int) addr + offset;
                bit = (int) ((value - addr) * 100.0);
                bit = bit > 65 ? 7 :
                    bit > 55 ? 6 :
                    bit > 45 ? 5 :
                    bit > 35 ? 4 :
                    bit > 25 ? 3 :
                    bit > 15 ? 2 :
                    bit > 5 ? 1 : 0;
                return false;
            }
            // If prefixed parsing fails, fall back to named constants or plain ints.
        }
        if (token.type == TOKEN_INTEGER) { // Expect token "2" to be address 2 at the index 0 by default 
            address = token.value_int;
            bit = 0;
            return false;
        }
        float value = 0;
        bool e_real = realFromToken(token, value);
        bool isReal = !e_real;
        if (isReal || token.type == TOKEN_REAL) { // Expect token "2.7" to be address 2 at the index 7
            // Iterate over the float value and separate the address and bit
            float addr = (int) value;
            address = (int) addr;
            bit = (int) ((value - addr) * 100.0);
            bit = bit > 65 ? 7 :
                bit > 55 ? 6 :
                bit > 45 ? 5 :
                bit > 35 ? 4 :
                bit > 25 ? 3 :
                bit > 15 ? 2 :
                bit > 5 ? 1 : 0;
            return false;
        }
        // Try to resolve as a symbol
        Symbol* sym = findSymbol(token.string);
        if (sym != nullptr) {
            // Symbol found - use its address
            address = sym->address;
            if (sym->is_bit) {
                bit = sym->bit;
            } else {
                bit = 0;
            }
            return false;
        }
        return true;
    }

    // Parse duration string like "T#5s", "T#200ms", "T#4m10s500ms" into milliseconds
    // Returns false on success, true on error
    bool parseDuration(StringView& str, u32& milliseconds) {
        if (str.length < 3) return true; // Must be at least "T#x"
        if (str.data[0] != 'T' || str.data[1] != '#') return true;

        milliseconds = 0;
        int i = 2; // Start after "T#"

        while (i < str.length) {
            // Parse number
            int num_start = i;
            while (i < str.length && str.data[i] >= '0' && str.data[i] <= '9') {
                i++;
            }
            if (i == num_start) return true; // No number found

            // Convert number substring to integer
            u32 value = 0;
            for (int j = num_start; j < i; j++) {
                value = value * 10 + (str.data[j] - '0');
            }

            // Parse unit
            if (i >= str.length) return true; // No unit found

            char unit1 = str.data[i];
            char unit2 = (i + 1 < str.length) ? str.data[i + 1] : '\0';

            u32 multiplier = 0;
            int unit_len = 0;

            // Check for two-character units first
            if (unit1 == 'm' && unit2 == 's') {
                multiplier = 1; // milliseconds
                unit_len = 2;
            } else if (unit1 == 's') {
                multiplier = 1000; // seconds
                unit_len = 1;
            } else if (unit1 == 'm') {
                multiplier = 60000; // minutes
                unit_len = 1;
            } else if (unit1 == 'h') {
                multiplier = 3600000; // hours
                unit_len = 1;
            } else if (unit1 == 'd') {
                multiplier = 86400000; // days
                unit_len = 1;
            } else {
                return true; // Unknown unit
            }

            milliseconds += value * multiplier;
            i += unit_len;
        }

        return false; // Success
    }

    bool labelFromToken(Token& token, int& output) {
        if (token.type == TOKEN_KEYWORD) {
            for (int i = 0; i < LUT_label_count; i++) {
                LUT_label& l = LUT_labels[i];
                if (str_cmp(token, l.string)) {
                    output = l.address;
                    return false;
                }
            }
        }
        if (token.type == TOKEN_INTEGER) {
            output = token.value_int;
            return false;
        }
        return true;
    }

    virtual bool reportError(Token& token, const char* message) {
        Serial.print(F(" ERROR: ")); Serial.print(F(message)); Serial.print(F(" -> ")); token.print(); Serial.print(F(" at line ")); Serial.print(token.line); Serial.print(F(":")); Serial.println(token.column);
        token.highlight(assembly_string);
        return true;
    }

    bool buildError(Token token, const char* message) { return reportError(token, message); }
    bool buildErrorSizeLimit(Token token) { return buildError(token, "program size limit reached"); }
    bool buildErrorUnknownToken(Token token) { return buildError(token, "unknown token"); }
    bool buildErrorExpectedInt(Token token) { return buildError(token, "unexpected token, expected integer"); }
    bool buildErrorExpectedIntSameLine(Token& current, Token& next, bool& rewind) {
        if (next.line != current.line) {
            rewind = true;
            return buildError(current, "missing a number in the same line");
        }
        rewind = false;
        return buildErrorExpectedInt(next);
    }
    bool buildErrorExpectedFloat(Token token) { return buildError(token, "unexpected token, expected float"); }
    bool buildErrorUnknownLabel(Token token) { return buildError(token, "unknown label"); }

    // Helper to add an IR entry
    void addIREntry(u32 offset, Token& token, u8 size, u8 opcode, u8 flags,
        u8 op_count = 0,
        IR_OperandType op1_type = IR_OP_NONE, u8 op1_pos = 0, u64 op1_val = 0,
        IR_OperandType op2_type = IR_OP_NONE, u8 op2_pos = 0, u64 op2_val = 0,
        IR_OperandType op3_type = IR_OP_NONE, u8 op3_pos = 0, u64 op3_val = 0) {
        if (ir_entry_count >= MAX_IR_ENTRIES) return;
        IR_Entry& e = ir_entries[ir_entry_count++];
        e.bytecode_offset = offset;
        e.source_line = (u16) token.line;
        e.source_column = (u16) token.column;
        e.bytecode_size = size;
        e.opcode = opcode;
        e.flags = flags;
        e.operand_count = op_count;

        if (op_count >= 1) {
            e.operands[0].type = op1_type;
            e.operands[0].bytecode_pos = op1_pos;
            e.operands[0].val_u64 = op1_val;
        }
        if (op_count >= 2) {
            e.operands[1].type = op2_type;
            e.operands[1].bytecode_pos = op2_pos;
            e.operands[1].val_u64 = op2_val;
        }
        if (op_count >= 3) {
            e.operands[2].type = op3_type;
            e.operands[2].bytecode_pos = op3_pos;
            e.operands[2].val_u64 = op3_val;
        }
    }

    // Simplified IR entry for simple opcodes with no operands
    void addIRSimple(u32 offset, Token& token, u8 size, u8 opcode) {
        addIREntry(offset, token, size, opcode, IR_FLAG_NONE);
    }

    bool build(bool finalPass, bool lintMode = false) {
        int auto_assigned_bit_counter = 0;
        programLineCount = 0;
        built_bytecode_length = 0;
        built_bytecode_checksum = 0;
        emit_warnings = finalPass && !lintMode;
        if (finalPass) ir_entry_count = 0; // Clear IR on final pass

        // IR flags and operand info to be set before _line_push
        u8 _ir_flags = IR_FLAG_NONE;
        u8 _ir_op_count = 0;
        IR_OperandType _ir_op1_type = IR_OP_NONE; u8 _ir_op1_pos = 0; u64 _ir_op1_val = 0;
        IR_OperandType _ir_op2_type = IR_OP_NONE; u8 _ir_op2_pos = 0; u64 _ir_op2_val = 0;
        IR_OperandType _ir_op3_type = IR_OP_NONE; u8 _ir_op3_pos = 0; u64 _ir_op3_val = 0;

#define _ir_reset() \
            _ir_flags = IR_FLAG_NONE; \
            _ir_op_count = 0; \
            _ir_op1_type = IR_OP_NONE; _ir_op1_pos = 0; _ir_op1_val = 0; \
            _ir_op2_type = IR_OP_NONE; _ir_op2_pos = 0; _ir_op2_val = 0; \
            _ir_op3_type = IR_OP_NONE; _ir_op3_pos = 0; _ir_op3_val = 0;

#define _ir_set_const(typ, pos, val) \
            _ir_flags |= IR_FLAG_CONST | IR_FLAG_EDITABLE; \
            _ir_op_count = 1; \
            _ir_op1_type = typ; _ir_op1_pos = pos; _ir_op1_val = val;

#define _ir_set_ptr(pos, val) \
            _ir_flags |= IR_FLAG_READ; \
            _ir_op_count = 1; \
            _ir_op1_type = IR_OP_PTR; _ir_op1_pos = pos; _ir_op1_val = val;

#define _ir_set_write_ptr(pos, val) \
            _ir_flags |= IR_FLAG_WRITE; \
            _ir_op_count = 1; \
            _ir_op1_type = IR_OP_PTR; _ir_op1_pos = pos; _ir_op1_val = val;

#define _ir_set_jump(pos, val) \
            _ir_flags |= IR_FLAG_JUMP; \
            _ir_op_count = 1; \
            _ir_op1_type = IR_OP_LABEL; _ir_op1_pos = pos; _ir_op1_val = val;

#define _ir_set_timer_const(timer_pos, timer_val, pt_pos, pt_val) \
            _ir_flags |= IR_FLAG_TIMER | IR_FLAG_CONST | IR_FLAG_EDITABLE; \
            _ir_op_count = 2; \
            _ir_op1_type = IR_OP_PTR; _ir_op1_pos = timer_pos; _ir_op1_val = timer_val; \
            _ir_op2_type = IR_OP_U32; _ir_op2_pos = pt_pos; _ir_op2_val = pt_val;

#define _ir_set_timer_mem(timer_pos, timer_val, pt_pos, pt_val) \
            _ir_flags |= IR_FLAG_TIMER; \
            _ir_op_count = 2; \
            _ir_op1_type = IR_OP_PTR; _ir_op1_pos = timer_pos; _ir_op1_val = timer_val; \
            _ir_op2_type = IR_OP_PTR; _ir_op2_pos = pt_pos; _ir_op2_val = pt_val;

#define _line_push \
            address_end = built_bytecode_length + line.size; \
            if (address_end >= PLCRUNTIME_MAX_PROGRAM_SIZE) { buildErrorSizeLimit(token); return true; } \
            if (!lintMode) { \
                for (int j = 0; j < line.size; j++) { \
                    built_bytecode[built_bytecode_length + j] = bytecode[j]; \
                    crc8_simple(built_bytecode_checksum, bytecode[j]); \
                } \
            } \
            if (finalPass && !lintMode) { \
                addIREntry(built_bytecode_length, token, line.size, bytecode[0], _ir_flags, \
                    _ir_op_count, _ir_op1_type, _ir_op1_pos, _ir_op1_val, \
                    _ir_op2_type, _ir_op2_pos, _ir_op2_val, \
                    _ir_op3_type, _ir_op3_pos, _ir_op3_val); \
            } \
            _ir_reset(); \
            built_bytecode_length = address_end; \
            continue;

        int address_end = 0;

        // First pass: parse symbol definitions ($$)
        // Also parse in lint mode to show symbol errors in the linter
        if (!finalPass || lintMode) {
            symbol_count = base_symbol_count; // Reset to base symbols (preserve external symbols)
            for (int i = 0; i < token_count; i++) {
                Token& token = tokens[i];
                if (str_cmp(token.string, "$$")) {
                    // Found symbol definition
                    if (parseSymbolDefinition(i)) {
                        return true; // Error occurred
                    }
                    // Skip past the symbol definition tokens
                    // Format: $$ name | type | address [| comment]
                    // Minimum is 6 tokens: $$ name | type | address
                    i += 5; // Skip to address token, loop will increment
                    // Skip optional comment (everything until end of line)
                    while (i + 1 < token_count && tokens[i + 1].line == token.line) {
                        i++;
                    }
                }
            }

            // Check for overlaps after all symbols are parsed
            if (emit_warnings || lintMode) {
                for (int i = 0; i < symbol_count; i++) {
                    checkSymbolOverlap(i);
                }
            }
        }

        for (int i = 0; i < token_count; i++) {
            Token& token = tokens[i];
            TokenType type = token.type;

            // Skip symbol definitions in both passes
            if (str_cmp(token.string, "$$")) {
                // Skip past entire symbol definition
                i += 5; // Skip $$ name | type | address
                while (i + 1 < token_count && tokens[i + 1].line == token.line) {
                    i++;
                }
                continue;
            }

            // Handle .runtime_config directive for dynamic T/C configuration
            // Format: .runtime_config T <offset> <count> C <offset> <count>
            if (token == ".runtime_config") {
                // Parse: T <timer_offset> <timer_count> C <counter_offset> <counter_count>
                if (i + 6 < token_count) {
                    Token& t_tok = tokens[i + 1];
                    Token& t_off = tokens[i + 2];
                    Token& t_cnt = tokens[i + 3];
                    Token& c_tok = tokens[i + 4];
                    Token& c_off = tokens[i + 5];
                    Token& c_cnt = tokens[i + 6];
                    
                    if ((t_tok == "T" || t_tok == "t") && (c_tok == "C" || c_tok == "c")) {
                        int timer_offset_val = 0, timer_count_val = 0;
                        int counter_offset_val = 0, counter_count_val = 0;
                        
                        if (!intFromToken(t_off, timer_offset_val) &&
                            !intFromToken(t_cnt, timer_count_val) &&
                            !intFromToken(c_off, counter_offset_val) &&
                            !intFromToken(c_cnt, counter_count_val)) {
                            
                            // Apply the configuration to compiler offsets
                            plcasm_timer_offset = timer_offset_val;
                            plcasm_counter_offset = counter_offset_val;
                            
                            // Emit CONFIG_TC bytecode instruction to configure runtime at startup
                            // Format: CONFIG_TC <timer_offset:u16> <timer_count:u8> <counter_offset:u16> <counter_count:u8>
                            ProgramLine& line = programLines[programLineCount];
                            line.index = built_bytecode_length;
                            line.refToken = &token;
                            u8* bytecode = line.code;
                            u8 offset = 0;
                            bytecode[offset++] = CONFIG_TC;
                            bytecode[offset++] = (timer_offset_val >> 8) & 0xFF;   // timer_offset high byte
                            bytecode[offset++] = timer_offset_val & 0xFF;          // timer_offset low byte
                            bytecode[offset++] = timer_count_val & 0xFF;           // timer_count
                            bytecode[offset++] = (counter_offset_val >> 8) & 0xFF; // counter_offset high byte
                            bytecode[offset++] = counter_offset_val & 0xFF;        // counter_offset low byte
                            bytecode[offset++] = counter_count_val & 0xFF;         // counter_count
                            line.size = offset;
                            i += 6; // Skip all the tokens we consumed (MUST be before _line_push which has continue)
                            _line_push;
                        }
                    }
                }
                // If we get here, format was invalid - just skip the directive token
                // Skip to end of line
                while (i + 1 < token_count && tokens[i + 1].line == token.line) {
                    i++;
                }
                continue;
            }

            if (type == TOKEN_UNKNOWN) {
                // Try to resolve as symbol
                Symbol* sym = findSymbol(token.string);
                if (sym) {
                    // Symbol found! Replace token with appropriate address/bit access
                    if (sym->is_bit) {
                        // Generate bit read instruction
                        ProgramLine& line = programLines[programLineCount];
                        line.index = built_bytecode_length;
                        line.refToken = &token;
                        u8* bytecode = line.code;

                        // Determine bit read instruction based on bit position
                        PLCRuntimeInstructionSet bit_op = (PLCRuntimeInstructionSet) (READ_X8_B0 + sym->bit);
                        line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, bit_op, sym->address);
                        _ir_set_ptr(1, sym->address);
                        _line_push;
                    } else {
                        // Generate value load instruction based on type
                        ProgramLine& line = programLines[programLineCount];
                        line.index = built_bytecode_length;
                        line.refToken = &token;
                        u8* bytecode = line.code;

                        // Determine load type based on symbol type
                        u8 type_byte = 0;
                        if (str_cmp(sym->type, "u8") || str_cmp(sym->type, "byte")) type_byte = type_u8;
                        else if (str_cmp(sym->type, "i8")) type_byte = type_i8;
                        else if (str_cmp(sym->type, "u16")) type_byte = type_u16;
                        else if (str_cmp(sym->type, "i16")) type_byte = type_i16;
                        else if (str_cmp(sym->type, "u32")) type_byte = type_u32;
                        else if (str_cmp(sym->type, "i32")) type_byte = type_i32;
                        else if (str_cmp(sym->type, "f32")) type_byte = type_f32;
                        else if (str_cmp(sym->type, "u64")) type_byte = type_u64;
                        else if (str_cmp(sym->type, "i64")) type_byte = type_i64;
                        else if (str_cmp(sym->type, "f64")) type_byte = type_f64;
                        else {
                            // Default to pointer load for unknown types
                            type_byte = type_pointer;
                        }

                        line.size = InstructionCompiler::push_load_from(bytecode, (PLCRuntimeInstructionSet) type_byte, sym->address);
                        _ir_set_ptr(2, sym->address);
                        _line_push;
                    }
                    continue;
                }

                if (buildErrorUnknownToken(token)) return true;
                continue;
            }

            if (type == TOKEN_LABEL) {
                if (finalPass) continue;
                for (int i = 0; i < LUT_label_count; i++) {
                    LUT_label& label = LUT_labels[i];
                    if (str_cmp(token, label.string)) {
                        label.address = built_bytecode_length;
                        break;
                    }
                }
                continue;
            }

            // bool value_bool;
            int label_address = -1;
            int value_int;
            float value_float;
            u8 data_type = 0;

            ProgramLine& line = programLines[programLineCount];
            line.index = built_bytecode_length;
            line.refToken = &token;
            u8* bytecode = line.code;

            bool hasNext = i + 1 < token_count;
            // [ u8.const , 3 ]
            Token& token_p1 = hasNext ? tokens[i + 1] : tokens[i];
            bool e_label = finalPass ? labelFromToken(token_p1, label_address) : true;
            // bool e_bool = boolFromToken(token_p1, value_bool);
            bool e_int = intFromToken(token_p1, value_int);
            bool e_real = realFromToken(token_p1, value_float);
            /* bool e_data_type = */ typeFromToken(token, data_type);


            bool hasThird = i + 2 < token_count;
            Token& token_p2 = hasThird ? tokens[i + 2] : tokens[i];

            if (type == TOKEN_KEYWORD) {
                // Handle timers
                {
                    bool is_ton = token == "ton";
                    bool is_tof = token == "tof";
                    bool is_tp = token == "tp";

                    if (is_ton || is_tof || is_tp) {
                        // Timer instruction found - consume it and process parameters
                        if (!hasNext) {
                            return buildError(token, "missing timer parameters");
                        }
                        if (!hasThird) {
                            return buildError(token_p1, "missing timer parameters");
                        }

                        int timer_addr = 0;
                        if (addressFromToken(token_p1, timer_addr)) {
                            // First parameter is not a valid address
                            return buildError(token_p1, "invalid timer address ");
                        }

                        // First parameter is valid, now check second parameter
                        bool is_const_param = token_p2.length > 1 && (token_p2.string[0] == '#' || (token_p2.length > 2 && token_p2.string[0] == 'T' && token_p2.string[1] == '#'));

                        if (is_const_param) {
                            // Constant timer preset (either #123 or T#5s format)
                            u32 pt_val = 0;

                            if (token_p2.string[0] == 'T' && token_p2.string[1] == '#') {
                                // Duration format: T#5s, T#200ms, T#4m10s500ms
                                if (parseDuration(token_p2.string, pt_val)) {
                                    return buildError(token_p2, "invalid duration format");
                                }
                            } else {
                                // Simple integer format: #123
                                StringView valStr;
                                valStr.data = token_p2.string.data + 1;
                                valStr.length = token_p2.string.length - 1;
                                int pt_val_int = 0;
                                if (!isInteger(valStr, pt_val_int)) {
                                    return buildError(token_p2, "unknown variable or value");
                                }
                                pt_val = (u32) pt_val_int;
                            }

                            // Constant timer: [opcode][timer_addr][pt_value]
                            PLCRuntimeInstructionSet op = (PLCRuntimeInstructionSet) (is_ton ? TON_CONST : (is_tof ? TOF_CONST : TP_CONST));
                            i += 2;
                            line.size = InstructionCompiler::push_timer_const(bytecode, op, (MY_PTR_t) timer_addr, (u32) pt_val);
                            _ir_set_timer_const(1, timer_addr, 1 + sizeof(MY_PTR_t), pt_val);
                            _line_push;
                        } else {
                            // Memory-based timer preset
                            int pt_addr = 0;
                            if (addressFromToken(token_p2, pt_addr)) {
                                // Second parameter is not a valid address or constant
                                return buildError(token_p2, "unknown variable or value");
                            }
                            // Memory timer: [opcode][timer_addr][pt_addr]
                            PLCRuntimeInstructionSet op = (PLCRuntimeInstructionSet) (is_ton ? TON_MEM : (is_tof ? TOF_MEM : TP_MEM));
                            i += 2;
                            line.size = InstructionCompiler::push_timer_mem(bytecode, op, (MY_PTR_t) timer_addr, (MY_PTR_t) pt_addr);
                            _ir_set_timer_mem(1, timer_addr, 1 + sizeof(MY_PTR_t), pt_addr);
                            _line_push;
                        }
                    }
                }

                // Handle counters (CTU, CTD)
                {
                    bool is_ctu = token == "ctu";
                    bool is_ctd = token == "ctd";

                    if (is_ctu || is_ctd) {
                        // Counter instruction found - consume it and process parameters
                        if (!hasNext) {
                            return buildError(token, "missing counter parameters");
                        }
                        if (!hasThird) {
                            return buildError(token_p1, "missing counter parameters");
                        }

                        int counter_addr = 0;
                        if (addressFromToken(token_p1, counter_addr)) {
                            // First parameter is not a valid address
                            return buildError(token_p1, "invalid counter address ");
                        }

                        // First parameter is valid, now check second parameter
                        bool is_const_param = token_p2.length > 1 && token_p2.string[0] == '#';

                        if (is_const_param) {
                            // Constant preset value: #123
                            StringView valStr;
                            valStr.data = token_p2.string.data + 1;
                            valStr.length = token_p2.string.length - 1;
                            int pv_val_int = 0;
                            if (!isInteger(valStr, pv_val_int)) {
                                return buildError(token_p2, "expected integer value after #");
                            }
                            u32 pv_val = (u32) pv_val_int;

                            // Constant counter: [opcode][counter_addr][pv_value]
                            PLCRuntimeInstructionSet op = (PLCRuntimeInstructionSet) (is_ctu ? CTU_CONST : CTD_CONST);
                            i += 2;
                            line.size = InstructionCompiler::push_counter_const(bytecode, op, (MY_PTR_t) counter_addr, (u32) pv_val);
                            _ir_set_timer_const(1, counter_addr, 1 + sizeof(MY_PTR_t), pv_val);
                            _line_push;
                        } else {
                            // Memory-based preset value
                            int pv_addr = 0;
                            if (addressFromToken(token_p2, pv_addr)) {
                                // Second parameter is not a valid address or constant
                                return buildError(token_p2, "invalid preset address");
                            }
                            // Memory counter: [opcode][counter_addr][pv_addr]
                            PLCRuntimeInstructionSet op = (PLCRuntimeInstructionSet) (is_ctu ? CTU_MEM : CTD_MEM);
                            i += 2;
                            line.size = InstructionCompiler::push_counter_mem(bytecode, op, (MY_PTR_t) counter_addr, (MY_PTR_t) pv_addr);
                            _ir_set_timer_mem(1, counter_addr, 1 + sizeof(MY_PTR_t), pv_addr);
                            _line_push;
                        }
                    }
                }

                { // Handle flow
                    if (hasNext && (token == "jmp" || token == "jump")) { if (finalPass && e_label) { if (buildErrorUnknownLabel(token_p1)) return true; } i++; line.size = InstructionCompiler::push_jmp(bytecode, label_address); _ir_set_jump(1, label_address); _line_push; }
                    if (hasNext && (token == "jmp_if" || token == "jump_if")) { if (finalPass && e_label) { if (buildErrorUnknownLabel(token_p1)) return true; } i++; line.size = InstructionCompiler::push_jmp_if(bytecode, label_address); _ir_set_jump(1, label_address); _line_push; }
                    if (hasNext && (token == "jmp_if_not" || token == "jump_if_not")) { if (finalPass && e_label) { if (buildErrorUnknownLabel(token_p1)) return true; } i++; line.size = InstructionCompiler::push_jmp_if_not(bytecode, label_address); _ir_set_jump(1, label_address); _line_push; }
                    if (hasNext && token == "call") { if (finalPass && e_label) { if (buildErrorUnknownLabel(token_p1)) return true; } i++; line.size = InstructionCompiler::pushCALL(bytecode, label_address); _ir_set_jump(1, label_address); _line_push; }
                    if (hasNext && token == "call_if") { if (finalPass && e_label) { if (buildErrorUnknownLabel(token_p1)) return true; } i++; line.size = InstructionCompiler::pushCALL_IF(bytecode, label_address); _ir_set_jump(1, label_address); _line_push; }
                    if (hasNext && token == "call_if_not") { if (finalPass && e_label) { if (buildErrorUnknownLabel(token_p1)) return true; } i++; line.size = InstructionCompiler::pushCALL_IF_NOT(bytecode, label_address); _ir_set_jump(1, label_address); _line_push; }

                    if (hasNext && (token == "jmp_rel" || token == "jump_rel")) { if (finalPass && e_label) { if (buildErrorUnknownLabel(token_p1)) return true; } int off = !e_label ? (label_address - (int) (line.index + 3)) : (!e_int ? value_int : 0); i++; line.size = InstructionCompiler::push_jmp_rel(bytecode, (i16) off); _ir_set_jump(1, off); _line_push; }
                    if (hasNext && (token == "jmp_if_rel" || token == "jump_if_rel")) { if (finalPass && e_label) { if (buildErrorUnknownLabel(token_p1)) return true; } int off = !e_label ? (label_address - (int) (line.index + 3)) : (!e_int ? value_int : 0); i++; line.size = InstructionCompiler::push_jmp_if_rel(bytecode, (i16) off); _ir_set_jump(1, off); _line_push; }
                    if (hasNext && (token == "jmp_if_not_rel" || token == "jump_if_not_rel")) { if (finalPass && e_label) { if (buildErrorUnknownLabel(token_p1)) return true; } int off = !e_label ? (label_address - (int) (line.index + 3)) : (!e_int ? value_int : 0); i++; line.size = InstructionCompiler::push_jmp_if_not_rel(bytecode, (i16) off); _ir_set_jump(1, off); _line_push; }

                    if (hasNext && token == "call_rel") { if (finalPass && e_label) { if (buildErrorUnknownLabel(token_p1)) return true; } int off = !e_label ? (label_address - (int) (line.index + 3)) : (!e_int ? value_int : 0); i++; line.size = InstructionCompiler::pushCALL_REL(bytecode, (i16) off); _ir_set_jump(1, off); _line_push; }
                    if (hasNext && token == "call_if_rel") { if (finalPass && e_label) { if (buildErrorUnknownLabel(token_p1)) return true; } int off = !e_label ? (label_address - (int) (line.index + 3)) : (!e_int ? value_int : 0); i++; line.size = InstructionCompiler::pushCALL_IF_REL(bytecode, (i16) off); _line_push; }
                    if (hasNext && token == "call_if_not_rel") { if (finalPass && e_label) { if (buildErrorUnknownLabel(token_p1)) return true; } int off = !e_label ? (label_address - (int) (line.index + 3)) : (!e_int ? value_int : 0); i++; line.size = InstructionCompiler::pushCALL_IF_NOT_REL(bytecode, (i16) off); _line_push; }
                    if (token == "ret" || token == "return") { line.size = InstructionCompiler::push(bytecode, RET); _line_push; }
                    if (token == "ret_if" || token == "return_if") { line.size = InstructionCompiler::push(bytecode, RET_IF); _line_push; }
                    if (token == "ret_if_not" || token == "return_if_not") { line.size = InstructionCompiler::push(bytecode, RET_IF_NOT); _line_push; }
                    if (token == "nop") { line.size = InstructionCompiler::push(bytecode, NOP); _line_push; }
                    
                    // Metadata instructions (for decompilation and debugging)
                    if (hasNext && token == "lang") {
                        u8 lang_id = LANG_UNKNOWN;
                        if (token_p1 == "plcasm") lang_id = LANG_PLCASM;
                        else if (token_p1 == "stl") lang_id = LANG_STL;
                        else if (token_p1 == "ladder") lang_id = LANG_LADDER;
                        else if (token_p1 == "fbd") lang_id = LANG_FBD;
                        else if (token_p1 == "sfc") lang_id = LANG_SFC;
                        else if (token_p1 == "st") lang_id = LANG_ST;
                        else if (token_p1 == "il") lang_id = LANG_IL;
                        else if (token_p1 == "plcscript") lang_id = LANG_PLCSCRIPT;
                        else if (token_p1 == "custom") lang_id = LANG_CUSTOM;
                        else if (!e_int) lang_id = (u8) value_int;
                        else { if (buildError(token_p1, "expected language name or id")) return true; }
                        i++;
                        line.size = InstructionCompiler::push_lang(bytecode, lang_id);
                        _line_push;
                    }
                    if (hasNext && token == "comment") {
                        // Expect: comment 'string content'
                        // The tokenizer collapses 'string' into a single TOKEN_STRING
                        if (token_p1.type != TOKEN_STRING) {
                            if (buildError(token_p1, "expected string for comment (e.g., comment 'text')")) return true; 
                        }
                        const char* raw = token_p1.string.data;
                        int comment_len = token_p1.string.length;
                        if (comment_len > 255) { if (buildError(token_p1, "comment too long (max 255 chars)")) return true; }
                        i++; // Skip the string token
                        line.size = InstructionCompiler::push_comment(bytecode, raw, comment_len);
                        _line_push;
                    }
                }

                { // Stack operations
                    if (token == "clear") { line.size = InstructionCompiler::push(bytecode, CLEAR); _line_push; }
                }

                { // Branch stack operations (for parallel branches in ladder logic)
                    if (token == "br.save") { line.size = InstructionCompiler::push_br_save(bytecode); _line_push; }
                    if (token == "br.read") { line.size = InstructionCompiler::push_br_read(bytecode); _line_push; }
                    if (token == "br.drop") { line.size = InstructionCompiler::push_br_drop(bytecode); _line_push; }
                    if (token == "br.clr") { line.size = InstructionCompiler::push_br_clr(bytecode); _line_push; }
                }

                { // Bitwise operations (bw.and.x8, bw.or.x16, bw.xor.x32, bw.not.x64, bw.shl.x8, bw.shr.x16, etc.)
                    if (token == "bw.and.x8") { line.size = InstructionCompiler::push(bytecode, BW_AND_X8); _line_push; }
                    if (token == "bw.and.x16") { line.size = InstructionCompiler::push(bytecode, BW_AND_X16); _line_push; }
                    if (token == "bw.and.x32") { line.size = InstructionCompiler::push(bytecode, BW_AND_X32); _line_push; }
                    if (token == "bw.and.x64") { line.size = InstructionCompiler::push(bytecode, BW_AND_X64); _line_push; }
                    if (token == "bw.or.x8") { line.size = InstructionCompiler::push(bytecode, BW_OR_X8); _line_push; }
                    if (token == "bw.or.x16") { line.size = InstructionCompiler::push(bytecode, BW_OR_X16); _line_push; }
                    if (token == "bw.or.x32") { line.size = InstructionCompiler::push(bytecode, BW_OR_X32); _line_push; }
                    if (token == "bw.or.x64") { line.size = InstructionCompiler::push(bytecode, BW_OR_X64); _line_push; }
                    if (token == "bw.xor.x8") { line.size = InstructionCompiler::push(bytecode, BW_XOR_X8); _line_push; }
                    if (token == "bw.xor.x16") { line.size = InstructionCompiler::push(bytecode, BW_XOR_X16); _line_push; }
                    if (token == "bw.xor.x32") { line.size = InstructionCompiler::push(bytecode, BW_XOR_X32); _line_push; }
                    if (token == "bw.xor.x64") { line.size = InstructionCompiler::push(bytecode, BW_XOR_X64); _line_push; }
                    if (token == "bw.not.x8") { line.size = InstructionCompiler::push(bytecode, BW_NOT_X8); _line_push; }
                    if (token == "bw.not.x16") { line.size = InstructionCompiler::push(bytecode, BW_NOT_X16); _line_push; }
                    if (token == "bw.not.x32") { line.size = InstructionCompiler::push(bytecode, BW_NOT_X32); _line_push; }
                    if (token == "bw.not.x64") { line.size = InstructionCompiler::push(bytecode, BW_NOT_X64); _line_push; }
                    if (token == "bw.shl.x8") { line.size = InstructionCompiler::push(bytecode, BW_LSHIFT_X8); _line_push; }
                    if (token == "bw.shl.x16") { line.size = InstructionCompiler::push(bytecode, BW_LSHIFT_X16); _line_push; }
                    if (token == "bw.shl.x32") { line.size = InstructionCompiler::push(bytecode, BW_LSHIFT_X32); _line_push; }
                    if (token == "bw.shl.x64") { line.size = InstructionCompiler::push(bytecode, BW_LSHIFT_X64); _line_push; }
                    if (token == "bw.shr.x8") { line.size = InstructionCompiler::push(bytecode, BW_RSHIFT_X8); _line_push; }
                    if (token == "bw.shr.x16") { line.size = InstructionCompiler::push(bytecode, BW_RSHIFT_X16); _line_push; }
                    if (token == "bw.shr.x32") { line.size = InstructionCompiler::push(bytecode, BW_RSHIFT_X32); _line_push; }
                    if (token == "bw.shr.x64") { line.size = InstructionCompiler::push(bytecode, BW_RSHIFT_X64); _line_push; }
                }

                { // Handle Bit operations (PLC specific)

                    if (data_type) {
                        PLCRuntimeInstructionSet type = (PLCRuntimeInstructionSet) data_type;
                        if (type == type_u8) {

                            // Stack operations
                            PLCRuntimeInstructionSet stack_bit_task = (PLCRuntimeInstructionSet) 0;
                            if (!stack_bit_task && token.endsWith(".get")) stack_bit_task = GET_X8_B0; // READ BIT FROM BYTE
                            if (!stack_bit_task && token.endsWith(".set")) stack_bit_task = SET_X8_B0; // SET BIT IN BYTE
                            if (!stack_bit_task && token.endsWith(".rset")) stack_bit_task = RSET_X8_B0; // RESET BIT IN BYTE
                            if (stack_bit_task) {
                                if (e_int) {
                                    bool rewind = false;
                                    if (buildErrorExpectedIntSameLine(token, token_p1, rewind)) return true;
                                    if (rewind) continue;
                                }
                                i++;
                                if (value_int < 0 || value_int > 7) { if (buildError(token_p1, "bit value out of range for 8-bit type")) return true; }
                                stack_bit_task = (PLCRuntimeInstructionSet) ((int) stack_bit_task + value_int);
                                line.size = InstructionCompiler::push(bytecode, stack_bit_task); _line_push;
                            }

                            if (token == "u8.and") { line.size = InstructionCompiler::push(bytecode, LOGIC_AND); _line_push; }
                            if (token == "u8.or") { line.size = InstructionCompiler::push(bytecode, LOGIC_OR); _line_push; }
                            if (token == "u8.xor") { line.size = InstructionCompiler::push(bytecode, LOGIC_XOR); _line_push; }
                            if (token == "u8.not") { line.size = InstructionCompiler::push(bytecode, LOGIC_NOT); _line_push; }

                            // Edge detection
                            PLCRuntimeInstructionSet edge_task = (PLCRuntimeInstructionSet) 0;
                            if (token.endsWith(".readBitDU")) edge_task = READ_BIT_DU;
                            else if (token.endsWith(".readBitDD")) edge_task = READ_BIT_DD;
                            else if (token.endsWith(".readBitInvDU")) edge_task = READ_BIT_INV_DU;
                            else if (token.endsWith(".readBitInvDD")) edge_task = READ_BIT_INV_DD;
                            else if (token.endsWith(".writeBitDU")) edge_task = WRITE_BIT_DU;
                            else if (token.endsWith(".writeBitDD")) edge_task = WRITE_BIT_DD;
                            else if (token.endsWith(".writeBitInvDU")) edge_task = WRITE_BIT_INV_DU;
                            else if (token.endsWith(".writeBitInvDD")) edge_task = WRITE_BIT_INV_DD;
                            else if (token.endsWith(".writeBitOnDU")) edge_task = WRITE_SET_DU;
                            else if (token.endsWith(".writeBitOnDD")) edge_task = WRITE_SET_DD;
                            else if (token.endsWith(".writeBitOffDU")) edge_task = WRITE_RSET_DU;
                            else if (token.endsWith(".writeBitOffDD")) edge_task = WRITE_RSET_DD;

                            if (edge_task) {
                                int addr1 = 0, bit1 = 0;
                                int addr2 = 0, bit2 = 0;
                                if (memoryBitFromToken(token_p1, addr1, bit1)) { if (buildError(token_p1, "expected address.bit")) return true; }
                                i++;

                                bool has_second_addr = false;
                                if (hasThird) if (!memoryBitFromToken(token_p2, addr2, bit2)) has_second_addr = true;

                                if (has_second_addr) {
                                    i++;
                                } else {
                                    // Automatically assign bits in the system address space offset
                                    int bit_index = auto_assigned_bit_counter++;
                                    addr2 = plcasm_system_offset + bit_index / 8;
                                    bit2 = bit_index % 8;
                                    if (addr2 >= plcasm_system_offset + PLCRUNTIME_NUM_OF_SYSTEMS) {
                                        if (buildError(token, "System memory exhausted for auto-assigned bits")) return true;
                                    }
                                }

                                line.size = InstructionCompiler::push_InstructionWithTwoPointers(bytecode, edge_task, addr1, bit1, addr2, bit2);
                                _line_push;
                            } else if (token.endsWith(".du") || token.endsWith(".dd") || token.endsWith(".dc")) {
                                PLCRuntimeInstructionSet stack_edge_task = token.endsWith(".du") ? STACK_DU : (token.endsWith(".dd") ? STACK_DD : STACK_DC);
                                int addr_state = 0, bit_state = 0;
                                // Expect [State] for static linking
                                bool has_state_addr = false;
                                if (hasNext) if (!memoryBitFromToken(token_p1, addr_state, bit_state)) has_state_addr = true;

                                if (has_state_addr) {
                                    i++;
                                } else {
                                    // Automatically assign bits in the system address space offset
                                    int bit_index = auto_assigned_bit_counter++;
                                    addr_state = plcasm_system_offset + bit_index / 8;
                                    bit_state = bit_index % 8;
                                    if (addr_state >= plcasm_system_offset + PLCRUNTIME_NUM_OF_SYSTEMS) {
                                        if (buildError(token, "System memory exhausted for auto-assigned bits")) return true;
                                    }
                                }

                                line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, stack_edge_task, addr_state, bit_state);
                                _line_push;
                            }

                            // Direct memory read/write
                            PLCRuntimeInstructionSet mem_bit_task = (PLCRuntimeInstructionSet) 0;
                            if (!mem_bit_task && token.includes(".writeBitInv")) mem_bit_task = WRITE_INV_X8_B0; // WRITE_INV_X8 (INVERT)
                            if (!mem_bit_task && token.includes(".writeBitOff")) mem_bit_task = WRITE_R_X8_B0; // WRITE_R_X8 (RESET)
                            if (!mem_bit_task && token.includes(".writeBitOn")) mem_bit_task = WRITE_S_X8_B0; // WRITE_S_X8 (SET)
                            if (!mem_bit_task && token.includes(".writeBit")) mem_bit_task = WRITE_X8_B0; // WRITE_X8
                            if (!mem_bit_task && token.includes(".readBit")) mem_bit_task = READ_X8_B0; // READ_X8
                            if (mem_bit_task) {
                                int address = 0;
                                int bit = 0;
                                bool e_membit = memoryBitFromToken(token_p1, address, bit);
                                if (e_membit) {
                                    if (token_p1.line != token.line) {
                                        if (buildError(token, "missing a number in the same line")) return true;
                                        continue;
                                    }
                                    if (buildError(token_p1, "unexpected token, expected bit representation")) return true;
                                }
                                i++;
                                if (bit < 0 || bit > 7) { if (buildError(token_p1, "bit value out of range for 8-bit type")) return true; }
                                mem_bit_task = (PLCRuntimeInstructionSet) ((int) mem_bit_task + bit);
                                line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, mem_bit_task, address); _line_push;
                            }
                        }
                    }
                }

                { // Handle data type operations
                    if (data_type) {
                        PLCRuntimeInstructionSet type = (PLCRuntimeInstructionSet) data_type;
                        if (hasNext && token.endsWith(".const")) {
                            if (type == type_pointer) {
                                int address_value = 0;
                                bool e_addr = addressFromToken(token_p1, address_value);
                                if (e_addr) {
                                    bool rewind = false;
                                    if (buildErrorExpectedIntSameLine(token, token_p1, rewind)) return true;
                                    if (rewind) continue;
                                }
                                i++; line.size = InstructionCompiler::push_pointer(bytecode, address_value); _line_push;
                            }
                            if (type == type_bool) {
                                if (e_int) {
                                    bool rewind = false;
                                    if (buildErrorExpectedIntSameLine(token, token_p1, rewind)) return true;
                                    if (rewind) continue;
                                }
                                i++; line.size = InstructionCompiler::push_bool(bytecode, value_int != 0); _line_push;
                            }
                            if (type == type_u8) {
                                if (e_int) {
                                    bool rewind = false;
                                    if (buildErrorExpectedIntSameLine(token, token_p1, rewind)) return true;
                                    if (rewind) continue;
                                }
                                i++; line.size = InstructionCompiler::push_u8(bytecode, value_int);
                                _ir_set_const(IR_OP_U8, 1, value_int);
                                _line_push;
                            }
                            if (type == type_u16) {
                                if (e_int) {
                                    bool rewind = false;
                                    if (buildErrorExpectedIntSameLine(token, token_p1, rewind)) return true;
                                    if (rewind) continue;
                                }
                                i++; line.size = InstructionCompiler::push_u16(bytecode, value_int);
                                _ir_set_const(IR_OP_U16, 1, value_int);
                                _line_push;
                            }
                            if (type == type_u32) {
                                if (e_int) {
                                    bool rewind = false;
                                    if (buildErrorExpectedIntSameLine(token, token_p1, rewind)) return true;
                                    if (rewind) continue;
                                }
                                i++; line.size = InstructionCompiler::push_u32(bytecode, value_int);
                                _ir_set_const(IR_OP_U32, 1, value_int);
                                _line_push;
                            }
                            if (type == type_u64) {
                                if (e_int) {
                                    bool rewind = false;
                                    if (buildErrorExpectedIntSameLine(token, token_p1, rewind)) return true;
                                    if (rewind) continue;
                                }
                                i++; line.size = InstructionCompiler::push_u64(bytecode, value_int); _line_push;
                            }
                            if (type == type_i8) {
                                if (e_int) {
                                    bool rewind = false;
                                    if (buildErrorExpectedIntSameLine(token, token_p1, rewind)) return true;
                                    if (rewind) continue;
                                }
                                i++; line.size = InstructionCompiler::push_i8(bytecode, value_int); _line_push;
                            }
                            if (type == type_i16) {
                                if (e_int) {
                                    bool rewind = false;
                                    if (buildErrorExpectedIntSameLine(token, token_p1, rewind)) return true;
                                    if (rewind) continue;
                                }
                                i++; line.size = InstructionCompiler::push_i16(bytecode, value_int); _line_push;
                            }
                            if (type == type_i32) {
                                if (e_int) {
                                    bool rewind = false;
                                    if (buildErrorExpectedIntSameLine(token, token_p1, rewind)) return true;
                                    if (rewind) continue;
                                }
                                i++; line.size = InstructionCompiler::push_i32(bytecode, value_int); _line_push;
                            }
                            if (type == type_i64) {
                                if (e_int) {
                                    bool rewind = false;
                                    if (buildErrorExpectedIntSameLine(token, token_p1, rewind)) return true;
                                    if (rewind) continue;
                                }
                                i++; line.size = InstructionCompiler::push_i64(bytecode, value_int); _line_push;
                            }
                            if (type == type_f32) {
                                if (e_real) { if (buildErrorExpectedFloat(token_p1)) return true; }
                                i++; line.size = InstructionCompiler::push_f32(bytecode, value_float);
                                u32 f32_bits = 0; memcpy(&f32_bits, &value_float, 4);
                                _ir_set_const(IR_OP_F32, 1, f32_bits);
                                _line_push;
                            }
                            if (type == type_f64) {
                                if (e_real) { if (buildErrorExpectedFloat(token_p1)) return true; }
                                i++; line.size = InstructionCompiler::push_f64(bytecode, value_float);
                                double f64_val = (double) value_float;
                                u64 f64_bits = 0; memcpy(&f64_bits, &f64_val, 8);
                                _ir_set_const(IR_OP_F64, 1, f64_bits);
                                _line_push;
                            }
                            Serial.print(F("Error: unknown data type ")); token.print(); Serial.print(F(" at ")); Serial.print(token.line); Serial.print(F(":")); Serial.println(token.column);
                            if (buildErrorUnknownToken(token)) return true; continue;
                        }
                        if (hasNext && token.endsWith(".load_from")) {
                            int address_value = 0;
                            bool e_addr = addressFromToken(token_p1, address_value);
                            if (e_addr) {
                                bool rewind = false;
                                if (buildErrorExpectedIntSameLine(token, token_p1, rewind)) return true;
                                if (rewind) continue;
                            }
                            i++; line.size = InstructionCompiler::push_load_from(bytecode, type, address_value); _line_push;
                        }
                        if (hasNext && token.endsWith(".move_to")) {
                            int address_value = 0;
                            bool e_addr = addressFromToken(token_p1, address_value);
                            if (e_addr) {
                                bool rewind = false;
                                if (buildErrorExpectedIntSameLine(token, token_p1, rewind)) return true;
                                if (rewind) continue;
                            }
                            i++; line.size = InstructionCompiler::push_move_to(bytecode, type, address_value); _line_push;
                        }
                        if (hasNext && token.endsWith(".inc")) {
                            int address_value = 0;
                            bool e_addr = addressFromToken(token_p1, address_value);
                            if (e_addr) {
                                bool rewind = false;
                                if (buildErrorExpectedIntSameLine(token, token_p1, rewind)) return true;
                                if (rewind) continue;
                            }
                            i++; line.size = InstructionCompiler::push_inc(bytecode, type, address_value); _line_push;
                        }
                        if (hasNext && token.endsWith(".dec")) {
                            int address_value = 0;
                            bool e_addr = addressFromToken(token_p1, address_value);
                            if (e_addr) {
                                bool rewind = false;
                                if (buildErrorExpectedIntSameLine(token, token_p1, rewind)) return true;
                                if (rewind) continue;
                            }
                            i++; line.size = InstructionCompiler::push_dec(bytecode, type, address_value); _line_push;
                        }
                        // if (hasNext && token.endsWith(".load")) { if (e_int) return buildErrorExpectedInt(token_p1); i++; line.size = InstructionCompiler::pushGET(bytecode, value_int, type); _line_push; }
                        // if (hasNext && token.endsWith(".store")) { if (e_int) return buildErrorExpectedInt(token_p1); i++; line.size = InstructionCompiler::pushPUT(bytecode, value_int, type); _line_push; }
                        if (hasNext && token.endsWith(".load")) { line.size = InstructionCompiler::push_load(bytecode, type); _line_push; }
                        if (hasNext && token.endsWith(".move")) { line.size = InstructionCompiler::push_move(bytecode, type); _line_push; }
                        if (hasNext && token.endsWith(".move_copy")) { line.size = InstructionCompiler::push_move_copy(bytecode, type); _line_push; }
                        if (hasNext && token.endsWith(".copy")) { line.size = InstructionCompiler::push_copy(bytecode, type); _line_push; }
                        // if (hasNext && token.endsWith(".swap")) { line.size = InstructionCompiler::push_swap(bytecode, type); _line_push; }
                        if (hasNext && token.endsWith(".drop")) { line.size = InstructionCompiler::push_drop(bytecode, type); _line_push; }
                        // Pick value from stack at byte depth
                        if (hasNext && token.endsWith(".pick")) {
                            int depth_value = 0;
                            bool e_int = intFromToken(token_p1, depth_value);
                            if (e_int) {
                                bool rewind = false;
                                if (buildErrorExpectedIntSameLine(token, token_p1, rewind)) return true;
                                if (rewind) continue;
                            }
                            i++; line.size = InstructionCompiler::push_pick(bytecode, type, (MY_PTR_t)depth_value); _line_push;
                        }
                        // Poke (write) top value to stack at byte depth
                        if (hasNext && token.endsWith(".poke")) {
                            int depth_value = 0;
                            bool e_int = intFromToken(token_p1, depth_value);
                            if (e_int) {
                                bool rewind = false;
                                if (buildErrorExpectedIntSameLine(token, token_p1, rewind)) return true;
                                if (rewind) continue;
                            }
                            i++; line.size = InstructionCompiler::push_poke(bytecode, type, (MY_PTR_t)depth_value); _line_push;
                        }
                        if (token.endsWith(".cmp_lt")) { line.size = InstructionCompiler::push(bytecode, CMP_LT, type); _line_push; }
                        if (token.endsWith(".cmp_gt")) { line.size = InstructionCompiler::push(bytecode, CMP_GT, type); _line_push; }
                        if (token.endsWith(".cmp_eq")) { line.size = InstructionCompiler::push(bytecode, CMP_EQ, type); _line_push; }
                        if (token.endsWith(".cmp_neq")) { line.size = InstructionCompiler::push(bytecode, CMP_NEQ, type); _line_push; }
                        if (token.endsWith(".cmp_gte")) { line.size = InstructionCompiler::push(bytecode, CMP_GTE, type); _line_push; }
                        if (token.endsWith(".cmp_lte")) { line.size = InstructionCompiler::push(bytecode, CMP_LTE, type); _line_push; }

                        if (token.endsWith(".add")) { line.size = InstructionCompiler::push(bytecode, ADD, type); _line_push; }
                        if (token.endsWith(".sub")) { line.size = InstructionCompiler::push(bytecode, SUB, type); _line_push; }
                        if (token.endsWith(".mul")) { line.size = InstructionCompiler::push(bytecode, MUL, type); _line_push; }
                        if (token.endsWith(".div")) { line.size = InstructionCompiler::push(bytecode, DIV, type); _line_push; }
                        if (token.endsWith(".mod")) { line.size = InstructionCompiler::push(bytecode, MOD, type); _line_push; }
                        if (token.endsWith(".pow")) { line.size = InstructionCompiler::push(bytecode, POW, type); _line_push; }
                        if (token.endsWith(".sqrt")) { line.size = InstructionCompiler::push(bytecode, SQRT, type); _line_push; }
                        if (token.endsWith(".neg")) { line.size = InstructionCompiler::push(bytecode, NEG, type); _line_push; }
                        if (token.endsWith(".abs")) { line.size = InstructionCompiler::push(bytecode, ABS, type); _line_push; }
                        if (token.endsWith(".sin")) { line.size = InstructionCompiler::push(bytecode, SIN, type); _line_push; }
                        if (token.endsWith(".cos")) { line.size = InstructionCompiler::push(bytecode, COS, type); _line_push; }
                    }
                }
            }

            // [cvt , keyword , keyword]
            if (hasThird) {
                u8 type_1;
                u8 type_2;
                bool e_dataType1 = typeFromToken(token_p1, type_1);
                bool e_dataType2 = typeFromToken(token_p2, type_2);
                if (token == "cvt") { // Convert from one type to another
                    if (e_dataType1) { if (e_dataType1) return true; } i++;
                    if (e_dataType2) { if (e_dataType2) return true; } i++;
                    if (type_1 == type_2) continue; // No need to convert if types are the same
                    line.size = InstructionCompiler::push_cvt(bytecode, (PLCRuntimeInstructionSet) type_1, (PLCRuntimeInstructionSet) type_2);
                    _line_push;
                }
                if (token == "swap") { // Swap two values on the stack of any combination of types
                    if (e_dataType1) { if (e_dataType1) return true; } i++;
                    if (e_dataType2) { if (e_dataType2) return true; } i++;
                    line.size = InstructionCompiler::push_swap(bytecode, (PLCRuntimeInstructionSet) type_1, (PLCRuntimeInstructionSet) type_2);
                    _line_push;

                }
            }

            // if (type == TOKEN_KEYWORD && token == "u8.loadI") {
            //     if (!hasNext || !hasThird) return buildError(token_p1, "requires numeric parameter");
            //     bool nextHasDot = str_includes(token_p1, ".");
            //     i += 1;
            // }

            if (type == TOKEN_KEYWORD && token == "exit") {
                line.size = InstructionCompiler::push(bytecode, EXIT);
                _line_push;
            }

            if (buildErrorUnknownToken(token)) return true; continue;
        }
        for (int i = 0; i < LUT_label_count; i++) {
            LUT_label& label = LUT_labels[i];
            if (label.address == -1) {
                Serial.print(F("Error: label ")); label.string.print(); Serial.print(F(" is not defined"));
                return true;
            }
        }
        return false;
    }


    void loadAssembly() {
        int size = 0;
        streamRead(assembly_string, size, MAX_ASSEMBLY_STRING_SIZE);
    }

    void logBytecode() {
        Serial.print(F("Bytecode checksum ")); print_hex(built_bytecode_checksum); Serial.print(F(", ")); Serial.print(built_bytecode_length); Serial.println(F(" bytes:"));
        for (int i = 0; i < built_bytecode_length; i++) {
            u8 byte = built_bytecode[i]; // Format it as hex
            char c1, c2;
            byteToHex(byte, c1, c2);
            Serial.print(F(" ")); Serial.print(c1); Serial.print(c2);
        }
        Serial.println();
    }


    // Check if I missed functions: uploadProgram, downloadProgram

    u32 uploadProgram() {
        for (int i = 0; i < built_bytecode_length; i++) {
            u8 byte = built_bytecode[i]; // Format it as hex
            char c1, c2;
            byteToHex(byte, c1, c2);
            streamOut(c1);
            streamOut(c2);
        }
        return built_bytecode_length;
    }

    int downloadProgram(int size, int crc);

    // loadCompiledProgram
    // Needs access to runtime global? Wait, loadCompiledProgram in original code called runtime.loadProgram
    // User said: "loadCompiledProgram (from WASM_EXPORT below, implementation needs to be in class to access built_bytecode)."
    bool loadCompiledProgram(); // Impl needs access to built_bytecode which is member.

    bool compileAssembly(bool debug = true, bool lintMode = false) {
        num_of_compile_runs++;
        if (num_of_compile_runs == 1) debug = true;
        long total = millis();
        if (num_of_compile_runs == 1) {
            VovkPLCRuntime::splash();
        }
        if (debug) Serial.println(F("VovkPLCRuntime - PLCASM Compiler v1.0"));
        if (debug) Serial.print(F("Compiling assembly to bytecode "));
        bool error = false;
        if (debug) Serial.print(F("."));
        long t1 = millis();
        error = tokenize();
        t1 = millis() - t1;
        if (error) { Serial.println(F("Failed at tokenization"));  return error; }

        if (debug) Serial.print(F("."));
        t1 = millis();
        error = build(false, lintMode); // First pass
        t1 = millis() - t1;
        if (error) { Serial.println(F("Failed at building"));  return error; }


        if (debug) Serial.print(F("."));
        t1 = millis();
        error = build(true, lintMode); // Second pass to link labels
        t1 = millis() - t1;
        if (error) { Serial.println(F("Failed at linking"));  return error; }

        total = millis() - total;
        if (debug) { Serial.print(F(" finished in ")); Serial.print(total); Serial.println(F(" ms")); }

        if (debug) {
            if (symbol_count > 0) {
                Serial.print(F("Symbols ")); Serial.print(symbol_count); Serial.println(F(":"));
                for (int i = 0; i < symbol_count; i++) {
                    Serial.print(F("    "));
                    symbols[i].print();
                    Serial.println();
                }
            } else Serial.println(F("No symbols"));

            if (LUT_label_count > 0) {
                Serial.print(F("Labels ")); Serial.print(LUT_label_count); Serial.println(F(":"));
                for (int i = 0; i < LUT_label_count; i++) {
                    LUT_label& label = LUT_labels[i];
                    Serial.print(F("    ")); label.string.print(); Serial.print(F(" = ")); Serial.println(label.address);
                }
            } else Serial.println(F("No labels"));

            if (LUT_const_count > 0) {
                Serial.print(F("Consts ")); Serial.print(LUT_const_count); Serial.println(F(":"));
                for (int i = 0; i < LUT_const_count; i++) {
                    LUT_const& c = LUT_consts[i];
                    Serial.print(F("    ")); c.string.print(); Serial.print(F(" <"));
                    switch (c.type) {
                        case VAL_BOOLEAN: Serial.print(F("bool")); break;
                        case VAL_INTEGER: Serial.print(F("int")); break;
                        case VAL_REAL: Serial.print(F("real")); break;
                        case VAL_STRING: Serial.print(F("string")); break;
                        default: Serial.print(F("unknown")); break;
                    }
                    Serial.print(F("> = "));
                    c.value_string.print();
                    Serial.print(F(" => "));
                    if (c.type == VAL_BOOLEAN) Serial.print(c.value_bool ? "true" : "false");
                    if (c.type == VAL_INTEGER) Serial.print(c.value_int);
                    if (c.type == VAL_REAL) Serial.print(c.value_float);
                    if (c.type == VAL_STRING) c.value_string.print();
                    Serial.println();
                }
            } else Serial.println(F("No consts"));

            logBytecode();
        }

        return false;
    }
};

// Include linter after compiler class is defined
#include "plcasm-linter.h"

// Singleton wrapper for runtime
VovkPLCRuntime& getDefaultRuntime() {

    static VovkPLCRuntime instance;
    return instance;
}
#define runtime getDefaultRuntime()

// VovkPLCRuntime runtime;

// Implementation of methods relying on 'runtime'

bool PLCASMCompiler::loadCompiledProgram() {
    Serial.printf("Loading program with %d bytes and checksum 0x%02X ...\n", built_bytecode_length, built_bytecode_checksum);
    runtime.loadProgram(built_bytecode, built_bytecode_length, built_bytecode_checksum);
    return false;
}


int PLCASMCompiler::downloadProgram(int size, int crc) {
    streamRead(downloaded_program, downloaded_program_size, MAX_DOWNLOADED_PROGRAM_SIZE);
    if (downloaded_program_size != size) return 1;
    u8 calculated_crc = 0;
    for (u32 i = 0; i < size; i++) {
        u8 byte = downloaded_program[i];
        crc8_simple(calculated_crc, byte);
    }
    if (calculated_crc != crc) return 2;
    runtime.loadProgram(downloaded_program, downloaded_program_size, calculated_crc);
    return 0;
}

// Singleton wrapper to prevent re-initialization
PLCASMCompiler& getDefaultCompiler() {
    static PLCASMCompiler instance;
    return instance;
}
#define defaultCompiler getDefaultCompiler()

// PLCASMCompiler defaultCompiler;


WASM_EXPORT void loadAssembly() {
    defaultCompiler.loadAssembly();
}

WASM_EXPORT void loadAssemblyForLint() {
    getDefaultLinter().loadAssembly();
}

WASM_EXPORT void clearLinter() {
    getDefaultLinter().clearArray();
}

WASM_EXPORT void setLinterAssembly(char* s) {
    getDefaultLinter().set_assembly_string(s);
}

WASM_EXPORT void copyAssemblyToLinter() {
    getDefaultLinter().set_assembly_string(defaultCompiler.assembly_string);
}

WASM_EXPORT void logBytecode() {
    defaultCompiler.logBytecode();
}

WASM_EXPORT bool compileAssembly(bool debug = true) {
    return defaultCompiler.compileAssembly(debug, false);
}

// IR (Intermediate Representation) exports for Front-End editor
WASM_EXPORT int ir_get_count() {
    return defaultCompiler.ir_entry_count;
}

WASM_EXPORT IR_Entry* ir_get_pointer() {
    return defaultCompiler.ir_entries;
}

WASM_EXPORT int ir_get_entry_size() {
    return sizeof(IR_Entry);
}

WASM_EXPORT int ir_get_labels_count() {
    return defaultCompiler.LUT_label_count;
}

WASM_EXPORT int ir_get_consts_count() {
    return defaultCompiler.LUT_const_count;
}

WASM_EXPORT bool lintAssembly(bool debug = true) {
    return getDefaultLinter().compileAssembly(debug, true);
}

WASM_EXPORT int getCompiledBytecodeLength() {
    return defaultCompiler.built_bytecode_length;
}

WASM_EXPORT void initialize() {
    runtime.initialize();
    IntervalReset();
}

WASM_EXPORT void printProperties() {
    runtime.printProperties();
}
WASM_EXPORT void printInfo() {
    runtime.printInfo();
}

// ============ Direct Info Getter (buffer-based API for WASM) ============
// Returns pointer to a static string with same format as printInfo() serial output
// Format: [VovkPLCRuntime,ARCH,MAJOR,MINOR,PATCH,BUILD,DATE,STACK,MEM,PROG,K_OFF,K_SZ,X_OFF,X_SZ,Y_OFF,Y_SZ,S_OFF,S_SZ,M_OFF,M_SZ,T_OFF,T_CNT,T_SZ,C_OFF,C_CNT,C_SZ,DEVICE]
static char info_buffer[512];

WASM_EXPORT const char* getInfoString() {
    sprintf(info_buffer, "[VovkPLCRuntime,%s,%d,%d,%d,%d,%s,%d,%d,%d,%u,%d,%u,%d,%u,%d,%u,%d,%u,%d,%d,%u,%d,%d,%s]",
        VOVKPLC_ARCH,
        VOVKPLCRUNTIME_VERSION_MAJOR,
        VOVKPLCRUNTIME_VERSION_MINOR,
        VOVKPLCRUNTIME_VERSION_PATCH,
        VOVKPLCRUNTIME_VERSION_BUILD,
        __ISO_TIMESTAMP__,
        PLCRUNTIME_MAX_STACK_SIZE,
        PLCRUNTIME_MAX_MEMORY_SIZE,
        PLCRUNTIME_MAX_PROGRAM_SIZE,
        runtime.system_offset,
        PLCRUNTIME_NUM_OF_SYSTEMS,
        runtime.input_offset,
        PLCRUNTIME_NUM_OF_INPUTS,
        runtime.output_offset,
        PLCRUNTIME_NUM_OF_OUTPUTS,
        runtime.marker_offset,
        PLCRUNTIME_NUM_OF_MARKERS,
        runtime.timer_offset,
        PLCRUNTIME_NUM_OF_TIMERS,
        PLCRUNTIME_TIMER_STRUCT_SIZE,
        runtime.counter_offset,
        PLCRUNTIME_NUM_OF_COUNTERS,
        PLCRUNTIME_COUNTER_STRUCT_SIZE,
        VOVKPLC_DEVICE_NAME
    );
    return info_buffer;
}

WASM_EXPORT void setRuntimeOffsets(u32 systemOffset, u32 inputOffset, u32 outputOffset, u32 markerOffset) {
    plcasm_system_offset = systemOffset;
    plcasm_input_offset = inputOffset;
    plcasm_output_offset = outputOffset;
    plcasm_marker_offset = markerOffset;
    // Timer and counter offsets are auto-calculated based on marker offset
    plcasm_timer_offset = markerOffset + PLCRUNTIME_NUM_OF_MARKERS;
    plcasm_counter_offset = plcasm_timer_offset + (PLCRUNTIME_NUM_OF_TIMERS * PLCRUNTIME_TIMER_STRUCT_SIZE);
    runtime.system_offset = systemOffset;
    runtime.input_offset = inputOffset;
    runtime.output_offset = outputOffset;
    runtime.marker_offset = markerOffset;
    runtime.timer_offset = plcasm_timer_offset;
    runtime.counter_offset = plcasm_counter_offset;
}

// Extended version with timer and counter offsets
WASM_EXPORT void setRuntimeOffsetsEx(u32 systemOffset, u32 inputOffset, u32 outputOffset, u32 markerOffset, u32 timerOffset, u32 counterOffset) {
    plcasm_system_offset = systemOffset;
    plcasm_input_offset = inputOffset;
    plcasm_output_offset = outputOffset;
    plcasm_marker_offset = markerOffset;
    plcasm_timer_offset = timerOffset;
    plcasm_counter_offset = counterOffset;
    runtime.system_offset = systemOffset;
    runtime.input_offset = inputOffset;
    runtime.output_offset = outputOffset;
    runtime.marker_offset = markerOffset;
    runtime.timer_offset = timerOffset;
    runtime.counter_offset = counterOffset;
}

WASM_EXPORT bool loadCompiledProgram() {
    return defaultCompiler.loadCompiledProgram();
}

WASM_EXPORT void runFullProgramDebug() {
    IntervalGlobalLoopCheck();
    RuntimeError status = UnitTest::fullProgramDebug(runtime);
    const char* status_name = RUNTIME_ERROR_NAME(status);
    Serial.print(F("Runtime status: ")); Serial.println(status_name);
}

WASM_EXPORT void runFullProgram() {
    IntervalGlobalLoopCheck();
    RuntimeError status = runtime.run();
    const char* status_name = RUNTIME_ERROR_NAME(status);
    Serial.print(F("Runtime status: ")); Serial.println(status_name);
}

WASM_EXPORT u32 uploadProgram() {
    return defaultCompiler.uploadProgram();
}

WASM_EXPORT int downloadProgram(int size, int crc) {
    return defaultCompiler.downloadProgram(size, crc);
}

WASM_EXPORT u32 getMemoryArea(u32 address, u32 size) {
    u32 end = address + size;
    if (end > PLCRUNTIME_MAX_STACK_SIZE) end = PLCRUNTIME_MAX_STACK_SIZE;
    u8 byte = 0;
    bool error = false;
    for (u32 i = address; i < end; i++) {
        if (i > address) streamOut(' ');
        // error = runtime.memory->get(i, byte); // Format it as hex
        error = get_u8(runtime.memory, i, byte);
        if (error) return 0;
        char c1, c2;
        byteToHex(byte, c1, c2);
        streamOut(c1);
        streamOut(c2);
    }
    return size;
}

WASM_EXPORT u32 writeMemoryByte(u32 address, u8 byte) {
    // bool error = runtime.memory->set(address, byte);
    bool error = set_u8(runtime.memory, address, byte);
    if (error) return 0;
    return 1;
}

WASM_EXPORT u32 writeMemoryByteMasked(u32 address, u8 byte, u8 mask) {
    u8 current = 0;
    bool error = get_u8(runtime.memory, address, current);
    if (error) return 0;
    u8 next = (current & ~mask) | (byte & mask);
    error = set_u8(runtime.memory, address, next);
    if (error) return 0;
    return 1;
}

WASM_EXPORT void external_print(const char* string) {
    Serial.print(string);
}

void set_assembly_string(char* s) { defaultCompiler.set_assembly_string(s); }

#else

void set_assembly_string(char* new_assembly_string) {}
bool compileAssembly() { return false; }
void runFullProgramDebug() {}
u32 uploadProgram() { return 0; }
u32 getMemoryArea(u32 address, u32 size) { return 0; }
u32 writeMemoryByte(u32 address, u8 byte) { return 0; }
u32 writeMemoryByteMasked(u32 address, u8 byte, u8 mask) { return 0; }

#endif // __WASM__
