// Main file for testing the VovkPLCRuntime library





// #define __RUNTIME_DEBUG__  // Avoid using this with microcontrollers with limited RAM!
#ifndef __RUNTIME_PRODUCTION__
#define __RUNTIME_FULL_UNIT_TEST___ // Only use this for testing the library
#endif // __RUNTIME_PRODUCTION__


// #define __RUNTIME_DEBUG__
// #define __RUNTIME_FULL_UNIT_TEST___

#include <VovkPLCRuntime.h>

#define stack_size 64
#define memory_size 64
#define program_size 128

VovkPLCRuntime runtime(stack_size, memory_size, program_size); // Stack size, memory size, program size

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
#ifdef USER_BTN
  pinMode(USER_BTN, INPUT_PULLUP);
#endif // USER_BTN
  Serial.begin(115200);
  while (!Serial && (millis() < 10000)) { // wait for serial port to connect. Needed for native USB port only
    digitalWrite(LED_BUILTIN, LOW);
    delay(1);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
  }

#ifdef __RUNTIME_PRODUCTION__
  runtime.startup();
  // input  = I0.0  (10.0 absolute position)
  // output = Q0.0  (20.0 absolute position)
  // The PLCASM:
  /*
    u8.load 20
    u8.load 10
    u8.0.get

    jump_if_not RESET
    u8.0.set
    jump END

    RESET:
    u8.0.rset

    END:
    u8.store 20
    exit
  */
  // 13 02 00 0A 40 13 02 00 14 15 02 C2 00 12 48 C0 00 13 50 12 02 00 14 FF
  const uint8_t program[] = { 0x13,0x02,0x00,0x0A,0x40,0x13,0x02,0x00,0x14,0x15,0x02,0xC2,0x00,0x12,0x48,0xC0,0x00,0x13,0x50,0x12,0x02,0x00,0x14,0xFF };
  const uint16_t size = sizeof(program);
  runtime.program->loadUnsafe(program, size);
#else // __RUNTIME_PRODUCTION__
  // Start the runtime unit test
  runtime_unit_test();
  runtime.startup();
#endif // __RUNTIME_PRODUCTION__
}

bool startup = true;

long timer_temp = 0;
int cycle = 0;
int last_cycle = 0;
void loop() {
#ifdef __RUNTIME_PRODUCTION__
  long t = millis();
#ifdef USER_BTN
  bool input = digitalRead(USER_BTN);
#else // USER_BTN
  int t_temp = t / 2000;
  bool input = t_temp % 2 == 0;
#endif // USER_BTN
  runtime.setInputBit(0, 0, input);
  runtime.cleanRun();
  bool output = runtime.getOutputBit(0, 0);
  digitalWrite(LED_BUILTIN, output);


  // RuntimeError status = UnitTest::fullProgramDebug(runtime);
  // const char* status_name = RUNTIME_ERROR_NAME(status);
  // Serial.print(F("Runtime status: ")); Serial.println(status_name);

  cycle++;
  if (timer_temp > t) timer_temp = t;
  long diff = t - timer_temp;
  if (diff >= 200) {
    timer_temp = t;
    long mem = freeMemory();
    long cycles = cycle - last_cycle;
    last_cycle = cycle;
    long cps = cycles * 1000 / diff;
    // Serial.printf("Free mem: %6d bytes  CPS: %6d", mem, cps); // AVR doesn't support printf
    Serial.print(F("Free mem: ")); Serial.print(mem); Serial.print(F(" bytes  CPS: ")); Serial.print(cps);
    uint8_t* memory = new uint8_t[32];
    runtime.readMemory(0, memory, 32);
    Serial.print(F("  Memory: "));
    for (int i = 0; i < 32; i++) {
      if (memory[i] < 0x10) Serial.print('0');
      Serial.print(memory[i], HEX);
      Serial.print(' ');
      if (i % 10 == 9) Serial.print(' ');
    }
    delete [] memory;
    Serial.println();
  }

#else // __RUNTIME_PRODUCTION__
  // Blink the LED to indicate that the tests are done
  digitalWrite(LED_BUILTIN, LOW);
  delay(2);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);

  // Custom program test
  if (startup) {
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

#ifdef __RUNTIME_DEBUG__
    RuntimeError status = UnitTest::fullProgramDebug(runtime);
    const char* status_name = RUNTIME_ERROR_NAME(status);
    Serial.print(F("Runtime status: ")); Serial.println(status_name);
#else
    Serial.print(F("Loaded bytecode: ")); runtime.program->println();
    runtime.cleanRun();
#endif
    float output = runtime.read<float>();


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
    bool input = digitalRead(10);
    runtime.setInputBit(0, 0, input);
    runtime.cleanRun();
    bool output = runtime.getOutputBit(0, 0);
    digitalWrite(11, output);
    result = runtime.read<float>();
  }
  t = micros() - t;
  float ms = t / 1000.0;
  int mem = freeMemory();
  Serial.print(F("Program executed for ")); Serial.print(cycles); Serial.print(F(" cycles in ")); Serial.print(ms, 3); Serial.print(F(" ms. Result: ")); Serial.print(result); Serial.print(F("  Free memory: ")); Serial.print(mem); Serial.println(F(" bytes."));
#endif
}