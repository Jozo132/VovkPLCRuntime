// VovkPLC.cpp - 2022-12-11
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

#define __WASM_TIME__
#define __SIMULATOR__
#define __RUNTIME_DEBUG__
#define __RUNTIME_FULL_UNIT_TEST___
#define USE_X64_OPS

#define VOVKPLC_DEVICE_NAME "Simulator"

#include "../src/VovkPLCRuntime.h"

#ifndef WASM_EXPORT
#define WASM_EXPORT
#endif // WASM_EXPORT

bool startup = true;

void custom_test() {
    // Blink the LED to indicate that the tests are done
    digitalWrite(LED_BUILTIN, LOW);
    delay(2);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);

    // Custom program test
    if (startup) {
        runtime.initialize();
        Serial.println(F("Custom program test"));
        Serial.println(F("Variables  = { a: 1, b: 2, c: 3, d: 4, e: 5, f: 6 }"));
        Serial.println(F("Function   = 10 * (1 - 'a' * ('b' + 'c' * ('c' + 'd' * ('d'-'e' *('e'-'f')))) / 'd')"));
        Serial.println(F("RPN        = 10|1|'a'|'b'|'c'|'c'|'d'|'d'|'e'|'e'|'f'|-|*|-|*|+|*|+|'d'|/|*|-|*"));
        Serial.println(F("Result     = -287.5"));
        Serial.println();

        float a = 1; float b = 2; float c = 3; float d = 4; float e = 5; float f = 6;
        float expected = 10 * (1 - a * (b + c * (c + d * (d - e * (e - f)))) / d);


        // Hand-coded RPN instructions:
        runtime.program.format();
        runtime.program.push_f32(10);
        runtime.program.push_f32(1);
        runtime.program.push_f32(a);
        runtime.program.push_f32(b);
        runtime.program.push_f32(c);
        runtime.program.push_f32(c);
        runtime.program.push_f32(d);
        runtime.program.push_f32(d);
        runtime.program.push_f32(e);
        runtime.program.push_f32(e);
        runtime.program.push_f32(f);
        runtime.program.push(SUB, type_f32);
        runtime.program.push(MUL, type_f32);
        runtime.program.push(SUB, type_f32);
        runtime.program.push(MUL, type_f32);
        runtime.program.push(ADD, type_f32);
        runtime.program.push(MUL, type_f32);
        runtime.program.push(ADD, type_f32);
        runtime.program.push_f32(d);
        runtime.program.push(DIV, type_f32);
        runtime.program.push(MUL, type_f32);
        runtime.program.push(SUB, type_f32);
        runtime.program.push(MUL, type_f32);


        /*
          // Compiled RPN bytecode:
          static const u8 bytecode [] = { 0x0A,0x41,0x20,0x00,0x00,0x0A,0x3F,0x80,0x00,0x00,0x0A,0x3F,0x80,0x00,0x00,0x0A,0x40,0x00,0x00,0x00,0x0A,0x40,0x40,0x00,0x00,0x0A,0x40,0x40,0x00,0x00,0x0A,0x40,0x80,0x00,0x00,0x0A,0x40,0x80,0x00,0x00,0x0A,0x40,0xA0,0x00,0x00,0x0A,0x40,0xA0,0x00,0x00,0x0A,0x40,0xC0,0x00,0x00,0x21,0x0A,0x22,0x0A,0x21,0x0A,0x22,0x0A,0x20,0x0A,0x22,0x0A,0x20,0x0A,0x0A,0x40,0x80,0x00,0x00,0x23,0x0A,0x22,0x0A,0x21,0x0A,0x22,0x0A };
          static const u16 size = 82;
          static const u32 checksum = 2677;
          runtime.loadProgram(bytecode, size, checksum);
          // runtime.loadProgramUnsafe(bytecode, size);
        */

        runtime.clear();

        RuntimeError status = UnitTest::fullProgramDebug(runtime);

        float output = runtime.read<float>();

        const char* status_name = RUNTIME_ERROR_NAME(status);
        Serial.print(F("Runtime status: ")); Serial.println(status_name);

        Serial.print(F("Result: ")); Serial.println(output);
        Serial.print(F("Expected result: ")); Serial.println(expected);
        Serial.print(F("Test passed: ")); Serial.println(expected == output ? F("YES") : F("NO - TEST DID NOT PASS !!!"));
        Serial.println();
        startup = false;
    }

    const int cycles = 1000;
    float result = -1;
    long t = micros();
    for (int i = 0; i < cycles; i++) {
        runtime.run();
        result = runtime.read<float>();
    }
    t = micros() - t;
    float ms = t / 1000.0;
    int mem = freeMemory();
    Serial.print(F("Program executed for ")); Serial.print(cycles); Serial.print(F(" cycles in ")); Serial.print(ms, 3); Serial.print(F(" ms. Result: ")); Serial.print(result); Serial.print(F("  Free memory: ")); Serial.print(mem); Serial.println(F(" bytes."));
}

WASM_EXPORT int run() {
    IntervalGlobalLoopCheck();
    return runtime.run();
}

WASM_EXPORT int runDirty() {
    IntervalGlobalLoopCheck();
    return runtime.runDirty();
}

WASM_EXPORT int runExplain() {
    IntervalGlobalLoopCheck();
    return UnitTest::fullProgramDebug(runtime);
}

WASM_EXPORT void run_unit_test() {
    IntervalGlobalLoopCheck();
    runtime_unit_test(runtime);
}

WASM_EXPORT void run_custom_test() {
    custom_test();
}

WASM_EXPORT void doNothing() {
    // Do nothing
}

WASM_EXPORT void doSomething() {
    runtime_unit_test(runtime);
    custom_test();
}


WASM_EXPORT u32 getMemoryLocation() {
    return (u32) runtime.memory;
}

WASM_EXPORT u32 getLastCycleTimeUs() {
    return runtime.getLastCycleTimeUs();
}

WASM_EXPORT u32 getMaxCycleTimeUs() {
    return runtime.getMaxCycleTimeUs();
}

WASM_EXPORT u32 getRamFree() {
    return runtime.getRamFree();
}

WASM_EXPORT u32 getMinRamFree() {
    return runtime.getMinRamFree();
}

WASM_EXPORT void resetDeviceHealth() {
    runtime.resetDeviceHealth();
}


// template <typename T> struct OutputBuffer {
//     T* data = nullptr;
//     u32 type_size = sizeof(T);
//     u32 length = 0;
//     u32 max_size = 0;
//     OutputBuffer(u32 size = 0) { init(size); }
//     void init(u32 size = 0) {
//         if (!size) return;
//         if (data == nullptr) {
//             data = new T[size];
//             length = 0;
//             max_size = size;
//         }
//     }
//     void push(T value) {
//         if (length < max_size) {
//             data[length++] = value;
//         }
//     }
//     T pop() {
//         if (length > 0) {
//             return data[--length];
//         }
//         return 0;
//     }
//     T shift() {
//         if (length > 0) {
//             T value = data[0];
//             for (u32 i = 1; i < length; i++) {
//                 data[i - 1] = data[i];
//             }
//             length--;
//             return value;
//         }
//         return 0;
//     }
//     // Array operator getter
//     T& operator[](u32 index) {
//         if (index < length)
//             return data[index];
//         return data[0];
//     }
//     // Array operator setter (avoid overloaded 'operator[]' cannot have more than one parameter before C++2b)
//     void set(u32 index, T value) {
//         if (index < length)
//             data[index] = value;
//     }

// };

// OutputBuffer<unsigned char> myBuffer(10);

// int export_request_count = 0;
// WASM_EXPORT void* export_output() {
//     export_request_count++;
//     if (myBuffer.length > 9)
//         myBuffer.shift();
//     myBuffer.push(export_request_count);
//     //Print buffer contents
//     Serial.print(F("Buffer <"));
//     for (u32 i = 0; i < myBuffer.length; i++) {
//         Serial.print(myBuffer[i], HEX);
//         if (i < myBuffer.length - 1) Serial.print(F(", "));
//     }
//     Serial.println(F(">"));
//     return (void*) (&myBuffer);
// }



// // Export program definition API
// WASM_EXPORT int API_resizeProgram(int new_size) {
//     initRuntime();
//     runtime.program.format(new_size);
//     return 0;
// }
// WASM_EXPORT int API_getProgramSize() {
//     initRuntime();
//     return runtime.program.size();
// }
// WASM_EXPORT int API_getProgramSizeMax() {
//     initRuntime();
//     return runtime.program.MAX_PROGRAM_SIZE;
// }
// WASM_EXPORT void API_printProgram() {
//     initRuntime();
//     runtime.program.println();
// }

// WASM_EXPORT void printErrorMessage() {
//     Error.println("Error: This is a test error message.");
// }
// WASM_EXPORT void printStreamTest() {
//     Stream.print("{ \"test\": \"This is a test stream message.\" }");
// }
