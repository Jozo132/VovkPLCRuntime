#pragma once

#if !defined(ARDUINO_ARCH_ESP32)
#error PLCRUNTIME_WIFI_MANAGER currently requires ESP32
#endif

#include <WiFi.h>
#include <Preferences.h>

class PLCWiFiManager {
    struct Network {
        char ssid[33];
        char password[65];
    };
    Network networks[3] = {};
    Preferences preferences;
    int current = -1;
    unsigned long lastAttempt = 0;
    bool attempting = false;
    bool storageReady = false;

    bool saveNetworks(Network* updated) {
        if (preferences.putBytes("networks", updated, sizeof(networks)) != sizeof(networks)) return false;
        memcpy(networks, updated, sizeof(networks));
        for (int index = 0; index < 3; index++) {
            char key[] = "network0";
            key[7] += index;
            if (preferences.isKey(key)) preferences.remove(key);
        }
        return true;
    }

    void printHexString(const char* text) {
        while (*text) {
            char high, low;
            byteToHex((uint8_t)*text++, high, low);
            Serial.print(high);
            Serial.print(low);
        }
    }

public:
    void begin() {
        storageReady = preferences.begin("plc-wifi", false);
        if (storageReady) {
            if (preferences.getBytesLength("networks") == sizeof(networks)) {
                preferences.getBytes("networks", networks, sizeof(networks));
            } else {
                for (int index = 0; index < 3; index++) {
                    char key[] = "network0";
                    key[7] += index;
                    if (preferences.getBytesLength(key) == sizeof(Network)) {
                        preferences.getBytes(key, &networks[index], sizeof(Network));
                    }
                }
            }
            for (int index = 0; index < 3; index++) {
                networks[index].ssid[32] = 0;
                networks[index].password[64] = 0;
            }
        }
        WiFi.persistent(false);
        WiFi.mode(WIFI_STA);
        WiFi.setAutoReconnect(false);
    }

    void poll() {
        if (WiFi.status() == WL_CONNECTED || WiFi.scanComplete() == WIFI_SCAN_RUNNING) return;
        if (attempting && millis() - lastAttempt < 15000) return;
        if (!attempting && millis() - lastAttempt < 1000) return;
        for (int offset = 1; offset <= 3; offset++) {
            int next = (current + offset) % 3;
            if (networks[next].ssid[0]) {
                current = next;
                WiFi.disconnect();
                WiFi.begin(networks[next].ssid, networks[next].password);
                lastAttempt = millis();
                attempting = true;
                return;
            }
        }
        lastAttempt = millis();
        attempting = false;
    }

    bool configure(uint8_t index, const char* ssid, const char* password, bool replacePassword) {
        if (!storageReady || index >= 3 || (!replacePassword && strcmp(ssid, networks[index].ssid))) return false;
        Network updated[3];
        memcpy(updated, networks, sizeof(networks));
        strcpy(updated[index].ssid, ssid);
        if (!ssid[0]) memset(updated[index].password, 0, sizeof(updated[index].password));
        else if (replacePassword) strcpy(updated[index].password, password);
        if (!saveNetworks(updated)) return false;
        WiFi.disconnect();
        current = -1;
        attempting = false;
        lastAttempt = 0;
        return true;
    }

    bool move(uint8_t from, uint8_t to) {
        if (!storageReady || from >= 3 || to >= 3 || !networks[from].ssid[0]) return false;
        if (from == to) return true;
        Network updated[3];
        memcpy(updated, networks, sizeof(networks));
        Network moved = updated[from];
        if (from < to) {
            for (int index = from; index < to; index++) updated[index] = updated[index + 1];
        } else {
            for (int index = from; index > to; index--) updated[index] = updated[index - 1];
        }
        updated[to] = moved;
        if (!saveNetworks(updated)) return false;
        if (current == from) current = to;
        else if (from < to && current > from && current <= to) current--;
        else if (from > to && current >= to && current < from) current++;
        return true;
    }

    bool forget(uint8_t index) {
        if (!storageReady || index >= 3 || !networks[index].ssid[0]) return false;
        Network updated[3];
        memcpy(updated, networks, sizeof(networks));
        for (int position = index; position < 2; position++) updated[position] = updated[position + 1];
        memset(&updated[2], 0, sizeof(Network));
        if (!saveNetworks(updated)) return false;
        if (current == index) {
            WiFi.disconnect();
            current = -1;
            attempting = false;
            lastAttempt = 0;
        } else if (current > index) current--;
        return true;
    }

    void printStatus() {
        Serial.print(F("WG:"));
        Serial.print(WiFi.status() == WL_CONNECTED ? current : -1);
        Serial.print(':');
        Serial.print(WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : String());
        for (int index = 0; index < 3; index++) {
            Serial.print(':');
            printHexString(networks[index].ssid);
            Serial.print(',');
            Serial.print(networks[index].password[0] ? 1 : 0);
        }
        Serial.println();
    }

    bool startScan() {
        if (WiFi.scanComplete() == WIFI_SCAN_RUNNING) return false;
        WiFi.scanDelete();
        return WiFi.scanNetworks(true, true) == WIFI_SCAN_RUNNING;
    }

    void printScan() {
        int count = WiFi.scanComplete();
        Serial.print(F("WR:"));
        Serial.print(count);
        if (count >= 0) {
            for (int index = 0; index < count; index++) {
                Serial.print(':');
                printHexString(WiFi.SSID(index).c_str());
                Serial.print(',');
                Serial.print(WiFi.RSSI(index));
                Serial.print(',');
                Serial.print(WiFi.encryptionType(index) == WIFI_AUTH_OPEN ? 0 : 1);
            }
        }
        Serial.println();
    }
};