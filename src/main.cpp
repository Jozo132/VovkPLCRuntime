// Main file for testing the VovkPLCRuntime library

#define __RUNTIME_TEST__ // Avoid using this with microcontrollers with limited RAM!
#define __RUNTIME_TEST_ADVANCED_DEBUGGING__ // Avoid using this with microcontrollers with limited RAM! This makes the output logs more verbose

#include <VovkPLCRuntime.h>

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  while (!Serial) delay(10); // wait for serial port to connect. Needed for native USB port only

  // Start the runtime tests
  runtime_test();

  // Blink the LED to indicate that the tests are done
  while (1) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(500);
  }
}

void loop() {
  // Nothing to do here
}