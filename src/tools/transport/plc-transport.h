// plc-transport.h - Main transport system include
//
// Copyright (c) 2022-2026 J.Vovk
//
// This file is part of VovkPLCRuntime.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// ============================================================================
// Transport System Architecture
// ============================================================================
//
// Enable with: #define PLCRUNTIME_TRANSPORT before including VovkPLCRuntime.h
//
// If PLCRUNTIME_TRANSPORT is NOT defined:
//   - Simple Serial-only mode (current legacy behavior)
//   - No sessions, no authentication
//   - Zero overhead
//
// If PLCRUNTIME_TRANSPORT IS defined:
//   - Multi-transport support (Serial, WiFi, Ethernet, custom)
//   - Session management
//   - Optional authentication
//   - User-extensible via PLCTransportInterface
//
// ============================================================================
// Usage Examples
// ============================================================================
//
// 1. Simple (no transport system, legacy mode):
//    -----------------------------------------
//    #include <VovkPLCRuntime.h>
//    VovkPLCRuntime runtime;
//    void setup() { Serial.begin(115200); runtime.initialize(); }
//    void loop() { runtime.listen(); runtime.run(); }
//
// 2. With transport system - ESP32 WiFi:
//    -----------------------------------
//    #define PLCRUNTIME_TRANSPORT
//    #include <WiFi.h>
//    #include <VovkPLCRuntime.h>
//    
//    WiFiServer wifiServer(502);
//    VovkPLCRuntime runtime;
//    
//    void setup() {
//        Serial.begin(115200);
//        WiFi.begin("SSID", "password");
//        while (WiFi.status() != WL_CONNECTED) delay(500);
//        wifiServer.begin();
//        
//        runtime
//            .addSerial(Serial, PLC_SEC_NONE)
//            .addTCP<WiFiServer, WiFiClient>(wifiServer, PLC_SEC_PASSWORD)
//            .addUser("admin", "admin123", PERM_FULL)
//            .addUser("operator", "op123", PERM_OPERATOR)
//            .initialize();
//    }
//    void loop() { runtime.listen(); runtime.run(); }
//
// 3. With transport system - W5500 Ethernet:
//    ---------------------------------------
//    #define PLCRUNTIME_TRANSPORT
//    #include <Ethernet.h>
//    #include <VovkPLCRuntime.h>
//    
//    EthernetServer ethServer(502);
//    VovkPLCRuntime runtime;
//    
//    void setup() {
//        byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//        Ethernet.begin(mac, IPAddress(192,168,1,100));
//        ethServer.begin();
//        
//        runtime
//            .addSerial(Serial)
//            .addTCP<EthernetServer, EthernetClient>(ethServer, PLC_SEC_PASSWORD)
//            .addUser("admin", "pass", PERM_FULL)
//            .initialize();
//    }
//
// 4. With custom transport (RS485, CAN, LoRa, etc.):
//    -----------------------------------------------
//    class MyTransport : public PLCTransportInterface {
//        // Implement: begin(), end(), connected(), poll(),
//        //            available(), read(), peek(), write(), flush()
//    };
//    
//    MyTransport myTransport;
//    runtime
//        .addTransport(&myTransport, PLC_SEC_PASSWORD, "MyProto")
//        .addUser("admin", "pass", PERM_FULL);
//
// ============================================================================
// Permission Levels
// ============================================================================
//
// PERM_MONITOR    - Read status and memory (view-only)
// PERM_OPERATOR   - Monitor + write values + start/stop
// PERM_PROGRAMMER - Operator + upload/download programs  
// PERM_FULL       - All permissions including admin
//
// ============================================================================
// Security Levels
// ============================================================================
//
// PLC_SEC_NONE      - No authentication (trusted local connections)
// PLC_SEC_PASSWORD  - Username/password authentication
// PLC_SEC_CHALLENGE - Challenge-response (HMAC) for sensitive networks
//

#pragma once

#ifdef PLCRUNTIME_TRANSPORT

// Core components
#include "plc-transport-interface.h"
#include "plc-security.h"
#include "plc-transport-manager.h"

// Built-in transports
#include "plc-transport-serial.h"
#include "plc-transport-tcp.h"

#endif // PLCRUNTIME_TRANSPORT
