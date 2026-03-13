// plc-serial-rs232.h - Generic Serial RS232 communication driver
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
//
// ============================================================================
// Generic Serial RS232 Library
// ============================================================================
//
// Enable with: #define PLCRUNTIME_SERIAL_RS232 before including VovkPLCRuntime.h
//
// Provides raw byte-level serial communication through any Arduino Stream
// interface. This is the base protocol for all serial communication — no
// framing, no CRC, just raw TX/RX with configurable delimiter-based or
// length-based message boundaries.
//
// Supports two operation modes:
//   - Raw mode (default): read/write arbitrary byte buffers
//   - Delimiter mode: accumulate incoming bytes until a delimiter is found
//
// ============================================================================
// Usage Examples
// ============================================================================
//
// 1. Raw byte communication:
//    ------------------------
//    #define PLCRUNTIME_SERIAL_RS232
//    #include <VovkPLCRuntime.h>
//
//    SerialRS232 serial0(Serial1, 9600);
//
//    void setup() {
//        serial0.begin();
//    }
//    void loop() {
//        // Write bytes from buffer
//        uint8_t txData[] = { 0x01, 0x02, 0x03 };
//        serial0.write(txData, 3);
//
//        // Read available bytes
//        uint8_t rxBuf[64];
//        uint16_t count = serial0.read(rxBuf, 64);
//    }
//
// 2. Delimiter-based messages (e.g. newline-terminated):
//    ----------------------------------------------------
//    SerialRS232 serial0(Serial1, 9600, '\n');
//
//    void setup() {
//        serial0.begin();
//    }
//    void loop() {
//        serial0.poll();  // Accumulate incoming bytes
//        if (serial0.messageReady()) {
//            uint8_t msg[128];
//            uint16_t len = serial0.readMessage(msg, 128);
//            // Process msg[0..len-1]
//        }
//    }

#pragma once

#ifdef PLCRUNTIME_SERIAL_RS232

#include <Arduino.h>

// ============================================================================
// Configuration
// ============================================================================

#ifndef SERIAL_RS232_RX_BUFFER_SIZE
#define SERIAL_RS232_RX_BUFFER_SIZE 256
#endif

#ifndef SERIAL_RS232_DEFAULT_TIMEOUT_MS
#define SERIAL_RS232_DEFAULT_TIMEOUT_MS 100
#endif

// ============================================================================
// Result Codes
// ============================================================================

enum SerialRS232Result : uint8_t {
    SERIAL_OK                   = 0x00,
    SERIAL_ERR_TIMEOUT          = 0x01,
    SERIAL_ERR_OVERFLOW         = 0x02,
    SERIAL_ERR_NOT_READY        = 0x03,
    SERIAL_ERR_NO_DATA          = 0x04,
    SERIAL_ERR_INVALID_PARAMS   = 0x05,
};

// ============================================================================
// Baud Rate Lookup Table
// ============================================================================
// Used by COMMS_BEGIN config byte to select standard baud rates.
// Index 0 = use stored baudrate (no change).
// ============================================================================

static uint32_t serialBaudFromIndex(uint8_t index) {
    switch (index) {
        case 0:  return 0;       // Use stored baudrate (no change)
        case 1:  return 1200;
        case 2:  return 2400;
        case 3:  return 4800;
        case 4:  return 9600;
        case 5:  return 19200;
        case 6:  return 38400;
        case 7:  return 57600;
        case 8:  return 115200;
        case 9:  return 230400;
        case 10: return 460800;
        case 11: return 921600;
        default: return 9600;
    }
}

// ============================================================================
// SerialRS232 Class
// ============================================================================

class SerialRS232 {
private:
    HardwareSerial* _serial;
    uint32_t _baudrate;
    uint32_t _timeoutMs;
    bool _begun;            // Whether _serial->begin() has been called

    // Receive ring buffer
    uint8_t _rxBuf[SERIAL_RS232_RX_BUFFER_SIZE];
    volatile uint16_t _rxHead;
    volatile uint16_t _rxTail;

    // Delimiter-based message detection
    uint8_t _delimiter;
    bool _useDelimiter;
    volatile bool _msgReady;     // A complete delimited message is in the buffer
    volatile uint16_t _msgLen;   // Length of the next complete message (including delimiter)

    // Statistics
    uint32_t _rxCount;     // Total bytes received
    uint32_t _txCount;     // Total bytes transmitted
    uint32_t _errCount;    // Error count (overflows, etc.)
    SerialRS232Result _lastError;

    // ========================================================================
    // Ring buffer helpers
    // ========================================================================

    uint16_t rxAvailable() const {
        if (_rxHead >= _rxTail) return _rxHead - _rxTail;
        return SERIAL_RS232_RX_BUFFER_SIZE - _rxTail + _rxHead;
    }

    uint16_t rxFree() const {
        return SERIAL_RS232_RX_BUFFER_SIZE - 1 - rxAvailable();
    }

    bool rxPush(uint8_t byte) {
        uint16_t next = (_rxHead + 1) % SERIAL_RS232_RX_BUFFER_SIZE;
        if (next == _rxTail) {
            _errCount++;
            _lastError = SERIAL_ERR_OVERFLOW;
            return false; // Buffer full
        }
        _rxBuf[_rxHead] = byte;
        _rxHead = next;
        return true;
    }

    uint8_t rxPop() {
        if (_rxHead == _rxTail) return 0;
        uint8_t byte = _rxBuf[_rxTail];
        _rxTail = (_rxTail + 1) % SERIAL_RS232_RX_BUFFER_SIZE;
        return byte;
    }

    uint8_t rxPeek(uint16_t offset) const {
        return _rxBuf[(_rxTail + offset) % SERIAL_RS232_RX_BUFFER_SIZE];
    }

    // Scan the ring buffer for the delimiter and update _msgReady/_msgLen
    void scanForMessage() {
        if (_msgReady) return; // Already have a pending message
        uint16_t avail = rxAvailable();
        for (uint16_t i = 0; i < avail; i++) {
            if (rxPeek(i) == _delimiter) {
                _msgReady = true;
                _msgLen = i + 1; // Include the delimiter byte
                return;
            }
        }
    }

public:

    // ========================================================================
    // Constructor
    // ========================================================================

    SerialRS232(HardwareSerial& serial, uint32_t baudrate = 9600, uint8_t delimiter = 0, bool useDelimiter = false)
        : _serial(&serial)
        , _baudrate(baudrate)
        , _timeoutMs(SERIAL_RS232_DEFAULT_TIMEOUT_MS)
        , _begun(false)
        , _rxHead(0)
        , _rxTail(0)
        , _delimiter(delimiter)
        , _useDelimiter(useDelimiter)
        , _msgReady(false)
        , _msgLen(0)
        , _rxCount(0)
        , _txCount(0)
        , _errCount(0)
        , _lastError(SERIAL_OK)
    {}

    // Convenience: delimiter constructor
    SerialRS232(HardwareSerial& serial, uint32_t baudrate, uint8_t delimiter)
        : SerialRS232(serial, baudrate, delimiter, delimiter != 0)
    {}

    // ========================================================================
    // Initialization
    // ========================================================================

    void begin() {
        _serial->begin(_baudrate);
        _begun = true;
        _rxHead = 0;
        _rxTail = 0;
        _msgReady = false;
        _msgLen = 0;
        _lastError = SERIAL_OK;
    }

    // Begin with a specific baud rate (stores and initializes)
    void begin(uint32_t baudrate) {
        _baudrate = baudrate;
        begin();
    }

    // Stop the serial port
    void end() {
        if (_begun) {
            _serial->end();
            _begun = false;
        }
        _rxHead = 0;
        _rxTail = 0;
        _msgReady = false;
        _msgLen = 0;
    }

    // Change baud rate at runtime (auto re-initializes if already begun)
    void setBaudrate(uint32_t baudrate) {
        if (_baudrate == baudrate) return;
        _baudrate = baudrate;
        if (_begun) {
            _serial->end();
            _serial->begin(_baudrate);
            _rxHead = 0;
            _rxTail = 0;
            _msgReady = false;
            _msgLen = 0;
        }
    }

    bool isBegun() const { return _begun; }

    // ========================================================================
    // Polling - call from main loop or PLC scan cycle
    // ========================================================================

    // Drain hardware serial into ring buffer; if using delimiters, scan for messages
    void poll() {
        while (_serial->available()) {
            int b = _serial->read();
            if (b < 0) break;
            if (!rxPush((uint8_t)b)) break; // Buffer full
            _rxCount++;
        }
        if (_useDelimiter) scanForMessage();
    }

    // ========================================================================
    // Raw read/write (no framing)
    // ========================================================================

    // Write bytes directly to the serial port
    uint16_t write(const uint8_t* data, uint16_t length) {
        if (!data || length == 0) return 0;
        uint16_t written = _serial->write(data, length);
        _txCount += written;
        return written;
    }

    // Write a single byte
    bool writeByte(uint8_t byte) {
        if (_serial->write(byte) == 1) {
            _txCount++;
            return true;
        }
        return false;
    }

    // Read up to maxLen bytes from the ring buffer into dest
    uint16_t read(uint8_t* dest, uint16_t maxLen) {
        if (!dest || maxLen == 0) return 0;
        poll(); // Drain hardware buffer first
        uint16_t avail = rxAvailable();
        uint16_t count = (avail < maxLen) ? avail : maxLen;
        for (uint16_t i = 0; i < count; i++) {
            dest[i] = rxPop();
        }
        return count;
    }

    // Read a single byte (returns -1 if none available)
    int16_t readByte() {
        poll();
        if (rxAvailable() == 0) return -1;
        return (int16_t)rxPop();
    }

    // Number of bytes available in the ring buffer
    uint16_t available() {
        poll();
        return rxAvailable();
    }

    // Discard all data in the ring buffer
    void flush() {
        _rxHead = 0;
        _rxTail = 0;
        _msgReady = false;
        _msgLen = 0;
        // Also drain hardware buffer
        while (_serial->available()) _serial->read();
    }

    // ========================================================================
    // Delimiter-based message interface
    // ========================================================================

    void setDelimiter(uint8_t delim) {
        _delimiter = delim;
        _useDelimiter = (delim != 0);
        _msgReady = false;
        _msgLen = 0;
        if (_useDelimiter) scanForMessage();
    }

    uint8_t getDelimiter() const { return _delimiter; }
    bool isDelimiterMode() const { return _useDelimiter; }

    // Check if a complete delimited message is available
    bool messageReady() {
        poll();
        return _msgReady;
    }

    // Read the next complete message into dest (up to maxLen bytes)
    // Returns number of bytes read (including delimiter), or 0 if no message
    uint16_t readMessage(uint8_t* dest, uint16_t maxLen) {
        if (!_msgReady || !dest) return 0;
        uint16_t toRead = (_msgLen < maxLen) ? _msgLen : maxLen;
        for (uint16_t i = 0; i < toRead; i++) {
            dest[i] = rxPop();
        }
        // If we couldn't read the full message, discard remaining bytes
        if (toRead < _msgLen) {
            for (uint16_t i = toRead; i < _msgLen; i++) rxPop();
        }
        _msgReady = false;
        _msgLen = 0;
        // Scan for next message
        if (_useDelimiter) scanForMessage();
        return toRead;
    }

    // Peek at the message length (including delimiter) without consuming it
    uint16_t peekMessageLength() const { return _msgReady ? _msgLen : 0; }

    // ========================================================================
    // Blocking read with timeout (for request-response patterns)
    // ========================================================================

    // Read exactly 'length' bytes, blocking up to timeoutMs
    SerialRS232Result readBlocking(uint8_t* dest, uint16_t length, uint32_t timeoutMs = 0) {
        if (!dest || length == 0) return SERIAL_ERR_INVALID_PARAMS;
        if (timeoutMs == 0) timeoutMs = _timeoutMs;

        uint16_t received = 0;
        uint32_t startMs = millis();

        while (received < length) {
            poll();
            while (rxAvailable() > 0 && received < length) {
                dest[received++] = rxPop();
            }
            if (received >= length) break;
            if ((millis() - startMs) > timeoutMs) {
                _lastError = SERIAL_ERR_TIMEOUT;
                return SERIAL_ERR_TIMEOUT;
            }
        }
        return SERIAL_OK;
    }

    // Read until delimiter is found or timeout, blocking
    SerialRS232Result readUntilDelimiter(uint8_t* dest, uint16_t maxLen, uint16_t* outLen, uint8_t delim, uint32_t timeoutMs = 0) {
        if (!dest || maxLen == 0 || !outLen) return SERIAL_ERR_INVALID_PARAMS;
        if (timeoutMs == 0) timeoutMs = _timeoutMs;

        *outLen = 0;
        uint32_t startMs = millis();

        while (*outLen < maxLen) {
            poll();
            while (rxAvailable() > 0 && *outLen < maxLen) {
                uint8_t b = rxPop();
                dest[(*outLen)++] = b;
                if (b == delim) return SERIAL_OK;
            }
            if ((millis() - startMs) > timeoutMs) {
                _lastError = SERIAL_ERR_TIMEOUT;
                return SERIAL_ERR_TIMEOUT;
            }
        }
        _lastError = SERIAL_ERR_OVERFLOW;
        return SERIAL_ERR_OVERFLOW;
    }

    // ========================================================================
    // Configuration & Status
    // ========================================================================

    void setTimeout(uint32_t ms) { _timeoutMs = ms; }
    uint32_t getTimeout() const { return _timeoutMs; }
    uint32_t getBaudrate() const { return _baudrate; }

    uint32_t rxByteCount() const { return _rxCount; }
    uint32_t txByteCount() const { return _txCount; }
    uint32_t errorCount() const { return _errCount; }
    SerialRS232Result lastError() const { return _lastError; }
    void clearError() { _lastError = SERIAL_OK; }

    void resetCounters() { _rxCount = 0; _txCount = 0; _errCount = 0; }
};

#endif // PLCRUNTIME_SERIAL_RS232
