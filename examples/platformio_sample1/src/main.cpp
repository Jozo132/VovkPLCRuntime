// STM32F401RC BlackPill development board configuration for VovkPLCRuntime library
// 256kb flash, 96kb RAM, 84MHz CPU
// 128kb of flash is used for dynamic program storage

#define PLCRUNTIME_SERIAL_ENABLED

#define PLCRUNTIME_NUM_OF_INPUTS 64
#define PLCRUNTIME_NUM_OF_OUTPUTS 64
#define PLCRUNTIME_NUM_OF_SYSTEMS 256
#define PLCRUNTIME_NUM_OF_MARKERS 256

#define PLCRUNTIME_MAX_STACK_SIZE   64
#define PLCRUNTIME_MAX_MEMORY_SIZE  1024
#define PLCRUNTIME_MAX_PROGRAM_SIZE 1024*32

#define PLCRUNTIME_EEPROM_STORAGE
#define PLCRUNTIME_STM32_FLASH_ADDRESS 0x08020000  // Sector 5 start address (F401RC)
#define PLCRUNTIME_STM32_FLASH_MAX_SIZE 0x00020000  // 128KB (Sector 5 size)
#define PLCRUNTIME_STM32_FLASH_SECTOR 5  // Last sector on F401RC
#define PLCRUNTIME_STM32_FLASH_SECTOR_COUNT 1  // Single sector
#define PLCRUNTIME_EEPROM_SIZE PLCRUNTIME_MAX_PROGRAM_SIZE+16 // Ensure enough space for program + size + checksum
#define RUNTIME_THREAD_IMPL

#define PLCRUNTIME_VARIABLE_REGISTRATION_ENABLED

#include <VovkPLCRuntime.h>
VovkPLCRuntime runtime = VovkPLCRuntime();

bool& user_button    = runtime.registerIBit  (0, 0, "user_button", "BTN on the development board");     // References directly to I0.0 and reads the boolean value at the start of each PLC cycle
bool& builtin_led    = runtime.registerQBit  (0, 0, "builtin_led", "LED on the development board");     // At the end of each PLC cycle, the runtime updates this boolean to reflect the state of Q0.0
int& plc_adc_input_1 = runtime.registerI<int>(2,    "adc1", "PA1");     // References directly to IW2.0 (2 bytes) and reads the integer value at the start of each PLC cycle
int& plc_motor_pwm   = runtime.registerQ<int>(2,   "motor1_pwm", "PA2");   // References directly to MW10.0 (2 bytes) and reads and writes the integer value during each PLC cycle


void plc_cycle();
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(BUTTON_BUILTIN, INPUT_PULLUP);
    Serial.begin(115200);

    runtime.initialize();
    runtime.threadSetup(1000, plc_cycle);
}

void loop() {
    // Read and write IOs in the main loop and let the PLC cycle run in the thread
    user_button = !digitalRead(BUTTON_BUILTIN);
    digitalWrite(LED_BUILTIN, !builtin_led);
    plc_adc_input_1 = analogRead(PA1);

    runtime.listen(); // Listen for remote commands
}

void plc_cycle() {
    runtime.run();
}