// math-u32.h - 1.0.0 - 2022-12-11
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

    RuntimeError ADD_uint32_t(RuntimeStack* stack) {
        u32 b = stack->pop_u32();
        u32 a = stack->pop_u32();
        stack->push_u32(a + b);
        return STATUS_SUCCESS;
    }
    
    RuntimeError SUB_uint32_t(RuntimeStack* stack) {
        u32 b = stack->pop_u32();
        u32 a = stack->pop_u32();
        stack->push_u32(a - b);
        return STATUS_SUCCESS;
    }

    RuntimeError MUL_uint32_t(RuntimeStack* stack) {
        u32 b = stack->pop_u32();
        u32 a = stack->pop_u32();
        stack->push_u32(a * b);
        return STATUS_SUCCESS;
    }

    RuntimeError DIV_uint32_t(RuntimeStack* stack) {
        u32 b = stack->pop_u32();
        u32 a = stack->pop_u32();
        stack->push_u32(a / b);
        return STATUS_SUCCESS;
    }
    
}