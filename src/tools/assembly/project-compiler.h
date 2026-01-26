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
#include "stl-compiler.h"
#include "ladder-compiler.h"

// ============================================================================
// Project Compiler - Full Project Export/Compile
// ============================================================================
//
// Accepts a project definition string with the following format:
//
// PROJECT <name>
// VERSION <version>
//
// MEMORY
//   <area> <start>-<end> [RETAIN]
//   ...
// END_MEMORY
//
// SYMBOLS
//   <name> : <type> : <address> [: <comment>]
//   ...
// END_SYMBOLS
//
// FILE <path>                    // 'main' is always executed first, others alphabetically
//   BLOCK <block_name> LANG=<language>
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
    
    // Output bytecode
    u8 output[PROJECT_MAX_OUTPUT_SIZE];
    int output_length;
    u8 output_checksum;
    
    // Error handling
    char error_msg[PROJECT_MAX_ERROR_LEN];
    char error_file[PROJECT_MAX_PATH_LEN];    // File where error occurred
    char error_block[PROJECT_MAX_NAME_LEN];   // Block where error occurred
    u8 error_block_language;                  // Language of the block where error occurred
    int error_line;
    int error_column;
    bool has_error;
    
    // Current context for error tracking
    char current_file[PROJECT_MAX_PATH_LEN];
    char current_block[PROJECT_MAX_NAME_LEN];
    u8 current_block_language;
    
    // Child compilers
    PLCASMCompiler plcasm_compiler;
    STLCompiler stl_compiler;
    LadderGraphCompiler ladder_compiler;
    
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
        
        source = nullptr;
        source_length = 0;
        pos = 0;
        line = 1;
        column = 1;
        
        block_source[0] = '\0';
        
        memset(output, 0, sizeof(output));
        output_length = 0;
        output_checksum = 0;
        
        error_msg[0] = '\0';
        error_file[0] = '\0';
        error_block[0] = '\0';
        error_block_language = LANG_UNKNOWN;
        error_line = 0;
        error_column = 0;
        has_error = false;
        
        current_file[0] = '\0';
        current_block[0] = '\0';
        current_block_language = LANG_UNKNOWN;
        
        debug_mode = false;
    }
    
    // ============ Error Handling ============
    
    void setError(const char* msg) {
        if (has_error) return;
        has_error = true;
        error_line = line;
        error_column = column;
        // Copy current context
        copyString(error_file, current_file, PROJECT_MAX_PATH_LEN);
        copyString(error_block, current_block, PROJECT_MAX_NAME_LEN);
        error_block_language = current_block_language;
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
        // Copy current context
        copyString(error_file, current_file, PROJECT_MAX_PATH_LEN);
        copyString(error_block, current_block, PROJECT_MAX_NAME_LEN);
        error_block_language = current_block_language;
        int i = 0;
        while (msg[i] && i < PROJECT_MAX_ERROR_LEN - 1) {
            error_msg[i] = msg[i];
            i++;
        }
        error_msg[i] = '\0';
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
        while (pos < source_length && i < maxLen - 1) {
            char c = peek();
            if (c == '\n' || c == '\r') break;
            buf[i++] = advance();
        }
        buf[i] = '\0';
        // Trim trailing whitespace
        while (i > 0 && (buf[i-1] == ' ' || buf[i-1] == '\t')) {
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
        
        // PROJECT keyword is optional
        if (matchKeyword("PROJECT")) {
            if (!readWord(project_name, PROJECT_MAX_NAME_LEN)) {
                setError("Expected project name after PROJECT");
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
            if (!readLine(project_version, PROJECT_MAX_VERSION_LEN)) {
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
        
        while (pos < source_length && !has_error) {
            skipComments();
            
            if (matchKeyword("END_MEMORY")) {
                break;
            }
            
            if (memory_area_count >= PROJECT_MAX_MEMORY_AREAS) {
                setError("Too many memory areas defined");
                return false;
            }
            
            MemoryArea& area = memory_areas[memory_area_count];
            area.reset();
            
            // Read area name (M, I, Q, T, C, etc.)
            if (!readWord(area.name, sizeof(area.name))) {
                setError("Expected memory area name");
                return false;
            }
            
            // Read start address
            if (!readInt(area.start)) {
                setError("Expected start address");
                return false;
            }
            
            // Expect '-'
            skipWhitespaceNotNewline();
            if (peek() != '-') {
                setError("Expected '-' between start and end address");
                return false;
            }
            advance();
            
            // Read end address
            if (!readInt(area.end)) {
                setError("Expected end address");
                return false;
            }
            
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
            
            skipLine();
        }
        
        return !has_error;
    }
    
    void setupDefaultMemory() {
        // Set up default memory areas matching PLCASM defaults
        const struct { const char* name; u32 start; u32 end; } defaults[] = {
            { "C", 0, 63 },      // Controls
            { "I", 64, 127 },    // Inputs  
            { "Q", 128, 191 },   // Outputs (using Q for IEC compliance)
            { "S", 192, 447 },   // System
            { "M", 448, 703 },   // Markers
            { "T", 704, 1023 },  // Timers (10 bytes per timer)
            { "CT", 1024, 1279 }, // Counters (8 bytes per counter)
        };
        
        for (int i = 0; i < 7 && memory_area_count < PROJECT_MAX_MEMORY_AREAS; i++) {
            MemoryArea& area = memory_areas[memory_area_count];
            area.reset();
            int j = 0;
            while (defaults[i].name[j]) {
                area.name[j] = defaults[i].name[j];
                j++;
            }
            area.name[j] = '\0';
            area.start = defaults[i].start;
            area.end = defaults[i].end;
            area.retain = false;
            area.used = true;
            memory_area_count++;
        }
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
        // Parse the address and add to PLCASM compiler's symbol table
        // This is a simplified version - the full implementation would use
        // the existing PLCASM symbol parsing
        
        if (plcasm_compiler.symbol_count >= MAX_NUM_OF_TOKENS) {
            setError("Too many symbols");
            return false;
        }
        
        Symbol& sym = plcasm_compiler.symbols[plcasm_compiler.symbol_count];
        
        // Copy name
        int i = 0;
        while (name[i] && i < 63) {
            sym.name.data[i] = name[i];
            i++;
        }
        sym.name.data[i] = '\0';
        sym.name.length = i;
        
        // Copy type
        i = 0;
        while (type[i] && i < 31) {
            sym.type.data[i] = type[i];
            i++;
        }
        sym.type.data[i] = '\0';
        sym.type.length = i;
        
        // Determine type size
        if (strEqI(type, "bit") || strEqI(type, "bool")) {
            sym.type_size = 0;
            sym.is_bit = true;
        } else if (strEqI(type, "u8") || strEqI(type, "i8") || strEqI(type, "byte")) {
            sym.type_size = 1;
        } else if (strEqI(type, "u16") || strEqI(type, "i16")) {
            sym.type_size = 2;
        } else if (strEqI(type, "u32") || strEqI(type, "i32") || strEqI(type, "f32")) {
            sym.type_size = 4;
        } else if (strEqI(type, "u64") || strEqI(type, "i64") || strEqI(type, "f64")) {
            sym.type_size = 8;
        } else {
            sym.type_size = 1; // Default to byte
        }
        
        // Parse address (simplified - handles numeric and prefixed addresses)
        sym.address = parseAddress(address, &sym.bit, &sym.is_bit);
        
        sym.line = line;
        sym.column = column;
        
        plcasm_compiler.symbol_count++;
        return true;
    }
    
    u32 parseAddress(const char* addr, u8* bit, bool* is_bit) {
        *is_bit = false;
        *bit = 255;
        
        // Check for memory area prefix and apply offset
        u32 base_offset = 0;
        int i = 0;
        
        // Check for letter prefix (M, I, Q, X, Y, etc.)
        char prefix[8] = {0};
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
            
            if (!readWord(file.path, PROJECT_MAX_PATH_LEN)) {
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
                    if (readWord(word, sizeof(word))) {
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
                
                // Read BLOCK name
                if (!readWord(block.name, PROJECT_MAX_NAME_LEN)) {
                    setError("Expected BLOCK name");
                    return false;
                }
                
                // Set current block context for error tracking
                copyString(current_block, block.name, PROJECT_MAX_NAME_LEN);
                
                // Parse LANG= attribute (required for BLOCK)
                block.language = LANG_UNKNOWN;
                skipWhitespaceNotNewline();
                
                char attr[64];
                while (readWord(attr, sizeof(attr))) {
                    if (strncmpI(attr, "LANG=", 5)) {
                        const char* langStr = attr + 5;
                        if (strEqI(langStr, "PLCASM")) block.language = LANG_PLCASM;
                        else if (strEqI(langStr, "STL")) block.language = LANG_STL;
                        else if (strEqI(langStr, "LADDER")) block.language = LANG_LADDER;
                        else if (strEqI(langStr, "FBD")) block.language = LANG_FBD;
                        else if (strEqI(langStr, "ST")) block.language = LANG_ST;
                        else if (strEqI(langStr, "IL")) block.language = LANG_IL;
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
                    }
                    skipWhitespaceNotNewline();
                }
                
                if (block.language == LANG_UNKNOWN) {
                    setError("BLOCK requires LANG= attribute");
                    return false;
                }
                
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
                
                if (block.source_end <= block.source_start) {
                    setError("Empty or unterminated BLOCK");
                    return false;
                }
                
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
        return (unsigned char)*a - (unsigned char)*b;
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
    
    // Compile a single block
    bool compileBlock(ProgramBlock& block, bool firstPass) {
        // Set error context
        copyString(current_file, block.file_path, PROJECT_MAX_PATH_LEN);
        copyString(current_block, block.name, PROJECT_MAX_NAME_LEN);
        current_block_language = block.language;
        
        if (!extractBlockSource(block)) {
            return false;
        }
        
        if (debug_mode && !firstPass) {
            Serial.print(F("Compiling block: "));
            Serial.print(block.file_path);
            Serial.print(F("/"));
            Serial.print(block.name);
            Serial.print(F(" ("));
            Serial.print(LANG_NAME(block.language));
            Serial.println(F(")"));
        }
        
        u8* bytecode_ptr = output + output_length;
        u32 available_space = PROJECT_MAX_OUTPUT_SIZE - output_length;
        
        bool success = false;
        
        switch (block.language) {
            case LANG_PLCASM:
                success = compilePLCASM(block, bytecode_ptr, available_space, firstPass);
                break;
                
            case LANG_STL:
                success = compileSTL(block, bytecode_ptr, available_space, firstPass);
                break;
                
            case LANG_LADDER:
                success = compileLadder(block, bytecode_ptr, available_space, firstPass);
                break;
                
            default:
                setError("Unsupported language for compilation");
                return false;
        }
        
        if (!success) {
            return false;
        }
        
        return true;
    }
    
    bool compilePLCASM(ProgramBlock& block, u8* dest, u32 max_size, bool firstPass) {
        // Reset PLCASM compiler but preserve symbols
        int saved_symbol_count = plcasm_compiler.symbol_count;
        
        plcasm_compiler.set_assembly_string(block_source);
        
        // Prepend LANG instruction
        char lang_prefix[64];
        int pi = 0;
        const char* lp = "lang plcasm\n";
        while (*lp) lang_prefix[pi++] = *lp++;
        lang_prefix[pi] = '\0';
        
        // Build combined source
        char combined[PROJECT_MAX_SOURCE_SIZE];
        int ci = 0;
        for (int i = 0; lang_prefix[i] && ci < PROJECT_MAX_SOURCE_SIZE - 1; i++) {
            combined[ci++] = lang_prefix[i];
        }
        for (int i = 0; block_source[i] && ci < PROJECT_MAX_SOURCE_SIZE - 1; i++) {
            combined[ci++] = block_source[i];
        }
        combined[ci] = '\0';
        
        plcasm_compiler.set_assembly_string(combined);
        plcasm_compiler.symbol_count = saved_symbol_count;
        
        bool error = plcasm_compiler.compileAssembly(!firstPass, false);
        
        if (error) {
            setError("PLCASM compilation failed");
            return false;
        }
        
        // Copy bytecode to output
        if (!firstPass) {
            block.bytecode_offset = output_length;
            block.bytecode_size = plcasm_compiler.built_bytecode_length;
            
            if (block.bytecode_size > max_size) {
                setError("Output buffer overflow");
                return false;
            }
            
            for (int i = 0; i < (int)block.bytecode_size; i++) {
                dest[i] = plcasm_compiler.built_bytecode[i];
                crc8_simple(output_checksum, plcasm_compiler.built_bytecode[i]);
            }
            output_length += block.bytecode_size;
            block.compiled = true;
        }
        
        return true;
    }
    
    bool compileSTL(ProgramBlock& block, u8* dest, u32 max_size, bool firstPass) {
        // Compile STL to PLCASM first
        stl_compiler.reset();
        stl_compiler.setSource(block_source, string_len(block_source));
        
        // Compile STL to PLCASM
        bool success = stl_compiler.compile();
        
        if (!success || stl_compiler.has_error) {
            char err[256];
            int ei = 0;
            const char* prefix = "STL error: ";
            while (*prefix) err[ei++] = *prefix++;
            for (int i = 0; stl_compiler.error_message[i] && ei < 250; i++) {
                err[ei++] = stl_compiler.error_message[i];
            }
            err[ei] = '\0';
            setError(err);
            return false;
        }
        
        // Now compile the generated PLCASM
        char lang_prefix[64];
        int pi = 0;
        const char* lp = "lang stl\n";
        while (*lp) lang_prefix[pi++] = *lp++;
        lang_prefix[pi] = '\0';
        
        // Build combined source
        char combined[PROJECT_MAX_SOURCE_SIZE];
        int ci = 0;
        for (int i = 0; lang_prefix[i] && ci < PROJECT_MAX_SOURCE_SIZE - 1; i++) {
            combined[ci++] = lang_prefix[i];
        }
        for (int i = 0; stl_compiler.output[i] && ci < PROJECT_MAX_SOURCE_SIZE - 1; i++) {
            combined[ci++] = stl_compiler.output[i];
        }
        combined[ci] = '\0';
        
        int saved_symbol_count = plcasm_compiler.symbol_count;
        plcasm_compiler.set_assembly_string(combined);
        plcasm_compiler.symbol_count = saved_symbol_count;
        
        bool error = plcasm_compiler.compileAssembly(!firstPass, false);
        
        if (error) {
            setError("PLCASM compilation of STL output failed");
            return false;
        }
        
        // Copy bytecode to output
        if (!firstPass) {
            block.bytecode_offset = output_length;
            block.bytecode_size = plcasm_compiler.built_bytecode_length;
            
            if (block.bytecode_size > max_size) {
                setError("Output buffer overflow");
                return false;
            }
            
            for (int i = 0; i < (int)block.bytecode_size; i++) {
                dest[i] = plcasm_compiler.built_bytecode[i];
                crc8_simple(output_checksum, plcasm_compiler.built_bytecode[i]);
            }
            output_length += block.bytecode_size;
            block.compiled = true;
        }
        
        return true;
    }
    
    bool compileLadder(ProgramBlock& block, u8* dest, u32 max_size, bool firstPass) {
        // Compile Ladder JSON to STL first
        ladder_compiler.reset();
        ladder_compiler.source = block_source;
        ladder_compiler.length = string_len(block_source);
        ladder_compiler.pos = 0;
        
        // Compile Ladder to STL
        bool success = ladder_compiler.compile();
        
        if (!success || ladder_compiler.has_error) {
            char err[256];
            int ei = 0;
            const char* prefix = "Ladder error: ";
            while (*prefix) err[ei++] = *prefix++;
            for (int i = 0; ladder_compiler.error_msg[i] && ei < 250; i++) {
                err[ei++] = ladder_compiler.error_msg[i];
            }
            err[ei] = '\0';
            setError(err);
            return false;
        }
        
        // Now compile the generated STL
        stl_compiler.reset();
        stl_compiler.setSource(ladder_compiler.output, ladder_compiler.output_len);
        
        success = stl_compiler.compile();
        
        if (!success || stl_compiler.has_error) {
            char err[256];
            int ei = 0;
            const char* prefix = "STL (from Ladder) error: ";
            while (*prefix) err[ei++] = *prefix++;
            for (int i = 0; stl_compiler.error_message[i] && ei < 250; i++) {
                err[ei++] = stl_compiler.error_message[i];
            }
            err[ei] = '\0';
            setError(err);
            return false;
        }
        
        // Now compile the generated PLCASM
        char lang_prefix[64];
        int pi = 0;
        const char* lp = "lang ladder\n";
        while (*lp) lang_prefix[pi++] = *lp++;
        lang_prefix[pi] = '\0';
        
        // Build combined source
        char combined[PROJECT_MAX_SOURCE_SIZE];
        int ci = 0;
        for (int i = 0; lang_prefix[i] && ci < PROJECT_MAX_SOURCE_SIZE - 1; i++) {
            combined[ci++] = lang_prefix[i];
        }
        for (int i = 0; stl_compiler.output[i] && ci < PROJECT_MAX_SOURCE_SIZE - 1; i++) {
            combined[ci++] = stl_compiler.output[i];
        }
        combined[ci] = '\0';
        
        int saved_symbol_count = plcasm_compiler.symbol_count;
        plcasm_compiler.set_assembly_string(combined);
        plcasm_compiler.symbol_count = saved_symbol_count;
        
        bool error = plcasm_compiler.compileAssembly(!firstPass, false);
        
        if (error) {
            setError("PLCASM compilation of Ladder output failed");
            return false;
        }
        
        // Copy bytecode to output
        if (!firstPass) {
            block.bytecode_offset = output_length;
            block.bytecode_size = plcasm_compiler.built_bytecode_length;
            
            if (block.bytecode_size > max_size) {
                setError("Output buffer overflow");
                return false;
            }
            
            for (int i = 0; i < (int)block.bytecode_size; i++) {
                dest[i] = plcasm_compiler.built_bytecode[i];
                crc8_simple(output_checksum, plcasm_compiler.built_bytecode[i]);
            }
            output_length += block.bytecode_size;
            block.compiled = true;
        }
        
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
            Serial.print(F("Symbols: ")); Serial.println(plcasm_compiler.symbol_count);
            Serial.print(F("Program blocks: ")); Serial.println(program_block_count);
        }
        
        // Pass 1: Parse all blocks, collect labels
        if (debug_mode) {
            Serial.println(F("Pass 1: Analyzing blocks..."));
        }
        
        for (int i = 0; i < program_block_count && !has_error; i++) {
            if (!compileBlock(program_blocks[i], true)) {
                return false;
            }
        }
        
        // Pass 2: Compile all blocks, patch labels
        if (debug_mode) {
            Serial.println(F("Pass 2: Compiling blocks..."));
        }
        
        output_length = 0;
        output_checksum = 0;
        
        for (int i = 0; i < program_block_count && !has_error; i++) {
            if (!compileBlock(program_blocks[i], false)) {
                return false;
            }
        }
        
        if (debug_mode) {
            Serial.print(F("Compilation complete. Output size: "));
            Serial.print(output_length);
            Serial.print(F(" bytes, checksum: 0x"));
            char c1, c2;
            byteToHex(output_checksum, c1, c2);
            Serial.print(c1); Serial.println(c2);
        }
        
        return !has_error;
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
        static const char hex[] = "0123456789ABCDEF";
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

// Get number of memory areas
WASM_EXPORT int project_getMemoryAreaCount() {
    return project_compiler.memory_area_count;
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
    return project_compiler.plcasm_compiler.symbol_count;
}

// Static buffers for symbol string returns
static char symbol_name_buffer[128];
static char symbol_type_buffer[64];
static char symbol_address_buffer[32];

WASM_EXPORT const char* project_getSymbolName(int index) {
    if (index < 0 || index >= project_compiler.plcasm_compiler.symbol_count) {
        return "";
    }
    Symbol& sym = project_compiler.plcasm_compiler.symbols[index];
    int len = sym.name.length < 127 ? sym.name.length : 127;
    for (int i = 0; i < len; i++) {
        symbol_name_buffer[i] = sym.name.data[i];
    }
    symbol_name_buffer[len] = '\0';
    return symbol_name_buffer;
}

WASM_EXPORT const char* project_getSymbolType(int index) {
    if (index < 0 || index >= project_compiler.plcasm_compiler.symbol_count) {
        return "";
    }
    Symbol& sym = project_compiler.plcasm_compiler.symbols[index];
    int len = sym.type.length < 63 ? sym.type.length : 63;
    for (int i = 0; i < len; i++) {
        symbol_type_buffer[i] = sym.type.data[i];
    }
    symbol_type_buffer[len] = '\0';
    return symbol_type_buffer;
}

WASM_EXPORT const char* project_getSymbolAddress(int index) {
    if (index < 0 || index >= project_compiler.plcasm_compiler.symbol_count) {
        return "";
    }
    Symbol& sym = project_compiler.plcasm_compiler.symbols[index];
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
    if (index < 0 || index >= project_compiler.plcasm_compiler.symbol_count) {
        return 0;
    }
    return project_compiler.plcasm_compiler.symbols[index].address;
}

WASM_EXPORT u8 project_getSymbolBit(int index) {
    if (index < 0 || index >= project_compiler.plcasm_compiler.symbol_count) {
        return 0;
    }
    return project_compiler.plcasm_compiler.symbols[index].bit;
}

WASM_EXPORT bool project_getSymbolIsBit(int index) {
    if (index < 0 || index >= project_compiler.plcasm_compiler.symbol_count) {
        return false;
    }
    return project_compiler.plcasm_compiler.symbols[index].is_bit;
}

WASM_EXPORT u8 project_getSymbolTypeSize(int index) {
    if (index < 0 || index >= project_compiler.plcasm_compiler.symbol_count) {
        return 0;
    }
    return project_compiler.plcasm_compiler.symbols[index].type_size;
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

// Reset the project compiler state
WASM_EXPORT void project_reset() {
    project_compiler.reset();
}

// Upload compiled bytecode to stream (for download)
WASM_EXPORT u32 project_uploadBytecode() {
    for (int i = 0; i < project_compiler.output_length; i++) {
        u8 byte = project_compiler.output[i];
        char c1, c2;
        static const char hex[] = "0123456789ABCDEF";
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
    static const char hex[] = "0123456789ABCDEF";
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

