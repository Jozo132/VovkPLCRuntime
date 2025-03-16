// runtime-tools.h - 2022-12-11
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


// EDIT #### Manually provide `USE_X64_OPS` to enable x64 operations
//// Use x64 operations on all architectures except AVR to fit the memory
// #ifndef __AVR__
// #define USE_X64_OPS
// #endif // __AVR__

#ifdef __SIMULATOR__

#ifndef __WASM__
#include "stdint.h"
#include "stdio.h"
#include "string.h"

int get_used_memory();
#endif // __WASM__

#define PROGMEM
#define pgm_read_byte(x) (*(x))
#define pgm_read_word(x) (*(x))
#define pgm_read_dword(x) (*(x))
#define pgm_read_float(x) (*(x))
#define pgm_read_double(x) (*(x))
#define pgm_read_ptr(x) (*(x))
#define __FlashStringHelper char

#ifndef size_t
#define size_t int
#endif // size_t

#ifndef NULL
#define NULL 0
#endif // NULL

#define LOW 0
#define HIGH 1

#define OUTPUT 0
#define INPUT 1
#define INPUT_PULLUP 2

#define LSBFIRST 0
#define MSBFIRST 1

#define CHANGE 1
#define FALLING 2
#define RISING 3

#define LED_BUILTIN 13


#define HEX 16
#define DEC 10
#define OCT 8
#define BIN 2

#define LN10 2.3025850929940456840179914546844

double ln(double x);

double log10(double x);

#else // __SIMULATOR__
#include <Arduino.h>
#endif // __SIMULATOR__

#include "runtime-types.h"


/* ##################### DEVICE NAME ###################### */
#ifndef device_name
#ifdef __WASM__
const char* device_name = "WASM";
#elif defined(ESP8266)
const char* device_name = "ESP8266";
#elif defined(ESP32)
const char* device_name = "ESP32";
#elif defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_ARCH_MBED_RP2040)
#ifndef ARDUINO_ARCH_RP2040
#define ARDUINO_ARCH_RP2040
#endif // ARDUINO_ARCH_RP2040
#include <hardware/watchdog.h>
#include <String.h>
#ifdef F
#undef F
#endif // F
#define F(x) x
const char* device_name = "RPi Pico";
#elif defined(STM32F1)
const char* device_name = "STM32F1";
#elif defined(STM32F4)
const char* device_name = "STM32F4";
#elif defined(__SIMULATOR__)
const char* device_name = "Simulator";
#else
const char* device_name = "Unknown";
#endif
#endif // device_name




#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#elif defined(ESP8266) || defined(ESP32) || defined(__SIMULATOR__)
// ESP8266 has no sbrk
#else  // __ARM__
extern char* __brkval;
#endif  // __arm__

int freeMemory();
void processExit();

#define LOG_FREE_MEMORY() Serial.print(F("Free memory: ")); Serial.print(freeMemory()); Serial.println(F(" bytes"));

void fstrcpy(char* buff, const char* fstr);
void byteToHex(u8 byte, char& c1, char& c2);

// Define an empty F() macro if it isn't defined, so that the code can be compiled on non-Arduino platforms
#ifndef F 
#define F(x) x
#endif

// If Raspeberry Pi Pico is used
#if defined(ARDUINO_ARCH_RP2040)
#define FSH char
#else 
#define FSH __FlashStringHelper
#endif

#define IGNORE_UNUSED __attribute__((unused))

#define SIZE_OF_ARRAY(a) (sizeof(a) / sizeof(a[0]))

#define STRINGIFY(X) #X

#define CVT_8(type) union u8_to_##type { u8 type_u8; type type_##type; }
#define CVT_16(type) union u16_to_##type { u16 type_u16; type type_##type; }
#define CVT_32(type) union u32_to_##type { u32 type_u32; type type_##type; }

#ifdef USE_X64_OPS
#define CVT_64(type) union u64_to_##type { u64 type_u64; type type_##type; }
#endif // USE_X64_OPS

CVT_32(f32);

#ifdef USE_X64_OPS
CVT_64(f64);
#endif // USE_X64_OPS

union u8A_to_u16 { u8 u8A[2]; u16 _u16; };



// BCD to DEC
u8 bcd2dec(u8 bcd);

// DEC to BCD
u8 dec2bcd(u8 dec);

bool get_u8(u8* memory, u32 offset, u8& value);
bool set_u8(u8* memory, u32 offset, u8 value);
bool readArea_u8(u8* memory, u32 offset, u8* value, u32 size);
bool writeArea_u8(u8* memory, u32 offset, u8* value, u32 size);

template <typename T> T reverse_byte_order(T value) {
    u8* ___reverse_byte_order_ptr = 0;
    u8* ___reverse_byte_order_res_ptr = 0;
    u32 size = sizeof(T);
    switch (size) {
        case 1: return value;
        case 2: {
            u16 result = 0;
            ___reverse_byte_order_ptr = (u8*) &value;
            ___reverse_byte_order_res_ptr = (u8*) &result;
            ___reverse_byte_order_res_ptr[0] = ___reverse_byte_order_ptr[1];
            ___reverse_byte_order_res_ptr[1] = ___reverse_byte_order_ptr[0];
            return (T) result;
        }
        case 4: {
            u32 result = 0;
            ___reverse_byte_order_ptr = (u8*) &value;
            ___reverse_byte_order_res_ptr = (u8*) &result;
            ___reverse_byte_order_res_ptr[0] = ___reverse_byte_order_ptr[3];
            ___reverse_byte_order_res_ptr[1] = ___reverse_byte_order_ptr[2];
            ___reverse_byte_order_res_ptr[2] = ___reverse_byte_order_ptr[1];
            ___reverse_byte_order_res_ptr[3] = ___reverse_byte_order_ptr[0];
            return (T) result;
        }
#ifdef USE_X64_OPS
        case 8: {
            u64 result = 0;
            ___reverse_byte_order_ptr = (u8*) &value;
            ___reverse_byte_order_res_ptr = (u8*) &result;
            ___reverse_byte_order_res_ptr[0] = ___reverse_byte_order_ptr[7];
            ___reverse_byte_order_res_ptr[1] = ___reverse_byte_order_ptr[6];
            ___reverse_byte_order_res_ptr[2] = ___reverse_byte_order_ptr[5];
            ___reverse_byte_order_res_ptr[3] = ___reverse_byte_order_ptr[4];
            ___reverse_byte_order_res_ptr[4] = ___reverse_byte_order_ptr[3];
            ___reverse_byte_order_res_ptr[5] = ___reverse_byte_order_ptr[2];
            ___reverse_byte_order_res_ptr[6] = ___reverse_byte_order_ptr[1];
            ___reverse_byte_order_res_ptr[7] = ___reverse_byte_order_ptr[0];
            return (T) result;
        }
#endif // USE_X64_OPS
        default: return value;
    }
}

#ifndef MAX_PROGRAM_CYCLE_COUNT
#define MAX_PROGRAM_CYCLE_COUNT 200
#endif // MAX_PROGRAM_CYCLE_COUNT

float reverse_byte_order(float value) {
    u32_to_f32 converter;
    converter.type_f32 = value;
    converter.type_u32 = reverse_byte_order(converter.type_u32);
    return converter.type_f32;
}

#ifdef USE_X64_OPS
double reverse_byte_order(double value) {
    u64_to_f64 converter;
    converter.type_f64 = value;
    converter.type_u64 = reverse_byte_order(converter.type_u64);
    return converter.type_f64;
}
#endif // USE_X64_OPS

const char PROGRAM_SIZE_EXCEEDED_MSG [] PROGMEM = "Program size exceeded: ";

#ifdef __SIMULATOR__
void pinMode(int pin, int mode);
void digitalWrite(int pin, int value);
int digitalRead(int pin);
#ifndef __WASM__
void delay(int ms);
void delayMicroseconds(int us);
unsigned long millis();
unsigned long micros();
#endif // __WASM__
void attachInterrupt(int pin, void (*callback)(), int mode);
void detachInterrupt(int pin);
void yield();


#if !defined(__WASM__)
#define STREAM_TO_STDOUT 0
#define STREAM_TO_STDERR 1
#define STREAM_TO_STREAMOUT 2
int print_direction = STREAM_TO_STDOUT;
void __print(char c);
#endif // __WASM__


// To upper case
void toUpper(char* buff);


class Serial_t {
    char input[256];
    int input_len = 0;
    int stream_direction = STREAM_TO_STDOUT;
    void (*print_func)(char) = nullptr;
public:
    operator bool();
    Serial_t(int STREAM_DIRECTION = STREAM_TO_STDOUT);
    void begin(int baudrate);
    void end();
    int print(const char* str);
    int print(const char* str, int len);
    int print(char c);
    int print(int i, int base = DEC);
    int print(unsigned int i, int base = DEC);
    int print(long i, int base = DEC);
    int print(unsigned long i, int base = DEC);
    int print(float i, int decimals = -1);
    int print(double i, int decimals = -1);
    int printf(const char* format, ...);
    int println(const char* str);
    int println(const char* str, int len);
    int println(char c);
    int println(int i, int base = DEC);
    int println(unsigned int i, int base = DEC);
    int println(long i, int base = DEC);
    int println(unsigned long i, int base = DEC);
    int println(double i, int base = DEC);
    int println();
    int available();
    int read();
    void write(char c);
    void write(const char* str);
    void write(const char* str, int len);
    void write(int i, int base = DEC);
    void write(unsigned int i, int base = DEC);
    void write(long i, int base = DEC);
    void write(unsigned long i, int base = DEC);
    void write(double i, int base = DEC);
    void flush();
    void clear();
};

Serial_t Serial(STREAM_TO_STDOUT);
Serial_t Error(STREAM_TO_STDERR);
Serial_t Stream(STREAM_TO_STREAMOUT);

#else // __SIMULATOR__

int get_used_memory();

extern bool serial_timeout;
extern "C"
char serialReadTimeout(u32 timeout = 100);
extern "C"
u8 serialReadHexByteTimeout(u32 timeout = 100);

#endif // __SIMULATOR__

int print_number_padStart(int value, int pad, char padChar = ' ', int base = 10);
int print_number_padEnd(int value, int pad, char padChar = ' ', int base = 10);

#define REPRINT(count, str) for (u8 i = 0; i < count; i++) { Serial.print(str); }
#define REPRINTLN(count, str) REPRINT(count, str); Serial.println();

#include "runtime-tools-impl.h"