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


#ifdef __WASM__

#include "wasm/wasm.h"

#include "./../runtime-types.h"

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

// Part 1: Reading the assembly
char assembly_string[MAX_ASSEMBLY_STRING_SIZE] = R"(
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
    if (size > MAX_ASSEMBLY_STRING_SIZE) {
        printf("Error: input assembly is too long (%d characters). Max size is %d characters.\n", size, MAX_ASSEMBLY_STRING_SIZE);
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
    void highlight() {
        // jump test
        //      ~~~~
        int row = 1;
        char* c = assembly_string;
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
        char c = string[0];
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '&' || c == '|' || c == '^' || c == '~' || c == '!' || c == '<' || c == '>' || c == '?' || c == ':' || c == ',' || c == ';' || c == '[' || c == ']' || c == '{' || c == '}' || c == '\'' || c == '"' || c == '`' || c == '\\') {
            this->type = TOKEN_OPERATOR;
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
const int illegal_keywords_count = sizeof(illegal_keywords) / sizeof(illegal_keywords[0]);

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


bool last_token_is_exit = false;
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
        if (token == "'" && p2_token == "'") {
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

const char lex_ignored [] = { ' ', ';', '\t', '\r', '\n', '\0' };
const char lex_dividers [] = { '(', ')', '=', '+', '-', '*', '/', '%', '&', '|', '^', '~', '!', '<', '>', '?', ':', ',', ';', '[', ']', '{', '}', '\'', '"', '`', '\\', '\0' };

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
    if (!last_token_is_exit) {
        char* exit_string = new char[5];
        exit_string[0] = 'e';
        exit_string[1] = 'x';
        exit_string[2] = 'i';
        exit_string[3] = 't';
        exit_string[4] = '\0';
        error = add_token(exit_string, 4);
        if (error) return error;
    }
    token_count = token_count_temp;
    token_count_temp = 0;
    line = 1;
    column = 1;
    return false;
}


// data type keywords
const char* data_type_keywords [] = { "i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f32", "f64", "bool", "string", "bit", "byte", "ptr", "pointer", "*" };
const int data_type_keywords_count = sizeof(data_type_keywords) / sizeof(data_type_keywords[0]);


u8 built_bytecode[PLCRUNTIME_MAX_PROGRAM_SIZE] = { };
int built_bytecode_length = 0;
u8 built_bytecode_checksum = 0;

struct ProgramLine {
    u8 code[16];
    int index;
    int size;
    Token* refToken;
};

ProgramLine programLines[PLCRUNTIME_MAX_PROGRAM_SIZE] = { };
int programLineCount = 0;

int address_end = 0;
#define _line_push \
    address_end = built_bytecode_length + line.size; \
    if (address_end >= PLCRUNTIME_MAX_PROGRAM_SIZE) return buildErrorSizeLimit(token); \
    for (int j = 0; j < line.size; j++) { \
        built_bytecode[built_bytecode_length + j] = bytecode[j]; \
        crc8_simple(built_bytecode_checksum, bytecode[j]); \
    } \
    built_bytecode_length = address_end; \
    continue;


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
bool memoryBitFromToken(Token& token, int& address, int& bit) {
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

bool buildError(Token token, const char* message) {
    Serial.print(F(" ERROR: ")); Serial.print(F(message)); Serial.print(F(" -> ")); token.print(); Serial.print(F(" at line ")); Serial.print(token.line); Serial.print(F(":")); Serial.println(token.column);
    token.highlight();
    return true;
}

bool buildErrorSizeLimit(Token token) { return buildError(token, "program size limit reached"); }
bool buildErrorUnknownToken(Token token) { return buildError(token, "unknown token"); }
bool buildErrorExpectedInt(Token token) { return buildError(token, "unexpected token, expected integer"); }
bool buildErrorExpectedFloat(Token token) { return buildError(token, "unexpected token, expected float"); }
bool buildErrorUnknownLabel(Token token) { return buildError(token, "unknown label"); }

bool build(bool finalPass) {
    programLineCount = 0;
    built_bytecode_length = 0;
    built_bytecode_checksum = 0;
    for (int i = 0; i < token_count; i++) {
        Token& token = tokens[i];
        TokenType type = token.type;
        if (type == TOKEN_UNKNOWN) return buildErrorUnknownToken(token);

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
        u8 data_type;

        ProgramLine& line = programLines[programLineCount];
        line.index = built_bytecode_length;
        line.refToken = &token;
        u8* bytecode = line.code;

        bool hasNext = i + 1 < token_count;
        // [ u8.const , 3 ]
        Token& token_p1 = hasNext ? tokens[i + 1] : tokens[i];
        bool e_label = finalPass ? labelFromToken(token_p1, label_address) : false;
        // bool e_bool = boolFromToken(token_p1, value_bool);
        bool e_int = intFromToken(token_p1, value_int);
        bool e_real = realFromToken(token_p1, value_float);
        /* bool e_data_type = */ typeFromToken(token, data_type);


        bool hasThird = i + 2 < token_count;
        Token& token_p2 = hasThird ? tokens[i + 2] : tokens[i];

        if (type == TOKEN_KEYWORD) {
            { // Handle flow
                if (hasNext && (token == "jmp" || token == "jump")) { if (finalPass && e_label) return buildErrorUnknownLabel(token_p1); i++; line.size = InstructionCompiler::push_jmp(bytecode, label_address); _line_push; }
                if (hasNext && (token == "jmp_if" || token == "jump_if")) { if (finalPass && e_label) return buildErrorUnknownLabel(token_p1); i++; line.size = InstructionCompiler::push_jmp_if(bytecode, label_address); _line_push; }
                if (hasNext && (token == "jmp_if_not" || token == "jump_if_not")) { if (finalPass && e_label) return buildErrorUnknownLabel(token_p1); i++; line.size = InstructionCompiler::push_jmp_if_not(bytecode, label_address); _line_push; }
                if (hasNext && token == "call") { if (finalPass && e_label) return buildErrorUnknownLabel(token_p1); i++; line.size = InstructionCompiler::pushCALL(bytecode, label_address); _line_push; }
                if (hasNext && token == "call_if") { if (finalPass && e_label) return buildErrorUnknownLabel(token_p1); i++; line.size = InstructionCompiler::pushCALL_IF(bytecode, label_address); _line_push; }
                if (hasNext && token == "call_if_not") { if (finalPass && e_label) return buildErrorUnknownLabel(token_p1); i++; line.size = InstructionCompiler::pushCALL_IF_NOT(bytecode, label_address); _line_push; }
                if (token == "ret" || token == "return") { line.size = InstructionCompiler::push(bytecode, RET); _line_push; }
                if (token == "ret_if" || token == "return_if") { line.size = InstructionCompiler::push(bytecode, RET_IF); _line_push; }
                if (token == "ret_if_not" || token == "return_if_not") { line.size = InstructionCompiler::push(bytecode, RET_IF_NOT); _line_push; }
                if (token == "nop") { line.size = InstructionCompiler::push(bytecode, NOP); _line_push; }
            }

            { // Stack operations
                if (token == "clear") { line.size = InstructionCompiler::push(bytecode, CLEAR); _line_push; }
            }

            { // Mapped instructions
                if (token.equals("P_On")) { line.size = InstructionCompiler::push_u8(bytecode, 1); _line_push; } // P_On: "u8.const 1",
                if (token.equals("P_Off")) { line.size = InstructionCompiler::push_u8(bytecode, 0); _line_push; } // P_Off: "u8.const 0",
                if (token.equals("P_100ms")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B0, 2); _line_push; } // P_100ms: "u8.readBit 2.0",
                if (token.equals("P_200ms")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B1, 2); _line_push; } // P_200ms: "u8.readBit 2.1",
                if (token.equals("P_300ms")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B2, 2); _line_push; } // P_300ms: "u8.readBit 2.2",
                if (token.equals("P_500ms")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B3, 2); _line_push; } // P_500ms: "u8.readBit 2.3",
                if (token.equals("P_1s")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B4, 2); _line_push; } // P_1s: "u8.readBit 2.4",
                if (token.equals("P_2s")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B5, 2); _line_push; } // P_2s: "u8.readBit 2.5",
                if (token.equals("P_5s")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B6, 2); _line_push; } // P_5s: "u8.readBit 2.6",
                if (token.equals("P_10s")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B7, 2); _line_push; } // P_10s: "u8.readBit 2.7",
                if (token.equals("P_30s")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B0, 3); _line_push; } // P_30s: "u8.readBit 3.0",
                if (token.equals("P_1min")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B1, 3); _line_push; } // P_1min: "u8.readBit 3.1",
                if (token.equals("P_2min")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B2, 3); _line_push; } // P_2min: "u8.readBit 3.2",
                if (token.equals("P_5min")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B3, 3); _line_push; } // P_5min: "u8.readBit 3.3",
                if (token.equals("P_10min")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B4, 3); _line_push; } // P_10min: "u8.readBit 3.4",
                if (token.equals("P_15min")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B5, 3); _line_push; } // P_10min: "u8.readBit 3.5",
                if (token.equals("P_30min")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B6, 3); _line_push; } // P_30min: "u8.readBit 3.6",
                if (token.equals("P_1hr")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B7, 3); _line_push; } // P_1hr: "u8.readBit 3.7",
                if (token.equals("P_2hr")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B0, 4); _line_push; } // P_2hr: "u8.readBit 4.0",
                if (token.equals("P_3hr")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B1, 4); _line_push; } // P_3hr: "u8.readBit 4.1",
                if (token.equals("P_4hr")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B2, 4); _line_push; } // P_4hr: "u8.readBit 4.2",
                if (token.equals("P_5hr")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B3, 4); _line_push; } // P_5hr: "u8.readBit 4.3",
                if (token.equals("P_6hr")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B4, 4); _line_push; } // P_6hr: "u8.readBit 4.4",
                if (token.equals("P_12hr")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B5, 4); _line_push; } // P_12hr: "u8.readBit 4.5",
                if (token.equals("P_1day")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B6, 4); _line_push; } // P_1day: "u8.readBit 4.6",
                
                if (token.equals("S_100ms")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B0, 5); _line_push; } // P_100ms: "u8.readBit 2.0",
                if (token.equals("S_200ms")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B1, 5); _line_push; } // P_200ms: "u8.readBit 2.1",
                if (token.equals("S_300ms")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B2, 5); _line_push; } // P_300ms: "u8.readBit 2.2",
                if (token.equals("S_500ms")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B3, 5); _line_push; } // P_500ms: "u8.readBit 2.3",
                if (token.equals("S_1s")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B4, 5); _line_push; } // P_1s: "u8.readBit 2.4",
                if (token.equals("S_2s")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B5, 5); _line_push; } // P_2s: "u8.readBit 2.5",
                if (token.equals("S_5s")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B6, 5); _line_push; } // P_5s: "u8.readBit 2.6",
                if (token.equals("S_10s")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B7, 5); _line_push; } // P_10s: "u8.readBit 2.7",
                if (token.equals("S_30s")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B0, 6); _line_push; } // P_30s: "u8.readBit 3.0",
                if (token.equals("S_1min")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B1, 6); _line_push; } // P_1min: "u8.readBit 3.1",
                if (token.equals("S_2min")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B2, 6); _line_push; } // P_2min: "u8.readBit 3.2",
                if (token.equals("S_5min")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B3, 6); _line_push; } // P_5min: "u8.readBit 3.3",
                if (token.equals("S_10min")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B4, 6); _line_push; } // P_10min: "u8.readBit 3.4",
                if (token.equals("S_15min")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B5, 6); _line_push; } // P_10min: "u8.readBit 3.5",
                if (token.equals("S_30min")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B6, 6); _line_push; } // P_30min: "u8.readBit 3.6",
                if (token.equals("S_1hr")) { line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, READ_X8_B7, 6); _line_push; } // P_1hr: "u8.readBit 3.7",
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
                            if (e_int) return buildErrorExpectedInt(token_p1); i++;
                            if (value_int < 0 || value_int > 7) return buildError(token_p1, "bit value out of range for 8-bit type");
                            stack_bit_task = (PLCRuntimeInstructionSet) ((int) stack_bit_task + value_int);
                            line.size = InstructionCompiler::push(bytecode, stack_bit_task); _line_push;
                        }

                        if (token == "u8.and") { line.size = InstructionCompiler::push(bytecode, LOGIC_AND); _line_push; }
                        if (token == "u8.or") { line.size = InstructionCompiler::push(bytecode, LOGIC_OR); _line_push; }
                        if (token == "u8.xor") { line.size = InstructionCompiler::push(bytecode, LOGIC_XOR); _line_push; }
                        if (token == "u8.not") { line.size = InstructionCompiler::push(bytecode, LOGIC_NOT); _line_push; }

                        // Direct memory read/write
                        PLCRuntimeInstructionSet mem_bit_task = (PLCRuntimeInstructionSet) 0;
                        if (!mem_bit_task && token.includes(".writeBitInv")) mem_bit_task = WRITE_INV_X8_B0; // WRITE_INV_X8 (INVERT)
                        if (!mem_bit_task && token.includes(".writeBitOff")) mem_bit_task = WRITE_R_X8_B0; // WRITE_R_X8 (RESET)
                        if (!mem_bit_task && token.includes(".writeBitOn")) mem_bit_task = WRITE_S_X8_B0; // WRITE_S_X8 (SET)
                        if (!mem_bit_task && token.includes(".writeBit")) mem_bit_task = WRITE_X8_B0; // WRITE_X8
                        if (!mem_bit_task && token.includes(".readBit")) mem_bit_task = READ_X8_B0; // READ_X8
                        if (mem_bit_task) {
                            if (e_int) return buildErrorExpectedInt(token_p1); i++;
                            int address, bit;
                            bool e_membit = memoryBitFromToken(token_p1, address, bit);
                            if (e_membit) return buildError(token_p1, "unexpected token, expected bit representation");
                            if (bit < 0 || bit > 7) return buildError(token_p1, "bit value out of range for 8-bit type");
                            mem_bit_task = (PLCRuntimeInstructionSet) ((int) mem_bit_task + bit);
                            line.size = InstructionCompiler::push_InstructionWithPointer(bytecode, mem_bit_task, value_int); _line_push;
                        }
                    }
                }
            }

            { // Handle data type operations
                if (data_type) {
                    PLCRuntimeInstructionSet type = (PLCRuntimeInstructionSet) data_type;
                    if (hasNext && token.endsWith(".const")) {
                        if (type == type_pointer) { if (e_int) return buildErrorExpectedInt(token_p1); i++; line.size = InstructionCompiler::push_pointer(bytecode, value_int); _line_push; }
                        if (type == type_bool) { if (e_int) return buildErrorExpectedInt(token_p1); i++; line.size = InstructionCompiler::push_bool(bytecode, value_int != 0); _line_push; }
                        if (type == type_u8) { if (e_int) return buildErrorExpectedInt(token_p1); i++; line.size = InstructionCompiler::push_u8(bytecode, value_int); _line_push; }
                        if (type == type_u16) { if (e_int) return buildErrorExpectedInt(token_p1); i++; line.size = InstructionCompiler::push_u16(bytecode, value_int); _line_push; }
                        if (type == type_u32) { if (e_int) return buildErrorExpectedInt(token_p1); i++; line.size = InstructionCompiler::push_u32(bytecode, value_int); _line_push; }
                        if (type == type_u64) { if (e_int) return buildErrorExpectedInt(token_p1); i++; line.size = InstructionCompiler::push_u64(bytecode, value_int); _line_push; }
                        if (type == type_i8) { if (e_int) return buildErrorExpectedInt(token_p1); i++; line.size = InstructionCompiler::push_i8(bytecode, value_int); _line_push; }
                        if (type == type_i16) { if (e_int) return buildErrorExpectedInt(token_p1); i++; line.size = InstructionCompiler::push_i16(bytecode, value_int); _line_push; }
                        if (type == type_i32) { if (e_int) return buildErrorExpectedInt(token_p1); i++; line.size = InstructionCompiler::push_i32(bytecode, value_int); _line_push; }
                        if (type == type_i64) { if (e_int) return buildErrorExpectedInt(token_p1); i++; line.size = InstructionCompiler::push_i64(bytecode, value_int); _line_push; }
                        if (type == type_f32) { if (e_real) return buildErrorExpectedFloat(token_p1); i++; line.size = InstructionCompiler::push_f32(bytecode, value_float); _line_push; }
                        if (type == type_f64) { if (e_real) return buildErrorExpectedFloat(token_p1); i++; line.size = InstructionCompiler::push_f64(bytecode, value_float); _line_push; }
                        Serial.print(F("Error: unknown data type ")); token.print(); Serial.print(F(" at ")); Serial.print(token.line); Serial.print(F(":")); Serial.println(token.column);
                        return true;
                    }
                    // if (hasNext && token.endsWith(".load")) { if (e_int) return buildErrorExpectedInt(token_p1); i++; line.size = InstructionCompiler::pushGET(bytecode, value_int, type); _line_push; }
                    // if (hasNext && token.endsWith(".store")) { if (e_int) return buildErrorExpectedInt(token_p1); i++; line.size = InstructionCompiler::pushPUT(bytecode, value_int, type); _line_push; }
                    if (hasNext && token.endsWith(".load")) { line.size = InstructionCompiler::push_load(bytecode, type); _line_push; }
                    if (hasNext && token.endsWith(".move")) { line.size = InstructionCompiler::push_move(bytecode, type); _line_push; }
                    if (hasNext && token.endsWith(".move_copy")) { line.size = InstructionCompiler::push_move_copy(bytecode, type); _line_push; }
                    if (hasNext && token.endsWith(".copy")) { line.size = InstructionCompiler::push_copy(bytecode, type); _line_push; }
                    // if (hasNext && token.endsWith(".swap")) { line.size = InstructionCompiler::push_swap(bytecode, type); _line_push; }
                    if (hasNext && token.endsWith(".drop")) { line.size = InstructionCompiler::push_drop(bytecode, type); _line_push; }
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
                if (e_dataType1) return e_dataType1; i++;
                if (e_dataType2) return e_dataType2; i++;
                if (type_1 == type_2) continue; // No need to convert if types are the same
                line.size = InstructionCompiler::push_cvt(bytecode, (PLCRuntimeInstructionSet) type_1, (PLCRuntimeInstructionSet) type_2);
                _line_push;
            }
            if (token == "swap") { // Swap two values on the stack of any combination of types
                if (e_dataType1) return e_dataType1; i++;
                if (e_dataType2) return e_dataType2; i++;
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

        return buildErrorUnknownToken(token);
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


WASM_EXPORT void loadAssembly() {
    int size = 0;
    streamRead(assembly_string, size, MAX_ASSEMBLY_STRING_SIZE);
}


WASM_EXPORT void logBytecode() {
    Serial.print(F("Bytecode checksum ")); print_hex(built_bytecode_checksum); Serial.print(F(", ")); Serial.print(built_bytecode_length); Serial.println(F(" bytes:"));
    for (int i = 0; i < built_bytecode_length; i++) {
        u8 byte = built_bytecode[i]; // Format it as hex
        char c1, c2;
        byteToHex(byte, c1, c2);
        Serial.print(F(" ")); Serial.print(c1); Serial.print(c2);
    }
    Serial.println();
}

int num_of_compile_runs = 0;
WASM_EXPORT bool compileAssembly(bool debug = true) {
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

    if (debug) Serial.print(F("."));
    t1 = millis();
    error = build(false); // First pass
    t1 = millis() - t1;
    if (error) { Serial.println(F("Failed at building"));  return error; }


    if (debug) Serial.print(F("."));
    t1 = millis();
    error = build(true); // Second pass to link labels
    t1 = millis() - t1;
    if (error) { Serial.println(F("Failed at linking"));  return error; }

    total = millis() - total;
    if (debug) { Serial.print(F(" finished in ")); Serial.print(total); Serial.println(F(" ms")); }

    if (debug) {
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
        logBytecode();
    }
    return false;
}

VovkPLCRuntime runtime;

WASM_EXPORT void printProperties() {
    runtime.printProperties();
}

WASM_EXPORT bool loadCompiledProgram() {
    Serial.printf("Loading program with %d bytes and checksum 0x%02X ...\n", built_bytecode_length, built_bytecode_checksum);
    runtime.loadProgram(built_bytecode, built_bytecode_length, built_bytecode_checksum);
    return false;
}

WASM_EXPORT void runFullProgramDebug() {
    RuntimeError status = UnitTest::fullProgramDebug(runtime);
    const char* status_name = RUNTIME_ERROR_NAME(status);
    Serial.print(F("Runtime status: ")); Serial.println(status_name);
}

WASM_EXPORT void runFullProgram() {
    RuntimeError status = runtime.run();
    const char* status_name = RUNTIME_ERROR_NAME(status);
    Serial.print(F("Runtime status: ")); Serial.println(status_name);
}

WASM_EXPORT u32 uploadProgram() {
    for (u32 i = 0; i < built_bytecode_length; i++) {
        u8 byte = built_bytecode[i]; // Format it as hex
        char c1, c2;
        byteToHex(byte, c1, c2);
        streamOut(c1);
        streamOut(c2);
    }
    return built_bytecode_length;
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

WASM_EXPORT void external_print(const char* string) {
    Serial.print(string);
}

#else

void set_assembly_string(char* new_assembly_string) {}
bool compileAssembly() { return false; }
void runFullProgramDebug() {}
u32 uploadProgram() { return 0; }
u32 getMemoryArea(u32 address, u32 size) { return 0; }
u32 writeMemoryByte(u32 address, u8 byte) { return 0; }

#endif // __WASM__