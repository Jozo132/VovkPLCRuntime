// wasm.h - 2022-12-11
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

#include "jvint.h"
#include "jvmath.h"
#include "jvmalloc.h"

#ifdef __WASM__
#define WASM_EXPORT __attribute__((visibility("default"))) \
    extern "C"
#define WASM_IMPORT extern "C"
#define WASM_KEEPALIVE __attribute__((used))

#define WASM_EXPORT_FN(name) \
  __attribute__((export_name(#name))) \
  name

#ifdef __WASM_TIME__
volatile unsigned long _millis_time = 0;
volatile unsigned long _micros_time = 0;
unsigned long millis() { // const millis = () => +performance.now().toFixed(0);
    return _millis_time;
}
unsigned long micros() { // const micros = () => +(performance.now() * 1000).toFixed(0);
    return _micros_time;
}
WASM_EXPORT void setMillis(int ms) {
    _millis_time = ms;
}
WASM_EXPORT void setMicros(int us) {
    _micros_time = us;
}
WASM_EXPORT unsigned long getMillis() {
    return _millis_time;
}
WASM_EXPORT unsigned long getMicros() {
    return _micros_time;
}

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

#ifndef __WASM_STREAM_SIZE_IN__
#define __WASM_STREAM_SIZE_IN__ 64536
#endif // __WASM_STREAM_SIZE_IN__

// Circular buffer
char __wasm_stream_in__[__WASM_STREAM_SIZE_IN__];
int __wasm_stream_in_cursor__ = 0;
int __wasm_stream_in_index__ = 0;

WASM_EXPORT bool streamIn(char c) { // Retrun true if the char was written to the buffer
    int next = (__wasm_stream_in_index__ + 1) % __WASM_STREAM_SIZE_IN__;
    if (next == __wasm_stream_in_cursor__) return false;
    __wasm_stream_in__[__wasm_stream_in_index__] = c;
    __wasm_stream_in_index__ = next;
    return true;
}
char __streamInRead() {
    if (__wasm_stream_in_cursor__ == __wasm_stream_in_index__) return '\0';
    char c = __wasm_stream_in__[__wasm_stream_in_cursor__];
    __wasm_stream_in_cursor__ = (__wasm_stream_in_cursor__ + 1) % __WASM_STREAM_SIZE_IN__;
    return c;
}


int streamAvailable() { // Return the size of the buffer
    if (__wasm_stream_in_index__ >= __wasm_stream_in_cursor__) return __wasm_stream_in_index__ - __wasm_stream_in_cursor__;
    return __WASM_STREAM_SIZE_IN__ - __wasm_stream_in_cursor__ + __wasm_stream_in_index__;
}
// Inject the wasm stream into the char array reference and update its length, also check for the max length and return false if there is no problem 
bool streamRead(char* arr, int& len, int max) {
    int i = 0;
    while (i < max - 1) {
        if (streamAvailable() == 0) break;
        arr[i] = __streamInRead();
        i++;
    }
    arr[i] = '\0';
    len = i;
    return i == max;
}
bool streamRead(unsigned char* arr, int& len, int max) {
    return streamRead((char*) arr, len, max);
}


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

#include "printf/printf.h"
#include "printf/printf.c"


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