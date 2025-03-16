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

#ifndef PLCRUNTIME_NUM_OF_INPUTS
#define PLCRUNTIME_NUM_OF_INPUTS 16
#endif // PLCRUNTIME_NUM_OF_INPUTS

#ifndef PLCRUNTIME_NUM_OF_OUTPUTS
#define PLCRUNTIME_NUM_OF_OUTPUTS 16
#endif // PLCRUNTIME_NUM_OF_OUTPUTS

#ifndef PLCRUNTIME_INPUT_OFFSET 
#define PLCRUNTIME_INPUT_OFFSET 16
#endif // PLCRUNTIME_INPUT_OFFSET

#ifndef PLCRUNTIME_OUTPUT_OFFSET
#define PLCRUNTIME_OUTPUT_OFFSET PLCRUNTIME_INPUT_OFFSET + PLCRUNTIME_NUM_OF_INPUTS
#endif // PLCRUNTIME_OUTPUT_OFFSET

#ifndef PLCRUNTIME_MAX_STACK_SIZE
#define PLCRUNTIME_MAX_STACK_SIZE 16
#endif // PLCRUNTIME_MAX_STACK_SIZE

#ifndef PLCRUNTIME_MAX_MEMORY_SIZE
#define PLCRUNTIME_MAX_MEMORY_SIZE 64
#endif // PLCRUNTIME_MAX_MEMORY_SIZE

#ifndef PLCRUNTIME_MAX_PROGRAM_SIZE
#define PLCRUNTIME_MAX_PROGRAM_SIZE 1024
#endif // PLCRUNTIME_MAX_PROGRAM_SIZE

#ifdef __WASM__
#include "assembly/wasm/wasm.h"
#endif

#include "runtime-tools.h"
#include "stack/runtime-stack.h"
#include "runtime-interval.h"
#include "arithmetics/crc8.h"
#include "stack/stack-struct-impl.h"
#include "runtime-instructions.h"
#include "stack/runtime-stack.h"
#include "arithmetics/runtime-arithmetics.h"
#include "runtime-program.h"

#define SERIAL_TIMEOUT_RETURN if (serial_timeout) return;
#define SERIAL_TIMEOUT_JOB(task) if (serial_timeout) { task; return; };

class VovkPLCRuntime {
private:
    bool started_up = false;
public:
    const u32 input_offset = PLCRUNTIME_INPUT_OFFSET; // Output offset in memory
    const u32 output_offset = PLCRUNTIME_OUTPUT_OFFSET; // Output offset in memory

    RuntimeStack stack = RuntimeStack(); // Active memory stack for PLC execution
    u8 memory[PLCRUNTIME_MAX_MEMORY_SIZE]; // PLC memory to manipulate
    RuntimeProgram program = RuntimeProgram(); // Active PLC program

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
    }


    void printProperties() {
#ifdef __WASM__
        Serial.printf("Inputs [%d] at offset %d\n", PLCRUNTIME_NUM_OF_INPUTS, input_offset);
        Serial.printf("Outputs [%d] at offset %d\n", PLCRUNTIME_NUM_OF_OUTPUTS, output_offset);
        Serial.printf("Stack: %d (%d)\n", stack.size(), PLCRUNTIME_MAX_STACK_SIZE);
        Serial.printf("Memory: %d\n", PLCRUNTIME_MAX_MEMORY_SIZE);
        Serial.printf("Program: %d (%d)\n", program.prog_size, PLCRUNTIME_MAX_PROGRAM_SIZE);
#endif // __WASM__
    }

    void formatMemory() {
        for (u32 i = 0; i < PLCRUNTIME_MAX_MEMORY_SIZE; i++) memory[i] = 0;
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

    void listen() {
#ifdef __WASM__
        // Unused, access is available directly through the wasm.h interface
#else // __WASM__
        // Production code for microcontrollers
        // Listen for input from [serial, ethernet, wifi, etc.]
#ifdef PLCRUNTIME_SERIAL_ENABLED
            // If the serial port is available and the first character is not 'P' or 'M', skip the character
        while (Serial.available() && Serial.peek() != 'R' && Serial.peek() != 'P' && Serial.peek() != 'M' && Serial.peek() != 'S') Serial.read();
        if (Serial.available() > 1) {
            // Command syntax:
            // <command>[<size>][<data>]<checksum>
            // Where the command is always 2 characters and the rest is %02x encoded
            // Possible commands:
            //  - PLC reset:        'RS<u8>' (checksum)
            //  - Program download: 'PD<u32><u8[]><u8>' (size, data, checksum)
            //  - Program upload:   'PU<u8>' (checksum)
            //  - Program run:      'PR<u8>' (checksum)
            //  - Program stop:     'PS<u8>' (checksum)
            //  - Memory read:      'MR<u32><u32><u8>' (address, size, checksum)
            //  - Memory write:     'MW<u32><u32><u8[]><u8>' (address, size, data, checksum)
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
            cmd[1] = serialReadTimeout(); SERIAL_TIMEOUT_RETURN;

            crc8_simple(checksum_calc, cmd[0]);
            crc8_simple(checksum_calc, cmd[1]);

            bool plc_reset = cmd[0] == 'R' && cmd[1] == 'S';
            bool program_download = cmd[0] == 'P' && cmd[1] == 'D';
            bool program_upload = cmd[0] == 'P' && cmd[1] == 'U';
            bool program_run = cmd[0] == 'P' && cmd[1] == 'R';
            bool program_stop = cmd[0] == 'P' && cmd[1] == 'S';
            bool memory_read = cmd[0] == 'M' && cmd[1] == 'R';
            bool memory_write = cmd[0] == 'M' && cmd[1] == 'W';
            bool memory_format = cmd[0] == 'M' && cmd[1] == 'F';
            bool source_download = cmd[0] == 'S' && cmd[1] == 'D';
            bool source_upload = cmd[0] == 'S' && cmd[1] == 'U';

            if (plc_reset) {
                Serial.print(F("PLC RESET - "));

                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;

                // Verify the checksum
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum"));
                    return;
                }

                Serial.println(F("Complete"));
                processExit();
                return;

            } else if (program_download) {
                Serial.print(F("PROGRAM DOWNLOAD - "));
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

                Serial.println(F("Complete"));
            } else if (program_upload) {
                Serial.print(F("PROGRAM UPLOAD - "));
                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;

                // Verify the checksum
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum"));
                    return;
                }

                // Print the program
                program.println();

                Serial.println(F("Complete"));
            } else if (program_run) {
                Serial.print(F("PROGRAM RUN - "));
                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;

                // Verify the checksum
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum"));
                    return;
                }

                Serial.println(F("Complete"));
            } else if (program_stop) {
                Serial.print(F("PROGRAM STOP - "));
                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;

                // Verify the checksum
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum"));
                    return;
                }

                Serial.println(F("Complete"));
            } else if (memory_read) {
                Serial.print(F("MEMORY READ - "));
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
                    Serial.print(' ');
                }

                // Print the data
                Serial.println();

                Serial.println(F("Complete"));
            } else if (memory_write) {
                Serial.print(F("MEMORY WRITE - "));
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

                // Write the data
                for (u32 i = 0; i < size; i++)
                    set_u8(memory, address + i, data[i]);

                Serial.println(F("Complete"));
            } else if (memory_format) {
                Serial.print(F("MEMORY FORMAT - "));
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

                // Format the memory
                for (u32 i = 0; i < size; i++)
                    set_u8(memory, address + i, value);

                Serial.println(F("Complete"));
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
    u8 state = 0;
    state = state << 1 | P_10s; // 2.7
    state = state << 1 | P_5s; // 2.6
    state = state << 1 | P_2s; // 2.5
    state = state << 1 | P_1s; // 2.4
    state = state << 1 | P_500ms; // 2.3
    state = state << 1 | P_300ms; // 2.2
    state = state << 1 | P_200ms; // 2.1
    state = state << 1 | P_100ms; // 2.0
    memory[2] = state;

    state = 0;
    state = state << 1 | P_1hr; // 3.7
    state = state << 1 | P_30min; // 3.6
    state = state << 1 | P_15min; // 3.5
    state = state << 1 | P_10min; // 3.4
    state = state << 1 | P_5min; // 3.3
    state = state << 1 | P_2min; // 3.2
    state = state << 1 | P_1min; // 3.1
    state = state << 1 | P_30s; // 3.0
    memory[3] = state;

    state = 0;
    state = state << 1 | P_1day; // 4.6
    state = state << 1 | P_12hr; // 4.5
    state = state << 1 | P_6hr; // 4.4
    state = state << 1 | P_5hr; // 4.3
    state = state << 1 | P_4hr; // 4.2
    state = state << 1 | P_3hr; // 4.1
    state = state << 1 | P_2hr; // 4.0
    memory[4] = state;

    state = 0;
    state = state << 1 | S_10s; // 5.7
    state = state << 1 | S_5s; // 5.6
    state = state << 1 | S_2s; // 5.5
    state = state << 1 | S_1s; // 5.4
    state = state << 1 | S_500ms; // 5.3
    state = state << 1 | S_300ms; // 5.2 
    state = state << 1 | S_200ms; // 5.1
    state = state << 1 | S_100ms; // 5.0
    memory[5] = state;
    
    state = 0;
    state = state << 1 | S_1hr; // 6.7
    state = state << 1 | S_30min; // 6.6
    state = state << 1 | S_15min; // 6.5
    state = state << 1 | S_10min; // 6.4
    state = state << 1 | S_5min; // 6.3
    state = state << 1 | S_2min; // 6.2
    state = state << 1 | S_1min; // 6.1
    state = state << 1 | S_30s; // 6.0
    memory[6] = state;

    memory[8] = interval_time_seconds;
    memory[9] = interval_time_minutes;
    memory[10] = interval_time_hours;
    memory[11] = interval_time_days;

    writeMemory(12, (u8*) &uptime_seconds, sizeof(u32));
}

// Execute the whole PLC program, returns an erro code (0 on success)
RuntimeError VovkPLCRuntime::run(RuntimeProgram& program) { return run(program.program, program.prog_size); }

// Execute the whole PLC program, returns an erro code (0 on success)
RuntimeError VovkPLCRuntime::run(u8* program, u32 prog_size) {
    #ifndef __WASM__ // WASM can optionally execute the global loop check, embedded systems must always call this
    IntervalGlobalLoopCheck();
    #endif // __WASM__
    updateGlobals();
    u32 index = 0;
    while (index < prog_size) {
        RuntimeError status = step(program, prog_size, index);
        if (status != STATUS_SUCCESS) {
            if (status == PROGRAM_EXITED)
                return STATUS_SUCCESS;
            return status;
        }
    }
    return STATUS_SUCCESS;
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
        case EXIT: {
            return PROGRAM_EXITED;
            // return PLCMethods::handle_EXIT(this->stack, program, prog_size, index);
        }
        default: return UNKNOWN_INSTRUCTION;
    }
}