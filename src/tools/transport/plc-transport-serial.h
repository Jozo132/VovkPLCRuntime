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
    
public:
    /**
     * @brief Construct with a Stream reference
     * @param stream Reference to Serial, Serial1, SoftwareSerial, etc.
     */
    PLCSerialTransport(Stream& stream) : _stream(&stream) {}
    
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
    
    // Direct access to underlying stream if needed
    Stream* stream() { return _stream; }
};

#endif // PLCRUNTIME_TRANSPORT
