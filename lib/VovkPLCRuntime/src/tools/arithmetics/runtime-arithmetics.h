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

#include "runtime-parsing.h"
#include "math-u8.h"
#include "math-u16.h"
#include "math-u32.h"
#include "math-i8.h"
#include "math-i16.h"
#include "math-i32.h"
#include "math-f32.h"
#ifdef USE_X64_OPS
#include "math-u64.h"
#include "math-i64.h"
#include "math-f64.h"
#endif // USE_X64_OPS
#include "memory-manipulation.h"
#include "methods-bitwise.h"
#include "methods-logic.h"
#include "methods-comparison.h"
#include "methods-flow.h"

namespace PLCMethods {

    RuntimeError handle_ADD(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u32 size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = program[index++];
        switch (data_type) {
            case type_bool:
            case type_u8: return ADD_uint8_t(stack);
            case type_u16: return ADD_uint16_t(stack);
            case type_u32: return ADD_uint32_t(stack);
            case type_i8: return ADD_int8_t(stack);
            case type_i16: return ADD_int16_t(stack);
            case type_i32: return ADD_int32_t(stack);
            case type_f32: return ADD_float(stack);
#ifdef USE_X64_OPS
            case type_u64: return ADD_uint64_t(stack);
            case type_i64: return ADD_int64_t(stack);
            case type_f64: return ADD_double(stack);
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }

    RuntimeError handle_SUB(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u32 size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = program[index++];
        switch (data_type) {
            case type_bool:
            case type_u8: return SUB_uint8_t(stack);
            case type_u16: return SUB_uint16_t(stack);
            case type_u32: return SUB_uint32_t(stack);
            case type_i8: return SUB_int8_t(stack);
            case type_i16: return SUB_int16_t(stack);
            case type_i32: return SUB_int32_t(stack);
            case type_f32: return SUB_float(stack);
#ifdef USE_X64_OPS
            case type_u64: return SUB_uint64_t(stack);
            case type_i64: return SUB_int64_t(stack);
            case type_f64: return SUB_double(stack);
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }

    RuntimeError handle_MUL(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u32 size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = program[index++];
        switch (data_type) {
            case type_bool:
            case type_u8: return MUL_uint8_t(stack);
            case type_u16: return MUL_uint16_t(stack);
            case type_u32: return MUL_uint32_t(stack);
            case type_i8: return MUL_int8_t(stack);
            case type_i16: return MUL_int16_t(stack);
            case type_i32: return MUL_int32_t(stack);
            case type_f32: return MUL_float(stack);
#ifdef USE_X64_OPS
            case type_u64: return MUL_uint64_t(stack);
            case type_i64: return MUL_int64_t(stack);
            case type_f64: return MUL_double(stack);
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }

    RuntimeError handle_DIV(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u32 size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = program[index++];
        switch (data_type) {
            case type_bool:
            case type_u8: return DIV_uint8_t(stack);
            case type_u16: return DIV_uint16_t(stack);
            case type_u32: return DIV_uint32_t(stack);
            case type_i8: return DIV_int8_t(stack);
            case type_i16: return DIV_int16_t(stack);
            case type_i32: return DIV_int32_t(stack);
            case type_f32: return DIV_float(stack);
#ifdef USE_X64_OPS
            case type_u64: return DIV_uint64_t(stack);
            case type_i64: return DIV_int64_t(stack);
            case type_f64: return DIV_double(stack);
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }



    // Comparison operators
    RuntimeError handle_CMP_EQ(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u32 size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = program[index++];
        switch (data_type) {
            case type_bool:
            case type_u8: return CMP_EQ_uint8_t(stack);
            case type_u16: return CMP_EQ_uint16_t(stack);
            case type_u32: return CMP_EQ_uint32_t(stack);
            case type_i8: return CMP_EQ_int8_t(stack);
            case type_i16: return CMP_EQ_int16_t(stack);
            case type_i32: return CMP_EQ_int32_t(stack);
            case type_f32: return CMP_EQ_float(stack);
#ifdef USE_X64_OPS
            case type_u64: return CMP_EQ_uint64_t(stack);
            case type_i64: return CMP_EQ_int64_t(stack);
            case type_f64: return CMP_EQ_double(stack);
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }
    RuntimeError handle_CMP_NEQ(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u32 size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = program[index++];
        switch (data_type) {
            case type_bool:
            case type_u8: return CMP_NEQ_uint8_t(stack);
            case type_u16: return CMP_NEQ_uint16_t(stack);
            case type_u32: return CMP_NEQ_uint32_t(stack);
            case type_i8: return CMP_NEQ_int8_t(stack);
            case type_i16: return CMP_NEQ_int16_t(stack);
            case type_i32: return CMP_NEQ_int32_t(stack);
            case type_f32: return CMP_NEQ_float(stack);
#ifdef USE_X64_OPS
            case type_u64: return CMP_NEQ_uint64_t(stack);
            case type_i64: return CMP_NEQ_int64_t(stack);
            case type_f64: return CMP_NEQ_double(stack);
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }
    RuntimeError handle_CMP_GT(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u32 size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = program[index++];
        switch (data_type) {
            case type_bool:
            case type_u8: return CMP_GT_uint8_t(stack);
            case type_u16: return CMP_GT_uint16_t(stack);
            case type_u32: return CMP_GT_uint32_t(stack);
            case type_i8: return CMP_GT_int8_t(stack);
            case type_i16: return CMP_GT_int16_t(stack);
            case type_i32: return CMP_GT_int32_t(stack);
            case type_f32: return CMP_GT_float(stack);
#ifdef USE_X64_OPS
            case type_u64: return CMP_GT_uint64_t(stack);
            case type_i64: return CMP_GT_int64_t(stack);
            case type_f64: return CMP_GT_double(stack);
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }
    RuntimeError handle_CMP_GTE(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u32 size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = program[index++];
        switch (data_type) {
            case type_bool:
            case type_u8: return CMP_GTE_uint8_t(stack);
            case type_u16: return CMP_GTE_uint16_t(stack);
            case type_u32: return CMP_GTE_uint32_t(stack);
            case type_i8: return CMP_GTE_int8_t(stack);
            case type_i16: return CMP_GTE_int16_t(stack);
            case type_i32: return CMP_GTE_int32_t(stack);
            case type_f32: return CMP_GTE_float(stack);
#ifdef USE_X64_OPS
            case type_u64: return CMP_GTE_uint64_t(stack);
            case type_i64: return CMP_GTE_int64_t(stack);
            case type_f64: return CMP_GTE_double(stack);
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }
    RuntimeError handle_CMP_LT(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u32 size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = program[index++];
        switch (data_type) {
            case type_bool:
            case type_u8: return CMP_LT_uint8_t(stack);
            case type_u16: return CMP_LT_uint16_t(stack);
            case type_u32: return CMP_LT_uint32_t(stack);
            case type_i8: return CMP_LT_int8_t(stack);
            case type_i16: return CMP_LT_int16_t(stack);
            case type_i32: return CMP_LT_int32_t(stack);
            case type_f32: return CMP_LT_float(stack);
#ifdef USE_X64_OPS
            case type_u64: return CMP_LT_uint64_t(stack);
            case type_i64: return CMP_LT_int64_t(stack);
            case type_f64: return CMP_LT_double(stack);
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }
    RuntimeError handle_CMP_LTE(RuntimeStack* stack, u8* program, u32 program_size, u32& index) {
        u32 size = 1;
        if (index + size > program_size) return PROGRAM_POINTER_OUT_OF_BOUNDS;
        u8 data_type = program[index++];
        switch (data_type) {
            case type_bool:
            case type_u8: return CMP_LTE_uint8_t(stack);
            case type_u16: return CMP_LTE_uint16_t(stack);
            case type_u32: return CMP_LTE_uint32_t(stack);
            case type_i8: return CMP_LTE_int8_t(stack);
            case type_i16: return CMP_LTE_int16_t(stack);
            case type_i32: return CMP_LTE_int32_t(stack);
            case type_f32: return CMP_LTE_float(stack);
#ifdef USE_X64_OPS
            case type_u64: return CMP_LTE_uint64_t(stack);
            case type_i64: return CMP_LTE_int64_t(stack);
            case type_f64: return CMP_LTE_double(stack);
#endif // USE_X64_OPS
            default: return INVALID_DATA_TYPE;
        }
    }

}

