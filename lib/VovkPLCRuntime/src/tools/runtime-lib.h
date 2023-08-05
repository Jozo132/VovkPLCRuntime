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
#include "arithmetics/crc8.h"
#include "stack/stack-struct.h"
#include "runtime-memory.h"
#include "runtime-instructions.h"
#include "stack/runtime-stack.h"
#include "arithmetics/runtime-arithmetics.h"
#include "runtime-program.h"
#include "runtime-cmd-parser.h"

#define SERIAL_TIMEOUT_RETURN if (serial_timeout) return;
#define SERIAL_TIMEOUT_JOB(task) if (serial_timeout) { Serial.flush(); task; return; };

class VovkPLCRuntime {
private:
    bool started_up = false;
public:
    const uint32_t input_offset = PLCRUNTIME_INPUT_OFFSET; // Output offset in memory
    const uint32_t output_offset = PLCRUNTIME_OUTPUT_OFFSET + PLCRUNTIME_INPUT_OFFSET; // Output offset in memory
    uint32_t max_stack_size = PLCRUNTIME_DEFAULT_STACK_SIZE; // Maximum stack size
    uint32_t memory_size = PLCRUNTIME_DEFAULT_MEMORY_SIZE; // PLC memory size
    uint32_t program_size = PLCRUNTIME_DEFAULT_PROGRAM_SIZE; // PLC program size
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

    void formatMemory(uint32_t size, uint8_t* data = nullptr) {
        if (size == 0) return;
        if (stack == nullptr) stack = new RuntimeStack(max_stack_size, max_stack_size, memory_size);
        stack->memory->format(size);
        if (data == nullptr) {
            for (uint32_t i = 0; i < size; i++)
                stack->memory->set(i, 0);
        } else {
            for (uint32_t i = 0; i < size; i++)
                stack->memory->set(i, data[i]);
        }
    }

    VovkPLCRuntime(uint32_t max_stack_size, uint32_t memory_size, uint32_t program_size) {
        this->max_stack_size = max_stack_size;
        this->memory_size = memory_size;
        this->program_size = program_size;
    }
    VovkPLCRuntime(uint32_t max_stack_size, uint32_t memory_size) {
        this->max_stack_size = max_stack_size;
        this->memory_size = memory_size;
    }
    VovkPLCRuntime(uint32_t max_stack_size, RuntimeProgram& program) {
        this->max_stack_size = max_stack_size;
        this->program = &program;
    }
    VovkPLCRuntime(uint32_t max_stack_size, uint32_t memory_size, RuntimeProgram& program) {
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

    void loadProgramUnsafe(uint8_t* program, uint32_t program_size) {
        if (this->program == nullptr) this->program = new RuntimeProgram(program_size);
        this->program->loadUnsafe(program, program_size);
    }

    void loadProgram(uint8_t* program, uint32_t program_size, uint8_t checksum) {
        if (this->program == nullptr) this->program = new RuntimeProgram(program_size);
        this->program->load(program, program_size, checksum);
    }

    // Clear the stack
    void clear();
    // Clear the stack and reset the program line
    void clear(RuntimeProgram& program);
    // Execute one PLC instruction at index, returns an error code (0 on success)
    RuntimeError step(uint8_t* program, uint32_t program_size, uint32_t& index);
    // Execute the whole PLC program, returns an error code (0 on success)
    RuntimeError run(uint8_t* program, uint32_t program_size);
    // Execute one PLC instruction, returns an error code (0 on success)
    RuntimeError step(RuntimeProgram& program);
    // Run/Continue the whole PLC program from where it left off, returns an error code (0 on success)
    RuntimeError run(RuntimeProgram& program);
    // Execute one PLC instruction at index, returns an error code (0 on success)
    RuntimeError step(uint32_t& index) {
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
        if (stack->size() < (uint32_t) sizeof(T)) return 0;
        uint8_t temp[sizeof(T)];
        for (uint32_t i = 0; i < (uint32_t) sizeof(T); i++)
            temp[i] = stack->pop();
        T value = *(T*) temp;
        return value;
    }

    // Print the stack buffer to the serial port
    int printStack();

    void setInput(uint32_t index, byte value) {
        if (stack == nullptr) return;
        if (stack->memory == nullptr) return;
        stack->memory->set(index + input_offset, value);
    }

    void setInputBit(uint32_t index, uint8_t bit, bool value) {
        if (stack == nullptr) return;
        if (stack->memory == nullptr) return;
        byte temp = 0;
        bool error = stack->memory->get(index + input_offset, temp);
        if (error) return;
        if (value) temp |= (1 << bit);
        else temp &= ~(1 << bit);
        stack->memory->set(index + input_offset, temp);
    }

    byte getOutput(uint32_t index) {
        if (stack == nullptr) return 0;
        if (stack->memory == nullptr) return 0;
        byte value;
        stack->memory->get(index + output_offset, value);
        return value;
    }

    bool getOutputBit(uint32_t index, uint8_t bit) {
        if (stack == nullptr) return false;
        if (stack->memory == nullptr) return false;
        byte temp = 0;
        bool error = stack->memory->get(index + output_offset, temp);
        if (error) return false;
        return temp & (1 << bit);
    }

    bool readMemory(uint32_t offset, uint8_t*& value, uint32_t size = 1) {
        if (stack == nullptr) return false;
        if (stack->memory == nullptr) return false;
        bool error = false;
        uint8_t temp = 0;
        for (uint32_t i = 0; i < size; i++) {
            error = stack->memory->get(offset + i, temp);
            value[i] = temp;
            if (error) return error;
        }
        return false;
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
            //  - PLC reset:        'RS<uint8_t>' (checksum)
            //  - Program download: 'PD<uint32_t><uint8_t[]><uint8_t>' (size, data, checksum)
            //  - Program upload:   'PU<uint8_t>' (checksum)
            //  - Program run:      'PR<uint8_t>' (checksum)
            //  - Program stop:     'PS<uint8_t>' (checksum)
            //  - Memory read:      'MR<uint32_t><uint32_t><uint8_t>' (address, size, checksum)
            //  - Memory write:     'MW<uint32_t><uint32_t><uint8_t[]><uint8_t>' (address, size, data, checksum)
            //  - Memory format:    'MF<uint32_t><uint32_t><uint8_t><uint8_t>' (address, size, value, checksum)
            //  - Source download:  'SD<uint32_t><uint8_t[]><uint8_t>' (size, data, checksum) // Only available if PLCRUNTIME_SOURCE_ENABLED is defined
            //  - Source upload:    'SU<uint32_t><uint8_t>' (size, checksum) // Only available if PLCRUNTIME_SOURCE_ENABLED is defined
            // If the program is downloaded and the checksum is invalid, the runtime will restart
            uint8_t cmd[2] = { 0, 0 };
            uint32_t size = 0;
            uint32_t address = 0;
            uint8_t* data = nullptr;
            uint8_t checksum = 0;
            uint8_t checksum_calc = 0;

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
                size = (uint32_t) serialReadHexByteTimeout();  SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));


                // Read the data
                program->format(size);
                program->program_size = size;
                uint8_t b = 0;
                for (uint32_t i = 0; i < size; i++) {
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
                address = (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));

                // Read the size
                size = (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
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
                uint8_t value;
                for (uint32_t i = 0; i < size; i++) {
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
                address = (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));

                // Read the size
                size = (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));

                // Read the data
                data = new uint8_t[size];
                for (uint32_t i = 0; i < size; i++) {
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
                for (uint32_t i = 0; i < size; i++)
                    stack->memory->set(address + i, data[i]);

                Serial.flush();
                Serial.println(F("Complete"));
            } else if (memory_format) {
                Serial.print(F("MEMORY FORMAT - "));
                // Read the address
                address = (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));
                address = address << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (address & 0xff));

                // Read the size
                size = (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));
                size = size << 8 | (uint32_t) serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
                crc8_simple(checksum_calc, (size & 0xff));

                // Read the value
                uint8_t value = serialReadHexByteTimeout(); SERIAL_TIMEOUT_RETURN;
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
                for (uint32_t i = 0; i < size; i++)
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
RuntimeError VovkPLCRuntime::run(uint8_t* program, uint32_t program_size) {
    if (!started_up) startup();
    uint32_t index = 0;
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
RuntimeError VovkPLCRuntime::step(uint8_t* program, uint32_t program_size, uint32_t& index) {
    if (program_size == 0) return EMPTY_PROGRAM;
    if (index >= program_size) return PROGRAM_SIZE_EXCEEDED;
    uint8_t opcode = program[index];
    index++;
    switch (opcode) {
        case NOP: return STATUS_SUCCESS;
        case LOGIC_AND: return PLCMethods::LOGIC_AND(this->stack);
        case LOGIC_OR: return PLCMethods::LOGIC_OR(this->stack);
        case LOGIC_NOT: return PLCMethods::LOGIC_NOT(this->stack);
        case LOGIC_XOR: return PLCMethods::LOGIC_XOR(this->stack);
        case CVT: return PLCMethods::CVT(this->stack, program, program_size, index);
        case PUT: return PLCMethods::PUT(this->stack, program, program_size, index);
        case GET: return PLCMethods::GET(this->stack, program, program_size, index);
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
        case type_bool: return PLCMethods::PUSH_bool(this->stack, program, program_size, index);
        case type_uint8_t: return PLCMethods::PUSH_uint8_t(this->stack, program, program_size, index);
        case type_int8_t: return PLCMethods::PUSH_int8_t(this->stack, program, program_size, index);
        case type_uint16_t: return PLCMethods::PUSH_uint16_t(this->stack, program, program_size, index);
        case type_int16_t: return PLCMethods::PUSH_int16_t(this->stack, program, program_size, index);
        case type_uint32_t: return PLCMethods::PUSH_uint32_t(this->stack, program, program_size, index);
        case type_int32_t: return PLCMethods::PUSH_int32_t(this->stack, program, program_size, index);
        case type_float: return PLCMethods::PUSH_float(this->stack, program, program_size, index);
#ifdef USE_X64_OPS
        case type_uint64_t: return PLCMethods::PUSH_uint64_t(this->stack, program, program_size, index);
        case type_int64_t: return PLCMethods::PUSH_int64_t(this->stack, program, program_size, index);
        case type_double: return PLCMethods::PUSH_double(this->stack, program, program_size, index);
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