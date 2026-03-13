// plc-comms-manager.h - 2025
//
// Copyright (c) 2025 J.Vovk
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

// ============================================================================
// PLC Communication Protocol Manager
// ============================================================================
// Manages multiple communication protocol instances (ModbusRTU, ModbusTCP,
// raw TCP, raw UDP). Each instance is identified by a u8 index (0-N).
//
// Architecture:
//   C++ code registers hardware drivers (Serial, Ethernet, WiFi, etc.)
//   Bytecode configures protocol-level settings and executes operations
//
// Usage (C++ side):
//   ModbusRTU mb0(Serial1, 9600, 2);
//   g_plcComms.registerModbusRTU(0, &mb0);
//
// Usage (PLCASM side):
//   comms_begin #0 #0          ; Initialize instance 0 as master
//   mb_read_holding #0 #1 #0 #4 $100  ; Read 4 regs from slave 1 to M100
// ============================================================================

#ifndef PLCRUNTIME_MAX_COMMS_INSTANCES
#define PLCRUNTIME_MAX_COMMS_INSTANCES 4
#endif

// ============================================================================
// Protocol Type Identifiers
// ============================================================================
enum PLCCommsProtocol : u8 {
    COMMS_PROTO_NONE       = 0x00,
    COMMS_PROTO_MODBUS_RTU = 0x01,
    COMMS_PROTO_MODBUS_TCP = 0x02,
    COMMS_PROTO_RAW_TCP    = 0x03,
    COMMS_PROTO_RAW_UDP    = 0x04,
};

// ============================================================================
// COMMS Sub-Function Codes
// ============================================================================
// Format: [COMMS opcode (0xF9)] [sub_fn:u8] [params...]
// These define the operation to perform on a communication instance.
// ============================================================================
enum PLCCommsSubFunction : u8 {
    // ---- Common (0x00-0x07) ------------------------------------------------
    COMMS_BEGIN         = 0x00, // [inst:u8] [config:u8]             -> push bool
    COMMS_END           = 0x01, // [inst:u8]
    COMMS_ENABLED       = 0x02, // [inst:u8]                         -> push bool
    COMMS_STATUS        = 0x03, // [inst:u8]                         -> push u8

    // ---- Modbus Data Area Config (0x08-0x0B) --------------------------------
    MB_ADD_COILS        = 0x08, // [inst:u8] [start:u16] [count:u16]
    MB_ADD_DISCRETE     = 0x09, // [inst:u8] [start:u16] [count:u16]
    MB_ADD_HOLDING      = 0x0A, // [inst:u8] [start:u16] [count:u16]
    MB_ADD_INPUT_REG    = 0x0B, // [inst:u8] [start:u16] [count:u16]

    // ---- Modbus Master Read (0x10-0x13) - align with FC codes ---------------
    MB_READ_COILS       = 0x10, // [inst:u8] [slave:u8] [start:u16] [qty:u16] [dest_mem:ptr] -> push u8
    MB_READ_DISCRETE    = 0x11, // [inst:u8] [slave:u8] [start:u16] [qty:u16] [dest_mem:ptr] -> push u8
    MB_READ_HOLDING     = 0x12, // [inst:u8] [slave:u8] [start:u16] [qty:u16] [dest_mem:ptr] -> push u8
    MB_READ_INPUT       = 0x13, // [inst:u8] [slave:u8] [start:u16] [qty:u16] [dest_mem:ptr] -> push u8

    // ---- Modbus Master Write (0x14-0x17) ------------------------------------
    MB_WRITE_COIL       = 0x14, // [inst:u8] [slave:u8] [addr:u16] + pop bool      -> push u8
    MB_WRITE_REG        = 0x15, // [inst:u8] [slave:u8] [addr:u16] + pop u16       -> push u8
    MB_WRITE_COILS      = 0x16, // [inst:u8] [slave:u8] [start:u16] [qty:u16] [src_mem:ptr] -> push u8
    MB_WRITE_REGS       = 0x17, // [inst:u8] [slave:u8] [start:u16] [qty:u16] [src_mem:ptr] -> push u8

    // ---- Modbus Slave Operations (0x18-0x20) --------------------------------
    MB_POLL             = 0x18, // [inst:u8]                         -> push bool
    MB_SLV_GET_COIL     = 0x19, // [inst:u8] [addr:u16]             -> push bool
    MB_SLV_SET_COIL     = 0x1A, // [inst:u8] [addr:u16]             + pop bool
    MB_SLV_GET_REG      = 0x1B, // [inst:u8] [addr:u16]             -> push u16
    MB_SLV_SET_REG      = 0x1C, // [inst:u8] [addr:u16]             + pop u16
    MB_SLV_GET_DI       = 0x1D, // [inst:u8] [addr:u16]             -> push bool
    MB_SLV_SET_DI       = 0x1E, // [inst:u8] [addr:u16]             + pop bool
    MB_SLV_GET_IR       = 0x1F, // [inst:u8] [addr:u16]             -> push u16
    MB_SLV_SET_IR       = 0x20, // [inst:u8] [addr:u16]             + pop u16

    // ---- Raw TCP (0x30-0x37) ------------------------------------------------
    TCP_CONNECT         = 0x30, // [inst:u8] [ip0:u8] [ip1:u8] [ip2:u8] [ip3:u8] [port:u16] -> push bool
    TCP_DISCONNECT      = 0x31, // [inst:u8]
    TCP_CONNECTED       = 0x32, // [inst:u8]                         -> push bool
    TCP_LISTEN          = 0x33, // [inst:u8] [port:u16]              -> push bool
    TCP_ACCEPT          = 0x34, // [inst:u8]                         -> push bool
    TCP_SEND            = 0x35, // [inst:u8] [src_mem:ptr] [len:u16] -> push u16
    TCP_RECV            = 0x36, // [inst:u8] [dest_mem:ptr] [max:u16] -> push u16
    TCP_AVAILABLE       = 0x37, // [inst:u8]                         -> push u16

    // ---- Raw UDP (0x40-0x44) ------------------------------------------------
    UDP_OPEN            = 0x40, // [inst:u8] [port:u16]              -> push bool
    UDP_CLOSE           = 0x41, // [inst:u8]
    UDP_SEND            = 0x42, // [inst:u8] [ip0-3:u8x4] [port:u16] [src_mem:ptr] [len:u16] -> push u16
    UDP_RECV            = 0x43, // [inst:u8] [dest_mem:ptr] [max:u16] -> push u16
    UDP_AVAILABLE       = 0x44, // [inst:u8]                         -> push u16
};

// ============================================================================
// Sub-function parameter size calculator
// ============================================================================
// Returns the number of parameter bytes AFTER the sub-function byte.
// This is needed for WASM to skip COMMS instructions it cannot execute,
// and for the bytecode explainer to decode instruction boundaries.
// Note: MY_PTR_SIZE_BYTES varies by platform (1, 2, or 4 bytes)
// ============================================================================
static u8 comms_subfn_param_size(u8 sub_fn) {
    switch ((PLCCommsSubFunction) sub_fn) {
        // Common
        case COMMS_BEGIN:       return 2;   // inst + config
        case COMMS_END:         return 1;   // inst
        case COMMS_ENABLED:     return 1;   // inst
        case COMMS_STATUS:      return 1;   // inst

        // Modbus data area config
        case MB_ADD_COILS:      return 5;   // inst + start(2) + count(2)
        case MB_ADD_DISCRETE:   return 5;
        case MB_ADD_HOLDING:    return 5;
        case MB_ADD_INPUT_REG:  return 5;

        // Modbus master read: inst + slave + start(2) + qty(2) + dest_mem(ptr)
        case MB_READ_COILS:     return 6 + MY_PTR_SIZE_BYTES;
        case MB_READ_DISCRETE:  return 6 + MY_PTR_SIZE_BYTES;
        case MB_READ_HOLDING:   return 6 + MY_PTR_SIZE_BYTES;
        case MB_READ_INPUT:     return 6 + MY_PTR_SIZE_BYTES;

        // Modbus master write single: inst + slave + addr(2)
        case MB_WRITE_COIL:     return 4;   // + pop bool, push u8
        case MB_WRITE_REG:      return 4;   // + pop u16, push u8

        // Modbus master write multiple: inst + slave + start(2) + qty(2) + src_mem(ptr)
        case MB_WRITE_COILS:    return 6 + MY_PTR_SIZE_BYTES;
        case MB_WRITE_REGS:     return 6 + MY_PTR_SIZE_BYTES;

        // Modbus slave operations
        case MB_POLL:           return 1;   // inst
        case MB_SLV_GET_COIL:   return 3;   // inst + addr(2)
        case MB_SLV_SET_COIL:   return 3;
        case MB_SLV_GET_REG:    return 3;
        case MB_SLV_SET_REG:    return 3;
        case MB_SLV_GET_DI:     return 3;
        case MB_SLV_SET_DI:     return 3;
        case MB_SLV_GET_IR:     return 3;
        case MB_SLV_SET_IR:     return 3;

        // Raw TCP
        case TCP_CONNECT:       return 7;   // inst + ip(4) + port(2)
        case TCP_DISCONNECT:    return 1;
        case TCP_CONNECTED:     return 1;
        case TCP_LISTEN:        return 3;   // inst + port(2)
        case TCP_ACCEPT:        return 1;
        case TCP_SEND:          return 3 + MY_PTR_SIZE_BYTES; // inst + src(ptr) + len(2)
        case TCP_RECV:          return 3 + MY_PTR_SIZE_BYTES; // inst + dest(ptr) + max(2)
        case TCP_AVAILABLE:     return 1;

        // Raw UDP
        case UDP_OPEN:          return 3;   // inst + port(2)
        case UDP_CLOSE:         return 1;
        case UDP_SEND:          return 9 + MY_PTR_SIZE_BYTES; // inst + ip(4) + port(2) + src(ptr) + len(2)
        case UDP_RECV:          return 3 + MY_PTR_SIZE_BYTES; // inst + dest(ptr) + max(2)
        case UDP_AVAILABLE:     return 1;

        default:                return 0;   // Unknown sub-function
    }
}

// ============================================================================
// Sub-function name for debug/explain output
// ============================================================================
static const FSH* comms_subfn_name(u8 sub_fn) {
    switch ((PLCCommsSubFunction) sub_fn) {
        case COMMS_BEGIN:       return F("COMMS_BEGIN");
        case COMMS_END:         return F("COMMS_END");
        case COMMS_ENABLED:     return F("COMMS_ENABLED");
        case COMMS_STATUS:      return F("COMMS_STATUS");
        case MB_ADD_COILS:      return F("MB_ADD_COILS");
        case MB_ADD_DISCRETE:   return F("MB_ADD_DISCRETE");
        case MB_ADD_HOLDING:    return F("MB_ADD_HOLDING");
        case MB_ADD_INPUT_REG:  return F("MB_ADD_INPUT_REG");
        case MB_READ_COILS:     return F("MB_READ_COILS");
        case MB_READ_DISCRETE:  return F("MB_READ_DISCRETE");
        case MB_READ_HOLDING:   return F("MB_READ_HOLDING");
        case MB_READ_INPUT:     return F("MB_READ_INPUT");
        case MB_WRITE_COIL:     return F("MB_WRITE_COIL");
        case MB_WRITE_REG:      return F("MB_WRITE_REG");
        case MB_WRITE_COILS:    return F("MB_WRITE_COILS");
        case MB_WRITE_REGS:     return F("MB_WRITE_REGS");
        case MB_POLL:           return F("MB_POLL");
        case MB_SLV_GET_COIL:   return F("MB_SLV_GET_COIL");
        case MB_SLV_SET_COIL:   return F("MB_SLV_SET_COIL");
        case MB_SLV_GET_REG:    return F("MB_SLV_GET_REG");
        case MB_SLV_SET_REG:    return F("MB_SLV_SET_REG");
        case MB_SLV_GET_DI:     return F("MB_SLV_GET_DI");
        case MB_SLV_SET_DI:     return F("MB_SLV_SET_DI");
        case MB_SLV_GET_IR:     return F("MB_SLV_GET_IR");
        case MB_SLV_SET_IR:     return F("MB_SLV_SET_IR");
        case TCP_CONNECT:       return F("TCP_CONNECT");
        case TCP_DISCONNECT:    return F("TCP_DISCONNECT");
        case TCP_CONNECTED:     return F("TCP_CONNECTED");
        case TCP_LISTEN:        return F("TCP_LISTEN");
        case TCP_ACCEPT:        return F("TCP_ACCEPT");
        case TCP_SEND:          return F("TCP_SEND");
        case TCP_RECV:          return F("TCP_RECV");
        case TCP_AVAILABLE:     return F("TCP_AVAILABLE");
        case UDP_OPEN:          return F("UDP_OPEN");
        case UDP_CLOSE:         return F("UDP_CLOSE");
        case UDP_SEND:          return F("UDP_SEND");
        case UDP_RECV:          return F("UDP_RECV");
        case UDP_AVAILABLE:     return F("UDP_AVAILABLE");
        default:                return F("COMMS_UNKNOWN");
    }
}

// ============================================================================
// Communication Instance
// ============================================================================
struct PLCCommsInstance {
    PLCCommsProtocol protocol;
    bool active;
    void* driver;       // Points to ModbusRTU, etc.
    u8 lastError;       // Last operation error code

    PLCCommsInstance() : protocol(COMMS_PROTO_NONE), active(false), driver(nullptr), lastError(0) { }
};

// ============================================================================
// Communication Manager (Global Registry)
// ============================================================================
class PLCCommsManager {
    PLCCommsInstance _instances[PLCRUNTIME_MAX_COMMS_INSTANCES];

public:
    PLCCommsManager() { }

    PLCCommsInstance* getInstance(u8 index) {
        if (index >= PLCRUNTIME_MAX_COMMS_INSTANCES) return nullptr;
        return &_instances[index];
    }

    bool isRegistered(u8 index) const {
        if (index >= PLCRUNTIME_MAX_COMMS_INSTANCES) return false;
        return _instances[index].protocol != COMMS_PROTO_NONE;
    }

    bool isActive(u8 index) const {
        if (index >= PLCRUNTIME_MAX_COMMS_INSTANCES) return false;
        return _instances[index].active;
    }

#ifdef PLCRUNTIME_MODBUS_RTU
    bool registerModbusRTU(u8 index, ModbusRTU* driver) {
        if (index >= PLCRUNTIME_MAX_COMMS_INSTANCES || !driver) return false;
        _instances[index].protocol = COMMS_PROTO_MODBUS_RTU;
        _instances[index].driver = (void*) driver;
        _instances[index].active = false;
        _instances[index].lastError = 0;
        return true;
    }

    ModbusRTU* getModbusRTU(u8 index) {
        if (index >= PLCRUNTIME_MAX_COMMS_INSTANCES) return nullptr;
        if (_instances[index].protocol != COMMS_PROTO_MODBUS_RTU) return nullptr;
        return (ModbusRTU*) _instances[index].driver;
    }
#endif // PLCRUNTIME_MODBUS_RTU

    // Future: registerModbusTCP, registerTCP, registerUDP
    // These will be added when the respective protocol libraries are implemented.
};

// Global communication manager instance (like g_ffiRegistry)
static PLCCommsManager g_plcComms;
