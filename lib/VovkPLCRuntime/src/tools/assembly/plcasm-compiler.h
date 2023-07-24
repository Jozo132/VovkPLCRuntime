// plcasm-compiler.h - 1.0.0 - 2022-12-11
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

#include "wasm/wasm.h"

#ifdef __WASM__


#define max_assembly_string_size 64535
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
    program.push_float(0.1);
    program.push_float(0.2);
    program.push(ADD, type_float);
    program.push_float(-1);
    program.push(MUL, type_float);
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

// Part 1: Reading the assembly
char assembly_string[max_assembly_string_size] = R"(
# This is a comment
    f32.const 0.1
    f32.const 0.2
    f32.add
    f32.const -1
    f32.mul
)";

void set_assembly_string(char* new_assembly_string) {
    const int size = string_len(new_assembly_string);
    if (size == 0) {
        printf("Error: input assembly is empty.\n");
        return;
    }
    if (size > max_assembly_string_size) {
        printf("Error: input assembly is too long (%d characters). Max size is %d characters.\n", size, max_assembly_string_size);
        return;
    }
    string_copy(assembly_string, new_assembly_string);
}

// ################################################################################################

// Part 2: Tokenization
struct StringView {
    char* data;
    int length;
    int print() {
        for (int i = 0; i < length; i++) {
            printf("%c", data[i]);
        }
        return length;
    }
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
    int print() {
        if (type == TOKEN_BOOLEAN) return printf("%s", value_bool ? "true" : "false");
        if (type == TOKEN_INTEGER) return printf("%d", value_int);
        if (type == TOKEN_REAL) return printf("%f", value_float);
        if (type == TOKEN_STRING) return printf("'") + string.print() + printf("'");
        return printf("\"") + string.print() + printf("\"");
    }
    void parse();
};

bool str_cmp(StringView a, StringView b) {
    if (a.length != b.length) return false;
    for (int i = 0; i < a.length; i++) {
        if (a.data[i] != b.data[i]) return false;
    }
    return true;
}

bool str_cmp(StringView a, const char* b) {
    int b_length = string_len(b);
    if (a.length != b_length) return false;
    for (int i = 0; i < a.length; i++) {
        if (a.data[i] != b[i]) return false;
    }
    return true;
}

bool str_cmp(Token a, StringView b) {
    if (a.length != b.length) return false;
    for (int i = 0; i < a.length; i++) {
        if (a.string.data[i] != b.data[i]) return false;
    }
    return true;
}

bool str_cmp(Token a, const char* b) {
    int b_length = string_len(b);
    if (a.length != b_length) return false;
    for (int i = 0; i < a.length; i++) {
        if (a.string.data[i] != b[i]) return false;
    }
    return true;
}

bool str_cmp(Token a, Token b) {
    if (a.length != b.length) return false;
    for (int i = 0; i < a.length; i++) {
        if (a.string.data[i] != b.string.data[i]) return false;
    }
    return true;
}



bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isBoolean(StringView string, bool& output) {
    if (string.length == 0) return false;
    if (str_cmp(string, "true")) {
        output = true;
        return true;
    }
    if (str_cmp(string, "false")) {
        output = false;
        return true;
    }
    if (string.length == 1) {
        if (string.data[0] == '1') {
            output = true;
            return true;
        }
        if (string.data[0] == '0') {
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
    if (string.data[0] == '-') {
        is_negative = true;
        i++;
    }
    for (; i < string.length && string.data[i] != '\0'; i++) {
        if (!isDigit(string.data[i])) {
            return false;
        }
        number = number * 10 + (string.data[i] - '0');
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
    if (string.data[0] == '-') {
        is_negative = true;
        i++;
    }
    for (; i < string.length && string.data[i] != '\0'; i++) {
        if (string.data[i] == '.') {
            if (dot) return false;
            dot = true;
            continue;
        }
        if (!isDigit(string.data[i])) return false;
        if (dot) decimal_places++;
        number = number * 10.0 + float(string.data[i] - '0');
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
        char c = string.data[0];
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '&' || c == '|' || c == '^' || c == '~' || c == '!' || c == '<' || c == '>' || c == '?' || c == ':' || c == ',' || c == ';' || c == '[' || c == ']' || c == '{' || c == '}' || c == '\'' || c == '"' || c == '`' || c == '\\') {
            this->type = TOKEN_OPERATOR;
            return;
        }
    }
    if (str_cmp(string, "==") || str_cmp(string, "!=") || str_cmp(string, "<>") || str_cmp(string, "<=") || str_cmp(string, ">=") || str_cmp(string, "&&") || str_cmp(string, "||")) {
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


Token tokens[MAX_NUM_OF_TOKENS] = { };
int token_count = 0;
int token_count_temp = 0;

int line = 1;
int column = 1;



struct LUT_label {
    StringView string;
    int address;
};

struct LUT_label LUT_labels[MAX_NUM_OF_TOKENS] = { };
int LUT_label_count = 0;

struct LUT_const {
    int address;
    StringView string;
    ValueType type;
    bool value_bool;
    int value_int;
    float value_float;
    StringView value_string;
};

struct LUT_const LUT_consts[MAX_NUM_OF_TOKENS] = { };
int LUT_const_count = 0;

// List of illegal keywords
const char* illegal_keywords [] = { "const", "var", "i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f32", "f64", "bool", "string", "true", "false", "if", "else", "while", "for", "do", "break", "continue", "return", "function" };
constexpr int illegal_keywords_count = sizeof(illegal_keywords) / sizeof(illegal_keywords[0]);

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
    bool isNumber = token.type == TOKEN_INTEGER || token.type == TOKEN_REAL;
    if (token_count_temp >= 2 && isNumber) { // handle negative numbers
        Token& p1_token = tokens[token_count_temp - 1];
        Token& p2_token = tokens[token_count_temp - 2];
        TokenType p1_type = p1_token.type;
        TokenType p2_type = p2_token.type;
        // If [keyword , - , number] then change to [keyword , -number]

        bool skip = false;
        if (p1_type == TOKEN_OPERATOR && p2_type == TOKEN_KEYWORD) {
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
        if (str_cmp(token, "'") && str_cmp(p2_token, "'")) {
            p2_token.type = TOKEN_STRING;
            p2_token.string = p1_token.string;
            token_count_temp--;
            token_count_temp--;
            return false;
        }
    }
    if (token_count_temp >= 1) {
        Token& p1_token = tokens[token_count_temp - 1];
        TokenType p1_type = p1_token.type;
        // If [keyword , :] then change to [label]
        if (p1_type == TOKEN_KEYWORD) {
            if (token.type == TOKEN_OPERATOR && str_cmp(token, ":")) {
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

const char lex_ignored [] = { ' ', ';', '\t', '\r', '\n' };
const char lex_dividers [] = { '(', ')', '=', '+', '-', '*', '/', '%', '&', '|', '^', '~', '!', '<', '>', '?', ':', ',', ';', '[', ']', '{', '}', '\'', '"', '`', '\\' };

bool tokenize() {
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

        // c == # || c == /
        if (c == '#' || c == '/') {
            error = add_token_optional(token_start, token_length);
            if (error) return error;
            while (i < assembly_string_length && assembly_string[i] != '\n') i++;
            token_start = assembly_string + i + 1;
            token_length = 0;
            line++;
            column = 1;
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
    token_count = token_count_temp;
    token_count_temp = 0;
    line = 1;
    column = 1;
    return false;
}


// data type keywords
const char* data_type_keywords [] = { "i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f32", "f64", "bool", "string" };
const char* op_type_keywords [] = { "const", "put", "get", "cmp_gt", "cmp_lt", "cmp_eq", "cmp_ne", "cmp_ge", "cmp_le", "cmp_and", "cmp_or", "cmp_xor", "cmp_not", "add", "sub", "mul", "div", "mod", "and", "or", "xor", "not", "shl", "shr", "rol", "ror", "neg", "inc", "dec", "jmp", "jz", "jnz", "call", "ret", "nop", "halt" };

#define __MAX_BUILT_BYTECODE_SIZE 1024

uint8_t built_bytecode[__MAX_BUILT_BYTECODE_SIZE] = { };
int built_bytecode_length = 0;

struct ProgramLine {
    uint8_t code[16];
    int index;
    int size;
    Token* refToken;
};

ProgramLine programLines[__MAX_BUILT_BYTECODE_SIZE] = { };
int programLineCount = 0;

#define _line_push \
    int address_end = built_bytecode_length + line.size; \
    if (address_end >= __MAX_BUILT_BYTECODE_SIZE) error = true; \
    if (error) return error; \
    for (int j = 0; j < line.size; j++) built_bytecode[built_bytecode_length + j] = bytecode[j]; \
    built_bytecode_length = address_end; \
    continue;


bool typeFromToken(Token& token, uint8_t& type) {
    if (token.type == TOKEN_KEYWORD) {
        // "u8" = type_uint8_t
        for (int i = 0; i < sizeof(data_type_keywords) / sizeof(data_type_keywords[0]); i++) {
            if (str_cmp(token, data_type_keywords[i])) {
                switch (i) {
                    case 0:  type = type_int8_t; break;
                    case 1:  type = type_int16_t; break;
                    case 2:  type = type_int32_t; break;
                    case 3:  type = type_int64_t; break;
                    case 4:  type = type_uint8_t; break;
                    case 5:  type = type_uint16_t; break;
                    case 6:  type = type_uint32_t; break;
                    case 7:  type = type_uint64_t; break;
                    case 8:  type = type_float; break;
                    case 9:  type = type_double; break;
                    case 10: type = type_bool; break;
                        // case 11: type = type_string; break;
                    default: return true;
                }
                return false;
            }
        }
    }
    return true;
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
                    output = c.value_bool;
                    return false;
                }
                if (c.type == VAL_INTEGER) {
                    output = c.value_int != 0;
                    return false;
                }
                if (c.type == VAL_REAL) {
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
                    output = c.value_bool;
                    return false;
                }
                if (c.type == VAL_INTEGER) {
                    output = c.value_int;
                    return false;
                }
                if (c.type == VAL_REAL) {
                    output = c.value_float;
                    return false;
                }
                return true;
            }
        }
    }
    return true;
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
                    output = c.value_bool;
                    return false;
                }
                if (c.type == VAL_INTEGER) {
                    output = c.value_int;
                    return false;
                }
                if (c.type == VAL_REAL) {
                    output = c.value_float;
                    return false;
                }
                return true;
            }
        }
    }
    return true;
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

bool build(bool finalPass) {
    bool error = false;
    programLineCount = 0;
    built_bytecode_length = 0;
    for (int i = 0; i < token_count; i++) {
        Token& token = tokens[i];
        TokenType type = token.type;
        if (type == TOKEN_UNKNOWN) error = true;
        if (error) return error;

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

        ProgramLine& line = programLines[programLineCount];
        line.index = built_bytecode_length;
        line.refToken = &token;
        uint8_t* bytecode = line.code;

        bool hasNext = i + 1 < token_count;
        // [ u8.const , 3 ]
        Token& token_p1 = hasNext ? tokens[i + 1] : tokens[i];
        bool e_label = finalPass ? labelFromToken(token_p1, label_address) : false;
        // bool e_bool = boolFromToken(token_p1, value_bool);
        bool e_int = intFromToken(token_p1, value_int);
        bool e_real = realFromToken(token_p1, value_float);

        if (type == TOKEN_KEYWORD) {
            { // Handle flow
                if (hasNext && str_cmp(token, "jmp")) { if (finalPass && e_label) return e_label; i++; line.size = InstructionCompiler::pushJMP(bytecode, label_address); _line_push; }
                if (hasNext && str_cmp(token, "jmp_if")) { if (finalPass && e_label) return e_label; i++; line.size = InstructionCompiler::pushJMP_IF(bytecode, label_address); _line_push; }
                if (hasNext && str_cmp(token, "jmp_if_not")) { if (finalPass && e_label) return e_label; i++; line.size = InstructionCompiler::pushJMP_IF_NOT(bytecode, label_address); _line_push; }
                // if (hasNext && str_cmp(token, "call")) { if (e_int) return e_int; i++; line.size = InstructionCompiler::pushCALL(bytecode, value_int); _line_push; }
                // if (str_cmp(token, "ret")) { line.size = InstructionCompiler::pushRET(bytecode); _line_push; }
                if (str_cmp(token, "nop")) { line.size = InstructionCompiler::push(bytecode, NOP); _line_push; }
            }

            { // Handle Bit operations (PLC specific)
                if (str_cmp(token, "u8.0.get")) { line.size = InstructionCompiler::push(bytecode, GET_X8_B0); _line_push; }
                if (str_cmp(token, "u8.1.get")) { line.size = InstructionCompiler::push(bytecode, GET_X8_B1); _line_push; }
                if (str_cmp(token, "u8.2.get")) { line.size = InstructionCompiler::push(bytecode, GET_X8_B2); _line_push; }
                if (str_cmp(token, "u8.3.get")) { line.size = InstructionCompiler::push(bytecode, GET_X8_B3); _line_push; }
                if (str_cmp(token, "u8.4.get")) { line.size = InstructionCompiler::push(bytecode, GET_X8_B4); _line_push; }
                if (str_cmp(token, "u8.5.get")) { line.size = InstructionCompiler::push(bytecode, GET_X8_B5); _line_push; }
                if (str_cmp(token, "u8.6.get")) { line.size = InstructionCompiler::push(bytecode, GET_X8_B6); _line_push; }
                if (str_cmp(token, "u8.7.get")) { line.size = InstructionCompiler::push(bytecode, GET_X8_B7); _line_push; }

                if (str_cmp(token, "u8.0.set")) { line.size = InstructionCompiler::push(bytecode, SET_X8_B0); _line_push; }
                if (str_cmp(token, "u8.1.set")) { line.size = InstructionCompiler::push(bytecode, SET_X8_B1); _line_push; }
                if (str_cmp(token, "u8.2.set")) { line.size = InstructionCompiler::push(bytecode, SET_X8_B2); _line_push; }
                if (str_cmp(token, "u8.3.set")) { line.size = InstructionCompiler::push(bytecode, SET_X8_B3); _line_push; }
                if (str_cmp(token, "u8.4.set")) { line.size = InstructionCompiler::push(bytecode, SET_X8_B4); _line_push; }
                if (str_cmp(token, "u8.5.set")) { line.size = InstructionCompiler::push(bytecode, SET_X8_B5); _line_push; }
                if (str_cmp(token, "u8.6.set")) { line.size = InstructionCompiler::push(bytecode, SET_X8_B6); _line_push; }
                if (str_cmp(token, "u8.7.set")) { line.size = InstructionCompiler::push(bytecode, SET_X8_B7); _line_push; }

                if (str_cmp(token, "u8.0.rset")) { line.size = InstructionCompiler::push(bytecode, RSET_X8_B0); _line_push; }
                if (str_cmp(token, "u8.1.rset")) { line.size = InstructionCompiler::push(bytecode, RSET_X8_B1); _line_push; }
                if (str_cmp(token, "u8.2.rset")) { line.size = InstructionCompiler::push(bytecode, RSET_X8_B2); _line_push; }
                if (str_cmp(token, "u8.3.rset")) { line.size = InstructionCompiler::push(bytecode, RSET_X8_B3); _line_push; }
                if (str_cmp(token, "u8.4.rset")) { line.size = InstructionCompiler::push(bytecode, RSET_X8_B4); _line_push; }
                if (str_cmp(token, "u8.5.rset")) { line.size = InstructionCompiler::push(bytecode, RSET_X8_B5); _line_push; }
                if (str_cmp(token, "u8.6.rset")) { line.size = InstructionCompiler::push(bytecode, RSET_X8_B6); _line_push; }
                if (str_cmp(token, "u8.7.rset")) { line.size = InstructionCompiler::push(bytecode, RSET_X8_B7); _line_push; }
            }

            { // Handle u8
                if (hasNext && str_cmp(token, "u8.load")) { if (e_int) return e_int; i++; line.size = InstructionCompiler::pushGET(bytecode, value_int, type_uint8_t); _line_push; }
                if (hasNext && str_cmp(token, "u8.store")) { if (e_int) return e_int; i++; line.size = InstructionCompiler::pushPUT(bytecode, value_int, type_uint8_t); _line_push; }
                if (hasNext && str_cmp(token, "u8.const")) { if (e_int) return e_int; i++; line.size = InstructionCompiler::push_uint8_t(bytecode, value_int); _line_push; }
                if (str_cmp(token, "u8.cmp_lt")) { line.size = InstructionCompiler::push(bytecode, CMP_LT, type_uint8_t); _line_push; }
                if (str_cmp(token, "u8.cmp_gt")) { line.size = InstructionCompiler::push(bytecode, CMP_GT, type_uint8_t); _line_push; }
                if (str_cmp(token, "u8.cmp_eq")) { line.size = InstructionCompiler::push(bytecode, CMP_EQ, type_uint8_t); _line_push; }
                if (str_cmp(token, "u8.cmp_neq")) { line.size = InstructionCompiler::push(bytecode, CMP_NEQ, type_uint8_t); _line_push; }
                if (str_cmp(token, "u8.cmp_gte")) { line.size = InstructionCompiler::push(bytecode, CMP_GTE, type_uint8_t); _line_push; }
                if (str_cmp(token, "u8.cmp_lte")) { line.size = InstructionCompiler::push(bytecode, CMP_LTE, type_uint8_t); _line_push; }
                if (str_cmp(token, "u8.add")) { line.size = InstructionCompiler::push(bytecode, ADD, type_uint8_t); _line_push; }
                if (str_cmp(token, "u8.sub")) { line.size = InstructionCompiler::push(bytecode, SUB, type_uint8_t); _line_push; }
                if (str_cmp(token, "u8.mul")) { line.size = InstructionCompiler::push(bytecode, MUL, type_uint8_t); _line_push; }
                if (str_cmp(token, "u8.div")) { line.size = InstructionCompiler::push(bytecode, DIV, type_uint8_t); _line_push; }
            }
        }

        bool hasThird = i + 2 < token_count;
        Token& token_p2 = hasThird ? tokens[i + 2] : tokens[i];
        // [cvt , keyword , keyword]
        if (hasThird) {
            uint8_t type_1;
            uint8_t type_2;
            bool e_dataType1 = typeFromToken(token_p1, type_1);
            bool e_dataType2 = typeFromToken(token_p2, type_2);
            if (str_cmp(token, "cvt")) {
                if (e_dataType1) return e_dataType1; i++;
                if (e_dataType2) return e_dataType2; i++;
                if (type_1 == type_2) continue; // No need to convert if types are the same
                line.size = InstructionCompiler::pushCVT(bytecode, (PLCRuntimeInstructionSet) type_1, (PLCRuntimeInstructionSet) type_2);
                _line_push;
            }
        }


        if (type == TOKEN_KEYWORD && str_cmp(token, "exit")) {
            line.size = InstructionCompiler::push(bytecode, EXIT);
            _line_push;
        }

        Serial.print(F("Error: unknown token ")); token.print(); Serial.print(F(" at ")); Serial.print(token.line); Serial.print(F(":")); Serial.println(token.column);
        return true;
    }
    for (int i = 0; i < LUT_label_count; i++) {
        LUT_label& label = LUT_labels[i];
        if (label.address == -1) {
            Serial.print(F("Error: label ")); label.string.print(); Serial.print(F(" is not defined"));
            return true;
        }
    }
    return error;
}


WASM_EXPORT void loadAssembly() {
    int size = 0;
    streamRead(assembly_string, size, max_assembly_string_size);
}

WASM_EXPORT bool compileTest() {
    long total = millis();
    Serial.println(F("Compiling ..."));
    bool error = false;
    Serial.print(F(" - Tokenization "));
    long t1 = millis();
    error = tokenize();
    t1 = millis() - t1;
    if (error) { Serial.println(F("FAILED"));  return error; }
    Serial.print(F("OK (")); Serial.print(t1); Serial.println(F(" ms)"));


    // if (token_count > 0) {
    //     Serial.print(F("Tokens ")); Serial.print(token_count); Serial.println(F(":"));
    //     for (int i = 0; i < token_count; i++) {
    //         Token& token = tokens[i];
    //         TokenType type = token.type;
    //         Serial.print(F("    "));
    //         fill(' ', 4 - Serial.print(i));
    //         int s = Serial.print(F(" (")) + Serial.print(token.line) + Serial.print(F(":")) + Serial.print(token.column) + Serial.print(F(") "));
    //         fill(' ', 10 - s);
    //         Serial.print(F(" "));
    //         fill(' ', 12 - printTokenType(type));
    //         token.print();
    //         Serial.println();
    //     }
    // } else Serial.println(F("No tokens"));

    Serial.print(F(" - Building "));
    t1 = millis();
    error = build(false); // First pass
    t1 = millis() - t1;
    if (error) { Serial.println(F("FAILED"));  return error; }
    Serial.print(F("OK (")); Serial.print(t1); Serial.println(F(" ms)"));


    Serial.print(F(" - Linking "));
    t1 = millis();
    error = build(true); // Second pass to link labels
    t1 = millis() - t1;
    if (error) { Serial.println(F("FAILED"));  return error; }
    Serial.print(F("OK (")); Serial.print(t1); Serial.println(F(" ms)"));

    total = millis() - total;
    Serial.print(F("Compilation finished in ")); Serial.print(total); Serial.println(F(" ms"));

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
            Serial.print(F("    ")); c.string.print(); Serial.print(F(" = "));
            if (c.type == VAL_BOOLEAN) Serial.print(c.value_bool ? "true" : "false");
            if (c.type == VAL_INTEGER) Serial.print(c.value_int);
            if (c.type == VAL_REAL) Serial.print(c.value_float);
            if (c.type == VAL_STRING) c.value_string.print();
            Serial.println();
        }
    } else Serial.println(F("No consts"));

    // if (token_count > 0) {
    //     Serial.print(F("Tokens ")); Serial.print(token_count); Serial.println(F(":"));
    //     for (int i = 0; i < token_count; i++) {
    //         Token& token = tokens[i];
    //         TokenType type = token.type;
    //         Serial.print(F("    "));
    //         fill(' ', 4 - Serial.print(i));
    //         int s = Serial.print(F(" (")) + Serial.print(token.line) + Serial.print(F(":")) + Serial.print(token.column) + Serial.print(F(") "));
    //         fill(' ', 10 - s);
    //         Serial.print(F(" "));
    //         fill(' ', 12 - printTokenType(type));
    //         token.print();
    //         Serial.println();
    //     }
    // } else Serial.println(F("No tokens"));

    // Serial.print(F("Assembly: \"")); Serial.print(assembly_string); Serial.println(F("\""));

    Serial.print(F("Bytecode ")); Serial.print(built_bytecode_length); Serial.println(F(" bytes:"));
    for (int i = 0; i < built_bytecode_length; i++) {
        uint8_t byte = built_bytecode[i]; // Format it as hex
        char c1 = ((byte >> 4) & 0xF) + '0';
        char c2 = ((byte) & 0xF) + '0';
        if (c1 > '9') c1 += 'A' - '9' - 1;
        if (c2 > '9') c2 += 'A' - '9' - 1;
        Serial.print(F(" ")); Serial.print(c1); Serial.print(c2);
    }
    Serial.println();
    return false;
}



WASM_EXPORT void verifyCode() {
    VovkPLCRuntime runtime(128, 10000, built_bytecode, built_bytecode_length); // Stack size
    runtime.startup();
    RuntimeError status = UnitTest::fullProgramDebug(runtime);
    const char* status_name = RUNTIME_ERROR_NAME(status);
    Serial.print(F("Runtime status: ")); Serial.println(status_name);
}

#endif // __WASM__