// VovkPLCRuntime WCET Calibration Firmware
//
// This firmware runs on target MCUs to measure real cycle times for
// WCET calibration. It uses the standard VovkPLC runtime with serial
// transport — no special calibration code is needed on the device side.
//
// The Node.js calibration runner (calibrate.js) will:
//   1. Connect via serial
//   2. Query device info (PI command) to identify arch/clock/features
//   3. Upload calibration bytecode programs (PD command)
//   4. Start execution (PR command)
//   5. Read cycle time from health stats (PH command)
//   6. Repeat for each opcode category
//
// Memory layout for calibration:
//   - Marker memory M0-M63 is reserved for calibration timing data
//   - M0-M3: u32 last cycle time (µs) — written by calibration bytecode
//   - M4-M7: u32 iteration counter
//   - M8+:   additional timing scratch space
//   - The calibration bytecode programs will NOT touch M0-M63
//     (timing is read from PH health command instead)
//
// The firmware is intentionally minimal — it just sets up the runtime
// with a serial transport and lets the remote runner do all the work.

// Runtime configuration — generous memory for calibration workloads
#define PLCRUNTIME_SERIAL_ENABLED
#define PLCRUNTIME_TRANSPORT

#define PLCRUNTIME_NUM_OF_INPUTS   64
#define PLCRUNTIME_NUM_OF_OUTPUTS  64
#define PLCRUNTIME_NUM_OF_MARKERS  256

#define PLCRUNTIME_MAX_STACK_SIZE    128
#define PLCRUNTIME_MAX_MEMORY_SIZE   2048
#define PLCRUNTIME_MAX_PROGRAM_SIZE  4096

// Enable variable registration so symbols are visible in SL command
#define PLCRUNTIME_VARIABLE_REGISTRATION_ENABLED

#include <VovkPLCRuntime.h>

VovkPLCRuntime runtime = VovkPLCRuntime();

// Register calibration marker variables for easy identification
u32& calib_cycle_us    = runtime.registerM<u32>(0,  "calib_cycle_us",   "Last calibration cycle time (us)");
u32& calib_iterations  = runtime.registerM<u32>(4,  "calib_iterations", "Calibration iteration counter");
u32& calib_total_us    = runtime.registerM<u32>(8,  "calib_total_us",   "Total calibration time (us)");
u32& calib_status      = runtime.registerM<u32>(12, "calib_status",     "Calibration status flags");

#ifdef LED_BUILTIN
  #define led_pin LED_BUILTIN
#else
  #define led_pin -1
#endif

// Black pills (STM32F401/F411) have active-low LED on PC13
#if defined(STM32F4xx) || defined(STM32F1xx)
  #define LED_ON  LOW
  #define LED_OFF HIGH
#else
  #define LED_ON  HIGH
  #define LED_OFF LOW
#endif

// PLC cycle callback — called by timer interrupt or main loop
void plc_cycle() {
    runtime.run();
}

void setup() {
    if (led_pin >= 0) pinMode(led_pin, OUTPUT);
    Serial.begin(115200);

    // Wait for serial connection (USB CDC devices need this)
    #if defined(USBCON) || defined(ARDUINO_USB_CDC_ON_BOOT)
    while (!Serial) { delay(10); }
    #endif

    // Set up serial transport with no authentication for calibration
    runtime
        .addSerial(Serial, PLC_SEC_NONE)
        .addUser("admin", "admin", PERM_FULL);

    runtime.initialize();

    // For calibration, we run the PLC cycle directly in loop()
    // (no hardware timer interrupt) so timing is as clean as possible
    // and we can control when cycles execute from the remote runner.
    //
    // The calibration runner will:
    //   - Upload program via PD
    //   - Send PR to start
    //   - The runtime.run() in loop() will execute the program
    //   - Runner reads PH for cycle times
    //   - Runner sends PS to stop when done

    // LED blink to indicate firmware is ready
    if (led_pin >= 0) {
        for (int i = 0; i < 3; i++) {
            digitalWrite(led_pin, LED_ON);
            delay(100);
            digitalWrite(led_pin, LED_OFF);
            delay(100);
        }
    }
}

void loop() {
    // Toggle LED slowly to show we're alive
    static u32 led_timer = 0;
    static bool led_state = false;
    if (led_pin >= 0 && millis() - led_timer > 500) {
        led_timer = millis();
        led_state = !led_state;
        digitalWrite(led_pin, led_state ? LED_ON : LED_OFF);
    }

    // Listen for serial commands from the calibration runner
    runtime.listen();

    // Run the PLC program continuously.
    // After a program is downloaded via PD, runtime.run() will execute it.
    // The runtime measures cycle time internally via micros() and
    // exposes it through the PH (health) command.
    // When no program is loaded (size=0), run() returns immediately.
    plc_cycle();
}
