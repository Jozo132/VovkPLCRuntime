// plc-transport-serial.h - Serial transport implementation
//
// Copyright (c) 2022-2026 J.Vovk
//
// This file is part of VovkPLCRuntime.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#ifdef PLCRUNTIME_TRANSPORT

#include "plc-transport-interface.h"

/**
 * @brief Serial/Stream transport wrapper
 * 
 * Wraps any Arduino Stream (Serial, SoftwareSerial, etc.) as a PLCTransport.
 * 
 * Example:
 * @code
 * PLCSerialTransport serial(Serial);
 * runtime.addTransport(&serial);
 * @endcode
 */
class PLCSerialTransport : public PLCTransportInterface {
private:
    Stream* _stream;
    uint32_t _baudrate;
    
public:
    /**
     * @brief Construct with a Stream reference
     * @param stream Reference to Serial, Serial1, SoftwareSerial, etc.
     * @param baudrate Baudrate (for info reporting, user must call Serial.begin())
     */
    PLCSerialTransport(Stream& stream, uint32_t baudrate = 115200) 
        : _stream(&stream), _baudrate(baudrate) {}
    
    bool begin() override { 
        // Assume Serial.begin() is called by user
        return true; 
    }
    
    void end() override {}
    
    bool connected() override { 
        // Serial is always "connected"
        return true; 
    }
    
    void poll() override {
        // Nothing to do for serial
    }
    
    int available() override { 
        return _stream->available(); 
    }
    
    int read() override { 
        return _stream->read(); 
    }
    
    int peek() override { 
        return _stream->peek(); 
    }
    
    size_t readBytes(uint8_t* buffer, size_t length) override {
        return _stream->readBytes(buffer, length);
    }
    
    size_t write(uint8_t byte) override { 
        return _stream->write(byte); 
    }
    
    size_t write(const uint8_t* buffer, size_t size) override { 
        return _stream->write(buffer, size); 
    }
    
    void flush() override { 
        _stream->flush(); 
    }
    
    const char* name() override { return "Serial"; }
    bool isNetworkTransport() override { return false; }
    
    PLCTransportType transportType() override { return TRANSPORT_SERIAL; }
    
    void getConnectionInfo(PLCConnectionInfo& info) override {
        PLCTransportInterface::getConnectionInfo(info);
        info.baudrate = _baudrate;
    }
    
    // Accessors
    Stream* stream() { return _stream; }
    uint32_t baudrate() const { return _baudrate; }
    void setBaudrate(uint32_t baud) { _baudrate = baud; }
};

#endif // PLCRUNTIME_TRANSPORT
