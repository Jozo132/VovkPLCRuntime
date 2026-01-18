#pragma once

#ifdef RUNTIME_THREAD_IMPL

// ============================================================================
// Architecture detection and timer setup
// ============================================================================

typedef void (*thread_handle_t)(void); // Define void thread_handle function type
thread_handle_t _thread_handler = nullptr; // Define thread_handle function pointer

volatile long thread_HAL_time = 0;
volatile long thread_HAL_time_max = 0;
volatile long thread_HAL_time_min = 0;

volatile bool thread_busy = false;
volatile bool thread_enabled = false;

// Common thread loop function (called from timer interrupt)
void thread_loop() {
    if (thread_busy || !thread_enabled) return;
    thread_busy = true;
    long thread_NEW_time = micros();
    if (_thread_handler) _thread_handler();
    thread_HAL_time = micros() - thread_NEW_time;
    if (thread_HAL_time > thread_HAL_time_max) thread_HAL_time_max = thread_HAL_time;
    if (thread_HAL_time < thread_HAL_time_min || thread_HAL_time_min == 0) thread_HAL_time_min = thread_HAL_time;
    thread_busy = false;
}

void thread_onEvent(thread_handle_t handler) {
    _thread_handler = handler;
}

// ============================================================================
// STM32 Implementation (HardwareTimer)
// ============================================================================
#if defined(STM32F0xx) || defined(STM32F1xx) || defined(STM32F2xx) || defined(STM32F3xx) || \
    defined(STM32F4xx) || defined(STM32F7xx) || defined(STM32G0xx) || defined(STM32G4xx) || \
    defined(STM32H7xx) || defined(STM32L0xx) || defined(STM32L1xx) || defined(STM32L4xx) || \
    defined(STM32L5xx) || defined(STM32WBxx) || defined(STM32WLxx) || defined(STM32U5xx)

#define THREAD_ARCH_STM32
static HardwareTimer _thread_Timer(TIM2);

void thread_setup(uint32_t period_us, thread_handle_t handler = nullptr) {
    if (handler) thread_onEvent(handler);
    if (period_us < 20) period_us = 20; // Minimum period is 20us (50kHz)
    thread_enabled = true;
    _thread_Timer.pause();
    _thread_Timer.setOverflow(period_us, MICROSEC_FORMAT);
    _thread_Timer.attachInterrupt(thread_loop);
    _thread_Timer.refresh();
    _thread_Timer.resume();
    _thread_Timer.setInterruptPriority(15, 0); // Lowest priority for TIM2
}

void thread_pause() {
    thread_enabled = false;
    _thread_Timer.pause();
}

void thread_resume() {
    thread_enabled = true;
    _thread_Timer.resume();
}

// ============================================================================
// ESP32 Implementation (esp_timer)
// ============================================================================
#elif defined(ESP32)

#define THREAD_ARCH_ESP32
#include <esp_timer.h>

static esp_timer_handle_t _thread_esp_timer = nullptr;

void IRAM_ATTR _thread_esp_timer_callback(void* arg) {
    thread_loop();
}

void thread_setup(uint32_t period_us, thread_handle_t handler = nullptr) {
    if (handler) thread_onEvent(handler);
    if (period_us < 50) period_us = 50; // Minimum period is 50us for ESP32
    thread_enabled = true;
    
    if (_thread_esp_timer != nullptr) {
        esp_timer_stop(_thread_esp_timer);
        esp_timer_delete(_thread_esp_timer);
    }
    
    esp_timer_create_args_t timer_args = {
        .callback = _thread_esp_timer_callback,
        .arg = nullptr,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "plc_thread"
    };
    esp_timer_create(&timer_args, &_thread_esp_timer);
    esp_timer_start_periodic(_thread_esp_timer, period_us);
}

void thread_pause() {
    thread_enabled = false;
    if (_thread_esp_timer != nullptr) {
        esp_timer_stop(_thread_esp_timer);
    }
}

void thread_resume() {
    thread_enabled = true;
    if (_thread_esp_timer != nullptr) {
        // Need to restart with same period - get period from config or store it
        esp_timer_start_periodic(_thread_esp_timer, 1000); // Default 1ms, user should call thread_setup again
    }
}

// ============================================================================
// ESP8266 Implementation (Ticker)
// ============================================================================
#elif defined(ESP8266)

#define THREAD_ARCH_ESP8266
#include <Ticker.h>

static Ticker _thread_ticker;
static uint32_t _thread_period_us = 1000;

void IRAM_ATTR _thread_ticker_callback() {
    thread_loop();
}

void thread_setup(uint32_t period_us, thread_handle_t handler = nullptr) {
    if (handler) thread_onEvent(handler);
    if (period_us < 1000) period_us = 1000; // Minimum period is 1ms for ESP8266 Ticker
    _thread_period_us = period_us;
    thread_enabled = true;
    _thread_ticker.attach_ms(period_us / 1000, _thread_ticker_callback);
}

void thread_pause() {
    thread_enabled = false;
    _thread_ticker.detach();
}

void thread_resume() {
    thread_enabled = true;
    _thread_ticker.attach_ms(_thread_period_us / 1000, _thread_ticker_callback);
}

// ============================================================================
// RP2040/RP2350 Implementation (alarm)
// ============================================================================
#elif defined(ARDUINO_ARCH_RP2040)

#define THREAD_ARCH_RP2040
#include <hardware/timer.h>
#include <hardware/irq.h>

static uint32_t _thread_period_us = 1000;
static alarm_id_t _thread_alarm_id = -1;

int64_t _thread_alarm_callback(alarm_id_t id, void* user_data) {
    thread_loop();
    return _thread_period_us; // Return period to reschedule
}

void thread_setup(uint32_t period_us, thread_handle_t handler = nullptr) {
    if (handler) thread_onEvent(handler);
    if (period_us < 50) period_us = 50; // Minimum period 50us
    _thread_period_us = period_us;
    thread_enabled = true;
    
    if (_thread_alarm_id >= 0) {
        cancel_alarm(_thread_alarm_id);
    }
    _thread_alarm_id = add_alarm_in_us(_thread_period_us, _thread_alarm_callback, nullptr, true);
}

void thread_pause() {
    thread_enabled = false;
    if (_thread_alarm_id >= 0) {
        cancel_alarm(_thread_alarm_id);
        _thread_alarm_id = -1;
    }
}

void thread_resume() {
    thread_enabled = true;
    if (_thread_alarm_id < 0) {
        _thread_alarm_id = add_alarm_in_us(_thread_period_us, _thread_alarm_callback, nullptr, true);
    }
}

// ============================================================================
// AVR Implementation (Timer1)
// ============================================================================
#elif defined(__AVR__)

#define THREAD_ARCH_AVR
#include <avr/interrupt.h>

static uint32_t _thread_period_us = 1000;

// Timer1 Compare Match A interrupt
ISR(TIMER1_COMPA_vect) {
    thread_loop();
}

void thread_setup(uint32_t period_us, thread_handle_t handler = nullptr) {
    if (handler) thread_onEvent(handler);
    if (period_us < 100) period_us = 100; // Minimum period 100us for AVR
    _thread_period_us = period_us;
    thread_enabled = true;
    
    cli(); // Disable interrupts
    
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    
    // Calculate OCR1A value for desired period
    // Using prescaler 8: timer_freq = F_CPU / 8
    // OCR1A = (period_us * (F_CPU / 1000000)) / 8 - 1
    uint32_t ocr_value = (period_us * (F_CPU / 1000000UL)) / 8 - 1;
    if (ocr_value > 65535) ocr_value = 65535;
    OCR1A = (uint16_t)ocr_value;
    
    TCCR1B |= (1 << WGM12); // CTC mode
    TCCR1B |= (1 << CS11);  // Prescaler 8
    TIMSK1 |= (1 << OCIE1A); // Enable Timer1 compare interrupt
    
    sei(); // Enable interrupts
}

void thread_pause() {
    thread_enabled = false;
    TIMSK1 &= ~(1 << OCIE1A); // Disable Timer1 compare interrupt
}

void thread_resume() {
    thread_enabled = true;
    TIMSK1 |= (1 << OCIE1A); // Enable Timer1 compare interrupt
}

// ============================================================================
// SAMD21/SAMD51 Implementation (TC timer)
// ============================================================================
#elif defined(ARDUINO_ARCH_SAMD)

#define THREAD_ARCH_SAMD
static uint32_t _thread_period_us = 1000;

#if defined(__SAMD51__)
// SAMD51 uses TC0
void TC0_Handler() {
    if (TC0->COUNT16.INTFLAG.bit.MC0) {
        TC0->COUNT16.INTFLAG.reg = TC_INTFLAG_MC0;
        thread_loop();
    }
}

void thread_setup(uint32_t period_us, thread_handle_t handler = nullptr) {
    if (handler) thread_onEvent(handler);
    if (period_us < 50) period_us = 50;
    _thread_period_us = period_us;
    thread_enabled = true;
    
    // Enable TC0 clock
    GCLK->PCHCTRL[TC0_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK0 | GCLK_PCHCTRL_CHEN;
    while (!GCLK->PCHCTRL[TC0_GCLK_ID].bit.CHEN);
    
    TC0->COUNT16.CTRLA.bit.ENABLE = 0;
    while (TC0->COUNT16.SYNCBUSY.bit.ENABLE);
    
    TC0->COUNT16.CTRLA.reg = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_PRESCALER_DIV16;
    TC0->COUNT16.WAVE.reg = TC_WAVE_WAVEGEN_MFRQ;
    
    // Calculate CC value: (period_us * (48MHz / 16)) / 1000000
    uint32_t cc_value = (period_us * 3) - 1;
    if (cc_value > 65535) cc_value = 65535;
    TC0->COUNT16.CC[0].reg = (uint16_t)cc_value;
    while (TC0->COUNT16.SYNCBUSY.bit.CC0);
    
    TC0->COUNT16.INTENSET.reg = TC_INTENSET_MC0;
    NVIC_EnableIRQ(TC0_IRQn);
    NVIC_SetPriority(TC0_IRQn, 3);
    
    TC0->COUNT16.CTRLA.bit.ENABLE = 1;
    while (TC0->COUNT16.SYNCBUSY.bit.ENABLE);
}

void thread_pause() {
    thread_enabled = false;
    TC0->COUNT16.CTRLA.bit.ENABLE = 0;
    while (TC0->COUNT16.SYNCBUSY.bit.ENABLE);
}

void thread_resume() {
    thread_enabled = true;
    TC0->COUNT16.CTRLA.bit.ENABLE = 1;
    while (TC0->COUNT16.SYNCBUSY.bit.ENABLE);
}

#else
// SAMD21 uses TC3
void TC3_Handler() {
    if (TC3->COUNT16.INTFLAG.bit.MC0) {
        TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_MC0;
        thread_loop();
    }
}

void thread_setup(uint32_t period_us, thread_handle_t handler = nullptr) {
    if (handler) thread_onEvent(handler);
    if (period_us < 50) period_us = 50;
    _thread_period_us = period_us;
    thread_enabled = true;
    
    // Enable TC3 clock
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCM_TCC2_TC3) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN;
    while (GCLK->STATUS.bit.SYNCBUSY);
    
    TC3->COUNT16.CTRLA.bit.ENABLE = 0;
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
    
    TC3->COUNT16.CTRLA.reg = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV16;
    
    // Calculate CC value: (period_us * (48MHz / 16)) / 1000000
    uint32_t cc_value = (period_us * 3) - 1;
    if (cc_value > 65535) cc_value = 65535;
    TC3->COUNT16.CC[0].reg = (uint16_t)cc_value;
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
    
    TC3->COUNT16.INTENSET.reg = TC_INTENSET_MC0;
    NVIC_EnableIRQ(TC3_IRQn);
    NVIC_SetPriority(TC3_IRQn, 3);
    
    TC3->COUNT16.CTRLA.bit.ENABLE = 1;
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
}

void thread_pause() {
    thread_enabled = false;
    TC3->COUNT16.CTRLA.bit.ENABLE = 0;
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
}

void thread_resume() {
    thread_enabled = true;
    TC3->COUNT16.CTRLA.bit.ENABLE = 1;
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
}
#endif // __SAMD51__

// ============================================================================
// nRF52 Implementation (Timer)
// ============================================================================
#elif defined(ARDUINO_NRF52_ADAFRUIT) || defined(NRF52) || defined(NRF52840_XXAA)

#define THREAD_ARCH_NRF52
#include <nrf_timer.h>

static uint32_t _thread_period_us = 1000;

extern "C" void TIMER2_IRQHandler(void) {
    if (NRF_TIMER2->EVENTS_COMPARE[0]) {
        NRF_TIMER2->EVENTS_COMPARE[0] = 0;
        thread_loop();
    }
}

void thread_setup(uint32_t period_us, thread_handle_t handler = nullptr) {
    if (handler) thread_onEvent(handler);
    if (period_us < 50) period_us = 50;
    _thread_period_us = period_us;
    thread_enabled = true;
    
    NRF_TIMER2->TASKS_STOP = 1;
    NRF_TIMER2->MODE = TIMER_MODE_MODE_Timer;
    NRF_TIMER2->BITMODE = TIMER_BITMODE_BITMODE_32Bit;
    NRF_TIMER2->PRESCALER = 4; // 16MHz / 16 = 1MHz (1us resolution)
    NRF_TIMER2->CC[0] = period_us;
    NRF_TIMER2->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Msk;
    NRF_TIMER2->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
    
    NVIC_SetPriority(TIMER2_IRQn, 7); // Low priority
    NVIC_EnableIRQ(TIMER2_IRQn);
    
    NRF_TIMER2->TASKS_START = 1;
}

void thread_pause() {
    thread_enabled = false;
    NRF_TIMER2->TASKS_STOP = 1;
}

void thread_resume() {
    thread_enabled = true;
    NRF_TIMER2->TASKS_START = 1;
}

// ============================================================================
// Renesas RA (Arduino UNO R4) Implementation
// ============================================================================
#elif defined(ARDUINO_ARCH_RENESAS) || defined(ARDUINO_ARCH_RENESAS_UNO)

#define THREAD_ARCH_RENESAS
#include "FspTimer.h"

static FspTimer _thread_fsp_timer;
static uint32_t _thread_period_us = 1000;

void _thread_fsp_timer_callback(timer_callback_args_t* p_args) {
    (void)p_args;
    thread_loop();
}

void thread_setup(uint32_t period_us, thread_handle_t handler = nullptr) {
    if (handler) thread_onEvent(handler);
    if (period_us < 100) period_us = 100;
    _thread_period_us = period_us;
    thread_enabled = true;
    
    uint8_t timer_type = GPT_TIMER;
    int8_t timer_ch = FspTimer::get_available_timer(timer_type);
    if (timer_ch < 0) {
        timer_type = AGT_TIMER;
        timer_ch = FspTimer::get_available_timer(timer_type);
    }
    
    if (timer_ch >= 0) {
        _thread_fsp_timer.begin(TIMER_MODE_PERIODIC, timer_type, timer_ch, 
                                1000000.0f / period_us, 0.0f, _thread_fsp_timer_callback);
        _thread_fsp_timer.setup_overflow_irq();
        _thread_fsp_timer.open();
        _thread_fsp_timer.start();
    }
}

void thread_pause() {
    thread_enabled = false;
    _thread_fsp_timer.stop();
}

void thread_resume() {
    thread_enabled = true;
    _thread_fsp_timer.start();
}

// ============================================================================
// Fallback: Software timer using millis() - Not recommended for precise timing
// ============================================================================
#else

#define THREAD_ARCH_FALLBACK
#warning "No hardware timer support for this architecture. Using software polling fallback."

static uint32_t _thread_period_us = 1000;
static uint32_t _thread_last_run = 0;

// For fallback, user must call thread_poll() in their main loop
void thread_poll() {
    if (!thread_enabled) return;
    uint32_t now = micros();
    if (now - _thread_last_run >= _thread_period_us) {
        _thread_last_run = now;
        thread_loop();
    }
}

void thread_setup(uint32_t period_us, thread_handle_t handler = nullptr) {
    if (handler) thread_onEvent(handler);
    _thread_period_us = period_us;
    _thread_last_run = micros();
    thread_enabled = true;
}

void thread_pause() {
    thread_enabled = false;
}

void thread_resume() {
    thread_enabled = true;
    _thread_last_run = micros();
}

#endif // Architecture selection

#endif // RUNTIME_THREAD_IMPL