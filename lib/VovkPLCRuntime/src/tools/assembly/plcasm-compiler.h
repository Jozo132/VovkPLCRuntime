#pragma once

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
    const int size = strlen(new_assembly_string);
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

#define MAX_NUM_OF_TOKENS 1000
struct Token {
    char* string;
    int length;
    int line;
    int column;
};

Token tokens[MAX_NUM_OF_TOKENS] = { };
int token_count = 0;

int line = 1;
int column = 1;

int assembly_string_length = strlen(assembly_string);

void add_token(char* string, int length) {
    if (token_count >= MAX_NUM_OF_TOKENS) {
        Serial.print(F("Error: too many tokens. Max number of tokens is")); Serial.println(MAX_NUM_OF_TOKENS);
        return;
    }
    Token &token = tokens[token_count];
    token.string = string;
    token.length = length;
    token.line = line;
    token.column = column;
    token_count++;
}

const char lex_ignored [] = { ' ', ';', '\t', '\r', '\n' };
const char lex_dividers [] = { '(', ')', '=', '+', '-', '*', '/', '%', '&', '|', '^', '~', '!', '<', '>', '?', ':', ',', ';', '[', ']', '{', '}', '\'', '"', '`', '\\' };

void tokenize() {
    char* token_start = assembly_string;
    int token_length = 0;
    for (int i = 0; i < assembly_string_length; i++) {
        char c = assembly_string[i];
        // c == #
        if (c == '#') {
            while (i < assembly_string_length && assembly_string[i] != '\n') {
                i++;
            }
            continue;
        }
        // c == \n
        if (c == '\n') {
            line++;
            column = 1;
            continue;
        }
        // c == ' ' || c == '\t' || c == '\r'
        if (string_chr(lex_ignored, c) != NULL) {
            if (token_length > 0) {
                add_token(token_start, token_length);
                token_start = assembly_string + i + 1;
                token_length = 0;
            }
            continue;
        }
        // c == '(' || c == ')' || c == '=' || c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '&' || c == '|' || c == '^' || c == '~' || c == '!' || c == '<' || c == '>' || c == '?' || c == ':' || c == ',' || c == ';' || c == '[' || c == ']' || c == '{' || c == '}' || c == '\'' || c == '"' || c == '`' || c == '\\'
        if (string_chr(lex_dividers, c) != NULL) {
            if (token_length > 0) {
                add_token(token_start, token_length);
                token_start = assembly_string + i + 1;
                token_length = 0;
            }
            add_token(assembly_string + i, 1);
            continue;
        }
        token_length++;
    }
    if (token_length > 0) {
        add_token(token_start, token_length);
    }
}





WASM_EXPORT void compileTest() {
    Serial.println(F("Compiling test..."));
    tokenize();
    Serial.print(F("Token count: ")); Serial.println(token_count);
    for (int i = 0; i < token_count; i++) {
        Token& token = tokens[i];
        Serial.print(F("Token "));
        Serial.print(i);
        Serial.print(F(": "));
        Serial.print(token.string);
        Serial.print(F(" ("));
        Serial.print(token.length);
        Serial.print(F(")"));
        Serial.print(F(" line: "));
        Serial.print(token.line);
        Serial.print(F(" column: "));
        Serial.println(token.column);
    }
}