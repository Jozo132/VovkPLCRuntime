#define __RUNTIME_PRODUCTION__
#define PLCRUNTIME_SERIAL_ENABLED

#define PLCRUNTIME_NUM_OF_OUTPUTS   16
#define PLCRUNTIME_INPUT_OFFSET     16
#define PLCRUNTIME_MAX_STACK_SIZE   64
#define PLCRUNTIME_MAX_MEMORY_SIZE  64
#define PLCRUNTIME_MAX_PROGRAM_SIZE 1024

#include <VovkPLCRuntime.h>
VovkPLCRuntime runtime;

#define BUTTON_PIN 12
#define LED_PIN LED_BUILTIN

void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    Serial.begin(115200);
    runtime.initialize();

    // input  = I0.0  (location 16.0 in the memory map)
    // output = Q0.0  (location 32.0 in the memory map)
    //
    // LADDER diagram:
    /*
     *     P_1s     output
     *   ---| |------(  )---
     *      2.4      Q0.0
    **/
    // PLCASM code:
    /*
     *       u8.readBit     2.4    // Read bit 2.4 which is 1s pulse
     *       jump_if_not    end    // Jump to the label 'end' if the bit is OFF
     *       u8.writeBitInv 32.0   // Invert bit Q0.0 (Outputs start at 0x20 -> 32)
     *   end:                      // Label to jump to
    **/
    // Compiled bytecode:
    // Program[10] [5C 00 02 E2 00 09 78 00 20 FF]
    // Explanation:
    /*
     *  5C 00 02  ->  read u8 bit 4 at address 0x0002 and put it on the stack
     *  E2 00 09  ->  jump to address 0x0009 if the value from the stack is OFF
     *  78 00 20  ->  toggle u8 bit 0 at address 0x0020
     *  FF        ->  end of program, which is also the address 0x0009 to jump to
    **/
    const u8 program [] = {
        0x5C, 0x00, 0x02,
        0xE2, 0x00, 0x09,
        0x78, 0x00, 0x20,
        0xFF,
    };
    const u16 size = sizeof(program);
    // Load the program into the runtime without checksum verification, assuming the program is correct
    runtime.loadProgramUnsafe(program, size);
}

void loop() {
    // ######### 1. Read inputs #########
    bool input = !digitalRead(BUTTON_PIN);
    runtime.setInputBit(0, 0, input);

    // ######### 2. Run the PLC program #########
    runtime.run();

    // ######### 3. Write outputs #########
    bool output = runtime.getOutputBit(0, 0);
    digitalWrite(LED_PIN, output);

    // ######### 4. Listen for remote commands #########
    runtime.listen();
}