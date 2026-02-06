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

    // STR_INIT: Initialize string with capacity and zero length
    // Format: [ STR_INIT, str_type, addr ]
    // Pops u16 capacity from stack, writes header (capacity, length=0) to memory
    RuntimeError handle_STR_INIT(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        SAFE_BOUNDS_CHECK(index + 1 + MY_PTR_SIZE_BYTES > prog_size, PROGRAM_SIZE_EXCEEDED);
        u8 str_type = program[index++];
        MY_PTR_t addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        
        u16 cap = stack.pop_u16();
        
        // Write capacity and length=0 to memory
        if (str_type == type_str8) {
            // str8: [ u8 capacity, u8 length ]
            memory[addr] = (u8)(cap > 254 ? 254 : cap);
            memory[addr + 1] = 0;
        } else {
            // str16: [ u16 capacity, u16 length ]
            write_u16(memory + addr, cap > 65534 ? 65534 : cap);
            write_u16(memory + addr + 2, 0);
        }
        return STATUS_SUCCESS;
    }

    // STR_TO_NUM: Parse string to number -> push result
    // Format: [ STR_TO_NUM, str_type, str_addr, num_type ]
    RuntimeError handle_STR_TO_NUM(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        SAFE_BOUNDS_CHECK(index + 2 + MY_PTR_SIZE_BYTES > prog_size, PROGRAM_SIZE_EXCEEDED);
        u8 str_type = program[index++];
        MY_PTR_t str_addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        u8 num_type = program[index++];
        
        u16 len = str_get_length(memory, str_addr, str_type);
        u8* data = str_data_ptr(memory, str_addr, str_type);
        
        // Parse based on target number type
        bool negative = false;
        u16 start = 0;
        if (len > 0 && data[0] == '-') {
            negative = true;
            start = 1;
        } else if (len > 0 && data[0] == '+') {
            start = 1;
        }
        
        // Check for hex (0x) or binary (0b) prefix
        int base = 10;
        if (start + 1 < len && data[start] == '0') {
            if (data[start + 1] == 'x' || data[start + 1] == 'X') {
                base = 16;
                start += 2;
            } else if (data[start + 1] == 'b' || data[start + 1] == 'B') {
                base = 2;
                start += 2;
            }
        }
        
        // Check for float types
        if (num_type == type_f32 || num_type == type_f64) {
            // For hex/binary, parse as integer first then convert to float
            if (base != 10) {
                u64 intVal = 0;
                for (u16 i = start; i < len; i++) {
                    u8 ch = data[i];
                    int digit = -1;
                    if (ch >= '0' && ch <= '9') digit = ch - '0';
                    else if (ch >= 'a' && ch <= 'f') digit = 10 + (ch - 'a');
                    else if (ch >= 'A' && ch <= 'F') digit = 10 + (ch - 'A');
                    if (digit < 0 || digit >= base) break;
                    intVal = intVal * base + digit;
                }
                double value = (double)intVal;
                if (negative) value = -value;
                if (num_type == type_f32) return stack.push_f32((f32)value);
                else return stack.push_f64(value);
            }
            
            // Parse as double (decimal)
            double value = 0.0;
            bool hasDecimal = false;
            double decimalFactor = 0.1;
            
            for (u16 i = start; i < len; i++) {
                u8 ch = data[i];
                if (ch == '.') {
                    hasDecimal = true;
                } else if (ch >= '0' && ch <= '9') {
                    if (hasDecimal) {
                        value += (ch - '0') * decimalFactor;
                        decimalFactor *= 0.1;
                    } else {
                        value = value * 10 + (ch - '0');
                    }
                } else {
                    break; // Stop at non-digit
                }
            }
            
            if (negative) value = -value;
            
            if (num_type == type_f32) {
                return stack.push_f32((f32)value);
            } else {
                return stack.push_f64(value);
            }
        } else {
            // Parse as integer with base support
            i64 value = 0;
            for (u16 i = start; i < len; i++) {
                u8 ch = data[i];
                int digit = -1;
                if (ch >= '0' && ch <= '9') digit = ch - '0';
                else if (base == 16 && ch >= 'a' && ch <= 'f') digit = 10 + (ch - 'a');
                else if (base == 16 && ch >= 'A' && ch <= 'F') digit = 10 + (ch - 'A');
                if (digit < 0 || digit >= base) break;
                value = value * base + digit;
            }
            
            if (negative) value = -value;
            
            switch (num_type) {
                case type_i8:  return stack.push_i8((i8)value);
                case type_i16: return stack.push_i16((i16)value);
                case type_i32: return stack.push_i32((i32)value);
                case type_i64: return stack.push_i64(value);
                case type_u8:  return stack.push_u8((u8)(u64)value);
                case type_u16: return stack.push_u16((u16)(u64)value);
                case type_u32: return stack.push_u32((u32)(u64)value);
                case type_u64: return stack.push_u64((u64)value);
                case type_bool: return stack.push_u8(value != 0 ? 1 : 0);
                default: return INVALID_DATA_TYPE;
            }
        }
    }

    // STR_FROM_NUM: Convert number (from stack) to string
    // Format: [ STR_FROM_NUM, str_type, str_addr, num_type, u8 base_or_decimals ]
    // For integers: base_or_decimals is the base (2-36, default 10)
    // For floats: base_or_decimals is the number of decimal places
    RuntimeError handle_STR_FROM_NUM(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        SAFE_BOUNDS_CHECK(index + 3 + MY_PTR_SIZE_BYTES > prog_size, PROGRAM_SIZE_EXCEEDED);
        u8 str_type = program[index++];
        MY_PTR_t str_addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        u8 num_type = program[index++];
        u8 base_or_dec = program[index++];
        
        u16 cap = str_get_capacity(memory, str_addr, str_type);
        u8* data = str_data_ptr(memory, str_addr, str_type);
        
        char buf[32];
        int bufLen = 0;
        
        if (num_type == type_f32 || num_type == type_f64) {
            // Float to string with decimal places
            double value = (num_type == type_f32) ? (double)stack.pop_f32() : stack.pop_f64();
            u8 decimals = base_or_dec;
            if (decimals > 15) decimals = 15;
            
            bool neg = value < 0;
            if (neg) value = -value;
            
            // Integer part
            u64 intPart = (u64)value;
            double fracPart = value - (double)intPart;
            
            // Convert integer part to string (in reverse)
            char intBuf[24];
            int intLen = 0;
            if (intPart == 0) {
                intBuf[intLen++] = '0';
            } else {
                while (intPart > 0) {
                    intBuf[intLen++] = '0' + (intPart % 10);
                    intPart /= 10;
                }
            }
            
            // Build output buffer
            if (neg) buf[bufLen++] = '-';
            for (int i = intLen - 1; i >= 0; i--) {
                buf[bufLen++] = intBuf[i];
            }
            
            // Decimal part
            if (decimals > 0) {
                buf[bufLen++] = '.';
                for (u8 d = 0; d < decimals; d++) {
                    fracPart *= 10;
                    int digit = (int)fracPart;
                    buf[bufLen++] = '0' + digit;
                    fracPart -= digit;
                }
            }
        } else {
            // Integer to string with base
            u8 base = base_or_dec;
            if (base < 2) base = 10;
            if (base > 36) base = 36;
            
            bool neg = false;
            u64 value;
            
            switch (num_type) {
                case type_i8:  { i8 v = stack.pop_i8(); neg = v < 0; value = neg ? (u64)(-(i64)v) : (u64)v; } break;
                case type_i16: { i16 v = stack.pop_i16(); neg = v < 0; value = neg ? (u64)(-(i64)v) : (u64)v; } break;
                case type_i32: { i32 v = stack.pop_i32(); neg = v < 0; value = neg ? (u64)(-(i64)v) : (u64)v; } break;
                case type_i64: { i64 v = stack.pop_i64(); neg = v < 0; value = neg ? (u64)(-v) : (u64)v; } break;
                case type_u8:  value = stack.pop_u8(); break;
                case type_u16: value = stack.pop_u16(); break;
                case type_u32: value = stack.pop_u32(); break;
                case type_u64: value = stack.pop_u64(); break;
                case type_bool: value = stack.pop_u8() ? 1 : 0; break;
                default: return INVALID_DATA_TYPE;
            }
            
            // Convert to string (in reverse)
            const char* digits = "0123456789abcdefghijklmnopqrstuvwxyz";
            char intBuf[68]; // enough for 64-bit binary
            int intLen = 0;
            
            if (value == 0) {
                intBuf[intLen++] = '0';
            } else {
                while (value > 0) {
                    intBuf[intLen++] = digits[value % base];
                    value /= base;
                }
            }
            
            // Build output buffer
            if (neg) buf[bufLen++] = '-';
            for (int i = intLen - 1; i >= 0; i--) {
                buf[bufLen++] = intBuf[i];
            }
        }
        
        // Copy to string (truncate if needed)
        u16 copyLen = (bufLen > cap) ? cap : bufLen;
        for (u16 i = 0; i < copyLen; i++) {
            data[i] = buf[i];
        }
        str_set_length(memory, str_addr, str_type, copyLen);
        
        return STATUS_SUCCESS;
    }

    // =========================================================================
    // Constant String Operations (cstr8/cstr16 - immutable strings in program memory)
    // =========================================================================
    // 
    // cstr layout in program memory:
    //   cstr8:  [ u8 length, char[length] ]  - 1 byte header
    //   cstr16: [ u16 length, char[length] ] - 2 byte header (little-endian)
    //
    // These are used for string literals embedded directly in bytecode.

    // CSTR_LIT: Copy inline constant string to mutable string in memory
    // Format: [ CSTR_LIT, dest_type, dest_addr, u16 len, char data... ]
    // The string data is inline in the program immediately after the length.
    // This is a variable-length instruction.
    RuntimeError handle_CSTR_LIT(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        (void)stack; // unused
        
        SAFE_BOUNDS_CHECK(index + 1 + MY_PTR_SIZE_BYTES + 2 > prog_size, PROGRAM_SIZE_EXCEEDED);
        
        // Read destination string type and address
        u8 dest_type = program[index++];
        MY_PTR_t dest_addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        
        // Read inline string length (always u16 in bytecode for consistency)
        u16 src_len = read_u16(program + index);
        index += 2;
        
        SAFE_BOUNDS_CHECK(index + src_len > prog_size, PROGRAM_SIZE_EXCEEDED);
        
        // Get destination capacity
        u16 dest_cap = str_get_capacity(memory, dest_addr, dest_type);
        
        // Copy data (truncate if needed)
        u16 copy_len = (src_len > dest_cap) ? dest_cap : src_len;
        u8* dest_data = str_data_ptr(memory, dest_addr, dest_type);
        
        for (u16 i = 0; i < copy_len; i++) {
            dest_data[i] = program[index + i];
        }
        
        // Update destination length
        str_set_length(memory, dest_addr, dest_type, copy_len);
        
        // Advance past the inline string data
        index += src_len;
        
        return STATUS_SUCCESS;
    }

    // CSTR_CPY: Copy constant string at program offset to mutable string
    // Format: [ CSTR_CPY, cstr_type, dest_type, dest_addr, u16 prog_offset ]
    // The cstr at prog_offset has format: [ len (u8 or u16), char data... ]
    RuntimeError handle_CSTR_CPY(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        (void)stack; // unused
        
        SAFE_BOUNDS_CHECK(index + 2 + MY_PTR_SIZE_BYTES + 2 > prog_size, PROGRAM_SIZE_EXCEEDED);
        
        u8 cstr_type = program[index++];
        u8 dest_type = program[index++];
        MY_PTR_t dest_addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        u16 prog_offset = read_u16(program + index);
        index += 2;
        
        // Read cstr length from program memory
        SAFE_BOUNDS_CHECK(prog_offset >= prog_size, PROGRAM_POINTER_OUT_OF_BOUNDS);
        
        u16 src_len;
        u16 data_offset;
        if (cstr_type == type_cstr8) {
            src_len = program[prog_offset];
            data_offset = prog_offset + 1;
        } else { // type_cstr16
            SAFE_BOUNDS_CHECK(prog_offset + 2 > prog_size, PROGRAM_POINTER_OUT_OF_BOUNDS);
            src_len = read_u16(program + prog_offset);
            data_offset = prog_offset + 2;
        }
        
        SAFE_BOUNDS_CHECK(data_offset + src_len > prog_size, PROGRAM_POINTER_OUT_OF_BOUNDS);
        
        // Get destination capacity
        u16 dest_cap = str_get_capacity(memory, dest_addr, dest_type);
        
        // Copy data (truncate if needed)
        u16 copy_len = (src_len > dest_cap) ? dest_cap : src_len;
        u8* dest_data = str_data_ptr(memory, dest_addr, dest_type);
        
        for (u16 i = 0; i < copy_len; i++) {
            dest_data[i] = program[data_offset + i];
        }
        
        // Update destination length
        str_set_length(memory, dest_addr, dest_type, copy_len);
        
        return STATUS_SUCCESS;
    }

    // CSTR_EQ: Compare constant string at program offset with mutable string
    // Format: [ CSTR_EQ, cstr_type, str_type, str_addr, u16 prog_offset ]
    // Pushes 1 if equal, 0 if not equal
    RuntimeError handle_CSTR_EQ(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        SAFE_BOUNDS_CHECK(index + 2 + MY_PTR_SIZE_BYTES + 2 > prog_size, PROGRAM_SIZE_EXCEEDED);
        
        u8 cstr_type = program[index++];
        u8 str_type = program[index++];
        MY_PTR_t str_addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        u16 prog_offset = read_u16(program + index);
        index += 2;
        
        // Read cstr length from program memory
        SAFE_BOUNDS_CHECK(prog_offset >= prog_size, PROGRAM_POINTER_OUT_OF_BOUNDS);
        
        u16 cstr_len;
        u16 data_offset;
        if (cstr_type == type_cstr8) {
            cstr_len = program[prog_offset];
            data_offset = prog_offset + 1;
        } else { // type_cstr16
            SAFE_BOUNDS_CHECK(prog_offset + 2 > prog_size, PROGRAM_POINTER_OUT_OF_BOUNDS);
            cstr_len = read_u16(program + prog_offset);
            data_offset = prog_offset + 2;
        }
        
        SAFE_BOUNDS_CHECK(data_offset + cstr_len > prog_size, PROGRAM_POINTER_OUT_OF_BOUNDS);
        
        // Get mutable string length
        u16 str_len = str_get_length(memory, str_addr, str_type);
        
        // Quick length check
        if (cstr_len != str_len) {
            return stack.push_u8(0);
        }
        
        // Compare character by character
        u8* str_data = str_data_ptr(memory, str_addr, str_type);
        for (u16 i = 0; i < cstr_len; i++) {
            if (program[data_offset + i] != str_data[i]) {
                return stack.push_u8(0);
            }
        }
        
        return stack.push_u8(1);
    }

    // CSTR_CAT: Concatenate inline constant string to mutable string in memory
    // Format: [ CSTR_CAT, dest_type, dest_addr, u16 len, char data... ]
    // Similar to CSTR_LIT but appends instead of replaces.
    // This is a variable-length instruction.
    RuntimeError handle_CSTR_CAT(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        (void)stack; // unused
        
        SAFE_BOUNDS_CHECK(index + 1 + MY_PTR_SIZE_BYTES + 2 > prog_size, PROGRAM_SIZE_EXCEEDED);
        
        // Read destination string type and address
        u8 dest_type = program[index++];
        MY_PTR_t dest_addr = read_ptr(program + index);
        index += MY_PTR_SIZE_BYTES;
        
        // Read inline string length (always u16 in bytecode for consistency)
        u16 src_len = read_u16(program + index);
        index += 2;
        
        SAFE_BOUNDS_CHECK(index + src_len > prog_size, PROGRAM_SIZE_EXCEEDED);
        
        // Get destination capacity and current length
        u16 dest_cap = str_get_capacity(memory, dest_addr, dest_type);
        u16 dest_len = str_get_length(memory, dest_addr, dest_type);
        
        // Calculate how many chars we can append
        u16 space_left = (dest_cap > dest_len) ? (dest_cap - dest_len) : 0;
        u16 copy_len = (src_len > space_left) ? space_left : src_len;
        
        // Append data
        u8* dest_data = str_data_ptr(memory, dest_addr, dest_type);
        for (u16 i = 0; i < copy_len; i++) {
            dest_data[dest_len + i] = program[index + i];
        }
        
        // Update destination length
        str_set_length(memory, dest_addr, dest_type, dest_len + copy_len);
        
        // Advance past the inline string data
        index += src_len;
        
        return STATUS_SUCCESS;
    }

} // namespace PLCMethods
