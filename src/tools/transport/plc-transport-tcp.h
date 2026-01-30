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
    PLCTransportType _transportType = TRANSPORT_WIFI;  // Default to WiFi, can be changed
    const char* _transportName = "TCP";
    
public:
    /**
     * @brief Create transport with specified port (creates server internally)
     * @param port TCP port to listen on
     * @param type Transport type (TRANSPORT_WIFI or TRANSPORT_ETHERNET)
     */
    PLCTCPTransport(uint16_t port, PLCTransportType type = TRANSPORT_WIFI) 
        : _port(port), _ownsServer(true), _transportType(type) {
        _transportName = (type == TRANSPORT_ETHERNET) ? "Ethernet" : "WiFi";
    }
    
    /**
     * @brief Create transport with existing server
     * @param server Reference to existing server
     * @param type Transport type (TRANSPORT_WIFI or TRANSPORT_ETHERNET)
     */
    PLCTCPTransport(TServer& server, PLCTransportType type = TRANSPORT_WIFI) 
        : _server(&server), _port(0), _ownsServer(false), _transportType(type) {
        _transportName = (type == TRANSPORT_ETHERNET) ? "Ethernet" : "WiFi";
    }
    
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
    
    const char* name() override { return _transportName; }
    bool isNetworkTransport() override { return true; }
    PLCTransportType transportType() override { return _transportType; }
    
    const char* clientAddress() override { return _clientAddr; }
    
    void getConnectionInfo(PLCConnectionInfo& info) override {
        PLCTransportInterface::getConnectionInfo(info);
        info.port = _port;
        
        // Get local IP, gateway, subnet based on transport type
        #if defined(ESP8266) || defined(ESP32)
        if (_transportType == TRANSPORT_WIFI) {
            IPAddress localIP = WiFi.localIP();
            IPAddress gateway = WiFi.gatewayIP();
            IPAddress subnet = WiFi.subnetMask();
            uint8_t* mac = WiFi.macAddress(info.mac);
            (void)mac; // Suppress unused warning
            
            info.ip[0] = localIP[0]; info.ip[1] = localIP[1];
            info.ip[2] = localIP[2]; info.ip[3] = localIP[3];
            info.gateway[0] = gateway[0]; info.gateway[1] = gateway[1];
            info.gateway[2] = gateway[2]; info.gateway[3] = gateway[3];
            info.subnet[0] = subnet[0]; info.subnet[1] = subnet[1];
            info.subnet[2] = subnet[2]; info.subnet[3] = subnet[3];
        }
        #endif
        
        #if defined(ETHERNET_H) || defined(Ethernet_h)
        if (_transportType == TRANSPORT_ETHERNET) {
            IPAddress localIP = Ethernet.localIP();
            IPAddress gateway = Ethernet.gatewayIP();
            IPAddress subnet = Ethernet.subnetMask();
            // Ethernet library doesn't have getMacAddress in same way
            
            info.ip[0] = localIP[0]; info.ip[1] = localIP[1];
            info.ip[2] = localIP[2]; info.ip[3] = localIP[3];
            info.gateway[0] = gateway[0]; info.gateway[1] = gateway[1];
            info.gateway[2] = gateway[2]; info.gateway[3] = gateway[3];
            info.subnet[0] = subnet[0]; info.subnet[1] = subnet[1];
            info.subnet[2] = subnet[2]; info.subnet[3] = subnet[3];
        }
        #endif
        
        // Client info if connected
        if (_client && _client.connected()) {
            #if defined(ESP8266) || defined(ESP32)
            IPAddress clientIP = _client.remoteIP();
            info.clientIp[0] = clientIP[0]; info.clientIp[1] = clientIP[1];
            info.clientIp[2] = clientIP[2]; info.clientIp[3] = clientIP[3];
            info.clientPort = _client.remotePort();
            #endif
        }
    }
    
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
