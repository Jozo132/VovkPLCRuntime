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

#include "math-u8.h"
#include "math-u16.h"
#include "math-u32.h"
#include "math-u64.h"
#include "math-s8.h"
#include "math-s16.h"
#include "math-s32.h"
#include "math-s64.h"
#include "math-f32.h"
#include "math-f64.h"
#include "memory-manipulation.h"
#include "methods-bitwise.h"
#include "methods-logic.h"
#include "methods-comparison.h"
#include "methods-flow.h"

namespace PLCMethods {

    RuntimeError handle_ADD(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        uint16_t size = 1;
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
        uint16_t size = 1;
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
        uint16_t size = 1;
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
        uint16_t size = 1;
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



    // Comparison operators
    RuntimeError handle_CMP_EQ(RuntimeStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        uint16_t size = 1;
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
        uint16_t size = 1;
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
        uint16_t size = 1;
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
        uint16_t size = 1;
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
        uint16_t size = 1;
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
        uint16_t size = 1;
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

}

