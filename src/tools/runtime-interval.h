// runtime-interval.h - 2023-08-05
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

#include "runtime-tools.h"

volatile u32 interval_millis_now = 0;
volatile u32 interval_millis_last = 0;
volatile u32 interval_counter_50ms = 0;
volatile u32 uptime_seconds = 0;

volatile u8 interval_time_seconds = 0;
volatile u8 interval_time_minutes = 0;
volatile u8 interval_time_hours = 0;
volatile u8 interval_time_days = 0;

volatile bool P_50ms = false;
volatile bool P_100ms = false;
volatile bool P_200ms = false;
volatile bool P_300ms = false;
volatile bool P_500ms = false;
volatile bool P_1s = false;
volatile bool P_2s = false;
volatile bool P_5s = false;
volatile bool P_10s = false;
volatile bool P_30s = false;
volatile bool P_1min = false;
volatile bool P_2min = false;
volatile bool P_5min = false;
volatile bool P_10min = false;
volatile bool P_15min = false;
volatile bool P_30min = false;
volatile bool P_1hr = false;
volatile bool P_2hr = false;
volatile bool P_3hr = false;
volatile bool P_4hr = false;
volatile bool P_5hr = false;
volatile bool P_6hr = false;
volatile bool P_12hr = false;
volatile bool P_1day = false;

volatile bool S_100ms = false;
volatile bool S_200ms = false;
volatile bool S_300ms = false;
volatile bool S_500ms = false;
volatile bool S_1s = false;
volatile bool S_2s = false;
volatile bool S_5s = false;
volatile bool S_10s = false;
volatile bool S_30s = false;
volatile bool S_1min = false;
volatile bool S_2min = false;
volatile bool S_5min = false;
volatile bool S_10min = false;
volatile bool S_15min = false;
volatile bool S_30min = false;
volatile bool S_1hr = false;
volatile bool S_2hr = false;

volatile u32 P_1day_hour_cnt = 0;
volatile u32 P_12hr_hour_cnt = 0;
volatile u32 P_6hr_hour_cnt = 0;
volatile u32 P_3hr_hour_cnt = 0;
volatile u32 P_2hr_hour_cnt = 0;
volatile u32 P_1hr_min_cnt = 0;
volatile u32 P_30min_min_cnt = 0;
volatile u32 P_15min_min_cnt = 0;
volatile u32 P_10min_min_cnt = 0;
volatile u32 P_5min_min_cnt = 0;
volatile u32 P_2min_sec_cnt = 0;
volatile u32 P_1min_sec_cnt = 0;
volatile u32 P_30s_sec_cnt = 0;
volatile u32 P_10s_sec_cnt = 0;
volatile u32 P_5s_sec_cnt = 0;
volatile u32 P_2s_sec_cnt = 0;

void IntervalGlobalLoopCheck() {
    P_1day = false;
    P_12hr = false;
    P_6hr = false;
    P_5hr = false;
    P_4hr = false;
    P_3hr = false;
    P_2hr = false;
    P_1hr = false;
    P_30min = false;
    P_15min = false;
    P_10min = false;
    P_5min = false;
    P_2min = false;
    P_1min = false;
    P_30s = false;
    P_10s = false;
    P_5s = false;
    P_2s = false;
    P_1s = false;
    P_500ms = false;
    P_300ms = false;
    P_200ms = false;
    P_100ms = false;
    P_50ms = false;
    volatile u32 t = millis();
    if (t == interval_millis_now) return; // No need to check if the time hasn't changed
    interval_millis_now = t;

    if (interval_millis_last > t) {
        interval_millis_last = t;
        P_50ms = true;
        interval_counter_50ms++;
    }
    u32 diff = t - interval_millis_last;
    while (diff >= 50) {
        P_50ms = true;
        interval_counter_50ms++;
        interval_millis_last += 50;
        diff -= 50;
    }
    if (!P_50ms) return; // No need to check if the time hasn't changed
    P_100ms = interval_counter_50ms % 2 == 0;
    P_200ms = interval_counter_50ms % 4 == 0;
    P_300ms = interval_counter_50ms % 6 == 0;
    P_500ms = interval_counter_50ms % 10 == 0;
    P_1s = interval_counter_50ms % 20 == 0;
    
    S_100ms = !S_100ms; // The 50ms pulse is the half period of 100ms square wave
    if (interval_counter_50ms % 2 == 0) S_200ms = !S_200ms;
    if (interval_counter_50ms % 3 == 0) S_300ms = !S_300ms;
    if (interval_counter_50ms % 5 == 0) S_500ms = !S_500ms;
    if (interval_counter_50ms % 10 == 0) S_1s = !S_1s;
    if (interval_counter_50ms % 50 == 0) S_5s = !S_5s;
    if (interval_counter_50ms % 100 == 0) S_10s = !S_10s;
    if (interval_counter_50ms % 300 == 0) S_30s = !S_30s;
    if (interval_counter_50ms % 600 == 0) S_1min = !S_1min;
    if (interval_counter_50ms % 1200 == 0) S_2min = !S_2min;
    if (interval_counter_50ms % 3000 == 0) S_5min = !S_5min;
    if (interval_counter_50ms % 6000 == 0) S_10min = !S_10min;
    if (interval_counter_50ms % 9000 == 0) S_15min = !S_15min;
    if (interval_counter_50ms % 18000 == 0) S_30min = !S_30min;
    if (interval_counter_50ms % 36000 == 0) S_1hr = !S_1hr;
    if (interval_counter_50ms % 72000 == 0) S_2hr = !S_2hr;

    if (P_1s) {
        S_2s = !S_2s;
        uptime_seconds++;
        interval_time_seconds = (interval_time_seconds + 1) % 60;
        if (interval_time_seconds == 0) {
            interval_time_minutes = (interval_time_minutes + 1) % 60;
            if (interval_time_minutes == 0) {
                interval_time_hours = (interval_time_hours + 1) % 24;
                if (interval_time_hours == 0) {
                    interval_time_days = (interval_time_days + 1) % 100;
                }
            }
        }
        P_2s_sec_cnt++;
        P_5s_sec_cnt++;
        P_10s_sec_cnt++;
        if (P_2s_sec_cnt >= 2) {
            P_2s_sec_cnt = 0;
            P_2s = true;
        }
        if (P_5s_sec_cnt >= 5) {
            S_10s = !S_10s;
            P_5s_sec_cnt = 0;
            P_5s = true;
        }
        if (P_10s_sec_cnt >= 10) {
            P_10s_sec_cnt = 0;
            P_10s = true;
        }
    }
    if (P_10s) {
        P_30s_sec_cnt++;
        P_1min_sec_cnt++;
        if (P_30s_sec_cnt >= 30) {
            P_30s_sec_cnt = 0;
            P_30s = true;
        }
        if (P_1min_sec_cnt >= 60) {
            P_1min_sec_cnt = 0;
            P_1min = true;
        }
        if (P_1min) {
            P_2min_sec_cnt++;
            P_5min_min_cnt++;
            P_10min_min_cnt++;
            P_15min_min_cnt++;
            P_30min_min_cnt++;
            P_1hr_min_cnt++;
            if (P_2min_sec_cnt >= 2) {
                P_2min_sec_cnt = 0;
                P_2min = true;
            }
            if (P_5min_min_cnt >= 5) {
                P_5min_min_cnt = 0;
                P_5min = true;
            }
            if (P_10min_min_cnt >= 10) {
                P_10min_min_cnt = 0;
                P_10min = true;
            }
            if (P_15min_min_cnt >= 15) {
                P_15min_min_cnt = 0;
                P_15min = true;
            }
            if (P_30min_min_cnt >= 30) {
                P_30min_min_cnt = 0;
                P_30min = true;
            }
            if (P_1hr_min_cnt >= 60) {
                P_1hr_min_cnt = 0;
                P_1hr = true;
            }
            if (P_1hr) {
                P_2hr_hour_cnt++;
                P_3hr_hour_cnt++;
                P_6hr_hour_cnt++;
                P_12hr_hour_cnt++;
                P_1day_hour_cnt++;
                if (P_2hr_hour_cnt >= 2) {
                    P_2hr_hour_cnt = 0;
                    P_2hr = true;
                }
                if (P_3hr_hour_cnt >= 3) {
                    P_3hr_hour_cnt = 0;
                    P_3hr = true;
                }
                if (P_6hr_hour_cnt >= 6) {
                    P_6hr_hour_cnt = 0;
                    P_6hr = true;
                }
                if (P_12hr_hour_cnt >= 12) {
                    P_12hr_hour_cnt = 0;
                    P_12hr = true;
                }
                if (P_1day_hour_cnt >= 24) {
                    P_1day_hour_cnt = 0;
                    P_1day = true;
                }
            }
        }
    }
}