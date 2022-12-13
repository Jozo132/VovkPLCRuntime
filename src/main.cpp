// Main file for testing the VovkPLCRuntime library

#define __RUNTIME_TEST__
#define __RUNTIME_FULL_UNIT_TEST___ // Avoid using this with microcontrollers with limited RAM!

#include <VovkPLCRuntime.h>


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  while (!Serial) delay(10); // wait for serial port to connect. Needed for native USB port only

  // Start the runtime tests
  runtime_test();
}


VovkPLCRuntime runtime(64); // Stack size
RuntimeProgram program(128); // Program size

bool startup = true;

void loop() {
  // Blink the LED to indicate that the tests are done
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  delay(500);

  // Custom program test
  if (startup) {
    Serial.println();
    Serial.println("Custom program test");
    Serial.println("Variables  = { a: 1, b: 2, c: 3, d: 4, e: 5, f: 6 }");
    Serial.println("Function   = 10 * (1 - 'a' * ('b' + 'c' * ('c' + 'd' * ('d'-'e' *('e'-'f')))) / 'd')");
    Serial.println("RPN        = 10|1|'a'|'b'|'c'|'c'|'d'|'d'|'e'|'e'|'f'|-|*|-|*|+|*|+|'d'|/|*|-|*");
    Serial.println("Result     = -287.5");
    Serial.println();

    float a = 1; float b = 2; float c = 3; float d = 4; float e = 5; float f = 6;
    float expected = 10 * (1 - a * (b + c * (c + d * (d - e * (e - f)))) / d);

    /*
    // Hand-coded RPN instructions:
    program.erase();
    program.pushF32(10);
    program.pushF32(1);
    program.pushF32(a);
    program.pushF32(b);
    program.pushF32(c);
    program.pushF32(c);
    program.pushF32(d);
    program.pushF32(d);
    program.pushF32(e);
    program.pushF32(e);
    program.pushF32(f);
    program.push(SUB, F32);
    program.push(MUL, F32);
    program.push(SUB, F32);
    program.push(MUL, F32);
    program.push(ADD, F32);
    program.push(MUL, F32);
    program.push(ADD, F32);
    program.pushF32(d);
    program.push(DIV, F32);
    program.push(MUL, F32);
    program.push(SUB, F32);
    program.push(MUL, F32);
    */

    // Compiled RPN bytecode:
    static const uint8_t bytecode [] = { 0x0A,0x41,0x20,0x00,0x00,0x0A,0x3F,0x80,0x00,0x00,0x0A,0x3F,0x80,0x00,0x00,0x0A,0x40,0x00,0x00,0x00,0x0A,0x40,0x40,0x00,0x00,0x0A,0x40,0x40,0x00,0x00,0x0A,0x40,0x80,0x00,0x00,0x0A,0x40,0x80,0x00,0x00,0x0A,0x40,0xA0,0x00,0x00,0x0A,0x40,0xA0,0x00,0x00,0x0A,0x40,0xC0,0x00,0x00,0x21,0x0A,0x22,0x0A,0x21,0x0A,0x22,0x0A,0x20,0x0A,0x22,0x0A,0x20,0x0A,0x0A,0x40,0x80,0x00,0x00,0x23,0x0A,0x22,0x0A,0x21,0x0A,0x22,0x0A };
    static const uint16_t size = 82;
    static const uint32_t checksum = 2677;
    program.load(bytecode, size, checksum);
    // program.loadUnsafe(bytecode, size);

    RuntimeTest::fullProgramDebug(runtime, program);
    float output = runtime.read<float>();

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
    runtime.cleanRun(program);
    result = runtime.read<float>();
  }
  runtime.cleanRun(program);
  result = runtime.read<float>();
  t = micros() - t;
  float ms = t / 1000.0;
  int mem = freeMemory();
  Serial.print("Program executed for "); Serial.print(cycles); Serial.print(" cycles in "); Serial.print(ms, 3); Serial.print(" ms. Result: "); Serial.print(result); Serial.print("\t\t   Free memory: "); Serial.print(mem); Serial.println(" bytes.");
}