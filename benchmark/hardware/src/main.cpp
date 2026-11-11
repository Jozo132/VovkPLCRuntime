#define PLCRUNTIME_MAX_STACK_SIZE 32
#define PLCRUNTIME_MAX_MEMORY_SIZE 384
#ifdef __AVR__
#define PLCRUNTIME_MAX_PROGRAM_SIZE 128
#else
#define PLCRUNTIME_MAX_PROGRAM_SIZE 384
#endif
#define PLCRUNTIME_NUM_OF_INPUTS 16
#define PLCRUNTIME_NUM_OF_OUTPUTS 16
#define PLCRUNTIME_NUM_OF_MARKERS 16
#define PLCRUNTIME_NUM_OF_TIMERS 0
#define PLCRUNTIME_NUM_OF_COUNTERS 0
#define PLCRUNTIME_NUM_OF_DATABLOCKS 1
#define PLCRUNTIME_MAX_FFI_FUNCTIONS 1
#define PLCRUNTIME_MAX_COMMS_INSTANCES 1
#define PLCRUNTIME_COMMS_QUEUE_DEPTH 1
#include <tools/runtime-lib.h>
#if defined(VOVK_BENCH_STM32H523_250MHZ)
#include <stm32h5xx_hal.h>
#endif
#if defined(ARDUINO_ARCH_MBED) && defined(RP2040)
#include <hardware/clocks.h>
#if defined(VOVK_BENCH_RP2040_CLOCK_KHZ)
#include <hardware/pll.h>
#include <hardware/vreg.h>
#endif
#endif

VovkPLCRuntime runtime;
char commandType = 0;
u8 commandPhase = 0;
bool commandInvalid = false;
bool haveScanDigit = false;
unsigned long scanCount = 0;
u16 programLength = 0;
int highNibble = -1;
#if defined(ARDUINO_ARCH_MBED) && defined(RP2040) && defined(VOVK_BENCH_RP2040_CLOCK_KHZ)
bool benchmarkClockValid = false;
#endif

u32 benchmarkCycleCount() {
#if defined(VOVK_BENCH_STM32H523_250MHZ)
    return DWT->CYCCNT;
#else
    return 0;
#endif
}

#ifdef PLCRUNTIME_FFI_ENABLED
u8 benchmarkFFI() { return 42; }
#endif

void printInfo() {
#if defined(ARDUINO_ARCH_MBED) && defined(RP2040) && defined(VOVK_BENCH_RP2040_CLOCK_KHZ)
    if (!benchmarkClockValid) {
        Serial.println(F("@ERROR,clock"));
        return;
    }
#endif
    Serial.print(F("@INFO,2,"));
#if defined(ARDUINO_ARCH_MBED) && defined(RP2040)
    Serial.print((unsigned long)clock_get_hz(clk_sys));
#elif defined(AIR105)
    Serial.print((unsigned long)SystemCoreClock);
#elif defined(VOVK_BENCH_STM32H523_250MHZ)
    Serial.print((unsigned long)SystemCoreClock);
#else
    Serial.print((unsigned long)F_CPU);
#endif
    Serial.print(',');
    Serial.print(plcruntime_get_feature_flags());
    Serial.print(',');
#ifdef PLCRUNTIME_USE_COMPUTED_GOTO
    Serial.print(F("goto,"));
#else
    Serial.print(F("switch,"));
#endif
    const char* digits = "0123456789ABCDEF";
    for (unsigned base = 0; base < 256; base += 4) {
        unsigned mask = 0;
        for (unsigned bit = 0; bit < 4; ++bit) {
            unsigned opcode = base + bit;
            bool supported = OPCODE_EXISTS((PLCRuntimeInstructionSet)opcode);
            if (opcode >= type_char && opcode <= type_str16) supported = false;
#ifdef PLCRUNTIME_FFI_ENABLED
            if (opcode == FFI_CALL || opcode == FFI_CALL_STACK) supported = true;
#endif
            if (supported) mask |= 1U << bit;
        }
        Serial.print(digits[mask]);
    }
    Serial.print(',');
    Serial.print(PLCRUNTIME_MAX_PROGRAM_SIZE);
    Serial.print(',');
    Serial.print(PLCRUNTIME_MAX_MEMORY_SIZE);
    Serial.print(',');
    Serial.println(PLCRUNTIME_MAX_STACK_SIZE);
}

int hexDigit(char digit) {
    if (digit >= '0' && digit <= '9') return digit - '0';
    if (digit >= 'a' && digit <= 'f') return digit - 'a' + 10;
    if (digit >= 'A' && digit <= 'F') return digit - 'A' + 10;
    return -1;
}

void runBenchmark() {
    runtime.stack.format();
    runtime.formatMemory();
    RuntimeError status = runtime.program.loadUnsafe(runtime.program.program, programLength);
    if (status != STATUS_SUCCESS) { Serial.println(F("@ERROR,load")); return; }
    u32 start = micros();
    unsigned long completed = 0;
    for (; completed < scanCount; ++completed) {
        status = runtime.run();
        if (status != STATUS_SUCCESS) break;
    }
    u32 elapsed = (u32)micros() - start;
    Serial.print(F("@RESULT,"));
    Serial.print(completed);
    Serial.print(',');
    Serial.print((unsigned long)elapsed);
    Serial.print(',');
    Serial.print((unsigned)status);
    Serial.print(',');
    Serial.print(runtime.stack.size());
    Serial.print(',');
    Serial.println((unsigned long)runtime.last_instruction_count);
}

#if defined(VOVK_BENCH_STM32H523_250MHZ)
void configureBenchmarkClock() {
    RCC_OscInitTypeDef oscillator = {};
    RCC_ClkInitTypeDef clocks = {};
    RCC_PeriphCLKInitTypeDef peripheralClocks = {};

    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    clocks.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1
        | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK3;
    clocks.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clocks.APB1CLKDivider = RCC_HCLK_DIV1;
    clocks.APB2CLKDivider = RCC_HCLK_DIV1;
    clocks.APB3CLKDivider = RCC_HCLK_DIV1;

#if defined(VOVK_BENCH_STM32H523_HSE)
    oscillator.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI48;
    oscillator.HSEState = RCC_HSE_ON;
#else
    oscillator.OscillatorType = RCC_OSCILLATORTYPE_CSI | RCC_OSCILLATORTYPE_HSI48;
    oscillator.CSIState = RCC_CSI_ON;
    oscillator.CSICalibrationValue = RCC_CSICALIBRATION_DEFAULT;
#endif
    oscillator.HSI48State = RCC_HSI48_ON;
    oscillator.PLL.PLLState = RCC_PLL_ON;
#if defined(VOVK_BENCH_STM32H523_HSE)
    oscillator.PLL.PLLSource = RCC_PLL1_SOURCE_HSE;
    oscillator.PLL.PLLM = 4;
#else
    oscillator.PLL.PLLSource = RCC_PLL1_SOURCE_CSI;
    oscillator.PLL.PLLM = 2;
#endif
    oscillator.PLL.PLLN = 250;
    oscillator.PLL.PLLP = 2;
    oscillator.PLL.PLLQ = 10;
    oscillator.PLL.PLLR = 2;
    oscillator.PLL.PLLRGE = RCC_PLL1_VCIRANGE_1;
    oscillator.PLL.PLLVCOSEL = RCC_PLL1_VCORANGE_WIDE;
    oscillator.PLL.PLLFRACN = 0;
    if (HAL_RCC_OscConfig(&oscillator) != HAL_OK) Error_Handler();
    clocks.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    if (HAL_RCC_ClockConfig(&clocks, FLASH_LATENCY_5) != HAL_OK) Error_Handler();

    peripheralClocks.PeriphClockSelection = RCC_PERIPHCLK_USB;
    peripheralClocks.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
    if (HAL_RCCEx_PeriphCLKConfig(&peripheralClocks) != HAL_OK) Error_Handler();

#if defined(VOVK_BENCH_STM32H523_250MHZ)
    __HAL_FLASH_SET_PROGRAM_DELAY(FLASH_PROGRAMMING_DELAY_2);
#endif
}
#endif

void runTimingTest() {
    const u32 delayStart = micros();
    const u32 delayCyclesStart = benchmarkCycleCount();
    for (u8 iteration = 0; iteration < 10; ++iteration) delay(100);
    const u32 delayCycles = benchmarkCycleCount() - delayCyclesStart;
    const u32 delayElapsed = (u32)micros() - delayStart;

    const u32 microDelayStart = micros();
    const u32 microDelayCyclesStart = benchmarkCycleCount();
    for (u8 iteration = 0; iteration < 100; ++iteration) delayMicroseconds(10000);
    const u32 microDelayCycles = benchmarkCycleCount() - microDelayCyclesStart;
    const u32 microDelayElapsed = (u32)micros() - microDelayStart;

    Serial.print(F("@TIME,10,100000,"));
    Serial.print((unsigned long)delayElapsed);
    Serial.print(',');
    Serial.print((unsigned long)delayCycles);
    Serial.print(F(",100,10000,"));
    Serial.print((unsigned long)microDelayElapsed);
    Serial.print(',');
    Serial.println((unsigned long)microDelayCycles);
}

void setup() {
#if defined(VOVK_BENCH_STM32H523_ICACHE)
    HAL_ICACHE_Enable();
#endif
#if defined(VOVK_BENCH_STM32H523_250MHZ)
    configureBenchmarkClock();
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
#endif
#if defined(ARDUINO_ARCH_MBED) && defined(RP2040) && defined(VOVK_BENCH_RP2040_CLOCK_KHZ)
#if VOVK_BENCH_RP2040_CLOCK_KHZ > 200000
    vreg_set_voltage(VREG_VOLTAGE_1_30);
    delay(10);
#elif VOVK_BENCH_RP2040_CLOCK_KHZ > 133000
    vreg_set_voltage(VREG_VOLTAGE_1_20);
    delay(10);
#endif
#if VOVK_BENCH_RP2040_CLOCK_KHZ == 133000
    const uint32_t pllFrequency = 1596000000UL;
    const unsigned postDivider1 = 6, postDivider2 = 2;
#elif VOVK_BENCH_RP2040_CLOCK_KHZ == 200000
    const uint32_t pllFrequency = 1200000000UL;
    const unsigned postDivider1 = 3, postDivider2 = 2;
#elif VOVK_BENCH_RP2040_CLOCK_KHZ == 240000
    const uint32_t pllFrequency = 1440000000UL;
    const unsigned postDivider1 = 3, postDivider2 = 2;
#else
#error Unsupported RP2040 benchmark clock
#endif
    const uint32_t referenceClock = clock_get_hz(clk_ref);
    const uint32_t interruptState = __get_PRIMASK();
    __disable_irq();
    bool switched = clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF, 0, referenceClock, referenceClock);
    if (switched) {
        pll_deinit(pll_sys);
        pll_init(pll_sys, 1, pllFrequency, postDivider1, postDivider2);
        switched = clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
            CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, VOVK_BENCH_RP2040_CLOCK_KHZ * 1000UL,
            VOVK_BENCH_RP2040_CLOCK_KHZ * 1000UL);
    }
    if (!interruptState) __enable_irq();
    const uint32_t measuredClock = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
    benchmarkClockValid = switched && measuredClock >= VOVK_BENCH_RP2040_CLOCK_KHZ - 1000
        && measuredClock <= VOVK_BENCH_RP2040_CLOCK_KHZ + 1000;
#endif
    Serial.begin(115200);
    runtime.initialize();
#ifdef PLCRUNTIME_FFI_ENABLED
    runtime.registerFFI("benchmark", "void->u8", "", benchmarkFFI);
#endif
}

void loop() {
    while (Serial.available()) {
        char character = (char)Serial.read();
        if (character == '\n') {
            if (commandType == 'I' && commandPhase == 0 && !commandInvalid) printInfo();
            else if (commandType == 'B' && commandPhase == 2 && !commandInvalid && haveScanDigit && scanCount && programLength && highNibble < 0) runBenchmark();
            else if (commandType == 'T' && commandPhase == 0 && !commandInvalid) runTimingTest();
            else Serial.println(F("@ERROR,command"));
            commandType = 0;
            commandPhase = 0;
            commandInvalid = false;
            haveScanDigit = false;
            scanCount = 0;
            programLength = 0;
            highNibble = -1;
        } else if (character != '\r') {
            if (!commandType) commandType = character;
            else if (commandType != 'B') commandInvalid = true;
            else if (commandPhase == 0) {
                if (character == ',') commandPhase = 1;
                else commandInvalid = true;
            } else if (commandPhase == 1) {
                if (character == ',' && haveScanDigit) commandPhase = 2;
                else if (character >= '0' && character <= '9') {
                    haveScanDigit = true;
                    if (scanCount > 5000) commandInvalid = true;
                    else scanCount = scanCount * 10 + (character - '0');
                    if (scanCount > 50000) commandInvalid = true;
                } else commandInvalid = true;
            } else {
                int digit = hexDigit(character);
                if (digit < 0) commandInvalid = true;
                else if (highNibble < 0) highNibble = digit;
                else {
                    if (programLength < PLCRUNTIME_MAX_PROGRAM_SIZE) runtime.program.program[programLength++] = (u8)((highNibble << 4) | digit);
                    else commandInvalid = true;
                    highNibble = -1;
                }
            }
        }
    }
}