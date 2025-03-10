// Main file for testing the VovkPLCRuntime library

#undef __RUNTIME_PRODUCTION__
// #define __RUNTIME_DEBUG__
// #define __RUNTIME_FULL_UNIT_TEST___ // Only use this for testing the library

#define PLCRUNTIME_SERIAL_ENABLED


#define stack_size 64
#define memory_size 64
#define program_size 1024

#define PLCRUNTIME_NUM_OF_OUTPUTS 16
#define PLCRUNTIME_INPUT_OFFSET 16
#define PLCRUNTIME_MAX_STACK_SIZE stack_size
#define PLCRUNTIME_MAX_MEMORY_SIZE memory_size
#define PLCRUNTIME_MAX_PROGRAM_SIZE program_size

// #include <VovkPLCRuntime.h> // Practical include when the library is installed
#include "../../../src/VovkPLCRuntime.h" // Only for testing the library within the examples folder
VovkPLCRuntime runtime = VovkPLCRuntime(); // Stack size, memory size, program size



#ifdef __RUNTIME_PRODUCTION__
const bool production = true;
#else // __RUNTIME_PRODUCTION__
const bool production = false;
#endif // __RUNTIME_PRODUCTION__

void load_plc_blinky() {
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
    Serial.println(F("Loading program..."));
    runtime.loadProgramUnsafe(program, size);
    Serial.println(F("Program loaded"));
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
#ifdef BUTTON_BUILTIN
    pinMode(BUTTON_BUILTIN, INPUT_PULLUP);
#endif // BUTTON_BUILTIN
    Serial.begin(115200);
    while (!Serial && (millis() < 10000)) { // wait for serial port to connect. Needed for native USB port only
        digitalWrite(LED_BUILTIN, LOW);
        delay(1);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
    }

    runtime.initialize();

    if (production) {
        load_plc_blinky();
    } else {
        runtime_unit_test(runtime);
    }
}

bool startup = true;

long timer_temp = 0;
int cycle = 0;
int last_cycle = 0;
void loop() {
    cycle++;
    long t = millis();
#ifdef BUTTON_BUILTIN
    bool input = !digitalRead(BUTTON_BUILTIN);
#else // BUTTON_BUILTIN
    int t_temp = t / 2000;
    bool input = t_temp % 2 == 0;
#endif // BUTTON_BUILTIN

    if (production || cycle >= 10) {
        if (!production && cycle == 10) {
            Serial.println(F("Starting PLC Runtime Production demo"));
            load_plc_blinky();
        }

        runtime.listen(); // Listen for remote commands
        runtime.setInputBit(0, 0, input);
        runtime.run();
        bool output = runtime.getOutputBit(0, 0);
        digitalWrite(LED_BUILTIN, !output);


        // RuntimeError status = UnitTest::fullProgramDebug(runtime);
        // const char* status_name = RUNTIME_ERROR_NAME(status);
        // Serial.print(F("Runtime status: ")); Serial.println(status_name);

        if (timer_temp > t) timer_temp = t;
        long diff = t - timer_temp;
        // if (diff >= 200) {
        bool b = 0;
        runtime.getBit(1, 1, b); // Get P_200ms pulse
        if (b) {
            timer_temp = t;
            long mem = freeMemory();
            long cycles = cycle - last_cycle;
            last_cycle = cycle;
            long cps = cycles * 1000 / diff;
            float duration_ms = cps > 0 ? 1000.0 / (float) cps : 0;
            // Serial.printf("Free mem: %6d bytes  CPS: %6d", mem, cps); // AVR doesn't support printf
            Serial.print(F("Free mem: ")); Serial.print(mem); Serial.print(F(" bytes  Cycle time: ")); Serial.print(duration_ms, 2); Serial.print(F(" ms"));

            u8 temp_memory[32];
            runtime.readMemory(0, temp_memory, 32);
            Serial.print(F("  Memory: "));
            for (int i = 0; i < 32; i++) {
                if (temp_memory[i] < 0x10) Serial.print('0');
                Serial.print(temp_memory[i], HEX);
                Serial.print(' ');
                if (i % 10 == 9) Serial.print(' ');
            }
            Serial.println();
        }
    } else {

        if (!production) {
            // Blink the LED to indicate that the tests are done
            digitalWrite(LED_BUILTIN, LOW);
            delay(2);
            digitalWrite(LED_BUILTIN, HIGH);

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
                runtime.program.push_f32(10);
                runtime.program.push_f32(1);
                runtime.program.push_f32(a);
                runtime.program.push_f32(b);
                runtime.program.push_f32(c);
                runtime.program.push_f32(c);
                runtime.program.push_f32(d);
                runtime.program.push_f32(d);
                runtime.program.push_f32(e);
                runtime.program.push_f32(e);
                runtime.program.push_f32(f);
                runtime.program.push(SUB, type_f32);
                runtime.program.push(MUL, type_f32);
                runtime.program.push(SUB, type_f32);
                runtime.program.push(MUL, type_f32);
                runtime.program.push(ADD, type_f32);
                runtime.program.push(MUL, type_f32);
                runtime.program.push(ADD, type_f32);
                runtime.program.push_f32(d);
                runtime.program.push(DIV, type_f32);
                runtime.program.push(MUL, type_f32);
                runtime.program.push(SUB, type_f32);
                runtime.program.push(MUL, type_f32);


                /*
                  // Compiled RPN bytecode:
                  static const u8 bytecode [] = { 0x0A,0x41,0x20,0x00,0x00,0x0A,0x3F,0x80,0x00,0x00,0x0A,0x3F,0x80,0x00,0x00,0x0A,0x40,0x00,0x00,0x00,0x0A,0x40,0x40,0x00,0x00,0x0A,0x40,0x40,0x00,0x00,0x0A,0x40,0x80,0x00,0x00,0x0A,0x40,0x80,0x00,0x00,0x0A,0x40,0xA0,0x00,0x00,0x0A,0x40,0xA0,0x00,0x00,0x0A,0x40,0xC0,0x00,0x00,0x21,0x0A,0x22,0x0A,0x21,0x0A,0x22,0x0A,0x20,0x0A,0x22,0x0A,0x20,0x0A,0x0A,0x40,0x80,0x00,0x00,0x23,0x0A,0x22,0x0A,0x21,0x0A,0x22,0x0A };
                  static const u16 size = 82;
                  static const u32 checksum = 2677;
                  runtime.loadProgram(bytecode, size, checksum);
                  // runtime.loadProgramUnsafe(bytecode, size);
                */

#ifdef __RUNTIME_DEBUG__
                RuntimeError status = UnitTest::fullProgramDebug(runtime);
                const char* status_name = RUNTIME_ERROR_NAME(status);
                Serial.print(F("Runtime status: ")); Serial.println(status_name);
#else
                Serial.print(F("Loaded bytecode: ")); runtime.program.println();
                runtime.run();
#endif
                float output = runtime.read<float>();


                Serial.print(F("Result: ")); Serial.println(output);
                Serial.print(F("Expected result: ")); Serial.println(expected);
                Serial.print(F("Test passed: ")); Serial.println(expected == output ? F("YES") : F("DID NOT PASS !!!"));
                Serial.println();
                startup = false;
            }

            const int cycles = 1000;
            float result = -1;
            long t = micros();
            for (int i = 0; i < cycles; i++) {
                bool input = digitalRead(10);
                // runtime.setInputBit(0.0, input);
                runtime.setInputBit(0, 0, input);
                runtime.run();
                // bool output = runtime.getOutputBit(0.0);
                bool output = runtime.getOutputBit(0, 0);
                digitalWrite(11, output);
                result = runtime.read<float>();
            }
            t = micros() - t;
            float ms = t / 1000.0;
            int mem = freeMemory();
            Serial.print(F("Program executed for ")); Serial.print(cycles); Serial.print(F(" cycles in ")); Serial.print(ms, 3); Serial.print(F(" ms. Result: ")); Serial.print(result); Serial.print(F("  Free memory: ")); Serial.print(mem); Serial.println(F(" bytes."));
        }
    }
}