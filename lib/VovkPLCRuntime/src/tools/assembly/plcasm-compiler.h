#pragma once

#include "wasm/wasm.h"

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

// Part 1: Reading the assembly
#define max_assembly_string_size 1024
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
    TOKEN_OPERATOR,
    TOKEN_KEYWORD,
    TOKEN_LABEL,
};

int printTokenType(TokenType type) {
    if (type == TOKEN_BOOLEAN) return printf("boolean");
    if (type == TOKEN_INTEGER) return printf("integer");
    if (type == TOKEN_REAL) return printf("real");
    if (type == TOKEN_STRING) return printf("string");
    if (type == TOKEN_OPERATOR) return printf("operator");
    if (type == TOKEN_KEYWORD) return printf("keyword");
    if (type == TOKEN_LABEL) return printf("label");
    return printf("unknown");
}

#define MAX_NUM_OF_TOKENS 1000
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
        if (type == TOKEN_STRING) return printf("\"%s\"", string.data);
        return string.print();
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

void add_token(char* string, int length) {
    if (token_count_temp >= MAX_NUM_OF_TOKENS) {
        Serial.print(F("Error: too many tokens. Max number of tokens is")); Serial.println(MAX_NUM_OF_TOKENS);
        return;
    }
    Token& token = tokens[token_count_temp];
    token.string.data = string;
    token.string.length = length;
    token.length = length;
    token.line = line;
    token.column = column;
    token.parse();
    if (token.type == TOKEN_UNKNOWN) {
        Serial.print(F("Error: unknown token \"")); token.print(); Serial.print(F("\" at ")); Serial.print(line); Serial.print(F(":")); Serial.println(column);
        return;
    }
    bool isNumber = token.type == TOKEN_INTEGER || token.type == TOKEN_REAL;
    if (token_count_temp >= 2 && isNumber) { // handle negative numbers
        Token& p1_token = tokens[token_count_temp - 1];
        Token& p2_token = tokens[token_count_temp - 2];
        TokenType p1_type = p1_token.type;
        TokenType p2_type = p2_token.type;
        // If [keyword , - , number] then change to [keyword , -number]

        if (p1_type == TOKEN_OPERATOR && p2_type == TOKEN_KEYWORD) {
            if (token.type == TOKEN_INTEGER) {
                p1_token.value_int = -token.value_int;
                p1_token.type = TOKEN_INTEGER;
                return;
            }
            if (token.type == TOKEN_REAL) {
                p1_token.value_float = -token.value_float;
                p1_token.type = TOKEN_REAL;
                return;
            }
        }
    }
    token_count_temp++;
}

bool add_token_optional(char* string, int length) {
    if (length == 0) {
        return false;
    }
    add_token(string, length);
    return true;
}

const char lex_ignored [] = { ' ', ';', '\t', '\r', '\n' };
const char lex_dividers [] = { '(', ')', '=', '+', '-', '*', '/', '%', '&', '|', '^', '~', '!', '<', '>', '?', ':', ',', ';', '[', ']', '{', '}', '\'', '"', '`', '\\' };

void tokenize() {
    char* token_start = assembly_string;
    int token_length = 0;
    int assembly_string_length = string_len(assembly_string);
    for (int i = 0; i < assembly_string_length; i++) {
        char c = assembly_string[i];
        // c == # || c == /
        if (c == '#' || c == '/') {
            add_token_optional(token_start, token_length);
            while (i < assembly_string_length && assembly_string[i] != '\n') i++;
            token_start = assembly_string + i + 1;
            token_length = 0;
            line++;
            column = 1;
            continue;
        }
        // c == \n
        if (c == '\n') {
            add_token_optional(token_start, token_length);
            token_start = assembly_string + i + 1;
            token_length = 0;
            line++;
            column = 1;
            continue;
        }
        // c == ' ' || c == '\t' || c == '\r'
        if (string_chr(lex_ignored, c) != NULL) {
            add_token_optional(token_start, token_length);
            column += token_length + 1;
            token_length = 0;
            continue;
        }
        // c == '(' || c == ')' || c == '=' || c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '&' || c == '|' || c == '^' || c == '~' || c == '!' || c == '<' || c == '>' || c == '?' || c == ':' || c == ',' || c == ';' || c == '[' || c == ']' || c == '{' || c == '}' || c == '\'' || c == '"' || c == '`' || c == '\\'
        if (string_chr(lex_dividers, c) != NULL) {
            add_token_optional(token_start, token_length);
            column += token_length;
            add_token(assembly_string + i, 1);
            token_length = 0;
            column++;
            continue;
        }
        if (token_length == 0) token_start = assembly_string + i;
        token_length++;
    }
    add_token_optional(token_start, token_length);
    token_count = token_count_temp;
    token_count_temp = 0;
    line = 1;
    column = 1;
}


void fill(char c, int count) {
    for (int i = 0; i < count; i++) Serial.print(c);
}


WASM_EXPORT void compileTest() {
    Serial.println(F("Compiling test..."));
    tokenize();
    Serial.print(F("Assembly: \"")); Serial.print(assembly_string); Serial.println(F("\""));
    Serial.print(F("Token count: ")); Serial.println(token_count);
    Serial.println(F("Tokens:"));
    for (int i = 0; i < token_count; i++) {
        Token& token = tokens[i];
        Serial.print(F(" "));
        fill(' ', 4 - Serial.print(i));
        int s = Serial.print(F(" (")) + Serial.print(token.line) + Serial.print(F(":")) + Serial.print(token.column) + Serial.print(F(") "));
        fill(' ', 8 - s);
        Serial.print(F(" "));
        fill(' ', 10 - printTokenType(token.type));
        if (token.type == TOKEN_KEYWORD) Serial.print(F("\""));
        token.print();
        if (token.type == TOKEN_KEYWORD) Serial.print(F("\""));
        Serial.println();
    }
}

