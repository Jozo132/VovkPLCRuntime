// plc-transport-manager.h - Multi-transport manager
//
// Copyright (c) 2022-2026 J.Vovk
//
// This file is part of VovkPLCRuntime.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#ifdef PLCRUNTIME_TRANSPORT

#include "plc-transport-interface.h"
#include "plc-security.h"

#ifndef PLCRUNTIME_MAX_TRANSPORTS
#define PLCRUNTIME_MAX_TRANSPORTS 4
#endif

/**
 * @brief Transport entry with security settings
 */
struct PLCTransportEntry {
    PLCTransportInterface* transport = nullptr;
    PLCSecurity security = PLC_SEC_NONE;
    const char* name = nullptr;
    bool ownsTransport = false;  // If true, manager will delete on cleanup
};

/**
 * @brief Manages multiple transports with unified interface
 */
class PLCTransportManager {
private:
    PLCTransportEntry _entries[PLCRUNTIME_MAX_TRANSPORTS];
    uint8_t _count = 0;
    uint8_t _lastPolledIndex = 0;
    
public:
    ~PLCTransportManager() {
        end();
    }
    
    /**
     * @brief Add a transport
     * @param transport Pointer to transport (user manages lifetime unless ownsTransport=true)
     * @param security Security level for this transport
     * @param name Optional name for logging
     * @param ownsTransport If true, manager will delete the transport on cleanup
     * @return true if added successfully
     */
    bool addTransport(PLCTransportInterface* transport, 
                      PLCSecurity security = PLC_SEC_NONE,
                      const char* name = nullptr,
                      bool ownsTransport = false) {
        if (_count >= PLCRUNTIME_MAX_TRANSPORTS) return false;
        if (!transport) return false;
        
        _entries[_count].transport = transport;
        _entries[_count].security = security;
        _entries[_count].name = name ? name : transport->name();
        _entries[_count].ownsTransport = ownsTransport;
        _count++;
        return true;
    }
    
    /**
     * @brief Initialize all transports
     */
    void begin() {
        for (uint8_t i = 0; i < _count; i++) {
            if (_entries[i].transport) {
                _entries[i].transport->begin();
            }
        }
    }
    
    /**
     * @brief Shutdown all transports
     */
    void end() {
        for (uint8_t i = 0; i < _count; i++) {
            if (_entries[i].transport) {
                _entries[i].transport->end();
                if (_entries[i].ownsTransport) {
                    delete _entries[i].transport;
                    _entries[i].transport = nullptr;
                }
            }
        }
    }
    
    /**
     * @brief Poll all transports for connections
     */
    void poll() {
        for (uint8_t i = 0; i < _count; i++) {
            if (_entries[i].transport) {
                _entries[i].transport->poll();
            }
        }
    }
    
    /**
     * @brief Get transport with data available (round-robin fairness)
     * @param outIndex Receives the transport index
     * @return Transport with data, or nullptr if none
     */
    PLCTransportInterface* getActiveTransport(uint8_t* outIndex = nullptr) {
        for (uint8_t i = 0; i < _count; i++) {
            uint8_t idx = (_lastPolledIndex + i) % _count;
            PLCTransportInterface* t = _entries[idx].transport;
            if (t && t->available() > 0) {
                _lastPolledIndex = (idx + 1) % _count;
                if (outIndex) *outIndex = idx;
                return t;
            }
        }
        return nullptr;
    }
    
    /**
     * @brief Broadcast message to all connected transports
     * @param data Data to send
     * @param len Data length
     */
    void broadcast(const uint8_t* data, size_t len) {
        for (uint8_t i = 0; i < _count; i++) {
            PLCTransportInterface* t = _entries[i].transport;
            if (t && t->connected()) {
                t->write(data, len);
            }
        }
    }
    
    /**
     * @brief Broadcast string to all connected transports
     * @param str Null-terminated string
     */
    void broadcast(const char* str) {
        broadcast((const uint8_t*)str, strlen(str));
    }
    
    // Accessors
    uint8_t count() const { return _count; }
    
    PLCTransportInterface* get(uint8_t idx) { 
        return idx < _count ? _entries[idx].transport : nullptr; 
    }
    
    PLCTransportEntry* getEntry(uint8_t idx) {
        return idx < _count ? &_entries[idx] : nullptr;
    }
    
    PLCSecurity getSecurity(uint8_t idx) const {
        return idx < _count ? _entries[idx].security : PLC_SEC_NONE;
    }
    
    const char* getName(uint8_t idx) const {
        return idx < _count ? _entries[idx].name : nullptr;
    }
};

#endif // PLCRUNTIME_TRANSPORT
