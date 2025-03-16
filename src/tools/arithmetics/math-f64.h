// math-f64.h - 2022-12-11
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

#ifdef USE_X64_OPS
namespace PLCMethods {

    RuntimeError ADD_double(RuntimeStack& stack) {
        f64 b = stack.pop_f64();
        f64 a = stack.pop_f64();
        stack.push_f64(a + b);
        return STATUS_SUCCESS;
    }

    RuntimeError SUB_double(RuntimeStack& stack) {
        f64 b = stack.pop_f64();
        f64 a = stack.pop_f64();
        stack.push_f64(a - b);
        return STATUS_SUCCESS;
    }

    RuntimeError MUL_double(RuntimeStack& stack) {
        f64 b = stack.pop_f64();
        f64 a = stack.pop_f64();
        stack.push_f64(a * b);
        return STATUS_SUCCESS;
    }

    RuntimeError DIV_double(RuntimeStack& stack) {
        f64 b = stack.pop_f64();
        f64 a = stack.pop_f64();
        stack.push_f64(a / b);
        return STATUS_SUCCESS;
    }

    RuntimeError MOD_double(RuntimeStack& stack) {
        f64 b = stack.pop_f64();
        f64 a = stack.pop_f64();
        stack.push_f64(fmod(a, b));
        return STATUS_SUCCESS;
    }

    RuntimeError POW_double(RuntimeStack& stack) {
        f64 b = stack.pop_f64();
        f64 a = stack.pop_f64();
        stack.push_f64(pow(a, b));
        return STATUS_SUCCESS;
    }

    RuntimeError NEG_double(RuntimeStack& stack) {
        f64 a = stack.pop_f64();
        stack.push_f64(-a);
        return STATUS_SUCCESS;
    }

    RuntimeError ABS_double(RuntimeStack& stack) {
        f64 a = stack.pop_f64();
        stack.push_f64(fabs(a));
        return STATUS_SUCCESS;
    }

    RuntimeError SQRT_double(RuntimeStack& stack) {
        f64 a = stack.pop_f64();
        stack.push_f64(sqrt(a));
        return STATUS_SUCCESS;
    }

    RuntimeError SIN_double(RuntimeStack& stack) {
        f64 a = stack.pop_f64();
        stack.push_f64(sin(a));
        return STATUS_SUCCESS;
    }

    RuntimeError COS_double(RuntimeStack& stack) {
        f64 a = stack.pop_f64();
        stack.push_f64(cos(a));
        return STATUS_SUCCESS;
    }

}

#endif // USE_X64_OPS