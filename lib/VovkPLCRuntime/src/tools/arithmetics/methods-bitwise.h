// methods-bitwise.h - 1.0.0 - 2022-12-11
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
    
    RuntimeError BW_AND_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a & b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_AND_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a & b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_AND_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a & b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_AND_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a & b);
        return STATUS_SUCCESS;
    }

    RuntimeError BW_OR_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a | b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_OR_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a | b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_OR_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a | b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_OR_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a | b);
        return STATUS_SUCCESS;
    }

    RuntimeError BW_XOR_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a ^ b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_XOR_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a ^ b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_XOR_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a ^ b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_XOR_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a ^ b);
        return STATUS_SUCCESS;
    }

    RuntimeError BW_NOT_X8(RuntimeStack* stack) {
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(~a);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_NOT_X16(RuntimeStack* stack) {
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(~a);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_NOT_X32(RuntimeStack* stack) {
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(~a);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_NOT_X64(RuntimeStack* stack) {
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(~a);
        return STATUS_SUCCESS;
    }

    RuntimeError BW_LSHIFT_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a << b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_LSHIFT_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a << b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_LSHIFT_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a << b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_LSHIFT_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a << b);
        return STATUS_SUCCESS;
    }

    RuntimeError BW_RSHIFT_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a >> b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_RSHIFT_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a >> b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_RSHIFT_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a >> b);
        return STATUS_SUCCESS;
    }
    RuntimeError BW_RSHIFT_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a >> b);
        return STATUS_SUCCESS;
    }

}