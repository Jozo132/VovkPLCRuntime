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

    // Skip a COMMS instruction without executing it, pushing default 0/false
    // for sub-functions that normally push a result. Prevents stack underflow.
    static RuntimeError handle_COMMS_skip(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        if (index >= prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 sub_fn = program[index++];
        u8 param_size = comms_subfn_param_size(sub_fn);
        if (index + param_size > prog_size) return PROGRAM_SIZE_EXCEEDED;
        index += param_size;
        u8 rtype = comms_subfn_result_type(sub_fn);
        switch (rtype) {
            case 1: return stack.push_bool(false);
            case 2: return stack.push_u8(0);
            case 3: return stack.push_u16(0);
            default: return STATUS_SUCCESS;
        }
    }

#ifdef __WASM__
    // ========================================================================
    // WASM COMMS Handler - delegates to JavaScript via import
    // ========================================================================
    // When running in WASM, COMMS instructions are forwarded to JavaScript
    // via the js_comms_invoke import. If the JS net bridge is enabled, it
    // handles the operation and returns a result type code + value.
    // If disabled, returns 0 (skip, no stack push).
    //
    // Result buffer layout (16 bytes in WASM linear memory):
    //   [0..1]  u16 result value (for push_u16 or push_bool/push_u8)
    //   [2]     reserved
    //   [3]     reserved
    //
    // Return value from js_comms_invoke:
    //   0    = not handled (skip, no stack push)
    //   1    = push bool from result_buf[0]
    //   2    = push u8 from result_buf[0]
    //   3    = push u16 from result_buf[0..1]
    //   0xFF = error
    // ========================================================================

    static u8 js_comms_result_buf[16] __attribute__((aligned(4))) = {0};

    extern "C" {
        __attribute__((import_module("env"), import_name("js_comms_invoke")))
        u32 js_comms_invoke(u8 sub_fn, u32 params_ptr, u8 param_size, u32 memory_ptr, u32 result_buf_ptr);
    }

    static RuntimeError handle_COMMS_wasm(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        if (index >= prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 sub_fn = program[index++];
        u8 param_size = comms_subfn_param_size(sub_fn);
        if (index + param_size > prog_size) return PROGRAM_SIZE_EXCEEDED;

        // Clear result buffer
        js_comms_result_buf[0] = 0;
        js_comms_result_buf[1] = 0;

        // Call JS handler with pointers into WASM linear memory
        u32 status = js_comms_invoke(
            sub_fn,
            (u32)(uintptr_t)&program[index],
            param_size,
            (u32)(uintptr_t)memory,
            (u32)(uintptr_t)js_comms_result_buf
        );
        index += param_size;

        if (status == 0) {
            // Bridge not connected - push default result to keep stack balanced
            u8 rtype = comms_subfn_result_type(sub_fn);
            switch (rtype) {
                case 1: return stack.push_bool(false);
                case 2: return stack.push_u8(0);
                case 3: return stack.push_u16(0);
                default: return STATUS_SUCCESS;
            }
        }
        if (status == 0xFF) return EXECUTION_ERROR;

        // Push result to stack based on type code
        switch (status) {
            case 1: return stack.push_bool(js_comms_result_buf[0] != 0);
            case 2: return stack.push_u8(js_comms_result_buf[0]);
            case 3: return stack.push_u16((u16)js_comms_result_buf[0] | ((u16)js_comms_result_buf[1] << 8));
            default: return STATUS_SUCCESS;
        }
    }
#endif // __WASM__

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
#ifdef PLCRUNTIME_SERIAL_RS232
            case COMMS_PROTO_SERIAL: {
                SerialRS232* ser = (SerialRS232*) ci->driver;
                if (!ser) return stack.push_bool(false);
                // config byte selects baud rate from lookup table
                // 0 = use stored baudrate, 1-11 = standard rates
                uint32_t baud = serialBaudFromIndex(config);
                if (baud > 0) ser->setBaudrate(baud);
                if (!ser->isBegun()) ser->begin();
                ci->active = true;
                ci->lastError = 0;
                return stack.push_bool(true);
            }
#endif
#ifdef PLCRUNTIME_ETHERNET_W5500
            case COMMS_PROTO_RAW_TCP:
            case COMMS_PROTO_RAW_UDP: {
                EthernetW5500* eth = (EthernetW5500*) ci->driver;
                if (!eth) return stack.push_bool(false);
                if (!eth->isBegun()) eth->begin();
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
        if (ci) {
            ci->active = false;
#ifdef PLCRUNTIME_SERIAL_RS232
            if (ci->protocol == COMMS_PROTO_SERIAL) {
                SerialRS232* ser = (SerialRS232*) ci->driver;
                if (ser) ser->end();
            }
#endif
#ifdef PLCRUNTIME_ETHERNET_W5500
            if (ci->protocol == COMMS_PROTO_RAW_TCP || ci->protocol == COMMS_PROTO_RAW_UDP) {
                EthernetW5500* eth = (EthernetW5500*) ci->driver;
                if (eth) eth->end();
            }
#endif
        }
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
    // Priority / Async Queue (0x04-0x06)
    // ========================================================================

    static RuntimeError handle_COMMS_SET_PRIORITY(u8* program, u32 prog_size, u32& index) {
        if (index + 2 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        u8 prio = program[index++];

        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (ci) ci->priority = prio;
        return STATUS_SUCCESS;
    }

    static RuntimeError handle_COMMS_POLL_ASYNC(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        if (index + 1 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];

        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (!ci || !ci->active) return stack.push_u8(0xFF);

        PLCCommsTxn* txn = ci->asyncQueue.peek();
        if (!txn) return stack.push_u8(0); // idle - nothing queued

        // Execute the transaction synchronously now
        // We re-use the existing handlers by constructing a mini program from txn->params
        // For simplicity, we execute the transaction inline using the stored sub_fn + params
        // The result is stored in txn->resultError, then dequeued
        // Note: async poll executes at most one transaction per call
        u8 mini_prog[20]; // sub_fn params fit in 16 + some overhead
        u32 mini_size = txn->paramLen;
        for (u8 j = 0; j < txn->paramLen; j++) mini_prog[j] = txn->params[j];
        u32 mini_index = 0;

        // Dispatch the stored sub_fn through the main handler
        // We construct: [sub_fn] [params...] and call handle_COMMS
        u8 exec_prog[20];
        exec_prog[0] = txn->sub_fn;
        for (u8 j = 0; j < txn->paramLen; j++) exec_prog[1 + j] = txn->params[j];
        u32 exec_size = 1 + txn->paramLen;
        u32 exec_index = 0;

        RuntimeError err = handle_COMMS(stack, memory, exec_prog, exec_size, exec_index);
        txn->resultError = (err == STATUS_SUCCESS) ? ci->lastError : 0xFF;
        ci->asyncQueue.dequeue();

        // Return busy if more transactions remain, idle if done
        return stack.push_u8(ci->asyncQueue.count > 0 ? 1 : 0);
    }

    static RuntimeError handle_COMMS_QUEUE_SIZE(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        if (index + 1 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];

        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        return stack.push_u8(ci ? ci->asyncQueue.count : 0);
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
    // Serial RS232 Operations (0x50-0x59)
    // ========================================================================

#ifdef PLCRUNTIME_SERIAL_RS232
    static RuntimeError handle_SER_WRITE(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] [src_mem:ptr] [len:u16] -> push u16 (bytes written)
        if (index + 3 + MY_PTR_SIZE_BYTES > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        MY_PTR_t src_mem = read_ptr(program + index); index += MY_PTR_SIZE_BYTES;
        u16 len = read_u16(program + index); index += 2;

        SerialRS232* ser = g_plcComms.getSerial(inst);
        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (!ser || !ci || !ci->active) {
            if (ci) ci->lastError = 0xFF;
            return stack.push_u16(0);
        }
        u16 written = ser->write(memory + src_mem, len);
        ci->lastError = 0;
        return stack.push_u16(written);
    }

    static RuntimeError handle_SER_READ(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] [dest_mem:ptr] [max:u16] -> push u16 (bytes read)
        if (index + 3 + MY_PTR_SIZE_BYTES > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        MY_PTR_t dest_mem = read_ptr(program + index); index += MY_PTR_SIZE_BYTES;
        u16 max_len = read_u16(program + index); index += 2;

        SerialRS232* ser = g_plcComms.getSerial(inst);
        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (!ser || !ci || !ci->active) {
            if (ci) ci->lastError = 0xFF;
            return stack.push_u16(0);
        }
        u16 count = ser->read(memory + dest_mem, max_len);
        ci->lastError = 0;
        return stack.push_u16(count);
    }

    static RuntimeError handle_SER_AVAILABLE(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] -> push u16
        if (index + 1 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];

        SerialRS232* ser = g_plcComms.getSerial(inst);
        if (!ser) return stack.push_u16(0);
        return stack.push_u16(ser->available());
    }

    static RuntimeError handle_SER_FLUSH(u8* program, u32 prog_size, u32& index) {
        // [inst:u8]
        if (index + 1 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];

        SerialRS232* ser = g_plcComms.getSerial(inst);
        if (ser) ser->flush();
        return STATUS_SUCCESS;
    }

    static RuntimeError handle_SER_WRITE_BYTE(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] + pop u8 -> push bool
        if (index + 1 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];

        u8 byte_val = stack.pop();
        SerialRS232* ser = g_plcComms.getSerial(inst);
        if (!ser) return stack.push_bool(false);
        return stack.push_bool(ser->writeByte(byte_val));
    }

    static RuntimeError handle_SER_READ_BYTE(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] -> push i16 (-1 if none)
        if (index + 1 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];

        SerialRS232* ser = g_plcComms.getSerial(inst);
        if (!ser) return stack.push_i16(-1);
        return stack.push_i16(ser->readByte());
    }

    static RuntimeError handle_SER_POLL(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] -> push u16 (bytes available after poll)
        if (index + 1 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];

        SerialRS232* ser = g_plcComms.getSerial(inst);
        if (!ser) return stack.push_u16(0);
        ser->poll();
        return stack.push_u16(ser->available());
    }

    static RuntimeError handle_SER_MSG_READY(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] -> push bool
        if (index + 1 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];

        SerialRS232* ser = g_plcComms.getSerial(inst);
        if (!ser) return stack.push_bool(false);
        return stack.push_bool(ser->messageReady());
    }

    static RuntimeError handle_SER_READ_MSG(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] [dest_mem:ptr] [max:u16] -> push u16 (msg length)
        if (index + 3 + MY_PTR_SIZE_BYTES > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        MY_PTR_t dest_mem = read_ptr(program + index); index += MY_PTR_SIZE_BYTES;
        u16 max_len = read_u16(program + index); index += 2;

        SerialRS232* ser = g_plcComms.getSerial(inst);
        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (!ser || !ci || !ci->active) {
            if (ci) ci->lastError = 0xFF;
            return stack.push_u16(0);
        }
        u16 len = ser->readMessage(memory + dest_mem, max_len);
        ci->lastError = 0;
        return stack.push_u16(len);
    }

    static RuntimeError handle_SER_SET_DELIM(u8* program, u32 prog_size, u32& index) {
        // [inst:u8] [delim:u8]
        if (index + 2 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        u8 delim = program[index++];

        SerialRS232* ser = g_plcComms.getSerial(inst);
        if (ser) ser->setDelimiter(delim);
        return STATUS_SUCCESS;
    }

    static RuntimeError handle_SER_SET_BAUD(u8* program, u32 prog_size, u32& index) {
        // [inst:u8] [baud:u32] -> auto re-init if active
        if (index + 5 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        u32 baud = read_u32(program + index); index += 4;

        SerialRS232* ser = g_plcComms.getSerial(inst);
        if (ser) ser->setBaudrate(baud);
        return STATUS_SUCCESS;
    }
#endif // PLCRUNTIME_SERIAL_RS232

    // ========================================================================
    // Ethernet W5500 - TCP Operations (0x30-0x37)
    // ========================================================================

#ifdef PLCRUNTIME_ETHERNET_W5500
    static RuntimeError handle_TCP_CONNECT(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] [ip0:u8] [ip1:u8] [ip2:u8] [ip3:u8] [port:u16] -> push bool
        if (index + 7 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        u8 ip0 = program[index++];
        u8 ip1 = program[index++];
        u8 ip2 = program[index++];
        u8 ip3 = program[index++];
        u16 port = read_u16(program + index); index += 2;

        EthernetW5500* eth = g_plcComms.getEthernet(inst);
        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (!eth || !ci || !ci->active) {
            if (ci) ci->lastError = ETH_ERR_NOT_READY;
            return stack.push_bool(false);
        }
        EthernetW5500Result res = eth->tcpConnect(IPAddress(ip0, ip1, ip2, ip3), port);
        ci->lastError = (u8) res;
        return stack.push_bool(res == ETH_OK);
    }

    static RuntimeError handle_TCP_DISCONNECT(u8* program, u32 prog_size, u32& index) {
        // [inst:u8]
        if (index + 1 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];

        EthernetW5500* eth = g_plcComms.getEthernet(inst);
        if (eth) eth->tcpDisconnect();
        return STATUS_SUCCESS;
    }

    static RuntimeError handle_TCP_CONNECTED(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] -> push bool
        if (index + 1 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];

        EthernetW5500* eth = g_plcComms.getEthernet(inst);
        if (!eth) return stack.push_bool(false);
        return stack.push_bool(eth->tcpConnected());
    }

    static RuntimeError handle_TCP_LISTEN(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] [port:u16] -> push bool
        if (index + 3 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        u16 port = read_u16(program + index); index += 2;

        EthernetW5500* eth = g_plcComms.getEthernet(inst);
        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (!eth || !ci || !ci->active) {
            if (ci) ci->lastError = ETH_ERR_NOT_READY;
            return stack.push_bool(false);
        }
        EthernetW5500Result res = eth->tcpListen(port);
        ci->lastError = (u8) res;
        return stack.push_bool(res == ETH_OK);
    }

    static RuntimeError handle_TCP_ACCEPT(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] -> push bool
        if (index + 1 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];

        EthernetW5500* eth = g_plcComms.getEthernet(inst);
        if (!eth) return stack.push_bool(false);
        return stack.push_bool(eth->tcpAccept());
    }

    static RuntimeError handle_TCP_SEND(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] [src_mem:ptr] [len:u16] -> push u16
        if (index + 3 + MY_PTR_SIZE_BYTES > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        MY_PTR_t src_mem = read_ptr(program + index); index += MY_PTR_SIZE_BYTES;
        u16 len = read_u16(program + index); index += 2;

        EthernetW5500* eth = g_plcComms.getEthernet(inst);
        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (!eth || !ci || !ci->active) {
            if (ci) ci->lastError = ETH_ERR_NOT_READY;
            return stack.push_u16(0);
        }
        u16 sent = eth->tcpSend(memory + src_mem, len);
        ci->lastError = (sent > 0) ? ETH_OK : ETH_ERR_SEND_FAILED;
        return stack.push_u16(sent);
    }

    static RuntimeError handle_TCP_RECV(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] [dest_mem:ptr] [max:u16] -> push u16
        if (index + 3 + MY_PTR_SIZE_BYTES > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        MY_PTR_t dest_mem = read_ptr(program + index); index += MY_PTR_SIZE_BYTES;
        u16 max_len = read_u16(program + index); index += 2;

        EthernetW5500* eth = g_plcComms.getEthernet(inst);
        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (!eth || !ci || !ci->active) {
            if (ci) ci->lastError = ETH_ERR_NOT_READY;
            return stack.push_u16(0);
        }
        u16 count = eth->tcpRecv(memory + dest_mem, max_len);
        ci->lastError = ETH_OK;
        return stack.push_u16(count);
    }

    static RuntimeError handle_TCP_AVAILABLE(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] -> push u16
        if (index + 1 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];

        EthernetW5500* eth = g_plcComms.getEthernet(inst);
        if (!eth) return stack.push_u16(0);
        return stack.push_u16(eth->tcpAvailable());
    }

    // ========================================================================
    // Ethernet W5500 - UDP Operations (0x40-0x44)
    // ========================================================================

    static RuntimeError handle_UDP_OPEN(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] [port:u16] -> push bool
        if (index + 3 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        u16 port = read_u16(program + index); index += 2;

        EthernetW5500* eth = g_plcComms.getEthernet(inst);
        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (!eth || !ci || !ci->active) {
            if (ci) ci->lastError = ETH_ERR_NOT_READY;
            return stack.push_bool(false);
        }
        EthernetW5500Result res = eth->udpOpen(port);
        ci->lastError = (u8) res;
        return stack.push_bool(res == ETH_OK);
    }

    static RuntimeError handle_UDP_CLOSE(u8* program, u32 prog_size, u32& index) {
        // [inst:u8]
        if (index + 1 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];

        EthernetW5500* eth = g_plcComms.getEthernet(inst);
        if (eth) eth->udpClose();
        return STATUS_SUCCESS;
    }

    static RuntimeError handle_UDP_SEND(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] [ip0-3:u8x4] [port:u16] [src_mem:ptr] [len:u16] -> push u16
        if (index + 9 + MY_PTR_SIZE_BYTES > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        u8 ip0 = program[index++];
        u8 ip1 = program[index++];
        u8 ip2 = program[index++];
        u8 ip3 = program[index++];
        u16 port = read_u16(program + index); index += 2;
        MY_PTR_t src_mem = read_ptr(program + index); index += MY_PTR_SIZE_BYTES;
        u16 len = read_u16(program + index); index += 2;

        EthernetW5500* eth = g_plcComms.getEthernet(inst);
        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (!eth || !ci || !ci->active) {
            if (ci) ci->lastError = ETH_ERR_NOT_READY;
            return stack.push_u16(0);
        }
        u16 sent = eth->udpSend(IPAddress(ip0, ip1, ip2, ip3), port, memory + src_mem, len);
        ci->lastError = (sent > 0) ? ETH_OK : ETH_ERR_UDP_SEND_FAILED;
        return stack.push_u16(sent);
    }

    static RuntimeError handle_UDP_RECV(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] [dest_mem:ptr] [max:u16] -> push u16
        if (index + 3 + MY_PTR_SIZE_BYTES > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        MY_PTR_t dest_mem = read_ptr(program + index); index += MY_PTR_SIZE_BYTES;
        u16 max_len = read_u16(program + index); index += 2;

        EthernetW5500* eth = g_plcComms.getEthernet(inst);
        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (!eth || !ci || !ci->active) {
            if (ci) ci->lastError = ETH_ERR_NOT_READY;
            return stack.push_u16(0);
        }
        u16 count = eth->udpRecv(memory + dest_mem, max_len);
        ci->lastError = ETH_OK;
        return stack.push_u16(count);
    }

    static RuntimeError handle_UDP_AVAILABLE(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] -> push u16
        if (index + 1 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];

        EthernetW5500* eth = g_plcComms.getEthernet(inst);
        if (!eth) return stack.push_u16(0);
        return stack.push_u16(eth->udpAvailable());
    }

    // ========================================================================
    // Ethernet W5500 - Socket Reservation Operations (0x60-0x64)
    // ========================================================================

    static RuntimeError handle_ETH_SOCK_ACQUIRE(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] [pool:u8] -> push u8 (slot index, 0xFF=none)
        // pool: 0=RT_TCP, 1=BG_TCP, 2=RT_UDP, 3=BG_UDP
        if (index + 2 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        u8 pool = program[index++];

        EthernetW5500* eth = g_plcComms.getEthernet(inst);
        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (!eth || !ci || !ci->active) {
            if (ci) ci->lastError = ETH_ERR_NOT_READY;
            return stack.push_u8(0xFF);
        }

        uint8_t slot = 0xFF;
        switch (pool) {
            case 0: slot = eth->acquireTcpSlot(true);  break; // RT TCP
            case 1: slot = eth->acquireTcpSlot(false); break; // BG TCP
            case 2: slot = eth->acquireUdpSlot(true);  break; // RT UDP
            case 3: slot = eth->acquireUdpSlot(false); break; // BG UDP
            default: break;
        }

        ci->lastError = (slot != 0xFF) ? ETH_OK : ETH_ERR_NO_SLOT;
        return stack.push_u8(slot);
    }

    static RuntimeError handle_ETH_SOCK_RELEASE(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] [slot:u8] [type:u8] -> push u8 (result)
        // type: 0=TCP, 1=UDP
        if (index + 3 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        u8 slot = program[index++];
        u8 type = program[index++];

        EthernetW5500* eth = g_plcComms.getEthernet(inst);
        PLCCommsInstance* ci = g_plcComms.getInstance(inst);
        if (!eth || !ci || !ci->active) {
            if (ci) ci->lastError = ETH_ERR_NOT_READY;
            return stack.push_u8(ETH_ERR_NOT_READY);
        }

        EthernetW5500Result res;
        if (type == 0) {
            res = eth->releaseTcpSlot(slot);
        } else {
            res = eth->releaseUdpSlot(slot);
        }
        ci->lastError = (u8) res;
        return stack.push_u8((u8) res);
    }

    static RuntimeError handle_ETH_SOCK_STATUS(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] [slot:u8] [type:u8] -> push u8 (EthSocketState)
        if (index + 3 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        u8 slot = program[index++];
        u8 type = program[index++];

        EthernetW5500* eth = g_plcComms.getEthernet(inst);
        if (!eth) return stack.push_u8(ETH_SOCK_FREE);

        if (type == 0) {
            return stack.push_u8((u8) eth->tcpSlotState(slot));
        } else {
            return stack.push_u8((u8) eth->udpSlotState(slot));
        }
    }

    static RuntimeError handle_ETH_SOCK_SET_ACTIVE(u8* program, u32 prog_size, u32& index) {
        // [inst:u8] [slot:u8] [type:u8]
        // type: 0=TCP, 1=UDP
        if (index + 3 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];
        u8 slot = program[index++];
        u8 type = program[index++];

        EthernetW5500* eth = g_plcComms.getEthernet(inst);
        if (!eth) return STATUS_SUCCESS;

        if (type == 0) {
            eth->setActiveTcpSlot(slot);
        } else {
            eth->setActiveUdpSlot(slot);
        }
        return STATUS_SUCCESS;
    }

    static RuntimeError handle_ETH_SOCK_INFO(RuntimeStack& stack, u8* program, u32 prog_size, u32& index) {
        // [inst:u8] -> push u8 (packed: hi nibble = tcp counts, lo nibble = udp counts)
        // Bits: [rt_tcp:2][bg_tcp:2][rt_udp:2][bg_udp:2]
        if (index + 1 > prog_size) return PROGRAM_SIZE_EXCEEDED;
        u8 inst = program[index++];

        EthernetW5500* eth = g_plcComms.getEthernet(inst);
        if (!eth) return stack.push_u8(0);

        u8 packed = ((eth->rtTcpSlots() & 0x03) << 6)
                  | ((eth->bgTcpSlots() & 0x03) << 4)
                  | ((eth->rtUdpSlots() & 0x03) << 2)
                  | ((eth->bgUdpSlots() & 0x03));
        return stack.push_u8(packed);
    }
#endif // PLCRUNTIME_ETHERNET_W5500

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

            // Priority / Async
            case COMMS_SET_PRIORITY: return handle_COMMS_SET_PRIORITY(program, prog_size, index);
            case COMMS_POLL_ASYNC:   return handle_COMMS_POLL_ASYNC(stack, memory, program, prog_size, index);
            case COMMS_QUEUE_SIZE:   return handle_COMMS_QUEUE_SIZE(stack, program, prog_size, index);

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

            // Raw TCP
#ifdef PLCRUNTIME_ETHERNET_W5500
            case TCP_CONNECT:    return handle_TCP_CONNECT(stack, program, prog_size, index);
            case TCP_DISCONNECT: return handle_TCP_DISCONNECT(program, prog_size, index);
            case TCP_CONNECTED:  return handle_TCP_CONNECTED(stack, program, prog_size, index);
            case TCP_LISTEN:     return handle_TCP_LISTEN(stack, program, prog_size, index);
            case TCP_ACCEPT:     return handle_TCP_ACCEPT(stack, program, prog_size, index);
            case TCP_SEND:       return handle_TCP_SEND(stack, memory, program, prog_size, index);
            case TCP_RECV:       return handle_TCP_RECV(stack, memory, program, prog_size, index);
            case TCP_AVAILABLE:  return handle_TCP_AVAILABLE(stack, program, prog_size, index);

            // Raw UDP
            case UDP_OPEN:       return handle_UDP_OPEN(stack, program, prog_size, index);
            case UDP_CLOSE:      return handle_UDP_CLOSE(program, prog_size, index);
            case UDP_SEND:       return handle_UDP_SEND(stack, memory, program, prog_size, index);
            case UDP_RECV:       return handle_UDP_RECV(stack, memory, program, prog_size, index);
            case UDP_AVAILABLE:  return handle_UDP_AVAILABLE(stack, program, prog_size, index);

            // Socket Reservation
            case ETH_SOCK_ACQUIRE:    return handle_ETH_SOCK_ACQUIRE(stack, program, prog_size, index);
            case ETH_SOCK_RELEASE:    return handle_ETH_SOCK_RELEASE(stack, program, prog_size, index);
            case ETH_SOCK_STATUS:     return handle_ETH_SOCK_STATUS(stack, program, prog_size, index);
            case ETH_SOCK_SET_ACTIVE: return handle_ETH_SOCK_SET_ACTIVE(program, prog_size, index);
            case ETH_SOCK_INFO:       return handle_ETH_SOCK_INFO(stack, program, prog_size, index);
#else
            // Raw TCP - stub (skip parameters when W5500 not enabled)
            case TCP_CONNECT:
            case TCP_DISCONNECT:
            case TCP_CONNECTED:
            case TCP_LISTEN:
            case TCP_ACCEPT:
            case TCP_SEND:
            case TCP_RECV:
            case TCP_AVAILABLE:
            // Raw UDP - stub
            case UDP_OPEN:
            case UDP_CLOSE:
            case UDP_SEND:
            case UDP_RECV:
            case UDP_AVAILABLE:
            // Socket reservation - stub
            case ETH_SOCK_ACQUIRE:
            case ETH_SOCK_RELEASE:
            case ETH_SOCK_STATUS:
            case ETH_SOCK_SET_ACTIVE:
            case ETH_SOCK_INFO: {
                u8 param_size = comms_subfn_param_size(sub_fn);
                if (index + param_size > prog_size) return PROGRAM_SIZE_EXCEEDED;
                index += param_size;
                return STATUS_SUCCESS;
            }
#endif // PLCRUNTIME_ETHERNET_W5500

#ifdef PLCRUNTIME_SERIAL_RS232
            // Serial RS232
            case SER_WRITE:     return handle_SER_WRITE(stack, memory, program, prog_size, index);
            case SER_READ:      return handle_SER_READ(stack, memory, program, prog_size, index);
            case SER_AVAILABLE: return handle_SER_AVAILABLE(stack, program, prog_size, index);
            case SER_FLUSH:     return handle_SER_FLUSH(program, prog_size, index);
            case SER_WRITE_BYTE:return handle_SER_WRITE_BYTE(stack, program, prog_size, index);
            case SER_READ_BYTE: return handle_SER_READ_BYTE(stack, program, prog_size, index);
            case SER_POLL:      return handle_SER_POLL(stack, program, prog_size, index);
            case SER_MSG_READY: return handle_SER_MSG_READY(stack, program, prog_size, index);
            case SER_READ_MSG:  return handle_SER_READ_MSG(stack, memory, program, prog_size, index);
            case SER_SET_DELIM: return handle_SER_SET_DELIM(program, prog_size, index);
            case SER_SET_BAUD:  return handle_SER_SET_BAUD(program, prog_size, index);
#endif // PLCRUNTIME_SERIAL_RS232

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
