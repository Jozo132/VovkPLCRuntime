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
    COMMS_PROTO_SERIAL     = 0x05, // Generic serial RS232
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
    COMMS_SET_PRIORITY  = 0x04, // [inst:u8] [priority:u8]           (0=sync/high, 1=async/low)
    COMMS_POLL_ASYNC    = 0x05, // [inst:u8]                         -> push u8 (0=idle, 1=busy, 0xFF=error)
    COMMS_QUEUE_SIZE    = 0x06, // [inst:u8]                         -> push u8

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

    // ---- Ethernet Socket Reservation (0x60-0x67) ----------------------------
    // Socket pool management for thread-safe realtime/background I/O.
    // pool: 0=RT_TCP, 1=BG_TCP, 2=RT_UDP, 3=BG_UDP
    ETH_SOCK_ACQUIRE    = 0x60, // [inst:u8] [pool:u8]              -> push u8 (slot, 0xFF=none)
    ETH_SOCK_RELEASE    = 0x61, // [inst:u8] [slot:u8] [type:u8]    -> push u8 (result)
    ETH_SOCK_STATUS     = 0x62, // [inst:u8] [slot:u8] [type:u8]    -> push u8 (EthSocketState)
    ETH_SOCK_SET_ACTIVE = 0x63, // [inst:u8] [slot:u8] [type:u8]    (set active TCP/UDP slot)
    ETH_SOCK_INFO       = 0x64, // [inst:u8]                        -> push u8 (packed: rt_tcp|bg_tcp|rt_udp|bg_udp)
    UDP_CLOSE           = 0x41, // [inst:u8]
    UDP_SEND            = 0x42, // [inst:u8] [ip0-3:u8x4] [port:u16] [src_mem:ptr] [len:u16] -> push u16
    UDP_RECV            = 0x43, // [inst:u8] [dest_mem:ptr] [max:u16] -> push u16
    UDP_AVAILABLE       = 0x44, // [inst:u8]                         -> push u16

    // ---- Serial RS232 (0x50-0x58) -------------------------------------------
    SER_WRITE           = 0x50, // [inst:u8] [src_mem:ptr] [len:u16] -> push u16 (bytes written)
    SER_READ            = 0x51, // [inst:u8] [dest_mem:ptr] [max:u16] -> push u16 (bytes read)
    SER_AVAILABLE       = 0x52, // [inst:u8]                         -> push u16
    SER_FLUSH           = 0x53, // [inst:u8]
    SER_WRITE_BYTE      = 0x54, // [inst:u8]                         + pop u8 -> push bool
    SER_READ_BYTE       = 0x55, // [inst:u8]                         -> push i16 (-1 if none)
    SER_POLL            = 0x56, // [inst:u8]                         -> push u16 (bytes received)
    SER_MSG_READY       = 0x57, // [inst:u8]                         -> push bool
    SER_READ_MSG        = 0x58, // [inst:u8] [dest_mem:ptr] [max:u16] -> push u16 (msg length)
    SER_SET_DELIM       = 0x59, // [inst:u8] [delim:u8]
    SER_SET_BAUD        = 0x5A, // [inst:u8] [baud:u32]              (auto re-init if active)
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
        case COMMS_SET_PRIORITY:return 2;   // inst + priority
        case COMMS_POLL_ASYNC:  return 1;   // inst
        case COMMS_QUEUE_SIZE:  return 1;   // inst

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

        // Ethernet Socket Reservation
        case ETH_SOCK_ACQUIRE:  return 2;   // inst + pool
        case ETH_SOCK_RELEASE:  return 3;   // inst + slot + type(0=tcp,1=udp)
        case ETH_SOCK_STATUS:   return 3;   // inst + slot + type
        case ETH_SOCK_SET_ACTIVE: return 3; // inst + slot + type
        case ETH_SOCK_INFO:     return 1;   // inst

        // Serial RS232
        case SER_WRITE:         return 3 + MY_PTR_SIZE_BYTES; // inst + src(ptr) + len(2)
        case SER_READ:          return 3 + MY_PTR_SIZE_BYTES; // inst + dest(ptr) + max(2)
        case SER_AVAILABLE:     return 1;
        case SER_FLUSH:         return 1;
        case SER_WRITE_BYTE:    return 1;   // + pop u8
        case SER_READ_BYTE:     return 1;
        case SER_POLL:          return 1;
        case SER_MSG_READY:     return 1;
        case SER_READ_MSG:      return 3 + MY_PTR_SIZE_BYTES; // inst + dest(ptr) + max(2)
        case SER_SET_DELIM:     return 2;   // inst + delim
        case SER_SET_BAUD:      return 5;   // inst + baud(4)

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
        case COMMS_SET_PRIORITY:return F("COMMS_SET_PRIORITY");
        case COMMS_POLL_ASYNC:  return F("COMMS_POLL_ASYNC");
        case COMMS_QUEUE_SIZE:  return F("COMMS_QUEUE_SIZE");
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
        case ETH_SOCK_ACQUIRE:  return F("ETH_SOCK_ACQUIRE");
        case ETH_SOCK_RELEASE:  return F("ETH_SOCK_RELEASE");
        case ETH_SOCK_STATUS:   return F("ETH_SOCK_STATUS");
        case ETH_SOCK_SET_ACTIVE: return F("ETH_SOCK_SET_ACTIVE");
        case ETH_SOCK_INFO:     return F("ETH_SOCK_INFO");
        case SER_WRITE:         return F("SER_WRITE");
        case SER_READ:          return F("SER_READ");
        case SER_AVAILABLE:     return F("SER_AVAILABLE");
        case SER_FLUSH:         return F("SER_FLUSH");
        case SER_WRITE_BYTE:    return F("SER_WRITE_BYTE");
        case SER_READ_BYTE:     return F("SER_READ_BYTE");
        case SER_POLL:          return F("SER_POLL");
        case SER_MSG_READY:     return F("SER_MSG_READY");
        case SER_READ_MSG:      return F("SER_READ_MSG");
        case SER_SET_DELIM:     return F("SER_SET_DELIM");
        case SER_SET_BAUD:      return F("SER_SET_BAUD");
        default:                return F("COMMS_UNKNOWN");
    }
}

// ============================================================================
// Async Transaction Priority
// ============================================================================
// Comms instances can operate in two modes:
//   COMMS_PRIORITY_SYNC  (0): All operations are synchronous (blocking).
//                              High priority - executes immediately, fastest.
//   COMMS_PRIORITY_ASYNC (1): I/O operations are queued and executed one per
//                              poll cycle. Low priority - non-blocking, allows
//                              other work to proceed between transactions.
//
// Priority can be changed at runtime via comms_set_priority instruction.
// Mixing is supported: set SYNC for critical reads, ASYNC for background polling.
// ============================================================================

#define COMMS_PRIORITY_SYNC  0  // Blocking: execute immediately
#define COMMS_PRIORITY_ASYNC 1  // Non-blocking: queue and poll

// ============================================================================
// Async Transaction Queue
// ============================================================================
// When priority=ASYNC, I/O operations (reads, writes, etc.) are enqueued here.
// Each poll cycle processes at most one transaction from the queue.
// The queue is per-instance and has a fixed depth.
// ============================================================================

#ifndef PLCRUNTIME_COMMS_QUEUE_DEPTH
#define PLCRUNTIME_COMMS_QUEUE_DEPTH 8
#endif

struct PLCCommsTxn {
    u8 sub_fn;          // The sub-function to execute
    u8 params[16];      // Inline parameters (max needed: inst+slave+start(2)+qty(2)+ptr(4)+extra = ~12)
    u8 paramLen;        // How many bytes in params[]
    u8 resultError;     // Result code after execution (0 = pending / not yet run)
    bool pending;       // true if this slot is waiting to be executed
};

struct PLCCommsTxnQueue {
    PLCCommsTxn txns[PLCRUNTIME_COMMS_QUEUE_DEPTH];
    u8 head;    // Next slot to write
    u8 tail;    // Next slot to execute
    u8 count;   // Pending count

    PLCCommsTxnQueue() : head(0), tail(0), count(0) {
        for (u8 i = 0; i < PLCRUNTIME_COMMS_QUEUE_DEPTH; i++) txns[i].pending = false;
    }

    bool enqueue(u8 sub_fn, const u8* params, u8 paramLen) {
        if (count >= PLCRUNTIME_COMMS_QUEUE_DEPTH) return false;
        if (paramLen > sizeof(txns[0].params)) return false;
        PLCCommsTxn& t = txns[head];
        t.sub_fn = sub_fn;
        for (u8 i = 0; i < paramLen; i++) t.params[i] = params[i];
        t.paramLen = paramLen;
        t.resultError = 0;
        t.pending = true;
        head = (head + 1) % PLCRUNTIME_COMMS_QUEUE_DEPTH;
        count++;
        return true;
    }

    PLCCommsTxn* peek() {
        if (count == 0) return nullptr;
        return &txns[tail];
    }

    void dequeue() {
        if (count == 0) return;
        txns[tail].pending = false;
        tail = (tail + 1) % PLCRUNTIME_COMMS_QUEUE_DEPTH;
        count--;
    }

    void clear() {
        head = 0; tail = 0; count = 0;
        for (u8 i = 0; i < PLCRUNTIME_COMMS_QUEUE_DEPTH; i++) txns[i].pending = false;
    }
};

// ============================================================================
// Communication Instance
// ============================================================================
struct PLCCommsInstance {
    PLCCommsProtocol protocol;
    bool active;
    void* driver;       // Points to ModbusRTU, SerialRS232, etc.
    u8 lastError;       // Last operation error code
    u8 priority;        // COMMS_PRIORITY_SYNC or COMMS_PRIORITY_ASYNC
    PLCCommsTxnQueue asyncQueue; // Transaction queue for async mode

    PLCCommsInstance() : protocol(COMMS_PROTO_NONE), active(false), driver(nullptr),
                         lastError(0), priority(COMMS_PRIORITY_SYNC) { }
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
        _instances[index].priority = COMMS_PRIORITY_SYNC;
        _instances[index].asyncQueue.clear();
        return true;
    }

    ModbusRTU* getModbusRTU(u8 index) {
        if (index >= PLCRUNTIME_MAX_COMMS_INSTANCES) return nullptr;
        if (_instances[index].protocol != COMMS_PROTO_MODBUS_RTU) return nullptr;
        return (ModbusRTU*) _instances[index].driver;
    }
#endif // PLCRUNTIME_MODBUS_RTU

#ifdef PLCRUNTIME_SERIAL_RS232
    bool registerSerial(u8 index, SerialRS232* driver) {
        if (index >= PLCRUNTIME_MAX_COMMS_INSTANCES || !driver) return false;
        _instances[index].protocol = COMMS_PROTO_SERIAL;
        _instances[index].driver = (void*) driver;
        _instances[index].active = false;
        _instances[index].lastError = 0;
        _instances[index].priority = COMMS_PRIORITY_SYNC;
        _instances[index].asyncQueue.clear();
        return true;
    }

    SerialRS232* getSerial(u8 index) {
        if (index >= PLCRUNTIME_MAX_COMMS_INSTANCES) return nullptr;
        if (_instances[index].protocol != COMMS_PROTO_SERIAL) return nullptr;
        return (SerialRS232*) _instances[index].driver;
    }
#endif // PLCRUNTIME_SERIAL_RS232

#ifdef PLCRUNTIME_ETHERNET_W5500
    // Register an EthernetW5500 driver for both TCP and UDP protocols.
    // The protocol field is set to RAW_TCP by default; the driver handles
    // both TCP and UDP operations transparently.
    bool registerEthernet(u8 index, EthernetW5500* driver) {
        if (index >= PLCRUNTIME_MAX_COMMS_INSTANCES || !driver) return false;
        _instances[index].protocol = COMMS_PROTO_RAW_TCP;
        _instances[index].driver = (void*) driver;
        _instances[index].active = false;
        _instances[index].lastError = 0;
        _instances[index].priority = COMMS_PRIORITY_SYNC;
        _instances[index].asyncQueue.clear();
        return true;
    }

    EthernetW5500* getEthernet(u8 index) {
        if (index >= PLCRUNTIME_MAX_COMMS_INSTANCES) return nullptr;
        PLCCommsProtocol proto = _instances[index].protocol;
        if (proto != COMMS_PROTO_RAW_TCP && proto != COMMS_PROTO_RAW_UDP) return nullptr;
        return (EthernetW5500*) _instances[index].driver;
    }
#endif // PLCRUNTIME_ETHERNET_W5500
};

// Global communication manager instance (like g_ffiRegistry)
static PLCCommsManager g_plcComms;
