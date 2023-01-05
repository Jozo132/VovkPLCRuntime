#pragma once

#ifdef __WASM__
#define WASM_EXPORT __attribute__((visibility("default"))) \
    extern "C"
#define WASM_IMPORT extern "C"
#define WASM_KEEPALIVE __attribute__((used))

typedef __SIZE_TYPE__ size_t;

#define WASM_EXPORT_FN(name) \
  __attribute__((export_name(#name))) \
  name

// Declare these as coming from walloc.c.
void *malloc(size_t size);
void free(void *p);
                          
void* WASM_EXPORT_FN(walloc)(size_t size) {
  return malloc(size);
}

void WASM_EXPORT_FN(wfree)(void* ptr) {
  free(ptr);
}

#include <stdint.h>
// #include <stdlib.h>
#include <string.h>

#else // __WASM__
#define WASM_EXPORT
#define WASM_IMPORT
#define WASM_KEEPALIVE
#endif // __WASM__