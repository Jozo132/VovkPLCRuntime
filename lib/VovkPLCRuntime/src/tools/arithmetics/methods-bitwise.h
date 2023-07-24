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
    
    RuntimeError handle_GET_X8_B0(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a & 0x01 >> 0); return STATUS_SUCCESS; }
    RuntimeError handle_GET_X8_B1(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a & 0x02 >> 1); return STATUS_SUCCESS; }
    RuntimeError handle_GET_X8_B2(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a & 0x04 >> 2); return STATUS_SUCCESS; }
    RuntimeError handle_GET_X8_B3(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a & 0x08 >> 3); return STATUS_SUCCESS; }
    RuntimeError handle_GET_X8_B4(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a & 0x10 >> 4); return STATUS_SUCCESS; }
    RuntimeError handle_GET_X8_B5(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a & 0x20 >> 5); return STATUS_SUCCESS; }
    RuntimeError handle_GET_X8_B6(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a & 0x40 >> 6); return STATUS_SUCCESS; }
    RuntimeError handle_GET_X8_B7(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a & 0x80 >> 7); return STATUS_SUCCESS; }

    RuntimeError handle_SET_X8_B0(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a | 1 << 0); return STATUS_SUCCESS; }
    RuntimeError handle_SET_X8_B1(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a | 1 << 1); return STATUS_SUCCESS; }
    RuntimeError handle_SET_X8_B2(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a | 1 << 2); return STATUS_SUCCESS; }
    RuntimeError handle_SET_X8_B3(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a | 1 << 3); return STATUS_SUCCESS; }
    RuntimeError handle_SET_X8_B4(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a | 1 << 4); return STATUS_SUCCESS; }
    RuntimeError handle_SET_X8_B5(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a | 1 << 5); return STATUS_SUCCESS; }
    RuntimeError handle_SET_X8_B6(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a | 1 << 6); return STATUS_SUCCESS; }
    RuntimeError handle_SET_X8_B7(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a | 1 << 7); return STATUS_SUCCESS; }

    RuntimeError handle_RSET_X8_B0(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a & ~(1 << 0)); return STATUS_SUCCESS; }
    RuntimeError handle_RSET_X8_B1(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a & ~(1 << 1)); return STATUS_SUCCESS; }
    RuntimeError handle_RSET_X8_B2(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a & ~(1 << 2)); return STATUS_SUCCESS; }
    RuntimeError handle_RSET_X8_B3(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a & ~(1 << 3)); return STATUS_SUCCESS; }
    RuntimeError handle_RSET_X8_B4(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a & ~(1 << 4)); return STATUS_SUCCESS; }
    RuntimeError handle_RSET_X8_B5(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a & ~(1 << 5)); return STATUS_SUCCESS; }
    RuntimeError handle_RSET_X8_B6(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a & ~(1 << 6)); return STATUS_SUCCESS; }
    RuntimeError handle_RSET_X8_B7(RuntimeStack* stack) { uint8_t a = stack->pop_uint8_t(); stack->push_uint8_t(a & ~(1 << 7)); return STATUS_SUCCESS; }
    
    RuntimeError handle_BW_AND_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a & b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_AND_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a & b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_AND_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a & b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_AND_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a & b);
        return STATUS_SUCCESS;
    }

    RuntimeError handle_BW_OR_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a | b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_OR_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a | b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_OR_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a | b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_OR_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a | b);
        return STATUS_SUCCESS;
    }

    RuntimeError handle_BW_XOR_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a ^ b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_XOR_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a ^ b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_XOR_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a ^ b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_XOR_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a ^ b);
        return STATUS_SUCCESS;
    }

    RuntimeError handle_BW_NOT_X8(RuntimeStack* stack) {
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(~a);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_NOT_X16(RuntimeStack* stack) {
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(~a);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_NOT_X32(RuntimeStack* stack) {
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(~a);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_NOT_X64(RuntimeStack* stack) {
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(~a);
        return STATUS_SUCCESS;
    }

    RuntimeError handle_BW_LSHIFT_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a << b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_LSHIFT_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a << b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_LSHIFT_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a << b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_LSHIFT_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a << b);
        return STATUS_SUCCESS;
    }

    RuntimeError handle_BW_RSHIFT_X8(RuntimeStack* stack) {
        uint8_t b = stack->pop_uint8_t();
        uint8_t a = stack->pop_uint8_t();
        stack->push_uint8_t(a >> b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_RSHIFT_X16(RuntimeStack* stack) {
        uint16_t b = stack->pop_uint16_t();
        uint16_t a = stack->pop_uint16_t();
        stack->push_uint16_t(a >> b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_RSHIFT_X32(RuntimeStack* stack) {
        uint32_t b = stack->pop_uint32_t();
        uint32_t a = stack->pop_uint32_t();
        stack->push_uint32_t(a >> b);
        return STATUS_SUCCESS;
    }
    RuntimeError handle_BW_RSHIFT_X64(RuntimeStack* stack) {
        uint64_t b = stack->pop_uint64_t();
        uint64_t a = stack->pop_uint64_t();
        stack->push_uint64_t(a >> b);
        return STATUS_SUCCESS;
    }

}