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

    RuntimeError PUSH_bool(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        bool value = 0;
        extract_status = ProgramExtract.type_bool(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_bool(value);
    }
    RuntimeError PUSH_uint8_t(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        uint8_t value = 0;
        extract_status = ProgramExtract.type_uint8_t(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_uint8_t(value);
    }
    RuntimeError PUSH_uint16_t(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        uint16_t value = 0;
        extract_status = ProgramExtract.type_uint16_t(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_uint16_t(value);
    }
    RuntimeError PUSH_uint32_t(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        uint32_t value = 0;
        extract_status = ProgramExtract.type_uint32_t(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_uint32_t(value);
    }
#ifdef USE_X64_OPS
    RuntimeError PUSH_uint64_t(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        uint64_t value = 0;
        extract_status = ProgramExtract.type_uint64_t(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_uint64_t(value);
    }
#endif // USE_X64_OPS
    RuntimeError PUSH_int8_t(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        int8_t value = 0;
        extract_status = ProgramExtract.type_int8_t(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_int8_t(value);
    }
    RuntimeError PUSH_int16_t(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        int16_t value = 0;
        extract_status = ProgramExtract.type_int16_t(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_int16_t(value);
    }
    RuntimeError PUSH_int32_t(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        int32_t value = 0;
        extract_status = ProgramExtract.type_int32_t(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_int32_t(value);
    }

#ifdef USE_X64_OPS
    RuntimeError PUSH_int64_t(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        int64_t value = 0;
        extract_status = ProgramExtract.type_int64_t(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_int64_t(value);
    }
#endif // USE_X64_OPS
    RuntimeError PUSH_float(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        float value = 0;
        extract_status = ProgramExtract.type_float(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_float(value);
    }

#ifdef USE_X64_OPS
    RuntimeError PUSH_double(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        double value = 0;
        extract_status = ProgramExtract.type_double(program, program_size, index, &value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_double(value);
    }
#endif // USE_X64_OPS

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

    // Convert value from one type to another ()
    RuntimeError CVT(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        uint32_t size = 2;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t from_type = 0;
        uint8_t to_type = 0;
        extract_status = ProgramExtract.type_uint8_t(program, program_size, index, &from_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        extract_status = ProgramExtract.type_uint8_t(program, program_size, index, &to_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        switch (from_type) {
            case type_bool:
                switch (to_type) {
                    case type_bool: return stack->push_bool(stack->pop_bool());
                    case type_uint8_t: return stack->push_uint8_t(stack->pop_bool());
                    case type_uint16_t: return stack->push_uint16_t(stack->pop_bool());
                    case type_uint32_t: return stack->push_uint32_t(stack->pop_bool());
                    case type_uint64_t: return stack->push_uint64_t(stack->pop_bool());
                    case type_int8_t: return stack->push_int8_t(stack->pop_bool());
                    case type_int16_t: return stack->push_int16_t(stack->pop_bool());
                    case type_int32_t: return stack->push_int32_t(stack->pop_bool());
                    case type_int64_t: return stack->push_int64_t(stack->pop_bool());
                    case type_float: return stack->push_float(stack->pop_bool());
                    case type_double: return stack->push_double(stack->pop_bool());
                    default: return INVALID_DATA_TYPE;
                }
            case type_uint8_t:
                switch (to_type) {
                    case type_bool: return stack->push_bool(stack->pop_uint8_t());
                    case type_uint8_t: return stack->push_uint8_t(stack->pop_uint8_t());
                    case type_uint16_t: return stack->push_uint16_t(stack->pop_uint8_t());
                    case type_uint32_t: return stack->push_uint32_t(stack->pop_uint8_t());
                    case type_int8_t: return stack->push_int8_t(stack->pop_uint8_t());
                    case type_int16_t: return stack->push_int16_t(stack->pop_uint8_t());
                    case type_int32_t: return stack->push_int32_t(stack->pop_uint8_t());
                    case type_float: return stack->push_float(stack->pop_uint8_t());
#ifdef USE_X64_OPS
                    case type_uint64_t: return stack->push_uint64_t(stack->pop_uint8_t());
                    case type_int64_t: return stack->push_int64_t(stack->pop_uint8_t());
                    case type_double: return stack->push_double(stack->pop_uint8_t());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
            case type_uint16_t:
                switch (to_type) {
                    case type_bool: return stack->push_bool(stack->pop_uint16_t());
                    case type_uint8_t: return stack->push_uint8_t(stack->pop_uint16_t());
                    case type_uint16_t: return stack->push_uint16_t(stack->pop_uint16_t());
                    case type_uint32_t: return stack->push_uint32_t(stack->pop_uint16_t());
                    case type_int8_t: return stack->push_int8_t(stack->pop_uint16_t());
                    case type_int16_t: return stack->push_int16_t(stack->pop_uint16_t());
                    case type_int32_t: return stack->push_int32_t(stack->pop_uint16_t());
                    case type_float: return stack->push_float(stack->pop_uint16_t());
#ifdef USE_X64_OPS
                    case type_uint64_t: return stack->push_uint64_t(stack->pop_uint16_t());
                    case type_int64_t: return stack->push_int64_t(stack->pop_uint16_t());
                    case type_double: return stack->push_double(stack->pop_uint16_t());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
            case type_uint32_t:
                switch (to_type) {
                    case type_bool: return stack->push_bool(stack->pop_uint32_t());
                    case type_uint8_t: return stack->push_uint8_t(stack->pop_uint32_t());
                    case type_uint16_t: return stack->push_uint16_t(stack->pop_uint32_t());
                    case type_uint32_t: return stack->push_uint32_t(stack->pop_uint32_t());
                    case type_int8_t: return stack->push_int8_t(stack->pop_uint32_t());
                    case type_int16_t: return stack->push_int16_t(stack->pop_uint32_t());
                    case type_int32_t: return stack->push_int32_t(stack->pop_uint32_t());
                    case type_float: return stack->push_float(stack->pop_uint32_t());
#ifdef USE_X64_OPS
                    case type_uint64_t: return stack->push_uint64_t(stack->pop_uint32_t());
                    case type_int64_t: return stack->push_int64_t(stack->pop_uint32_t());
                    case type_double: return stack->push_double(stack->pop_uint32_t());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }

#ifdef USE_X64_OPS
            case type_uint64_t:
                switch (to_type) {
                    case type_bool: return stack->push_bool(stack->pop_uint64_t());
                    case type_uint8_t: return stack->push_uint8_t(stack->pop_uint64_t());
                    case type_uint16_t: return stack->push_uint16_t(stack->pop_uint64_t());
                    case type_uint32_t: return stack->push_uint32_t(stack->pop_uint64_t());
                    case type_uint64_t: return stack->push_uint64_t(stack->pop_uint64_t());
                    case type_int8_t: return stack->push_int8_t(stack->pop_uint64_t());
                    case type_int16_t: return stack->push_int16_t(stack->pop_uint64_t());
                    case type_int32_t: return stack->push_int32_t(stack->pop_uint64_t());
                    case type_int64_t: return stack->push_int64_t(stack->pop_uint64_t());
                    case type_float: return stack->push_float(stack->pop_uint64_t());
                    case type_double: return stack->push_double(stack->pop_uint64_t());
                    default: return INVALID_DATA_TYPE;
                }
#endif // USE_X64_OPS
            case type_int8_t:
                switch (to_type) {
                    case type_bool: return stack->push_bool(stack->pop_int8_t());
                    case type_uint8_t: return stack->push_uint8_t(stack->pop_int8_t());
                    case type_uint16_t: return stack->push_uint16_t(stack->pop_int8_t());
                    case type_uint32_t: return stack->push_uint32_t(stack->pop_int8_t());
                    case type_int8_t: return stack->push_int8_t(stack->pop_int8_t());
                    case type_int16_t: return stack->push_int16_t(stack->pop_int8_t());
                    case type_int32_t: return stack->push_int32_t(stack->pop_int8_t());
                    case type_float: return stack->push_float(stack->pop_int8_t());
#ifdef USE_X64_OPS
                    case type_uint64_t: return stack->push_uint64_t(stack->pop_int8_t());
                    case type_int64_t: return stack->push_int64_t(stack->pop_int8_t());
                    case type_double: return stack->push_double(stack->pop_int8_t());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
            case type_int16_t:
                switch (to_type) {
                    case type_bool: return stack->push_bool(stack->pop_int16_t());
                    case type_uint8_t: return stack->push_uint8_t(stack->pop_int16_t());
                    case type_uint16_t: return stack->push_uint16_t(stack->pop_int16_t());
                    case type_uint32_t: return stack->push_uint32_t(stack->pop_int16_t());
                    case type_int8_t: return stack->push_int8_t(stack->pop_int16_t());
                    case type_int16_t: return stack->push_int16_t(stack->pop_int16_t());
                    case type_int32_t: return stack->push_int32_t(stack->pop_int16_t());
                    case type_float: return stack->push_float(stack->pop_int16_t());
#ifdef USE_X64_OPS
                    case type_uint64_t: return stack->push_uint64_t(stack->pop_int16_t());
                    case type_int64_t: return stack->push_int64_t(stack->pop_int16_t());
                    case type_double: return stack->push_double(stack->pop_int16_t());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
            case type_int32_t:
                switch (to_type) {
                    case type_bool: return stack->push_bool(stack->pop_int32_t());
                    case type_uint8_t: return stack->push_uint8_t(stack->pop_int32_t());
                    case type_uint16_t: return stack->push_uint16_t(stack->pop_int32_t());
                    case type_uint32_t: return stack->push_uint32_t(stack->pop_int32_t());
                    case type_int8_t: return stack->push_int8_t(stack->pop_int32_t());
                    case type_int16_t: return stack->push_int16_t(stack->pop_int32_t());
                    case type_int32_t: return stack->push_int32_t(stack->pop_int32_t());
                    case type_float: return stack->push_float(stack->pop_int32_t());
#ifdef USE_X64_OPS
                    case type_uint64_t: return stack->push_uint64_t(stack->pop_int32_t());
                    case type_int64_t: return stack->push_int64_t(stack->pop_int32_t());
                    case type_double: return stack->push_double(stack->pop_int32_t());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
#ifdef USE_X64_OPS
            case type_int64_t:
                switch (to_type) {
                    case type_bool: return stack->push_bool(stack->pop_int64_t());
                    case type_uint8_t: return stack->push_uint8_t(stack->pop_int64_t());
                    case type_uint16_t: return stack->push_uint16_t(stack->pop_int64_t());
                    case type_uint32_t: return stack->push_uint32_t(stack->pop_int64_t());
                    case type_uint64_t: return stack->push_uint64_t(stack->pop_int64_t());
                    case type_int8_t: return stack->push_int8_t(stack->pop_int64_t());
                    case type_int16_t: return stack->push_int16_t(stack->pop_int64_t());
                    case type_int32_t: return stack->push_int32_t(stack->pop_int64_t());
                    case type_int64_t: return stack->push_int64_t(stack->pop_int64_t());
                    case type_float: return stack->push_float(stack->pop_int64_t());
                    case type_double: return stack->push_double(stack->pop_int64_t());
                    default: return INVALID_DATA_TYPE;
                }
#endif // USE_X64_OPS
            case type_float:
                switch (to_type) {
                    case type_bool: return stack->push_bool((int) stack->pop_float());
                    case type_uint8_t: return stack->push_uint8_t((int) stack->pop_float());
                    case type_uint16_t: return stack->push_uint16_t(stack->pop_float());
                    case type_uint32_t: return stack->push_uint32_t(stack->pop_float());
                    case type_int8_t: return stack->push_int8_t(stack->pop_float());
                    case type_int16_t: return stack->push_int16_t(stack->pop_float());
                    case type_int32_t: return stack->push_int32_t(stack->pop_float());
                    case type_float: return stack->push_float(stack->pop_float());
#ifdef USE_X64_OPS
                    case type_uint64_t: return stack->push_uint64_t(stack->pop_float());
                    case type_int64_t: return stack->push_int64_t(stack->pop_float());
                    case type_double: return stack->push_double(stack->pop_float());
#endif // USE_X64_OPS
                    default: return INVALID_DATA_TYPE;
                }
#ifdef USE_X64_OPS
            case type_double:
                switch (to_type) {
                    case type_bool: return stack->push_bool((int) stack->pop_double());
                    case type_uint8_t: return stack->push_uint8_t((int) stack->pop_double());
                    case type_uint16_t: return stack->push_uint16_t(stack->pop_double());
                    case type_uint32_t: return stack->push_uint32_t(stack->pop_double());
                    case type_uint64_t: return stack->push_uint64_t(stack->pop_double());
                    case type_int8_t: return stack->push_int8_t(stack->pop_double());
                    case type_int16_t: return stack->push_int16_t(stack->pop_double());
                    case type_int32_t: return stack->push_int32_t(stack->pop_double());
                    case type_int64_t: return stack->push_int64_t(stack->pop_double());
                    case type_float: return stack->push_float(stack->pop_double());
                    case type_double: return stack->push_double(stack->pop_double());
                    default: return INVALID_DATA_TYPE;
                }
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }

    // Put value from stack into memory
    RuntimeError PUT(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        uint32_t size = 3;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = 0;
        uint16_t address = 0;
        extract_status = ProgramExtract.type_uint8_t(program, program_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        extract_status = ProgramExtract.type_uint16_t(program, program_size, index, &address);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        uint8_t value = stack->pop_uint8_t();
        bool error = stack->memory->set(address, value);
        if (error) {
#ifdef __RUNTIME_DEBUG__
            Serial.print(F("PUT: Memory access error. Max memory size is "));
            Serial.print(stack->memory->size());
            Serial.print(F(" but target address is "));
            Serial.println(address);
#endif
            return MEMORY_ACCESS_ERROR;
        }
        return STATUS_SUCCESS;
    }

    // Get value from memory and put it on stack
    RuntimeError GET(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        uint32_t size = 3;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = 0;
        uint16_t address = 0;
        extract_status = ProgramExtract.type_uint8_t(program, program_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        extract_status = ProgramExtract.type_uint16_t(program, program_size, index, &address);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        uint8_t value = 0;
        bool error = stack->memory->get(address, value);
        if (error) {
#ifdef __RUNTIME_DEBUG__
            Serial.print(F("GET: Memory access error. Max memory size is "));
            Serial.print(stack->memory->size());
            Serial.print(F(" but target address is "));
            Serial.println(address);
#endif
            return MEMORY_ACCESS_ERROR;
        }
        return stack->push_uint8_t(value);
    }

    // Duplicate the value on top of the stack
    RuntimeError COPY(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        uint32_t size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = 0;
        extract_status = ProgramExtract.type_uint8_t(program, program_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        switch (data_type) {
            case type_bool: return stack->push_bool(stack->peek_bool());
            case type_uint8_t: return stack->push_uint8_t(stack->peek_uint8_t());
            case type_uint16_t: return stack->push_uint16_t(stack->peek_uint16_t());
            case type_uint32_t: return stack->push_uint32_t(stack->peek_uint32_t());
            case type_int8_t: return stack->push_int8_t(stack->peek_int8_t());
            case type_int16_t: return stack->push_int16_t(stack->peek_int16_t());
            case type_int32_t: return stack->push_int32_t(stack->peek_int32_t());
            case type_float: return stack->push_float(stack->peek_float());
#ifdef USE_X64_OPS
            case type_uint64_t: return stack->push_uint64_t(stack->peek_uint64_t());
            case type_int64_t: return stack->push_int64_t(stack->peek_int64_t());
            case type_double: return stack->push_double(stack->peek_double());
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }
    // Swap the two values on top of the stack
    RuntimeError SWAP(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        uint32_t size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = 0;
        extract_status = ProgramExtract.type_uint8_t(program, program_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        switch (data_type) {
            case type_bool: {
                bool a = stack->pop_bool();
                bool b = stack->pop_bool();
                stack->push_bool(a);
                stack->push_bool(b);
                return STATUS_SUCCESS;
            }
            case type_uint8_t: {
                uint8_t a = stack->pop_uint8_t();
                uint8_t b = stack->pop_uint8_t();
                stack->push_uint8_t(a);
                stack->push_uint8_t(b);
                return STATUS_SUCCESS;
            }
            case type_uint16_t: {
                uint16_t a = stack->pop_uint16_t();
                uint16_t b = stack->pop_uint16_t();
                stack->push_uint16_t(a);
                stack->push_uint16_t(b);
                return STATUS_SUCCESS;
            }
            case type_uint32_t: {
                uint32_t a = stack->pop_uint32_t();
                uint32_t b = stack->pop_uint32_t();
                stack->push_uint32_t(a);
                stack->push_uint32_t(b);
                return STATUS_SUCCESS;
            }
#ifdef USE_X64_OPS
            case type_uint64_t: {
                uint64_t a = stack->pop_uint64_t();
                uint64_t b = stack->pop_uint64_t();
                stack->push_uint64_t(a);
                stack->push_uint64_t(b);
                return STATUS_SUCCESS;
            }
#endif // USE_X64_OPS
            case type_int8_t: {
                int8_t a = stack->pop_int8_t();
                int8_t b = stack->pop_int8_t();
                stack->push_int8_t(a);
                stack->push_int8_t(b);
                return STATUS_SUCCESS;
            }
            case type_int16_t: {
                int16_t a = stack->pop_int16_t();
                int16_t b = stack->pop_int16_t();
                stack->push_int16_t(a);
                stack->push_int16_t(b);
                return STATUS_SUCCESS;
            }
            case type_int32_t: {
                int32_t a = stack->pop_int32_t();
                int32_t b = stack->pop_int32_t();
                stack->push_int32_t(a);
                stack->push_int32_t(b);
                return STATUS_SUCCESS;
            }
#ifdef USE_X64_OPS
            case type_int64_t: {
                int64_t a = stack->pop_int64_t();
                int64_t b = stack->pop_int64_t();
                stack->push_int64_t(a);
                stack->push_int64_t(b);
                return STATUS_SUCCESS;
            }
#endif // USE_X64_OPS
            case type_float: {
                float a = stack->pop_float();
                float b = stack->pop_float();
                stack->push_float(a);
                stack->push_float(b);
                return STATUS_SUCCESS;
            }
#ifdef USE_X64_OPS
            case type_double: {
                double a = stack->pop_double();
                double b = stack->pop_double();
                stack->push_double(a);
                stack->push_double(b);
                return STATUS_SUCCESS;
            }
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }
    // Drop the value on top of the stack
    RuntimeError DROP(RuntimeStack* stack, uint8_t* program, uint32_t program_size, uint32_t& index) {
        uint32_t size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = 0;
        extract_status = ProgramExtract.type_uint8_t(program, program_size, index, &data_type);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        switch (data_type) {
            case type_bool: stack->pop_bool(); break;
            case type_uint8_t: stack->pop_uint8_t(); break;
            case type_uint16_t: stack->pop_uint16_t(); break;
            case type_uint32_t: stack->pop_uint32_t(); break;
            case type_int8_t: stack->pop_int8_t(); break;
            case type_int16_t: stack->pop_int16_t(); break;
            case type_int32_t: stack->pop_int32_t(); break;
            case type_float: stack->pop_float(); break;
#ifdef USE_X64_OPS
            case type_uint64_t: stack->pop_uint64_t(); break;
            case type_int64_t: stack->pop_int64_t(); break;
            case type_double: stack->pop_double(); break;
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
        return STATUS_SUCCESS;
    }
    // Clear the stack
    RuntimeError CLEAR(RuntimeStack* stack) {
        stack->clear();
        return STATUS_SUCCESS;
    }
}