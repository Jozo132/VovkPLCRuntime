// methods-string.h - 2026-02-05
//
// Copyright (c) 2024 J.Vovk
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

// String memory layout:
//   str8:  [ u8 capacity, u8 length, char[capacity] ] - header 2 bytes, max 254 chars
//   str16: [ u16 capacity, u16 length, char[capacity] ] - header 4 bytes, max 65534 chars
//
// All string instructions format: [ opcode, u8 str_type, u16 addr, ... ]

namespace PLCMethods {

    // Helper: Get string capacity from memory
    inline u16 str_get_capacity(u8* memory, MY_PTR_t addr, u8 str_type) {
        if (str_type == type_str8) {
            return memory[addr];
        } else { // type_str16
            return read_u16(memory + addr);
        }
    }

    // Helper: Get string length from memory
    inline u16 str_get_length(u8* memory, MY_PTR_t addr, u8 str_type) {
        if (str_type == type_str8) {
            return memory[addr + 1];
        } else { // type_str16
            return read_u16(memory + addr + 2);
        }
    }

    // Helper: Set string length in memory
    inline void str_set_length(u8* memory, MY_PTR_t addr, u8 str_type, u16 len) {
        if (str_type == type_str8) {
            memory[addr + 1] = (u8)len;
        } else { // type_str16
            write_u16(memory + addr + 2, len);
        }
    }

    // Helper: Get string header size
    inline u8 str_header_size(u8 str_type) {
        return (str_type == type_str8) ? 2 : 4;
    }

    // Helper: Get pointer to string data
    inline u8* str_data_ptr(u8* memory, MY_PTR_t addr, u8 str_type) {
        return &memory[addr + str_header_size(str_type)];
    }

    // Helper: Get char at index
    inline u8 str_char_at(u8* memory, MY_PTR_t addr, u8 str_type, u16 index) {
        return memory[addr + str_header_size(str_type) + index];
    }

    // Helper: Set char at index
    inline void str_set_char_at(u8* memory, MY_PTR_t addr, u8 str_type, u16 index, u8 ch) {
        memory[addr + str_header_size(str_type) + index] = ch;
    }

    // STR_LEN: Get string length -> push u16
    RuntimeError handle_STR_LEN(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        SAFE_BOUNDS_CHECK(index + 1 + MY_PTR_SIZE_BYTES > prog_size, PROGRAM_SIZE_EXCEEDED);
        u8 str_type = program[index++];
        MY_PTR_t addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        u16 len = str_get_length(memory, addr, str_type);
        return stack.push_u16(len);
    }

    // STR_CAP: Get string capacity -> push u16
    RuntimeError handle_STR_CAP(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        SAFE_BOUNDS_CHECK(index + 1 + MY_PTR_SIZE_BYTES > prog_size, PROGRAM_SIZE_EXCEEDED);
        u8 str_type = program[index++];
        MY_PTR_t addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        u16 cap = str_get_capacity(memory, addr, str_type);
        return stack.push_u16(cap);
    }

    // STR_GET: Get char at index (pop u16 index) -> push u8 char
    RuntimeError handle_STR_GET(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        SAFE_BOUNDS_CHECK(index + 1 + MY_PTR_SIZE_BYTES > prog_size, PROGRAM_SIZE_EXCEEDED);
        u8 str_type = program[index++];
        MY_PTR_t addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        u16 char_index = stack.pop_u16();
        u16 len = str_get_length(memory, addr, str_type);
        if (char_index >= len) {
            return stack.push_u8(0); // Return null char for out of bounds
        }
        u8 ch = str_char_at(memory, addr, str_type, char_index);
        return stack.push_u8(ch);
    }

    // STR_SET: Set char at index (pop u8 char, pop u16 index)
    RuntimeError handle_STR_SET(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        SAFE_BOUNDS_CHECK(index + 1 + MY_PTR_SIZE_BYTES > prog_size, PROGRAM_SIZE_EXCEEDED);
        u8 str_type = program[index++];
        MY_PTR_t addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        u8 ch = stack.pop_u8();
        u16 char_index = stack.pop_u16();
        u16 len = str_get_length(memory, addr, str_type);
        if (char_index < len) {
            str_set_char_at(memory, addr, str_type, char_index, ch);
        }
        return STATUS_SUCCESS;
    }

    // STR_CLEAR: Clear string (set length to 0)
    RuntimeError handle_STR_CLEAR(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        (void)stack; // unused
        SAFE_BOUNDS_CHECK(index + 1 + MY_PTR_SIZE_BYTES > prog_size, PROGRAM_SIZE_EXCEEDED);
        u8 str_type = program[index++];
        MY_PTR_t addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        str_set_length(memory, addr, str_type, 0);
        return STATUS_SUCCESS;
    }

    // STR_CMP: Compare two strings -> push i8 (-1, 0, 1)
    // Format: [dest_type][src_type][addr1:MY_PTR_t][addr2:MY_PTR_t]
    RuntimeError handle_STR_CMP(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        SAFE_BOUNDS_CHECK(index + 2 + 2 * MY_PTR_SIZE_BYTES > prog_size, PROGRAM_SIZE_EXCEEDED);
        u8 type1 = program[index++];
        u8 type2 = program[index++];
        MY_PTR_t addr1 = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        MY_PTR_t addr2 = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        
        u16 len1 = str_get_length(memory, addr1, type1);
        u16 len2 = str_get_length(memory, addr2, type2);
        u16 min_len = (len1 < len2) ? len1 : len2;
        
        u8* data1 = str_data_ptr(memory, addr1, type1);
        u8* data2 = str_data_ptr(memory, addr2, type2);
        
        for (u16 i = 0; i < min_len; i++) {
            if (data1[i] < data2[i]) return stack.push_i8(-1);
            if (data1[i] > data2[i]) return stack.push_i8(1);
        }
        
        if (len1 < len2) return stack.push_i8(-1);
        if (len1 > len2) return stack.push_i8(1);
        return stack.push_i8(0);
    }

    // STR_EQ: Check string equality -> push bool
    // Format: [dest_type][src_type][addr1:MY_PTR_t][addr2:MY_PTR_t]
    RuntimeError handle_STR_EQ(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        SAFE_BOUNDS_CHECK(index + 2 + 2 * MY_PTR_SIZE_BYTES > prog_size, PROGRAM_SIZE_EXCEEDED);
        u8 type1 = program[index++];
        u8 type2 = program[index++];
        MY_PTR_t addr1 = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        MY_PTR_t addr2 = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        
        u16 len1 = str_get_length(memory, addr1, type1);
        u16 len2 = str_get_length(memory, addr2, type2);
        
        if (len1 != len2) return stack.push_u8(0);
        
        u8* data1 = str_data_ptr(memory, addr1, type1);
        u8* data2 = str_data_ptr(memory, addr2, type2);
        
        for (u16 i = 0; i < len1; i++) {
            if (data1[i] != data2[i]) return stack.push_u8(0);
        }
        return stack.push_u8(1);
    }

    // STR_CONCAT: Concat src to dest (dest = dest + src)
    // Format: [dest_type][src_type][dest_addr:MY_PTR_t][src_addr:MY_PTR_t]
    // Supports cross-type: str8 dest + str16 src, or str16 dest + str8 src
    RuntimeError handle_STR_CONCAT(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        (void)stack;
        SAFE_BOUNDS_CHECK(index + 2 + 2 * MY_PTR_SIZE_BYTES > prog_size, PROGRAM_SIZE_EXCEEDED);
        u8 dest_type = program[index++];
        u8 src_type = program[index++];
        MY_PTR_t dest_addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        MY_PTR_t src_addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        
        u16 dest_cap = str_get_capacity(memory, dest_addr, dest_type);
        u16 dest_len = str_get_length(memory, dest_addr, dest_type);
        u16 src_len = str_get_length(memory, src_addr, src_type);
        
        u16 copy_len = src_len;
        if (dest_len + copy_len > dest_cap) {
            copy_len = dest_cap - dest_len;
        }
        
        u8* dest_data = str_data_ptr(memory, dest_addr, dest_type);
        u8* src_data = str_data_ptr(memory, src_addr, src_type);
        
        for (u16 i = 0; i < copy_len; i++) {
            dest_data[dest_len + i] = src_data[i];
        }
        str_set_length(memory, dest_addr, dest_type, dest_len + copy_len);
        return STATUS_SUCCESS;
    }

    // STR_COPY: Copy src to dest (dest = src)
    // Format: [dest_type][src_type][dest_addr:MY_PTR_t][src_addr:MY_PTR_t]
    // Supports cross-type operations
    RuntimeError handle_STR_COPY(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        (void)stack;
        SAFE_BOUNDS_CHECK(index + 2 + 2 * MY_PTR_SIZE_BYTES > prog_size, PROGRAM_SIZE_EXCEEDED);
        u8 dest_type = program[index++];
        u8 src_type = program[index++];
        MY_PTR_t dest_addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        MY_PTR_t src_addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        
        u16 dest_cap = str_get_capacity(memory, dest_addr, dest_type);
        u16 src_len = str_get_length(memory, src_addr, src_type);
        
        u16 copy_len = (src_len <= dest_cap) ? src_len : dest_cap;
        
        u8* dest_data = str_data_ptr(memory, dest_addr, dest_type);
        u8* src_data = str_data_ptr(memory, src_addr, src_type);
        
        for (u16 i = 0; i < copy_len; i++) {
            dest_data[i] = src_data[i];
        }
        str_set_length(memory, dest_addr, dest_type, copy_len);
        return STATUS_SUCCESS;
    }

    // STR_SUBSTR: Extract substring (pop u16 len, pop u16 start) to dest
    // Format: [dest_type][src_type][dest_addr:MY_PTR_t][src_addr:MY_PTR_t]
    // Supports cross-type operations
    RuntimeError handle_STR_SUBSTR(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        SAFE_BOUNDS_CHECK(index + 2 + 2 * MY_PTR_SIZE_BYTES > prog_size, PROGRAM_SIZE_EXCEEDED);
        u8 dest_type = program[index++];
        u8 src_type = program[index++];
        MY_PTR_t dest_addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        MY_PTR_t src_addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        
        u16 sub_len = stack.pop_u16();
        u16 start = stack.pop_u16();
        
        u16 dest_cap = str_get_capacity(memory, dest_addr, dest_type);
        u16 src_len = str_get_length(memory, src_addr, src_type);
        
        // Clamp start to source length
        if (start >= src_len) {
            str_set_length(memory, dest_addr, dest_type, 0);
            return STATUS_SUCCESS;
        }
        
        // Clamp length
        u16 available = src_len - start;
        if (sub_len > available) sub_len = available;
        if (sub_len > dest_cap) sub_len = dest_cap;
        
        u8* dest_data = str_data_ptr(memory, dest_addr, dest_type);
        u8* src_data = str_data_ptr(memory, src_addr, src_type);
        
        for (u16 i = 0; i < sub_len; i++) {
            dest_data[i] = src_data[start + i];
        }
        str_set_length(memory, dest_addr, dest_type, sub_len);
        return STATUS_SUCCESS;
    }

    // STR_FIND: Find substring -> push i16 index (-1 if not found)
    // Format: [haystack_type][needle_type][haystack_addr:MY_PTR_t][needle_addr:MY_PTR_t]
    // Supports cross-type operations
    RuntimeError handle_STR_FIND(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        SAFE_BOUNDS_CHECK(index + 2 + 2 * MY_PTR_SIZE_BYTES > prog_size, PROGRAM_SIZE_EXCEEDED);
        u8 haystack_type = program[index++];
        u8 needle_type = program[index++];
        MY_PTR_t haystack_addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        MY_PTR_t needle_addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        
        u16 haystack_len = str_get_length(memory, haystack_addr, haystack_type);
        u16 needle_len = str_get_length(memory, needle_addr, needle_type);
        
        if (needle_len == 0) return stack.push_i16(0); // Empty needle found at position 0
        if (needle_len > haystack_len) return stack.push_i16(-1);
        
        u8* haystack = str_data_ptr(memory, haystack_addr, haystack_type);
        u8* needle = str_data_ptr(memory, needle_addr, needle_type);
        
        u16 search_limit = haystack_len - needle_len + 1;
        for (u16 i = 0; i < search_limit; i++) {
            bool found = true;
            for (u16 j = 0; j < needle_len; j++) {
                if (haystack[i + j] != needle[j]) {
                    found = false;
                    break;
                }
            }
            if (found) return stack.push_i16((i16)i);
        }
        return stack.push_i16(-1);
    }

    // STR_CHAR: Append char (pop u8 char)
    RuntimeError handle_STR_CHAR(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        SAFE_BOUNDS_CHECK(index + 1 + MY_PTR_SIZE_BYTES > prog_size, PROGRAM_SIZE_EXCEEDED);
        u8 str_type = program[index++];
        MY_PTR_t addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        
        u8 ch = stack.pop_u8();
        u16 cap = str_get_capacity(memory, addr, str_type);
        u16 len = str_get_length(memory, addr, str_type);
        
        if (len < cap) {
            str_set_char_at(memory, addr, str_type, len, ch);
            str_set_length(memory, addr, str_type, len + 1);
        }
        return STATUS_SUCCESS;
    }

} // namespace PLCMethods
