#define __RUNTIME_PRODUCTION__
#define PLCRUNTIME_SERIAL_ENABLED

#include <VovkPLCRuntime.h>
VovkPLCRuntime runtime;

void setup() {
    Serial.begin(115200);
    runtime.initialize();
}

void loop() {
    // ############## Prepare PLC inputs to be processed ##############
    runtime.setInputBit(0, 0, false); // Set I0.0 to false
    // ######## Run one PLC scan (cycle) of the active program ########
    runtime.run();
    // ############ Extract the outputs from the runtime ##############
    bool out = runtime.getOutputBit(0, 0); // Get Q0.0
    // #### Listen for remote commands over serial ##########
    runtime.listen();
}