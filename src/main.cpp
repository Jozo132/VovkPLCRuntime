#include <avr/wdt.h>


#define __RUNTIME_TEST__ // Avoid using this with microcontrollers with limited RAM!
#define __RUNTIME_TEST_ADVANCED_DEBUGGING__ // Avoid using this with microcontrollers with limited RAM! This makes the output logs more verbose
#include <VovkPLCRuntime.h>


void setup() {
  wdt_disable();
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  while (!Serial) delay(10); // wait for serial port to connect. Needed for native USB port only

  runtime_test(); // Start the runtime tests
}

int cycle = 0;
void loop() {
  cycle++;
  digitalWrite(LED_BUILTIN, cycle % 2);
  delay(500);
}