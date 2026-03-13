// plc-modbus-rtu.h - Modbus RTU RS485 master/slave implementation
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
// Modbus RTU RS485 Library
// ============================================================================
//
// Enable with: #define PLCRUNTIME_MODBUS_RTU before including VovkPLCRuntime.h
//
// Supports both master and slave modes over RS485 via any Arduino Serial
// interface with optional DE/RE (driver enable / receiver enable) pin for
// generic TTL-to-RS485 transceiver modules (e.g. MAX485, SP3485, etc.)
//
// Wiring for generic TTL-to-RS485 module:
//   Arduino TX  -> DI  (Data In)
//   Arduino RX  <- RO  (Receiver Out)
//   Arduino GPIO -> DE + RE tied together (direction control)
//   A/B         -> RS485 bus
//
// If DE_PIN is -1, direction control is disabled (for auto-direction modules
// or RS485 transceivers with built-in flow control).
//
// ============================================================================
// Usage Examples
// ============================================================================
//
// 1. Modbus RTU Slave:
//    -----------------
//    #define PLCRUNTIME_MODBUS_RTU
//    #include <VovkPLCRuntime.h>
//
//    ModbusRTU modbus(Serial1, 9600, 2);  // Serial1, 9600 baud, DE pin 2
//    
//    void setup() {
//        modbus.begin(1);  // Slave address 1
//        modbus.addCoils(0, 16);              // 16 coils starting at address 0
//        modbus.addHoldingRegisters(0, 10);   // 10 holding registers at address 0
//    }
//    void loop() {
//        modbus.poll();  // Process incoming requests
//    }
//
// 2. Modbus RTU Master:
//    ------------------
//    #define PLCRUNTIME_MODBUS_RTU
//    #include <VovkPLCRuntime.h>
//
//    ModbusRTU modbus(Serial1, 9600, 2);
//
//    void setup() {
//        modbus.begin(0);  // Address 0 = master mode
//    }
//    void loop() {
//        uint16_t regs[4];
//        if (modbus.readHoldingRegisters(1, 0, 4, regs) == MODBUS_OK) {
//            // regs[0..3] contain the values from slave 1
//        }
//        delay(100);
//    }
//

#pragma once

#ifdef PLCRUNTIME_MODBUS_RTU

#include <Arduino.h>

// ============================================================================
// Configuration
// ============================================================================

#ifndef MODBUS_RTU_MAX_PDU
#define MODBUS_RTU_MAX_PDU 253  // Max Modbus PDU size (without address + CRC)
#endif

#ifndef MODBUS_RTU_MAX_ADU
#define MODBUS_RTU_MAX_ADU (1 + MODBUS_RTU_MAX_PDU + 2)  // Address + PDU + CRC16
#endif

#ifndef MODBUS_RTU_DEFAULT_TIMEOUT_MS
#define MODBUS_RTU_DEFAULT_TIMEOUT_MS 1000
#endif

#ifndef MODBUS_RTU_MAX_COILS
#define MODBUS_RTU_MAX_COILS 128
#endif

#ifndef MODBUS_RTU_MAX_HOLDING_REGS
#define MODBUS_RTU_MAX_HOLDING_REGS 64
#endif

#ifndef MODBUS_RTU_MAX_INPUT_REGS
#define MODBUS_RTU_MAX_INPUT_REGS 64
#endif

#ifndef MODBUS_RTU_MAX_DISCRETE_INPUTS
#define MODBUS_RTU_MAX_DISCRETE_INPUTS 128
#endif

// ============================================================================
// Modbus Function Codes
// ============================================================================

enum ModbusFunctionCode : uint8_t {
    MODBUS_FC_READ_COILS                = 0x01,
    MODBUS_FC_READ_DISCRETE_INPUTS      = 0x02,
    MODBUS_FC_READ_HOLDING_REGISTERS    = 0x03,
    MODBUS_FC_READ_INPUT_REGISTERS      = 0x04,
    MODBUS_FC_WRITE_SINGLE_COIL         = 0x05,
    MODBUS_FC_WRITE_SINGLE_REGISTER     = 0x06,
    MODBUS_FC_WRITE_MULTIPLE_COILS      = 0x0F,
    MODBUS_FC_WRITE_MULTIPLE_REGISTERS  = 0x10,
};

// ============================================================================
// Modbus Exception Codes
// ============================================================================

enum ModbusException : uint8_t {
    MODBUS_EX_NONE                      = 0x00,
    MODBUS_EX_ILLEGAL_FUNCTION          = 0x01,
    MODBUS_EX_ILLEGAL_DATA_ADDRESS      = 0x02,
    MODBUS_EX_ILLEGAL_DATA_VALUE        = 0x03,
    MODBUS_EX_SLAVE_DEVICE_FAILURE      = 0x04,
    MODBUS_EX_ACKNOWLEDGE               = 0x05,
    MODBUS_EX_SLAVE_DEVICE_BUSY         = 0x06,
};

// ============================================================================
// Modbus Result Codes (for master operations)
// ============================================================================

enum ModbusResult : uint8_t {
    MODBUS_OK                           = 0x00,
    MODBUS_ERR_TIMEOUT                  = 0x01,
    MODBUS_ERR_CRC                      = 0x02,
    MODBUS_ERR_FRAME                    = 0x03,
    MODBUS_ERR_EXCEPTION                = 0x04,
    MODBUS_ERR_SLAVE_ADDR               = 0x05,
    MODBUS_ERR_BUFFER_OVERFLOW          = 0x06,
    MODBUS_ERR_INVALID_PARAMS           = 0x07,
};

// ============================================================================
// Modbus CRC16 (polynomial 0xA001)
// ============================================================================

static uint16_t modbus_crc16(const uint8_t* data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// ============================================================================
// Slave Data Blocks
// ============================================================================

struct ModbusCoilBlock {
    uint16_t startAddress = 0;
    uint16_t count = 0;
    uint8_t data[(MODBUS_RTU_MAX_COILS + 7) / 8] = { 0 };

    bool valid() const { return count > 0; }

    bool contains(uint16_t addr, uint16_t qty) const {
        return valid() && addr >= startAddress && (addr + qty) <= (startAddress + count);
    }

    bool getCoil(uint16_t addr) const {
        uint16_t offset = addr - startAddress;
        return (data[offset / 8] >> (offset % 8)) & 0x01;
    }

    void setCoil(uint16_t addr, bool value) {
        uint16_t offset = addr - startAddress;
        if (value) data[offset / 8] |= (1 << (offset % 8));
        else       data[offset / 8] &= ~(1 << (offset % 8));
    }
};

struct ModbusRegisterBlock {
    uint16_t startAddress = 0;
    uint16_t count = 0;
    uint16_t data[MODBUS_RTU_MAX_HOLDING_REGS] = { 0 };

    bool valid() const { return count > 0; }

    bool contains(uint16_t addr, uint16_t qty) const {
        return valid() && addr >= startAddress && (addr + qty) <= (startAddress + count);
    }

    uint16_t getReg(uint16_t addr) const {
        return data[addr - startAddress];
    }

    void setReg(uint16_t addr, uint16_t value) {
        data[addr - startAddress] = value;
    }
};

struct ModbusDiscreteInputBlock {
    uint16_t startAddress = 0;
    uint16_t count = 0;
    uint8_t data[(MODBUS_RTU_MAX_DISCRETE_INPUTS + 7) / 8] = { 0 };

    bool valid() const { return count > 0; }

    bool contains(uint16_t addr, uint16_t qty) const {
        return valid() && addr >= startAddress && (addr + qty) <= (startAddress + count);
    }

    bool getInput(uint16_t addr) const {
        uint16_t offset = addr - startAddress;
        return (data[offset / 8] >> (offset % 8)) & 0x01;
    }

    void setInput(uint16_t addr, bool value) {
        uint16_t offset = addr - startAddress;
        if (value) data[offset / 8] |= (1 << (offset % 8));
        else       data[offset / 8] &= ~(1 << (offset % 8));
    }
};

struct ModbusInputRegisterBlock {
    uint16_t startAddress = 0;
    uint16_t count = 0;
    uint16_t data[MODBUS_RTU_MAX_INPUT_REGS] = { 0 };

    bool valid() const { return count > 0; }

    bool contains(uint16_t addr, uint16_t qty) const {
        return valid() && addr >= startAddress && (addr + qty) <= (startAddress + count);
    }

    uint16_t getReg(uint16_t addr) const {
        return data[addr - startAddress];
    }

    void setReg(uint16_t addr, uint16_t value) {
        data[addr - startAddress] = value;
    }
};

// ============================================================================
// ModbusRTU Class
// ============================================================================

class ModbusRTU {
private:
    Stream* _serial;
    int16_t _dePin;
    uint32_t _baudrate;
    uint8_t _slaveAddr;           // 0 = master mode, 1-247 = slave mode
    uint32_t _t15;                // Inter-character timeout (1.5 char times) in microseconds
    uint32_t _t35;                // Inter-frame delay (3.5 char times) in microseconds
    uint32_t _timeoutMs;          // Response timeout for master mode

    // TX/RX buffer
    uint8_t _buf[MODBUS_RTU_MAX_ADU];
    uint16_t _bufLen;

    // Slave data areas
    ModbusCoilBlock _coils;
    ModbusRegisterBlock _holdingRegs;
    ModbusDiscreteInputBlock _discreteInputs;
    ModbusInputRegisterBlock _inputRegs;

    // Statistics
    uint32_t _rxCount;
    uint32_t _txCount;
    uint32_t _errCount;
    ModbusResult _lastError;

    // User callback for custom function codes (slave mode)
    typedef ModbusException (*CustomFunctionHandler)(uint8_t fc, const uint8_t* request, uint16_t reqLen, uint8_t* response, uint16_t* respLen);
    CustomFunctionHandler _customHandler;

    // ========================================================================
    // RS485 Direction Control
    // ========================================================================

    void setTxMode() {
        if (_dePin >= 0) {
            digitalWrite(_dePin, HIGH);
            delayMicroseconds(10);  // Small settling time for transceiver
        }
    }

    void setRxMode() {
        if (_dePin >= 0) {
            // Flush any remaining TX data before switching direction
            _serial->flush();
            delayMicroseconds(10);
            digitalWrite(_dePin, LOW);
        }
    }

    // ========================================================================
    // Timing Calculations
    // ========================================================================

    void calculateTimings() {
        // Character time = 11 bits / baudrate (1 start + 8 data + 1 parity/stop + 1 stop)
        // For baudrates >= 19200, use fixed values per Modbus spec
        if (_baudrate >= 19200) {
            _t15 = 750;    // 750 us fixed
            _t35 = 1750;   // 1750 us fixed
        } else {
            // t1.5 = 1.5 * 11 / baudrate * 1000000 = 16500000 / baudrate
            _t15 = 16500000UL / _baudrate;
            // t3.5 = 3.5 * 11 / baudrate * 1000000 = 38500000 / baudrate
            _t35 = 38500000UL / _baudrate;
        }
    }

    // ========================================================================
    // Frame TX/RX
    // ========================================================================

    void sendFrame(const uint8_t* data, uint16_t length) {
        // Wait inter-frame gap before transmitting
        delayMicroseconds(_t35);

        setTxMode();
        _serial->write(data, length);
        _serial->flush();  // Wait for all data to be sent
        setRxMode();

        _txCount++;
    }

    void sendResponse(uint8_t slaveAddr, const uint8_t* pdu, uint16_t pduLen) {
        if (pduLen + 3 > MODBUS_RTU_MAX_ADU) return;
        uint8_t frame[MODBUS_RTU_MAX_ADU];
        frame[0] = slaveAddr;
        memcpy(&frame[1], pdu, pduLen);
        uint16_t crc = modbus_crc16(frame, pduLen + 1);
        frame[pduLen + 1] = crc & 0xFF;         // CRC low byte first
        frame[pduLen + 2] = (crc >> 8) & 0xFF;  // CRC high byte
        sendFrame(frame, pduLen + 3);
    }

    void sendException(uint8_t slaveAddr, uint8_t fc, ModbusException ex) {
        uint8_t pdu[2];
        pdu[0] = fc | 0x80;  // Set exception bit
        pdu[1] = ex;
        sendResponse(slaveAddr, pdu, 2);
        _errCount++;
    }

    /**
     * @brief Receive a complete Modbus RTU frame
     * @param timeoutMs Maximum time to wait for first byte
     * @return Number of bytes received (including address + CRC), 0 on timeout/error
     */
    uint16_t receiveFrame(uint32_t timeoutMs) {
        _bufLen = 0;
        uint32_t startMs = millis();

        // Wait for first byte
        while (!_serial->available()) {
            if ((millis() - startMs) > timeoutMs) return 0;
        }

        // Read bytes, using t1.5 silence to detect end-of-frame
        uint32_t lastByteTime = micros();
        while (_bufLen < MODBUS_RTU_MAX_ADU) {
            if (_serial->available()) {
                int b = _serial->read();
                if (b < 0) break;
                _buf[_bufLen++] = (uint8_t)b;
                lastByteTime = micros();
            } else {
                // Check for inter-frame gap (t3.5) to detect end of frame
                uint32_t elapsed = micros() - lastByteTime;
                if (elapsed > _t35) break;
            }
        }

        // Minimum valid frame: address(1) + fc(1) + CRC(2) = 4 bytes
        if (_bufLen < 4) {
            if (_bufLen > 0) _errCount++;
            return 0;
        }

        // Verify CRC
        uint16_t receivedCrc = (uint16_t)_buf[_bufLen - 2] | ((uint16_t)_buf[_bufLen - 1] << 8);
        uint16_t calculatedCrc = modbus_crc16(_buf, _bufLen - 2);
        if (receivedCrc != calculatedCrc) {
            _errCount++;
            _lastError = MODBUS_ERR_CRC;
            return 0;
        }

        _rxCount++;
        return _bufLen;
    }

    // ========================================================================
    // Slave Mode: Request Handlers
    // ========================================================================

    void handleReadCoils(uint8_t slaveAddr, const uint8_t* pdu, uint16_t pduLen) {
        if (pduLen < 5) { sendException(slaveAddr, MODBUS_FC_READ_COILS, MODBUS_EX_ILLEGAL_DATA_VALUE); return; }
        uint16_t startAddr = ((uint16_t)pdu[1] << 8) | pdu[2];
        uint16_t quantity  = ((uint16_t)pdu[3] << 8) | pdu[4];
        if (quantity == 0 || quantity > 2000) { sendException(slaveAddr, MODBUS_FC_READ_COILS, MODBUS_EX_ILLEGAL_DATA_VALUE); return; }
        if (!_coils.contains(startAddr, quantity)) { sendException(slaveAddr, MODBUS_FC_READ_COILS, MODBUS_EX_ILLEGAL_DATA_ADDRESS); return; }

        uint8_t byteCount = (quantity + 7) / 8;
        uint8_t resp[2 + (2000 + 7) / 8];
        resp[0] = MODBUS_FC_READ_COILS;
        resp[1] = byteCount;
        memset(&resp[2], 0, byteCount);
        for (uint16_t i = 0; i < quantity; i++) {
            if (_coils.getCoil(startAddr + i))
                resp[2 + i / 8] |= (1 << (i % 8));
        }
        sendResponse(slaveAddr, resp, 2 + byteCount);
    }

    void handleReadDiscreteInputs(uint8_t slaveAddr, const uint8_t* pdu, uint16_t pduLen) {
        if (pduLen < 5) { sendException(slaveAddr, MODBUS_FC_READ_DISCRETE_INPUTS, MODBUS_EX_ILLEGAL_DATA_VALUE); return; }
        uint16_t startAddr = ((uint16_t)pdu[1] << 8) | pdu[2];
        uint16_t quantity  = ((uint16_t)pdu[3] << 8) | pdu[4];
        if (quantity == 0 || quantity > 2000) { sendException(slaveAddr, MODBUS_FC_READ_DISCRETE_INPUTS, MODBUS_EX_ILLEGAL_DATA_VALUE); return; }
        if (!_discreteInputs.contains(startAddr, quantity)) { sendException(slaveAddr, MODBUS_FC_READ_DISCRETE_INPUTS, MODBUS_EX_ILLEGAL_DATA_ADDRESS); return; }

        uint8_t byteCount = (quantity + 7) / 8;
        uint8_t resp[2 + (2000 + 7) / 8];
        resp[0] = MODBUS_FC_READ_DISCRETE_INPUTS;
        resp[1] = byteCount;
        memset(&resp[2], 0, byteCount);
        for (uint16_t i = 0; i < quantity; i++) {
            if (_discreteInputs.getInput(startAddr + i))
                resp[2 + i / 8] |= (1 << (i % 8));
        }
        sendResponse(slaveAddr, resp, 2 + byteCount);
    }

    void handleReadHoldingRegisters(uint8_t slaveAddr, const uint8_t* pdu, uint16_t pduLen) {
        if (pduLen < 5) { sendException(slaveAddr, MODBUS_FC_READ_HOLDING_REGISTERS, MODBUS_EX_ILLEGAL_DATA_VALUE); return; }
        uint16_t startAddr = ((uint16_t)pdu[1] << 8) | pdu[2];
        uint16_t quantity  = ((uint16_t)pdu[3] << 8) | pdu[4];
        if (quantity == 0 || quantity > 125) { sendException(slaveAddr, MODBUS_FC_READ_HOLDING_REGISTERS, MODBUS_EX_ILLEGAL_DATA_VALUE); return; }
        if (!_holdingRegs.contains(startAddr, quantity)) { sendException(slaveAddr, MODBUS_FC_READ_HOLDING_REGISTERS, MODBUS_EX_ILLEGAL_DATA_ADDRESS); return; }

        uint8_t resp[2 + 125 * 2];
        resp[0] = MODBUS_FC_READ_HOLDING_REGISTERS;
        resp[1] = quantity * 2;
        for (uint16_t i = 0; i < quantity; i++) {
            uint16_t val = _holdingRegs.getReg(startAddr + i);
            resp[2 + i * 2]     = (val >> 8) & 0xFF;
            resp[2 + i * 2 + 1] = val & 0xFF;
        }
        sendResponse(slaveAddr, resp, 2 + quantity * 2);
    }

    void handleReadInputRegisters(uint8_t slaveAddr, const uint8_t* pdu, uint16_t pduLen) {
        if (pduLen < 5) { sendException(slaveAddr, MODBUS_FC_READ_INPUT_REGISTERS, MODBUS_EX_ILLEGAL_DATA_VALUE); return; }
        uint16_t startAddr = ((uint16_t)pdu[1] << 8) | pdu[2];
        uint16_t quantity  = ((uint16_t)pdu[3] << 8) | pdu[4];
        if (quantity == 0 || quantity > 125) { sendException(slaveAddr, MODBUS_FC_READ_INPUT_REGISTERS, MODBUS_EX_ILLEGAL_DATA_VALUE); return; }
        if (!_inputRegs.contains(startAddr, quantity)) { sendException(slaveAddr, MODBUS_FC_READ_INPUT_REGISTERS, MODBUS_EX_ILLEGAL_DATA_ADDRESS); return; }

        uint8_t resp[2 + 125 * 2];
        resp[0] = MODBUS_FC_READ_INPUT_REGISTERS;
        resp[1] = quantity * 2;
        for (uint16_t i = 0; i < quantity; i++) {
            uint16_t val = _inputRegs.getReg(startAddr + i);
            resp[2 + i * 2]     = (val >> 8) & 0xFF;
            resp[2 + i * 2 + 1] = val & 0xFF;
        }
        sendResponse(slaveAddr, resp, 2 + quantity * 2);
    }

    void handleWriteSingleCoil(uint8_t slaveAddr, const uint8_t* pdu, uint16_t pduLen) {
        if (pduLen < 5) { sendException(slaveAddr, MODBUS_FC_WRITE_SINGLE_COIL, MODBUS_EX_ILLEGAL_DATA_VALUE); return; }
        uint16_t addr  = ((uint16_t)pdu[1] << 8) | pdu[2];
        uint16_t value = ((uint16_t)pdu[3] << 8) | pdu[4];
        if (value != 0x0000 && value != 0xFF00) { sendException(slaveAddr, MODBUS_FC_WRITE_SINGLE_COIL, MODBUS_EX_ILLEGAL_DATA_VALUE); return; }
        if (!_coils.contains(addr, 1)) { sendException(slaveAddr, MODBUS_FC_WRITE_SINGLE_COIL, MODBUS_EX_ILLEGAL_DATA_ADDRESS); return; }

        _coils.setCoil(addr, value == 0xFF00);
        // Echo back the request as response
        sendResponse(slaveAddr, pdu, 5);
    }

    void handleWriteSingleRegister(uint8_t slaveAddr, const uint8_t* pdu, uint16_t pduLen) {
        if (pduLen < 5) { sendException(slaveAddr, MODBUS_FC_WRITE_SINGLE_REGISTER, MODBUS_EX_ILLEGAL_DATA_VALUE); return; }
        uint16_t addr  = ((uint16_t)pdu[1] << 8) | pdu[2];
        uint16_t value = ((uint16_t)pdu[3] << 8) | pdu[4];
        if (!_holdingRegs.contains(addr, 1)) { sendException(slaveAddr, MODBUS_FC_WRITE_SINGLE_REGISTER, MODBUS_EX_ILLEGAL_DATA_ADDRESS); return; }

        _holdingRegs.setReg(addr, value);
        sendResponse(slaveAddr, pdu, 5);
    }

    void handleWriteMultipleCoils(uint8_t slaveAddr, const uint8_t* pdu, uint16_t pduLen) {
        if (pduLen < 6) { sendException(slaveAddr, MODBUS_FC_WRITE_MULTIPLE_COILS, MODBUS_EX_ILLEGAL_DATA_VALUE); return; }
        uint16_t startAddr = ((uint16_t)pdu[1] << 8) | pdu[2];
        uint16_t quantity  = ((uint16_t)pdu[3] << 8) | pdu[4];
        uint8_t byteCount  = pdu[5];
        if (quantity == 0 || quantity > 1968) { sendException(slaveAddr, MODBUS_FC_WRITE_MULTIPLE_COILS, MODBUS_EX_ILLEGAL_DATA_VALUE); return; }
        if (byteCount != (quantity + 7) / 8) { sendException(slaveAddr, MODBUS_FC_WRITE_MULTIPLE_COILS, MODBUS_EX_ILLEGAL_DATA_VALUE); return; }
        if (pduLen < (uint16_t)(6 + byteCount)) { sendException(slaveAddr, MODBUS_FC_WRITE_MULTIPLE_COILS, MODBUS_EX_ILLEGAL_DATA_VALUE); return; }
        if (!_coils.contains(startAddr, quantity)) { sendException(slaveAddr, MODBUS_FC_WRITE_MULTIPLE_COILS, MODBUS_EX_ILLEGAL_DATA_ADDRESS); return; }

        for (uint16_t i = 0; i < quantity; i++) {
            bool val = (pdu[6 + i / 8] >> (i % 8)) & 0x01;
            _coils.setCoil(startAddr + i, val);
        }
        // Response: FC + start addr + quantity
        uint8_t resp[5];
        resp[0] = MODBUS_FC_WRITE_MULTIPLE_COILS;
        resp[1] = pdu[1]; resp[2] = pdu[2];
        resp[3] = pdu[3]; resp[4] = pdu[4];
        sendResponse(slaveAddr, resp, 5);
    }

    void handleWriteMultipleRegisters(uint8_t slaveAddr, const uint8_t* pdu, uint16_t pduLen) {
        if (pduLen < 6) { sendException(slaveAddr, MODBUS_FC_WRITE_MULTIPLE_REGISTERS, MODBUS_EX_ILLEGAL_DATA_VALUE); return; }
        uint16_t startAddr = ((uint16_t)pdu[1] << 8) | pdu[2];
        uint16_t quantity  = ((uint16_t)pdu[3] << 8) | pdu[4];
        uint8_t byteCount  = pdu[5];
        if (quantity == 0 || quantity > 123) { sendException(slaveAddr, MODBUS_FC_WRITE_MULTIPLE_REGISTERS, MODBUS_EX_ILLEGAL_DATA_VALUE); return; }
        if (byteCount != quantity * 2) { sendException(slaveAddr, MODBUS_FC_WRITE_MULTIPLE_REGISTERS, MODBUS_EX_ILLEGAL_DATA_VALUE); return; }
        if (pduLen < (uint16_t)(6 + byteCount)) { sendException(slaveAddr, MODBUS_FC_WRITE_MULTIPLE_REGISTERS, MODBUS_EX_ILLEGAL_DATA_VALUE); return; }
        if (!_holdingRegs.contains(startAddr, quantity)) { sendException(slaveAddr, MODBUS_FC_WRITE_MULTIPLE_REGISTERS, MODBUS_EX_ILLEGAL_DATA_ADDRESS); return; }

        for (uint16_t i = 0; i < quantity; i++) {
            uint16_t val = ((uint16_t)pdu[6 + i * 2] << 8) | pdu[6 + i * 2 + 1];
            _holdingRegs.setReg(startAddr + i, val);
        }
        uint8_t resp[5];
        resp[0] = MODBUS_FC_WRITE_MULTIPLE_REGISTERS;
        resp[1] = pdu[1]; resp[2] = pdu[2];
        resp[3] = pdu[3]; resp[4] = pdu[4];
        sendResponse(slaveAddr, resp, 5);
    }

    // ========================================================================
    // Slave Mode: Request Dispatcher
    // ========================================================================

    void processSlaveRequest() {
        uint8_t addr = _buf[0];
        // Not for us (and not broadcast)
        if (addr != _slaveAddr && addr != 0) return;

        const uint8_t* pdu = &_buf[1];
        uint16_t pduLen = _bufLen - 3;  // Remove address and CRC
        uint8_t fc = pdu[0];

        switch (fc) {
            case MODBUS_FC_READ_COILS:              handleReadCoils(addr, pdu, pduLen); break;
            case MODBUS_FC_READ_DISCRETE_INPUTS:    handleReadDiscreteInputs(addr, pdu, pduLen); break;
            case MODBUS_FC_READ_HOLDING_REGISTERS:  handleReadHoldingRegisters(addr, pdu, pduLen); break;
            case MODBUS_FC_READ_INPUT_REGISTERS:    handleReadInputRegisters(addr, pdu, pduLen); break;
            case MODBUS_FC_WRITE_SINGLE_COIL:       handleWriteSingleCoil(addr, pdu, pduLen); break;
            case MODBUS_FC_WRITE_SINGLE_REGISTER:   handleWriteSingleRegister(addr, pdu, pduLen); break;
            case MODBUS_FC_WRITE_MULTIPLE_COILS:    handleWriteMultipleCoils(addr, pdu, pduLen); break;
            case MODBUS_FC_WRITE_MULTIPLE_REGISTERS: handleWriteMultipleRegisters(addr, pdu, pduLen); break;
            default:
                if (_customHandler) {
                    uint8_t respBuf[MODBUS_RTU_MAX_PDU];
                    uint16_t respLen = 0;
                    ModbusException ex = _customHandler(fc, pdu, pduLen, respBuf, &respLen);
                    if (ex != MODBUS_EX_NONE) {
                        sendException(addr, fc, ex);
                    } else if (respLen > 0) {
                        sendResponse(addr, respBuf, respLen);
                    }
                } else {
                    sendException(addr, fc, MODBUS_EX_ILLEGAL_FUNCTION);
                }
                break;
        }
    }

    // ========================================================================
    // Master Mode: Send Request and Receive Response
    // ========================================================================

    ModbusResult masterTransaction(uint8_t slaveAddr, const uint8_t* pdu, uint16_t pduLen,
                                   uint8_t* responsePdu, uint16_t* responseLen, uint16_t maxResponseLen) {
        if (slaveAddr == 0 || slaveAddr > 247) return MODBUS_ERR_INVALID_PARAMS;
        if (pduLen + 3 > MODBUS_RTU_MAX_ADU) return MODBUS_ERR_BUFFER_OVERFLOW;

        // Build and send request frame
        uint8_t frame[MODBUS_RTU_MAX_ADU];
        frame[0] = slaveAddr;
        memcpy(&frame[1], pdu, pduLen);
        uint16_t crc = modbus_crc16(frame, pduLen + 1);
        frame[pduLen + 1] = crc & 0xFF;
        frame[pduLen + 2] = (crc >> 8) & 0xFF;

        // Drain any stale data in RX buffer before sending
        while (_serial->available()) _serial->read();

        sendFrame(frame, pduLen + 3);

        // Receive response
        uint16_t rxLen = receiveFrame(_timeoutMs);
        if (rxLen == 0) {
            _lastError = MODBUS_ERR_TIMEOUT;
            return MODBUS_ERR_TIMEOUT;
        }

        // Verify slave address
        if (_buf[0] != slaveAddr) {
            _lastError = MODBUS_ERR_SLAVE_ADDR;
            return MODBUS_ERR_SLAVE_ADDR;
        }

        // Check for exception response
        if (_buf[1] & 0x80) {
            _lastError = MODBUS_ERR_EXCEPTION;
            if (responsePdu && maxResponseLen >= 2) {
                responsePdu[0] = _buf[1];
                responsePdu[1] = _buf[2];
                *responseLen = 2;
            }
            return MODBUS_ERR_EXCEPTION;
        }

        // Copy PDU to caller buffer
        uint16_t respPduLen = rxLen - 3;  // Remove address + CRC
        if (respPduLen > maxResponseLen) {
            _lastError = MODBUS_ERR_BUFFER_OVERFLOW;
            return MODBUS_ERR_BUFFER_OVERFLOW;
        }
        if (responsePdu) {
            memcpy(responsePdu, &_buf[1], respPduLen);
            *responseLen = respPduLen;
        }

        _lastError = MODBUS_OK;
        return MODBUS_OK;
    }

public:

    // ========================================================================
    // Constructor
    // ========================================================================

    /**
     * @brief Construct a ModbusRTU instance
     * @param serial Reference to a hardware or software serial port (must be begun by user)
     * @param baudrate Baudrate (used for timing calculations; user must call Serial.begin() separately)
     * @param dePin GPIO pin connected to DE+RE on the RS485 transceiver. Use -1 for auto-direction modules.
     */
    ModbusRTU(Stream& serial, uint32_t baudrate = 9600, int16_t dePin = -1)
        : _serial(&serial)
        , _dePin(dePin)
        , _baudrate(baudrate)
        , _slaveAddr(0)
        , _t15(0)
        , _t35(0)
        , _timeoutMs(MODBUS_RTU_DEFAULT_TIMEOUT_MS)
        , _bufLen(0)
        , _rxCount(0)
        , _txCount(0)
        , _errCount(0)
        , _lastError(MODBUS_OK)
        , _customHandler(nullptr)
    {}

    // ========================================================================
    // Initialization
    // ========================================================================

    /**
     * @brief Initialize modbus communication
     * @param slaveAddress Slave address (1-247) for slave mode, 0 for master mode
     * 
     * NOTE: The user must call Serial.begin(baudrate) before calling this method.
     */
    void begin(uint8_t slaveAddress = 0) {
        _slaveAddr = slaveAddress;
        calculateTimings();
        if (_dePin >= 0) {
            pinMode(_dePin, OUTPUT);
            digitalWrite(_dePin, LOW);  // Start in RX mode
        }
    }

    // ========================================================================
    // Slave Mode: Data Area Configuration
    // ========================================================================

    /**
     * @brief Configure coil block (FC 01, 05, 0F)
     * @param startAddress Starting Modbus address
     * @param count Number of coils (max MODBUS_RTU_MAX_COILS)
     */
    void addCoils(uint16_t startAddress, uint16_t count) {
        if (count > MODBUS_RTU_MAX_COILS) count = MODBUS_RTU_MAX_COILS;
        _coils.startAddress = startAddress;
        _coils.count = count;
        memset(_coils.data, 0, sizeof(_coils.data));
    }

    /**
     * @brief Configure discrete input block (FC 02)
     * @param startAddress Starting Modbus address
     * @param count Number of discrete inputs (max MODBUS_RTU_MAX_DISCRETE_INPUTS)
     */
    void addDiscreteInputs(uint16_t startAddress, uint16_t count) {
        if (count > MODBUS_RTU_MAX_DISCRETE_INPUTS) count = MODBUS_RTU_MAX_DISCRETE_INPUTS;
        _discreteInputs.startAddress = startAddress;
        _discreteInputs.count = count;
        memset(_discreteInputs.data, 0, sizeof(_discreteInputs.data));
    }

    /**
     * @brief Configure holding register block (FC 03, 06, 10)
     * @param startAddress Starting Modbus address
     * @param count Number of registers (max MODBUS_RTU_MAX_HOLDING_REGS)
     */
    void addHoldingRegisters(uint16_t startAddress, uint16_t count) {
        if (count > MODBUS_RTU_MAX_HOLDING_REGS) count = MODBUS_RTU_MAX_HOLDING_REGS;
        _holdingRegs.startAddress = startAddress;
        _holdingRegs.count = count;
        memset(_holdingRegs.data, 0, sizeof(_holdingRegs.data));
    }

    /**
     * @brief Configure input register block (FC 04)
     * @param startAddress Starting Modbus address
     * @param count Number of registers (max MODBUS_RTU_MAX_INPUT_REGS)
     */
    void addInputRegisters(uint16_t startAddress, uint16_t count) {
        if (count > MODBUS_RTU_MAX_INPUT_REGS) count = MODBUS_RTU_MAX_INPUT_REGS;
        _inputRegs.startAddress = startAddress;
        _inputRegs.count = count;
        memset(_inputRegs.data, 0, sizeof(_inputRegs.data));
    }

    // ========================================================================
    // Slave Mode: Direct Data Access
    // ========================================================================

    bool coil(uint16_t address) const { return _coils.contains(address, 1) ? _coils.getCoil(address) : false; }
    void coil(uint16_t address, bool value) { if (_coils.contains(address, 1)) _coils.setCoil(address, value); }

    bool discreteInput(uint16_t address) const { return _discreteInputs.contains(address, 1) ? _discreteInputs.getInput(address) : false; }
    void discreteInput(uint16_t address, bool value) { if (_discreteInputs.contains(address, 1)) _discreteInputs.setInput(address, value); }

    uint16_t holdingRegister(uint16_t address) const { return _holdingRegs.contains(address, 1) ? _holdingRegs.getReg(address) : 0; }
    void holdingRegister(uint16_t address, uint16_t value) { if (_holdingRegs.contains(address, 1)) _holdingRegs.setReg(address, value); }

    uint16_t inputRegister(uint16_t address) const { return _inputRegs.contains(address, 1) ? _inputRegs.getReg(address) : 0; }
    void inputRegister(uint16_t address, uint16_t value) { if (_inputRegs.contains(address, 1)) _inputRegs.setReg(address, value); }

    // ========================================================================
    // Slave Mode: Poll (call from loop)
    // ========================================================================

    /**
     * @brief Process incoming Modbus requests (slave mode)
     * Call this regularly in the main loop.
     */
    void poll() {
        if (_slaveAddr == 0) return;  // Master mode, nothing to poll
        if (!_serial->available()) return;

        uint16_t rxLen = receiveFrame(_timeoutMs);
        if (rxLen > 0) {
            processSlaveRequest();
        }
    }

    // ========================================================================
    // Master Mode: Read Functions
    // ========================================================================

    /**
     * @brief FC 01 - Read Coils from a slave
     * @param slaveAddr Target slave address (1-247)
     * @param startAddress Starting coil address
     * @param quantity Number of coils to read (1-2000)
     * @param result Output buffer for coil states (packed bits, LSB first)
     * @return ModbusResult
     */
    ModbusResult readCoils(uint8_t slaveAddr, uint16_t startAddress, uint16_t quantity, uint8_t* result) {
        if (quantity == 0 || quantity > 2000 || !result) return MODBUS_ERR_INVALID_PARAMS;
        uint8_t pdu[5];
        pdu[0] = MODBUS_FC_READ_COILS;
        pdu[1] = (startAddress >> 8) & 0xFF;
        pdu[2] = startAddress & 0xFF;
        pdu[3] = (quantity >> 8) & 0xFF;
        pdu[4] = quantity & 0xFF;

        uint8_t resp[MODBUS_RTU_MAX_PDU];
        uint16_t respLen = 0;
        ModbusResult r = masterTransaction(slaveAddr, pdu, 5, resp, &respLen, sizeof(resp));
        if (r != MODBUS_OK) return r;
        if (respLen < 2) return MODBUS_ERR_FRAME;

        uint8_t byteCount = resp[1];
        if (respLen < (uint16_t)(2 + byteCount)) return MODBUS_ERR_FRAME;
        memcpy(result, &resp[2], byteCount);
        return MODBUS_OK;
    }

    /**
     * @brief FC 02 - Read Discrete Inputs from a slave
     * @param slaveAddr Target slave address (1-247)
     * @param startAddress Starting input address
     * @param quantity Number of inputs to read (1-2000)
     * @param result Output buffer for input states (packed bits, LSB first)
     * @return ModbusResult
     */
    ModbusResult readDiscreteInputs(uint8_t slaveAddr, uint16_t startAddress, uint16_t quantity, uint8_t* result) {
        if (quantity == 0 || quantity > 2000 || !result) return MODBUS_ERR_INVALID_PARAMS;
        uint8_t pdu[5];
        pdu[0] = MODBUS_FC_READ_DISCRETE_INPUTS;
        pdu[1] = (startAddress >> 8) & 0xFF;
        pdu[2] = startAddress & 0xFF;
        pdu[3] = (quantity >> 8) & 0xFF;
        pdu[4] = quantity & 0xFF;

        uint8_t resp[MODBUS_RTU_MAX_PDU];
        uint16_t respLen = 0;
        ModbusResult r = masterTransaction(slaveAddr, pdu, 5, resp, &respLen, sizeof(resp));
        if (r != MODBUS_OK) return r;
        if (respLen < 2) return MODBUS_ERR_FRAME;

        uint8_t byteCount = resp[1];
        if (respLen < (uint16_t)(2 + byteCount)) return MODBUS_ERR_FRAME;
        memcpy(result, &resp[2], byteCount);
        return MODBUS_OK;
    }

    /**
     * @brief FC 03 - Read Holding Registers from a slave
     * @param slaveAddr Target slave address (1-247)
     * @param startAddress Starting register address
     * @param quantity Number of registers to read (1-125)
     * @param result Output buffer for register values (host byte order)
     * @return ModbusResult
     */
    ModbusResult readHoldingRegisters(uint8_t slaveAddr, uint16_t startAddress, uint16_t quantity, uint16_t* result) {
        if (quantity == 0 || quantity > 125 || !result) return MODBUS_ERR_INVALID_PARAMS;
        uint8_t pdu[5];
        pdu[0] = MODBUS_FC_READ_HOLDING_REGISTERS;
        pdu[1] = (startAddress >> 8) & 0xFF;
        pdu[2] = startAddress & 0xFF;
        pdu[3] = (quantity >> 8) & 0xFF;
        pdu[4] = quantity & 0xFF;

        uint8_t resp[MODBUS_RTU_MAX_PDU];
        uint16_t respLen = 0;
        ModbusResult r = masterTransaction(slaveAddr, pdu, 5, resp, &respLen, sizeof(resp));
        if (r != MODBUS_OK) return r;
        if (respLen < 2) return MODBUS_ERR_FRAME;

        uint8_t byteCount = resp[1];
        if (byteCount != quantity * 2) return MODBUS_ERR_FRAME;
        if (respLen < (uint16_t)(2 + byteCount)) return MODBUS_ERR_FRAME;
        for (uint16_t i = 0; i < quantity; i++) {
            result[i] = ((uint16_t)resp[2 + i * 2] << 8) | resp[2 + i * 2 + 1];
        }
        return MODBUS_OK;
    }

    /**
     * @brief FC 04 - Read Input Registers from a slave
     * @param slaveAddr Target slave address (1-247)
     * @param startAddress Starting register address
     * @param quantity Number of registers to read (1-125)
     * @param result Output buffer for register values (host byte order)
     * @return ModbusResult
     */
    ModbusResult readInputRegisters(uint8_t slaveAddr, uint16_t startAddress, uint16_t quantity, uint16_t* result) {
        if (quantity == 0 || quantity > 125 || !result) return MODBUS_ERR_INVALID_PARAMS;
        uint8_t pdu[5];
        pdu[0] = MODBUS_FC_READ_INPUT_REGISTERS;
        pdu[1] = (startAddress >> 8) & 0xFF;
        pdu[2] = startAddress & 0xFF;
        pdu[3] = (quantity >> 8) & 0xFF;
        pdu[4] = quantity & 0xFF;

        uint8_t resp[MODBUS_RTU_MAX_PDU];
        uint16_t respLen = 0;
        ModbusResult r = masterTransaction(slaveAddr, pdu, 5, resp, &respLen, sizeof(resp));
        if (r != MODBUS_OK) return r;
        if (respLen < 2) return MODBUS_ERR_FRAME;

        uint8_t byteCount = resp[1];
        if (byteCount != quantity * 2) return MODBUS_ERR_FRAME;
        if (respLen < (uint16_t)(2 + byteCount)) return MODBUS_ERR_FRAME;
        for (uint16_t i = 0; i < quantity; i++) {
            result[i] = ((uint16_t)resp[2 + i * 2] << 8) | resp[2 + i * 2 + 1];
        }
        return MODBUS_OK;
    }

    // ========================================================================
    // Master Mode: Write Functions
    // ========================================================================

    /**
     * @brief FC 05 - Write Single Coil on a slave
     * @param slaveAddr Target slave address (1-247)
     * @param address Coil address
     * @param value true = ON, false = OFF
     * @return ModbusResult
     */
    ModbusResult writeSingleCoil(uint8_t slaveAddr, uint16_t address, bool value) {
        uint8_t pdu[5];
        pdu[0] = MODBUS_FC_WRITE_SINGLE_COIL;
        pdu[1] = (address >> 8) & 0xFF;
        pdu[2] = address & 0xFF;
        pdu[3] = value ? 0xFF : 0x00;
        pdu[4] = 0x00;

        uint8_t resp[MODBUS_RTU_MAX_PDU];
        uint16_t respLen = 0;
        return masterTransaction(slaveAddr, pdu, 5, resp, &respLen, sizeof(resp));
    }

    /**
     * @brief FC 06 - Write Single Register on a slave
     * @param slaveAddr Target slave address (1-247)
     * @param address Register address
     * @param value Register value
     * @return ModbusResult
     */
    ModbusResult writeSingleRegister(uint8_t slaveAddr, uint16_t address, uint16_t value) {
        uint8_t pdu[5];
        pdu[0] = MODBUS_FC_WRITE_SINGLE_REGISTER;
        pdu[1] = (address >> 8) & 0xFF;
        pdu[2] = address & 0xFF;
        pdu[3] = (value >> 8) & 0xFF;
        pdu[4] = value & 0xFF;

        uint8_t resp[MODBUS_RTU_MAX_PDU];
        uint16_t respLen = 0;
        return masterTransaction(slaveAddr, pdu, 5, resp, &respLen, sizeof(resp));
    }

    /**
     * @brief FC 0F - Write Multiple Coils on a slave
     * @param slaveAddr Target slave address (1-247)
     * @param startAddress Starting coil address
     * @param quantity Number of coils to write (1-1968)
     * @param values Packed coil values (LSB first)
     * @return ModbusResult
     */
    ModbusResult writeMultipleCoils(uint8_t slaveAddr, uint16_t startAddress, uint16_t quantity, const uint8_t* values) {
        if (quantity == 0 || quantity > 1968 || !values) return MODBUS_ERR_INVALID_PARAMS;
        uint8_t byteCount = (quantity + 7) / 8;
        if (6 + byteCount > MODBUS_RTU_MAX_PDU) return MODBUS_ERR_BUFFER_OVERFLOW;

        uint8_t pdu[MODBUS_RTU_MAX_PDU];
        pdu[0] = MODBUS_FC_WRITE_MULTIPLE_COILS;
        pdu[1] = (startAddress >> 8) & 0xFF;
        pdu[2] = startAddress & 0xFF;
        pdu[3] = (quantity >> 8) & 0xFF;
        pdu[4] = quantity & 0xFF;
        pdu[5] = byteCount;
        memcpy(&pdu[6], values, byteCount);

        uint8_t resp[MODBUS_RTU_MAX_PDU];
        uint16_t respLen = 0;
        return masterTransaction(slaveAddr, pdu, 6 + byteCount, resp, &respLen, sizeof(resp));
    }

    /**
     * @brief FC 10 - Write Multiple Registers on a slave
     * @param slaveAddr Target slave address (1-247)
     * @param startAddress Starting register address
     * @param quantity Number of registers to write (1-123)
     * @param values Register values array (host byte order)
     * @return ModbusResult
     */
    ModbusResult writeMultipleRegisters(uint8_t slaveAddr, uint16_t startAddress, uint16_t quantity, const uint16_t* values) {
        if (quantity == 0 || quantity > 123 || !values) return MODBUS_ERR_INVALID_PARAMS;
        uint8_t byteCount = quantity * 2;
        if (6 + byteCount > MODBUS_RTU_MAX_PDU) return MODBUS_ERR_BUFFER_OVERFLOW;

        uint8_t pdu[MODBUS_RTU_MAX_PDU];
        pdu[0] = MODBUS_FC_WRITE_MULTIPLE_REGISTERS;
        pdu[1] = (startAddress >> 8) & 0xFF;
        pdu[2] = startAddress & 0xFF;
        pdu[3] = (quantity >> 8) & 0xFF;
        pdu[4] = quantity & 0xFF;
        pdu[5] = byteCount;
        for (uint16_t i = 0; i < quantity; i++) {
            pdu[6 + i * 2]     = (values[i] >> 8) & 0xFF;
            pdu[6 + i * 2 + 1] = values[i] & 0xFF;
        }

        uint8_t resp[MODBUS_RTU_MAX_PDU];
        uint16_t respLen = 0;
        return masterTransaction(slaveAddr, pdu, 6 + byteCount, resp, &respLen, sizeof(resp));
    }

    // ========================================================================
    // Master Mode: Send Raw PDU (for custom function codes)
    // ========================================================================

    /**
     * @brief Send a raw Modbus PDU and receive the response
     * @param slaveAddr Target slave address (1-247)
     * @param requestPdu PDU to send (function code + data)
     * @param requestLen Length of request PDU
     * @param responsePdu Buffer for response PDU
     * @param responseLen Receives the length of the response PDU
     * @param maxResponseLen Maximum size of responsePdu buffer
     * @return ModbusResult
     */
    ModbusResult rawRequest(uint8_t slaveAddr, const uint8_t* requestPdu, uint16_t requestLen,
                            uint8_t* responsePdu, uint16_t* responseLen, uint16_t maxResponseLen) {
        return masterTransaction(slaveAddr, requestPdu, requestLen, responsePdu, responseLen, maxResponseLen);
    }

    // ========================================================================
    // Configuration & Status
    // ========================================================================

    void setTimeout(uint32_t ms) { _timeoutMs = ms; }
    uint32_t getTimeout() const { return _timeoutMs; }

    uint8_t slaveAddress() const { return _slaveAddr; }
    bool isMaster() const { return _slaveAddr == 0; }
    bool isSlave() const { return _slaveAddr > 0; }

    uint32_t rxCount() const { return _rxCount; }
    uint32_t txCount() const { return _txCount; }
    uint32_t errorCount() const { return _errCount; }
    ModbusResult lastError() const { return _lastError; }

    void resetCounters() { _rxCount = 0; _txCount = 0; _errCount = 0; }

    /**
     * @brief Register a handler for custom/unsupported function codes (slave mode)
     * @param handler Callback function
     */
    void onCustomFunction(CustomFunctionHandler handler) { _customHandler = handler; }

    /**
     * @brief Get the last exception code from the response buffer (after MODBUS_ERR_EXCEPTION)
     * @return Exception code, or 0 if no exception
     */
    uint8_t lastExceptionCode() const {
        // After a failed masterTransaction that returned MODBUS_ERR_EXCEPTION,
        // the exception code is in buf[2]
        if (_bufLen >= 3 && (_buf[1] & 0x80)) return _buf[2];
        return 0;
    }
};

#endif // PLCRUNTIME_MODBUS_RTU
