// runtime-lib.h - 1.0.0 - 2022-12-11
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

#ifndef PLCRUNTIME_INPUT_OFFSET 
#define PLCRUNTIME_INPUT_OFFSET 10
#endif // PLCRUNTIME_INPUT_OFFSET

#ifndef PLCRUNTIME_OUTPUT_OFFSET
#define PLCRUNTIME_OUTPUT_OFFSET 10
#endif // PLCRUNTIME_OUTPUT_OFFSET

#ifndef PLCRUNTIME_DEFAULT_STACK_SIZE
#define PLCRUNTIME_DEFAULT_STACK_SIZE 16
#endif // PLCRUNTIME_DEFAULT_STACK_SIZE

#ifndef PLCRUNTIME_DEFAULT_MEMORY_SIZE
#define PLCRUNTIME_DEFAULT_MEMORY_SIZE 64
#endif // PLCRUNTIME_DEFAULT_MEMORY_SIZE

#ifndef PLCRUNTIME_DEFAULT_PROGRAM_SIZE
#define PLCRUNTIME_DEFAULT_PROGRAM_SIZE 1024
#endif // PLCRUNTIME_DEFAULT_PROGRAM_SIZE

#ifdef __WASM__
#include "assembly/wasm/wasm.h"
#endif

#include "runtime-tools.h"
#include "runtime-interval.h"
#include "arithmetics/crc8.h"
#include "stack/stack-struct-impl.h"
#include "runtime-instructions.h"
#include "stack/runtime-stack.h"
#include "arithmetics/runtime-arithmetics.h"
#include "runtime-program.h"

#define SERIAL_TIMEOUT_RETURN if (serial_timeout) return;
#define SERIAL_TIMEOUT_JOB(task) if (serial_timeout) { Serial.flush(); task; return; };

class VovkPLCRuntime {
private:
    bool started_up = false;
public:
    const u32 input_offset = PLCRUNTIME_INPUT_OFFSET; // Output offset in memory
    const u32 output_offset = PLCRUNTIME_OUTPUT_OFFSET + PLCRUNTIME_INPUT_OFFSET; // Output offset in memory
    u32 max_stack_size = PLCRUNTIME_DEFAULT_STACK_SIZE; // Maximum stack size
    u32 memory_size = PLCRUNTIME_DEFAULT_MEMORY_SIZE; // PLC memory size
    u32 program_size = PLCRUNTIME_DEFAULT_PROGRAM_SIZE; // PLC program size
#ifdef __WASM__
    RuntimeStack* stack = nullptr; // Active memory stack for PLC execution
    RuntimeProgram* program = nullptr; // Active PLC program
#else
    RuntimeStack* stack = new RuntimeStack(); // Active memory stack for PLC execution
    RuntimeProgram* program = new RuntimeProgram(); // Active PLC program
#endif

    static void splash() {
        Serial.println();
        REPRINTLN(70, ':');
        Serial.println(F(":: Using VovkPLCRuntime Library - Author J.Vovk <jozo132@gmail.com> ::"));
        REPRINTLN(70, ':');
    }

    void startup() {
        if (started_up) return;
        started_up = true;
        splash();
        if (stack == nullptr) stack = new RuntimeStack(max_stack_size, max_stack_size, memory_size);
        else stack->format(max_stack_size, max_stack_size, memory_size);
        if (program == nullptr) program = new RuntimeProgram(program_size);
        else program->format(program_size);
        program->begin();
        formatMemory(memory_size);
    }

    void formatMemory(u32 size, u8* data = nullptr) {
        if (size == 0) return;
        if (stack == nullptr) stack = new RuntimeStack(max_stack_size, max_stack_size, memory_size);
        stack->memory->format(size);
        if (data == nullptr) {
            for (u32 i = 0; i < size; i++)
                stack->memory->set(i, 0);
        } else {
            for (u32 i = 0; i < size; i++)
                stack->memory->set(i, data[i]);
        }
    }

    VovkPLCRuntime(u32 max_stack_size, u32 memory_size, u32 program_size) {
        this->max_stack_size = max_stack_size;
        this->memory_size = memory_size;
        this->program_size = program_size;
    }
    VovkPLCRuntime(u32 max_stack_size, u32 memory_size) {
        this->max_stack_size = max_stack_size;
        this->memory_size = memory_size;
    }
    VovkPLCRuntime(u32 max_stack_size, RuntimeProgram& program) {
        this->max_stack_size = max_stack_size;
        this->program = &program;
    }
    VovkPLCRuntime(u32 max_stack_size, u32 memory_size, RuntimeProgram& program) {
        this->max_stack_size = max_stack_size;
        this->program = &program;
        this->memory_size = memory_size;
    }
    ~VovkPLCRuntime() {
        if (stack != nullptr) delete stack;
        if (program != nullptr) delete program;
    }
    void attachProgram(RuntimeProgram& program) {
        if (this->program != nullptr) delete this->program;
        this->program = &program;
    }

    void loadProgramUnsafe(u8* program, u32 program_size) {
        if (this->program == nullptr) this->program = new RuntimeProgram(program_size);
        this->program->loadUnsafe(program, program_size);
    }

    void loadProgram(u8* program, u32 program_size, u8 checksum) {
        if (this->program == nullptr) this->program = new RuntimeProgram(program_size);
        this->program->load(program, program_size, checksum);
    }

    // Clear the stack
    void clear();
    // Clear the stack and reset the program line
    void clear(RuntimeProgram& program);
    // Execute one PLC instruction at index, returns an error code (0 on success)
    RuntimeError step(u8* program, u32 program_size, u32& index);
    // Execute the whole PLC program, returns an error code (0 on success)
    RuntimeError run(u8* program, u32 program_size);
    // Execute one PLC instruction, returns an error code (0 on success)
    RuntimeError step(RuntimeProgram& program);
    // Run/Continue the whole PLC program from where it left off, returns an error code (0 on success)
    RuntimeError run(RuntimeProgram& program);
    // Execute one PLC instruction at index, returns an error code (0 on success)
    RuntimeError step(u32& index) {
        if (program == nullptr) return NO_PROGRAM;
        return step(program->program, program->program_size, index);
    }
    // Execute the whole PLC program, returns an error code (0 on success)
    RuntimeError run() {
        if (!started_up) startup();
        if (program == nullptr) return NO_PROGRAM;
        return run(program->program, program->program_size);
    }
    // Run the whole PLC program from the beginning, returns an error code (0 on success)
    RuntimeError cleanRun(RuntimeProgram& program) {
        if (!started_up) startup();
        clear(program);
        return run(program);
    }
    // Run the whole PLC program from the beginning, returns an error code (0 on success)
    RuntimeError cleanRun() {
        if (!started_up) startup();
        if (program == nullptr) return NO_PROGRAM;
        clear();
        return run(program->program, program->program_size);
    }
    // Read a custom type T value from the stack. This will pop the stack by sizeof(T) bytes and return the value.
    template <typename T> T read() {
        if (!started_up) startup();
        if (stack->size() < (u32) sizeof(T)) return 0;
        u8 temp[sizeof(T)];
        for (u32 i = 0; i < (u32) sizeof(T); i++)
            temp[i] = stack->pop();
        T value = *(T*) temp;
        return value;
    }

    // Print the stack buffer to the serial port
    int printStack();

    void setInput(u32 index, byte value) {
        if (stack == nullptr) return;
        if (stack->memory == nullptr) return;
        stack->memory->set(index + input_offset, value);
    }

    void setInputBit(u32 index, u8 bit, bool value) {
        if (stack == nullptr) return;
        if (stack->memory == nullptr) return;
        byte temp = 0;
        bool error = stack->memory->get(index + input_offset, temp);
        if (error) return;
        if (value) temp |= (1 << bit);
        else temp &= ~(1 << bit);
        stack->memory->set(index + input_offset, temp);
    }

    byte getOutput(u32 index) {
        if (stack == nullptr) return 0;
        if (stack->memory == nullptr) return 0;
        byte value;
        stack->memory->get(index + output_offset, value);
        return value;
    }

    bool getOutputBit(u32 index, u8 bit) {
        if (stack == nullptr) return false;
        if (stack->memory == nullptr) return false;
        byte temp = 0;
        bool error = stack->memory->get(index + output_offset, temp);
        if (error) return false;
        return temp & (1 << bit);
    }

    bool readMemory(u32 offset, u8*& value, u32 size = 1) {
        if (stack == nullptr) return false;
        if (stack->memory == nullptr) return false;
        return stack->memory->readArea(offset, value, size);
    }

    bool writeMemory(u32 offset, u8* value, u32 size = 1) {
        if (stack == nullptr) return false;
        if (stack->memory == nullptr) return false;
        return stack->memory->writeArea(offset, value, size);
    }

public:
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
                    Serial.flush();
                    return;
                }

                Serial.println(F("Complete"));
                Serial.flush();
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
                program->format(size);
                program->program_size = size;
                u8 b = 0;
                for (u32 i = 0; i < size; i++) {
                    b = serialReadHexByteTimeout(); SERIAL_TIMEOUT_JOB(processExit());
                    program->modify(i, b);
                    crc8_simple(checksum_calc, b);
                }

                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_JOB(processExit());

                // If the checksum is invalid, restart the runtime
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum, restarting the runtime..."));
                    Serial.flush();
                    delay(1000);
                    processExit();
                    return;
                }
                program->resetLine();

                Serial.flush();
                Serial.println(F("Complete"));
            } else if (program_upload) {
                Serial.print(F("PROGRAM UPLOAD - "));
                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;

                // Verify the checksum
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum"));
                    Serial.flush();
                    return;
                }

                // Print the program
                program->println();

                Serial.flush();
                Serial.println(F("Complete"));
            } else if (program_run) {
                Serial.print(F("PROGRAM RUN - "));
                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;

                // Verify the checksum
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum"));
                    Serial.flush();
                    return;
                }

                Serial.flush();
                Serial.println(F("Complete"));
            } else if (program_stop) {
                Serial.print(F("PROGRAM STOP - "));
                // Read the checksum
                checksum = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;

                // Verify the checksum
                if (checksum != checksum_calc) {
                    Serial.println(F("Invalid checksum"));
                    Serial.flush();
                    return;
                }

                Serial.flush();
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
                    Serial.flush();
                    return;
                }

                // Read the data
                u8 value;
                for (u32 i = 0; i < size; i++) {
                    stack->memory->get(address + i, value);
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
                Serial.flush();

                Serial.flush();
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
                    Serial.flush();
                    return;
                }

                // Write the data
                for (u32 i = 0; i < size; i++)
                    stack->memory->set(address + i, data[i]);

                Serial.flush();
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
                    Serial.flush();
                    return;
                }

                // Format the memory
                for (u32 i = 0; i < size; i++)
                    stack->memory->set(address + i, value);

                Serial.flush();
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
    if (program != nullptr) program->resetLine();
    stack->clear();
}
// Clear the runtime stack and reset the program line
void VovkPLCRuntime::clear(RuntimeProgram& program) {
    program.resetLine();
    stack->clear();
}

// Print the stack
int VovkPLCRuntime::printStack() { return stack->print(); }


// Execute the whole PLC program, returns an erro code (0 on success)
RuntimeError VovkPLCRuntime::run(RuntimeProgram& program) { return run(program.program, program.program_size); }

// Execute the whole PLC program, returns an erro code (0 on success)
RuntimeError VovkPLCRuntime::run(u8* program, u32 program_size) {
    if (!started_up) startup();
    IntervalGlobalLoopCheck();
    u8 state = 0;
    state = state << 1 | P_10s;
    state = state << 1 | P_5s;
    state = state << 1 | P_2s;
    state = state << 1 | P_1s;
    state = state << 1 | P_500ms;
    state = state << 1 | P_300ms;
    state = state << 1 | P_200ms;
    state = state << 1 | P_100ms;
    stack->memory->set(1, state); // u8 -> 1 byte
    state = 0;
    state = state << 1 | P_2hr;
    state = state << 1 | P_1hr;
    state = state << 1 | P_30min;
    state = state << 1 | P_10min;
    state = state << 1 | P_5min;
    state = state << 1 | P_2min;
    state = state << 1 | P_1min;
    state = state << 1 | P_30s;
    stack->memory->set(2, state); // u8 -> 1 byte
    state = 0;
    state = state << 1 | P_1day;
    state = state << 1 | P_12hr;
    state = state << 1 | P_6hr;
    state = state << 1 | P_5hr;
    state = state << 1 | P_4hr;
    state = state << 1 | P_3hr;
    stack->memory->set(3, state); // u8 -> 1 byte

    stack->memory->set(4, interval_time_days);
    stack->memory->set(5, interval_time_hours);
    stack->memory->set(6, interval_time_minutes);
    stack->memory->set(7, interval_time_seconds);
    u32 index = 0;
    while (index < program_size) {
        RuntimeError status = step(program, program_size, index);
        if (status != STATUS_SUCCESS) {
            if (status == PROGRAM_EXITED)
                return STATUS_SUCCESS;
            return status;
        }
    }
    return STATUS_SUCCESS;
}




// Execute one PLC instruction at index, returns an error code (0 on success)
RuntimeError VovkPLCRuntime::step(RuntimeProgram& program) { return step(program.program, program.program_size, program.program_line); }

// Execute one PLC instruction at index, returns an error code (0 on success)
RuntimeError VovkPLCRuntime::step(u8* program, u32 program_size, u32& index) {
    if (program_size == 0) return EMPTY_PROGRAM;
    if (index >= program_size) return PROGRAM_SIZE_EXCEEDED;
    u8 opcode = program[index];
    index++;
    switch (opcode) {
        case NOP: return STATUS_SUCCESS;
        case LOGIC_AND: return PLCMethods::LOGIC_AND(this->stack);
        case LOGIC_OR: return PLCMethods::LOGIC_OR(this->stack);
        case LOGIC_NOT: return PLCMethods::LOGIC_NOT(this->stack);
        case LOGIC_XOR: return PLCMethods::LOGIC_XOR(this->stack);
        case CVT: return PLCMethods::CVT(this->stack, program, program_size, index);
        case LOAD: return PLCMethods::LOAD(this->stack, program, program_size, index);
        case MOVE: return PLCMethods::MOVE(this->stack, program, program_size, index);
        case MOVE_COPY: return PLCMethods::MOVE_COPY(this->stack, program, program_size, index);
        case COPY: return PLCMethods::COPY(this->stack, program, program_size, index);
        case SWAP: return PLCMethods::SWAP(this->stack, program, program_size, index);
        case DROP: return PLCMethods::DROP(this->stack, program, program_size, index);
        case CLEAR: return PLCMethods::CLEAR(this->stack);
        case JMP: return PLCMethods::handle_JMP(this->stack, program, program_size, index);
        case JMP_IF: return PLCMethods::handle_JMP_IF(this->stack, program, program_size, index);
        case JMP_IF_NOT: return PLCMethods::handle_JMP_IF_NOT(this->stack, program, program_size, index);
        case CALL: return PLCMethods::handle_CALL(this->stack, program, program_size, index);
        case CALL_IF: return PLCMethods::handle_CALL_IF(this->stack, program, program_size, index);
        case CALL_IF_NOT: return PLCMethods::handle_CALL_IF_NOT(this->stack, program, program_size, index);
        case RET: return PLCMethods::handle_RET(this->stack, program, program_size, index);
        case RET_IF: return PLCMethods::handle_RET_IF(this->stack, program, program_size, index);
        case RET_IF_NOT: return PLCMethods::handle_RET_IF_NOT(this->stack, program, program_size, index);
        case type_pointer: return PLCMethods::PUSH_pointer(this->stack, program, program_size, index);
        case type_bool: return PLCMethods::PUSH_bool(this->stack, program, program_size, index);
        case type_u8: return PLCMethods::push_u8(this->stack, program, program_size, index);
        case type_i8: return PLCMethods::push_i8(this->stack, program, program_size, index);
        case type_u16: return PLCMethods::push_u16(this->stack, program, program_size, index);
        case type_i16: return PLCMethods::push_i16(this->stack, program, program_size, index);
        case type_u32: return PLCMethods::push_u32(this->stack, program, program_size, index);
        case type_i32: return PLCMethods::push_i32(this->stack, program, program_size, index);
        case type_f32: return PLCMethods::push_f32(this->stack, program, program_size, index);
#ifdef USE_X64_OPS
        case type_u64: return PLCMethods::push_u64(this->stack, program, program_size, index);
        case type_i64: return PLCMethods::push_i64(this->stack, program, program_size, index);
        case type_f64: return PLCMethods::push_f64(this->stack, program, program_size, index);
#endif // USE_X64_OPS
        case ADD: return PLCMethods::handle_ADD(this->stack, program, program_size, index);
        case SUB: return PLCMethods::handle_SUB(this->stack, program, program_size, index);
        case MUL: return PLCMethods::handle_MUL(this->stack, program, program_size, index);
        case DIV: return PLCMethods::handle_DIV(this->stack, program, program_size, index);

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
        case READ_X8_B0: return PLCMethods::handle_READ_X8_B0(this->stack, program, program_size, index);
        case READ_X8_B1: return PLCMethods::handle_READ_X8_B1(this->stack, program, program_size, index);
        case READ_X8_B2: return PLCMethods::handle_READ_X8_B2(this->stack, program, program_size, index);
        case READ_X8_B3: return PLCMethods::handle_READ_X8_B3(this->stack, program, program_size, index);
        case READ_X8_B4: return PLCMethods::handle_READ_X8_B4(this->stack, program, program_size, index);
        case READ_X8_B5: return PLCMethods::handle_READ_X8_B5(this->stack, program, program_size, index);
        case READ_X8_B6: return PLCMethods::handle_READ_X8_B6(this->stack, program, program_size, index);
        case READ_X8_B7: return PLCMethods::handle_READ_X8_B7(this->stack, program, program_size, index);
        case WRITE_X8_B0: return PLCMethods::handle_WRITE_X8_B0(this->stack, program, program_size, index);
        case WRITE_X8_B1: return PLCMethods::handle_WRITE_X8_B1(this->stack, program, program_size, index);
        case WRITE_X8_B2: return PLCMethods::handle_WRITE_X8_B2(this->stack, program, program_size, index);
        case WRITE_X8_B3: return PLCMethods::handle_WRITE_X8_B3(this->stack, program, program_size, index);
        case WRITE_X8_B4: return PLCMethods::handle_WRITE_X8_B4(this->stack, program, program_size, index);
        case WRITE_X8_B5: return PLCMethods::handle_WRITE_X8_B5(this->stack, program, program_size, index);
        case WRITE_X8_B6: return PLCMethods::handle_WRITE_X8_B6(this->stack, program, program_size, index);
        case WRITE_X8_B7: return PLCMethods::handle_WRITE_X8_B7(this->stack, program, program_size, index);
        case WRITE_S_X8_B0: return PLCMethods::handle_WRITE_S_X8_B0(this->stack, program, program_size, index);
        case WRITE_S_X8_B1: return PLCMethods::handle_WRITE_S_X8_B1(this->stack, program, program_size, index);
        case WRITE_S_X8_B2: return PLCMethods::handle_WRITE_S_X8_B2(this->stack, program, program_size, index);
        case WRITE_S_X8_B3: return PLCMethods::handle_WRITE_S_X8_B3(this->stack, program, program_size, index);
        case WRITE_S_X8_B4: return PLCMethods::handle_WRITE_S_X8_B4(this->stack, program, program_size, index);
        case WRITE_S_X8_B5: return PLCMethods::handle_WRITE_S_X8_B5(this->stack, program, program_size, index);
        case WRITE_S_X8_B6: return PLCMethods::handle_WRITE_S_X8_B6(this->stack, program, program_size, index);
        case WRITE_S_X8_B7: return PLCMethods::handle_WRITE_S_X8_B7(this->stack, program, program_size, index);
        case WRITE_R_X8_B0: return PLCMethods::handle_WRITE_R_X8_B0(this->stack, program, program_size, index);
        case WRITE_R_X8_B1: return PLCMethods::handle_WRITE_R_X8_B1(this->stack, program, program_size, index);
        case WRITE_R_X8_B2: return PLCMethods::handle_WRITE_R_X8_B2(this->stack, program, program_size, index);
        case WRITE_R_X8_B3: return PLCMethods::handle_WRITE_R_X8_B3(this->stack, program, program_size, index);
        case WRITE_R_X8_B4: return PLCMethods::handle_WRITE_R_X8_B4(this->stack, program, program_size, index);
        case WRITE_R_X8_B5: return PLCMethods::handle_WRITE_R_X8_B5(this->stack, program, program_size, index);
        case WRITE_R_X8_B6: return PLCMethods::handle_WRITE_R_X8_B6(this->stack, program, program_size, index);
        case WRITE_R_X8_B7: return PLCMethods::handle_WRITE_R_X8_B7(this->stack, program, program_size, index);
        case WRITE_INV_X8_B0: return PLCMethods::handle_WRITE_INV_X8_B0(this->stack, program, program_size, index);
        case WRITE_INV_X8_B1: return PLCMethods::handle_WRITE_INV_X8_B1(this->stack, program, program_size, index);
        case WRITE_INV_X8_B2: return PLCMethods::handle_WRITE_INV_X8_B2(this->stack, program, program_size, index);
        case WRITE_INV_X8_B3: return PLCMethods::handle_WRITE_INV_X8_B3(this->stack, program, program_size, index);
        case WRITE_INV_X8_B4: return PLCMethods::handle_WRITE_INV_X8_B4(this->stack, program, program_size, index);
        case WRITE_INV_X8_B5: return PLCMethods::handle_WRITE_INV_X8_B5(this->stack, program, program_size, index);
        case WRITE_INV_X8_B6: return PLCMethods::handle_WRITE_INV_X8_B6(this->stack, program, program_size, index);
        case WRITE_INV_X8_B7: return PLCMethods::handle_WRITE_INV_X8_B7(this->stack, program, program_size, index);

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
        case CMP_EQ: return PLCMethods::handle_CMP_EQ(this->stack, program, program_size, index);
        case CMP_NEQ: return PLCMethods::handle_CMP_NEQ(this->stack, program, program_size, index);
        case CMP_GT: return PLCMethods::handle_CMP_GT(this->stack, program, program_size, index);
        case CMP_GTE: return PLCMethods::handle_CMP_GTE(this->stack, program, program_size, index);
        case CMP_LT: return PLCMethods::handle_CMP_LT(this->stack, program, program_size, index);
        case CMP_LTE: return PLCMethods::handle_CMP_LTE(this->stack, program, program_size, index);
        case EXIT: {
            return PROGRAM_EXITED;
            // return PLCMethods::handle_EXIT(this->stack, program, program_size, index);
        }
        default: return UNKNOWN_INSTRUCTION;
    }
}