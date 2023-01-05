// jvmalloc - J.Vovk memory allocation implementation for wasm without dependencies
// This is a simple implementation of malloc/free that uses a single global memory pool that is allocated at startup.
// The code is by no means fast or efficient, but it is simple and it just works.
// It is designed to be used on the web or in node.js.
// It exports 'malloc' and 'free' symbols along with some string functions.

// Heap size is in bytes and can be changed by defining HEAP_SIZE before including this file.
#ifndef HEAP_SIZE
#define HEAP_SIZE 1 * 1024 * 1024 // Default to 1MB
#endif // HEAP_SIZE

// The maximum number of unique allocations that can be made at once. Can be changed by defining MAX_ALLOCATIONS before including this file. Defaults to 4096.
#ifndef MAX_ALLOCATIONS
#define MAX_ALLOCATIONS 4096
#endif // MAX_ALLOCATIONS

#ifndef NULL
#define NULL ((void*)0)
#endif // NULL

#ifndef nullprt
#define nullprt NULL
#endif // nullprt

char heap[HEAP_SIZE];
int heap_size = HEAP_SIZE;
int heap_used = 0;

struct Allocation {
    void* ptr;
    int size;
    int used;
    int isFree;
};

struct Allocation allocations[MAX_ALLOCATIONS] = { };
int allocation_count = 0;

// Reuse freed allocations if possible
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    // malloc implementation
    void* malloc(int size) {
        if (heap_used + size > heap_size) return NULL;
        for (int i = 0; i < allocation_count; i++) {
            if (allocations[i].isFree && allocations[i].size >= size) {
                allocations[i].used = size;
                allocations[i].isFree = 0;
                return allocations[i].ptr;
            }
        }
        if (allocation_count >= MAX_ALLOCATIONS) return NULL;
        allocations[allocation_count].ptr = &heap[heap_used];
        allocations[allocation_count].size = size;
        allocations[allocation_count].used = size;
        allocations[allocation_count].isFree = 0;
        heap_used += size;
        allocation_count++;
        return allocations[allocation_count - 1].ptr;
    }

    // free implementation
    void free(void* ptr) {
        for (int i = 0; i < allocation_count; i++) {
            if (allocations[i].ptr == ptr) {
                allocations[i].isFree = 1;
                return;
            }
        }
    }

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

    // strlen implementation
    int strlen(const char* str) {
        int len = 0;
        while (str[len] != '\0') len++;
        return len;
    }

    // strcpy implementation
    char* strcpy(char* dest, const char* src) {
        int len = strlen(src);
        for (int i = 0; i < len; i++) {
            dest[i] = src[i];
        }
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

#ifdef __cplusplus
}
#endif // __cplusplus