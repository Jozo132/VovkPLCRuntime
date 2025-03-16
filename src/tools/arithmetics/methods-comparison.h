// methods-comparison.h - 2022-12-11
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

    RuntimeError CMP_EQ_uint8_t(RuntimeStack& stack) {
        u8 b = stack.pop_u8();
        u8 a = stack.pop_u8();
        stack.push_u8(a == b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_EQ_uint16_t(RuntimeStack& stack) {
        u16 b = stack.pop_u16();
        u16 a = stack.pop_u16();
        stack.push_u8(a == b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_EQ_uint32_t(RuntimeStack& stack) {
        u32 b = stack.pop_u32();
        u32 a = stack.pop_u32();
        stack.push_u8(a == b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_EQ_uint64_t(RuntimeStack& stack) {
        u64 b = stack.pop_u64();
        u64 a = stack.pop_u64();
        stack.push_u8(a == b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
    RuntimeError CMP_EQ_int8_t(RuntimeStack& stack) {
        i8 b = stack.pop_i8();
        i8 a = stack.pop_i8();
        stack.push_u8(a == b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_EQ_int16_t(RuntimeStack& stack) {
        i16 b = stack.pop_i16();
        i16 a = stack.pop_i16();
        stack.push_u8(a == b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_EQ_int32_t(RuntimeStack& stack) {
        i32 b = stack.pop_i32();
        i32 a = stack.pop_i32();
        stack.push_u8(a == b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_EQ_int64_t(RuntimeStack& stack) {
        i64 b = stack.pop_i64();
        i64 a = stack.pop_i64();
        stack.push_u8(a == b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
    RuntimeError CMP_EQ_float(RuntimeStack& stack) {
        float b = stack.pop_f32();
        float a = stack.pop_f32();
        stack.push_u8(a == b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_EQ_double(RuntimeStack& stack) {
        double b = stack.pop_f64();
        double a = stack.pop_f64();
        stack.push_u8(a == b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS

    RuntimeError CMP_NEQ_uint8_t(RuntimeStack& stack) {
        u8 b = stack.pop_u8();
        u8 a = stack.pop_u8();
        stack.push_u8(a != b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_NEQ_uint16_t(RuntimeStack& stack) {
        u16 b = stack.pop_u16();
        u16 a = stack.pop_u16();
        stack.push_u8(a != b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_NEQ_uint32_t(RuntimeStack& stack) {
        u32 b = stack.pop_u32();
        u32 a = stack.pop_u32();
        stack.push_u8(a != b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_NEQ_uint64_t(RuntimeStack& stack) {
        u64 b = stack.pop_u64();
        u64 a = stack.pop_u64();
        stack.push_u8(a != b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
    RuntimeError CMP_NEQ_int8_t(RuntimeStack& stack) {
        i8 b = stack.pop_i8();
        i8 a = stack.pop_i8();
        stack.push_u8(a != b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_NEQ_int16_t(RuntimeStack& stack) {
        i16 b = stack.pop_i16();
        i16 a = stack.pop_i16();
        stack.push_u8(a != b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_NEQ_int32_t(RuntimeStack& stack) {
        i32 b = stack.pop_i32();
        i32 a = stack.pop_i32();
        stack.push_u8(a != b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_NEQ_int64_t(RuntimeStack& stack) {
        i64 b = stack.pop_i64();
        i64 a = stack.pop_i64();
        stack.push_u8(a != b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
    RuntimeError CMP_NEQ_float(RuntimeStack& stack) {
        float b = stack.pop_f32();
        float a = stack.pop_f32();
        stack.push_u8(a != b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_NEQ_double(RuntimeStack& stack) {
        double b = stack.pop_f64();
        double a = stack.pop_f64();
        stack.push_u8(a != b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS

    RuntimeError CMP_GT_uint8_t(RuntimeStack& stack) {
        u8 b = stack.pop_u8();
        u8 a = stack.pop_u8();
        stack.push_u8(a > b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GT_uint16_t(RuntimeStack& stack) {
        u16 b = stack.pop_u16();
        u16 a = stack.pop_u16();
        stack.push_u8(a > b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GT_uint32_t(RuntimeStack& stack) {
        u32 b = stack.pop_u32();
        u32 a = stack.pop_u32();
        stack.push_u8(a > b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_GT_uint64_t(RuntimeStack& stack) {
        u64 b = stack.pop_u64();
        u64 a = stack.pop_u64();
        stack.push_u8(a > b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
    RuntimeError CMP_GT_int8_t(RuntimeStack& stack) {
        i8 b = stack.pop_i8();
        i8 a = stack.pop_i8();
        stack.push_u8(a > b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GT_int16_t(RuntimeStack& stack) {
        i16 b = stack.pop_i16();
        i16 a = stack.pop_i16();
        stack.push_u8(a > b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GT_int32_t(RuntimeStack& stack) {
        i32 b = stack.pop_i32();
        i32 a = stack.pop_i32();
        stack.push_u8(a > b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_GT_int64_t(RuntimeStack& stack) {
        i64 b = stack.pop_i64();
        i64 a = stack.pop_i64();
        stack.push_u8(a > b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
    RuntimeError CMP_GT_float(RuntimeStack& stack) {
        float b = stack.pop_f32();
        float a = stack.pop_f32();
        stack.push_u8(a > b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_GT_double(RuntimeStack& stack) {
        double b = stack.pop_f64();
        double a = stack.pop_f64();
        stack.push_u8(a > b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS

    RuntimeError CMP_LT_uint8_t(RuntimeStack& stack) {
        u8 b = stack.pop_u8();
        u8 a = stack.pop_u8();
        stack.push_u8(a < b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LT_uint16_t(RuntimeStack& stack) {
        u16 b = stack.pop_u16();
        u16 a = stack.pop_u16();
        stack.push_u8(a < b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LT_uint32_t(RuntimeStack& stack) {
        u32 b = stack.pop_u32();
        u32 a = stack.pop_u32();
        stack.push_u8(a < b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_LT_uint64_t(RuntimeStack& stack) {
        u64 b = stack.pop_u64();
        u64 a = stack.pop_u64();
        stack.push_u8(a < b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
    RuntimeError CMP_LT_int8_t(RuntimeStack& stack) {
        i8 b = stack.pop_i8();
        i8 a = stack.pop_i8();
        stack.push_u8(a < b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LT_int16_t(RuntimeStack& stack) {
        i16 b = stack.pop_i16();
        i16 a = stack.pop_i16();
        stack.push_u8(a < b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LT_int32_t(RuntimeStack& stack) {
        i32 b = stack.pop_i32();
        i32 a = stack.pop_i32();
        stack.push_u8(a < b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_LT_int64_t(RuntimeStack& stack) {
        i64 b = stack.pop_i64();
        i64 a = stack.pop_i64();
        stack.push_u8(a < b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
    RuntimeError CMP_LT_float(RuntimeStack& stack) {
        float b = stack.pop_f32();
        float a = stack.pop_f32();
        stack.push_u8(a < b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_LT_double(RuntimeStack& stack) {
        double b = stack.pop_f64();
        double a = stack.pop_f64();
        stack.push_u8(a < b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS

    RuntimeError CMP_GTE_uint8_t(RuntimeStack& stack) {
        u8 b = stack.pop_u8();
        u8 a = stack.pop_u8();
        stack.push_u8(a >= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GTE_uint16_t(RuntimeStack& stack) {
        u16 b = stack.pop_u16();
        u16 a = stack.pop_u16();
        stack.push_u8(a >= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GTE_uint32_t(RuntimeStack& stack) {
        u32 b = stack.pop_u32();
        u32 a = stack.pop_u32();
        stack.push_u8(a >= b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_GTE_uint64_t(RuntimeStack& stack) {
        u64 b = stack.pop_u64();
        u64 a = stack.pop_u64();
        stack.push_u8(a >= b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
    RuntimeError CMP_GTE_int8_t(RuntimeStack& stack) {
        i8 b = stack.pop_i8();
        i8 a = stack.pop_i8();
        stack.push_u8(a >= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GTE_int16_t(RuntimeStack& stack) {
        i16 b = stack.pop_i16();
        i16 a = stack.pop_i16();
        stack.push_u8(a >= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_GTE_int32_t(RuntimeStack& stack) {
        i32 b = stack.pop_i32();
        i32 a = stack.pop_i32();
        stack.push_u8(a >= b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_GTE_int64_t(RuntimeStack& stack) {
        i64 b = stack.pop_i64();
        i64 a = stack.pop_i64();
        stack.push_u8(a >= b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
    RuntimeError CMP_GTE_float(RuntimeStack& stack) {
        float b = stack.pop_f32();
        float a = stack.pop_f32();
        stack.push_u8(a >= b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_GTE_double(RuntimeStack& stack) {
        double b = stack.pop_f64();
        double a = stack.pop_f64();
        stack.push_u8(a >= b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS

    RuntimeError CMP_LTE_uint8_t(RuntimeStack& stack) {
        u8 b = stack.pop_u8();
        u8 a = stack.pop_u8();
        stack.push_u8(a <= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LTE_uint16_t(RuntimeStack& stack) {
        u16 b = stack.pop_u16();
        u16 a = stack.pop_u16();
        stack.push_u8(a <= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LTE_uint32_t(RuntimeStack& stack) {
        u32 b = stack.pop_u32();
        u32 a = stack.pop_u32();
        stack.push_u8(a <= b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_LTE_uint64_t(RuntimeStack& stack) {
        u64 b = stack.pop_u64();
        u64 a = stack.pop_u64();
        stack.push_u8(a <= b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
    RuntimeError CMP_LTE_int8_t(RuntimeStack& stack) {
        i8 b = stack.pop_i8();
        i8 a = stack.pop_i8();
        stack.push_u8(a <= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LTE_int16_t(RuntimeStack& stack) {
        i16 b = stack.pop_i16();
        i16 a = stack.pop_i16();
        stack.push_u8(a <= b);
        return STATUS_SUCCESS;
    }
    RuntimeError CMP_LTE_int32_t(RuntimeStack& stack) {
        i32 b = stack.pop_i32();
        i32 a = stack.pop_i32();
        stack.push_u8(a <= b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_LTE_int64_t(RuntimeStack& stack) {
        i64 b = stack.pop_i64();
        i64 a = stack.pop_i64();
        stack.push_u8(a <= b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
    RuntimeError CMP_LTE_float(RuntimeStack& stack) {
        float b = stack.pop_f32();
        float a = stack.pop_f32();
        stack.push_u8(a <= b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_LTE_double(RuntimeStack& stack) {
        double b = stack.pop_f64();
        double a = stack.pop_f64();
        stack.push_u8(a <= b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
}