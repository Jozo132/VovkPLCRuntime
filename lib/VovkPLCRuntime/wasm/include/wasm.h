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


WASM_IMPORT void stdout(char c);
void __print(char c) { stdout(c); }
void _putchar(char c) { stdout(c); }

#include <stdint.h>
#include "jvmalloc.h"


#else // __WASM__
#define WASM_EXPORT
#define WASM_IMPORT
#define WASM_KEEPALIVE
#endif // __WASM__