// runtime-arithmetics.h - 1.0.0 - 2022-12-11
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



struct PLCMethods_t {

    RuntimeError PUSH_bool(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        bool value = 0;
        RuntimeError extract_status = Extract.type_bool(program, program_size, index, value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_bool(value);
    }
    RuntimeError PUSH_uint8_t(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        uint8_t value = 0;
        RuntimeError extract_status = Extract.type_uint8_t(program, program_size, index, value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_uint8_t(value);
    }
    RuntimeError PUSH_uint16_t(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        uint16_t value = 0;
        RuntimeError extract_status = Extract.type_uint16_t(program, program_size, index, value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_uint16_t(value);
    }
    RuntimeError PUSH_uint32_t(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        uint32_t value = 0;
        RuntimeError extract_status = Extract.type_uint32_t(program, program_size, index, value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_uint32_t(value);
    }
    RuntimeError PUSH_uint64_t(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        uint64_t value = 0;
        RuntimeError extract_status = Extract.type_uint64_t(program, program_size, index, value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_uint64_t(value);
    }
    RuntimeError PUSH_int8_t(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        int8_t value = 0;
        RuntimeError extract_status = Extract.type_int8_t(program, program_size, index, value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_int8_t(value);
    }
    RuntimeError PUSH_int16_t(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        int16_t value = 0;
        RuntimeError extract_status = Extract.type_int16_t(program, program_size, index, value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_int16_t(value);
    }
    RuntimeError PUSH_int32_t(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        int32_t value = 0;
        RuntimeError extract_status = Extract.type_int32_t(program, program_size, index, value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_int32_t(value);
    }
    RuntimeError PUSH_int64_t(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        int64_t value = 0;
        RuntimeError extract_status = Extract.type_int64_t(program, program_size, index, value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_int64_t(value);
    }
    RuntimeError PUSH_float(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        float value = 0;
        RuntimeError extract_status = Extract.type_float(program, program_size, index, value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_float(value);
    }
    RuntimeError PUSH_double(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        double value = 0;
        RuntimeError extract_status = Extract.type_double(program, program_size, index, value);
        if (extract_status != STATUS_SUCCESS) return extract_status;
        return stack->push_double(value);
    }


    RuntimeError ADD_uint8_t(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push(a + b);
        return STATUS_SUCCESS;
    }
    RuntimeError SUB_uint8_t(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push(a - b);
        return STATUS_SUCCESS;
    }
    RuntimeError MUL_uint8_t(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push(a * b);
        return STATUS_SUCCESS;
    }
    RuntimeError DIV_uint8_t(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push(a / b);
        return STATUS_SUCCESS;
    }

    RuntimeError ADD_uint16_t(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a + b);
        return STATUS_SUCCESS;
    }
    RuntimeError SUB_uint16_t(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a - b);
        return STATUS_SUCCESS;
    }
    RuntimeError MUL_uint16_t(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a * b);
        return STATUS_SUCCESS;
    }
    RuntimeError DIV_uint16_t(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a / b);
        return STATUS_SUCCESS;
    }

    RuntimeError ADD_uint32_t(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a + b);
        return STATUS_SUCCESS;
    }
    RuntimeError SUB_uint32_t(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a - b);
        return STATUS_SUCCESS;
    }
    RuntimeError MUL_uint32_t(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a * b);
        return STATUS_SUCCESS;
    }
    RuntimeError DIV_uint32_t(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a / b);
        return STATUS_SUCCESS;
    }

    RuntimeError ADD_uint64_t(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a + b);
        return STATUS_SUCCESS;
    }
    RuntimeError SUB_uint64_t(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a - b);
        return STATUS_SUCCESS;
    }
    RuntimeError MUL_uint64_t(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a * b);
        return STATUS_SUCCESS;
    }
    RuntimeError DIV_uint64_t(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a / b);
        return STATUS_SUCCESS;
    }

    RuntimeError ADD_int8_t(RuntimeStack* stack) {
        int8_t b = stack->pop_int8_t();
        int8_t a = stack->pop_int8_t();
        stack->push_int8_t(a + b);
        return STATUS_SUCCESS;
    }
    RuntimeError SUB_int8_t(RuntimeStack* stack) {
        int8_t b = stack->pop_int8_t();
        int8_t a = stack->pop_int8_t();
        stack->push_int8_t(a - b);
        return STATUS_SUCCESS;
    }
    RuntimeError MUL_int8_t(RuntimeStack* stack) {
        int8_t b = stack->pop_int8_t();
        int8_t a = stack->pop_int8_t();
        stack->push_int8_t(a * b);
        return STATUS_SUCCESS;
    }
    RuntimeError DIV_int8_t(RuntimeStack* stack) {
        int8_t b = stack->pop_int8_t();
        int8_t a = stack->pop_int8_t();
        stack->push_int8_t(a / b);
        return STATUS_SUCCESS;
    }

    RuntimeError ADD_int16_t(RuntimeStack* stack) {
        int16_t b = stack->pop_int16_t();
        int16_t a = stack->pop_int16_t();
        stack->push_int16_t(a + b);
        return STATUS_SUCCESS;
    }
    RuntimeError SUB_int16_t(RuntimeStack* stack) {
        int16_t b = stack->pop_int16_t();
        int16_t a = stack->pop_int16_t();
        stack->push_int16_t(a - b);
        return STATUS_SUCCESS;
    }
    RuntimeError MUL_int16_t(RuntimeStack* stack) {
        int16_t b = stack->pop_int16_t();
        int16_t a = stack->pop_int16_t();
        stack->push_int16_t(a * b);
        return STATUS_SUCCESS;
    }
    RuntimeError DIV_int16_t(RuntimeStack* stack) {
        int16_t b = stack->pop_int16_t();
        int16_t a = stack->pop_int16_t();
        stack->push_int16_t(a / b);
        return STATUS_SUCCESS;
    }

    RuntimeError ADD_int32_t(RuntimeStack* stack) {
        int32_t b = stack->pop_int32_t();
        int32_t a = stack->pop_int32_t();
        stack->push_int32_t(a + b);
        return STATUS_SUCCESS;
    }
    RuntimeError SUB_int32_t(RuntimeStack* stack) {
        int32_t b = stack->pop_int32_t();
        int32_t a = stack->pop_int32_t();
        stack->push_int32_t(a - b);
        return STATUS_SUCCESS;
    }
    RuntimeError MUL_int32_t(RuntimeStack* stack) {
        int32_t b = stack->pop_int32_t();
        int32_t a = stack->pop_int32_t();
        stack->push_int32_t(a * b);
        return STATUS_SUCCESS;
    }
    RuntimeError DIV_int32_t(RuntimeStack* stack) {
        int32_t b = stack->pop_int32_t();
        int32_t a = stack->pop_int32_t();
        stack->push_int32_t(a / b);
        return STATUS_SUCCESS;
    }

    RuntimeError ADD_int64_t(RuntimeStack* stack) {
        int64_t b = stack->pop_int64_t();
        int64_t a = stack->pop_int64_t();
        stack->push_int64_t(a + b);
        return STATUS_SUCCESS;
    }
    RuntimeError SUB_int64_t(RuntimeStack* stack) {
        int64_t b = stack->pop_int64_t();
        int64_t a = stack->pop_int64_t();
        stack->push_int64_t(a - b);
        return STATUS_SUCCESS;
    }
    RuntimeError MUL_int64_t(RuntimeStack* stack) {
        int64_t b = stack->pop_int64_t();
        int64_t a = stack->pop_int64_t();
        stack->push_int64_t(a * b);
        return STATUS_SUCCESS;
    }
    RuntimeError DIV_int64_t(RuntimeStack* stack) {
        int64_t b = stack->pop_int64_t();
        int64_t a = stack->pop_int64_t();
        stack->push_int64_t(a / b);
        return STATUS_SUCCESS;
    }

    RuntimeError ADD_float(RuntimeStack* stack) {
        float b = stack->pop_float();
        float a = stack->pop_float();
        stack->push_float(a + b);
        return STATUS_SUCCESS;
    }
    RuntimeError SUB_float(RuntimeStack* stack) {
        float b = stack->pop_float();
        float a = stack->pop_float();
        stack->push_float(a - b);
        return STATUS_SUCCESS;
    }
    RuntimeError MUL_float(RuntimeStack* stack) {
        float b = stack->pop_float();
        float a = stack->pop_float();
        stack->push_float(a * b);
        return STATUS_SUCCESS;
    }
    RuntimeError DIV_float(RuntimeStack* stack) {
        float b = stack->pop_float();
        float a = stack->pop_float();
        stack->push_float(a / b);
        return STATUS_SUCCESS;
    }

    RuntimeError ADD_double(RuntimeStack* stack) {
        double b = stack->pop_double();
        double a = stack->pop_double();
        stack->push_double(a + b);
        return STATUS_SUCCESS;
    }
    RuntimeError SUB_double(RuntimeStack* stack) {
        double b = stack->pop_double();
        double a = stack->pop_double();
        stack->push_double(a - b);
        return STATUS_SUCCESS;
    }
    RuntimeError MUL_double(RuntimeStack* stack) {
        double b = stack->pop_double();
        double a = stack->pop_double();
        stack->push_double(a * b);
        return STATUS_SUCCESS;
    }
    RuntimeError DIV_double(RuntimeStack* stack) {
        double b = stack->pop_double();
        double a = stack->pop_double();
        stack->push_double(a / b);
        return STATUS_SUCCESS;
    }







    RuntimeError handle_ADD(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
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
    }

    RuntimeError handle_SUB(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = program[index++];
        switch (data_type) {
            case type_bool:
            case type_uint8_t: return SUB_uint8_t(stack);
            case type_uint16_t: return SUB_uint16_t(stack);
            case type_uint32_t: return SUB_uint32_t(stack);
            case type_uint64_t: return SUB_uint64_t(stack);
            case type_int8_t: return SUB_int8_t(stack);
            case type_int16_t: return SUB_int16_t(stack);
            case type_int32_t: return SUB_int32_t(stack);
            case type_int64_t: return SUB_int64_t(stack);
            case type_float: return SUB_float(stack);
            case type_double: return SUB_double(stack);
            default: return INVALID_DATA_TYPE;
        }
    }

    RuntimeError handle_MUL(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = program[index++];
        switch (data_type) {
            case type_bool:
            case type_uint8_t: return MUL_uint8_t(stack);
            case type_uint16_t: return MUL_uint16_t(stack);
            case type_uint32_t: return MUL_uint32_t(stack);
            case type_uint64_t: return MUL_uint64_t(stack);
            case type_int8_t: return MUL_int8_t(stack);
            case type_int16_t: return MUL_int16_t(stack);
            case type_int32_t: return MUL_int32_t(stack);
            case type_int64_t: return MUL_int64_t(stack);
            case type_float: return MUL_float(stack);
            case type_double: return MUL_double(stack);
            default: return INVALID_DATA_TYPE;
        }
    }

    RuntimeError handle_DIV(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = program[index++];
        switch (data_type) {
            case type_bool:
            case type_uint8_t: return DIV_uint8_t(stack);
            case type_uint16_t: return DIV_uint16_t(stack);
            case type_uint32_t: return DIV_uint32_t(stack);
            case type_uint64_t: return DIV_uint64_t(stack);
            case type_int8_t: return DIV_int8_t(stack);
            case type_int16_t: return DIV_int16_t(stack);
            case type_int32_t: return DIV_int32_t(stack);
            case type_int64_t: return DIV_int64_t(stack);
            case type_float: return DIV_float(stack);
            case type_double: return DIV_double(stack);
            default: return INVALID_DATA_TYPE;
        }
    }



    RuntimeError BW_AND_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a & b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_AND_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a & b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_AND_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a & b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_AND_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a & b);
        return STATUS_SUCCESS;
    }

    RuntimeError BW_OR_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a | b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_OR_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a | b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_OR_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a | b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_OR_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a | b);
        return STATUS_SUCCESS;
    }

    RuntimeError BW_XOR_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a ^ b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_XOR_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a ^ b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_XOR_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a ^ b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_XOR_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a ^ b);
        return STATUS_SUCCESS;
    }

    RuntimeError BW_NOT_X8(RuntimeStack* stack) {
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(~a);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_NOT_X16(RuntimeStack* stack) {
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(~a);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_NOT_X32(RuntimeStack* stack) {
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(~a);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_NOT_X64(RuntimeStack* stack) {
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(~a);
        return STATUS_SUCCESS;
    }

    RuntimeError BW_LSHIFT_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a << b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_LSHIFT_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a << b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_LSHIFT_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a << b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_LSHIFT_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a << b);
        return STATUS_SUCCESS;
    }

    RuntimeError BW_RSHIFT_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a >> b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_RSHIFT_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a >> b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_RSHIFT_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a >> b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_RSHIFT_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a >> b);
        return STATUS_SUCCESS;
    }

    // Boolean operators
    RuntimeError LOGIC_AND(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t() != 0;
        uint8_t a = stack->pop_uint8_t() != 0;
        stack->push_uint8_t(a && b);
        return STATUS_SUCCESS;
    }
    RuntimeError LOGIC_OR(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t() != 0;
        uint8_t a = stack->pop_uint8_t() != 0;
        stack->push_uint8_t(a || b);
        return STATUS_SUCCESS;
    }
    RuntimeError LOGIC_XOR(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t() != 0;
        uint8_t a = stack->pop_uint8_t() != 0;
        stack->push_uint8_t(a ^ b);
        return STATUS_SUCCESS;
    }
    RuntimeError LOGIC_NOT(RuntimeStack* stack) {
        uint8_t a = stack->pop_uint8_t() != 0;
        stack->push_uint8_t(!a);
        return STATUS_SUCCESS;
    }

    RuntimeError CMP_EQ_uint8_t(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a == b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_EQ_uint16_t(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint8_t(a == b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_EQ_uint32_t(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint8_t(a == b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_EQ_uint64_t(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint8_t(a == b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_EQ_int8_t(RuntimeStack* stack) {
        int8_t b = stack->pop_int8_t();
        int8_t a = stack->pop_int8_t();
        stack->push_uint8_t(a == b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_EQ_int16_t(RuntimeStack* stack) {
        int16_t b = stack->pop_int16_t();
        int16_t a = stack->pop_int16_t();
        stack->push_uint8_t(a == b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_EQ_int32_t(RuntimeStack* stack) {
        int32_t b = stack->pop_int32_t();
        int32_t a = stack->pop_int32_t();
        stack->push_uint8_t(a == b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_EQ_int64_t(RuntimeStack* stack) {
        int64_t b = stack->pop_int64_t();
        int64_t a = stack->pop_int64_t();
        stack->push_uint8_t(a == b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_EQ_float(RuntimeStack* stack) {
        float b = stack->pop_float();
        float a = stack->pop_float();
        stack->push_uint8_t(a == b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_EQ_double(RuntimeStack* stack) {
        double b = stack->pop_double();
        double a = stack->pop_double();
        stack->push_uint8_t(a == b);
        return STATUS_SUCCESS;
    }

    RuntimeError CMP_NEQ_uint8_t(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a != b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_NEQ_uint16_t(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint8_t(a != b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_NEQ_uint32_t(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint8_t(a != b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_NEQ_uint64_t(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint8_t(a != b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_NEQ_int8_t(RuntimeStack* stack) {
        int8_t b = stack->pop_int8_t();
        int8_t a = stack->pop_int8_t();
        stack->push_uint8_t(a != b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_NEQ_int16_t(RuntimeStack* stack) {
        int16_t b = stack->pop_int16_t();
        int16_t a = stack->pop_int16_t();
        stack->push_uint8_t(a != b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_NEQ_int32_t(RuntimeStack* stack) {
        int32_t b = stack->pop_int32_t();
        int32_t a = stack->pop_int32_t();
        stack->push_uint8_t(a != b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_NEQ_int64_t(RuntimeStack* stack) {
        int64_t b = stack->pop_int64_t();
        int64_t a = stack->pop_int64_t();
        stack->push_uint8_t(a != b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_NEQ_float(RuntimeStack* stack) {
        float b = stack->pop_float();
        float a = stack->pop_float();
        stack->push_uint8_t(a != b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_NEQ_double(RuntimeStack* stack) {
        double b = stack->pop_double();
        double a = stack->pop_double();
        stack->push_uint8_t(a != b);
        return STATUS_SUCCESS;
    }

    RuntimeError CMP_GT_uint8_t(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a > b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GT_uint16_t(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint8_t(a > b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GT_uint32_t(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint8_t(a > b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GT_uint64_t(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint8_t(a > b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GT_int8_t(RuntimeStack* stack) {
        int8_t b = stack->pop_int8_t();
        int8_t a = stack->pop_int8_t();
        stack->push_uint8_t(a > b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GT_int16_t(RuntimeStack* stack) {
        int16_t b = stack->pop_int16_t();
        int16_t a = stack->pop_int16_t();
        stack->push_uint8_t(a > b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GT_int32_t(RuntimeStack* stack) {
        int32_t b = stack->pop_int32_t();
        int32_t a = stack->pop_int32_t();
        stack->push_uint8_t(a > b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GT_int64_t(RuntimeStack* stack) {
        int64_t b = stack->pop_int64_t();
        int64_t a = stack->pop_int64_t();
        stack->push_uint8_t(a > b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GT_float(RuntimeStack* stack) {
        float b = stack->pop_float();
        float a = stack->pop_float();
        stack->push_uint8_t(a > b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GT_double(RuntimeStack* stack) {
        double b = stack->pop_double();
        double a = stack->pop_double();
        stack->push_uint8_t(a > b);
        return STATUS_SUCCESS;
    }

    RuntimeError CMP_LT_uint8_t(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a < b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LT_uint16_t(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint8_t(a < b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LT_uint32_t(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint8_t(a < b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LT_uint64_t(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint8_t(a < b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LT_int8_t(RuntimeStack* stack) {
        int8_t b = stack->pop_int8_t();
        int8_t a = stack->pop_int8_t();
        stack->push_uint8_t(a < b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LT_int16_t(RuntimeStack* stack) {
        int16_t b = stack->pop_int16_t();
        int16_t a = stack->pop_int16_t();
        stack->push_uint8_t(a < b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LT_int32_t(RuntimeStack* stack) {
        int32_t b = stack->pop_int32_t();
        int32_t a = stack->pop_int32_t();
        stack->push_uint8_t(a < b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LT_int64_t(RuntimeStack* stack) {
        int64_t b = stack->pop_int64_t();
        int64_t a = stack->pop_int64_t();
        stack->push_uint8_t(a < b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LT_float(RuntimeStack* stack) {
        float b = stack->pop_float();
        float a = stack->pop_float();
        stack->push_uint8_t(a < b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LT_double(RuntimeStack* stack) {
        double b = stack->pop_double();
        double a = stack->pop_double();
        stack->push_uint8_t(a < b);
        return STATUS_SUCCESS;
    }

    RuntimeError CMP_GTE_uint8_t(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a >= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GTE_uint16_t(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint8_t(a >= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GTE_uint32_t(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint8_t(a >= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GTE_uint64_t(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint8_t(a >= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GTE_int8_t(RuntimeStack* stack) {
        int8_t b = stack->pop_int8_t();
        int8_t a = stack->pop_int8_t();
        stack->push_uint8_t(a >= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GTE_int16_t(RuntimeStack* stack) {
        int16_t b = stack->pop_int16_t();
        int16_t a = stack->pop_int16_t();
        stack->push_uint8_t(a >= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GTE_int32_t(RuntimeStack* stack) {
        int32_t b = stack->pop_int32_t();
        int32_t a = stack->pop_int32_t();
        stack->push_uint8_t(a >= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GTE_int64_t(RuntimeStack* stack) {
        int64_t b = stack->pop_int64_t();
        int64_t a = stack->pop_int64_t();
        stack->push_uint8_t(a >= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GTE_float(RuntimeStack* stack) {
        float b = stack->pop_float();
        float a = stack->pop_float();
        stack->push_uint8_t(a >= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GTE_double(RuntimeStack* stack) {
        double b = stack->pop_double();
        double a = stack->pop_double();
        stack->push_uint8_t(a >= b);
        return STATUS_SUCCESS;
    }

    RuntimeError CMP_LTE_uint8_t(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a <= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LTE_uint16_t(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint8_t(a <= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LTE_uint32_t(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint8_t(a <= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LTE_uint64_t(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint8_t(a <= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LTE_int8_t(RuntimeStack* stack) {
        int8_t b = stack->pop_int8_t();
        int8_t a = stack->pop_int8_t();
        stack->push_uint8_t(a <= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LTE_int16_t(RuntimeStack* stack) {
        int16_t b = stack->pop_int16_t();
        int16_t a = stack->pop_int16_t();
        stack->push_uint8_t(a <= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LTE_int32_t(RuntimeStack* stack) {
        int32_t b = stack->pop_int32_t();
        int32_t a = stack->pop_int32_t();
        stack->push_uint8_t(a <= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LTE_int64_t(RuntimeStack* stack) {
        int64_t b = stack->pop_int64_t();
        int64_t a = stack->pop_int64_t();
        stack->push_uint8_t(a <= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LTE_float(RuntimeStack* stack) {
        float b = stack->pop_float();
        float a = stack->pop_float();
        stack->push_uint8_t(a <= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LTE_double(RuntimeStack* stack) {
        double b = stack->pop_double();
        double a = stack->pop_double();
        stack->push_uint8_t(a <= b);
        return STATUS_SUCCESS;
    }

    // Comparison operators
    RuntimeError handle_CMP_EQ(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = program[index++];
        switch (data_type) {
            case type_bool:
            case type_uint8_t: return CMP_EQ_uint8_t(stack);
            case type_uint16_t: return CMP_EQ_uint16_t(stack);
            case type_uint32_t: return CMP_EQ_uint32_t(stack);
            case type_uint64_t: return CMP_EQ_uint64_t(stack);
            case type_int8_t: return CMP_EQ_int8_t(stack);
            case type_int16_t: return CMP_EQ_int16_t(stack);
            case type_int32_t: return CMP_EQ_int32_t(stack);
            case type_int64_t: return CMP_EQ_int64_t(stack);
            case type_float: return CMP_EQ_float(stack);
            case type_double: return CMP_EQ_double(stack);
            default: return INVALID_DATA_TYPE;
        }
    }
    RuntimeError handle_CMP_NEQ(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = program[index++];
        switch (data_type) {
            case type_bool:
            case type_uint8_t: return CMP_NEQ_uint8_t(stack);
            case type_uint16_t: return CMP_NEQ_uint16_t(stack);
            case type_uint32_t: return CMP_NEQ_uint32_t(stack);
            case type_uint64_t: return CMP_NEQ_uint64_t(stack);
            case type_int8_t: return CMP_NEQ_int8_t(stack);
            case type_int16_t: return CMP_NEQ_int16_t(stack);
            case type_int32_t: return CMP_NEQ_int32_t(stack);
            case type_int64_t: return CMP_NEQ_int64_t(stack);
            case type_float: return CMP_NEQ_float(stack);
            case type_double: return CMP_NEQ_double(stack);
            default: return INVALID_DATA_TYPE;
        }
    }
    RuntimeError handle_CMP_GT(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = program[index++];
        switch (data_type) {
            case type_bool:
            case type_uint8_t: return CMP_GT_uint8_t(stack);
            case type_uint16_t: return CMP_GT_uint16_t(stack);
            case type_uint32_t: return CMP_GT_uint32_t(stack);
            case type_uint64_t: return CMP_GT_uint64_t(stack);
            case type_int8_t: return CMP_GT_int8_t(stack);
            case type_int16_t: return CMP_GT_int16_t(stack);
            case type_int32_t: return CMP_GT_int32_t(stack);
            case type_int64_t: return CMP_GT_int64_t(stack);
            case type_float: return CMP_GT_float(stack);
            case type_double: return CMP_GT_double(stack);
            default: return INVALID_DATA_TYPE;
        }
    }
    RuntimeError handle_CMP_GTE(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = program[index++];
        switch (data_type) {
            case type_bool:
            case type_uint8_t: return CMP_GTE_uint8_t(stack);
            case type_uint16_t: return CMP_GTE_uint16_t(stack);
            case type_uint32_t: return CMP_GTE_uint32_t(stack);
            case type_uint64_t: return CMP_GTE_uint64_t(stack);
            case type_int8_t: return CMP_GTE_int8_t(stack);
            case type_int16_t: return CMP_GTE_int16_t(stack);
            case type_int32_t: return CMP_GTE_int32_t(stack);
            case type_int64_t: return CMP_GTE_int64_t(stack);
            case type_float: return CMP_GTE_float(stack);
            case type_double: return CMP_GTE_double(stack);
            default: return INVALID_DATA_TYPE;
        }
    }
    RuntimeError handle_CMP_LT(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = program[index++];
        switch (data_type) {
            case type_bool:
            case type_uint8_t: return CMP_LT_uint8_t(stack);
            case type_uint16_t: return CMP_LT_uint16_t(stack);
            case type_uint32_t: return CMP_LT_uint32_t(stack);
            case type_uint64_t: return CMP_LT_uint64_t(stack);
            case type_int8_t: return CMP_LT_int8_t(stack);
            case type_int16_t: return CMP_LT_int16_t(stack);
            case type_int32_t: return CMP_LT_int32_t(stack);
            case type_int64_t: return CMP_LT_int64_t(stack);
            case type_float: return CMP_LT_float(stack);
            case type_double: return CMP_LT_double(stack);
            default: return INVALID_DATA_TYPE;
        }
    }
    RuntimeError handle_CMP_LTE(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = program[index++];
        switch (data_type) {
            case type_bool:
            case type_uint8_t: return CMP_LTE_uint8_t(stack);
            case type_uint16_t: return CMP_LTE_uint16_t(stack);
            case type_uint32_t: return CMP_LTE_uint32_t(stack);
            case type_uint64_t: return CMP_LTE_uint64_t(stack);
            case type_int8_t: return CMP_LTE_int8_t(stack);
            case type_int16_t: return CMP_LTE_int16_t(stack);
            case type_int32_t: return CMP_LTE_int32_t(stack);
            case type_int64_t: return CMP_LTE_int64_t(stack);
            case type_float: return CMP_LTE_float(stack);
            case type_double: return CMP_LTE_double(stack);
            default: return INVALID_DATA_TYPE;
        }
    }


    RuntimeError handle_JMP(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 2;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8A_to_u16 cvt;
        cvt.u8A[1] = program[index];
        cvt.u8A[0] = program[index + 1];
        index = cvt.u16;
        if (index >= program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        return STATUS_SUCCESS;
    }
    RuntimeError handle_JMP_IF(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 2;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8A_to_u16 cvt;
        cvt.u8A[1] = program[index];
        cvt.u8A[0] = program[index + 1];
        if (stack->pop_bool()) {
            index = cvt.u16;
            if (index >= program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        }
        return STATUS_SUCCESS;
    }
    RuntimeError handle_JMP_IF_NOT(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 2;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8A_to_u16 cvt;
        cvt.u8A[1] = program[index];
        cvt.u8A[0] = program[index + 1];
        if (!stack->pop_bool()) {
            index = cvt.u16;
            if (index >= program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        }
        return STATUS_SUCCESS;
    }

    RuntimeError handle_CALL(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 2;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8A_to_u16 cvt;
        cvt.u8A[1] = program[index];
        cvt.u8A[0] = program[index + 1];
        index = cvt.u16;
        if (index >= program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        RuntimeError call_store_status = stack->pushCall(index);
        if (call_store_status != STATUS_SUCCESS) return call_store_status;
        if (index >= program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        return STATUS_SUCCESS;
    }
    RuntimeError handle_CALL_IF(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 2;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8A_to_u16 cvt;
        cvt.u8A[1] = program[index];
        cvt.u8A[0] = program[index + 1];
        if (stack->pop_bool()) {
            index = cvt.u16;
            if (index == 0 || index >= program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
            RuntimeError call_store_status = stack->pushCall(index);
            if (call_store_status != STATUS_SUCCESS) return call_store_status;
            if (index >= program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        }
        return STATUS_SUCCESS;
    }
    RuntimeError handle_CALL_IF_NOT(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 2;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8A_to_u16 cvt;
        cvt.u8A[1] = program[index];
        cvt.u8A[0] = program[index + 1];
        if (!stack->pop_bool()) {
            index = cvt.u16;
            if (index == 0 || index >= program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
            RuntimeError call_store_status = stack->pushCall(index);
            if (call_store_status != STATUS_SUCCESS) return call_store_status;
            if (index >= program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        }
        return STATUS_SUCCESS;
    }
    RuntimeError handle_RET(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        if (stack->call_stack->size() == 0) return CALL_STACK_UNDERFLOW;
        uint16_t ret_index = stack->popCall();
        index = ret_index;
        if (index >= program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        return STATUS_SUCCESS;
    }


    RuntimeError handle_EXIT(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        if (index >= program_size) return STATUS_SUCCESS;
        uint8_t exit_code = program[index++];
        return (RuntimeError) exit_code;
    }

} PLCMethods;