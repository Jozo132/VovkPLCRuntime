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

#include <Arduino.h>

// Define an empty F() macro if it isn't defined, so that the code can be compiled on non-Arduino platforms
#ifndef F 
#define F(x) x
#endif

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#elif defined(ESP8266) 
// ESP8266 has no sbrk
#else  // __ARM__
extern char* __brkval;
#endif  // __arm__

int freeMemory() {
#if defined(ESP8266)
    return ESP.getFreeHeap();
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

#define REPRINT(count, str) for (uint8_t i = 0; i < count; i++) { Serial.print(str); }
#define REPRINTLN(count, str) REPRINT(count, str); Serial.println();

#define SIZE_OF_ARRAY(a) (sizeof(a) / sizeof(a[0]))

#define STRINGIFY(X) #X

#define CVT_8(type) union uint8_t_to_##type { uint8_t type_uint8_t; type type_##type; }
#define CVT_16(type) union uint16_t_to_##type { uint16_t type_uint16_t; type type_##type; }
#define CVT_32(type) union uint32_t_to_##type { uint32_t type_uint32_t; type type_##type; }
#define CVT_64(type) union uint64_t_to_##type { uint64_t type_uint64_t; type type_##type; }


CVT_32(float);
CVT_64(double);

union u8A_to_u16 { uint8_t u8A[2]; uint16_t u16; };

