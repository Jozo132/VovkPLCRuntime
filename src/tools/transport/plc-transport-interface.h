// plc-transport-interface.h - Transport abstraction layer
//
// Copyright (c) 2022-2026 J.Vovk
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

#ifdef PLCRUNTIME_TRANSPORT

#include <Arduino.h>

/**
 * @brief Abstract transport interface for custom protocols.
 * 
 * Implement this class to add support for any communication protocol:
 * - Modbus TCP/RTU
 * - MQTT
 * - BLE
 * - LoRa
 * - CAN bus
 * - RS485
 * - WebSocket
 * - Custom proprietary protocols
 * 
 * Example:
 * @code
 * class MyCustomTransport : public PLCTransportInterface {
 *     // ... implement all virtual methods
 * };
 * 
 * MyCustomTransport myTransport;
 * runtime.addTransport(&myTransport, PLC_SEC_PASSWORD);
 * @endcode
 */
class PLCTransportInterface {
public:
    virtual ~PLCTransportInterface() = default;
    
    // === Lifecycle ===
    
    /**
     * @brief Initialize the transport
     * @return true if initialization succeeded
     */
    virtual bool begin() = 0;
    
    /**
     * @brief Shutdown the transport
     */
    virtual void end() = 0;
    
    // === Connection State ===
    
    /**
     * @brief Check if a client is connected
     * @return true if connected
     */
    virtual bool connected() = 0;
    
    /**
     * @brief Poll for new connections, maintain state
     * Call this regularly in the main loop
     */
    virtual void poll() = 0;
    
    // === Stream Interface ===
    
    /**
     * @brief Get number of bytes available to read
     * @return Number of bytes in receive buffer
     */
    virtual int available() = 0;
    
    /**
     * @brief Read one byte from the transport
     * @return Byte read, or -1 if none available
     */
    virtual int read() = 0;
    
    /**
     * @brief Peek at the next byte without consuming it
     * @return Next byte, or -1 if none available
     */
    virtual int peek() = 0;
    
    /**
     * @brief Write one byte to the transport
     * @param byte Byte to write
     * @return Number of bytes written (0 or 1)
     */
    virtual size_t write(uint8_t byte) = 0;
    
    /**
     * @brief Write a buffer to the transport
     * @param buffer Data to write
     * @param size Number of bytes to write
     * @return Number of bytes actually written
     */
    virtual size_t write(const uint8_t* buffer, size_t size) = 0;
    
    /**
     * @brief Flush output buffer
     */
    virtual void flush() = 0;
    
    // === Optional: Bulk Read (default implementation provided) ===
    
    /**
     * @brief Read multiple bytes
     * @param buffer Destination buffer
     * @param length Maximum bytes to read
     * @return Number of bytes actually read
     */
    virtual size_t readBytes(uint8_t* buffer, size_t length) {
        size_t count = 0;
        unsigned long start = millis();
        while (count < length && (millis() - start) < 1000) {
            if (available()) {
                int b = read();
                if (b < 0) break;
                buffer[count++] = (uint8_t)b;
            }
        }
        return count;
    }
    
    // === Optional: Identification ===
    
    /**
     * @brief Get transport name for logging
     * @return Human-readable name
     */
    virtual const char* name() { return "Custom"; }
    
    /**
     * @brief Check if this is a network transport
     * @return true for network transports (TCP, UDP, etc.)
     */
    virtual bool isNetworkTransport() { return true; }
    
    // === Optional: Client Info (for logging/security) ===
    
    /**
     * @brief Get client IP address (for network transports)
     * @return IP address string or empty
     */
    virtual const char* clientAddress() { return ""; }
    
    /**
     * @brief Get client port (for network transports)
     * @return Port number or 0
     */
    virtual uint16_t clientPort() { return 0; }
    
    // === Helper: Print support ===
    
    size_t print(const char* str) {
        return write((const uint8_t*)str, strlen(str));
    }
    
    size_t println(const char* str) {
        size_t n = print(str);
        n += write('\n');
        return n;
    }
    
    size_t print(const __FlashStringHelper* str) {
        PGM_P p = reinterpret_cast<PGM_P>(str);
        size_t n = 0;
        while (1) {
            char c = pgm_read_byte(p++);
            if (c == 0) break;
            n += write((uint8_t)c);
        }
        return n;
    }
    
    size_t println(const __FlashStringHelper* str) {
        size_t n = print(str);
        n += write('\n');
        return n;
    }
    
    size_t print(int val) {
        char buf[12];
        itoa(val, buf, 10);
        return print(buf);
    }
    
    size_t println(int val) {
        size_t n = print(val);
        n += write('\n');
        return n;
    }
    
    size_t print(unsigned long val, int base = 10) {
        char buf[12];
        ultoa(val, buf, base);
        return print(buf);
    }
    
    size_t println() {
        return write('\n');
    }
};

#endif // PLCRUNTIME_TRANSPORT
