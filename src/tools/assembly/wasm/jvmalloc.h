// jvmalloc.h - 2022-12-11
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

// jvmalloc - memory allocation implementation for wasm without dependencies
// This is a simple implementation of malloc/free that uses a single global memory pool that is allocated at startup.
// The code is by no means fast or efficient, but it is simple and it just works.
// It is designed to be used on the web or in node.js.
// It exports 'malloc' and 'free' symbols along with some string functions.

#ifdef __WASM__
#include "jvint.h"

// Heap size is in bytes and can be changed by defining HEAP_SIZE before including this file.
#ifndef HEAP_SIZE
#define HEAP_SIZE 1 * 1024 * 1024 // Default to 1MB
#endif // HEAP_SIZE

// The maximum number of unique allocations that can be made at once. Can be changed by defining MAX_ALLOCATIONS before including this file. Defaults to 4096.
#ifndef MAX_ALLOCATIONS
#define MAX_ALLOCATIONS 4096
#endif // MAX_ALLOCATIONS

#ifndef NULL
#define NULL 0
#endif // NULL

#ifndef nullptr
#define nullptr 0
#endif // nullptr

// #ifndef WASM_IMPORT
// #define WASM_IMPORT
// #endif // WASM_IMPORT

#define heap_size HEAP_SIZE
#define heap_offset 8
#define heap_padding 8
char heap[heap_size + heap_offset] = { 0 };
int heap_used = 0;
int heap_effective_used = 0;

struct Allocation {
    void* ptr = nullptr;
    uint32_t size = 0;
    bool isAllocated = false;
};

struct Allocation allocations[MAX_ALLOCATIONS] = { };
int allocation_count = 0;

// Reuse freed allocations if possible
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    // memset implementation
    void* memset(void* ptr, int value, int num) {
        char* char_ptr = (char*) ptr;
        for (int i = 0; i < num; i++)
            char_ptr[i] = value;
        return ptr;
    }

    // memcpy implementation
    void* memcpy(void* dest, const void* src, int num) {
        char* char_dest = (char*) dest;
        char* char_src = (char*) src;
        for (int i = 0; i < num; i++)
            char_dest[i] = char_src[i];
        return dest;
    }

    // memcmp implementation
    int memcmp(const void* ptr1, const void* ptr2, int num) {
        char* char_ptr1 = (char*) ptr1;
        char* char_ptr2 = (char*) ptr2;
        for (int i = 0; i < num; i++) {
            if (char_ptr1[i] != char_ptr2[i])
                return char_ptr1[i] - char_ptr2[i];
        }
        return 0;
    }

    // malloc implementation
    void* malloc(short size) {
        if (size <= 0) return nullptr;
        // Find a free allocation that matches the size, or the smallest free allocation that is larger than the size
        // This is to avoid small allocations to take up larger chunks of memory
        short next_free_size = 0;
        int next_free_index = -1;
        for (int i = 0; i < allocation_count; i++) {
            if (!allocations[i].isAllocated && allocations[i].ptr != nullptr) {
                if (allocations[i].size == size) {
                    allocations[i].isAllocated = true;
                    heap_effective_used += size;
                    memset(allocations[i].ptr, 0, allocations[i].size);
                    return allocations[i].ptr;
                }
                if (allocations[i].size > size) {
                    if (next_free_index == -1 || allocations[i].size < next_free_size) {
                        next_free_size = allocations[i].size;
                        next_free_index = i;
                    }
                }
            }
        }
        if (next_free_index != -1) {
            allocations[next_free_index].isAllocated = true;
            heap_effective_used += size;
            memset(allocations[next_free_index].ptr, 0, allocations[next_free_index].size);
            return allocations[next_free_index].ptr;
        }
        if ((heap_used + heap_offset + size) > heap_size) return nullptr;
        if (allocation_count >= MAX_ALLOCATIONS) return nullptr;
        Allocation* alloc = &allocations[allocation_count];
        if (alloc == nullptr) return nullptr;
        alloc->ptr = &heap[heap_used + heap_offset];
        alloc->size = size;
        alloc->isAllocated = true;
        heap_used += size;
        heap_effective_used += size;
        allocation_count++;
        return alloc->ptr;
    }

    // free implementation
    void free(void* ptr) {
        if (ptr == nullptr) return;
        // Find the allocation and free it
        for (int i = 0; i < allocation_count; i++) {
            if (allocations[i].ptr == ptr && allocations[i].isAllocated) {
                allocations[i].isAllocated = false;
                heap_effective_used -= allocations[i].size;
                // NOTE: This is not to standard, but it would be nice to be able to set a pointer to nullptr after freeing it
                // void** ref = reinterpret_cast<void**>(&ptr);
                // *ref = nullptr;
                return;
            }
        }
    }

    // WASM_IMPORT void* malloc(int size);
    // WASM_IMPORT void free(void* ptr);

    // rpmalloc implementation
    void* rpmalloc(int size) { return malloc(size); }

    // _Znam implementation
    void* _Znam(int size) { return malloc(size); }

    // _Znwm implementation
    void* _Znwm(int size) { return malloc(size); }

    // _ZdaPv implementation
    void _ZdaPv(void* ptr) { free(ptr); }

    // _ZdlPv implementation
    void _ZdlPv(void* ptr) { free(ptr); }


    // __cxa_atexit implementation
    int __cxa_atexit(void (*func)(void*), void* arg, void* dso_handle) {
        return 0;
    }

    // strlen implementation
    int strlen(const char* str) {
        int len = 0;
        while (str[len] != '\0') len++;
        return len;
    }

    // strcpy implementation
    char* strcpy(char* dest, const char* src) {
        int len = strlen(src);
        for (int i = 0; i < len; i++)
            dest[i] = src[i];
        dest[len] = '\0';
        return dest;
    }

    // strcmp implementation
    int strcmp(const char* str1, const char* str2) {
        int len1 = strlen(str1);
        int len2 = strlen(str2);
        int len = len1 < len2 ? len1 : len2;
        for (int i = 0; i < len; i++) {
            if (str1[i] != str2[i])
                return str1[i] - str2[i];
        }
        return len1 - len2;
    }

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // __WASM__