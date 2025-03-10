// crc8.h - 1.0.0 - 2023-08-03
//
// Copyright (c) 2023 J.Vovk
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

#include "../runtime-tools.h"

u8 crc8_simple(u8& crc, const u8* data, u32 size) {
    if (data == nullptr) return 0xff;
    while (size--) {
        crc ^= *data++;
        for (u8 k = 0; k < 8; k++)
            crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
    }
    crc &= 0xff;
    return crc;
}


u8 crc8_simple(u8& crc, u8 data) {
    crc ^= data;
    for (u8 k = 0; k < 8; k++)
        crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
    crc &= 0xff;
    return crc;
}