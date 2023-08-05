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

uint32_t interval_millis_now = 0;
uint32_t interval_millis_last = 0;
uint32_t interval_counter_100ms = 0;

uint8_t interval_time_seconds = 0;
uint8_t interval_time_minutes = 0;
uint8_t interval_time_hours = 0;
uint8_t interval_time_days = 0;

bool P_100ms = false;
bool P_200ms = false;
bool P_300ms = false;
bool P_500ms = false;
bool P_1s = false;
bool P_2s = false;
bool P_5s = false;
bool P_10s = false;
bool P_30s = false;
bool P_1min = false;
bool P_2min = false;
bool P_5min = false;
bool P_10min = false;
bool P_15min = false;
bool P_30min = false;
bool P_1hr = false;
bool P_2hr = false;
bool P_3hr = false;
bool P_4hr = false;
bool P_5hr = false;
bool P_6hr = false;
bool P_12hr = false;
bool P_1day = false;

uint32_t P_1day_hour_cnt = 0;
uint32_t P_12hr_hour_cnt = 0;
uint32_t P_6hr_hour_cnt = 0;
uint32_t P_3hr_hour_cnt = 0;
uint32_t P_2hr_hour_cnt = 0;
uint32_t P_1hr_min_cnt = 0;
uint32_t P_30min_min_cnt = 0;
uint32_t P_15min_min_cnt = 0;
uint32_t P_10min_min_cnt = 0;
uint32_t P_5min_min_cnt = 0;
uint32_t P_2min_sec_cnt = 0;
uint32_t P_1min_sec_cnt = 0;
uint32_t P_30s_sec_cnt = 0;
uint32_t P_10s_sec_cnt = 0;
uint32_t P_5s_sec_cnt = 0;
uint32_t P_2s_sec_cnt = 0;
uint32_t P_1s_sec_cnt = 0;


void IntervalGlobalLoopCheck();

#include "runtime-interval-impl.h"