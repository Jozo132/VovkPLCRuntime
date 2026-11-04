#define PLCRUNTIME_SERIAL_ENABLED
#define PLCRUNTIME_WIFI_MANAGER
#define PLCRUNTIME_EEPROM_STORAGE
#define PLCRUNTIME_VARIABLE_REGISTRATION_ENABLED
#define PLCRUNTIME_NUM_OF_INPUTS 64
#define PLCRUNTIME_NUM_OF_OUTPUTS 64
#define PLCRUNTIME_NUM_OF_SYSTEMS 256
#define PLCRUNTIME_NUM_OF_MARKERS 256
#define PLCRUNTIME_MAX_STACK_SIZE 64
#define PLCRUNTIME_MAX_MEMORY_SIZE 1024
#define PLCRUNTIME_MAX_PROGRAM_SIZE (1024 * 32)
#define PLCRUNTIME_EEPROM_SIZE (PLCRUNTIME_MAX_PROGRAM_SIZE + 16)

#include <tools/runtime-lib.h>

constexpr int led_pin = 8;
constexpr int button_pin = 9;
constexpr int adc_pin = 1;

VovkPLCRuntime runtime;
bool& user_button = runtime.registerIBit(0, 0, "user_button", "BOOT button");
bool& builtin_led = runtime.registerQBit(0, 0, "builtin_led", "Built-in LED");
int& plc_adc_input = runtime.registerI<int>(2, "adc1", "GPIO1");

void setup() {
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, HIGH);
    pinMode(button_pin, INPUT_PULLUP);
    Serial.begin(115200);

    runtime.initialize();
}

void loop() {
    user_button = !digitalRead(button_pin);
    plc_adc_input = analogRead(adc_pin);
    runtime.listen();
    runtime.run();
    digitalWrite(led_pin, !builtin_led);
}