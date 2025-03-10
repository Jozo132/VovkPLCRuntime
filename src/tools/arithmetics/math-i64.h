// math-i64.h - 1.0.0 - 2022-12-11
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

    RuntimeError ADD_int64_t(RuntimeStack& stack) {
        i64 b = stack.pop_i64();
        i64 a = stack.pop_i64();
        stack.push_i64(a + b);
        return STATUS_SUCCESS;
    }

    RuntimeError SUB_int64_t(RuntimeStack& stack) {
        i64 b = stack.pop_i64();
        i64 a = stack.pop_i64();
        stack.push_i64(a - b);
        return STATUS_SUCCESS;
    }

    RuntimeError MUL_int64_t(RuntimeStack& stack) {
        i64 b = stack.pop_i64();
        i64 a = stack.pop_i64();
        stack.push_i64(a * b);
        return STATUS_SUCCESS;
    }

    RuntimeError DIV_int64_t(RuntimeStack& stack) {
        i64 b = stack.pop_i64();
        i64 a = stack.pop_i64();
        stack.push_i64(a / b);
        return STATUS_SUCCESS;
    }

    RuntimeError MOD_int64_t(RuntimeStack& stack) {
        i64 b = stack.pop_i64();
        i64 a = stack.pop_i64();
        stack.push_i64(a % b);
        return STATUS_SUCCESS;
    }

    RuntimeError POW_int64_t(RuntimeStack& stack) {
        i64 b = stack.pop_i64();
        i64 a = stack.pop_i64();
        stack.push_i64(pow(a, b));
        return STATUS_SUCCESS;
    }

    RuntimeError NEG_int64_t(RuntimeStack& stack) {
        i64 a = stack.pop_i64();
        stack.push_i64(-a);
        return STATUS_SUCCESS;
    }

    RuntimeError ABS_int64_t(RuntimeStack& stack) {
        i64 a = stack.pop_i64();
        stack.push_i64(abs(a));
        return STATUS_SUCCESS;
    }

}


#endif // USE_X64_OPS