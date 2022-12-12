// Main file for testing the VovkPLCRuntime library

#define __RUNTIME_TEST__ // Avoid using this with microcontrollers with limited RAM!
// #define __RUNTIME_TEST_ADVANCED_DEBUGGING__ // Avoid using this with microcontrollers with limited RAM! This makes the output logs more verbose

#include <VovkPLCRuntime.h>

#define __MEDIUM_COMPLEXITY_TEST__

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  while (!Serial) delay(10); // wait for serial port to connect. Needed for native USB port only

  // Start the runtime tests
  runtime_test();
}


#ifdef __MEDIUM_COMPLEXITY_TEST__
VovkPLCRuntime runtime(64); // Stack size
RuntimeProgram program(128); // Program size
#endif

bool startup = true;
void loop() {
  // Blink the LED to indicate that the tests are done
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  delay(500);


#ifdef __MEDIUM_COMPLEXITY_TEST__
  // Custom program test
  if (startup) {
    Serial.println();
    Serial.println("Custom program test");
    // program.pushF32(0.1);
    // program.pushF32(0.2);
    // program.pushF32(0.3);
    // program.pushF32(0.4);
    // program.pushF32(0.5);
    // program.push(ADD, F32);


    Serial.println("Variables  = { a: 1, b: 2, c: 3, d: 4, e: 5, f: 6 }");
    Serial.println("Function   = 10 * (1 - 'a' * ('b' + 'c' * ('c' + 'd' * ('d'-'e' *('e'-'f')))) / 'd')");
    Serial.println("RPN        = 10|1|'a'|'b'|'c'|'c'|'d'|'d'|'e'|'e'|'f'|-|*|-|*|+|*|+|'d'|/|*|-|*");
    Serial.println("Result     = -287.5\n");

    float a = 1; float b = 2; float c = 3; float d = 4; float e = 5; float f = 6;
    float expected = 10 * (1 - a * (b + c * (c + d * (d - e * (e - f)))) / d);

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
    program.println();

    uint16_t program_pointer = 0;
    bool finished = false;
    while (!finished) {
      program_pointer = program.getLine();
      long t = micros();
      RuntimeError status = runtime.step(program);
      t = micros() - t;
      float ms = (float) t * 0.001;
      if (status == RTE_PROGRAM_EXITED) finished = true;
      bool problem = status != RTE_SUCCESS && status != RTE_PROGRAM_EXITED;
      TRY_CATCH(problem, {
              const char* error = getRuntimeErrorName(status);
              Serial.print(F("Error at program pointer "));
              Serial.print(program_pointer);
              Serial.print(F(": "));
              Serial.println(error);
        });
      Serial.print(F("Stack trace @Program ["));
      Serial.print(program_pointer);
      Serial.print(F("]: "));
      runtime.printStack();
      Serial.print(F("   <= "));
      program.printOpcodeAt(program_pointer);
      Serial.print(F("  (executed in "));
      Serial.print(ms, 3);
      Serial.print(F(" ms)"));
      Serial.println();
      if (program.finished()) finished = true;
    }
    long t = micros();
    runtime.cleanRun(program);
    t = micros() - t;
    float ms = (float) t * 0.001;
    Serial.print(F("Leftover ")); runtime.printStack(); Serial.println();
    float output = runtime.read<float>();
    Serial.print(F("Time to execute program: ")); Serial.print(ms, 3); Serial.println(F(" ms"));
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

  Serial.print("Program executed for "); Serial.print(cycles); Serial.print(" cycles in "); Serial.print(ms, 3); Serial.print(" ms. Result: ");
  Serial.println(result);
#endif
}