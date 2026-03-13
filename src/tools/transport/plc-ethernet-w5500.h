// plc-ethernet-w5500.h - Non-blocking W5500 Wiznet Ethernet driver
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
// W5500 Wiznet Ethernet Driver (Non-blocking, with Socket Reservation)
// ============================================================================
//
// Enable with: #define PLCRUNTIME_ETHERNET_W5500 before including VovkPLCRuntime.h
//
// Non-blocking Ethernet state machine for W5500-based Ethernet modules.
// Handles hardware/software reset, DHCP with static IP fallback, link
// monitoring, TCP server/client, and UDP communication.
//
// Socket Reservation System:
// --------------------------
// The W5500 has 8 hardware sockets (0-7). This driver partitions them into
// pools for thread-safe operation between realtime PLC bytecode I/O and
// background tasks (editor monitoring, program upload, state management).
//
// Socket pools (configurable via ETH_W5500_RT_TCP_SLOTS, etc.):
//   - Realtime TCP slots:    For PLC bytecode TCP I/O (fast, fail-fast if busy)
//   - Background TCP slots:  For editor, monitoring, program upload
//   - Realtime UDP slots:    For PLC bytecode UDP I/O
//   - Background UDP slots:  For background UDP tasks
//   - Background server:     Listening server socket (always background)
//   - System:                DHCP/link management (internal to state machine)
//
// Reservation states prevent collisions:
//   - FREE:        Socket available for acquisition
//   - RESERVED_RT: Acquired by realtime code, not yet in active SPI I/O
//   - RESERVED_BG: Acquired by background code
//   - ACTIVE_RT:   Realtime code is doing SPI I/O on this socket
//   - ACTIVE_BG:   Background code is doing SPI I/O on this socket
//
// Realtime calls fail-fast (ETH_ERR_BUSY) if SPI bus is busy.
// Background calls in poll() skip this cycle if SPI is locked by realtime.
//
// Requires the Arduino Ethernet library (W5500-compatible).
//
// ============================================================================
// Usage Examples
// ============================================================================
//
// 1. Single W5500 (default SPI):
//    --------------------------------
//    #define PLCRUNTIME_ETHERNET_W5500
//    #include <VovkPLCRuntime.h>
//
//    uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x01 };
//    EthernetW5500 eth(10, 9, mac);  // CS=10, RST=9
//
//    void setup() {
//        eth.setDHCP(true);
//        eth.setStaticIP(IPAddress(192,168,1,100),
//                        IPAddress(192,168,1,1),
//                        IPAddress(255,255,255,0),
//                        IPAddress(8,8,8,8));
//        eth.begin();
//        g_plcComms.registerEthernet(0, &eth);
//    }
//
//    void loop() {
//        eth.poll();
//    }
//
// 2. Socket reservation (bytecode runtime):
//    ----------------------------------------
//    // From bytecode, acquire a realtime TCP slot:
//    //   eth_sock_acquire #inst #0     ; acquire realtime TCP -> push slot (0xFF=none)
//    //   tcp_connect #inst #ip0 #ip1 #ip2 #ip3 #port
//    //   tcp_send #inst #src #len
//    //   eth_sock_release #inst #slot  ; release when done
//    //
//    // Background sockets (for editor, monitoring):
//    //   eth_sock_acquire #inst #1     ; acquire background TCP
//    //   tcp_listen #inst #port
//    //   tcp_accept #inst              ; accept connection into active BG slot
//
// 3. Multiple W5500 on same SPI bus (different CS pins):
//    -----------------------------------------------------
//    uint8_t mac1[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x01 };
//    uint8_t mac2[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02 };
//    EthernetW5500 eth1(10, 9, mac1);  // CS=10, RST=9
//    EthernetW5500 eth2( 8, 7, mac2);  // CS=8,  RST=7
//
//    void setup() {
//        eth1.begin(); g_plcComms.registerEthernet(0, &eth1);
//        eth2.begin(); g_plcComms.registerEthernet(1, &eth2);
//    }
//    void loop() { eth1.poll(); eth2.poll(); }
//
// 4. Multiple W5500 on different SPI buses (custom SPI select):
//    -----------------------------------------------------------
//    void selectSPI1() { /* switch mux to SPI1 */ }
//    void selectSPI2() { /* switch mux to SPI2 */ }
//    void deselectSPI() { /* release mux */ }
//
//    EthernetW5500 eth1(10, 9, mac1);
//    EthernetW5500 eth2( 5, 4, mac2);
//
//    void setup() {
//        eth1.setSPICallbacks(selectSPI1, deselectSPI);
//        eth2.setSPICallbacks(selectSPI2, deselectSPI);
//        eth1.begin(); g_plcComms.registerEthernet(0, &eth1);
//        eth2.begin(); g_plcComms.registerEthernet(1, &eth2);
//    }

#pragma once

#ifdef PLCRUNTIME_ETHERNET_W5500

#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <utility/w5100.h>

// ============================================================================
// Configuration
// ============================================================================

#ifndef ETH_W5500_MAX_TCP_CLIENTS
#define ETH_W5500_MAX_TCP_CLIENTS 4
#endif

#ifndef ETH_W5500_RECV_BUFFER_SIZE
#define ETH_W5500_RECV_BUFFER_SIZE 256
#endif

// Socket pool sizes (must sum to <= 8, the W5500 hardware socket limit)
// Realtime slots: used by PLC bytecode for fast, thread-safe I/O
// Background slots: used by editor, monitoring, program upload
#ifndef ETH_W5500_RT_TCP_SLOTS
#define ETH_W5500_RT_TCP_SLOTS 2
#endif
#ifndef ETH_W5500_BG_TCP_SLOTS
#define ETH_W5500_BG_TCP_SLOTS 2
#endif
#ifndef ETH_W5500_RT_UDP_SLOTS
#define ETH_W5500_RT_UDP_SLOTS 1
#endif
#ifndef ETH_W5500_BG_UDP_SLOTS
#define ETH_W5500_BG_UDP_SLOTS 1
#endif
// Total managed slots (server + system are implicit)
// server = 1 BG socket, system = 1 socket for DHCP/link
// Total: RT_TCP + BG_TCP + RT_UDP + BG_UDP + 1(server) + 1(system) <= 8
#define ETH_W5500_TOTAL_TCP_SLOTS (ETH_W5500_RT_TCP_SLOTS + ETH_W5500_BG_TCP_SLOTS)
#define ETH_W5500_TOTAL_UDP_SLOTS (ETH_W5500_RT_UDP_SLOTS + ETH_W5500_BG_UDP_SLOTS)

// SPI bus selection callback type
// Used for multi-W5500 setups where SPI mux switching is needed.
typedef void (*EthSPICallback)();

// ============================================================================
// Socket Reservation States
// ============================================================================

enum EthSocketState : uint8_t {
    ETH_SOCK_FREE       = 0x00,  // Available for acquisition
    ETH_SOCK_RESERVED_RT= 0x01,  // Acquired by realtime code (not in SPI I/O)
    ETH_SOCK_RESERVED_BG= 0x02,  // Acquired by background code
    ETH_SOCK_ACTIVE_RT  = 0x03,  // Realtime code is doing SPI I/O
    ETH_SOCK_ACTIVE_BG  = 0x04,  // Background code is doing SPI I/O
};

// Socket pool type (for acquire/release API)
enum EthSocketPool : uint8_t {
    ETH_POOL_RT_TCP = 0,  // Realtime TCP
    ETH_POOL_BG_TCP = 1,  // Background TCP
    ETH_POOL_RT_UDP = 2,  // Realtime UDP
    ETH_POOL_BG_UDP = 3,  // Background UDP
};

// ============================================================================
// TCP Socket Slot
// ============================================================================

struct EthTcpSlot {
    EthernetClient client;
    EthSocketState state;
    bool isRealtime;        // true = realtime pool, false = background pool

    EthTcpSlot() : state(ETH_SOCK_FREE), isRealtime(false) { }
};

// ============================================================================
// UDP Socket Slot
// ============================================================================

struct EthUdpSlot {
    EthernetUDP udp;
    EthSocketState state;
    bool isRealtime;
    bool open;
    uint16_t port;

    EthUdpSlot() : state(ETH_SOCK_FREE), isRealtime(false), open(false), port(0) { }
};

// ============================================================================
// Ethernet State Machine States
// ============================================================================

enum EthernetState : uint8_t {
    ETH_STATE_IDLE,                  // Normal operation, network is ready
    ETH_STATE_HARD_RESET_START,      // Begin hard reset sequence
    ETH_STATE_HARD_RESET_LOW,        // RST pin held low
    ETH_STATE_HARD_RESET_WAIT,       // Waiting after RST pin released
    ETH_STATE_SOFT_RESET_START,      // Begin soft reset
    ETH_STATE_SOFT_RESET_WAIT,       // Waiting for soft reset to complete
    ETH_STATE_INIT_START,            // Begin initialization
    ETH_STATE_INIT_CHECK_LINK,       // Check link status
    ETH_STATE_INIT_DHCP,             // DHCP in progress
    ETH_STATE_INIT_STATIC,           // Static IP configuration
    ETH_STATE_INIT_SERVER_START,     // Start server
    ETH_STATE_INIT_COMPLETE,         // Initialization complete
    ETH_STATE_DHCP_RETRY,            // Background DHCP retry while on fallback static IP
    ETH_STATE_DISCONNECTED,          // Link is down
    ETH_STATE_ERROR                  // Error state, will retry
};

// ============================================================================
// Result Codes
// ============================================================================

enum EthernetW5500Result : uint8_t {
    ETH_OK                  = 0x00,
    ETH_ERR_NOT_READY       = 0x01,
    ETH_ERR_NOT_CONNECTED   = 0x02,
    ETH_ERR_CONNECT_FAILED  = 0x03,
    ETH_ERR_NO_CLIENT       = 0x04,
    ETH_ERR_SEND_FAILED     = 0x05,
    ETH_ERR_UDP_NOT_OPEN    = 0x06,
    ETH_ERR_UDP_SEND_FAILED = 0x07,
    ETH_ERR_BUSY            = 0x08,
    ETH_ERR_LISTEN_FAILED   = 0x09,
    ETH_ERR_NO_SLOT         = 0x0A,
    ETH_ERR_INVALID_SLOT    = 0x0B,
    ETH_ERR_SLOT_NOT_OWNED  = 0x0C,
    ETH_ERR_INVALID         = 0xFF,
};

// ============================================================================
// W5500 Ethernet Driver Class
// ============================================================================

class EthernetW5500 {
public:

    // Timing constants (milliseconds)
    static constexpr uint32_t HARD_RESET_LOW_TIME       = 5;
    static constexpr uint32_t HARD_RESET_WAIT_TIME      = 2500;
    static constexpr uint32_t SOFT_RESET_TIMEOUT        = 1000;
    static constexpr uint32_t SERVER_START_DELAY         = 10;
    static constexpr uint32_t MIN_HARD_RESET_INTERVAL   = 90000;
    static constexpr uint32_t MIN_SOFT_RESET_INTERVAL   = 45000;
    static constexpr uint32_t DHCP_TIMEOUT              = 10000;
    static constexpr uint32_t DHCP_RESPONSE_TIMEOUT     = 4000;
    static constexpr uint32_t DHCP_RETRY_INTERVAL       = 5000;
    static constexpr uint32_t DHCP_RETRY_TIMEOUT        = 2000;
    static constexpr uint32_t DHCP_RETRY_RESPONSE       = 1000;
    static constexpr uint32_t ERROR_RETRY_DELAY         = 5000;
    static constexpr uint32_t LINK_CHECK_INTERVAL       = 1000;
    static constexpr uint32_t RECONNECT_CHECK_INTERVAL  = 2000;

    // --------------------------------------------------------------------
    // Constructor
    // --------------------------------------------------------------------
    EthernetW5500(int csPin, int rstPin, const uint8_t* mac)
        : _csPin(csPin)
        , _rstPin(rstPin)
        , _spiSelect(nullptr)
        , _spiDeselect(nullptr)
        , _state(ETH_STATE_INIT_START)
        , _prevState(ETH_STATE_IDLE)
        , _stateEnteredAt(0)
        , _lastHardReset(0)
        , _lastSoftReset(0)
        , _lastLinkCheck(0)
        , _lastDhcpRetry(0)
        , _lastReconnectCheck(0)
        , _retryCount(0)
        , _initCycle(0)
        , _linkEstablished(false)
        , _serverReady(false)
        , _dhcpEnabled(true)
        , _dhcpInProgress(false)
        , _dhcpFallbackActive(false)
        , _begun(false)
        , _spiBusy(false)
        , _serverPort(80)
        , _server(nullptr)
        , _activeTcpSlot(0xFF)
        , _activeUdpSlot(0xFF)
    {
        memcpy(_mac, mac, 6);
        _staticIP = IPAddress(192, 168, 1, 100);
        _gateway  = IPAddress(192, 168, 1, 1);
        _subnet   = IPAddress(255, 255, 255, 0);
        _dns      = IPAddress(8, 8, 8, 8);
        _localIP  = IPAddress(0, 0, 0, 0);
        // Initialize socket slot pools
        for (uint8_t i = 0; i < ETH_W5500_TOTAL_TCP_SLOTS; i++) {
            _tcpSlots[i].isRealtime = (i < ETH_W5500_RT_TCP_SLOTS);
        }
        for (uint8_t i = 0; i < ETH_W5500_TOTAL_UDP_SLOTS; i++) {
            _udpSlots[i].isRealtime = (i < ETH_W5500_RT_UDP_SLOTS);
        }
    }

    // --------------------------------------------------------------------
    // Configuration (call before begin())
    // --------------------------------------------------------------------

    void setDHCP(bool enable) { _dhcpEnabled = enable; }
    void setServerPort(uint16_t port) { _serverPort = port; }

    void setStaticIP(IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress dns) {
        _staticIP = ip;
        _gateway  = gateway;
        _subnet   = subnet;
        _dns      = dns;
    }

    void setMAC(const uint8_t* mac) { memcpy(_mac, mac, 6); }

    // Set SPI bus selection callbacks for multi-W5500 or SPI-mux setups.
    void setSPICallbacks(EthSPICallback select, EthSPICallback deselect) {
        _spiSelect = select;
        _spiDeselect = deselect;
    }

    // --------------------------------------------------------------------
    // Lifecycle
    // --------------------------------------------------------------------

    void begin() {
        if (_rstPin >= 0) {
            pinMode(_rstPin, OUTPUT);
            digitalWrite(_rstPin, HIGH);
        }
        spiActivate();
        Ethernet.init(_csPin);
        spiRelease();
        _begun = true;
        _state = ETH_STATE_INIT_START;
        _stateEnteredAt = millis();
    }

    void end() {
        spiActivate();
        if (_server) {
            _serverReady = false;
        }
        // Close all TCP slots
        for (uint8_t i = 0; i < ETH_W5500_TOTAL_TCP_SLOTS; i++) {
            _tcpSlots[i].client.stop();
            _tcpSlots[i].state = ETH_SOCK_FREE;
        }
        // Close all UDP slots
        for (uint8_t i = 0; i < ETH_W5500_TOTAL_UDP_SLOTS; i++) {
            if (_udpSlots[i].open) {
                _udpSlots[i].udp.stop();
                _udpSlots[i].open = false;
            }
            _udpSlots[i].state = ETH_SOCK_FREE;
            _udpSlots[i].port = 0;
        }
        _activeTcpSlot = 0xFF;
        _activeUdpSlot = 0xFF;
        spiRelease();
        _begun = false;
        _linkEstablished = false;
        _serverReady = false;
        _state = ETH_STATE_IDLE;
    }

    // --------------------------------------------------------------------
    // Non-blocking poll (call from loop - background context)
    // --------------------------------------------------------------------

    void poll() {
        if (!_begun) return;
        // Background poll skips if SPI is locked by realtime code
        if (_spiBusy) return;
        spiActivate();

        uint32_t now = millis();
        uint32_t timeInState = now - _stateEnteredAt;

        switch (_state) {

            // =============================================================
            // IDLE - Normal operation
            // =============================================================
            case ETH_STATE_IDLE: {
                if (now - _lastLinkCheck >= LINK_CHECK_INTERVAL) {
                    _lastLinkCheck = now;
                    int linkStatus = Ethernet.linkStatus();
                    if (linkStatus != LinkON) {
                        _linkEstablished = false;
                        _serverReady = false;
                        enterState(ETH_STATE_DISCONNECTED);
                    } else {
                        Ethernet.maintain(); // DHCP lease renewal
                    }
                }
                // Periodic DHCP retry when on fallback static IP
                if (_dhcpFallbackActive && _dhcpEnabled) {
                    if (now - _lastDhcpRetry >= DHCP_RETRY_INTERVAL) {
                        _lastDhcpRetry = now;
                        enterState(ETH_STATE_DHCP_RETRY);
                    }
                }
                break;
            }

            // =============================================================
            // HARD RESET sequence (requires hardware reset pin)
            // =============================================================
            case ETH_STATE_HARD_RESET_START: {
                _lastHardReset = now;
                _linkEstablished = false;
                _serverReady = false;
                if (_rstPin >= 0) {
                    digitalWrite(_rstPin, LOW);
                }
                enterState(ETH_STATE_HARD_RESET_LOW);
                break;
            }

            case ETH_STATE_HARD_RESET_LOW: {
                if (timeInState >= HARD_RESET_LOW_TIME) {
                    if (_rstPin >= 0) {
                        digitalWrite(_rstPin, HIGH);
                    }
                    enterState(ETH_STATE_HARD_RESET_WAIT);
                }
                break;
            }

            case ETH_STATE_HARD_RESET_WAIT: {
                if (timeInState >= HARD_RESET_WAIT_TIME) {
                    enterState(ETH_STATE_SOFT_RESET_START);
                }
                break;
            }

            // =============================================================
            // SOFT RESET sequence
            // =============================================================
            case ETH_STATE_SOFT_RESET_START: {
                _lastSoftReset = now;
                W5100.writeMR(0x80); // Set RST bit in Mode Register
                enterState(ETH_STATE_SOFT_RESET_WAIT);
                break;
            }

            case ETH_STATE_SOFT_RESET_WAIT: {
                uint8_t mr = W5100.readMR();
                if ((mr & 0x80) == 0) {
                    // Soft reset complete
                    enterState(ETH_STATE_INIT_START);
                } else if (timeInState >= SOFT_RESET_TIMEOUT) {
                    enterState(ETH_STATE_ERROR);
                }
                break;
            }

            // =============================================================
            // INITIALIZATION sequence
            // =============================================================
            case ETH_STATE_INIT_START: {
                _initCycle++;
                _retryCount = 0;
                _dhcpFallbackActive = false;
                Ethernet.init(_csPin);
                // Release all socket slots on re-init
                for (uint8_t i = 0; i < ETH_W5500_TOTAL_TCP_SLOTS; i++) {
                    _tcpSlots[i].client.stop();
                    _tcpSlots[i].state = ETH_SOCK_FREE;
                }
                for (uint8_t i = 0; i < ETH_W5500_TOTAL_UDP_SLOTS; i++) {
                    if (_udpSlots[i].open) _udpSlots[i].udp.stop();
                    _udpSlots[i].state = ETH_SOCK_FREE;
                    _udpSlots[i].open = false;
                    _udpSlots[i].port = 0;
                }
                _activeTcpSlot = 0xFF;
                _activeUdpSlot = 0xFF;
                enterState(ETH_STATE_INIT_CHECK_LINK);
                break;
            }

            case ETH_STATE_INIT_CHECK_LINK: {
                int linkStatus = Ethernet.linkStatus();
                if (linkStatus == LinkON) {
                    _linkEstablished = true;
                    if (_dhcpEnabled && !_dhcpFallbackActive) {
                        enterState(ETH_STATE_INIT_DHCP);
                    } else {
                        enterState(ETH_STATE_INIT_STATIC);
                    }
                } else if (linkStatus == LinkOFF) {
                    _linkEstablished = false;
                    enterState(ETH_STATE_DISCONNECTED);
                } else {
                    // Unknown link state - wait then hard reset
                    if (timeInState > 5000) {
                        requestHardReset(true);
                    }
                }
                break;
            }

            case ETH_STATE_INIT_DHCP: {
                if (!_dhcpInProgress) {
                    _dhcpInProgress = true;
                    int result = Ethernet.begin(_mac, DHCP_TIMEOUT, DHCP_RESPONSE_TIMEOUT);
                    _dhcpInProgress = false;

                    if (result == 0) {
                        _retryCount++;
                        if (_retryCount >= 3) {
                            _dhcpFallbackActive = true;
                        }
                        enterState(ETH_STATE_ERROR);
                    } else {
                        _localIP = Ethernet.localIP();
                        _retryCount = 0;
                        enterState(ETH_STATE_INIT_SERVER_START);
                    }
                }
                break;
            }

            case ETH_STATE_INIT_STATIC: {
                Ethernet.begin(_mac, _staticIP, _dns, _gateway, _subnet);
                IPAddress myIP = Ethernet.localIP();
                if (myIP[0] == 0) {
                    _dhcpFallbackActive = false;
                    enterState(ETH_STATE_ERROR);
                } else {
                    _localIP = myIP;
                    enterState(ETH_STATE_INIT_SERVER_START);
                }
                break;
            }

            case ETH_STATE_INIT_SERVER_START: {
                if (timeInState >= SERVER_START_DELAY) {
                    if (!_server) {
                        _server = new EthernetServer(_serverPort);
                    }
                    _server->begin();
                    _serverReady = true;
                    enterState(ETH_STATE_INIT_COMPLETE);
                }
                break;
            }

            case ETH_STATE_INIT_COMPLETE: {
                enterState(ETH_STATE_IDLE);
                break;
            }

            // =============================================================
            // DHCP RETRY - Quick DHCP attempt while on fallback static IP
            // =============================================================
            case ETH_STATE_DHCP_RETRY: {
                int result = Ethernet.begin(_mac, DHCP_RETRY_TIMEOUT, DHCP_RETRY_RESPONSE);

                if (result != 0) {
                    _localIP = Ethernet.localIP();
                    _dhcpFallbackActive = false;
                    if (_server) _server->begin();
                    _serverReady = true;
                    enterState(ETH_STATE_IDLE);
                } else {
                    Ethernet.begin(_mac, _staticIP, _dns, _gateway, _subnet);
                    if (_server) _server->begin();
                    _serverReady = true;
                    enterState(ETH_STATE_IDLE);
                }
                break;
            }

            // =============================================================
            // DISCONNECTED - Link is down
            // =============================================================
            case ETH_STATE_DISCONNECTED: {
                if (timeInState < 100) {
                    _localIP = IPAddress(0, 0, 0, 0);
                }
                if (now - _lastReconnectCheck >= RECONNECT_CHECK_INTERVAL) {
                    _lastReconnectCheck = now;
                    int linkStatus = Ethernet.linkStatus();
                    if (linkStatus == LinkON) {
                        _dhcpFallbackActive = false;
                        enterState(ETH_STATE_INIT_START);
                    }
                }
                break;
            }

            // =============================================================
            // ERROR - Retry after delay
            // =============================================================
            case ETH_STATE_ERROR: {
                if (timeInState >= ERROR_RETRY_DELAY) {
                    _retryCount++;
                    if (_retryCount >= 5) {
                        _retryCount = 0;
                        requestHardReset(true);
                    } else {
                        enterState(ETH_STATE_INIT_START);
                    }
                }
                break;
            }
        }

        spiRelease();
    }

    // --------------------------------------------------------------------
    // Reset requests (non-blocking)
    // --------------------------------------------------------------------

    void requestHardReset(bool force = false) {
        if (_rstPin < 0) {
            requestSoftReset(force);
            return;
        }
        uint32_t elapsed = millis() - _lastHardReset;
        if (elapsed >= MIN_HARD_RESET_INTERVAL || force) {
            enterState(ETH_STATE_HARD_RESET_START);
        }
    }

    void requestSoftReset(bool force = false) {
        uint32_t elapsed = millis() - _lastSoftReset;
        if (elapsed >= MIN_SOFT_RESET_INTERVAL || force) {
            enterState(ETH_STATE_SOFT_RESET_START);
        }
    }

    void requestReconnect() {
        if (!isBusy()) {
            enterState(ETH_STATE_INIT_START);
        }
    }

    // --------------------------------------------------------------------
    // Status queries
    // --------------------------------------------------------------------

    bool isReady()    const { return _state == ETH_STATE_IDLE && _linkEstablished && _serverReady; }
    bool isBusy()     const { return _state != ETH_STATE_IDLE && _state != ETH_STATE_DISCONNECTED && _state != ETH_STATE_ERROR; }
    bool isBegun()    const { return _begun; }
    bool linkUp()     const { return _linkEstablished; }
    bool isSPIBusy()  const { return _spiBusy; }

    EthernetState getState() const { return _state; }

    IPAddress localIP() const { return _localIP; }
    const uint8_t* mac() const { return _mac; }
    uint16_t serverPort() const { return _serverPort; }

    const char* getStateName() const {
        switch (_state) {
            case ETH_STATE_IDLE:                return "IDLE";
            case ETH_STATE_HARD_RESET_START:    return "HARD_RESET_START";
            case ETH_STATE_HARD_RESET_LOW:      return "HARD_RESET_LOW";
            case ETH_STATE_HARD_RESET_WAIT:     return "HARD_RESET_WAIT";
            case ETH_STATE_SOFT_RESET_START:    return "SOFT_RESET_START";
            case ETH_STATE_SOFT_RESET_WAIT:     return "SOFT_RESET_WAIT";
            case ETH_STATE_INIT_START:          return "INIT_START";
            case ETH_STATE_INIT_CHECK_LINK:     return "INIT_CHECK_LINK";
            case ETH_STATE_INIT_DHCP:           return "INIT_DHCP";
            case ETH_STATE_INIT_STATIC:         return "INIT_STATIC";
            case ETH_STATE_INIT_SERVER_START:   return "INIT_SERVER_START";
            case ETH_STATE_INIT_COMPLETE:       return "INIT_COMPLETE";
            case ETH_STATE_DHCP_RETRY:          return "DHCP_RETRY";
            case ETH_STATE_DISCONNECTED:        return "DISCONNECTED";
            case ETH_STATE_ERROR:               return "ERROR";
            default:                            return "UNKNOWN";
        }
    }

    // ====================================================================
    // Socket Reservation API
    // ====================================================================
    // Acquire/release sockets from the realtime or background pools.
    // Returns slot index (0-based within pool type) or 0xFF if no slot free.
    // After acquiring, use the slot-indexed TCP/UDP methods, or set the
    // active slot and use the convenience methods.
    // ====================================================================

    // Acquire a TCP socket slot from the specified pool.
    // Returns the slot index (0..TOTAL_TCP-1) or 0xFF if none available.
    uint8_t acquireTcpSlot(bool realtime) {
        uint8_t start = realtime ? 0 : ETH_W5500_RT_TCP_SLOTS;
        uint8_t end   = realtime ? ETH_W5500_RT_TCP_SLOTS : ETH_W5500_TOTAL_TCP_SLOTS;
        for (uint8_t i = start; i < end; i++) {
            if (_tcpSlots[i].state == ETH_SOCK_FREE) {
                _tcpSlots[i].state = realtime ? ETH_SOCK_RESERVED_RT : ETH_SOCK_RESERVED_BG;
                // Auto-select as active slot for convenience
                _activeTcpSlot = i;
                return i;
            }
        }
        return 0xFF;
    }

    // Release a TCP socket slot back to the pool.
    EthernetW5500Result releaseTcpSlot(uint8_t slot) {
        if (slot >= ETH_W5500_TOTAL_TCP_SLOTS) return ETH_ERR_INVALID_SLOT;
        EthTcpSlot& s = _tcpSlots[slot];
        if (s.state == ETH_SOCK_FREE) return ETH_OK; // Already free
        if (s.state == ETH_SOCK_ACTIVE_RT || s.state == ETH_SOCK_ACTIVE_BG) {
            return ETH_ERR_BUSY; // Can't release while active SPI I/O
        }
        spiActivate();
        s.client.stop();
        spiRelease();
        s.state = ETH_SOCK_FREE;
        if (_activeTcpSlot == slot) _activeTcpSlot = 0xFF;
        return ETH_OK;
    }

    // Acquire a UDP socket slot from the specified pool.
    uint8_t acquireUdpSlot(bool realtime) {
        uint8_t start = realtime ? 0 : ETH_W5500_RT_UDP_SLOTS;
        uint8_t end   = realtime ? ETH_W5500_RT_UDP_SLOTS : ETH_W5500_TOTAL_UDP_SLOTS;
        for (uint8_t i = start; i < end; i++) {
            if (_udpSlots[i].state == ETH_SOCK_FREE) {
                _udpSlots[i].state = realtime ? ETH_SOCK_RESERVED_RT : ETH_SOCK_RESERVED_BG;
                _activeUdpSlot = i;
                return i;
            }
        }
        return 0xFF;
    }

    // Release a UDP socket slot back to the pool.
    EthernetW5500Result releaseUdpSlot(uint8_t slot) {
        if (slot >= ETH_W5500_TOTAL_UDP_SLOTS) return ETH_ERR_INVALID_SLOT;
        EthUdpSlot& s = _udpSlots[slot];
        if (s.state == ETH_SOCK_FREE) return ETH_OK;
        if (s.state == ETH_SOCK_ACTIVE_RT || s.state == ETH_SOCK_ACTIVE_BG) {
            return ETH_ERR_BUSY;
        }
        spiActivate();
        if (s.open) {
            s.udp.stop();
            s.open = false;
        }
        spiRelease();
        s.state = ETH_SOCK_FREE;
        s.port = 0;
        if (_activeUdpSlot == slot) _activeUdpSlot = 0xFF;
        return ETH_OK;
    }

    // Select which TCP slot is "active" for the convenience TCP methods
    void setActiveTcpSlot(uint8_t slot) {
        if (slot < ETH_W5500_TOTAL_TCP_SLOTS) _activeTcpSlot = slot;
    }
    uint8_t getActiveTcpSlot() const { return _activeTcpSlot; }

    // Select which UDP slot is "active" for the convenience UDP methods
    void setActiveUdpSlot(uint8_t slot) {
        if (slot < ETH_W5500_TOTAL_UDP_SLOTS) _activeUdpSlot = slot;
    }
    uint8_t getActiveUdpSlot() const { return _activeUdpSlot; }

    // Query slot states
    EthSocketState tcpSlotState(uint8_t slot) const {
        if (slot >= ETH_W5500_TOTAL_TCP_SLOTS) return ETH_SOCK_FREE;
        return _tcpSlots[slot].state;
    }
    EthSocketState udpSlotState(uint8_t slot) const {
        if (slot >= ETH_W5500_TOTAL_UDP_SLOTS) return ETH_SOCK_FREE;
        return _udpSlots[slot].state;
    }

    uint8_t totalTcpSlots() const { return ETH_W5500_TOTAL_TCP_SLOTS; }
    uint8_t totalUdpSlots() const { return ETH_W5500_TOTAL_UDP_SLOTS; }
    uint8_t rtTcpSlots()    const { return ETH_W5500_RT_TCP_SLOTS; }
    uint8_t bgTcpSlots()    const { return ETH_W5500_BG_TCP_SLOTS; }
    uint8_t rtUdpSlots()    const { return ETH_W5500_RT_UDP_SLOTS; }
    uint8_t bgUdpSlots()    const { return ETH_W5500_BG_UDP_SLOTS; }

    // ====================================================================
    // TCP Client Operations (slot-aware, for bytecode runtime)
    // ====================================================================
    // These operate on the active TCP slot. The active slot must be
    // acquired first via acquireTcpSlot() or setActiveTcpSlot().
    // Realtime slots fail-fast if SPI is busy (background is using it).
    // ====================================================================

    EthernetW5500Result tcpConnect(IPAddress ip, uint16_t port) {
        if (!isReady()) return ETH_ERR_NOT_READY;
        if (_activeTcpSlot >= ETH_W5500_TOTAL_TCP_SLOTS) return ETH_ERR_NO_SLOT;
        EthTcpSlot& slot = _tcpSlots[_activeTcpSlot];
        if (slot.state == ETH_SOCK_FREE) return ETH_ERR_SLOT_NOT_OWNED;
        // Realtime fail-fast if SPI busy
        if (slot.isRealtime && _spiBusy) return ETH_ERR_BUSY;
        EthSocketState prevState = slot.state;
        slot.state = slot.isRealtime ? ETH_SOCK_ACTIVE_RT : ETH_SOCK_ACTIVE_BG;
        spiActivate();
        slot.client.stop();
        bool ok = slot.client.connect(ip, port);
        spiRelease();
        slot.state = prevState;
        return ok ? ETH_OK : ETH_ERR_CONNECT_FAILED;
    }

    void tcpDisconnect() {
        if (_activeTcpSlot >= ETH_W5500_TOTAL_TCP_SLOTS) return;
        EthTcpSlot& slot = _tcpSlots[_activeTcpSlot];
        if (slot.state == ETH_SOCK_FREE) return;
        EthSocketState prevState = slot.state;
        slot.state = slot.isRealtime ? ETH_SOCK_ACTIVE_RT : ETH_SOCK_ACTIVE_BG;
        spiActivate();
        slot.client.stop();
        spiRelease();
        slot.state = prevState;
    }

    bool tcpConnected() {
        if (_activeTcpSlot >= ETH_W5500_TOTAL_TCP_SLOTS) return false;
        EthTcpSlot& slot = _tcpSlots[_activeTcpSlot];
        if (slot.state == ETH_SOCK_FREE) return false;
        if (slot.isRealtime && _spiBusy) return false;
        spiActivate();
        bool c = slot.client.connected();
        spiRelease();
        return c;
    }

    EthernetW5500Result tcpListen(uint16_t port) {
        if (!isReady()) return ETH_ERR_NOT_READY;
        if (_server && port == _serverPort) {
            return ETH_OK;
        }
        spiActivate();
        _serverPort = port;
        if (_server) delete _server;
        _server = new EthernetServer(port);
        _server->begin();
        _serverReady = true;
        spiRelease();
        return ETH_OK;
    }

    bool tcpAccept() {
        if (!_server || !_serverReady) return false;
        if (_activeTcpSlot >= ETH_W5500_TOTAL_TCP_SLOTS) return false;
        EthTcpSlot& slot = _tcpSlots[_activeTcpSlot];
        if (slot.state == ETH_SOCK_FREE) return false;
        spiActivate();
        EthernetClient client = _server->available();
        spiRelease();
        if (client) {
            slot.client = client;
            return true;
        }
        return false;
    }

    uint16_t tcpSend(const uint8_t* data, uint16_t len) {
        if (_activeTcpSlot >= ETH_W5500_TOTAL_TCP_SLOTS) return 0;
        EthTcpSlot& slot = _tcpSlots[_activeTcpSlot];
        if (slot.state == ETH_SOCK_FREE) return 0;
        if (slot.isRealtime && _spiBusy) return 0;
        EthSocketState prevState = slot.state;
        slot.state = slot.isRealtime ? ETH_SOCK_ACTIVE_RT : ETH_SOCK_ACTIVE_BG;
        spiActivate();
        if (!slot.client.connected()) { spiRelease(); slot.state = prevState; return 0; }
        uint16_t sent = (uint16_t) slot.client.write(data, len);
        spiRelease();
        slot.state = prevState;
        return sent;
    }

    uint16_t tcpRecv(uint8_t* buf, uint16_t maxLen) {
        if (_activeTcpSlot >= ETH_W5500_TOTAL_TCP_SLOTS) return 0;
        EthTcpSlot& slot = _tcpSlots[_activeTcpSlot];
        if (slot.state == ETH_SOCK_FREE) return 0;
        if (slot.isRealtime && _spiBusy) return 0;
        EthSocketState prevState = slot.state;
        slot.state = slot.isRealtime ? ETH_SOCK_ACTIVE_RT : ETH_SOCK_ACTIVE_BG;
        spiActivate();
        if (!slot.client.connected() && !slot.client.available()) { spiRelease(); slot.state = prevState; return 0; }
        uint16_t count = 0;
        while (slot.client.available() && count < maxLen) {
            buf[count++] = (uint8_t) slot.client.read();
        }
        spiRelease();
        slot.state = prevState;
        return count;
    }

    uint16_t tcpAvailable() {
        if (_activeTcpSlot >= ETH_W5500_TOTAL_TCP_SLOTS) return 0;
        EthTcpSlot& slot = _tcpSlots[_activeTcpSlot];
        if (slot.state == ETH_SOCK_FREE) return 0;
        if (slot.isRealtime && _spiBusy) return 0;
        spiActivate();
        uint16_t n = (uint16_t) slot.client.available();
        spiRelease();
        return n;
    }

    // ====================================================================
    // UDP Operations (slot-aware, for bytecode runtime)
    // ====================================================================

    EthernetW5500Result udpOpen(uint16_t port) {
        if (!isReady()) return ETH_ERR_NOT_READY;
        if (_activeUdpSlot >= ETH_W5500_TOTAL_UDP_SLOTS) return ETH_ERR_NO_SLOT;
        EthUdpSlot& slot = _udpSlots[_activeUdpSlot];
        if (slot.state == ETH_SOCK_FREE) return ETH_ERR_SLOT_NOT_OWNED;
        if (slot.isRealtime && _spiBusy) return ETH_ERR_BUSY;
        EthSocketState prevState = slot.state;
        slot.state = slot.isRealtime ? ETH_SOCK_ACTIVE_RT : ETH_SOCK_ACTIVE_BG;
        spiActivate();
        if (slot.open) slot.udp.stop();
        bool ok = slot.udp.begin(port);
        spiRelease();
        slot.state = prevState;
        if (ok) {
            slot.open = true;
            slot.port = port;
            return ETH_OK;
        }
        return ETH_ERR_UDP_NOT_OPEN;
    }

    void udpClose() {
        if (_activeUdpSlot >= ETH_W5500_TOTAL_UDP_SLOTS) return;
        EthUdpSlot& slot = _udpSlots[_activeUdpSlot];
        if (slot.state == ETH_SOCK_FREE) return;
        if (slot.open) {
            spiActivate();
            slot.udp.stop();
            spiRelease();
            slot.open = false;
            slot.port = 0;
        }
    }

    uint16_t udpSend(IPAddress ip, uint16_t port, const uint8_t* data, uint16_t len) {
        if (_activeUdpSlot >= ETH_W5500_TOTAL_UDP_SLOTS) return 0;
        EthUdpSlot& slot = _udpSlots[_activeUdpSlot];
        if (!slot.open) return 0;
        if (slot.isRealtime && _spiBusy) return 0;
        EthSocketState prevState = slot.state;
        slot.state = slot.isRealtime ? ETH_SOCK_ACTIVE_RT : ETH_SOCK_ACTIVE_BG;
        spiActivate();
        uint16_t written = 0;
        if (slot.udp.beginPacket(ip, port)) {
            written = (uint16_t) slot.udp.write(data, len);
            slot.udp.endPacket();
        }
        spiRelease();
        slot.state = prevState;
        return written;
    }

    uint16_t udpRecv(uint8_t* buf, uint16_t maxLen) {
        if (_activeUdpSlot >= ETH_W5500_TOTAL_UDP_SLOTS) return 0;
        EthUdpSlot& slot = _udpSlots[_activeUdpSlot];
        if (!slot.open) return 0;
        if (slot.isRealtime && _spiBusy) return 0;
        EthSocketState prevState = slot.state;
        slot.state = slot.isRealtime ? ETH_SOCK_ACTIVE_RT : ETH_SOCK_ACTIVE_BG;
        spiActivate();
        int packetSize = slot.udp.parsePacket();
        if (packetSize <= 0) { spiRelease(); slot.state = prevState; return 0; }
        uint16_t toRead = (uint16_t) packetSize;
        if (toRead > maxLen) toRead = maxLen;
        uint16_t n = (uint16_t) slot.udp.read(buf, toRead);
        spiRelease();
        slot.state = prevState;
        return n;
    }

    uint16_t udpAvailable() {
        if (_activeUdpSlot >= ETH_W5500_TOTAL_UDP_SLOTS) return 0;
        EthUdpSlot& slot = _udpSlots[_activeUdpSlot];
        if (!slot.open) return 0;
        if (slot.isRealtime && _spiBusy) return 0;
        spiActivate();
        uint16_t n = (uint16_t) slot.udp.available();
        spiRelease();
        return n;
    }

    // ====================================================================
    // Access to underlying server (for user code, e.g. REST API)
    // ====================================================================

    EthernetServer* getServer() { return _server; }

private:
    // SPI bus activation with busy flag for collision detection.
    // Realtime callers check _spiBusy before calling; background poll()
    // skips entirely when _spiBusy is set.
    void spiActivate() {
        _spiBusy = true;
        if (_spiSelect) _spiSelect();
        Ethernet.init(_csPin);
    }
    void spiRelease() {
        if (_spiDeselect) _spiDeselect();
        _spiBusy = false;
    }

    void enterState(EthernetState newState) {
        if (_state != newState) {
            _prevState = _state;
            _state = newState;
            _stateEnteredAt = millis();
        }
    }

    // Hardware config
    int _csPin;
    int _rstPin;
    EthSPICallback _spiSelect;
    EthSPICallback _spiDeselect;
    uint8_t _mac[6];

    // Network config
    IPAddress _staticIP;
    IPAddress _gateway;
    IPAddress _subnet;
    IPAddress _dns;
    IPAddress _localIP;
    bool _dhcpEnabled;
    uint16_t _serverPort;

    // State machine
    EthernetState _state;
    EthernetState _prevState;
    uint32_t _stateEnteredAt;
    uint32_t _lastHardReset;
    uint32_t _lastSoftReset;
    uint32_t _lastLinkCheck;
    uint32_t _lastDhcpRetry;
    uint32_t _lastReconnectCheck;
    uint8_t _retryCount;
    uint8_t _initCycle;
    bool _linkEstablished;
    bool _serverReady;
    bool _dhcpInProgress;
    bool _dhcpFallbackActive;
    bool _begun;

    // SPI collision guard
    volatile bool _spiBusy;

    // TCP socket slots (realtime slots first, then background)
    EthTcpSlot _tcpSlots[ETH_W5500_TOTAL_TCP_SLOTS];
    uint8_t _activeTcpSlot;  // Currently selected TCP slot (0xFF = none)

    // UDP socket slots (realtime slots first, then background)
    EthUdpSlot _udpSlots[ETH_W5500_TOTAL_UDP_SLOTS];
    uint8_t _activeUdpSlot;  // Currently selected UDP slot (0xFF = none)

    // Background server
    EthernetServer* _server;
};

#endif // PLCRUNTIME_ETHERNET_W5500
