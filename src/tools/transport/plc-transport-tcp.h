// plc-transport-tcp.h - TCP Server transport (WiFi/Ethernet)
//
// Copyright (c) 2022-2026 J.Vovk
//
// This file is part of VovkPLCRuntime.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#ifdef PLCRUNTIME_TRANSPORT

#include "plc-transport-interface.h"

/**
 * @brief Generic TCP Server transport template
 * 
 * Works with any Server/Client pair that follows Arduino conventions:
 * - WiFiServer/WiFiClient (ESP8266, ESP32)
 * - EthernetServer/EthernetClient (W5100, W5500)
 * 
 * Example:
 * @code
 * PLCTCPTransport<WiFiServer, WiFiClient> wifiTransport(502);
 * runtime.addTransport(&wifiTransport, PLC_SEC_PASSWORD);
 * @endcode
 */
template<typename TServer, typename TClient>
class PLCTCPTransport : public PLCTransportInterface {
private:
    TServer* _server = nullptr;
    TClient _client;
    uint16_t _port;
    bool _ownsServer;
    char _clientAddr[16] = {0};
    
public:
    /**
     * @brief Create transport with specified port (creates server internally)
     * @param port TCP port to listen on
     */
    PLCTCPTransport(uint16_t port) 
        : _port(port), _ownsServer(true) {}
    
    /**
     * @brief Create transport with existing server
     * @param server Reference to existing server
     */
    PLCTCPTransport(TServer& server) 
        : _server(&server), _port(0), _ownsServer(false) {}
    
    ~PLCTCPTransport() {
        if (_ownsServer && _server) {
            delete _server;
        }
    }
    
    bool begin() override {
        if (_ownsServer && !_server) {
            _server = new TServer(_port);
        }
        if (_server) {
            _server->begin();
            return true;
        }
        return false;
    }
    
    void end() override {
        if (_client) {
            _client.stop();
        }
        if (_ownsServer && _server) {
            // Most Arduino servers don't have end()
            delete _server;
            _server = nullptr;
        }
    }
    
    bool connected() override { 
        return _client && _client.connected(); 
    }
    
    void poll() override {
        // Accept new connection if not currently connected
        if (!_client || !_client.connected()) {
            if (_server) {
                TClient newClient = _server->available();
                if (newClient) {
                    _client = newClient;
                    updateClientAddress();
                }
            }
        }
    }
    
    int available() override { 
        return _client ? _client.available() : 0; 
    }
    
    int read() override { 
        return _client ? _client.read() : -1; 
    }
    
    int peek() override { 
        return _client ? _client.peek() : -1; 
    }
    
    size_t readBytes(uint8_t* buffer, size_t length) override {
        return _client ? _client.readBytes(buffer, length) : 0;
    }
    
    size_t write(uint8_t byte) override { 
        return _client ? _client.write(byte) : 0; 
    }
    
    size_t write(const uint8_t* buffer, size_t size) override { 
        return _client ? _client.write(buffer, size) : 0; 
    }
    
    void flush() override { 
        if (_client) _client.flush(); 
    }
    
    const char* name() override { return "TCP"; }
    bool isNetworkTransport() override { return true; }
    
    const char* clientAddress() override { return _clientAddr; }
    
    // Access to underlying client/server if needed
    TClient& client() { return _client; }
    TServer* server() { return _server; }
    uint16_t port() const { return _port; }
    
private:
    void updateClientAddress() {
        // Try to get client IP - works for WiFi/Ethernet clients
        #if defined(ESP8266) || defined(ESP32)
        if (_client) {
            IPAddress ip = _client.remoteIP();
            snprintf(_clientAddr, sizeof(_clientAddr), "%d.%d.%d.%d", 
                     ip[0], ip[1], ip[2], ip[3]);
        }
        #else
        _clientAddr[0] = '\0';
        #endif
    }
};

// ============================================================================
// Convenience typedefs for common platforms
// ============================================================================

// ESP8266 WiFi
#if defined(ESP8266)
#include <ESP8266WiFi.h>
using PLCWiFiTransport = PLCTCPTransport<WiFiServer, WiFiClient>;
#endif

// ESP32 WiFi
#if defined(ESP32)
#include <WiFi.h>
using PLCWiFiTransport = PLCTCPTransport<WiFiServer, WiFiClient>;
#endif

// Arduino Ethernet library (W5100, W5500, etc.)
#if defined(ETHERNET_H) || defined(Ethernet_h)
#include <Ethernet.h>
using PLCEthernetTransport = PLCTCPTransport<EthernetServer, EthernetClient>;
#endif

#endif // PLCRUNTIME_TRANSPORT
