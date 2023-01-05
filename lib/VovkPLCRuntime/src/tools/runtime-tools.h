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

#ifdef __SIMULATOR__

#define PROGMEM
#define pgm_read_byte(x) (*(x))
#define pgm_read_word(x) (*(x))
#define pgm_read_dword(x) (*(x))
#define pgm_read_float(x) (*(x))
#define pgm_read_double(x) (*(x))
#define pgm_read_ptr(x) (*(x))
#define __FlashStringHelper char

#define size_t int

#ifndef NULL
#define NULL nullptr
#endif 

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

double ln(double x) {
    double old_sum = 0.0;
    double xmlxpl = (x - 1) / (x + 1);
    double xmlxpl_2 = xmlxpl * xmlxpl;
    double denom = 1.0;
    double frac = xmlxpl;
    double term = frac;                 // denom start from 1.0
    double sum = term;

    while (sum != old_sum) {
        old_sum = sum;
        denom += 2.0;
        frac *= xmlxpl_2;
        sum += frac / denom;
    }
    return 2.0 * sum;
}

double log10(double x) { return ln(x) / LN10; }

#else
#include <Arduino.h>
#endif


#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#elif defined(ESP8266) || defined(__SIMULATOR__)
// ESP8266 has no sbrk
#else  // __ARM__
extern char* __brkval;
#endif  // __arm__

int freeMemory() {
#ifdef __SIMULATOR__
    return heap_size - heap_used;
#elif defined(ESP8266)
    return ESP.getFreeHeap();
#elif defined(__SIMULATOR__)
    return 9000;
#else
    char top;
#ifdef __arm__
    return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
    return &top - __brkval;
#else  // __arm__
    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
#endif  // ESP8266
}

void fstrcpy(char* buff, const char* fstr) {
    uint8_t i = 0;
    uint8_t c = 0;
    for (;;) {
        c = pgm_read_byte(fstr + i);
        buff[i++] = c;
        if (c == 0) return;
        if (i > 254) {
            buff[255] = 0;
            return;
        }
    }
}


// Define an empty F() macro if it isn't defined, so that the code can be compiled on non-Arduino platforms
#ifndef F 
#define F(x) x
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



#ifdef __SIMULATOR__
void pinMode(int pin, int mode) {}
void digitalWrite(int pin, int value) {}
int digitalRead(int pin) { return 0; }
void delay(int ms) {}
void delayMicroseconds(int us) {}
unsigned long millis() { return 0; }
unsigned long micros() { return 0; }
void attachInterrupt(int pin, void (*callback)(), int mode) {}
void detachInterrupt(int pin) {}
void yield() {}


#ifdef __WASM__
WASM_IMPORT void __print(char c);
void  _putchar(char c) { __print(c); }
#else // __WASM__
void __print(char c) { printf("%c", c); }
#endif // __WASM__

char buff[64];

// To upper case
void toUpper(char* buff) {
    for (int i = 0; buff[i]; i++) {
        if (buff[i] >= 'a' && buff[i] <= 'z') {
            buff[i] -= 'a' - 'A';
        }
    }
}

class Serial_t {
    char input[256];
    int input_len = 0;
public:
    operator bool() { return true; }
    void begin(int baudrate) {}
    void end() {}
    int print(const char* str) {
        int size = 0;
        char c = 0;
        while (*str) {
            c = *str++;
            size++;
            __print(c);
        }
        return size;
    }
    int print(const char* str, int len) {
        int size = 0;
        char c = 0;
        while (len--) {
            c = *str++;
            size++;
            __print(c);
        }
        return size;
    }
    int print(char c) {
        __print(c);
        return 1;
    }
    int print(int i, int base = DEC) {
        int size = 0;
        if (base == HEX) {
            sprintf(buff, "%x", i);
            toUpper(buff);
        } else if (base == OCT) sprintf(buff, "%o", i);
        else if (base == BIN) {
            int j = 0;
            for (j = 0; j < 32; j++) {
                buff[j] = (i & 0x80000000) ? '1' : '0';
                i <<= 1;
            }
            buff[j] = 0;
        } else sprintf(buff, "%d", i);
        size = print(buff);
        return size;
    }
    int print(unsigned int i, int base = DEC) {
        int size = 0;
        if (base == HEX) {
            sprintf(buff, "%x", i);
            toUpper(buff);
        } else if (base == OCT) sprintf(buff, "%o", i);
        else if (base == BIN) {
            int j = 0;
            for (j = 0; j < 32; j++) {
                buff[j] = (i & 0x80000000) ? '1' : '0';
                i <<= 1;
            }
            buff[j] = 0;
        } else sprintf(buff, "%u", i);
        size = print(buff);
        return size;
    }
    int print(long i, int base = DEC) {
        int size = 0;
        if (base == HEX) {
            sprintf(buff, "%lx", i);
            toUpper(buff);
        } else if (base == OCT) sprintf(buff, "%lo", i);
        else if (base == BIN) {
            int j = 0;
            for (j = 0; j < 32; j++) {
                buff[j] = (i & 0x80000000) ? '1' : '0';
                i <<= 1;
            }
            buff[j] = 0;
        } else sprintf(buff, "%ld", i);
        size = print(buff);
        return size;
    }
    int print(unsigned long i, int base = DEC) {
        int size = 0;
        if (base == HEX) {
            sprintf(buff, "%lx", i);
            toUpper(buff);
        } else if (base == OCT) sprintf(buff, "%lo", i);
        else if (base == BIN) {
            int j = 0;
            for (j = 0; j < 32; j++) {
                buff[j] = (i & 0x80000000) ? '1' : '0';
                i <<= 1;
            }
            buff[j] = 0;
        } else sprintf(buff, "%lu", i);
        size = print(buff);
        return size;
    }
    int print(float i, int decimals = -1) {
        int size = 0;
        if (decimals < 0) sprintf(buff, "%f", i);
        else sprintf(buff, "%.*f", decimals, i);
        size = print(buff);
        return size;
    }
    int print(double i, int decimals = -1) {
        int size = 0;
        if (decimals < 0) sprintf(buff, "%f", i);
        else sprintf(buff, "%.*f", decimals, i);
        size = print(buff);
        return size;
    }
    int println(const char* str) { int size = print(str); size += print("\n"); return size; }
    int println(const char* str, int len) { int size = print(str, len); size += print("\n"); return size; }
    int println(char c) { int size = print(c); size += print("\n"); return size; }
    int println(int i, int base = DEC) { int size = print(i, base); size += print("\n"); return size; }
    int println(unsigned int i, int base = DEC) { int size = print(i, base); size += print("\n"); return size; }
    int println(long i, int base = DEC) { int size = print(i, base); size += print("\n"); return size; }
    int println(unsigned long i, int base = DEC) { int size = print(i, base); size += print("\n"); return size; }
    int println(double i, int base = DEC) { int size = print(i, base); size += print("\n"); return size; }
    int println() { int size = print("\n"); return size; }
    int available() {
        return input_len;
    }
    int read() {
        if (input_len == 0) return -1;
        int c = input[0];
        for (int i = 0; i < input_len - 1; i++) {
            input[i] = input[i + 1];
        }
        input_len--;
        return c;
    }
    void write(char c) { __print(c); }
    void write(const char* str) { print(str); }
    void write(const char* str, int len) { print(str, len); }
    void write(int i, int base = DEC) { print(i, base); }
    void write(unsigned int i, int base = DEC) { print(i, base); }
    void write(long i, int base = DEC) { print(i, base); }
    void write(unsigned long i, int base = DEC) { print(i, base); }
    void write(double i, int base = DEC) { print(i, base); }
    void flush() {}
    void clear() {
        input_len = 0;
        input[0] = 0;
    }





};

Serial_t Serial;

#endif // __SIMULATOR__









int print_number_padStart(int value, int pad, char padChar = ' ', int base = 10) {
    int strlen = 0;
    int temp = value;
    int num_of_digits = temp == 0 ? 1 : 0;
    if (temp < 0) {
        num_of_digits++;
        temp = -temp;
    }
    while (temp > 0) {
        num_of_digits++;
        temp /= base;
    }
    if (num_of_digits < pad)
        for (int i = 0; i < pad - num_of_digits; i++)
            strlen += Serial.print(padChar);
    strlen += Serial.print(value, base);
    return strlen;
}

int pring_number_padEnd(int value, int pad, char padChar = ' ', int base = 10) {
    int strlen = 0;
    int temp = value;
    int num_of_digits = temp == 0 ? 1 : 0;
    if (temp < 0) {
        num_of_digits++;
        temp = -temp;
    }
    while (temp > 0) {
        num_of_digits++;
        temp /= base;
    }
    strlen += Serial.print(value, base);
    if (num_of_digits < pad)
        for (int i = 0; i < pad - num_of_digits; i++)
            strlen += Serial.print(padChar);
    return strlen;
}

#define REPRINT(count, str) for (uint8_t i = 0; i < count; i++) { Serial.print(str); }
#define REPRINTLN(count, str) REPRINT(count, str); Serial.println();