#ifndef __WASM__
#define __WASM__
#endif // __WASM__

#include "../include/wasm.h"

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

WASM_EXPORT void doNothing() {
    // Do nothing
}