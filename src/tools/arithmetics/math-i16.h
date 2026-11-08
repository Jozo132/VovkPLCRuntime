// math-i16.h - 2022-12-11
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

    RuntimeError ADD_int16_t(RuntimeStack& stack) {
        u16 b = stack.pop_u16();
        u16 a = stack.pop_u16();
        stack.push_u16((u16)((u32)a + (u32)b));
        return STATUS_SUCCESS;
    }
    
    RuntimeError SUB_int16_t(RuntimeStack& stack) {
        u16 b = stack.pop_u16();
        u16 a = stack.pop_u16();
        stack.push_u16((u16)((u32)a - (u32)b));
        return STATUS_SUCCESS;
    }

    RuntimeError MUL_int16_t(RuntimeStack& stack) {
        u16 b = stack.pop_u16();
        u16 a = stack.pop_u16();
        stack.push_u16((u16)((u32)a * (u32)b));
        return STATUS_SUCCESS;
    }

    RuntimeError DIV_int16_t(RuntimeStack& stack) {
        i16 b = stack.pop_i16();
        i16 a = stack.pop_i16();
        if (b == 0 || (b == -1 && a == -32768)) return EXECUTION_ERROR;
        stack.push_i16(a / b);
        return STATUS_SUCCESS;
    }
    
    RuntimeError MOD_int16_t(RuntimeStack& stack) {
        i16 b = stack.pop_i16();
        i16 a = stack.pop_i16();
        if (b == 0 || (b == -1 && a == -32768)) return EXECUTION_ERROR;
        stack.push_i16(a % b);
        return STATUS_SUCCESS;
    }

    RuntimeError POW_int16_t(RuntimeStack& stack) {
        i16 b = stack.pop_i16();
        i16 a = stack.pop_i16();
        stack.push_i16(pow(a, b));
        return STATUS_SUCCESS;
    }

    RuntimeError NEG_int16_t(RuntimeStack& stack) {
        u16 a = stack.pop_u16();
        stack.push_u16((u16)((u32)0 - (u32)a));
        return STATUS_SUCCESS;
    }

    RuntimeError ABS_int16_t(RuntimeStack& stack) {
        i16 a = stack.pop_i16();
        u16 value = (u16)a;
        stack.push_u16(a < 0 ? (u16)((u32)0 - (u32)value) : value);
        return STATUS_SUCCESS;
    }
}