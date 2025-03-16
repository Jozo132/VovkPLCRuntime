// math-i8.h - 2022-12-11
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

    RuntimeError ADD_int8_t(RuntimeStack& stack) {
        i8 b = stack.pop_i8();
        i8 a = stack.pop_i8();
        stack.push_i8(a + b);
        return STATUS_SUCCESS;
    }

    RuntimeError SUB_int8_t(RuntimeStack& stack) {
        i8 b = stack.pop_i8();
        i8 a = stack.pop_i8();
        stack.push_i8(a - b);
        return STATUS_SUCCESS;
    }

    RuntimeError MUL_int8_t(RuntimeStack& stack) {
        i8 b = stack.pop_i8();
        i8 a = stack.pop_i8();
        stack.push_i8(a * b);
        return STATUS_SUCCESS;
    }

    RuntimeError DIV_int8_t(RuntimeStack& stack) {
        i8 b = stack.pop_i8();
        i8 a = stack.pop_i8();
        stack.push_i8(a / b);
        return STATUS_SUCCESS;
    }

    RuntimeError MOD_int8_t(RuntimeStack& stack) {
        i8 b = stack.pop_i8();
        i8 a = stack.pop_i8();
        stack.push_i8(a % b);
        return STATUS_SUCCESS;
    }

    RuntimeError POW_int8_t(RuntimeStack& stack) {
        i8 b = stack.pop_i8();
        i8 a = stack.pop_i8();
        stack.push_i8(pow(a, b));
        return STATUS_SUCCESS;
    }

    RuntimeError NEG_int8_t(RuntimeStack& stack) {
        i8 a = stack.pop_i8();
        stack.push_i8(-a);
        return STATUS_SUCCESS;
    }

    RuntimeError ABS_int8_t(RuntimeStack& stack) {
        i8 a = stack.pop_i8();
        stack.push_i8(abs(a));
        return STATUS_SUCCESS;
    }




}