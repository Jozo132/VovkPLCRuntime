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

#include "runtime-tools.h"

#ifdef __SIMULATOR__

#ifndef __WASM__
int get_used_memory() { return 0; }
#endif // __WASM__

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
#endif // __SIMULATOR__



int freeMemory() {
#ifdef __WASM__
    return heap_size - heap_used;
#elif defined(ESP8266) || defined(ESP32)
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

void processExit() {
#ifdef __WASM__
    throw 1;
#elif defined(ESP8266) || defined(ESP32)
    ESP.restart();
#elif defined(__arm__)
    NVIC_SystemReset();
#elif defined(__SIMULATOR__)
    exit(0);
#else
    ((void (*)()) 0)(); // Call function null pointer
#endif // __WASM__
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

void byteToHex(uint8_t byte, char& c1, char& c2) {
    c1 = ((byte >> 4) & 0xF) + '0';
    c2 = ((byte) & 0xF) + '0';
    if (c1 > '9') c1 += 'A' - '9' - 1;
    if (c2 > '9') c2 += 'A' - '9' - 1;
}


// BCD to DEC
uint8_t bcd2dec(uint8_t bcd) { return ((bcd >> 4) * 10) + (bcd & 0xF); }

// DEC to BCD
uint8_t dec2bcd(uint8_t dec) { return ((dec / 10) << 4) + (dec % 10); }

#ifdef __SIMULATOR__
void pinMode(int pin, int mode) {}
void digitalWrite(int pin, int value) {}
int digitalRead(int pin) { return 0; }
#ifndef __WASM__
void delay(int ms) {}
void delayMicroseconds(int us) {}
unsigned long millis() { return 0; }
unsigned long micros() { return 0; }
#endif // __WASM__
void attachInterrupt(int pin, void (*callback)(), int mode) {}
void detachInterrupt(int pin) {}
void yield() {}


#if !defined(__WASM__)
int print_direction = STREAM_TO_STDOUT;
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

Serial_t::Serial_t(int STREAM_DIRECTION) {
    stream_direction = STREAM_DIRECTION;
    // Define a function pointer to redirect standard output
    print_func =
#ifdef __WASM__
        stream_direction == STREAM_TO_STREAMOUT ? (void(*)(char)) streamOut :
        stream_direction == STREAM_TO_STDERR ? (void(*)(char)) stderr :
        (void(*)(char)) stdout;
#else // __WASM__
        (void(*)(char)) __print;
#endif // __WASM__
}

void Serial_t::begin(int baudrate) {}
void Serial_t::end() {}
Serial_t::operator bool() { return true; }
int Serial_t::print(const char* str) {
    int size = 0;
    char c = 0;
    while (*str) {
        c = *str++;
        size++;
        print_func(c);
    }
    return size;
}
int Serial_t::print(const char* str, int len) {
    int size = 0;
    char c = 0;
    while (len--) {
        c = *str++;
        size++;
        print_func(c);
    }
    return size;
}
int Serial_t::print(char c) {
    print_func(c);
    return 1;
}
int Serial_t::print(int i, int base) {
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
int Serial_t::print(unsigned int i, int base) {
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
int Serial_t::print(long i, int base) {
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
int Serial_t::print(unsigned long i, int base) {
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
int Serial_t::print(float i, int decimals) {
    int size = 0;
    if (decimals < 0) sprintf(buff, "%f", i);
    else sprintf(buff, "%.*f", decimals, i);
    size = print(buff);
    return size;
}
int Serial_t::print(double i, int decimals) {
    int size = 0;
    if (decimals < 0) sprintf(buff, "%f", i);
    else sprintf(buff, "%.*f", decimals, i);
    size = print(buff);
    return size;
}
int Serial_t::println(const char* str) { int size = print(str); size += print("\n"); return size; }
int Serial_t::println(const char* str, int len) { int size = print(str, len); size += print("\n"); return size; }
int Serial_t::println(char c) { int size = print(c); size += print("\n"); return size; }
int Serial_t::println(int i, int base) { int size = print(i, base); size += print("\n"); return size; }
int Serial_t::println(unsigned int i, int base) { int size = print(i, base); size += print("\n"); return size; }
int Serial_t::println(long i, int base) { int size = print(i, base); size += print("\n"); return size; }
int Serial_t::println(unsigned long i, int base) { int size = print(i, base); size += print("\n"); return size; }
int Serial_t::println(double i, int base) { int size = print(i, base); size += print("\n"); return size; }
int Serial_t::println() { int size = print("\n"); return size; }
int Serial_t::available() {
    return input_len;
}
int Serial_t::read() {
    if (input_len == 0) return -1;
    int c = input[0];
    for (int i = 0; i < input_len - 1; i++) {
        input[i] = input[i + 1];
    }
    input_len--;
    return c;
}
void Serial_t::write(char c) { print_func(c); }
void Serial_t::write(const char* str) { print(str); }
void Serial_t::write(const char* str, int len) { print(str, len); }
void Serial_t::write(int i, int base) { print(i, base); }
void Serial_t::write(unsigned int i, int base) { print(i, base); }
void Serial_t::write(long i, int base) { print(i, base); }
void Serial_t::write(unsigned long i, int base) { print(i, base); }
void Serial_t::write(double i, int base) { print(i, base); }
void Serial_t::flush() {}
void Serial_t::clear() {
    input_len = 0;
    input[0] = 0;
}

#else // __SIMULATOR__

int get_used_memory() {
    return 0;
}

bool serial_timeout = false;
char serialReadTimeout(unsigned long timeout) {
    unsigned long t = millis();
    unsigned long start = t;
    unsigned long elapsed = 0;
    serial_timeout = false;
    while (Serial.available() == 0) {
        t = millis();
        elapsed = t - start;
        if (elapsed >= timeout || start > t) {
            serial_timeout = true;
            Serial.println(F("Serial read timeout"));
            return 0;
        }
        delay(1);
    }
    return Serial.read();
}

uint8_t serialReadHexByteTimeout(unsigned long timeout) {
    uint8_t b = 0;
    char c = serialReadTimeout(timeout);
    if (c >= '0' && c <= '9') b = c - '0';
    else if (c >= 'A' && c <= 'F') b = c - 'A' + 10;
    else if (c >= 'a' && c <= 'f') b = c - 'a' + 10;
    else return 0xff;
    b <<= 4;
    c = serialReadTimeout(timeout);
    if (c >= '0' && c <= '9') b |= c - '0';
    else if (c >= 'A' && c <= 'F') b |= c - 'A' + 10;
    else if (c >= 'a' && c <= 'f') b |= c - 'a' + 10;
    else return 0xff;
    return b;
}

#endif // __SIMULATOR__









int print_number_padStart(int value, int pad, char padChar, int base) {
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
int pring_number_padEnd(int value, int pad, char padChar, int base) {
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