#ifndef __WASM__
#define __WASM__
#endif // __WASM__

#include "../include/wasm.h"
#include "../lib/printf/printf.h"
#include "../lib/printf/printf.c"


int freeMemory() {
    return heap_size - heap_used;;
}

WASM_EXPORT int get_free_memory() {
    return freeMemory(); // heap_size - heap_used
}

WASM_EXPORT int get_used_memory() {
    return heap_used;
}

WASM_EXPORT int get_total_memory() {
    return heap_size;
}

WASM_EXPORT void doNothing() {
    // Do nothing
}