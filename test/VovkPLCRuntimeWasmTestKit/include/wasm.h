#pragma once

#ifdef __WASM__
#define WASM_EXPORT __attribute__((visibility("default"))) \
    extern "C"
#define WASM_IMPORT extern "C"
#define WASM_KEEPALIVE __attribute__((used))

#define WASM_EXPORT_FN(name) \
  __attribute__((export_name(#name))) \
  name

#ifdef __WASM_TIME__
WASM_IMPORT unsigned long millis(); // const millis = () => +performance.now().toFixed(0);
WASM_IMPORT unsigned long micros(); // const micros = () => +(performance.now() * 1000).toFixed(0);

// WARNING: these delay functions are blocking and stop the main thread for their duration, use with caution and only when necessary.
void delay(int ms) {
    unsigned long t = millis();
    const unsigned long end = t + ms;
    while (t < end) {
        t = millis();
    }
}
void delayMicroseconds(int us) {
    unsigned long t = micros();
    const unsigned long end = t + us;
    while (t < end) {
        t = micros();
    }
}
#else // __WASM_TIME__
unsigned long millis() { return 0; }
unsigned long micros() { return 0; }
void delay(int ms) {}
void delayMicroseconds(int us) {}
#endif // __WASM_TIME__

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