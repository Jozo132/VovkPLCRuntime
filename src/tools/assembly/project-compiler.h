// project-compiler.h - Full Project Compiler for VovkPLCRuntime
// Compiles complete PLC projects with multiple program blocks in different languages
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
#include "plcasm-linter.h"
#include "stl-compiler.h"
#include "stl-linter.h"
#include "ladder-compiler.h"
#include "ladder-linter.h"
#include "plcscript-compiler.h"
#include "shared-symbols.h"

// ============================================================================
// Project Compiler - Full Project Export/Compile
// ============================================================================
//
// Accepts a project definition string with the following format:
//
// VOVKPLCPROJECT <name>
// VERSION <version>
//
// MEMORY
//   OFFSET <start>              // Required: starting offset
//   AVAILABLE <bytes>           // Required: total available memory size
//   K <size>                    // Required: Controls/Constants (100% usage assumed)
//   X <size>                    // Required: Inputs (100% usage assumed)
//   Y <size>                    // Required: Outputs (100% usage assumed)
//   S <size>                    // Required: System (usage based on access)
//   M <size>                    // Required: Markers (usage based on access)
//   T <size>                    // Required: Timers (usage based on access)
//   C <size>                    // Required: Counters (usage based on access)
// END_MEMORY
//
// FLASH
//   SIZE <bytes>                // Optional: flash size for bytecode (default 32KB)
// END_FLASH
//
// SYMBOLS
//   <name> : <type> : <address> [: <comment>]
//   ...
// END_SYMBOLS
//
// FILE <path>                    // 'main' is always executed first, others alphabetically
//   BLOCK LANG=<language> <block name>   // block name can have spaces
//     <source code>
//   END_BLOCK
//   ... (more BLOCK entries)
// END_FILE
//
// ... (more FILE blocks)
//
// Supported languages: PLCASM, STL, LADDER (JSON)
//
// Two-pass compilation:
//   Pass 1: Parse all files/blocks, detect labels, allocate addresses
//   Pass 2: Compile each block, patch label references, generate bytecode
//
// Files are executed in order: 'main' first, then others alphabetically by path.
// Use CALL instructions to invoke other program files.
//
// ============================================================================

#define PROJECT_MAX_NAME_LEN 64
#define PROJECT_MAX_PATH_LEN 128
#define PROJECT_MAX_VERSION_LEN 32
#define PROJECT_MAX_MEMORY_AREAS 16
#define PROJECT_MAX_FILES 32
#define PROJECT_MAX_PROGRAM_BLOCKS 64
#define PROJECT_MAX_LABELS 256
#define PROJECT_MAX_OUTPUT_SIZE 65536
#define PROJECT_MAX_SOURCE_SIZE 65536
#define PROJECT_MAX_ERROR_LEN 512

// Memory area definition
struct MemoryArea {
    char name[32];          // Area name (M, I, Q, T, C, etc.)
    u32 start;              // Start address
    u32 end;                // End address (inclusive)
    bool retain;            // Retain on power loss
    bool used;              // Is this entry used?

    void reset() {
        name[0] = '\0';
        start = 0;
        end = 0;
        retain = false;
        used = false;
    }
};

// Program block definition (within a file)
struct ProgramBlock {
    char file_path[PROJECT_MAX_PATH_LEN];     // Parent FILE path
    char name[PROJECT_MAX_NAME_LEN];          // BLOCK name
    u8 language;            // PLCSourceLanguage enum
    int source_start;       // Start position in project source
    int source_end;         // End position in project source
    u32 bytecode_offset;    // Offset in final bytecode
    u32 bytecode_size;      // Size of compiled bytecode
    int combined_line_start; // Start line in combined PLCASM (1-based)
    int combined_line_end;   // End line in combined PLCASM (1-based)
    bool compiled;          // Has been compiled
    bool used;              // Is this entry used?

    void reset() {
        file_path[0] = '\0';
        name[0] = '\0';
        language = LANG_UNKNOWN;
        source_start = 0;
        source_end = 0;
        bytecode_offset = 0;
        bytecode_size = 0;
        combined_line_start = 0;
        combined_line_end = 0;
        compiled = false;
        used = false;
    }
};

// Program file definition (contains blocks)
struct ProgramFile {
    char path[PROJECT_MAX_PATH_LEN];          // File path (e.g., "main", "utils/helpers")
    int first_block_index;                     // Index of first block in program_blocks array
    int block_count;                           // Number of blocks in this file
    int execution_order;                       // Execution order (0 = main, then alphabetical)
    bool used;

    void reset() {
        path[0] = '\0';
        first_block_index = -1;
        block_count = 0;
        execution_order = 0;
        used = false;
    }
};

// Symbol with owned string storage (unlike PLCASM's Symbol which uses StringView pointers)
struct ProjectSymbol {
    char name[64];          // Symbol name (owned storage)
    char type[32];          // Type name (owned storage)
    char address_str[32];   // Original address string (e.g. "M10.0")
    u32 address;            // Memory address or byte offset
    u8 bit;                 // Bit position (0-7) or 255 if not a bit type
    bool is_bit;            // True if this is a bit address
    u8 type_size;           // Size in bytes
    char default_value[32]; // Optional default value (e.g. "0", "1", "123.45")
    bool has_default;
    bool used;

    void reset() {
        name[0] = '\0';
        type[0] = '\0';
        address_str[0] = '\0';
        address = 0;
        bit = 255;
        is_bit = false;
        type_size = 0;
        default_value[0] = '\0';
        has_default = false;
        used = false;
    }
};

#define PROJECT_MAX_SYMBOLS 256

// Label reference for cross-block jumps
struct ProjectLabel {
    char name[64];
    char block_name[PROJECT_MAX_NAME_LEN];  // Which block defines this label
    u32 address;            // Address within the block (or global if resolved)
    bool resolved;          // Has been resolved to an address
    bool used;

    void reset() {
        name[0] = '\0';
        block_name[0] = '\0';
        address = 0;
        resolved = false;
        used = false;
    }
};

class ProjectCompiler {
public:
    // Project metadata
    char project_name[PROJECT_MAX_NAME_LEN];
    char project_version[PROJECT_MAX_VERSION_LEN];

    // Memory areas
    MemoryArea memory_areas[PROJECT_MAX_MEMORY_AREAS];
    int memory_area_count;
    u32 memory_available;    // Total available memory (from AVAILABLE keyword)
    u32 memory_used;         // Total memory used by all areas

    // Flash configuration
    u32 flash_size;          // Flash size for bytecode storage (default 32KB)

    // Symbols (with owned storage)
    ProjectSymbol symbols[PROJECT_MAX_SYMBOLS];
    int symbol_count;

    // Program files
    ProgramFile program_files[PROJECT_MAX_FILES];
    int program_file_count;

    // Program blocks (across all files)
    ProgramBlock program_blocks[PROJECT_MAX_PROGRAM_BLOCKS];
    int program_block_count;

    // Labels (for cross-block references)
    ProjectLabel labels[PROJECT_MAX_LABELS];
    int label_count;

    // Source input
    const char* source;
    int source_length;
    int pos;
    int line;
    int column;

    // Temporary source buffer for extracting block source
    char block_source[PROJECT_MAX_SOURCE_SIZE];

    // Combined PLCASM buffer (all blocks converted to PLCASM and concatenated)
    char combined_plcasm[PROJECT_MAX_SOURCE_SIZE];
    int combined_plcasm_length;
    int combined_plcasm_line;  // Current line number in combined output (1-based)

    // Output bytecode
    u8 output[PROJECT_MAX_OUTPUT_SIZE];
    int output_length;
    u8 output_checksum;

    // Error handling
    char error_msg[PROJECT_MAX_ERROR_LEN];
    char error_file[PROJECT_MAX_PATH_LEN];    // File where error occurred
    char error_block[PROJECT_MAX_NAME_LEN];   // Block where error occurred
    u8 error_block_language;                  // Language of the block where error occurred
    char error_compiler[64];                  // Which compiler reported the error
    char error_source_line[256];              // The source line content where error occurred
    char error_token[64];                     // The specific token that caused the error
    int error_token_length;                   // Length of the error token
    int error_line;
    int error_column;
    bool has_error;

    // Current context for error tracking
    char current_file[PROJECT_MAX_PATH_LEN];
    char current_block[PROJECT_MAX_NAME_LEN];
    u8 current_block_language;

    // Child compilers (using linter variants for better error messages)
    PLCASMLinter plcasm_compiler;
    STLLinter stl_compiler;
    LadderLinter ladder_compiler;
    PLCScriptCompiler plcscript_compiler;

    // Project-level edge memory counter for differentiation bits (shared across all blocks)
    int project_edge_mem_counter;

    // Problems array for multiple error tracking
    LinterProblem problems[MAX_LINT_PROBLEMS];
    int problem_count = 0;

    // Debug output
    bool debug_mode;

    ProjectCompiler() {
        reset();
    }

    void reset() {
        project_name[0] = '\0';
        project_version[0] = '\0';

        for (int i = 0; i < PROJECT_MAX_MEMORY_AREAS; i++) {
            memory_areas[i].reset();
        }
        memory_area_count = 0;
        memory_available = 0;
        memory_used = 0;

        flash_size = 32768; // Default 32KB

        for (int i = 0; i < PROJECT_MAX_SYMBOLS; i++) {
            symbols[i].reset();
        }
        symbol_count = 0;

        for (int i = 0; i < PROJECT_MAX_FILES; i++) {
            program_files[i].reset();
        }
        program_file_count = 0;

        for (int i = 0; i < PROJECT_MAX_PROGRAM_BLOCKS; i++) {
            program_blocks[i].reset();
        }
        program_block_count = 0;

        for (int i = 0; i < PROJECT_MAX_LABELS; i++) {
            labels[i].reset();
        }
        label_count = 0;

        // Reset problems
        problem_count = 0;
        for (int i = 0; i < MAX_LINT_PROBLEMS; i++) {
            problems[i].type = 0;
            problems[i].line = 0;
            problems[i].column = 0;
            problems[i].length = 0;
            problems[i].message[0] = '\0';
            problems[i].block[0] = '\0';
            problems[i].lang = 0;
            problems[i].token_text = nullptr;
        }

        source = nullptr;
        source_length = 0;
        pos = 0;
        line = 1;
        column = 1;

        block_source[0] = '\0';

        combined_plcasm[0] = '\0';
        combined_plcasm_length = 0;
        combined_plcasm_line = 1;

        memset(output, 0, sizeof(output));
        output_length = 0;
        output_checksum = 0;

        error_msg[0] = '\0';
        error_file[0] = '\0';
        error_block[0] = '\0';
        error_block_language = LANG_UNKNOWN;
        error_compiler[0] = '\0';
        error_source_line[0] = '\0';
        error_token[0] = '\0';
        error_token_length = 0;
        error_line = 0;
        error_column = 0;
        has_error = false;

        current_file[0] = '\0';
        current_block[0] = '\0';
        current_block_language = LANG_UNKNOWN;

        debug_mode = false;

        // Reset project-level edge memory counter (starts at bit 800 = M100.0)
        project_edge_mem_counter = 800;

        // Reset child compilers' symbol tables
        plcasm_compiler.symbol_count = 0;
        plcasm_compiler.base_symbol_count = 0;

        // Reset shared symbol table for fresh compile/lint
        resetSharedSymbols();
    }

    // ============ Error Handling ============

    void addProblem(uint32_t type, const char* msg, int l, int c, int length = 0, const char* token = nullptr) {
        if (problem_count >= MAX_LINT_PROBLEMS) return;

        LinterProblem& p = problems[problem_count++];
        p.type = type;
        p.line = l;
        p.column = c;
        p.length = length;
        
        // Copy token text into stable buffer
        p.token_buf[0] = '\0';
        if (token && length > 0) {
            int tlen = length < 63 ? length : 63;
            for (int i = 0; i < tlen; i++) {
                p.token_buf[i] = token[i];
            }
            p.token_buf[tlen] = '\0';
        }
        p.token_text = p.token_buf;  // Point to our stable buffer

        // Copy current context
        copyString(p.block, current_block, 64);
        copyString(p.program, current_file, 64);  // Copy program/file name
        p.lang = (uint32_t) current_block_language;

        // Copy message
        int i = 0;
        const char* src = msg;
        while (*src && i < 127) {
            p.message[i++] = *src++;
        }
        p.message[i] = '\0';
    }

    // Copy all problems (warnings, errors, info) from ladder compiler to project's problem list
    void copyLadderProblems() {
        for (int i = 0; i < ladder_compiler.problem_count && problem_count < MAX_LINT_PROBLEMS; i++) {
            LinterProblem& src = ladder_compiler.problems[i];
            LinterProblem& dest = problems[problem_count++];
            dest.type = src.type;
            dest.line = src.line;
            dest.column = src.column;
            dest.length = src.length;
            dest.lang = src.lang;
            // Copy message
            int mi = 0;
            while (src.message[mi] && mi < 127) { dest.message[mi] = src.message[mi]; mi++; }
            dest.message[mi] = '\0';
            // Copy token
            int ti = 0;
            while (src.token_buf[ti] && ti < 63) { dest.token_buf[ti] = src.token_buf[ti]; ti++; }
            dest.token_buf[ti] = '\0';
            dest.token_text = dest.token_buf;
            // Copy program and block names
            int pi = 0;
            while (current_file[pi] && pi < 63) { dest.program[pi] = current_file[pi]; pi++; }
            dest.program[pi] = '\0';
            int bi = 0;
            while (current_block[bi] && bi < 63) { dest.block[bi] = current_block[bi]; bi++; }
            dest.block[bi] = '\0';
        }
    }

    void setError(const char* msg) {
        if (has_error) return; // Still keep first error behavior for compilation failure

        has_error = true;
        error_line = line;
        error_column = column;

        // Add to problems list
        addProblem(LINT_ERROR, msg, line, column);

        // Copy current context
        copyString(error_file, current_file, PROJECT_MAX_PATH_LEN);
        copyString(error_block, current_block, PROJECT_MAX_NAME_LEN);
        error_block_language = current_block_language;
        copyString(error_compiler, "Project Parser", 64);
        error_source_line[0] = '\0';
        error_token[0] = '\0';
        error_token_length = 0;
        int i = 0;
        while (msg[i] && i < PROJECT_MAX_ERROR_LEN - 1) {
            error_msg[i] = msg[i];
            i++;
        }
        error_msg[i] = '\0';
    }

    void setErrorAt(const char* msg, int l, int c) {
        if (has_error) return;

        has_error = true;
        error_line = l;
        error_column = c;

        // Add to problems list
        addProblem(LINT_ERROR, msg, l, c);

        // Copy current context
        copyString(error_file, current_file, PROJECT_MAX_PATH_LEN);
        copyString(error_block, current_block, PROJECT_MAX_NAME_LEN);
        error_block_language = current_block_language;
        copyString(error_compiler, "Project Parser", 64);
        error_source_line[0] = '\0';
        error_token[0] = '\0';
        error_token_length = 0;
        int i = 0;
        while (msg[i] && i < PROJECT_MAX_ERROR_LEN - 1) {
            error_msg[i] = msg[i];
            i++;
        }
        error_msg[i] = '\0';
    }

    // Set error with full details including compiler, source line, and token
    void setErrorFull(const char* compiler_name, const char* msg, int l, int c,
        const char* source_text, int source_len,
        const char* token_text, int token_len) {

        // Always add to problems list regardless of has_error state
        addProblem(LINT_ERROR, msg, l, c, token_len, token_text);

        if (has_error) return;
        has_error = true;
        error_line = l;
        error_column = c;

        // Copy current context
        copyString(error_file, current_file, PROJECT_MAX_PATH_LEN);
        copyString(error_block, current_block, PROJECT_MAX_NAME_LEN);
        error_block_language = current_block_language;
        copyString(error_compiler, compiler_name, 64);

        // Copy error message
        int i = 0;
        while (msg[i] && i < PROJECT_MAX_ERROR_LEN - 1) {
            error_msg[i] = msg[i];
            i++;
        }
        error_msg[i] = '\0';

        // Copy token
        if (token_text && token_len > 0) {
            int tlen = token_len < 63 ? token_len : 63;
            for (i = 0; i < tlen; i++) {
                error_token[i] = token_text[i];
            }
            error_token[i] = '\0';
            error_token_length = tlen;
        } else {
            error_token[0] = '\0';
            error_token_length = 0;
        }

        // Extract source line containing the error
        if (source_text && source_len > 0 && l > 0) {
            int line_num = 1;
            int line_start = 0;

            // Find start of the line
            for (int j = 0; j < source_len && line_num < l; j++) {
                if (source_text[j] == '\n') {
                    line_num++;
                    line_start = j + 1;
                }
            }

            // Find end of the line and copy
            int line_end = line_start;
            while (line_end < source_len && source_text[line_end] != '\n' && source_text[line_end] != '\r') {
                line_end++;
            }

            int line_len = line_end - line_start;
            if (line_len > 255) line_len = 255;
            for (i = 0; i < line_len; i++) {
                error_source_line[i] = source_text[line_start + i];
            }
            error_source_line[i] = '\0';
        } else {
            error_source_line[0] = '\0';
        }
    }

    void copyString(char* dest, const char* src, int maxLen) {
        int i = 0;
        while (src[i] && i < maxLen - 1) {
            dest[i] = src[i];
            i++;
        }
        dest[i] = '\0';
    }

    // ============ Lexer Helpers ============

    char peek(int offset = 0) {
        int p = pos + offset;
        return (p < source_length) ? source[p] : '\0';
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
            if (c == ' ' || c == '\t' || c == '\r') {
                advance();
            } else if (c == '\n') {
                advance();
            } else {
                break;
            }
        }
    }

    void skipLine() {
        while (pos < source_length && peek() != '\n') {
            advance();
        }
        if (peek() == '\n') advance();
    }

    void skipWhitespaceNotNewline() {
        while (pos < source_length) {
            char c = peek();
            if (c == ' ' || c == '\t' || c == '\r') {
                advance();
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

    // Case-insensitive string comparison
    bool strEqI(const char* a, const char* b) {
        while (*a && *b) {
            char ca = *a;
            char cb = *b;
            if (ca >= 'A' && ca <= 'Z') ca += 32;
            if (cb >= 'A' && cb <= 'Z') cb += 32;
            if (ca != cb) return false;
            a++;
            b++;
        }
        return *a == '\0' && *b == '\0';
    }

    // Read an identifier/keyword
    bool readWord(char* buf, int maxLen) {
        skipWhitespaceNotNewline();
        int i = 0;
        while (pos < source_length && i < maxLen - 1) {
            char c = peek();
            // Include '=' to allow reading LANG=STL as a single token
            if (isAlphaNum(c) || c == '_' || c == '-' || c == '.' || c == '=') {
                buf[i++] = advance();
            } else {
                break;
            }
        }
        buf[i] = '\0';
        return i > 0;
    }

    // Read until end of line
    bool readLine(char* buf, int maxLen) {
        skipWhitespaceNotNewline();
        int i = 0;
        while (pos < source_length&& i < maxLen - 1) {
            char c = peek();
            if (c == '\n' || c == '\r') break;
            buf[i++] = advance();
        }
        buf[i] = '\0';
        // Trim trailing whitespace
        while (i > 0 && (buf[i - 1] == ' ' || buf[i - 1] == '\t')) {
            buf[--i] = '\0';
        }
        return i > 0;
    }

    // Read until end of line or specific delimiter
    bool readLineUntil(char* buf, int maxLen, char delimiter) {
        skipWhitespaceNotNewline();
        int i = 0;
        while (pos < source_length && i < maxLen - 1) {
            char c = peek();
            if (c == delimiter || c == '\n' || c == '\r') break;
            buf[i++] = advance();
        }
        buf[i] = '\0';
        // Trim trailing whitespace
        while (i > 0 && (buf[i - 1] == ' ' || buf[i - 1] == '\t')) {
            buf[--i] = '\0';
        }
        return i > 0;
    }

    // Read an integer
    bool readInt(u32& value) {
        skipWhitespaceNotNewline();
        value = 0;
        bool found = false;
        while (pos < source_length && isDigit(peek())) {
            value = value * 10 + (advance() - '0');
            found = true;
        }
        return found;
    }

    // Check if current position matches keyword (case-insensitive)
    bool matchKeyword(const char* keyword) {
        skipWhitespace();
        int start = pos;
        int startLine = line;
        int startCol = column;

        const char* k = keyword;
        while (*k) {
            char c = peek();
            char kc = *k;
            if (c >= 'A' && c <= 'Z') c += 32;
            if (kc >= 'A' && kc <= 'Z') kc += 32;
            if (c != kc) {
                // Restore position
                pos = start;
                line = startLine;
                column = startCol;
                return false;
            }
            advance();
            k++;
        }
        // Must be followed by whitespace or end
        char next = peek();
        if (isAlphaNum(next)) {
            pos = start;
            line = startLine;
            column = startCol;
            return false;
        }
        return true;
    }

    // Skip comment lines (starting with // or #)
    void skipComments() {
        while (pos < source_length) {
            skipWhitespace();
            if (peek() == '/' && peek(1) == '/') {
                skipLine();
            } else if (peek() == '#') {
                skipLine();
            } else {
                break;
            }
        }
    }

    // ============ Parsing Functions ============

    bool parseProject() {
        skipComments();

        // VOVKPLCPROJECT keyword is optional
        if (matchKeyword("VOVKPLCPROJECT")) {
            if (!readLineUntil(project_name, PROJECT_MAX_NAME_LEN, '/')) {
                setError("Expected project name after VOVKPLCPROJECT");
                return false;
            }
        } else {
            // Default project name
            const char* defName = "Unnamed";
            int i = 0;
            while (defName[i]) { project_name[i] = defName[i]; i++; }
            project_name[i] = '\0';
        }

        skipComments();

        // VERSION is optional
        if (matchKeyword("VERSION")) {
            if (!readLineUntil(project_version, PROJECT_MAX_VERSION_LEN, '/')) {
                setError("Expected version after VERSION");
                return false;
            }
        } else {
            const char* defVer = "1.0";
            int i = 0;
            while (defVer[i]) { project_version[i] = defVer[i]; i++; }
            project_version[i] = '\0';
        }

        return true;
    }

    bool parseMemory() {
        skipComments();

        if (!matchKeyword("MEMORY")) {
            // MEMORY section is optional - use defaults
            setupDefaultMemory();
            return true;
        }

        // Required sequence: OFFSET, AVAILABLE, K, X, Y, S, M, T, C
        const char* required_areas[] = { "K", "X", "Y", "S", "M", "T", "C" };
        const int num_required = 7;
        
        u32 current_offset = 0;
        bool has_offset = false;
        bool has_available = false;
        int area_index = 0;

        while (pos < source_length && !has_error) {
            skipComments();

            if (matchKeyword("END_MEMORY")) {
                break;
            }

            // First: OFFSET (required)
            if (!has_offset) {
                if (!matchKeyword("OFFSET")) {
                    setError("MEMORY section must start with OFFSET");
                    return false;
                }
                if (!readInt(current_offset)) {
                    setError("Expected offset value after OFFSET");
                    return false;
                }
                has_offset = true;
                skipLine();
                continue;
            }

            // Second: AVAILABLE (required)
            if (!has_available) {
                if (!matchKeyword("AVAILABLE")) {
                    setError("MEMORY section requires AVAILABLE after OFFSET");
                    return false;
                }
                if (!readInt(memory_available)) {
                    setError("Expected size value after AVAILABLE");
                    return false;
                }
                has_available = true;
                skipLine();
                continue;
            }

            // Then: memory areas in strict order K, X, Y, S, M, T, C
            if (area_index >= num_required) {
                setError("Too many memory areas - expected only K, X, Y, S, M, T, C");
                return false;
            }

            if (memory_area_count >= PROJECT_MAX_MEMORY_AREAS) {
                setError("Too many memory areas defined");
                return false;
            }

            MemoryArea& area = memory_areas[memory_area_count];
            area.reset();

            // Read area name
            if (!readWord(area.name, sizeof(area.name))) {
                setError("Expected memory area name");
                return false;
            }

            // Verify it matches the expected area in sequence
            if (!strEqI(area.name, required_areas[area_index])) {
                char err[128];
                int ei = 0;
                const char* msg = "Expected memory area '";
                while (*msg) err[ei++] = *msg++;
                const char* exp = required_areas[area_index];
                while (*exp) err[ei++] = *exp++;
                msg = "', got '";
                while (*msg) err[ei++] = *msg++;
                int ni = 0;
                while (area.name[ni] && ei < 120) err[ei++] = area.name[ni++];
                err[ei++] = '\'';
                err[ei] = '\0';
                setError(err);
                return false;
            }

            // Read size
            u32 size = 0;
            if (!readInt(size)) {
                setError("Expected size for memory area");
                return false;
            }

            // Calculate start and end from current offset and size
            area.start = current_offset;
            area.end = current_offset + size - 1;
            current_offset += size;

            // Optional RETAIN keyword
            skipWhitespaceNotNewline();
            char word[32];
            int savedPos = pos;
            int savedLine = line;
            int savedCol = column;
            if (readWord(word, sizeof(word))) {
                if (strEqI(word, "RETAIN")) {
                    area.retain = true;
                } else {
                    // Not RETAIN, restore position
                    pos = savedPos;
                    line = savedLine;
                    column = savedCol;
                }
            }

            area.used = true;
            memory_area_count++;
            area_index++;

            skipLine();
        }

        // Verify all required areas were defined
        if (area_index < num_required) {
            char err[128];
            int ei = 0;
            const char* msg = "Missing required memory area: ";
            while (*msg) err[ei++] = *msg++;
            const char* exp = required_areas[area_index];
            while (*exp && ei < 120) err[ei++] = *exp++;
            err[ei] = '\0';
            setError(err);
            return false;
        }

        // Store total allocated (will be recalculated after compilation based on actual usage)
        memory_used = current_offset;

        // Note: Memory limit checking is handled by the front-end editor based on stats
        // if (memory_used > memory_available) { ... }

        return !has_error;
    }

    void setupDefaultMemory() {
        // Set up default memory areas with sequential sizes
        // Format: name, size (offsets calculated sequentially from 0)
        const struct { const char* name; u32 size; } defaults[] = {
            { "K", 64 },    // Control variables
            { "X", 64 },    // Inputs
            { "Y", 64 },    // Outputs
            { "S", 256 },   // System variables
            { "M", 256 },   // Markers
            { "T", 90 },    // Timers
            { "C", 40 },    // Counters
        };

        u32 current_offset = 0;
        for (int i = 0; i < 7 && memory_area_count < PROJECT_MAX_MEMORY_AREAS; i++) {
            MemoryArea& area = memory_areas[memory_area_count];
            area.reset();
            int j = 0;
            while (defaults[i].name[j]) {
                area.name[j] = defaults[i].name[j];
                j++;
            }
            area.name[j] = '\0';
            area.start = current_offset;
            area.end = current_offset + defaults[i].size - 1;
            current_offset += defaults[i].size;
            area.retain = false;
            area.used = true;
            memory_area_count++;
        }
        
        // Set default available and used memory
        memory_used = current_offset;
        memory_available = current_offset;  // Default: available = used
    }

    bool parseFlash() {
        skipComments();

        if (!matchKeyword("FLASH")) {
            // FLASH section is optional - use default (32KB)
            return true;
        }

        while (pos < source_length && !has_error) {
            skipComments();

            if (matchKeyword("END_FLASH")) {
                break;
            }

            // SIZE property
            if (matchKeyword("SIZE")) {
                if (!readInt(flash_size)) {
                    setError("Expected size value after SIZE");
                    return false;
                }
                skipLine();
                continue;
            }

            // Unknown keyword
            char word[32];
            if (readWord(word, sizeof(word))) {
                char err[128];
                int ei = 0;
                const char* msg = "Unknown FLASH property: ";
                while (*msg) err[ei++] = *msg++;
                int wi = 0;
                while (word[wi] && ei < 120) err[ei++] = word[wi++];
                err[ei] = '\0';
                setError(err);
                return false;
            }

            skipLine();
        }

        return !has_error;
    }

    bool parseSymbols() {
        skipComments();

        if (!matchKeyword("SYMBOLS")) {
            // SYMBOLS section is optional
            return true;
        }

        // Collect symbol definitions to pass to PLCASM compiler
        // Format: name : type : address [: comment]
        // We'll build a symbol string compatible with PLCASM's $$ format

        while (pos < source_length && !has_error) {
            skipComments();

            if (matchKeyword("END_SYMBOLS")) {
                break;
            }

            char name[64], type[32], address[64];

            // Read name
            if (!readWord(name, sizeof(name))) {
                skipLine();
                continue;
            }

            // Expect ':'
            skipWhitespaceNotNewline();
            if (peek() != ':') {
                setError("Expected ':' after symbol name");
                return false;
            }
            advance();

            // Read type
            if (!readWord(type, sizeof(type))) {
                setError("Expected type after ':'");
                return false;
            }

            // Expect ':'
            skipWhitespaceNotNewline();
            if (peek() != ':') {
                setError("Expected ':' after type");
                return false;
            }
            advance();

            // Read address
            if (!readWord(address, sizeof(address))) {
                setError("Expected address after ':'");
                return false;
            }

            if (debug_mode) {
                Serial.print(F("Parsed symbol: name='"));
                Serial.print(name);
                Serial.print(F("' type='"));
                Serial.print(type);
                Serial.print(F("' address='"));
                Serial.print(address);
                Serial.println(F("'"));
            }

            // Add symbol to PLCASM compiler's symbol table
            // Convert to the internal format
            if (!addSymbolToCompiler(name, type, address)) {
                return false;
            }

            skipLine();
        }

        return !has_error;
    }

    bool addSymbolToCompiler(const char* name, const char* type, const char* address) {
        // Store symbol in project's own storage (with owned strings)
        // Then copy to PLCASM compiler before each compilation

        if (symbol_count >= PROJECT_MAX_SYMBOLS) {
            setError("Too many symbols");
            return false;
        }

        ProjectSymbol& psym = symbols[symbol_count];

        // Copy name (owned storage)
        int i = 0;
        while (name[i] && i < 63) {
            psym.name[i] = name[i];
            i++;
        }
        psym.name[i] = '\0';

        // Copy type (owned storage)
        i = 0;
        while (type[i] && i < 31) {
            psym.type[i] = type[i];
            i++;
        }
        psym.type[i] = '\0';

        // Determine type size
        if (strEqI(type, "bit") || strEqI(type, "bool")) {
            psym.type_size = 0;
            psym.is_bit = true;
        } else if (strEqI(type, "u8") || strEqI(type, "i8") || strEqI(type, "byte")) {
            psym.type_size = 1;
        } else if (strEqI(type, "u16") || strEqI(type, "i16")) {
            psym.type_size = 2;
        } else if (strEqI(type, "u32") || strEqI(type, "i32") || strEqI(type, "f32")) {
            psym.type_size = 4;
        } else if (strEqI(type, "u64") || strEqI(type, "i64") || strEqI(type, "f64")) {
            psym.type_size = 8;
        } else {
            psym.type_size = 1; // Default to byte
        }

        // Parse address (simplified - handles numeric and prefixed addresses)
        psym.address = parseAddress(address, &psym.bit, &psym.is_bit);

        // Save original address string
        int k = 0;
        while (address[k] && k < 31) {
            psym.address_str[k] = address[k];
            k++;
        }
        psym.address_str[k] = '\0';

        // Check for default value
        skipWhitespaceNotNewline();
        if (peek() == '=') {
            advance(); // Consume '='
            skipWhitespace();

            // Read default value
            if (!readWord(psym.default_value, 31)) {
                setError("Expected default value after '='");
                return false;
            }
            psym.has_default = true;
        }

        psym.used = true;

        // Also add to shared symbol table for cross-compiler access
        int result = addSharedSymbol(psym.name, psym.type, psym.address, psym.bit, psym.is_bit, psym.type_size);
        if (result == -2) {
            // Duplicate symbol with different definition
            char msg[128];
            int mi = 0;
            const char* prefix = "Symbol '";
            while (*prefix && mi < 100) msg[mi++] = *prefix++;
            int ni = 0;
            while (psym.name[ni] && mi < 100) msg[mi++] = psym.name[ni++];
            const char* suffix = "' redefined with different type or address";
            while (*suffix && mi < 127) msg[mi++] = *suffix++;
            msg[mi] = '\0';
            setError(msg);
            return false;
        }
        // result == 0 (new) or result == 1 (exact duplicate, silently ignored) are both OK

        symbol_count++;
        return true;
    }

    // Copy project symbols to PLCASM compiler's symbol table
    void copySymbolsToPLCASM() {
        plcasm_compiler.symbol_count = 0;
        plcasm_compiler.base_symbol_count = 0;

        for (int i = 0; i < symbol_count; i++) {
            ProjectSymbol& psym = symbols[i];
            Symbol& sym = plcasm_compiler.symbols[i];

            // Point StringView data to project's owned storage
            sym.name.data = psym.name;
            sym.name.length = 0;
            while (psym.name[sym.name.length]) sym.name.length++;

            sym.type.data = psym.type;
            sym.type.length = 0;
            while (psym.type[sym.type.length]) sym.type.length++;

            // Copy address string
            int k = 0;
            while (psym.address_str[k] && k < 31) {
                sym.address_str[k] = psym.address_str[k];
                k++;
            }
            sym.address_str[k] = '\0';

            sym.address = psym.address;
            sym.bit = psym.bit;
            sym.is_bit = psym.is_bit;
            sym.type_size = psym.type_size;
            sym.line = line;
            sym.column = column;
        }

        plcasm_compiler.symbol_count = symbol_count;
        plcasm_compiler.base_symbol_count = symbol_count;
    }

    // Copy project symbols to STL compiler's symbol table
    void copySymbolsToSTL() {
        stl_compiler.symbol_count = 0;

        for (int i = 0; i < symbol_count && i < STL_MAX_SYMBOLS; i++) {
            ProjectSymbol& psym = symbols[i];
            STLSymbol& ssym = stl_compiler.symbols[i];

            // Copy name
            int j = 0;
            while (psym.name[j] && j < 63) {
                ssym.name[j] = psym.name[j];
                j++;
            }
            ssym.name[j] = '\0';

            // Copy type
            j = 0;
            while (psym.type[j] && j < 15) {
                ssym.type[j] = psym.type[j];
                j++;
            }
            ssym.type[j] = '\0';

            ssym.address = psym.address;
            ssym.bit = psym.bit;
            ssym.is_bit = psym.is_bit;
            ssym.type_size = psym.type_size;
            ssym.line = line;
            ssym.column = column;
        }

        stl_compiler.symbol_count = symbol_count < STL_MAX_SYMBOLS ? symbol_count : STL_MAX_SYMBOLS;
    }

    u32 parseAddress(const char* addr, u8* bit, bool* is_bit) {
        *is_bit = false;
        *bit = 255;

        // Check for memory area prefix and apply offset
        u32 base_offset = 0;
        int i = 0;

        // Check for letter prefix (M, I, Q, X, Y, etc.)
        char prefix[8] = { 0 };
        int prefix_len = 0;
        while (addr[i] && isAlpha(addr[i]) && prefix_len < 7) {
            prefix[prefix_len++] = addr[i++];
        }
        prefix[prefix_len] = '\0';

        // Look up prefix in memory areas
        if (prefix_len > 0) {
            for (int j = 0; j < memory_area_count; j++) {
                if (strEqI(prefix, memory_areas[j].name)) {
                    base_offset = memory_areas[j].start;
                    break;
                }
            }
            // Special handling for X/Y (bit addresses for I/Q)
            if (strEqI(prefix, "X")) {
                base_offset = plcasm_input_offset;
            } else if (strEqI(prefix, "Y")) {
                base_offset = plcasm_output_offset;
            } else if (strEqI(prefix, "M") && prefix_len == 1) {
                base_offset = plcasm_marker_offset;
            }
        }

        // Parse numeric part
        u32 byte_addr = 0;
        while (addr[i] && isDigit(addr[i])) {
            byte_addr = byte_addr * 10 + (addr[i++] - '0');
        }

        // Check for bit notation (e.g., X0.1)
        if (addr[i] == '.') {
            i++;
            if (addr[i] >= '0' && addr[i] <= '7') {
                *is_bit = true;
                *bit = addr[i] - '0';
            }
        }

        return base_offset + byte_addr;
    }

    bool parseProgramBlocks() {
        while (pos < source_length && !has_error) {
            skipComments();

            if (pos >= source_length) break;

            // Accept both FILE and PROGRAM keywords for backwards compatibility
            bool isFile = matchKeyword("FILE");
            bool isProgram = !isFile && matchKeyword("PROGRAM");

            if (!isFile && !isProgram) {
                // Check for unexpected content
                char word[64];
                if (readWord(word, sizeof(word))) {
                    // Unknown keyword - error
                    char err[128];
                    int ei = 0;
                    const char* msg = "Unexpected keyword: ";
                    while (*msg) err[ei++] = *msg++;
                    int wi = 0;
                    while (word[wi] && ei < 120) err[ei++] = word[wi++];
                    err[ei] = '\0';
                    setError(err);
                    return false;
                }
                break;
            }

            if (program_file_count >= PROJECT_MAX_FILES) {
                setError("Too many program files");
                return false;
            }

            // Read FILE/PROGRAM path
            ProgramFile& file = program_files[program_file_count];
            file.reset();

            if (!readLineUntil(file.path, PROJECT_MAX_PATH_LEN, '/')) {
                setError(isFile ? "Expected FILE path" : "Expected PROGRAM name");
                return false;
            }

            // Set current file context for error tracking
            copyString(current_file, file.path, PROJECT_MAX_PATH_LEN);
            current_block[0] = '\0';

            // Determine execution order
            // 'main' is always first (order 0), others sorted alphabetically
            if (strEqI(file.path, "main")) {
                file.execution_order = 0;
            } else {
                file.execution_order = program_file_count + 1;  // Will be sorted later
            }

            file.first_block_index = program_block_count;
            file.block_count = 0;
            file.used = true;

            skipLine();

            // Parse BLOCKs within this FILE/PROGRAM
            while (pos < source_length && !has_error) {
                skipComments();

                if ((isFile && matchKeyword("END_FILE")) ||
                    (!isFile && matchKeyword("END_PROGRAM"))) {
                    break;  // End of this FILE/PROGRAM
                }

                if (!matchKeyword("BLOCK")) {
                    char word[64];
                    if (readLineUntil(word, sizeof(word), '/')) {
                        char err[128];
                        int ei = 0;
                        const char* msg = isFile ? "Expected BLOCK or END_FILE, got: "
                            : "Expected BLOCK or END_PROGRAM, got: ";
                        while (*msg) err[ei++] = *msg++;
                        int wi = 0;
                        while (word[wi] && ei < 120) err[ei++] = word[wi++];
                        err[ei] = '\0';
                        setError(err);
                        return false;
                    }
                    continue;
                }

                if (program_block_count >= PROJECT_MAX_PROGRAM_BLOCKS) {
                    setError("Too many program blocks");
                    return false;
                }

                ProgramBlock& block = program_blocks[program_block_count];
                block.reset();

                // Copy file path to block
                copyString(block.file_path, file.path, PROJECT_MAX_PATH_LEN);

                // Parse LANG= attribute first (required for BLOCK)
                block.language = LANG_UNKNOWN;
                skipWhitespaceNotNewline();

                char attr[64];
                if (readWord(attr, sizeof(attr))) {
                    if (strncmpI(attr, "LANG=", 5)) {
                        const char* langStr = attr + 5;
                        if (strEqI(langStr, "PLCASM")) block.language = LANG_PLCASM;
                        else if (strEqI(langStr, "STL")) block.language = LANG_STL;
                        else if (strEqI(langStr, "LADDER")) block.language = LANG_LADDER;
                        else if (strEqI(langStr, "FBD")) block.language = LANG_FBD;
                        else if (strEqI(langStr, "ST")) block.language = LANG_ST;
                        else if (strEqI(langStr, "IL")) block.language = LANG_IL;
                        else if (strEqI(langStr, "PLCSCRIPT")) block.language = LANG_PLCSCRIPT;
                        else {
                            char err[128];
                            int ei = 0;
                            const char* msg = "Unknown language: ";
                            while (*msg) err[ei++] = *msg++;
                            while (*langStr && ei < 120) err[ei++] = *langStr++;
                            err[ei] = '\0';
                            setError(err);
                            return false;
                        }
                    } else {
                        setError("BLOCK requires LANG= attribute (e.g., BLOCK LANG=STL My Block Name)");
                        return false;
                    }
                }

                if (block.language == LANG_UNKNOWN) {
                    setError("BLOCK requires LANG= attribute");
                    return false;
                }

                // Read BLOCK name (can contain spaces, reads until end of line or '/')
                if (!readLineUntil(block.name, PROJECT_MAX_NAME_LEN, '/')) {
                    setError("Expected BLOCK name after LANG=");
                    return false;
                }

                // Set current block context for error tracking
                copyString(current_block, block.name, PROJECT_MAX_NAME_LEN);

                // Set current block language for error tracking
                current_block_language = block.language;

                // Skip to next line
                skipLine();

                // Record source start
                block.source_start = pos;

                // Find END_BLOCK
                while (pos < source_length) {
                    skipComments();
                    if (matchKeyword("END_BLOCK")) {
                        block.source_end = pos - 9; // Back up before END_BLOCK
                        // Trim trailing whitespace from source_end
                        while (block.source_end > block.source_start &&
                            (source[block.source_end - 1] == ' ' ||
                                source[block.source_end - 1] == '\t' ||
                                source[block.source_end - 1] == '\r' ||
                                source[block.source_end - 1] == '\n')) {
                            block.source_end--;
                        }
                        break;
                    }
                    skipLine();
                }

                // Check for unterminated block (source_end == source_start means END_BLOCK was never found)
                if (block.source_end == 0) {
                    setError("Unterminated BLOCK - missing END_BLOCK");
                    return false;
                }

                // Empty blocks are allowed (source_end <= source_start after trimming)

                block.used = true;
                program_block_count++;
                file.block_count++;
            }

            // Clear block context when exiting file
            current_block[0] = '\0';
            program_file_count++;
        }

        // Sort files by execution order (main first, then alphabetically)
        sortFilesByExecutionOrder();

        return !has_error;
    }

    // Generate startup block for default values and initialization
    void generateStartupBlock() {
        // Opening banner for the full PLCASM assembly (always shown)
        appendToCombinedPLCASM("// +====================================================================+\n");
        appendToCombinedPLCASM("// |                      START OF VOVKPLCPROJECT                       |\n");
        appendToCombinedPLCASM("// +--------------------------------------------------------------------+\n");
        // Write the name of the project
        appendToCombinedPLCASM("// | NAME: ");
        appendToCombinedPLCASM(project_name);
        // Pad to length
        int name_len = 8; // " NAME: "
        int pn_len = 0;
        while (project_name[pn_len]) pn_len++;
        name_len += pn_len;
        for (int i = name_len; i < 69; i++) appendToCombinedPLCASM(" ");
        appendToCombinedPLCASM("|\n");
        appendToCombinedPLCASM("// +====================================================================+\n\n");

        // Remember position after the main banner, before startup content
        int initial_length = combined_plcasm_length;

        appendToCombinedPLCASM("// |                          Initial setup                             |\n");
        appendToCombinedPLCASM("// +--------------------------------------------------------------------+\n");
        appendToCombinedPLCASM("u8.readBit K20.0\njmp_if_not ____SKIP_STARTUP\n\n");
        appendToCombinedPLCASM("// Write default values\n");

        bool has_content = false;

        for (int i = 0; i < symbol_count; i++) {
            ProjectSymbol& sym = symbols[i];
            if (sym.has_default) {
                has_content = true;
                // Generate write instruction
                // "u8.const <val> \n ptr.const <addr> \n u8.store \n"
                // Need to handle types?
                // For now assuming u8/basic numeric types that parse directly or bit writes

                if (sym.is_bit) {
                    // Bit write
                    if (sym.default_value[0] == '1' || sym.default_value[0] == 't' || sym.default_value[0] == 'T') {
                        appendToCombinedPLCASM("u8.writeBitOn ");
                    } else {
                        appendToCombinedPLCASM("u8.writeBitOff ");
                    }

                    // Use original address string if available, otherwise computed address
                    if (sym.address_str[0] != '\0') {
                        appendToCombinedPLCASM(sym.address_str);
                    } else {
                        appendCombinedPLCASMInt(sym.address);
                        appendToCombinedPLCASM(".");
                        appendCombinedPLCASMInt(sym.bit);
                    }
                    appendToCombinedPLCASM("\n");
                } else {
                    // Byte/Word write
                    // Check type size
                    // If f32/u32, need different store? 
                    // PLCASM usually has specific store instructions or just typed stores.
                    // Let's use simple stores for now assuming standard types
                    const char* type_prefix = "u8";
                    if (sym.type_size == 2) type_prefix = "u16";
                    else if (sym.type_size == 4) type_prefix = "u32"; // or f32? PLCASM usually distinguishes
                    else if (sym.type_size == 8) type_prefix = "u64";

                    if (strEqI(sym.type, "f32")) type_prefix = "f32";
                    else if (strEqI(sym.type, "f64")) type_prefix = "f64";

                    // Value
                    appendToCombinedPLCASM(type_prefix);
                    appendToCombinedPLCASM(".const ");
                    appendToCombinedPLCASM(sym.default_value);
                    appendToCombinedPLCASM("\n");

                    // Store (move_to)
                    appendToCombinedPLCASM(type_prefix);
                    appendToCombinedPLCASM(".move_to ");

                    // Use original address string if available, otherwise computed address
                    if (sym.address_str[0] != '\0') {
                        appendToCombinedPLCASM(sym.address_str);
                    } else {
                        appendCombinedPLCASMInt(sym.address);
                    }
                    appendToCombinedPLCASM("\n");
                }
            }
        }

        appendToCombinedPLCASM("\n// Initialize Differentiation Bits (Safety Skip)\n");
        // Scan for edge detections in raw source or parsed blocks?
        // We can iterate program_blocks, look for "FP"/"FN" (STL) or "RE"/"FE" (PLCASM)
        // And attempt to initialize the memory bit with the input value.
        // This is complex because we need to parse the input expression.
        // Simplification: Look for "A <input>" followed immediately by "FP <mem>" in STL

        for (int i = 0; i < program_block_count; i++) {
            ProgramBlock& block = program_blocks[i];

            // Extract source for this block
            int len = block.source_end - block.source_start;
            if (len > PROJECT_MAX_SOURCE_SIZE - 1) len = PROJECT_MAX_SOURCE_SIZE - 1;

            for (int k = 0; k < len; k++) block_source[k] = source[block.source_start + k];
            block_source[len] = '\0';

            if (block.language == LANG_STL) {
                // STL Scanner
                // Look for:  A <symbol/addr> ... FP <symbol/addr>
                // We parse tokens.
                // Simple token scanner
                char token[64];
                char prev_token[64]; prev_token[0] = '\0';
                char prev_prev_token[64]; prev_prev_token[0] = '\0';

                int ptr = 0;
                while (ptr < len) {
                    // Skip whitespace
                    while (ptr < len && block_source[ptr] <= 32) ptr++;
                    if (ptr >= len) break;

                    // Skip comments (;)
                    if (block_source[ptr] == ';') {
                        while (ptr < len && block_source[ptr] != '\n') ptr++;
                        continue;
                    }

                    // Read token
                    int tstart = ptr;

                    // Simplified isalnum
                    char c = block_source[ptr];
                    bool is_ident = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '.';

                    while (ptr < len && is_ident) {
                        ptr++;
                        if (ptr < len) {
                            c = block_source[ptr];
                            is_ident = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '.';
                        }
                    }

                    int tlen = ptr - tstart;

                    if (tlen == 0) {
                        // Not an identifier start (e.g. operator or garbage), skip one char
                        ptr++;
                        continue;
                    }

                    if (tlen > 63) tlen = 63;

                    for (int j = 0; j < tlen; j++) token[j] = block_source[tstart + j];
                    token[tlen] = '\0';

                    // Check logic: A <input> FP <mem>
                    if (strEqI(token, "FP") || strEqI(token, "FN")) {
                        // Check if previous was an address/symbol
                        // And pre-previous was 'A' (Load) or 'AN' (Load Not) or 'L' (Load)
                        if (prev_token[0] != '\0' && (strEqI(prev_prev_token, "A") || strEqI(prev_prev_token, "L"))) {
                            // "Safely skip": Initialize <mem> with <input>
                            // Generate: A <input> = <mem>

                            // Find 'prev_token' in symbols
                            // Find 'next token' (parameter to FP)

                            // Read next token (memory bit)
                            while (ptr < len && block_source[ptr] <= 32) ptr++;
                            int mstart = ptr;
                            // Read mem token
                            c = block_source[ptr];
                            is_ident = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '.';
                            while (ptr < len && is_ident) {
                                ptr++;
                                if (ptr < len) {
                                    c = block_source[ptr];
                                    is_ident = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '.';
                                }
                            }

                            int mlen = ptr - mstart;
                            char mem_token[64];
                            if (mlen < 64 && mlen > 0) {
                                for (int j = 0; j < mlen; j++) mem_token[j] = block_source[mstart + j];
                                mem_token[mlen] = '\0';

                                int in_idx = findSymbolIndex(prev_token);
                                int mem_idx = findSymbolIndex(mem_token);

                                if (mem_idx >= 0 && symbols[mem_idx].is_bit) {
                                    // We found the memory bit.
                                    // Now result source.
                                    if (in_idx >= 0 && symbols[in_idx].is_bit) {
                                        // Copy Bit Input -> Bit Memory
                                        appendToCombinedPLCASM("u8.readBit ");
                                        appendCombinedPLCASMInt(symbols[in_idx].address);
                                        appendToCombinedPLCASM(".");
                                        appendCombinedPLCASMInt(symbols[in_idx].bit);
                                        appendToCombinedPLCASM("\n");

                                        appendToCombinedPLCASM("u8.writeBit ");
                                        appendCombinedPLCASMInt(symbols[mem_idx].address);
                                        appendToCombinedPLCASM(".");
                                        appendCombinedPLCASMInt(symbols[mem_idx].bit);
                                        appendToCombinedPLCASM("\n");
                                        has_content = true;
                                    }
                                }
                            }
                        }
                    }

                    // Shift tokens
                    copyString(prev_prev_token, prev_token, 64);
                    copyString(prev_token, token, 64);
                }
            }
        }

        if (has_content) {
            appendToCombinedPLCASM("____SKIP_STARTUP:\n// --- END STARTUP ---\n\n");
        } else {
            // Nothing to initialize, remove the startup block header
            combined_plcasm_length = initial_length;
            combined_plcasm[combined_plcasm_length] = '\0';
        }
    }

    int findSymbolIndex(const char* name) {
        for (int i = 0; i < symbol_count; i++) {
            if (strEqI(symbols[i].name, name)) return i;
            // Also check if name matches raw address? 
            // Maybe user used raw address in code like "I0.0".
            // Currently symbol table has defined symbols.
            // If code has "I0.0", it's not in symbols table by name.
        }
        return -1;
    }

    void appendCombinedPLCASMInt(int v) {
        char buf[16];
        int i = 0;
        bool neg = v < 0;
        if (neg) v = -v;
        if (v == 0) buf[i++] = '0';
        else {
            while (v > 0) {
                buf[i++] = (v % 10) + '0';
                v /= 10;
            }
        }
        if (neg) buf[i++] = '-';
        buf[i] = '\0';
        // reverse
        for (int j = 0; j < i / 2; j++) {
            char t = buf[j];
            buf[j] = buf[i - 1 - j];
            buf[i - 1 - j] = t;
        }
        appendToCombinedPLCASM(buf);
    }

    // Sort program files: main first, then alphabetically by path
    void sortFilesByExecutionOrder() {
        // Simple bubble sort for small arrays
        for (int i = 0; i < program_file_count - 1; i++) {
            for (int j = 0; j < program_file_count - i - 1; j++) {
                bool swap = false;

                // 'main' always comes first
                bool jIsMain = strEqI(program_files[j].path, "main");
                bool j1IsMain = strEqI(program_files[j + 1].path, "main");

                if (j1IsMain && !jIsMain) {
                    swap = true;
                } else if (!jIsMain && !j1IsMain) {
                    // Neither is main, sort alphabetically
                    if (strcmp(program_files[j].path, program_files[j + 1].path) > 0) {
                        swap = true;
                    }
                }

                if (swap) {
                    ProgramFile temp = program_files[j];
                    program_files[j] = program_files[j + 1];
                    program_files[j + 1] = temp;
                }
            }
        }

        // Update execution order after sorting
        for (int i = 0; i < program_file_count; i++) {
            program_files[i].execution_order = i;
        }
    }

    int strcmp(const char* a, const char* b) {
        while (*a && *b && *a == *b) {
            a++;
            b++;
        }
        return (unsigned char) *a - (unsigned char) *b;
    }

    bool strncmpI(const char* a, const char* b, int n) {
        for (int i = 0; i < n; i++) {
            char ca = a[i];
            char cb = b[i];
            if (ca >= 'A' && ca <= 'Z') ca += 32;
            if (cb >= 'A' && cb <= 'Z') cb += 32;
            if (ca != cb) return false;
            if (ca == '\0') return true;
        }
        return true;
    }

    // ============ Compilation Functions ============

    // Find the block that contains a given line number in the combined PLCASM
    // Returns the block index, or -1 if not found
    // Also calculates the relative line within that block
    int findBlockForCombinedLine(int combined_line, int& relative_line) {
        for (int i = 0; i < program_block_count; i++) {
            ProgramBlock& block = program_blocks[i];
            if (combined_line >= block.combined_line_start && combined_line <= block.combined_line_end) {
                relative_line = combined_line - block.combined_line_start + 1;
                return i;
            }
        }
        relative_line = combined_line;
        return -1;
    }

    // Set error with context from a combined PLCASM line number
    // Now includes full error details with token info from the linter problem
    void setErrorFromCombinedLine(const char* msg, int combined_line, int col,
        const char* token_text = nullptr, int token_len = 0) {
        int relative_line = 0;
        int block_idx = findBlockForCombinedLine(combined_line, relative_line);

        if (block_idx >= 0) {
            ProgramBlock& block = program_blocks[block_idx];
            copyString(current_file, block.file_path, PROJECT_MAX_PATH_LEN);
            copyString(current_block, block.name, PROJECT_MAX_NAME_LEN);
            current_block_language = block.language;
            // Use setErrorFull with combined_plcasm as source to extract the line
            setErrorFull("PLCASM Compiler", msg, relative_line, col,
                combined_plcasm, combined_plcasm_length,
                token_text, token_len);
        } else {
            // Couldn't map to a block, just use the combined line number
            copyString(error_compiler, "PLCASM Compiler", 64);
            setErrorFull("PLCASM Compiler", msg, combined_line, col,
                combined_plcasm, combined_plcasm_length,
                token_text, token_len);
        }
    }

    // Extract block source to temporary buffer
    bool extractBlockSource(ProgramBlock& block) {
        int len = block.source_end - block.source_start;
        if (len <= 0 || len >= PROJECT_MAX_SOURCE_SIZE - 1) {
            setError("Block source too large or empty");
            return false;
        }

        for (int i = 0; i < len; i++) {
            block_source[i] = source[block.source_start + i];
        }
        block_source[len] = '\0';

        return true;
    }

    // Append string to combined PLCASM buffer, tracking line count
    bool appendToCombinedPLCASM(const char* str) {
        int len = string_len(str);
        if (combined_plcasm_length + len >= PROJECT_MAX_SOURCE_SIZE - 1) {
            setError("Combined PLCASM buffer overflow");
            return false;
        }
        for (int i = 0; i < len; i++) {
            if (str[i] == '\n') combined_plcasm_line++;
            combined_plcasm[combined_plcasm_length++] = str[i];
        }
        combined_plcasm[combined_plcasm_length] = '\0';
        return true;
    }

    // Append character to combined PLCASM buffer, tracking line count
    bool appendCharToCombinedPLCASM(char c) {
        if (combined_plcasm_length >= PROJECT_MAX_SOURCE_SIZE - 1) {
            setError("Combined PLCASM buffer overflow");
            return false;
        }
        if (c == '\n') combined_plcasm_line++;
        combined_plcasm[combined_plcasm_length++] = c;
        combined_plcasm[combined_plcasm_length] = '\0';
        return true;
    }

    // Convert a block to PLCASM and append to combined buffer
    // This is the first pass - just gathering PLCASM from all blocks
    bool convertBlockToPLCASM(ProgramBlock& block) {
        // Set error context
        copyString(current_file, block.file_path, PROJECT_MAX_PATH_LEN);
        copyString(current_block, block.name, PROJECT_MAX_NAME_LEN);
        current_block_language = block.language;

        if (!extractBlockSource(block)) {
            return false;
        }

        if (debug_mode) {
            Serial.print(F("Parsing block: "));
            Serial.print(block.file_path);
            Serial.print(F("/"));
            Serial.print(block.name);
            Serial.print(F(" ("));
            Serial.print(LANG_NAME(block.language));
            Serial.println(F(")"));
        }

        // Add block comment marker with prominent banner
        if (!appendToCombinedPLCASM("// +--------------------------------------------------------------------+\n")) return false;
        if (!appendToCombinedPLCASM("// | BLOCK: ")) return false;
        if (!appendToCombinedPLCASM(block.file_path)) return false;
        if (!appendToCombinedPLCASM("/")) return false;
        if (!appendToCombinedPLCASM(block.name)) return false;
        if (!appendToCombinedPLCASM("\n")) return false;
        if (!appendToCombinedPLCASM("// +--------------------------------------------------------------------+\n")) return false;

        // Add language directive to preserve original source language in bytecode
        // This ensures each block retains its source language marker (FD xx)
        if (!appendToCombinedPLCASM("lang ")) return false;
        if (!appendToCombinedPLCASM(LANG_NAME_LOWER(block.language))) return false;
        if (!appendCharToCombinedPLCASM('\n')) return false;

        // Record the starting line for this block's actual content
        block.combined_line_start = combined_plcasm_line;

        switch (block.language) {
            case LANG_PLCASM:
                // PLCASM block - lint then append
                if (!lintPLCASMBlock(block)) return false;
                if (!appendToCombinedPLCASM(block_source)) return false;
                if (!appendCharToCombinedPLCASM('\n')) return false;
                break;

            case LANG_STL:
                // STL block - convert to PLCASM first
                if (!convertSTLToPLCASM(block)) return false;
                break;

            case LANG_LADDER:
                // Ladder block - convert to STL then to PLCASM
                if (!convertLadderToPLCASM(block)) return false;
                break;

            case LANG_PLCSCRIPT:
                // PLCScript block - convert TypeScript-like code to PLCASM
                if (!convertPLCScriptToPLCASM(block)) return false;
                break;

            default:
                setError("Unsupported language for compilation");
                appendToCombinedPLCASM("// ERROR: Unsupported language for compilation\n");
                return false;
        }

        // Record the ending line for this block
        block.combined_line_end = combined_plcasm_line - 1;  // -1 because we just added a newline

        return true;
    }

    // Lint PLCASM block for errors (used in Pass 1 before appending)
    bool lintPLCASMBlock(ProgramBlock& block) {
        plcasm_compiler.clearArray();
        copySymbolsToPLCASM();

        int source_len = string_len(block_source);
        plcasm_compiler.set_assembly_string(block_source);

        // Use lint mode (false, false) to check for errors without full compilation
        bool error = plcasm_compiler.compileAssembly(false, false);

        if (error || plcasm_compiler.problem_count > 0) {
            // Copy all problems from PLCASM linter to project problems
            for (int i = 0; i < plcasm_compiler.problem_count; i++) {
                LinterProblem& prob = plcasm_compiler.problems[i];
                setErrorFull("PLCASM Linter", prob.message, prob.line, prob.column,
                    block_source, source_len,
                    prob.token_text, prob.length);
            }
            return false;
        }

        return true;
    }

    // Convert STL block to PLCASM and append to combined buffer
    bool convertSTLToPLCASM(ProgramBlock& block) {
        stl_compiler.reset();
        stl_compiler.clearProblems();
        copySymbolsToSTL();

        int source_len = string_len(block_source);
        stl_compiler.setSource(block_source, source_len);

        bool success = stl_compiler.compile();

        if (!success || stl_compiler.has_error) {
            if (stl_compiler.problem_count > 0) {
                LinterProblem& prob = stl_compiler.problems[0];
                // Set error with full details including token
                setErrorFull("STL Compiler", prob.message, prob.line, prob.column,
                    block_source, source_len,
                    prob.token_text, prob.length);
            } else if (stl_compiler.error_message[0]) {
                setErrorFull("STL Compiler", stl_compiler.error_message,
                    stl_compiler.error_line, stl_compiler.error_column,
                    block_source, source_len,
                    nullptr, 0);
            } else {
                setErrorFull("STL Compiler", "STL compilation failed", 1, 1,
                    block_source, source_len, nullptr, 0);
            }
            return false;
        }

        // Append the generated PLCASM to combined buffer
        if (!appendToCombinedPLCASM(stl_compiler.output)) return false;
        if (!appendCharToCombinedPLCASM('\n')) return false;

        return true;
    }

    // Convert PLCScript block to PLCASM and append to combined buffer
    bool convertPLCScriptToPLCASM(ProgramBlock& block) {
        plcscript_compiler.reset();

        int source_len = string_len(block_source);
        plcscript_compiler.setSource(block_source, source_len);

        bool success = plcscript_compiler.compile();

        if (!success || plcscript_compiler.hasError) {
            setErrorFull("PLCScript Compiler", plcscript_compiler.errorMessage,
                plcscript_compiler.errorLine, plcscript_compiler.errorColumn,
                block_source, source_len, nullptr, 0);
            return false;
        }

        // Append the generated PLCASM to combined buffer
        if (!appendToCombinedPLCASM(plcscript_compiler.output)) return false;
        if (!appendCharToCombinedPLCASM('\n')) return false;

        return true;
    }

    // Convert Ladder block to PLCASM and append to combined buffer
    bool convertLadderToPLCASM(ProgramBlock& block) {
        // Compile Ladder JSON to STL first
        ladder_compiler.clearState();
        // Restore the project-level edge memory counter for cross-block continuity
        ladder_compiler.setEdgeMemCounter(project_edge_mem_counter);
        int source_len = string_len(block_source);

        // Use parse() which calls parseGraph() then compile()
        bool success = ladder_compiler.parse(block_source, source_len);

        // Save the updated edge memory counter back to project level
        project_edge_mem_counter = ladder_compiler.getEdgeMemCounter();

        // Always copy problems (warnings, info, errors) from ladder compiler
        if (ladder_compiler.problem_count > 0) {
            copyLadderProblems();
        }

        if (!success || ladder_compiler.hasError()) {
            if (ladder_compiler.problem_count > 0) {
                // Problems already copied above, just set error flag
                has_error = true;
            } else {
                // Fallback to old error handling
                setErrorFull("Ladder Compiler", ladder_compiler.getErrorMsg(), 1, 1,
                    block_source, source_len, nullptr, 0);
            }
            return false;
        }

        // Now compile STL to PLCASM
        // Note: errors from STL compilation of ladder output are internal errors
        // since the ladder compiler generated the STL
        stl_compiler.reset();
        stl_compiler.clearProblems();
        copySymbolsToSTL();
        stl_compiler.setSource(ladder_compiler.output, ladder_compiler.output_len);

        success = stl_compiler.compile();

        if (!success || stl_compiler.has_error) {
            // Internal error: STL generated from ladder failed
            if (stl_compiler.problem_count > 0) {
                LinterProblem& prob = stl_compiler.problems[0];
                setErrorFull("Ladder Compiler (STL stage)", prob.message, prob.line, prob.column,
                    ladder_compiler.output, ladder_compiler.output_len,
                    prob.token_text, prob.length);
            } else {
                setErrorFull("Ladder Compiler (STL stage)", stl_compiler.error_message,
                    stl_compiler.error_line, stl_compiler.error_column,
                    ladder_compiler.output, ladder_compiler.output_len,
                    nullptr, 0);
            }
            return false;
        }

        // Append the generated PLCASM to combined buffer
        if (!appendToCombinedPLCASM(stl_compiler.output)) return false;
        if (!appendCharToCombinedPLCASM('\n')) return false;

        return true;
    }

    // ============ Main Compilation Entry Point ============

    bool compile(const char* project_source, int length, bool debug = false) {
        reset();

        source = project_source;
        source_length = length;
        debug_mode = debug;

        if (debug_mode) {
            Serial.println(F("=== Project Compiler ==="));
        }

        // Parse project structure
        if (!parseProject()) return false;
        if (!parseMemory()) return false;
        if (!parseFlash()) return false;
        if (!parseSymbols()) return false;
        if (!parseProgramBlocks()) return false;

        if (program_block_count == 0) {
            setError("No program blocks found");
            return false;
        }

        if (debug_mode) {
            Serial.print(F("Project: ")); Serial.println(project_name);
            Serial.print(F("Version: ")); Serial.println(project_version);
            Serial.print(F("Memory areas: ")); Serial.println(memory_area_count);
            Serial.print(F("Symbols: ")); Serial.println(symbol_count);
            Serial.print(F("Program blocks: ")); Serial.println(program_block_count);
        }

        // Pass 1: Convert all blocks to PLCASM and concatenate
        if (debug_mode) {
            Serial.println(F("Pass 1: Converting blocks to PLCASM..."));
        }

        combined_plcasm_length = 0;
        combined_plcasm[0] = '\0';
        combined_plcasm_line = 1;

        // Generate Startup code for default values and differentiation logic
        generateStartupBlock();

        bool any_block_error = false;
        for (int i = 0; i < program_block_count; i++) {
            if (!convertBlockToPLCASM(program_blocks[i])) {
                any_block_error = true;
                // Continue processing to collect all errors
            }
        }
        
        // If any block had errors, stop compilation but preserve collected problems
        if (any_block_error || has_error) {
            return false;
        }

        // Add closing banner
        appendToCombinedPLCASM("\n// +====================================================================+\n");
        appendToCombinedPLCASM("// |               END OF PLCASM ASSEMBLY VOVKPLCPROJECT                |\n");
        appendToCombinedPLCASM("// +====================================================================+\n");

        if (debug_mode) {
            Serial.println(F("Combined PLCASM:\n "));
            Serial.println(combined_plcasm);
        }

        // Pass 2: Compile the combined PLCASM to bytecode
        if (debug_mode) {
            Serial.println(F("\n \nPass 2: Compiling combined PLCASM..."));
        }

        output_length = 0;
        output_checksum = 0;

        // Copy project symbols to PLCASM compiler
        copySymbolsToPLCASM();

        // The combined PLCASM already has 'lang' directives for each block
        // to preserve the original source language in the bytecode
        plcasm_compiler.set_assembly_string(combined_plcasm);
        plcasm_compiler.clearArray();

        bool error = plcasm_compiler.compileAssembly(true, false);  // true = real compile, not just analysis

        if (error || plcasm_compiler.problem_count > 0) {
            // Copy all problems from PLCASM compilter to project problems
            if (plcasm_compiler.problem_count > 0) {
                // Determine block from line number
                for (int i = 0; i < plcasm_compiler.problem_count; i++) {
                    LinterProblem& prob = plcasm_compiler.problems[i];
                    setErrorFromCombinedLine(prob.message, prob.line, prob.column, prob.token_text, prob.length);
                }
            } else {
                copyString(error_compiler, "PLCASM Compiler", 64);
                setError("PLCASM compilation failed");
            }
            return false;
        }

        // Copy bytecode to output
        if ((int) plcasm_compiler.built_bytecode_length > PROJECT_MAX_OUTPUT_SIZE) {
            setError("Output buffer overflow");
            return false;
        }

        for (int i = 0; i < (int) plcasm_compiler.built_bytecode_length; i++) {
            output[i] = plcasm_compiler.built_bytecode[i];
            crc8_simple(output_checksum, plcasm_compiler.built_bytecode[i]);
        }
        output_length = plcasm_compiler.built_bytecode_length;

        // Calculate actual memory usage based on bytecode access patterns
        calculateMemoryUsage();

        // Calculate block bytecode offsets and sizes from LANG markers
        calculateBlockOffsets();

        if (debug_mode) {
            Serial.print(F("Compilation complete. Output size: "));
            Serial.print(output_length);
            Serial.print(F(" bytes, checksum: 0x"));
            char c1, c2;
            byteToHex(output_checksum, c1, c2);
            Serial.print(c1); Serial.println(c2);
            Serial.print(F("Memory usage: "));
            Serial.print(memory_used);
            Serial.print(F("/"));
            Serial.print(memory_available);
            Serial.println(F(" bytes"));
        }

        return !has_error;
    }

    // Calculate actual memory usage based on K, X, Y (100%) + accessed addresses in S, M, T, C
    void calculateMemoryUsage() {
        // K, X, Y are assumed 100% used (first 3 areas)
        u32 total_used = 0;
        
        // Add K, X, Y sizes (100% usage)
        for (int i = 0; i < 3 && i < memory_area_count; i++) {
            total_used += (memory_areas[i].end - memory_areas[i].start + 1);
        }
        
        // For S, M, T, C - track highest accessed address in each area
        // We scan the bytecode for memory access instructions
        u32 max_addr_S = 0, max_addr_M = 0, max_addr_T = 0, max_addr_C = 0;
        bool used_S = false, used_M = false, used_T = false, used_C = false;
        
        // Get area boundaries
        u32 S_start = 0, S_end = 0;
        u32 M_start = 0, M_end = 0;
        u32 T_start = 0, T_end = 0;
        u32 C_start = 0, C_end = 0;
        
        for (int i = 0; i < memory_area_count; i++) {
            if (strEqI(memory_areas[i].name, "S")) { S_start = memory_areas[i].start; S_end = memory_areas[i].end; }
            else if (strEqI(memory_areas[i].name, "M")) { M_start = memory_areas[i].start; M_end = memory_areas[i].end; }
            else if (strEqI(memory_areas[i].name, "T")) { T_start = memory_areas[i].start; T_end = memory_areas[i].end; }
            else if (strEqI(memory_areas[i].name, "C")) { C_start = memory_areas[i].start; C_end = memory_areas[i].end; }
        }
        
        // Scan bytecode for memory addresses
        // Memory access instructions typically have address as 2-byte operand
        // Look for patterns in the bytecode that indicate memory access
        for (int i = 0; i < output_length; i++) {
            u8 opcode = output[i];
            
            // Check for memory access opcodes
            // These typically have a 2-byte address following
            bool is_memory_op = false;
            int addr_size = 0;
            
            // Common memory access opcodes (simplified detection)
            // 0x58-0x7F range includes various read/write bit/byte operations
            if ((opcode >= 0x58 && opcode <= 0x7F) || 
                (opcode >= 0x40 && opcode <= 0x57)) {
                is_memory_op = true;
                addr_size = 2;  // 16-bit address
            }
            
            if (is_memory_op && i + addr_size < output_length) {
                u32 addr = output[i + 1] | (output[i + 2] << 8);
                
                // Determine which area this address belongs to
                if (addr >= S_start && addr <= S_end) {
                    used_S = true;
                    if (addr > max_addr_S) max_addr_S = addr;
                } else if (addr >= M_start && addr <= M_end) {
                    used_M = true;
                    if (addr > max_addr_M) max_addr_M = addr;
                } else if (addr >= T_start && addr <= T_end) {
                    used_T = true;
                    if (addr > max_addr_T) max_addr_T = addr;
                } else if (addr >= C_start && addr <= C_end) {
                    used_C = true;
                    if (addr > max_addr_C) max_addr_C = addr;
                }
                
                i += addr_size;  // Skip address bytes
            }
        }
        
        // Also scan symbols for addresses (covers compile-time references)
        // Track the end address (addr + type_size - 1) to properly account for multi-byte types
        for (int i = 0; i < symbol_count; i++) {
            u32 addr = symbols[i].address;
            u8 type_size = symbols[i].type_size;
            if (type_size < 1) type_size = 1;  // Minimum 1 byte for bits
            u32 end_addr = addr + type_size - 1;  // Last byte used by this symbol
            
            if (addr >= S_start && addr <= S_end) {
                used_S = true;
                if (end_addr > max_addr_S) max_addr_S = end_addr;
            } else if (addr >= M_start && addr <= M_end) {
                used_M = true;
                if (end_addr > max_addr_M) max_addr_M = end_addr;
            } else if (addr >= T_start && addr <= T_end) {
                used_T = true;
                if (end_addr > max_addr_T) max_addr_T = end_addr;
            } else if (addr >= C_start && addr <= C_end) {
                used_C = true;
                if (end_addr > max_addr_C) max_addr_C = end_addr;
            }
        }
        
        // Add used bytes for each area (from start to highest accessed address + type size)
        if (used_S && max_addr_S >= S_start) {
            total_used += (max_addr_S - S_start + 1);
        }
        if (used_M && max_addr_M >= M_start) {
            total_used += (max_addr_M - M_start + 1);
        }
        if (used_T && max_addr_T >= T_start) {
            // Timers use 10 bytes each, round up to timer boundary
            u32 timer_bytes = max_addr_T - T_start + 10;  // Add full timer size
            total_used += timer_bytes;
        }
        if (used_C && max_addr_C >= C_start) {
            // Counters use 8 bytes each, round up to counter boundary  
            u32 counter_bytes = max_addr_C - C_start + 8;  // Add full counter size
            total_used += counter_bytes;
        }
        
        memory_used = total_used;
    }

    // Calculate block bytecode offsets and sizes by scanning for LANG markers (0xFD)
    // Each block starts with [LANG, language_id] and ends before the next LANG or END (0xFF)
    void calculateBlockOffsets() {
        if (program_block_count == 0 || output_length == 0) return;

        // Find all LANG marker positions in bytecode
        u32 block_starts[PROJECT_MAX_PROGRAM_BLOCKS];
        int found_blocks = 0;

        for (int i = 0; i < output_length - 1 && found_blocks < PROJECT_MAX_PROGRAM_BLOCKS; i++) {
            if (output[i] == 0xFD) {  // LANG opcode
                block_starts[found_blocks++] = i;
                i++;  // Skip the language ID byte
            }
        }

        // Assign offsets and sizes to blocks
        for (int b = 0; b < program_block_count && b < found_blocks; b++) {
            program_blocks[b].bytecode_offset = block_starts[b];

            // Size is from this block start to next block start (or END marker)
            if (b + 1 < found_blocks) {
                program_blocks[b].bytecode_size = block_starts[b + 1] - block_starts[b];
            } else {
                // Last block - size is to end of bytecode (excluding END marker if present)
                u32 end_pos = output_length;
                if (output_length > 0 && output[output_length - 1] == 0xFF) {
                    end_pos = output_length - 1;  // Exclude END marker
                }
                program_blocks[b].bytecode_size = end_pos - block_starts[b];
            }
        }
    }

    // ============ Output Accessors ============

    u8* getBytecode() { return output; }
    int getBytecodeLength() { return output_length; }
    u8 getChecksum() { return output_checksum; }

    const char* getError() { return error_msg; }
    int getErrorLine() { return error_line; }
    int getErrorColumn() { return error_column; }

    // Print compiled bytecode as hex
    void printBytecode() {
        Serial.print(F("Bytecode ("));
        Serial.print(output_length);
        Serial.print(F(" bytes): "));
        for (int i = 0; i < output_length; i++) {
            char c1, c2;
            byteToHex(output[i], c1, c2);
            Serial.print(c1); Serial.print(c2);
            if (i < output_length - 1) Serial.print(' ');
        }
        Serial.println();
    }

private:
    // Helper to convert byte to hex
    void byteToHex(u8 byte, char& c1, char& c2) {
        static const char hex [] = "0123456789ABCDEF";
        c1 = hex[(byte >> 4) & 0x0F];
        c2 = hex[byte & 0x0F];
    }
};

// Global project compiler instance (using getter pattern for reliable initialization)
ProjectCompiler& getProjectCompiler() {
    static ProjectCompiler instance;
    return instance;
}
#define project_compiler getProjectCompiler()

// ============================================================================
// WASM Export Functions for Project Compiler
// ============================================================================

extern "C" {

    // Compile a full project from source string
    // Returns true on success, false on error
    // Use project_getError() to get error message on failure
    WASM_EXPORT bool project_compile(bool debug = false) {
        // Source is expected to be in the stream buffer
        int len = streamAvailable();
        if (len <= 0) {
            project_compiler.reset();
            project_compiler.setError("No project source in stream buffer");
            return false;
        }

        // Read source from stream
        static char project_source[PROJECT_MAX_SOURCE_SIZE];
        int i = 0;
        while (i < len && i < PROJECT_MAX_SOURCE_SIZE - 1) {
            project_source[i] = __streamInRead();
            i++;
        }
        project_source[i] = '\0';
        streamClear();

        return project_compiler.compile(project_source, i, debug);
    }

    // Compile project from a direct string pointer (for internal use)
    WASM_EXPORT bool project_compileString(char* source, int length, bool debug = false) {
        return project_compiler.compile(source, length, debug);
    }

    // Get the combined PLCASM source (generated during compilation)
    WASM_EXPORT const char* project_getCombinedPLCASM() {
        return project_compiler.combined_plcasm;
    }

    // Get the combined PLCASM source length
    WASM_EXPORT int project_getCombinedPLCASMLength() {
        return project_compiler.combined_plcasm_length;
    }

    // Get the compiled bytecode pointer
    WASM_EXPORT u8* project_getBytecode() {
        return project_compiler.getBytecode();
    }

    // Get the compiled bytecode length
    WASM_EXPORT int project_getBytecodeLength() {
        return project_compiler.getBytecodeLength();
    }

    // Get the compiled bytecode checksum
    WASM_EXPORT u8 project_getChecksum() {
        return project_compiler.getChecksum();
    }

    // Check if compilation had an error
    WASM_EXPORT bool project_hasError() {
        return project_compiler.has_error;
    }

    // Get error message (returns pointer to internal buffer)
    WASM_EXPORT const char* project_getError() {
        return project_compiler.getError();
    }

    // Get error line number
    WASM_EXPORT int project_getErrorLine() {
        return project_compiler.getErrorLine();
    }

    // Get error column number
    WASM_EXPORT int project_getErrorColumn() {
        return project_compiler.getErrorColumn();
    }

    // Get project name after compilation
    WASM_EXPORT const char* project_getName() {
        return project_compiler.project_name;
    }

    // Get project version after compilation
    WASM_EXPORT const char* project_getVersion() {
        return project_compiler.project_version;
    }

    // Get number of program files
    WASM_EXPORT int project_getFileCount() {
        return project_compiler.program_file_count;
    }

    // Get file path by index
    WASM_EXPORT const char* project_getFilePath(int index) {
        if (index < 0 || index >= project_compiler.program_file_count) {
            return "";
        }
        return project_compiler.program_files[index].path;
    }

    // Get file's first block index
    WASM_EXPORT int project_getFileFirstBlockIndex(int index) {
        if (index < 0 || index >= project_compiler.program_file_count) {
            return -1;
        }
        return project_compiler.program_files[index].first_block_index;
    }

    // Get file's block count
    WASM_EXPORT int project_getFileBlockCount(int index) {
        if (index < 0 || index >= project_compiler.program_file_count) {
            return 0;
        }
        return project_compiler.program_files[index].block_count;
    }

    // Get file's execution order
    WASM_EXPORT int project_getFileExecutionOrder(int index) {
        if (index < 0 || index >= project_compiler.program_file_count) {
            return -1;
        }
        return project_compiler.program_files[index].execution_order;
    }

    // Get number of program blocks
    WASM_EXPORT int project_getBlockCount() {
        return project_compiler.program_block_count;
    }

    // Get block name by index
    WASM_EXPORT const char* project_getBlockName(int index) {
        if (index < 0 || index >= project_compiler.program_block_count) {
            return "";
        }
        return project_compiler.program_blocks[index].name;
    }

    // Get block's parent file path by index
    WASM_EXPORT const char* project_getBlockFilePath(int index) {
        if (index < 0 || index >= project_compiler.program_block_count) {
            return "";
        }
        return project_compiler.program_blocks[index].file_path;
    }

    // DEPRECATED: Use project_getBlockFilePath instead
    WASM_EXPORT const char* project_getBlockProgramName(int index) {
        return project_getBlockFilePath(index);
    }

    // Get block language by index
    WASM_EXPORT u8 project_getBlockLanguage(int index) {
        if (index < 0 || index >= project_compiler.program_block_count) {
            return LANG_UNKNOWN;
        }
        return project_compiler.program_blocks[index].language;
    }

    // Get block bytecode offset by index
    WASM_EXPORT u32 project_getBlockOffset(int index) {
        if (index < 0 || index >= project_compiler.program_block_count) {
            return 0;
        }
        return project_compiler.program_blocks[index].bytecode_offset;
    }

    // Get block bytecode size by index
    WASM_EXPORT u32 project_getBlockSize(int index) {
        if (index < 0 || index >= project_compiler.program_block_count) {
            return 0;
        }
        return project_compiler.program_blocks[index].bytecode_size;
    }

    // Get problem count from project linter
    WASM_EXPORT int project_getProblemCount() {
        return project_compiler.problem_count;
    }

    // Get pointer to problems array
    WASM_EXPORT LinterProblem* project_getProblems() {
        return project_compiler.problems;
    }

    // Error context functions
    WASM_EXPORT const char* project_getErrorFile() {
        return project_compiler.error_file;
    }

    WASM_EXPORT const char* project_getErrorBlock() {
        return project_compiler.error_block;
    }

    WASM_EXPORT u8 project_getErrorBlockLanguage() {
        return project_compiler.error_block_language;
    }

    WASM_EXPORT const char* project_getErrorCompiler() {
        return project_compiler.error_compiler;
    }

    WASM_EXPORT const char* project_getErrorSourceLine() {
        return project_compiler.error_source_line;
    }

    WASM_EXPORT const char* project_getErrorToken() {
        return project_compiler.error_token;
    }

    WASM_EXPORT int project_getErrorTokenLength() {
        return project_compiler.error_token_length;
    }

    // Get number of memory areas
    WASM_EXPORT int project_getMemoryAreaCount() {
        return project_compiler.memory_area_count;
    }

    // Get total memory available and used
    WASM_EXPORT u32 project_getMemoryAvailable() {
        return project_compiler.memory_available;
    }

    WASM_EXPORT u32 project_getMemoryUsed() {
        return project_compiler.memory_used;
    }

    // Get flash size and usage (bytecode length)
    WASM_EXPORT u32 project_getFlashSize() {
        return project_compiler.flash_size;
    }

    WASM_EXPORT u32 project_getFlashUsed() {
        return project_compiler.output_length;
    }

    // Get memory area info by index
    WASM_EXPORT const char* project_getMemoryAreaName(int index) {
        if (index < 0 || index >= project_compiler.memory_area_count) {
            return "";
        }
        return project_compiler.memory_areas[index].name;
    }

    WASM_EXPORT u32 project_getMemoryAreaStart(int index) {
        if (index < 0 || index >= project_compiler.memory_area_count) {
            return 0;
        }
        return project_compiler.memory_areas[index].start;
    }

    WASM_EXPORT u32 project_getMemoryAreaEnd(int index) {
        if (index < 0 || index >= project_compiler.memory_area_count) {
            return 0;
        }
        return project_compiler.memory_areas[index].end;
    }

    // Symbol accessor functions
    WASM_EXPORT int project_getSymbolCount() {
        return project_compiler.symbol_count;
    }

    // Static buffers for symbol string returns
    static char symbol_name_buffer[128];
    static char symbol_type_buffer[64];
    static char symbol_address_buffer[32];

    WASM_EXPORT const char* project_getSymbolName(int index) {
        if (index < 0 || index >= project_compiler.symbol_count) {
            return "";
        }
        return project_compiler.symbols[index].name;
    }

    WASM_EXPORT const char* project_getSymbolType(int index) {
        if (index < 0 || index >= project_compiler.symbol_count) {
            return "";
        }
        return project_compiler.symbols[index].type;
    }

    WASM_EXPORT const char* project_getSymbolAddress(int index) {
        if (index < 0 || index >= project_compiler.symbol_count) {
            return "";
        }
        ProjectSymbol& sym = project_compiler.symbols[index];
        // Format address as string
        if (sym.is_bit) {
            // Format as "X0.0" style
            int pos = 0;
            u32 addr = sym.address;
            char temp[16];
            int tempLen = 0;
            if (addr == 0) {
                temp[tempLen++] = '0';
            } else {
                while (addr > 0) {
                    temp[tempLen++] = '0' + (addr % 10);
                    addr /= 10;
                }
            }
            // Reverse
            for (int i = tempLen - 1; i >= 0; i--) {
                symbol_address_buffer[pos++] = temp[i];
            }
            symbol_address_buffer[pos++] = '.';
            symbol_address_buffer[pos++] = '0' + sym.bit;
            symbol_address_buffer[pos] = '\0';
        } else {
            // Format as byte address
            int pos = 0;
            u32 addr = sym.address;
            char temp[16];
            int tempLen = 0;
            if (addr == 0) {
                temp[tempLen++] = '0';
            } else {
                while (addr > 0) {
                    temp[tempLen++] = '0' + (addr % 10);
                    addr /= 10;
                }
            }
            // Reverse
            for (int i = tempLen - 1; i >= 0; i--) {
                symbol_address_buffer[pos++] = temp[i];
            }
            symbol_address_buffer[pos] = '\0';
        }
        return symbol_address_buffer;
    }

    WASM_EXPORT u32 project_getSymbolByteAddress(int index) {
        if (index < 0 || index >= project_compiler.symbol_count) {
            return 0;
        }
        return project_compiler.symbols[index].address;
    }

    WASM_EXPORT u8 project_getSymbolBit(int index) {
        if (index < 0 || index >= project_compiler.symbol_count) {
            return 0;
        }
        return project_compiler.symbols[index].bit;
    }

    WASM_EXPORT bool project_getSymbolIsBit(int index) {
        if (index < 0 || index >= project_compiler.symbol_count) {
            return false;
        }
        return project_compiler.symbols[index].is_bit;
    }

    WASM_EXPORT u8 project_getSymbolTypeSize(int index) {
        if (index < 0 || index >= project_compiler.symbol_count) {
            return 0;
        }
        return project_compiler.symbols[index].type_size;
    }

    // Load compiled project into runtime
    WASM_EXPORT bool project_load() {
        if (project_compiler.has_error || project_compiler.output_length == 0) {
            return false;
        }

        // Load bytecode into runtime (use unsafe load to skip checksum verification
        // since the project compiler already validated the bytecode)
        runtime.loadProgramUnsafe(project_compiler.output, project_compiler.output_length);
        return true;
    }

    // ========== Modifier API (editable constants in bytecode) ==========
    // Modifiers are values embedded in bytecode that can be hot-patched at runtime.
    // These are extracted from the IR entries that have IR_FLAG_EDITABLE set.
    // Examples: timer presets, counter presets, numeric constants.

    // Helper to find block info for a given bytecode offset
    static int findBlockForOffset(u32 offset) {
        for (int i = 0; i < project_compiler.program_block_count; i++) {
            ProgramBlock& block = project_compiler.program_blocks[i];
            if (offset >= block.bytecode_offset && 
                offset < block.bytecode_offset + block.bytecode_size) {
                return i;
            }
        }
        return -1;
    }

    // Get count of editable modifiers in the compiled project
    WASM_EXPORT int project_getModifierCount() {
        int count = 0;
        for (int i = 0; i < project_compiler.plcasm_compiler.ir_entry_count; i++) {
            if (project_compiler.plcasm_compiler.ir_entries[i].flags & IR_FLAG_EDITABLE) {
                count++;
            }
        }
        return count;
    }

    // Get the n-th editable modifier (0-based index among EDITABLE entries)
    static IR_Entry* getModifierEntry(int modifierIndex) {
        int count = 0;
        for (int i = 0; i < project_compiler.plcasm_compiler.ir_entry_count; i++) {
            if (project_compiler.plcasm_compiler.ir_entries[i].flags & IR_FLAG_EDITABLE) {
                if (count == modifierIndex) {
                    return &project_compiler.plcasm_compiler.ir_entries[i];
                }
                count++;
            }
        }
        return nullptr;
    }

    // Static buffers for modifier string returns
    static char modifier_name_buffer[64];
    static char modifier_datatype_buffer[16];
    static char modifier_description_buffer[128];
    static char modifier_token_buffer[64];

    // Get modifier name (generated from opcode and offset)
    WASM_EXPORT const char* project_getModifierName(int index) {
        IR_Entry* entry = getModifierEntry(index);
        if (!entry) return "";

        // Generate name based on opcode
        const char* opname = "const";
        if (entry->flags & IR_FLAG_TIMER) opname = "timer_preset";

        int pos = 0;
        while (*opname && pos < 48) modifier_name_buffer[pos++] = *opname++;
        modifier_name_buffer[pos++] = '_';

        // Add offset as hex
        u32 offset = entry->bytecode_offset;
        char temp[16];
        int tempLen = 0;
        if (offset == 0) {
            temp[tempLen++] = '0';
        } else {
            while (offset > 0 && tempLen < 16) {
                int digit = offset % 16;
                temp[tempLen++] = digit < 10 ? ('0' + digit) : ('a' + digit - 10);
                offset /= 16;
            }
        }
        for (int i = tempLen - 1; i >= 0 && pos < 62; i--) {
            modifier_name_buffer[pos++] = temp[i];
        }
        modifier_name_buffer[pos] = '\0';
        return modifier_name_buffer;
    }

    // Get modifier location: 0 = memory, 1 = program (bytecode)
    WASM_EXPORT int project_getModifierLocation(int index) {
        IR_Entry* entry = getModifierEntry(index);
        if (!entry) return 1; // Default to program
        return 1; // All modifiers are in program bytecode
    }

    // Get modifier byte offset in bytecode
    WASM_EXPORT u32 project_getModifierByteOffset(int index) {
        IR_Entry* entry = getModifierEntry(index);
        if (!entry) return 0;

        // For timer instructions with 2 operands, the preset is operand 2
        // The bytecode_pos tells us where in the instruction the value starts
        if (entry->operand_count >= 2 && (entry->flags & IR_FLAG_TIMER)) {
            // Timer preset is operand 2
            return entry->bytecode_offset + entry->operands[1].bytecode_pos;
        } else if (entry->operand_count >= 1) {
            // First operand
            return entry->bytecode_offset + entry->operands[0].bytecode_pos;
        }
        return entry->bytecode_offset;
    }

    // Get modifier datatype string
    WASM_EXPORT const char* project_getModifierDatatype(int index) {
        IR_Entry* entry = getModifierEntry(index);
        if (!entry) return "";

        // Get the editable operand's type
        IR_OperandType opType = IR_OP_U32; // Default
        if (entry->operand_count >= 2 && (entry->flags & IR_FLAG_TIMER)) {
            opType = (IR_OperandType)entry->operands[1].type;
        } else if (entry->operand_count >= 1) {
            opType = (IR_OperandType)entry->operands[0].type;
        }

        switch (opType) {
            case IR_OP_BOOL: return "bool";
            case IR_OP_I8:   return "i8";
            case IR_OP_U8:   return "u8";
            case IR_OP_I16:  return "i16";
            case IR_OP_U16:  return "u16";
            case IR_OP_I32:  return "i32";
            case IR_OP_U32:  return "u32";
            case IR_OP_I64:  return "i64";
            case IR_OP_U64:  return "u64";
            case IR_OP_F32:  return "f32";
            case IR_OP_F64:  return "f64";
            default:         return "u32";
        }
    }

    // Get modifier size in bytes
    WASM_EXPORT int project_getModifierSize(int index) {
        IR_Entry* entry = getModifierEntry(index);
        if (!entry) return 0;

        IR_OperandType opType = IR_OP_U32;
        if (entry->operand_count >= 2 && (entry->flags & IR_FLAG_TIMER)) {
            opType = (IR_OperandType)entry->operands[1].type;
        } else if (entry->operand_count >= 1) {
            opType = (IR_OperandType)entry->operands[0].type;
        }

        switch (opType) {
            case IR_OP_BOOL:
            case IR_OP_I8:
            case IR_OP_U8:   return 1;
            case IR_OP_I16:
            case IR_OP_U16:  return 2;
            case IR_OP_I32:
            case IR_OP_U32:
            case IR_OP_F32:  return 4;
            case IR_OP_I64:
            case IR_OP_U64:
            case IR_OP_F64:  return 8;
            default:         return 4;
        }
    }

    // Get modifier file path (which file this modifier is in)
    WASM_EXPORT const char* project_getModifierFile(int index) {
        IR_Entry* entry = getModifierEntry(index);
        if (!entry) return "";
        int blockIdx = findBlockForOffset(entry->bytecode_offset);
        if (blockIdx < 0) return "";
        return project_compiler.program_blocks[blockIdx].file_path;
    }

    // Get modifier program name
    WASM_EXPORT const char* project_getModifierProgram(int index) {
        return project_getModifierFile(index); // Same as file path
    }

    // Get modifier block name
    WASM_EXPORT const char* project_getModifierBlock(int index) {
        IR_Entry* entry = getModifierEntry(index);
        if (!entry) return "";
        int blockIdx = findBlockForOffset(entry->bytecode_offset);
        if (blockIdx < 0) return "";
        return project_compiler.program_blocks[blockIdx].name;
    }

    // Get modifier source line number
    WASM_EXPORT int project_getModifierLine(int index) {
        IR_Entry* entry = getModifierEntry(index);
        if (!entry) return 0;
        return entry->source_line;
    }

    // Get modifier source column number
    WASM_EXPORT int project_getModifierColumn(int index) {
        IR_Entry* entry = getModifierEntry(index);
        if (!entry) return 0;
        return entry->source_column;
    }

    // Get modifier token length (in source)
    WASM_EXPORT int project_getModifierLength(int index) {
        // We don't track token length in IR, estimate based on datatype
        IR_Entry* entry = getModifierEntry(index);
        if (!entry) return 0;
        // Return source_column as a fallback estimate (not ideal)
        // In practice, the caller should use the token string length
        return 4; // Default estimate
    }

    // Get modifier token string (the source text)
    WASM_EXPORT const char* project_getModifierToken(int index) {
        IR_Entry* entry = getModifierEntry(index);
        if (!entry) return "";

        // Extract the value and format it as a string
        u64 value = 0;

        if (entry->operand_count >= 2 && (entry->flags & IR_FLAG_TIMER)) {
            value = entry->operands[1].val_u64;
        } else if (entry->operand_count >= 1) {
            value = entry->operands[0].val_u64;
        }

        // Format as string based on type
        int pos = 0;
        if (entry->flags & IR_FLAG_TIMER) {
            // Format as time: T#<value>ms
            const char* prefix = "T#";
            while (*prefix && pos < 60) modifier_token_buffer[pos++] = *prefix++;

            u32 ms = (u32)value;
            char temp[16];
            int tempLen = 0;
            if (ms == 0) {
                temp[tempLen++] = '0';
            } else {
                while (ms > 0 && tempLen < 16) {
                    temp[tempLen++] = '0' + (ms % 10);
                    ms /= 10;
                }
            }
            for (int i = tempLen - 1; i >= 0 && pos < 58; i--) {
                modifier_token_buffer[pos++] = temp[i];
            }
            modifier_token_buffer[pos++] = 'm';
            modifier_token_buffer[pos++] = 's';
        } else {
            // Format as decimal number
            u32 v = (u32)value;
            char temp[16];
            int tempLen = 0;
            if (v == 0) {
                temp[tempLen++] = '0';
            } else {
                while (v > 0 && tempLen < 16) {
                    temp[tempLen++] = '0' + (v % 10);
                    v /= 10;
                }
            }
            for (int i = tempLen - 1; i >= 0 && pos < 62; i--) {
                modifier_token_buffer[pos++] = temp[i];
            }
        }
        modifier_token_buffer[pos] = '\0';
        return modifier_token_buffer;
    }

    // Get modifier description
    WASM_EXPORT const char* project_getModifierDescription(int index) {
        IR_Entry* entry = getModifierEntry(index);
        if (!entry) return "";

        if (entry->flags & IR_FLAG_TIMER) {
            return "Timer preset value (milliseconds)";
        }
        return "Editable constant value";
    }

    // Get modifier current value as u32 (for simple access)
    WASM_EXPORT u32 project_getModifierValue(int index) {
        IR_Entry* entry = getModifierEntry(index);
        if (!entry) return 0;

        if (entry->operand_count >= 2 && (entry->flags & IR_FLAG_TIMER)) {
            return entry->operands[1].val_u32;
        } else if (entry->operand_count >= 1) {
            return entry->operands[0].val_u32;
        }
        return 0;
    }

    // Reset the project compiler state
    WASM_EXPORT void project_reset() {
        project_compiler.reset();
    }

    // Upload compiled bytecode to stream (for download)
    WASM_EXPORT u32 project_uploadBytecode() {
        for (int i = 0; i < project_compiler.output_length; i++) {
            u8 byte = project_compiler.output[i];
            char c1, c2;
            static const char hex [] = "0123456789ABCDEF";
            c1 = hex[(byte >> 4) & 0x0F];
            c2 = hex[byte & 0x0F];
            streamOut(c1);
            streamOut(c2);
        }
        return project_compiler.output_length;
    }

    // Print project compilation info
    WASM_EXPORT void project_printInfo() {
        Serial.println(F("=== Project Compilation Info ==="));
        Serial.print(F("Project: ")); Serial.println(project_compiler.project_name);
        Serial.print(F("Version: ")); Serial.println(project_compiler.project_version);
        Serial.print(F("Memory areas: ")); Serial.println(project_compiler.memory_area_count);
        Serial.print(F("Program files: ")); Serial.println(project_compiler.program_file_count);
        Serial.print(F("Program blocks: ")); Serial.println(project_compiler.program_block_count);

        for (int i = 0; i < project_compiler.program_block_count; i++) {
            ProgramBlock& block = project_compiler.program_blocks[i];
            Serial.print(F("  Block ")); Serial.print(i);
            Serial.print(F(": ")); Serial.print(block.file_path);
            Serial.print(F(".")); Serial.print(block.name);
            Serial.print(F(" (")); Serial.print(LANG_NAME(block.language));
            Serial.print(F(") - offset: ")); Serial.print(block.bytecode_offset);
            Serial.print(F(", size: ")); Serial.println(block.bytecode_size);
        }

        Serial.print(F("Total bytecode: ")); Serial.print(project_compiler.output_length);
        Serial.print(F(" bytes, checksum: 0x"));
        char c1, c2;
        static const char hex [] = "0123456789ABCDEF";
        c1 = hex[(project_compiler.output_checksum >> 4) & 0x0F];
        c2 = hex[project_compiler.output_checksum & 0x0F];
        Serial.print(c1); Serial.println(c2);

        if (project_compiler.has_error) {
            Serial.print(F("Error at line ")); Serial.print(project_compiler.error_line);
            Serial.print(F(":")); Serial.print(project_compiler.error_column);
            Serial.print(F(": ")); Serial.println(project_compiler.error_msg);
        }
    }

} // extern "C"

#endif // __WASM__

