// runtime-tools.h - 1.0.0 - 2022-12-11
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
#ifndef __RUNTIME_TOOLS_H__
#define __RUNTIME_TOOLS_H__

// Use x64 operations on all architectures except AVR to fit the memory
#ifndef __AVR__
#define USE_X64_OPS
#endif // __AVR__

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
void byteToHex(uint8_t byte, char& c1, char& c2);

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

#define CVT_8(type) union uint8_t_to_##type { uint8_t type_uint8_t; type type_##type; }
#define CVT_16(type) union uint16_t_to_##type { uint16_t type_uint16_t; type type_##type; }
#define CVT_32(type) union uint32_t_to_##type { uint32_t type_uint32_t; type type_##type; }
#define CVT_64(type) union uint64_t_to_##type { uint64_t type_uint64_t; type type_##type; }


CVT_32(float);
CVT_64(double);

union u8A_to_u16 { uint8_t u8A[2]; uint16_t u16; };


// BCD to DEC
uint8_t bcd2dec(uint8_t bcd);

// DEC to BCD
uint8_t dec2bcd(uint8_t dec);

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
char serialReadTimeout(unsigned long timeout = 100);
uint8_t serialReadHexByteTimeout(unsigned long timeout = 100);

#endif // __SIMULATOR__

int print_number_padStart(int value, int pad, char padChar = ' ', int base = 10);
int pring_number_padEnd(int value, int pad, char padChar = ' ', int base = 10);

#define REPRINT(count, str) for (uint8_t i = 0; i < count; i++) { Serial.print(str); }
#define REPRINTLN(count, str) REPRINT(count, str); Serial.println();


#include "runtime-tools-impl.h"

#endif // __RUNTIME_TOOLS_H__