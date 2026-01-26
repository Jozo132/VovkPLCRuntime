// runtime-lib.h - 2022-12-11
//
// Copyright (c) 2022 J.Vovk
//
// This file is part of VovkPLCRuntime.
//
// VovkPLCRuntime is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// VovkPLCRuntime is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with VovkPLCRuntime.  If not, see <https://www.gnu.org/licenses/>.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#ifdef __WASM__
#define PLCRUNTIME_MAX_STACK_SIZE 1024
#define PLCRUNTIME_MAX_MEMORY_SIZE 104857
#define PLCRUNTIME_MAX_PROGRAM_SIZE 104857
#endif // __WASM__

#ifndef PLCRUNTIME_NUM_OF_CONTROLS
#define PLCRUNTIME_NUM_OF_CONTROLS 64
#endif // PLCRUNTIME_NUM_OF_CONTROLS

#ifndef PLCRUNTIME_NUM_OF_INPUTS
#define PLCRUNTIME_NUM_OF_INPUTS 64
#endif // PLCRUNTIME_NUM_OF_INPUTS

#ifndef PLCRUNTIME_NUM_OF_OUTPUTS
#define PLCRUNTIME_NUM_OF_OUTPUTS 64
#endif // PLCRUNTIME_NUM_OF_OUTPUTS

#ifndef PLCRUNTIME_NUM_OF_SYSTEMS
#define PLCRUNTIME_NUM_OF_SYSTEMS 256
#endif // PLCRUNTIME_NUM_OF_SYSTEMS

#ifndef PLCRUNTIME_NUM_OF_MARKERS
#define PLCRUNTIME_NUM_OF_MARKERS 256
#endif // PLCRUNTIME_NUM_OF_MARKERS

#ifndef PLCRUNTIME_NUM_OF_TIMERS
#define PLCRUNTIME_NUM_OF_TIMERS 16
#endif // PLCRUNTIME_NUM_OF_TIMERS

#ifndef PLCRUNTIME_NUM_OF_COUNTERS
#define PLCRUNTIME_NUM_OF_COUNTERS 16
#endif // PLCRUNTIME_NUM_OF_COUNTERS

// Timer struct size: ET(4) + StartTime(4) + Flags(1) = 9 bytes
#define PLCRUNTIME_TIMER_STRUCT_SIZE 9

// Counter struct size: CV(4) + Flags(1) = 5 bytes
#define PLCRUNTIME_COUNTER_STRUCT_SIZE 5

#ifndef PLCRUNTIME_CONTROL_OFFSET
#define PLCRUNTIME_CONTROL_OFFSET 0
#endif // PLCRUNTIME_CONTROL_OFFSET

#ifndef PLCRUNTIME_INPUT_OFFSET
#define PLCRUNTIME_INPUT_OFFSET (PLCRUNTIME_CONTROL_OFFSET + PLCRUNTIME_NUM_OF_CONTROLS)
#endif // PLCRUNTIME_INPUT_OFFSET

#ifndef PLCRUNTIME_OUTPUT_OFFSET
#define PLCRUNTIME_OUTPUT_OFFSET (PLCRUNTIME_INPUT_OFFSET + PLCRUNTIME_NUM_OF_INPUTS)
#endif // PLCRUNTIME_OUTPUT_OFFSET

#ifndef PLCRUNTIME_SYSTEM_OFFSET
#define PLCRUNTIME_SYSTEM_OFFSET (PLCRUNTIME_OUTPUT_OFFSET + PLCRUNTIME_NUM_OF_OUTPUTS)
#endif // PLCRUNTIME_SYSTEM_OFFSET

#ifndef PLCRUNTIME_MARKER_OFFSET
#define PLCRUNTIME_MARKER_OFFSET (PLCRUNTIME_SYSTEM_OFFSET + PLCRUNTIME_NUM_OF_SYSTEMS)
#endif // PLCRUNTIME_MARKER_OFFSET

#ifndef PLCRUNTIME_TIMER_OFFSET
#define PLCRUNTIME_TIMER_OFFSET (PLCRUNTIME_MARKER_OFFSET + PLCRUNTIME_NUM_OF_MARKERS)
#endif // PLCRUNTIME_TIMER_OFFSET

#ifndef PLCRUNTIME_COUNTER_OFFSET
#define PLCRUNTIME_COUNTER_OFFSET (PLCRUNTIME_TIMER_OFFSET + (PLCRUNTIME_NUM_OF_TIMERS * PLCRUNTIME_TIMER_STRUCT_SIZE))
#endif // PLCRUNTIME_COUNTER_OFFSET

#ifdef __WASM__
#include "assembly/wasm/wasm.h"
#define STDOUT_PRINT Stream
#else // Normal
#define STDOUT_PRINT Serial
#endif // __WASM__

#include "runtime-version.h"
#include "runtime-tools.h"
#include "stack/runtime-stack.h"
#include "runtime-interval.h"
#include "arithmetics/crc8.h"
#include "stack/stack-struct-impl.h"
#include "runtime-instructions.h"
#include "stack/runtime-stack.h"
#include "arithmetics/runtime-arithmetics.h"
#include "runtime-program.h"
#include "runtime-thread.h"

#define SERIAL_TIMEOUT_RETURN if (serial_timeout) return;
#define SERIAL_TIMEOUT_JOB(task) if (serial_timeout) { task; return; };

struct DeviceHealth {
    u32 last_cycle_time_us;
    u32 min_cycle_time_us;
    u32 max_cycle_time_us;
    u32 last_ram_free;
    u32 min_ram_free;
    u32 max_ram_free;
    u32 total_ram_size;
    u32 last_period_us;
    u32 min_period_us;
    u32 max_period_us;
    u32 last_jitter_us;
    u32 min_jitter_us;
    u32 max_jitter_us;
};

class VovkPLCRuntime {
private:
    bool started_up = false;
    bool is_first_cycle = true;
    void updateRamStats() {
        int free_mem = freeMemory();
        u32 free_u32 = free_mem < 0 ? 0u : (u32) free_mem;
        last_ram_free = free_u32;
        if (free_u32 < min_ram_free) min_ram_free = free_u32;
        if (free_u32 > max_ram_free) max_ram_free = free_u32;
    }
    void updateCycleStats(u32 cycle_time_us) {
        last_cycle_time_us = cycle_time_us;
        if (cycle_time_us > max_cycle_time_us) max_cycle_time_us = cycle_time_us;
        if (cycle_time_us < min_cycle_time_us) min_cycle_time_us = cycle_time_us;
        updateRamStats();
    }
    void printHexU32(u32 value) {
        char c1, c2;
        for (int shift = 24; shift >= 0; shift -= 8) {
            byteToHex((value >> shift) & 0xff, c1, c2);
            Serial.print(c1);
            Serial.print(c2);
        }
    }
public:
    u32 control_offset = PLCRUNTIME_CONTROL_OFFSET; // Control offset in memory
    u32 input_offset = PLCRUNTIME_INPUT_OFFSET; // Input offset in memory
    u32 output_offset = PLCRUNTIME_OUTPUT_OFFSET; // Output offset in memory
    u32 system_offset = PLCRUNTIME_SYSTEM_OFFSET; // System offset in memory
    u32 marker_offset = PLCRUNTIME_MARKER_OFFSET; // Marker offset in memory
    u32 timer_offset = PLCRUNTIME_TIMER_OFFSET; // Timer offset in memory (T0, T1, ...)
    u32 counter_offset = PLCRUNTIME_COUNTER_OFFSET; // Counter offset in memory (C0, C1, ...)

    RuntimeStack stack = RuntimeStack(); // Active memory stack for PLC execution
    u8 memory[PLCRUNTIME_MAX_MEMORY_SIZE]; // PLC memory to manipulate
    RuntimeProgram program = RuntimeProgram(); // Active PLC program
    u32 BR = 0; // Binary RLO branch stack (32 bits for up to 32 levels of parallel branch nesting)
    u32 last_cycle_time_us = 0;
    u32 min_cycle_time_us = 1000000000;
    u32 max_cycle_time_us = 0;
    u32 last_ram_free = 0;
    u32 min_ram_free = 1000000000;
    u32 max_ram_free = 0;
    u32 last_period_us = 0;
    u32 min_period_us = 1000000000;
    u32 max_period_us = 0;
    u32 last_jitter_us = 0;
    u32 min_jitter_us = 1000000000;
    u32 max_jitter_us = 0;
    u32 last_run_timestamp_us = 0;
    u32 previous_period_us = 0;
    u32 last_instruction_count = 0; // Number of instructions executed in last run()

    static void splash() {
        Serial.println();
        REPRINTLN(70, ':');
        Serial.println(F(":: Using VovkPLCRuntime Library - Author J.Vovk <jozo132@gmail.com> ::"));
        REPRINTLN(70, ':');
    }

    void initialize() {
        if (started_up) return;
        started_up = true;
        splash();
        stack.format();
        program.format();
        formatMemory();
#ifdef PLCRUNTIME_EEPROM_STORAGE
        program.loadFromEEPROM();
#endif // PLCRUNTIME_EEPROM_STORAGE
    }


    void printProperties() {
#ifdef __WASM__
        Serial.printf("Controls [%d] at offset %d\n", PLCRUNTIME_NUM_OF_CONTROLS, control_offset);
        Serial.printf("Inputs [%d] at offset %d\n", PLCRUNTIME_NUM_OF_INPUTS, input_offset);
        Serial.printf("Outputs [%d] at offset %d\n", PLCRUNTIME_NUM_OF_OUTPUTS, output_offset);
        Serial.printf("Systems [%d] at offset %d\n", PLCRUNTIME_NUM_OF_SYSTEMS, system_offset);
        Serial.printf("Markers [%d] at offset %d\n", PLCRUNTIME_NUM_OF_MARKERS, marker_offset);
        Serial.printf("Stack: %d (%d)\n", stack.size(), PLCRUNTIME_MAX_STACK_SIZE);
        Serial.printf("Memory: %d\n", PLCRUNTIME_MAX_MEMORY_SIZE);
        Serial.printf("Program: %d (%d)\n", program.prog_size, PLCRUNTIME_MAX_PROGRAM_SIZE);
#endif // __WASM__
    }

    void formatMemory() {
        for (u32 i = 0; i < PLCRUNTIME_MAX_MEMORY_SIZE; i++) memory[i] = 0;
        is_first_cycle = true;
    }

    void resetFirstCycle() {
        is_first_cycle = true;
    }

    VovkPLCRuntime() {}

    void loadProgramUnsafe(const u8* program, u32 prog_size) {
        this->program.loadUnsafe(program, prog_size);
    }

    void loadProgram(const u8* program, u32 prog_size, u8 checksum) {
        this->program.load(program, prog_size, checksum);
    }

    void updateGlobals();

    // Clear the stack
    void clear();
    // Clear the stack and reset the program line
    void clear(RuntimeProgram& program);
    // Execute one PLC instruction at index, returns an error code (0 on success)
    RuntimeError step(u8* program, u32 prog_size, u32& index);
    // Execute the whole PLC program, returns an error code (0 on success)
    RuntimeError run(u8* program, u32 prog_size);
    // Execute one PLC instruction, returns an error code (0 on success)
    RuntimeError step(RuntimeProgram& program);
    // Run/Continue the whole PLC program from where it left off, returns an error code (0 on success)
    RuntimeError run(RuntimeProgram& program);
    // Execute one PLC instruction at index, returns an error code (0 on success)
    RuntimeError step(u32& index) {
        return step(program.program, program.prog_size, index);
    }
    // Execute the whole PLC program, returns an error code (0 on success)
    RuntimeError runDirty() {
        return run(program.program, program.prog_size);
    }
    // Run the whole PLC program from the beginning, returns an error code (0 on success)
    RuntimeError run() {
        clear();
        return run(program.program, program.prog_size);
    }
    // Get all device health statistics in a single call
    void getDeviceHealth(DeviceHealth& health) {
        updateRamStats();
        health.last_cycle_time_us = last_cycle_time_us;
        health.min_cycle_time_us = min_cycle_time_us;
        health.max_cycle_time_us = max_cycle_time_us;
        health.last_ram_free = last_ram_free;
        health.min_ram_free = min_ram_free;
        health.max_ram_free = max_ram_free;
        health.total_ram_size = getTotalRam();
        health.last_period_us = last_period_us;
        health.min_period_us = min_period_us;
        health.max_period_us = max_period_us;
        health.last_jitter_us = last_jitter_us;
        health.min_jitter_us = min_jitter_us;
        health.max_jitter_us = max_jitter_us;
    }
    // Get total SRAM size of the device
    u32 getTotalRam() const {
        int total = getTotalMemory();
        return total < 0 ? 0u : (u32)total;
    }
    // Legacy individual getters for backward compatibility
    u32 getLastCycleTimeUs() const { return last_cycle_time_us; }
    u32 getMinCycleTimeUs() const { return min_cycle_time_us; }
    u32 getMaxCycleTimeUs() const { return max_cycle_time_us; }
    u32 getRamFree() {
        updateRamStats();
        return last_ram_free;
    }
    u32 getMinRamFree() {
        updateRamStats();
        return min_ram_free;
    }
    u32 getMaxRamFree() {
        updateRamStats();
        return max_ram_free;
    }
    u32 getLastPeriodUs() const { return last_period_us; }
    u32 getMinPeriodUs() const { return min_period_us; }
    u32 getMaxPeriodUs() const { return max_period_us; }
    u32 getLastJitterUs() const { return last_jitter_us; }
    u32 getMinJitterUs() const { return min_jitter_us; }
    u32 getMaxJitterUs() const { return max_jitter_us; }
    void resetDeviceHealth() {
        max_cycle_time_us = last_cycle_time_us;
        min_cycle_time_us = last_cycle_time_us;
        min_ram_free = last_ram_free;
        max_ram_free = last_ram_free;
        max_period_us = last_period_us;
        min_period_us = last_period_us;
        max_jitter_us = last_jitter_us;
        min_jitter_us = last_jitter_us;
    }
    // Read a custom type T value from the stack. This will pop the stack by sizeof(T) bytes and return the value.
    template <typename T> T read() {
        if (stack.size() < (u32) sizeof(T)) return 0;
        u8 temp[sizeof(T)];
        for (u32 i = 0; i < (u32) sizeof(T); i++)
            temp[i] = stack.pop();
        T value = *(T*) temp;
        return value;
    }

    // Print the stack buffer to the serial port
    int printStack();

    void setControl(u32 index, byte value) {
        memory[index + control_offset] = value;
    }

    void setControlBit(u32 index, u8 bit, bool value) {
        byte temp = 0;
        bool error = get_u8(memory, index + control_offset, temp);
        if (error) return;
        if (value) temp |= (1 << bit);
        else temp &= ~(1 << bit);
        memory[index + control_offset] = temp;
    }

#ifndef __AVR__
    void setControlBit(float index, bool value) {
        u32 i = (u32) index;
        u32 b = (u32) ((index - ((float) i)) * 10);
        b = b > 7 ? 7 : b;
        setControlBit(i, b, value);
    }
#endif // __AVR__

    byte getControl(u32 index) {
        byte value = 0;
        get_u8(memory, index + control_offset, value);
        return value;
    }

    bool getControlBit(u32 index, u8 bit) {
        byte temp = 0;
        bool error = get_u8(memory, index + control_offset, temp);
        if (error) return false;
        return temp & (1 << bit);
    }

#ifndef __AVR__
    bool getControlBit(float index) {
        u32 i = (u32) index;
        u32 b = (u32) ((index - ((float) i)) * 10);
        b = b > 7 ? 7 : b;
        return getControlBit(i, b);
    }
#endif // __AVR__

    void setInput(u32 index, byte value) {
        // memory.set(index + input_offset, value);
        memory[index + input_offset] = value;
    }

    void setInputBit(u32 index, u8 bit, bool value) {
        byte temp = 0;
        // bool error = memory.get(index + input_offset, temp);
        bool error = get_u8(memory, index + input_offset, temp);
        if (error) return;
        if (value) temp |= (1 << bit);
        else temp &= ~(1 << bit);
        // memory.set(index + input_offset, temp);
        memory[index + input_offset] = temp;
    }

#ifndef __AVR__
    void setInputBit(float index, bool value) {
        u32 i = (u32) index;
        u32 b = (u32) ((index - ((float) i)) * 10);
        b = b > 7 ? 7 : b;
        setInputBit(i, b, value);
    }
#endif // __AVR__

    byte getOutput(u32 index) {
        byte value = 0;
        // memory.get(index + output_offset, value);
        get_u8(memory, index + output_offset, value);
        return value;
    }

    bool getOutputBit(u32 index, u8 bit) {
        byte temp = 0;
        // bool error = memory.get(index + output_offset, temp);
        bool error = get_u8(memory, index + output_offset, temp);
        if (error) return false;
        return temp & (1 << bit);
    }

#ifndef __AVR__
    bool getOutputBit(float index) {
        u32 i = (u32) index;
        u32 b = (u32) ((index - ((float) i)) * 10);
        b = b > 7 ? 7 : b;
        return getOutputBit(i, b);
    }
#endif // __AVR__

    byte getSystem(u32 index) {
        byte value = 0;
        get_u8(memory, index + system_offset, value);
        return value;
    }

    bool getSystemBit(u32 index, u8 bit) {
        byte temp = 0;
        bool error = get_u8(memory, index + system_offset, temp);
        if (error) return false;
        return temp & (1 << bit);
    }

#ifndef __AVR__
    bool getSystemBit(float index) {
        u32 i = (u32) index;
        u32 b = (u32) ((index - ((float) i)) * 10);
        b = b > 7 ? 7 : b;
        return getSystemBit(i, b);
    }
#endif // __AVR__

    void setMarker(u32 index, byte value) {
        memory[index + marker_offset] = value;
    }

    void setMarkerBit(u32 index, u8 bit, bool value) {
        byte temp = 0;
        bool error = get_u8(memory, index + marker_offset, temp);
        if (error) return;
        if (value) temp |= (1 << bit);
        else temp &= ~(1 << bit);
        memory[index + marker_offset] = temp;
    }

#ifndef __AVR__
    void setMarkerBit(float index, bool value) {
        u32 i = (u32) index;
        u32 b = (u32) ((index - ((float) i)) * 10);
        b = b > 7 ? 7 : b;
        setMarkerBit(i, b, value);
    }
#endif // __AVR__

    byte getMarker(u32 index) {
        byte value = 0;
        get_u8(memory, index + marker_offset, value);
        return value;
    }

    bool getMarkerBit(u32 index, u8 bit) {
        byte temp = 0;
        bool error = get_u8(memory, index + marker_offset, temp);
        if (error) return false;
        return temp & (1 << bit);
    }

#ifndef __AVR__
    bool getMarkerBit(float index) {
        u32 i = (u32) index;
        u32 b = (u32) ((index - ((float) i)) * 10);
        b = b > 7 ? 7 : b;
        return getMarkerBit(i, b);
    }
#endif // __AVR__

    void setBit(u32 index, u8 bit, bool value) {
        byte temp = 0;
        // bool error = memory.get(index, temp);
        bool error = get_u8(memory, index, temp);
        if (error) return;
        if (value) temp |= (1 << bit);
        else temp &= ~(1 << bit);
        // memory.set(index, temp);
        memory[index] = temp;
    }

    bool getBit(u32 index, u8 bit, bool& value) {
        byte temp = 0;
        // bool error = memory.get(index, temp);
        bool error = get_u8(memory, index, temp);
        if (error) return true;
        value = temp & (1 << bit);
        return false;
    }

    void setBit(u32 index, bool value) {
        setBit(index, 0, value);
    }



    bool readMemory(u32 offset, u8* value, u32 size = 1) {
        return readArea_u8(memory, offset, value, size);
    }

    bool writeMemory(u32 offset, u8* value, u32 size = 1) {
        return writeArea_u8(memory, offset, value, size);
    }

    #ifdef RUNTIME_THREAD_IMPL
    void threadSetup(u32 period_us, thread_handle_t handle = nullptr) {
        thread_setup(period_us, handle);
    }
    #endif // RUNTIME_THREAD_IMPL


    void printInfo() {
        // Start of the info
        STDOUT_PRINT.print(F("[VovkPLCRuntime,"));
        // Architecture
        STDOUT_PRINT.print(F(VOVKPLC_ARCH)); STDOUT_PRINT.print(F(","));
        // Version
        STDOUT_PRINT.print(VOVKPLCRUNTIME_VERSION_MAJOR); STDOUT_PRINT.print(F(","));
        STDOUT_PRINT.print(VOVKPLCRUNTIME_VERSION_MINOR); STDOUT_PRINT.print(F(","));
        STDOUT_PRINT.print(VOVKPLCRUNTIME_VERSION_PATCH); STDOUT_PRINT.print(F(","));
        // Build
        STDOUT_PRINT.print(VOVKPLCRUNTIME_VERSION_BUILD); STDOUT_PRINT.print(F(","));
        // Compile date
        STDOUT_PRINT.print(__ISO_TIMESTAMP__); STDOUT_PRINT.print(F(","));
        // Memory info
        STDOUT_PRINT.print(PLCRUNTIME_MAX_STACK_SIZE); STDOUT_PRINT.print(F(","));
        STDOUT_PRINT.print(PLCRUNTIME_MAX_MEMORY_SIZE); STDOUT_PRINT.print(F(","));
        STDOUT_PRINT.print(PLCRUNTIME_MAX_PROGRAM_SIZE); STDOUT_PRINT.print(F(","));
        // IO map (Controls, Inputs, Outputs, Systems, Markers, Timers, Counters)
        STDOUT_PRINT.print(control_offset); STDOUT_PRINT.print(F(","));
        STDOUT_PRINT.print(PLCRUNTIME_NUM_OF_CONTROLS); STDOUT_PRINT.print(F(","));
        STDOUT_PRINT.print(input_offset); STDOUT_PRINT.print(F(","));
        STDOUT_PRINT.print(PLCRUNTIME_NUM_OF_INPUTS); STDOUT_PRINT.print(F(","));
        STDOUT_PRINT.print(output_offset); STDOUT_PRINT.print(F(","));
        STDOUT_PRINT.print(PLCRUNTIME_NUM_OF_OUTPUTS); STDOUT_PRINT.print(F(","));
        STDOUT_PRINT.print(system_offset); STDOUT_PRINT.print(F(","));
        STDOUT_PRINT.print(PLCRUNTIME_NUM_OF_SYSTEMS); STDOUT_PRINT.print(F(","));
        STDOUT_PRINT.print(marker_offset); STDOUT_PRINT.print(F(","));
        STDOUT_PRINT.print(PLCRUNTIME_NUM_OF_MARKERS); STDOUT_PRINT.print(F(","));
        STDOUT_PRINT.print(timer_offset); STDOUT_PRINT.print(F(","));
        STDOUT_PRINT.print(PLCRUNTIME_NUM_OF_TIMERS); STDOUT_PRINT.print(F(","));
        STDOUT_PRINT.print(PLCRUNTIME_TIMER_STRUCT_SIZE); STDOUT_PRINT.print(F(","));
        STDOUT_PRINT.print(counter_offset); STDOUT_PRINT.print(F(","));
        STDOUT_PRINT.print(PLCRUNTIME_NUM_OF_COUNTERS); STDOUT_PRINT.print(F(","));
        STDOUT_PRINT.print(PLCRUNTIME_COUNTER_STRUCT_SIZE); STDOUT_PRINT.print(F(","));
        // Device name
        STDOUT_PRINT.print(F(VOVKPLC_DEVICE_NAME));

        // End of the info
        STDOUT_PRINT.println(F("]"));
    }

    bool print_info_first_time = true;
    void listen() {
#ifdef __WASM__
        // Unused, access is available directly through the wasm.h interface
#else // __WASM__
        // Production code for microcontrollers
        // Listen for input from [serial, ethernet, wifi, etc.]
#ifdef PLCRUNTIME_SERIAL_ENABLED

        if (print_info_first_time) {
            printInfo();
            print_info_first_time = false;
        }

        // If the serial port is available and the first character is not 'P' or 'M', skip the character
        while (Serial.available() && Serial.peek() != 'R' && Serial.peek() != 'P' && Serial.peek() != 'M' && Serial.peek() != 'S' && Serial.peek() != '?') Serial.read();
        if (Serial.available() > 1 || Serial.peek() == '?') {
            // Command syntax:
            // <command>[<size>][<data>]<checksum>
            // Where the command is always 2 characters and the rest is %02x encoded
            // Possible commands:
            //  - PLC reset:        'RS<u8>' (checksum)
            //  - PLC health:       'PH<u8>' (checksum)
            //  - Health reset:     'RH<u8>' (checksum)
            //  - Program download: 'PD<u32><u8[]><u8>' (size, data, checksum)
            //  - Program upload:   'PU<u8>' (checksum)
            //  - Program run:      'PR<u8>' (checksum)
            //  - Program stop:     'PS<u8>' (checksum)
            //  - Memory read:      'MR<u32><u32><u8>' (address, size, checksum)
            //  - Memory write:     'MW<u32><u32><u8[]><u8>' (address, size, data, checksum)
            //  - Memory write mask:'MM<u32><u32><u8[]><u8[]><u8>' (address, size, data, mask, checksum)
            //  - Memory format:    'MF<u32><u32><u8><u8>' (address, size, value, checksum)
            //  - Source download:  'SD<u32><u8[]><u8>' (size, data, checksum) // Only available if PLCRUNTIME_SOURCE_ENABLED is defined
            //  - Source upload:    'SU<u32><u8>' (size, checksum) // Only available if PLCRUNTIME_SOURCE_ENABLED is defined
            // If the program is downloaded and the checksum is invalid, the runtime will restart
            u8 cmd[2] = { 0, 0 };
            u32 size = 0;
            u32 address = 0;
            u8* data = nullptr;
            u8 checksum = 0;
            u8 checksum_calc = 0;

            // Read the command
            cmd[0] = serialReadTimeout(); SERIAL_TIMEOUT_RETURN;
            if (Serial.peek() != '?')
                cmd[1] = serialReadTimeout(); SERIAL_TIMEOUT_RETURN;

            crc8_simple(checksum_calc, cmd[0]);
            crc8_simple(checksum_calc, cmd[1]);

            bool ping = cmd[0] == '?';
            bool plc_info = cmd[0] == 'P' && cmd[1] == 'I';
            bool plc_health = cmd[0] == 'P' && cmd[1] == 'H';
            bool plc_health_reset = cmd[0] == 'R' && cmd[1] == 'H';
            bool plc_reset = cmd[0] == 'R' && cmd[1] == 'S';
            bool program_download = cmd[0] == 'P' && cmd[1] == 'D';
            bool program_upload = cmd[0] == 'P' && cmd[1] == 'U';
            bool program_run = cmd[0] == 'P' && cmd[1] == 'R';
            bool program_stop = cmd[0] == 'P' && cmd[1] == 'S';
            bool memory_read = cmd[0] == 'M' && cmd[1] == 'R';
            bool memory_write = cmd[0] == 'M' && cmd[1] == 'W';
            bool memory_write_mask = cmd[0] == 'M' && cmd[1] == 'M';
            bool memory_format = cmd[0] == 'M' && cmd[1] == 'F';
            bool source_download = cmd[0] == 'S' && cmd[1] == 'D';
            bool source_upload = cmd[0] == 'S' && cmd[1] == 'U';

            if (ping) {
                Serial.println(F("<VovkPLC>"));
            } else if (plc_info) {
                Serial.print(F("PLC INFO - "));

                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;

                // Verify the checksum
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum"));
                    return;
                }
                printInfo();

            } else if (plc_health) {
                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;

                // Verify the checksum
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum"));
                    return;
                }

                DeviceHealth health;
                getDeviceHealth(health);
                
                Serial.print(F("PH"));
                printHexU32(health.last_cycle_time_us);
                printHexU32(health.min_cycle_time_us);
                printHexU32(health.max_cycle_time_us);
                printHexU32(health.last_ram_free);
                printHexU32(health.min_ram_free);
                printHexU32(health.max_ram_free);
                printHexU32(health.total_ram_size);
                printHexU32(health.last_period_us);
                printHexU32(health.min_period_us);
                printHexU32(health.max_period_us);
                printHexU32(health.last_jitter_us);
                printHexU32(health.min_jitter_us);
                printHexU32(health.max_jitter_us);
                Serial.println();

            } else if (plc_health_reset) {
                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;

                // Verify the checksum
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum"));
                    return;
                }

                resetDeviceHealth();
                Serial.println(F("OK HEALTH RESET"));

            } else if (plc_reset) {

                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;

                // Verify the checksum
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum"));
                    return;
                }

                Serial.println(F("OK PLC RESET"));
                processExit();
                return;

            } else if (program_download) {
                // Read the size
                size = (u32) serialReadHexByteTimeout();  SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));


                // Read the data
                program.format();
                program.prog_size = size;
                u8 b = 0;
                for (u32 i = 0; i < size; i++) {
                    b = serialReadHexByteTimeout(); SERIAL_TIMEOUT_JOB(processExit());
                    program.modify(i, b);
                    crc8_simple(checksum_calc, b);
                }

                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_JOB(processExit());

                // If the checksum is invalid, restart the runtime
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum, restarting the runtime..."));
                    delay(1000);
                    processExit();
                    return;
                }
                program.resetLine();

#ifdef PLCRUNTIME_EEPROM_STORAGE
                // Save program to flash storage
                u8 prog_checksum = 0;
                crc8_simple(prog_checksum, program.program, program.prog_size);
                if (!EEPROMStorage::saveProgram(program.program, program.prog_size, prog_checksum)) {
                    Serial.println(F("FLASH SAVE FAILED"));
                }
#endif // PLCRUNTIME_EEPROM_STORAGE

                Serial.println(F("PROGRAM DOWNLOAD COMPLETE"));
            } else if (program_upload) {
                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;

                // Verify the checksum
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum"));
                    return;
                }

                Serial.print(F("OK "));
                // Print the program
                program.println();

            } else if (program_run) {
                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;

                // Verify the checksum
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum"));
                    return;
                }

                Serial.println(F("OK PROGRAM RUN"));
            } else if (program_stop) {
                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;

                // Verify the checksum
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum"));
                    return;
                }

                Serial.println(F("OK PROGRAM STOP"));
            } else if (memory_read) {
                // Read the address
                address = (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));

                // Read the size
                size = (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));

                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;

                // Verify the checksum
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum"));
                    return;
                }

                // Check if the address and size are valid
                if (address + size > PLCRUNTIME_MAX_MEMORY_SIZE) {
                    Serial.println(F("Invalid address or size"));
                    return;
                }

                Serial.print(F("OK "));
                // Read the data
                u8 value;
                for (u32 i = 0; i < size; i++) {
                    get_u8(memory, address + i, value);
                    char c1 = (value >> 4) & 0x0f;
                    char c2 = value & 0x0f;
                    if (c1 < 10) c1 += '0';
                    else c1 += 'A' - 10;
                    if (c2 < 10) c2 += '0';
                    else c2 += 'A' - 10;
                    Serial.print(c1);
                    Serial.print(c2);
                }

                // Print the data
                Serial.println();
            } else if (memory_write) {
                // Read the address
                address = (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));

                // Read the size
                size = (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));

                // Read the data
                data = new u8[size];
                for (u32 i = 0; i < size; i++) {
                    data[i] = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                    crc8_simple(checksum_calc, data[i]);
                }

                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;

                // Verify the checksum
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum"));
                    return;
                }

                // Check if the address and size are valid
                if (address + size > PLCRUNTIME_MAX_MEMORY_SIZE) {
                    Serial.println(F("Invalid address or size"));
                    return;
                }

                // Write the data
                for (u32 i = 0; i < size; i++)
                    set_u8(memory, address + i, data[i]);

                Serial.println(F("OK MEMORY WRITE"));
            } else if (memory_write_mask) {
                // Read the address
                address = (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));

                // Read the size
                size = (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));

                // Read the data + mask (double length)
                data = new u8[size * 2];
                for (u32 i = 0; i < size * 2; i++) {
                    data[i] = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                    crc8_simple(checksum_calc, data[i]);
                }

                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;

                // Verify the checksum
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum"));
                    return;
                }

                // Check if the address and size are valid
                if (address + size > PLCRUNTIME_MAX_MEMORY_SIZE) {
                    Serial.println(F("Invalid address or size"));
                    return;
                }

                // Write the masked data
                for (u32 i = 0; i < size; i++) {
                    u8 current;
                    get_u8(memory, address + i, current);
                    u8 value = data[i];
                    u8 mask = data[size + i];
                    set_u8(memory, address + i, (current & ~mask) | (value & mask));
                }

                Serial.println(F("OK MEMORY WRITE MASK"));
            } else if (memory_format) {
                // Read the address
                address = (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));

                // Read the size
                size = (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (u32) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));

                // Read the value
                u8 value = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, value);

                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;

                // Verify the checksum
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum"));
                    return;
                }

                // Check if the address and size are valid
                if (address + size > PLCRUNTIME_MAX_MEMORY_SIZE) {
                    Serial.println(F("Invalid address or size"));
                    return;
                }

                // Format the memory
                for (u32 i = 0; i < size; i++)
                    set_u8(memory, address + i, value);


                Serial.println(F("OK MEMORY FORMAT"));
            } else if (source_download) {
                Serial.println(F("SOURCE DOWNLOAD - Not implemented"));
            } else if (source_upload) {
                Serial.println(F("SOURCE UPLOAD - Not implemented"));
            }
    }
#endif // PLCRUNTIME_SERIAL_ENABLED

#ifdef PLCRUNTIME_ETHERNET_ENABLED
        // To be implemented
#endif // PLCRUNTIME_ETHERNET_ENABLED

#ifdef PLCRUNTIME_WIFI_ENABLED
        // To be implemented
#endif // PLCRUNTIME_WIFI_ENABLED
#endif // __WASM__
}
};

// Clear the runtime stack
void VovkPLCRuntime::clear() {
    program.resetLine();
    stack.clear();
}
// Clear the runtime stack and reset the program line
void VovkPLCRuntime::clear(RuntimeProgram& program) {
    program.resetLine();
    stack.clear();
}

// Print the stack
int VovkPLCRuntime::printStack() { return stack.print(); }

void VovkPLCRuntime::updateGlobals() {
    u32 base = control_offset;
    u8 state = 0;
    state = state << 1 | P_10s; // 2.7
    state = state << 1 | P_5s; // 2.6
    state = state << 1 | P_2s; // 2.5
    state = state << 1 | P_1s; // 2.4
    state = state << 1 | P_500ms; // 2.3
    state = state << 1 | P_300ms; // 2.2
    state = state << 1 | P_200ms; // 2.1
    state = state << 1 | P_100ms; // 2.0
    memory[base + 2] = state;

    state = 0;
    state = state << 1 | P_1hr; // 3.7
    state = state << 1 | P_30min; // 3.6
    state = state << 1 | P_15min; // 3.5
    state = state << 1 | P_10min; // 3.4
    state = state << 1 | P_5min; // 3.3
    state = state << 1 | P_2min; // 3.2
    state = state << 1 | P_1min; // 3.1
    state = state << 1 | P_30s; // 3.0
    memory[base + 3] = state;

    state = 0;
    state = state << 1 | P_1day; // 4.6
    state = state << 1 | P_12hr; // 4.5
    state = state << 1 | P_6hr; // 4.4
    state = state << 1 | P_5hr; // 4.3
    state = state << 1 | P_4hr; // 4.2
    state = state << 1 | P_3hr; // 4.1
    state = state << 1 | P_2hr; // 4.0
    memory[base + 4] = state;

    state = 0;
    state = state << 1 | S_10s; // 5.7
    state = state << 1 | S_5s; // 5.6
    state = state << 1 | S_2s; // 5.5
    state = state << 1 | S_1s; // 5.4
    state = state << 1 | S_500ms; // 5.3
    state = state << 1 | S_300ms; // 5.2 
    state = state << 1 | S_200ms; // 5.1
    state = state << 1 | S_100ms; // 5.0
    memory[base + 5] = state;

    state = 0;
    state = state << 1 | S_1hr; // 6.7
    state = state << 1 | S_30min; // 6.6
    state = state << 1 | S_15min; // 6.5
    state = state << 1 | S_10min; // 6.4
    state = state << 1 | S_5min; // 6.3
    state = state << 1 | S_2min; // 6.2
    state = state << 1 | S_1min; // 6.1
    state = state << 1 | S_30s; // 6.0
    memory[base + 6] = state;

    memory[base + 8] = interval_time_seconds;
    memory[base + 9] = interval_time_minutes;
    memory[base + 10] = interval_time_hours;
    memory[base + 11] = interval_time_days;

    writeMemory(base + 12, (u8*) &uptime_seconds, sizeof(u32));

    // System Time (Unix Timestamp style) - Offset 16 (4 bytes)
    // Read from memory, increment if 1s elapsed, write back
    static bool _last_P_1s = false;
    if (P_1s && !_last_P_1s) {
        // Increment system time in memory (Little Endian)
        u32 t_current = 0;
        // Read current
        t_current |= memory[base + 16];
        t_current |= ((u32)memory[base + 17]) << 8;
        t_current |= ((u32)memory[base + 18]) << 16;
        t_current |= ((u32)memory[base + 19]) << 24;
        
        t_current++;
        
        // Write back
        memory[base + 16] = t_current & 0xFF;
        memory[base + 17] = (t_current >> 8) & 0xFF;
        memory[base + 18] = (t_current >> 16) & 0xFF;
        memory[base + 19] = (t_current >> 24) & 0xFF;
    }
    _last_P_1s = P_1s;

    // First Cycle Flag - Offset 20 (1 bit / byte)
    // 1 during the first cycle, 0 otherwise
    if (is_first_cycle) {
        memory[base + 20] |= 1; // Set bit 0
    } else {
        memory[base + 20] &= ~1; // Clear bit 0
    }
}

// Execute the whole PLC program, returns an erro code (0 on success)
RuntimeError VovkPLCRuntime::run(RuntimeProgram& program) { return run(program.program, program.prog_size); }

// Execute the whole PLC program, returns an erro code (0 on success)
RuntimeError VovkPLCRuntime::run(u8* program, u32 prog_size) {
#ifndef __WASM__ // WASM can optionally execute the global loop check, embedded systems must always call this
    IntervalGlobalLoopCheck();
#endif // __WASM__
    u32 start_us = (u32) micros();
    
    // Calculate period (time between run() calls)
    if (last_run_timestamp_us != 0) {
        last_period_us = start_us - last_run_timestamp_us;
        if (last_period_us < min_period_us) min_period_us = last_period_us;
        if (last_period_us > max_period_us) max_period_us = last_period_us;
        
        // Calculate jitter (variation in period)
        if (previous_period_us != 0) {
            last_jitter_us = last_period_us > previous_period_us ? 
                            (last_period_us - previous_period_us) : 
                            (previous_period_us - last_period_us);
            if (last_jitter_us < min_jitter_us) min_jitter_us = last_jitter_us;
            if (last_jitter_us > max_jitter_us) max_jitter_us = last_jitter_us;
        }
        previous_period_us = last_period_us;
    }
    last_run_timestamp_us = start_us;
    
    updateGlobals();
    u32 index = 0;
    u32 instruction_count = 0;
    RuntimeError status = STATUS_SUCCESS;
    while (index < prog_size) {
        status = step(program, prog_size, index);
        instruction_count++;
        if (status != STATUS_SUCCESS) {
            if (status == PROGRAM_EXITED)
                status = STATUS_SUCCESS;
            break;
        }
    }
    last_instruction_count = instruction_count;
    if (status == STATUS_SUCCESS) updateCycleStats((u32) (micros() - start_us));
    else updateRamStats();
    
    // Note: is_first_cycle is cleared here, but the memory flag 
    // at Offset 20 is NOT cleared. It remains set (if it was set) 
    // until the NEXT call to run(), where updateGlobals() will clear it 
    // if is_first_cycle is false.
    if (is_first_cycle) is_first_cycle = false;
    
    return status;
}




// Execute one PLC instruction at index, returns an error code (0 on success)
RuntimeError VovkPLCRuntime::step(RuntimeProgram& program) { return step(program.program, program.prog_size, program.program_line); }

// Execute one PLC instruction at index, returns an error code (0 on success)
RuntimeError VovkPLCRuntime::step(u8* program, u32 prog_size, u32& index) {
    if (prog_size == 0) return EMPTY_PROGRAM;
    if (index >= prog_size) return PROGRAM_SIZE_EXCEEDED;
    u8 opcode = program[index];
    index++;
    switch (opcode) {
        case NOP: return STATUS_SUCCESS;
        case LOGIC_AND: return PLCMethods::LOGIC_AND(this->stack);
        case LOGIC_OR: return PLCMethods::LOGIC_OR(this->stack);
        case LOGIC_NOT: return PLCMethods::LOGIC_NOT(this->stack);
        case LOGIC_XOR: return PLCMethods::LOGIC_XOR(this->stack);
        case CVT: return PLCMethods::CVT(this->stack, program, prog_size, index);
        case LOAD: return PLCMethods::LOAD(this->stack, this->memory, program, prog_size, index);
        case MOVE: return PLCMethods::MOVE(this->stack, this->memory, program, prog_size, index);
        case MOVE_COPY: return PLCMethods::MOVE_COPY(this->stack, this->memory, program, prog_size, index);
        case LOAD_FROM: return PLCMethods::LOAD_FROM(this->stack, this->memory, program, prog_size, index);
        case MOVE_TO: return PLCMethods::MOVE_TO(this->stack, this->memory, program, prog_size, index);
        case INC_MEM: return PLCMethods::INC_MEM(this->memory, program, prog_size, index);
        case DEC_MEM: return PLCMethods::DEC_MEM(this->memory, program, prog_size, index);
        case COPY: return PLCMethods::COPY(this->stack, program, prog_size, index);
        case SWAP: return PLCMethods::SWAP(this->stack, program, prog_size, index);
        case DROP: return PLCMethods::DROP(this->stack, program, prog_size, index);
        case CLEAR: return PLCMethods::CLEAR(this->stack);
        case JMP: return PLCMethods::handle_JMP(this->stack, program, prog_size, index);
        case JMP_IF: return PLCMethods::handle_JMP_IF(this->stack, program, prog_size, index);
        case JMP_IF_NOT: return PLCMethods::handle_JMP_IF_NOT(this->stack, program, prog_size, index);
        case CALL: return PLCMethods::handle_CALL(this->stack, program, prog_size, index);
        case CALL_IF: return PLCMethods::handle_CALL_IF(this->stack, program, prog_size, index);
        case CALL_IF_NOT: return PLCMethods::handle_CALL_IF_NOT(this->stack, program, prog_size, index);
        case JMP_REL: return PLCMethods::handle_JMP_REL(this->stack, program, prog_size, index);
        case JMP_IF_REL: return PLCMethods::handle_JMP_IF_REL(this->stack, program, prog_size, index);
        case JMP_IF_NOT_REL: return PLCMethods::handle_JMP_IF_NOT_REL(this->stack, program, prog_size, index);
        case CALL_REL: return PLCMethods::handle_CALL_REL(this->stack, program, prog_size, index);
        case CALL_IF_REL: return PLCMethods::handle_CALL_IF_REL(this->stack, program, prog_size, index);
        case CALL_IF_NOT_REL: return PLCMethods::handle_CALL_IF_NOT_REL(this->stack, program, prog_size, index);
        case RET: return PLCMethods::handle_RET(this->stack, program, prog_size, index);
        case RET_IF: return PLCMethods::handle_RET_IF(this->stack, program, prog_size, index);
        case RET_IF_NOT: return PLCMethods::handle_RET_IF_NOT(this->stack, program, prog_size, index);
        case type_pointer: return PLCMethods::PUSH_pointer(this->stack, program, prog_size, index);
        case type_bool: return PLCMethods::PUSH_bool(this->stack, program, prog_size, index);
        case type_u8: return PLCMethods::push_u8(this->stack, program, prog_size, index);
        case type_i8: return PLCMethods::push_i8(this->stack, program, prog_size, index);
        case type_u16: return PLCMethods::push_u16(this->stack, program, prog_size, index);
        case type_i16: return PLCMethods::push_i16(this->stack, program, prog_size, index);
        case type_u32: return PLCMethods::push_u32(this->stack, program, prog_size, index);
        case type_i32: return PLCMethods::push_i32(this->stack, program, prog_size, index);
        case type_f32: return PLCMethods::push_f32(this->stack, program, prog_size, index);
#ifdef USE_X64_OPS
        case type_u64: return PLCMethods::push_u64(this->stack, program, prog_size, index);
        case type_i64: return PLCMethods::push_i64(this->stack, program, prog_size, index);
        case type_f64: return PLCMethods::push_f64(this->stack, program, prog_size, index);
#endif // USE_X64_OPS
        case ADD: return PLCMethods::handle_ADD(this->stack, program, prog_size, index);
        case SUB: return PLCMethods::handle_SUB(this->stack, program, prog_size, index);
        case MUL: return PLCMethods::handle_MUL(this->stack, program, prog_size, index);
        case DIV: return PLCMethods::handle_DIV(this->stack, program, prog_size, index);
        case MOD: return PLCMethods::handle_MOD(this->stack, program, prog_size, index);
        case POW: return PLCMethods::handle_POW(this->stack, program, prog_size, index);
        case ABS: return PLCMethods::handle_ABS(this->stack, program, prog_size, index);
        case NEG: return PLCMethods::handle_NEG(this->stack, program, prog_size, index);
        case SQRT: return PLCMethods::handle_SQRT(this->stack, program, prog_size, index);
        case SIN: return PLCMethods::handle_SIN(this->stack, program, prog_size, index);
        case COS: return PLCMethods::handle_COS(this->stack, program, prog_size, index);

        case TON_CONST: return PLCMethods::handle_TON_CONST(this->stack, this->memory, program, prog_size, index);
        case TON_MEM: return PLCMethods::handle_TON_MEM(this->stack, this->memory, program, prog_size, index);
        case TOF_CONST: return PLCMethods::handle_TOF_CONST(this->stack, this->memory, program, prog_size, index);
        case TOF_MEM: return PLCMethods::handle_TOF_MEM(this->stack, this->memory, program, prog_size, index);
        case TP_CONST: return PLCMethods::handle_TP_CONST(this->stack, this->memory, program, prog_size, index);
        case TP_MEM: return PLCMethods::handle_TP_MEM(this->stack, this->memory, program, prog_size, index);

        case CTU_CONST: return PLCMethods::handle_CTU_CONST(this->stack, this->memory, program, prog_size, index);
        case CTU_MEM: return PLCMethods::handle_CTU_MEM(this->stack, this->memory, program, prog_size, index);
        case CTD_CONST: return PLCMethods::handle_CTD_CONST(this->stack, this->memory, program, prog_size, index);
        case CTD_MEM: return PLCMethods::handle_CTD_MEM(this->stack, this->memory, program, prog_size, index);

        case GET_X8_B0: return PLCMethods::handle_GET_X8_B0(this->stack);
        case GET_X8_B1: return PLCMethods::handle_GET_X8_B1(this->stack);
        case GET_X8_B2: return PLCMethods::handle_GET_X8_B2(this->stack);
        case GET_X8_B3: return PLCMethods::handle_GET_X8_B3(this->stack);
        case GET_X8_B4: return PLCMethods::handle_GET_X8_B4(this->stack);
        case GET_X8_B5: return PLCMethods::handle_GET_X8_B5(this->stack);
        case GET_X8_B6: return PLCMethods::handle_GET_X8_B6(this->stack);
        case GET_X8_B7: return PLCMethods::handle_GET_X8_B7(this->stack);
        case SET_X8_B0: return PLCMethods::handle_SET_X8_B0(this->stack);
        case SET_X8_B1: return PLCMethods::handle_SET_X8_B1(this->stack);
        case SET_X8_B2: return PLCMethods::handle_SET_X8_B2(this->stack);
        case SET_X8_B3: return PLCMethods::handle_SET_X8_B3(this->stack);
        case SET_X8_B4: return PLCMethods::handle_SET_X8_B4(this->stack);
        case SET_X8_B5: return PLCMethods::handle_SET_X8_B5(this->stack);
        case SET_X8_B6: return PLCMethods::handle_SET_X8_B6(this->stack);
        case SET_X8_B7: return PLCMethods::handle_SET_X8_B7(this->stack);
        case RSET_X8_B0: return PLCMethods::handle_RSET_X8_B0(this->stack);
        case RSET_X8_B1: return PLCMethods::handle_RSET_X8_B1(this->stack);
        case RSET_X8_B2: return PLCMethods::handle_RSET_X8_B2(this->stack);
        case RSET_X8_B3: return PLCMethods::handle_RSET_X8_B3(this->stack);
        case RSET_X8_B4: return PLCMethods::handle_RSET_X8_B4(this->stack);
        case RSET_X8_B5: return PLCMethods::handle_RSET_X8_B5(this->stack);
        case RSET_X8_B6: return PLCMethods::handle_RSET_X8_B6(this->stack);
        case RSET_X8_B7: return PLCMethods::handle_RSET_X8_B7(this->stack);
        case READ_X8_B0: return PLCMethods::handle_READ_X8_B0(this->stack, this->memory, program, prog_size, index);
        case READ_X8_B1: return PLCMethods::handle_READ_X8_B1(this->stack, this->memory, program, prog_size, index);
        case READ_X8_B2: return PLCMethods::handle_READ_X8_B2(this->stack, this->memory, program, prog_size, index);
        case READ_X8_B3: return PLCMethods::handle_READ_X8_B3(this->stack, this->memory, program, prog_size, index);
        case READ_X8_B4: return PLCMethods::handle_READ_X8_B4(this->stack, this->memory, program, prog_size, index);
        case READ_X8_B5: return PLCMethods::handle_READ_X8_B5(this->stack, this->memory, program, prog_size, index);
        case READ_X8_B6: return PLCMethods::handle_READ_X8_B6(this->stack, this->memory, program, prog_size, index);
        case READ_X8_B7: return PLCMethods::handle_READ_X8_B7(this->stack, this->memory, program, prog_size, index);
        case WRITE_X8_B0: return PLCMethods::handle_WRITE_X8_B0(this->stack, this->memory, program, prog_size, index);
        case WRITE_X8_B1: return PLCMethods::handle_WRITE_X8_B1(this->stack, this->memory, program, prog_size, index);
        case WRITE_X8_B2: return PLCMethods::handle_WRITE_X8_B2(this->stack, this->memory, program, prog_size, index);
        case WRITE_X8_B3: return PLCMethods::handle_WRITE_X8_B3(this->stack, this->memory, program, prog_size, index);
        case WRITE_X8_B4: return PLCMethods::handle_WRITE_X8_B4(this->stack, this->memory, program, prog_size, index);
        case WRITE_X8_B5: return PLCMethods::handle_WRITE_X8_B5(this->stack, this->memory, program, prog_size, index);
        case WRITE_X8_B6: return PLCMethods::handle_WRITE_X8_B6(this->stack, this->memory, program, prog_size, index);
        case WRITE_X8_B7: return PLCMethods::handle_WRITE_X8_B7(this->stack, this->memory, program, prog_size, index);
        case WRITE_S_X8_B0: return PLCMethods::handle_WRITE_S_X8_B0(this->stack, this->memory, program, prog_size, index);
        case WRITE_S_X8_B1: return PLCMethods::handle_WRITE_S_X8_B1(this->stack, this->memory, program, prog_size, index);
        case WRITE_S_X8_B2: return PLCMethods::handle_WRITE_S_X8_B2(this->stack, this->memory, program, prog_size, index);
        case WRITE_S_X8_B3: return PLCMethods::handle_WRITE_S_X8_B3(this->stack, this->memory, program, prog_size, index);
        case WRITE_S_X8_B4: return PLCMethods::handle_WRITE_S_X8_B4(this->stack, this->memory, program, prog_size, index);
        case WRITE_S_X8_B5: return PLCMethods::handle_WRITE_S_X8_B5(this->stack, this->memory, program, prog_size, index);
        case WRITE_S_X8_B6: return PLCMethods::handle_WRITE_S_X8_B6(this->stack, this->memory, program, prog_size, index);
        case WRITE_S_X8_B7: return PLCMethods::handle_WRITE_S_X8_B7(this->stack, this->memory, program, prog_size, index);
        case WRITE_R_X8_B0: return PLCMethods::handle_WRITE_R_X8_B0(this->stack, this->memory, program, prog_size, index);
        case WRITE_R_X8_B1: return PLCMethods::handle_WRITE_R_X8_B1(this->stack, this->memory, program, prog_size, index);
        case WRITE_R_X8_B2: return PLCMethods::handle_WRITE_R_X8_B2(this->stack, this->memory, program, prog_size, index);
        case WRITE_R_X8_B3: return PLCMethods::handle_WRITE_R_X8_B3(this->stack, this->memory, program, prog_size, index);
        case WRITE_R_X8_B4: return PLCMethods::handle_WRITE_R_X8_B4(this->stack, this->memory, program, prog_size, index);
        case WRITE_R_X8_B5: return PLCMethods::handle_WRITE_R_X8_B5(this->stack, this->memory, program, prog_size, index);
        case WRITE_R_X8_B6: return PLCMethods::handle_WRITE_R_X8_B6(this->stack, this->memory, program, prog_size, index);
        case WRITE_R_X8_B7: return PLCMethods::handle_WRITE_R_X8_B7(this->stack, this->memory, program, prog_size, index);
        case WRITE_INV_X8_B0: return PLCMethods::handle_WRITE_INV_X8_B0(this->stack, this->memory, program, prog_size, index);
        case WRITE_INV_X8_B1: return PLCMethods::handle_WRITE_INV_X8_B1(this->stack, this->memory, program, prog_size, index);
        case WRITE_INV_X8_B2: return PLCMethods::handle_WRITE_INV_X8_B2(this->stack, this->memory, program, prog_size, index);
        case WRITE_INV_X8_B3: return PLCMethods::handle_WRITE_INV_X8_B3(this->stack, this->memory, program, prog_size, index);
        case WRITE_INV_X8_B4: return PLCMethods::handle_WRITE_INV_X8_B4(this->stack, this->memory, program, prog_size, index);
        case WRITE_INV_X8_B5: return PLCMethods::handle_WRITE_INV_X8_B5(this->stack, this->memory, program, prog_size, index);
        case WRITE_INV_X8_B6: return PLCMethods::handle_WRITE_INV_X8_B6(this->stack, this->memory, program, prog_size, index);
        case WRITE_INV_X8_B7: return PLCMethods::handle_WRITE_INV_X8_B7(this->stack, this->memory, program, prog_size, index);

        case READ_BIT_DU: return PLCMethods::handle_READ_BIT_DU(this->stack, this->memory, program, prog_size, index);
        case READ_BIT_DD: return PLCMethods::handle_READ_BIT_DD(this->stack, this->memory, program, prog_size, index);
        case READ_BIT_INV_DU: return PLCMethods::handle_READ_BIT_INV_DU(this->stack, this->memory, program, prog_size, index);
        case READ_BIT_INV_DD: return PLCMethods::handle_READ_BIT_INV_DD(this->stack, this->memory, program, prog_size, index);

        case WRITE_BIT_DU: return PLCMethods::handle_WRITE_BIT_DU(this->stack, this->memory, program, prog_size, index);
        case WRITE_BIT_DD: return PLCMethods::handle_WRITE_BIT_DD(this->stack, this->memory, program, prog_size, index);
        case WRITE_BIT_INV_DU: return PLCMethods::handle_WRITE_BIT_INV_DU(this->stack, this->memory, program, prog_size, index);
        case WRITE_BIT_INV_DD: return PLCMethods::handle_WRITE_BIT_INV_DD(this->stack, this->memory, program, prog_size, index);
        case WRITE_SET_DU: return PLCMethods::handle_WRITE_SET_DU(this->stack, this->memory, program, prog_size, index);
        case WRITE_SET_DD: return PLCMethods::handle_WRITE_SET_DD(this->stack, this->memory, program, prog_size, index);
        case WRITE_RSET_DU: return PLCMethods::handle_WRITE_RSET_DU(this->stack, this->memory, program, prog_size, index);
        case WRITE_RSET_DD: return PLCMethods::handle_WRITE_RSET_DD(this->stack, this->memory, program, prog_size, index);
        
        case STACK_DU: return PLCMethods::handle_STACK_DU(this->stack, this->memory, program, prog_size, index);
        case STACK_DD: return PLCMethods::handle_STACK_DD(this->stack, this->memory, program, prog_size, index);
        case STACK_DC: return PLCMethods::handle_STACK_DC(this->stack, this->memory, program, prog_size, index);

        // Branch stack operations for parallel branches in ladder logic
        case BR_SAVE: {
            // Pop u8 from stack, push bit to BR: BR = (BR << 1) | (pop_u8() ? 1 : 0)
            u8 value = this->stack.pop_u8();
            this->BR = (this->BR << 1) | (value ? 1 : 0);
            return STATUS_SUCCESS;
        }
        case BR_READ: {
            // Peek BR top, push u8 to stack: push_u8((BR & 1) ? 1 : 0)
            u8 value = (this->BR & 1) ? 1 : 0;
            this->stack.push(value);
            return STATUS_SUCCESS;
        }
        case BR_DROP: {
            // Pop/discard BR top: BR >>= 1
            this->BR >>= 1;
            return STATUS_SUCCESS;
        }
        case BR_CLR: {
            // Clear BR stack: BR = 0
            this->BR = 0;
            return STATUS_SUCCESS;
        }

        case BW_AND_X8: return PLCMethods::handle_BW_AND_X8(this->stack);
        case BW_AND_X16: return PLCMethods::handle_BW_AND_X16(this->stack);
        case BW_AND_X32: return PLCMethods::handle_BW_AND_X32(this->stack);
        case BW_OR_X8: return PLCMethods::handle_BW_OR_X8(this->stack);
        case BW_OR_X16: return PLCMethods::handle_BW_OR_X16(this->stack);
        case BW_OR_X32: return PLCMethods::handle_BW_OR_X32(this->stack);
        case BW_XOR_X8: return PLCMethods::handle_BW_XOR_X8(this->stack);
        case BW_XOR_X16: return PLCMethods::handle_BW_XOR_X16(this->stack);
        case BW_XOR_X32: return PLCMethods::handle_BW_XOR_X32(this->stack);
        case BW_NOT_X8: return PLCMethods::handle_BW_NOT_X8(this->stack);
        case BW_NOT_X16: return PLCMethods::handle_BW_NOT_X16(this->stack);
        case BW_NOT_X32: return PLCMethods::handle_BW_NOT_X32(this->stack);
#ifdef USE_X64_OPS
        case BW_AND_X64: return PLCMethods::handle_BW_AND_X64(this->stack);
        case BW_OR_X64: return PLCMethods::handle_BW_OR_X64(this->stack);
        case BW_XOR_X64: return PLCMethods::handle_BW_XOR_X64(this->stack);
        case BW_NOT_X64: return PLCMethods::handle_BW_NOT_X64(this->stack);
#endif // USE_X64_OPS
        case CMP_EQ: return PLCMethods::handle_CMP_EQ(this->stack, program, prog_size, index);
        case CMP_NEQ: return PLCMethods::handle_CMP_NEQ(this->stack, program, prog_size, index);
        case CMP_GT: return PLCMethods::handle_CMP_GT(this->stack, program, prog_size, index);
        case CMP_GTE: return PLCMethods::handle_CMP_GTE(this->stack, program, prog_size, index);
        case CMP_LT: return PLCMethods::handle_CMP_LT(this->stack, program, prog_size, index);
        case CMP_LTE: return PLCMethods::handle_CMP_LTE(this->stack, program, prog_size, index);
        
        // Metadata instructions - runtime skips over these (used for decompilation)
        case LANG: {
            // Skip over language_id byte
            index += 1;
            return STATUS_SUCCESS;
        }
        case COMMENT: {
            // Skip over length byte + comment characters
            if (index >= prog_size) return PROGRAM_SIZE_EXCEEDED;
            u32 comment_len = program[index];
            u32 index_end = index + 1 + comment_len;
            if (index_end >= prog_size) return PROGRAM_SIZE_EXCEEDED;
            index = index_end;
            return STATUS_SUCCESS;
        }
        
        case EXIT: {
            return PROGRAM_EXITED;
            // return PLCMethods::handle_EXIT(this->stack, program, prog_size, index);
        }
        default: return UNKNOWN_INSTRUCTION;
    }
}
