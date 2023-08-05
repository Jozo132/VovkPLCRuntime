#pragma once

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
    uint32_t t = millis();
    if (t == interval_millis_now) return; // No need to check if the time hasn't changed
    interval_millis_now = t;

    if (interval_millis_last > t) {
        interval_millis_last = t;
        P_100ms = true;
        interval_counter_100ms++;
    }
    uint32_t diff = t - interval_millis_last;
    while (diff >= 100) {
        P_100ms = true;
        interval_counter_100ms++;
        interval_millis_last += 100;
        diff -= 100;
    }
    if (!P_100ms) return; // No need to check if the time hasn't changed


    P_200ms = interval_counter_100ms % 2 == 0;
    P_300ms = interval_counter_100ms % 3 == 0;
    P_500ms = interval_counter_100ms % 5 == 0;
    P_1s = interval_counter_100ms % 10 == 0;

    if (P_1s) {
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
        if (P_2s_sec_cnt >= 2) {
            P_2s_sec_cnt = 0;
            P_2s = true;
        }
        if (P_5s_sec_cnt >= 5) {
            P_5s_sec_cnt = 0;
            P_5s = true;
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
