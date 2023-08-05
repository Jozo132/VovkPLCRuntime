// runtime-interval.h - 1.0.0 - 2023-08-05
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

#ifndef __WASM__
#include <Arduino.h>
#endif // __WASM__

extern uint32_t interval_counter_100ms;

extern uint8_t interval_time_seconds;
extern uint8_t interval_time_minutes;
extern uint8_t interval_time_hours;
extern uint8_t interval_time_days;

extern bool P_100ms;
extern bool P_200ms;
extern bool P_300ms;
extern bool P_500ms;
extern bool P_1s;
extern bool P_2s;
extern bool P_5s;
extern bool P_10s;
extern bool P_30s;
extern bool P_1min;
extern bool P_2min;
extern bool P_5min;
extern bool P_10min;
extern bool P_15min;
extern bool P_30min;
extern bool P_1hr;
extern bool P_2hr;
extern bool P_3hr;
extern bool P_4hr;
extern bool P_5hr;
extern bool P_6hr;
extern bool P_12hr;
extern bool P_1day;

void IntervalGlobalLoopCheck();
