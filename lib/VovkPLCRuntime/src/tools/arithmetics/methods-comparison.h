// methods-comparison.h - 1.0.0 - 2022-12-11
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
#ifdef USE_X64_OPS
    RuntimeError CMP_EQ_uint64_t(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint8_t(a == b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
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
#ifdef USE_X64_OPS
    RuntimeError CMP_EQ_int64_t(RuntimeStack* stack) {
        int64_t b = stack->pop_int64_t();
        int64_t a = stack->pop_int64_t();
        stack->push_uint8_t(a == b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
    RuntimeError CMP_EQ_float(RuntimeStack* stack) {
        float b = stack->pop_float();
        float a = stack->pop_float();
        stack->push_uint8_t(a == b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_EQ_double(RuntimeStack* stack) {
        double b = stack->pop_double();
        double a = stack->pop_double();
        stack->push_uint8_t(a == b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS

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
#ifdef USE_X64_OPS
    RuntimeError CMP_NEQ_uint64_t(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint8_t(a != b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
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
#ifdef USE_X64_OPS
    RuntimeError CMP_NEQ_int64_t(RuntimeStack* stack) {
        int64_t b = stack->pop_int64_t();
        int64_t a = stack->pop_int64_t();
        stack->push_uint8_t(a != b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
    RuntimeError CMP_NEQ_float(RuntimeStack* stack) {
        float b = stack->pop_float();
        float a = stack->pop_float();
        stack->push_uint8_t(a != b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_NEQ_double(RuntimeStack* stack) {
        double b = stack->pop_double();
        double a = stack->pop_double();
        stack->push_uint8_t(a != b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS

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
#ifdef USE_X64_OPS
    RuntimeError CMP_GT_uint64_t(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint8_t(a > b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
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
#ifdef USE_X64_OPS
    RuntimeError CMP_GT_int64_t(RuntimeStack* stack) {
        int64_t b = stack->pop_int64_t();
        int64_t a = stack->pop_int64_t();
        stack->push_uint8_t(a > b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
    RuntimeError CMP_GT_float(RuntimeStack* stack) {
        float b = stack->pop_float();
        float a = stack->pop_float();
        stack->push_uint8_t(a > b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_GT_double(RuntimeStack* stack) {
        double b = stack->pop_double();
        double a = stack->pop_double();
        stack->push_uint8_t(a > b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS

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
#ifdef USE_X64_OPS
    RuntimeError CMP_LT_uint64_t(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint8_t(a < b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
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
#ifdef USE_X64_OPS
    RuntimeError CMP_LT_int64_t(RuntimeStack* stack) {
        int64_t b = stack->pop_int64_t();
        int64_t a = stack->pop_int64_t();
        stack->push_uint8_t(a < b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
    RuntimeError CMP_LT_float(RuntimeStack* stack) {
        float b = stack->pop_float();
        float a = stack->pop_float();
        stack->push_uint8_t(a < b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_LT_double(RuntimeStack* stack) {
        double b = stack->pop_double();
        double a = stack->pop_double();
        stack->push_uint8_t(a < b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS

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
#ifdef USE_X64_OPS
    RuntimeError CMP_GTE_uint64_t(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint8_t(a >= b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
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
#ifdef USE_X64_OPS
    RuntimeError CMP_GTE_int64_t(RuntimeStack* stack) {
        int64_t b = stack->pop_int64_t();
        int64_t a = stack->pop_int64_t();
        stack->push_uint8_t(a >= b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
    RuntimeError CMP_GTE_float(RuntimeStack* stack) {
        float b = stack->pop_float();
        float a = stack->pop_float();
        stack->push_uint8_t(a >= b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_GTE_double(RuntimeStack* stack) {
        double b = stack->pop_double();
        double a = stack->pop_double();
        stack->push_uint8_t(a >= b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS

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
#ifdef USE_X64_OPS
    RuntimeError CMP_LTE_uint64_t(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint8_t(a <= b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
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
#ifdef USE_X64_OPS
    RuntimeError CMP_LTE_int64_t(RuntimeStack* stack) {
        int64_t b = stack->pop_int64_t();
        int64_t a = stack->pop_int64_t();
        stack->push_uint8_t(a <= b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
    RuntimeError CMP_LTE_float(RuntimeStack* stack) {
        float b = stack->pop_float();
        float a = stack->pop_float();
        stack->push_uint8_t(a <= b);
        return STATUS_SUCCESS;
    }
#ifdef USE_X64_OPS
    RuntimeError CMP_LTE_double(RuntimeStack* stack) {
        double b = stack->pop_double();
        double a = stack->pop_double();
        stack->push_uint8_t(a <= b);
        return STATUS_SUCCESS;
    }
#endif // USE_X64_OPS
}