#define __RUNTIME_PRODUCTION__
#define PLCRUNTIME_SERIAL_ENABLED

// ###### IMPORTANT NOTE: ######
// You need to call runtime.run() as fast as you can if you want to reliably run the PLC program.
// If you have a slow loop, you should consider using interrupts to trigger the runtime.run() function.

#include <VovkPLCRuntime.h>
VovkPLCRuntime runtime;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
    runtime.initialize();
}

void loop() {

    /*  ADD YOUR OWN INPUT READING HERE */
    bool input = digitalRead(2);
    runtime.setInputBit(0, 0, input); // Set I0.0

    runtime.run(); /* Run one PLC scan cycle of the active program */

    /* ADD YOUR OWN OUTPUT HANDLING HERE */
    bool out = runtime.getOutputBit(0, 0); // Get Q0.0
    digitalWrite(LED_BUILTIN, out); // Set digital output 3 to the output value

    runtime.listen(); /* Listen for remote commands over serial - used for uploading PLC code */
}