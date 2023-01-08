#define __SIMULATOR__
#define __RUNTIME_DEBUG__
#define __RUNTIME_FULL_UNIT_TEST___

#include "common.h"

#include "../../VovkPLCRuntime/src/VovkPLCRuntime.h"

void custom_test();

bool startup = true;

WASM_EXPORT void doSomething() {
    if (startup) runtime_unit_test();
    custom_test();
}



RuntimeProgram program(86); // Program size
VovkPLCRuntime runtime(64, program); // Stack size


void custom_test() {
    // Blink the LED to indicate that the tests are done
    digitalWrite(LED_BUILTIN, LOW);
    delay(2);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);

    // Custom program test
    if (startup) {
        runtime.startup();
        Serial.println(F("Custom program test"));
        Serial.println(F("Variables  = { a: 1, b: 2, c: 3, d: 4, e: 5, f: 6 }"));
        Serial.println(F("Function   = 10 * (1 - 'a' * ('b' + 'c' * ('c' + 'd' * ('d'-'e' *('e'-'f')))) / 'd')"));
        Serial.println(F("RPN        = 10|1|'a'|'b'|'c'|'c'|'d'|'d'|'e'|'e'|'f'|-|*|-|*|+|*|+|'d'|/|*|-|*"));
        Serial.println(F("Result     = -287.5"));
        Serial.println();

        float a = 1; float b = 2; float c = 3; float d = 4; float e = 5; float f = 6;
        float expected = 10 * (1 - a * (b + c * (c + d * (d - e * (e - f)))) / d);


        // Hand-coded RPN instructions:
        runtime.program->push_float(10);
        runtime.program->push_float(1);
        runtime.program->push_float(a);
        runtime.program->push_float(b);
        runtime.program->push_float(c);
        runtime.program->push_float(c);
        runtime.program->push_float(d);
        runtime.program->push_float(d);
        runtime.program->push_float(e);
        runtime.program->push_float(e);
        runtime.program->push_float(f);
        runtime.program->push(SUB, type_float);
        runtime.program->push(MUL, type_float);
        runtime.program->push(SUB, type_float);
        runtime.program->push(MUL, type_float);
        runtime.program->push(ADD, type_float);
        runtime.program->push(MUL, type_float);
        runtime.program->push(ADD, type_float);
        runtime.program->push_float(d);
        runtime.program->push(DIV, type_float);
        runtime.program->push(MUL, type_float);
        runtime.program->push(SUB, type_float);
        runtime.program->push(MUL, type_float);


        /*
          // Compiled RPN bytecode:
          static const uint8_t bytecode [] = { 0x0A,0x41,0x20,0x00,0x00,0x0A,0x3F,0x80,0x00,0x00,0x0A,0x3F,0x80,0x00,0x00,0x0A,0x40,0x00,0x00,0x00,0x0A,0x40,0x40,0x00,0x00,0x0A,0x40,0x40,0x00,0x00,0x0A,0x40,0x80,0x00,0x00,0x0A,0x40,0x80,0x00,0x00,0x0A,0x40,0xA0,0x00,0x00,0x0A,0x40,0xA0,0x00,0x00,0x0A,0x40,0xC0,0x00,0x00,0x21,0x0A,0x22,0x0A,0x21,0x0A,0x22,0x0A,0x20,0x0A,0x22,0x0A,0x20,0x0A,0x0A,0x40,0x80,0x00,0x00,0x23,0x0A,0x22,0x0A,0x21,0x0A,0x22,0x0A };
          static const uint16_t size = 82;
          static const uint32_t checksum = 2677;
          program.load(bytecode, size, checksum);
          // program.loadUnsafe(bytecode, size);
        */

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

    const int cycles = 10000;
    float result = -1;
    long t = micros();
    for (int i = 0; i < cycles; i++) {
        runtime.cleanRun();
        result = runtime.read<float>();
    }
    t = micros() - t;
    float ms = t / 1000.0;
    int mem = freeMemory();
    Serial.print(F("Program executed for ")); Serial.print(cycles); Serial.print(F(" cycles in ")); Serial.print(ms, 3); Serial.print(F(" ms. Result: ")); Serial.print(result); Serial.print(F("  Free memory: ")); Serial.print(mem); Serial.println(F(" bytes."));
}
