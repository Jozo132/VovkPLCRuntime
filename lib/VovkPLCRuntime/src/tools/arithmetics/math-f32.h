// math-f32.h - 1.0.0 - 2022-12-11
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

    RuntimeError ADD_float(RuntimeStack* stack) {
        f32 b = stack->pop_f32();
        f32 a = stack->pop_f32();
        stack->push_f32(a + b);
        return STATUS_SUCCESS;
    }
    
    RuntimeError SUB_float(RuntimeStack* stack) {
        f32 b = stack->pop_f32();
        f32 a = stack->pop_f32();
        stack->push_f32(a - b);
        return STATUS_SUCCESS;
    }

    RuntimeError MUL_float(RuntimeStack* stack) {
        f32 b = stack->pop_f32();
        f32 a = stack->pop_f32();
        stack->push_f32(a * b);
        return STATUS_SUCCESS;
    }

    RuntimeError DIV_float(RuntimeStack* stack) {
        f32 b = stack->pop_f32();
        f32 a = stack->pop_f32();
        stack->push_f32(a / b);
        return STATUS_SUCCESS;
    }

    RuntimeError MOD_float(RuntimeStack* stack) {
        f32 b = stack->pop_f32();
        f32 a = stack->pop_f32();
        stack->push_f32(fmod(a, b));
        return STATUS_SUCCESS;
    }

    RuntimeError POW_float(RuntimeStack* stack) {
        f32 b = stack->pop_f32();
        f32 a = stack->pop_f32();
        f32 result = 1;
        for (u8 i = 0; i < b; i++) result *= a;
        stack->push_f32(result);
        return STATUS_SUCCESS;
    }

    RuntimeError NEG_float(RuntimeStack* stack) {
        f32 a = stack->pop_f32();
        stack->push_f32(-a);
        return STATUS_SUCCESS;
    }

    RuntimeError ABS_float(RuntimeStack* stack) {
        f32 a = stack->pop_f32();
        stack->push_f32(a < 0 ? -a : a);
        return STATUS_SUCCESS;
    }

    RuntimeError SQRT_float(RuntimeStack* stack) {
        f32 a = stack->pop_f32();
        stack->push_f32(sqrt(a));
        return STATUS_SUCCESS;
    }

    RuntimeError SIN_float(RuntimeStack* stack) {
        f32 a = stack->pop_f32();
        stack->push_f32(sin(a));
        return STATUS_SUCCESS;
    }

    RuntimeError COS_float(RuntimeStack* stack) {
        f32 a = stack->pop_f32();
        stack->push_f32(cos(a));
        return STATUS_SUCCESS;
    }
    
}