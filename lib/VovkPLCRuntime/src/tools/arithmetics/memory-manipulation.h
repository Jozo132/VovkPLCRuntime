// memory-manipulation.h - 1.0.0 - 2022-12-11
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

namespace PLCMethods {

    RuntimeError PUSH_bool(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        bool value = 0;
        extract_status = ProgramExtract.type_bool(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_bool(value);
    }
    RuntimeError PUSH_uint8_t(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        uint8_t value = 0;
        extract_status = ProgramExtract.type_uint8_t(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_uint8_t(value);
    }
    RuntimeError PUSH_uint16_t(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        uint16_t value = 0;
        extract_status = ProgramExtract.type_uint16_t(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_uint16_t(value);
    }
    RuntimeError PUSH_uint32_t(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        uint32_t value = 0;
        extract_status = ProgramExtract.type_uint32_t(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_uint32_t(value);
    }
    RuntimeError PUSH_uint64_t(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        uint64_t value = 0;
        extract_status = ProgramExtract.type_uint64_t(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_uint64_t(value);
    }
    RuntimeError PUSH_int8_t(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        int8_t value = 0;
        extract_status = ProgramExtract.type_int8_t(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_int8_t(value);
    }
    RuntimeError PUSH_int16_t(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        int16_t value = 0;
        extract_status = ProgramExtract.type_int16_t(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_int16_t(value);
    }
    RuntimeError PUSH_int32_t(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        int32_t value = 0;
        extract_status = ProgramExtract.type_int32_t(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_int32_t(value);
    }
    RuntimeError PUSH_int64_t(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        int64_t value = 0;
        extract_status = ProgramExtract.type_int64_t(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_int64_t(value);
    }
    RuntimeError PUSH_float(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        float value = 0;
        extract_status = ProgramExtract.type_float(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_float(value);
    }
    RuntimeError PUSH_double(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        double value = 0;
        extract_status = ProgramExtract.type_double(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_double(value);
    }

    /*
    uint8_t data_type = program[index++];
            switch (data_type) {
                case type_bool:
                case type_uint8_t: return ADD_uint8_t(stack);
                case type_uint16_t: return ADD_uint16_t(stack);
                case type_uint32_t: return ADD_uint32_t(stack);
                case type_uint64_t: return ADD_uint64_t(stack);
                case type_int8_t: return ADD_int8_t(stack);
                case type_int16_t: return ADD_int16_t(stack);
                case type_int32_t: return ADD_int32_t(stack);
                case type_int64_t: return ADD_int64_t(stack);
                case type_float: return ADD_float(stack);
                case type_double: return ADD_double(stack);
                default: return INVALID_DATA_TYPE;
            }
    */

    // Put value from stack into memory
    RuntimeError PUT(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        uint16_t size = 3;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = 0;
        uint16_t address = 0;
        extract_status = ProgramExtract.type_uint8_t(program, program_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        extract_status = ProgramExtract.type_uint16_t(program, program_size, index, &address);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        uint8_t value = stack->pop_uint8_t();
        bool error = stack->memory.set(address, value);
        if (error) {
#ifdef __RUNTIME_DEBUG__
            Serial.print(F("PUT: Memory access error. Max memory size is "));
            Serial.print(stack->memory.size());
            Serial.print(F(" but target address is "));
            Serial.println(address);
#endif
            return MEMORY_ACCESS_ERROR;
        }
        return STATUS_SUCCESS;
    }

    // Get value from memory and put it on stack
    RuntimeError GET(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        uint16_t size = 3;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = 0;
        uint16_t address = 0;
        extract_status = ProgramExtract.type_uint8_t(program, program_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        extract_status = ProgramExtract.type_uint16_t(program, program_size, index, &address);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        uint8_t value = 0;
        bool error = stack->memory.get(address, value);
        if (error) {
#ifdef __RUNTIME_DEBUG__
            Serial.print(F("GET: Memory access error. Max memory size is "));
            Serial.print(stack->memory.size());
            Serial.print(F(" but target address is "));
            Serial.println(address);
#endif
            return MEMORY_ACCESS_ERROR;
        }
        return stack->push_uint8_t(value);
    }
}