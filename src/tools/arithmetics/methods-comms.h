// methods-comms.h - 2025
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
// COMMS Bytecode Handler
// ============================================================================
// Implements the runtime handler for the COMMS opcode (0xF9).
// Reads the sub-function byte and dispatches to protocol-specific handlers.
//
// When PLCRUNTIME_COMMS_ENABLED is NOT defined, the handler skips past the
// instruction bytes (for WASM builds or builds without any protocol enabled).
// ============================================================================

namespace PLCMethods {

    // Skip a COMMS instruction without executing it (for WASM or disabled builds)
    static RuntimeError handle_COMMS_skip(u8* program, u32 prog_size, u32& index) {
        if (index >= prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 sub_fn = program[index++];
        u8 param_size = comms_subfn_param_size(sub_fn);
        if (index + param_size > prog_size) return PROGRAM_SIZE_EXCEEDED;
        index += param_size;
        return STATUS_SUCCESS;
    }

#ifdef PLCRUNTIME_COMMS_ENABLED

    // ========================================================================
    // Common sub-functions (0x00-0x07)
    // ========================================================================

    static RuntimeError handle_COMMS_BEGIN(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        if (index + 2 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        u8 config = program[index++];

        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (!ci || ci->protocol == COMMS_PROTO_NONE) return stack.push_bool(false);

        switch (ci->protocol) {
#ifdef PLCRUNTIME_MODBUS_RTU
            case COMMS_PROTO_MODBUS_RTU: {
                ModbusRTU* mb = (ModbusRTU*) ci->driver;
                if (!mb) return stack.push_bool(false);
                mb->begin(config); // config = slave address (0=master, 1-247=slave)
                ci->active = true;
                ci->lastError = 0;
                return stack.push_bool(true);
            }
#endif
            default:
                return stack.push_bool(false);
        }
    }

    static RuntimeError handle_COMMS_END(u8* program, u32 prog_size, u32& index) {
        if (index + 1 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];

        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (ci) ci->active = false;
        return STATUS_SUCCESS;
    }

    static RuntimeError handle_COMMS_ENABLED(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        if (index + 1 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        return stack.push_bool(g_plcComms.isActive(inst));
    }

    static RuntimeError handle_COMMS_STATUS(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        if (index + 1 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];

        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        return stack.push_u8(ci ? ci->lastError : 0xFF);
    }

    // ========================================================================
    // Modbus Data Area Configuration (0x08-0x0B)
    // ========================================================================

#ifdef PLCRUNTIME_MODBUS_RTU
    static RuntimeError handle_MB_ADD_DATA_AREA(u8 sub_fn, u8* program, u32 prog_size, u32& index) {
        if (index + 5 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        u16 start = read_u16(program + index); index += 2;
        u16 count = read_u16(program + index); index += 2;

        ModbusRTU* mb = g_plcComms.getModbusRTU(inst);
        if (!mb) return STATUS_SUCCESS; // Silently ignore if not registered

        switch ((PLCCommsSubFunction) sub_fn) {
            case MB_ADD_COILS:      mb->addCoils(start, count); break;
            case MB_ADD_DISCRETE:   mb->addDiscreteInputs(start, count); break;
            case MB_ADD_HOLDING:    mb->addHoldingRegisters(start, count); break;
            case MB_ADD_INPUT_REG:  mb->addInputRegisters(start, count); break;
            default: break;
        }
        return STATUS_SUCCESS;
    }
#endif // PLCRUNTIME_MODBUS_RTU

    // ========================================================================
    // Modbus Master Read Operations (0x10-0x13)
    // ========================================================================

#ifdef PLCRUNTIME_MODBUS_RTU
    static RuntimeError handle_MB_READ(RuntimeStack& stack, u8* memory, u8 sub_fn, u8* program, u32 prog_size, u32& index) {
        // Format: [inst:u8] [slave:u8] [start:u16] [qty:u16] [dest_mem:ptr]
        if (index + 6 + MY_PTR_SIZE_BYTES > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        u8 slave = program[index++];
        u16 start = read_u16(program + index); index += 2;
        u16 qty = read_u16(program + index); index += 2;
        MY_PTR_t dest_mem = read_ptr(program + index); index += MY_PTR_SIZE_BYTES;

        ModbusRTU* mb = g_plcComms.getModbusRTU(inst);
        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (!mb || !ci || !ci->active) {
            if (ci) ci->lastError = 0xFF;
            return stack.push_u8(0xFF);
        }

        ModbusResult result;
        switch ((PLCCommsSubFunction) sub_fn) {
            case MB_READ_COILS: {
                // Coils are packed bits - dest memory gets raw packed bytes
                u8 byte_count = (qty + 7) / 8;
                result = mb->readCoils(slave, start, qty, memory + dest_mem);
                break;
            }
            case MB_READ_DISCRETE: {
                u8 byte_count = (qty + 7) / 8;
                (void) byte_count;
                result = mb->readDiscreteInputs(slave, start, qty, memory + dest_mem);
                break;
            }
            case MB_READ_HOLDING: {
                // Registers are u16 - use temp buffer then write to memory
                u16 regs[125]; // Max Modbus read quantity
                if (qty > 125) qty = 125;
                result = mb->readHoldingRegisters(slave, start, qty, regs);
                if (result == MODBUS_OK) {
                    for (u16 r = 0; r < qty; r++) {
                        write_u16(memory + dest_mem + r * 2, regs[r]);
                    }
                }
                break;
            }
            case MB_READ_INPUT: {
                u16 regs[125];
                if (qty > 125) qty = 125;
                result = mb->readInputRegisters(slave, start, qty, regs);
                if (result == MODBUS_OK) {
                    for (u16 r = 0; r < qty; r++) {
                        write_u16(memory + dest_mem + r * 2, regs[r]);
                    }
                }
                break;
            }
            default:
                result = MODBUS_ERR_INVALID_PARAMS;
                break;
        }

        ci->lastError = (u8) result;
        return stack.push_u8((u8) result);
    }
#endif // PLCRUNTIME_MODBUS_RTU

    // ========================================================================
    // Modbus Master Write Operations (0x14-0x17)
    // ========================================================================

#ifdef PLCRUNTIME_MODBUS_RTU
    static RuntimeError handle_MB_WRITE_SINGLE(RuntimeStack& stack, u8* memory, u8 sub_fn, u8* program, u32 prog_size, u32& index) {
        // Format: [inst:u8] [slave:u8] [addr:u16] + pop value -> push u8 result
        if (index + 4 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        u8 slave = program[index++];
        u16 addr = read_u16(program + index); index += 2;

        ModbusRTU* mb = g_plcComms.getModbusRTU(inst);
        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (!mb || !ci || !ci->active) {
            // Still need to pop the value from stack
            if (sub_fn == MB_WRITE_COIL) { if (stack.size() > 0) stack.pop(); }
            else { if (stack.size() >= 2) { stack.pop(); stack.pop(); } }
            if (ci) ci->lastError = 0xFF;
            return stack.push_u8(0xFF);
        }

        ModbusResult result;
        if (sub_fn == MB_WRITE_COIL) {
            bool val = stack.pop() != 0;
            result = mb->writeSingleCoil(slave, addr, val);
        } else { // MB_WRITE_REG
            u8 hi = stack.pop();
            u8 lo = stack.pop();
            u16 val = (u16)lo | ((u16)hi << 8);
            result = mb->writeSingleRegister(slave, addr, val);
        }

        ci->lastError = (u8) result;
        return stack.push_u8((u8) result);
    }

    static RuntimeError handle_MB_WRITE_MULTIPLE(RuntimeStack& stack, u8* memory, u8 sub_fn, u8* program, u32 prog_size, u32& index) {
        // Format: [inst:u8] [slave:u8] [start:u16] [qty:u16] [src_mem:ptr] -> push u8 result
        if (index + 6 + MY_PTR_SIZE_BYTES > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        u8 slave = program[index++];
        u16 start = read_u16(program + index); index += 2;
        u16 qty = read_u16(program + index); index += 2;
        MY_PTR_t src_mem = read_ptr(program + index); index += MY_PTR_SIZE_BYTES;

        ModbusRTU* mb = g_plcComms.getModbusRTU(inst);
        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (!mb || !ci || !ci->active) {
            if (ci) ci->lastError = 0xFF;
            return stack.push_u8(0xFF);
        }

        ModbusResult result;
        if (sub_fn == MB_WRITE_COILS) {
            // Coils are packed bits in memory
            result = mb->writeMultipleCoils(slave, start, qty, memory + src_mem);
        } else { // MB_WRITE_REGS
            // Registers: read u16 values from PLC memory
            u16 regs[123]; // Max Modbus write quantity
            if (qty > 123) qty = 123;
            for (u16 r = 0; r < qty; r++) {
                regs[r] = read_u16(memory + src_mem + r * 2);
            }
            result = mb->writeMultipleRegisters(slave, start, qty, regs);
        }

        ci->lastError = (u8) result;
        return stack.push_u8((u8) result);
    }
#endif // PLCRUNTIME_MODBUS_RTU

    // ========================================================================
    // Modbus Slave Operations (0x18-0x20)
    // ========================================================================

#ifdef PLCRUNTIME_MODBUS_RTU
    static RuntimeError handle_MB_POLL(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        if (index + 1 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];

        ModbusRTU* mb = g_plcComms.getModbusRTU(inst);
        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (!mb || !ci || !ci->active) return stack.push_bool(false);

        mb->poll();
        return stack.push_bool(true);
    }

    static RuntimeError handle_MB_SLV_ACCESS(RuntimeStack& stack, u8 sub_fn, u8* program, u32 prog_size, u32& index) {
        // Format: [inst:u8] [addr:u16] -> push/pop value
        if (index + 3 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        u16 addr = read_u16(program + index); index += 2;

        ModbusRTU* mb = g_plcComms.getModbusRTU(inst);
        if (!mb) {
            // Need to handle stack balance for SET operations (pop value)
            switch ((PLCCommsSubFunction) sub_fn) {
                case MB_SLV_SET_COIL:
                case MB_SLV_SET_DI:
                    if (stack.size() > 0) stack.pop();
                    break;
                case MB_SLV_SET_REG:
                case MB_SLV_SET_IR:
                    if (stack.size() >= 2) { stack.pop(); stack.pop(); }
                    break;
                default: break;
            }
            // GET operations push default values
            switch ((PLCCommsSubFunction) sub_fn) {
                case MB_SLV_GET_COIL:
                case MB_SLV_GET_DI:
                    return stack.push_bool(false);
                case MB_SLV_GET_REG:
                case MB_SLV_GET_IR:
                    return stack.push_u16(0);
                default: return STATUS_SUCCESS;
            }
        }

        switch ((PLCCommsSubFunction) sub_fn) {
            case MB_SLV_GET_COIL:   return stack.push_bool(mb->coil(addr));
            case MB_SLV_SET_COIL:   { bool v = stack.pop() != 0; mb->coil(addr, v); return STATUS_SUCCESS; }
            case MB_SLV_GET_REG:    return stack.push_u16(mb->holdingRegister(addr));
            case MB_SLV_SET_REG:    { u8 hi = stack.pop(); u8 lo = stack.pop(); mb->holdingRegister(addr, (u16)lo | ((u16)hi << 8)); return STATUS_SUCCESS; }
            case MB_SLV_GET_DI:     return stack.push_bool(mb->discreteInput(addr));
            case MB_SLV_SET_DI:     { bool v = stack.pop() != 0; mb->discreteInput(addr, v); return STATUS_SUCCESS; }
            case MB_SLV_GET_IR:     return stack.push_u16(mb->inputRegister(addr));
            case MB_SLV_SET_IR:     { u8 hi = stack.pop(); u8 lo = stack.pop(); mb->inputRegister(addr, (u16)lo | ((u16)hi << 8)); return STATUS_SUCCESS; }
            default: return STATUS_SUCCESS;
        }
    }
#endif // PLCRUNTIME_MODBUS_RTU

    // ========================================================================
    // Main COMMS Handler - dispatches to sub-function handlers
    // ========================================================================

    static RuntimeError handle_COMMS(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        if (index >= prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 sub_fn = program[index++];

        switch ((PLCCommsSubFunction) sub_fn) {
            // Common
            case COMMS_BEGIN:    return handle_COMMS_BEGIN(stack, program, prog_size, index);
            case COMMS_END:      return handle_COMMS_END(program, prog_size, index);
            case COMMS_ENABLED:  return handle_COMMS_ENABLED(stack, program, prog_size, index);
            case COMMS_STATUS:   return handle_COMMS_STATUS(stack, program, prog_size, index);

#ifdef PLCRUNTIME_MODBUS_RTU
            // Modbus data area config
            case MB_ADD_COILS:
            case MB_ADD_DISCRETE:
            case MB_ADD_HOLDING:
            case MB_ADD_INPUT_REG:
                return handle_MB_ADD_DATA_AREA(sub_fn, program, prog_size, index);

            // Modbus master read
            case MB_READ_COILS:
            case MB_READ_DISCRETE:
            case MB_READ_HOLDING:
            case MB_READ_INPUT:
                return handle_MB_READ(stack, memory, sub_fn, program, prog_size, index);

            // Modbus master write single
            case MB_WRITE_COIL:
            case MB_WRITE_REG:
                return handle_MB_WRITE_SINGLE(stack, memory, sub_fn, program, prog_size, index);

            // Modbus master write multiple
            case MB_WRITE_COILS:
            case MB_WRITE_REGS:
                return handle_MB_WRITE_MULTIPLE(stack, memory, sub_fn, program, prog_size, index);

            // Modbus slave
            case MB_POLL:
                return handle_MB_POLL(stack, program, prog_size, index);
            case MB_SLV_GET_COIL:
            case MB_SLV_SET_COIL:
            case MB_SLV_GET_REG:
            case MB_SLV_SET_REG:
            case MB_SLV_GET_DI:
            case MB_SLV_SET_DI:
            case MB_SLV_GET_IR:
            case MB_SLV_SET_IR:
                return handle_MB_SLV_ACCESS(stack, sub_fn, program, prog_size, index);
#endif // PLCRUNTIME_MODBUS_RTU

            // Raw TCP - stub for future implementation
            case TCP_CONNECT:
            case TCP_DISCONNECT:
            case TCP_CONNECTED:
            case TCP_LISTEN:
            case TCP_ACCEPT:
            case TCP_SEND:
            case TCP_RECV:
            case TCP_AVAILABLE:
            // Raw UDP - stub for future implementation
            case UDP_OPEN:
            case UDP_CLOSE:
            case UDP_SEND:
            case UDP_RECV:
            case UDP_AVAILABLE: {
                // Skip parameters for unimplemented protocols
                u8 param_size = comms_subfn_param_size(sub_fn);
                if (index + param_size > prog_size) return PROGRAM_SIZE_EXCEEDED;
                index += param_size;
                return STATUS_SUCCESS;
            }

            default: {
                // Unknown sub-function - try to skip based on known size
                u8 param_size = comms_subfn_param_size(sub_fn);
                if (param_size > 0 && index + param_size <= prog_size) {
                    index += param_size;
                    return STATUS_SUCCESS;
                }
                return UNKNOWN_INSTRUCTION;
            }
        }
    }

#endif // PLCRUNTIME_COMMS_ENABLED

} // namespace PLCMethods
