#pragma once

#ifdef __WASM__
#define WASM_EXPORT __attribute__((visibility("default"))) \
    extern "C"
#define WASM_IMPORT extern "C"
#define WASM_KEEPALIVE __attribute__((used))

#define WASM_EXPORT_FN(name) \
  __attribute__((export_name(#name))) \
  name


WASM_IMPORT void stdout(char c);
WASM_IMPORT void stderr(char c);
WASM_IMPORT void streamOut(char c);


#define STREAM_TO_STDOUT 0
#define STREAM_TO_STDERR 1
#define STREAM_TO_STREAMOUT 2

int print_direction = STREAM_TO_STDOUT;

void __print(char c) {
    switch (print_direction) {
        case 0: return stdout(c);
        case 1: return stderr(c);
        case 2: return streamOut(c);
        default: return stdout(c);
    }
}
void _putchar(char c) {
    return __print(c);
}


#include "jvint.h"
#include "jvmalloc.h"

#include "../lib/printf/printf.h"
#include "../lib/printf/printf.c"


WASM_EXPORT int get_free_memory() {
    // return heap_size - heap_used;
    return heap_size - heap_effective_used;
}

WASM_EXPORT int get_used_memory() {
    // return heap_used;
    return heap_effective_used;
}

WASM_EXPORT int get_total_memory() {
    // return heap_size;
    return heap_used;
}

#else // __WASM__
#define WASM_EXPORT
#define WASM_IMPORT
#define WASM_KEEPALIVE
#endif // __WASM__