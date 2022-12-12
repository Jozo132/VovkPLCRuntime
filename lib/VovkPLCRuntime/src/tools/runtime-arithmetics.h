// runtime-arithmetics.h - 1.0.0 - 2022-12-11
//
// Copyright (c) 2022 J.Vovk
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

struct PLCMethods_t {

    RuntimeError PUSH_BOOL(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        bool value = 0;
        RuntimeError extract_status = Extract.BOOL(program, program_size, index, value);
        if (extract_status != RTE_SUCCESS) return extract_status;
        return stack->pushBool(value);
    }
    RuntimeError PUSH_U8(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        uint8_t value = 0;
        RuntimeError extract_status = Extract.U8(program, program_size, index, value);
        if (extract_status != RTE_SUCCESS) return extract_status;
        return stack->push(value);
    }
    RuntimeError PUSH_U16(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        uint16_t value = 0;
        RuntimeError extract_status = Extract.U16(program, program_size, index, value);
        if (extract_status != RTE_SUCCESS) return extract_status;
        return stack->pushU16(value);
    }
    RuntimeError PUSH_U32(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        uint32_t value = 0;
        RuntimeError extract_status = Extract.U32(program, program_size, index, value);
        if (extract_status != RTE_SUCCESS) return extract_status;
        return stack->pushU32(value);
    }
    RuntimeError PUSH_U64(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        uint64_t value = 0;
        RuntimeError extract_status = Extract.U64(program, program_size, index, value);
        if (extract_status != RTE_SUCCESS) return extract_status;
        return stack->pushU64(value);
    }
    RuntimeError PUSH_S8(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        int8_t value = 0;
        RuntimeError extract_status = Extract.S8(program, program_size, index, value);
        if (extract_status != RTE_SUCCESS) return extract_status;
        return stack->pushS8(value);
    }
    RuntimeError PUSH_S16(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        int16_t value = 0;
        RuntimeError extract_status = Extract.S16(program, program_size, index, value);
        if (extract_status != RTE_SUCCESS) return extract_status;
        return stack->pushS16(value);
    }
    RuntimeError PUSH_S32(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        int32_t value = 0;
        RuntimeError extract_status = Extract.S32(program, program_size, index, value);
        if (extract_status != RTE_SUCCESS) return extract_status;
        return stack->pushS32(value);
    }
    RuntimeError PUSH_S64(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        int64_t value = 0;
        RuntimeError extract_status = Extract.S64(program, program_size, index, value);
        if (extract_status != RTE_SUCCESS) return extract_status;
        return stack->pushS64(value);
    }
    RuntimeError PUSH_F32(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        float value = 0;
        RuntimeError extract_status = Extract.F32(program, program_size, index, value);
        if (extract_status != RTE_SUCCESS) return extract_status;
        return stack->pushF32(value);
    }
    RuntimeError PUSH_F64(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        double value = 0;
        RuntimeError extract_status = Extract.F64(program, program_size, index, value);
        if (extract_status != RTE_SUCCESS) return extract_status;
        return stack->pushF64(value);
    }


    RuntimeError ADD_U8(MyStack* stack) {
        uint8_t b = stack->popU8();
        uint8_t a = stack->popU8();
        stack->push(a + b);
        return RTE_SUCCESS;
    }
    RuntimeError SUB_U8(MyStack* stack) {
        uint8_t b = stack->popU8();
        uint8_t a = stack->popU8();
        stack->push(a - b);
        return RTE_SUCCESS;
    }
    RuntimeError MUL_U8(MyStack* stack) {
        uint8_t b = stack->popU8();
        uint8_t a = stack->popU8();
        stack->push(a * b);
        return RTE_SUCCESS;
    }
    RuntimeError DIV_U8(MyStack* stack) {
        uint8_t b = stack->popU8();
        uint8_t a = stack->popU8();
        stack->push(a / b);
        return RTE_SUCCESS;
    }

    RuntimeError ADD_U16(MyStack* stack) {
        uint16_t b = stack->popU16();
        uint16_t a = stack->popU16();
        stack->pushU16(a + b);
        return RTE_SUCCESS;
    }
    RuntimeError SUB_U16(MyStack* stack) {
        uint16_t b = stack->popU16();
        uint16_t a = stack->popU16();
        stack->pushU16(a - b);
        return RTE_SUCCESS;
    }
    RuntimeError MUL_U16(MyStack* stack) {
        uint16_t b = stack->popU16();
        uint16_t a = stack->popU16();
        stack->pushU16(a * b);
        return RTE_SUCCESS;
    }
    RuntimeError DIV_U16(MyStack* stack) {
        uint16_t b = stack->popU16();
        uint16_t a = stack->popU16();
        stack->pushU16(a / b);
        return RTE_SUCCESS;
    }

    RuntimeError ADD_U32(MyStack* stack) {
        uint32_t b = stack->popU32();
        uint32_t a = stack->popU32();
        stack->pushU32(a + b);
        return RTE_SUCCESS;
    }
    RuntimeError SUB_U32(MyStack* stack) {
        uint32_t b = stack->popU32();
        uint32_t a = stack->popU32();
        stack->pushU32(a - b);
        return RTE_SUCCESS;
    }
    RuntimeError MUL_U32(MyStack* stack) {
        uint32_t b = stack->popU32();
        uint32_t a = stack->popU32();
        stack->pushU32(a * b);
        return RTE_SUCCESS;
    }
    RuntimeError DIV_U32(MyStack* stack) {
        uint32_t b = stack->popU32();
        uint32_t a = stack->popU32();
        stack->pushU32(a / b);
        return RTE_SUCCESS;
    }

    RuntimeError ADD_U64(MyStack* stack) {
        uint64_t b = stack->popU64();
        uint64_t a = stack->popU64();
        stack->pushU64(a + b);
        return RTE_SUCCESS;
    }
    RuntimeError SUB_U64(MyStack* stack) {
        uint64_t b = stack->popU64();
        uint64_t a = stack->popU64();
        stack->pushU64(a - b);
        return RTE_SUCCESS;
    }
    RuntimeError MUL_U64(MyStack* stack) {
        uint64_t b = stack->popU64();
        uint64_t a = stack->popU64();
        stack->pushU64(a * b);
        return RTE_SUCCESS;
    }
    RuntimeError DIV_U64(MyStack* stack) {
        uint64_t b = stack->popU64();
        uint64_t a = stack->popU64();
        stack->pushU64(a / b);
        return RTE_SUCCESS;
    }

    RuntimeError ADD_S8(MyStack* stack) {
        int8_t b = stack->popS8();
        int8_t a = stack->popS8();
        stack->pushS8(a + b);
        return RTE_SUCCESS;
    }
    RuntimeError SUB_S8(MyStack* stack) {
        int8_t b = stack->popS8();
        int8_t a = stack->popS8();
        stack->pushS8(a - b);
        return RTE_SUCCESS;
    }
    RuntimeError MUL_S8(MyStack* stack) {
        int8_t b = stack->popS8();
        int8_t a = stack->popS8();
        stack->pushS8(a * b);
        return RTE_SUCCESS;
    }
    RuntimeError DIV_S8(MyStack* stack) {
        int8_t b = stack->popS8();
        int8_t a = stack->popS8();
        stack->pushS8(a / b);
        return RTE_SUCCESS;
    }

    RuntimeError ADD_S16(MyStack* stack) {
        int16_t b = stack->popS16();
        int16_t a = stack->popS16();
        stack->pushS16(a + b);
        return RTE_SUCCESS;
    }
    RuntimeError SUB_S16(MyStack* stack) {
        int16_t b = stack->popS16();
        int16_t a = stack->popS16();
        stack->pushS16(a - b);
        return RTE_SUCCESS;
    }
    RuntimeError MUL_S16(MyStack* stack) {
        int16_t b = stack->popS16();
        int16_t a = stack->popS16();
        stack->pushS16(a * b);
        return RTE_SUCCESS;
    }
    RuntimeError DIV_S16(MyStack* stack) {
        int16_t b = stack->popS16();
        int16_t a = stack->popS16();
        stack->pushS16(a / b);
        return RTE_SUCCESS;
    }

    RuntimeError ADD_S32(MyStack* stack) {
        int32_t b = stack->popS32();
        int32_t a = stack->popS32();
        stack->pushS32(a + b);
        return RTE_SUCCESS;
    }
    RuntimeError SUB_S32(MyStack* stack) {
        int32_t b = stack->popS32();
        int32_t a = stack->popS32();
        stack->pushS32(a - b);
        return RTE_SUCCESS;
    }
    RuntimeError MUL_S32(MyStack* stack) {
        int32_t b = stack->popS32();
        int32_t a = stack->popS32();
        stack->pushS32(a * b);
        return RTE_SUCCESS;
    }
    RuntimeError DIV_S32(MyStack* stack) {
        int32_t b = stack->popS32();
        int32_t a = stack->popS32();
        stack->pushS32(a / b);
        return RTE_SUCCESS;
    }

    RuntimeError ADD_S64(MyStack* stack) {
        int64_t b = stack->popS64();
        int64_t a = stack->popS64();
        stack->pushS64(a + b);
        return RTE_SUCCESS;
    }
    RuntimeError SUB_S64(MyStack* stack) {
        int64_t b = stack->popS64();
        int64_t a = stack->popS64();
        stack->pushS64(a - b);
        return RTE_SUCCESS;
    }
    RuntimeError MUL_S64(MyStack* stack) {
        int64_t b = stack->popS64();
        int64_t a = stack->popS64();
        stack->pushS64(a * b);
        return RTE_SUCCESS;
    }
    RuntimeError DIV_S64(MyStack* stack) {
        int64_t b = stack->popS64();
        int64_t a = stack->popS64();
        stack->pushS64(a / b);
        return RTE_SUCCESS;
    }

    RuntimeError ADD_F32(MyStack* stack) {
        float b = stack->popF32();
        float a = stack->popF32();
        stack->pushF32(a + b);
        return RTE_SUCCESS;
    }
    RuntimeError SUB_F32(MyStack* stack) {
        float b = stack->popF32();
        float a = stack->popF32();
        stack->pushF32(a - b);
        return RTE_SUCCESS;
    }
    RuntimeError MUL_F32(MyStack* stack) {
        float b = stack->popF32();
        float a = stack->popF32();
        stack->pushF32(a * b);
        return RTE_SUCCESS;
    }
    RuntimeError DIV_F32(MyStack* stack) {
        float b = stack->popF32();
        float a = stack->popF32();
        stack->pushF32(a / b);
        return RTE_SUCCESS;
    }

    RuntimeError ADD_F64(MyStack* stack) {
        double b = stack->popF64();
        double a = stack->popF64();
        stack->pushF64(a + b);
        return RTE_SUCCESS;
    }
    RuntimeError SUB_F64(MyStack* stack) {
        double b = stack->popF64();
        double a = stack->popF64();
        stack->pushF64(a - b);
        return RTE_SUCCESS;
    }
    RuntimeError MUL_F64(MyStack* stack) {
        double b = stack->popF64();
        double a = stack->popF64();
        stack->pushF64(a * b);
        return RTE_SUCCESS;
    }
    RuntimeError DIV_F64(MyStack* stack) {
        double b = stack->popF64();
        double a = stack->popF64();
        stack->pushF64(a / b);
        return RTE_SUCCESS;
    }







    RuntimeError handle_ADD(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = program[index++];
        switch (data_type) {
            case BOOL:
            case U8: return ADD_U8(stack);
            case U16: return ADD_U16(stack);
            case U32: return ADD_U32(stack);
            case U64: return ADD_U64(stack);
            case S8: return ADD_S8(stack);
            case S16: return ADD_S16(stack);
            case S32: return ADD_S32(stack);
            case S64: return ADD_S64(stack);
            case F32: return ADD_F32(stack);
            case F64: return ADD_F64(stack);
            default: return RTE_INVALID_DATA_TYPE;
        }
    }

    RuntimeError handle_SUB(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = program[index++];
        switch (data_type) {
            case BOOL:
            case U8: return SUB_U8(stack);
            case U16: return SUB_U16(stack);
            case U32: return SUB_U32(stack);
            case U64: return SUB_U64(stack);
            case S8: return SUB_S8(stack);
            case S16: return SUB_S16(stack);
            case S32: return SUB_S32(stack);
            case S64: return SUB_S64(stack);
            case F32: return SUB_F32(stack);
            case F64: return SUB_F64(stack);
            default: return RTE_INVALID_DATA_TYPE;
        }
    }

    RuntimeError handle_MUL(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = program[index++];
        switch (data_type) {
            case BOOL:
            case U8: return MUL_U8(stack);
            case U16: return MUL_U16(stack);
            case U32: return MUL_U32(stack);
            case U64: return MUL_U64(stack);
            case S8: return MUL_S8(stack);
            case S16: return MUL_S16(stack);
            case S32: return MUL_S32(stack);
            case S64: return MUL_S64(stack);
            case F32: return MUL_F32(stack);
            case F64: return MUL_F64(stack);
            default: return RTE_INVALID_DATA_TYPE;
        }
    }

    RuntimeError handle_DIV(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = program[index++];
        switch (data_type) {
            case BOOL:
            case U8: return DIV_U8(stack);
            case U16: return DIV_U16(stack);
            case U32: return DIV_U32(stack);
            case U64: return DIV_U64(stack);
            case S8: return DIV_S8(stack);
            case S16: return DIV_S16(stack);
            case S32: return DIV_S32(stack);
            case S64: return DIV_S64(stack);
            case F32: return DIV_F32(stack);
            case F64: return DIV_F64(stack);
            default: return RTE_INVALID_DATA_TYPE;
        }
    }



    RuntimeError BW_AND_X8(MyStack* stack) {
        uint8_t b = stack->popU8();
        uint8_t a = stack->popU8();
        stack->pushU8(a & b);
        return RTE_SUCCESS;
    }
    RuntimeError BW_AND_X16(MyStack* stack) {
        uint16_t b = stack->popU16();
        uint16_t a = stack->popU16();
        stack->pushU16(a & b);
        return RTE_SUCCESS;
    }
    RuntimeError BW_AND_X32(MyStack* stack) {
        uint32_t b = stack->popU32();
        uint32_t a = stack->popU32();
        stack->pushU32(a & b);
        return RTE_SUCCESS;
    }
    RuntimeError BW_AND_X64(MyStack* stack) {
        uint64_t b = stack->popU64();
        uint64_t a = stack->popU64();
        stack->pushU64(a & b);
        return RTE_SUCCESS;
    }

    RuntimeError BW_OR_X8(MyStack* stack) {
        uint8_t b = stack->popU8();
        uint8_t a = stack->popU8();
        stack->pushU8(a | b);
        return RTE_SUCCESS;
    }
    RuntimeError BW_OR_X16(MyStack* stack) {
        uint16_t b = stack->popU16();
        uint16_t a = stack->popU16();
        stack->pushU16(a | b);
        return RTE_SUCCESS;
    }
    RuntimeError BW_OR_X32(MyStack* stack) {
        uint32_t b = stack->popU32();
        uint32_t a = stack->popU32();
        stack->pushU32(a | b);
        return RTE_SUCCESS;
    }
    RuntimeError BW_OR_X64(MyStack* stack) {
        uint64_t b = stack->popU64();
        uint64_t a = stack->popU64();
        stack->pushU64(a | b);
        return RTE_SUCCESS;
    }

    RuntimeError BW_XOR_X8(MyStack* stack) {
        uint8_t b = stack->popU8();
        uint8_t a = stack->popU8();
        stack->pushU8(a ^ b);
        return RTE_SUCCESS;
    }
    RuntimeError BW_XOR_X16(MyStack* stack) {
        uint16_t b = stack->popU16();
        uint16_t a = stack->popU16();
        stack->pushU16(a ^ b);
        return RTE_SUCCESS;
    }
    RuntimeError BW_XOR_X32(MyStack* stack) {
        uint32_t b = stack->popU32();
        uint32_t a = stack->popU32();
        stack->pushU32(a ^ b);
        return RTE_SUCCESS;
    }
    RuntimeError BW_XOR_X64(MyStack* stack) {
        uint64_t b = stack->popU64();
        uint64_t a = stack->popU64();
        stack->pushU64(a ^ b);
        return RTE_SUCCESS;
    }

    RuntimeError BW_NOT_X8(MyStack* stack) {
        uint8_t a = stack->popU8();
        stack->pushU8(~a);
        return RTE_SUCCESS;
    }
    RuntimeError BW_NOT_X16(MyStack* stack) {
        uint16_t a = stack->popU16();
        stack->pushU16(~a);
        return RTE_SUCCESS;
    }
    RuntimeError BW_NOT_X32(MyStack* stack) {
        uint32_t a = stack->popU32();
        stack->pushU32(~a);
        return RTE_SUCCESS;
    }
    RuntimeError BW_NOT_X64(MyStack* stack) {
        uint64_t a = stack->popU64();
        stack->pushU64(~a);
        return RTE_SUCCESS;
    }

    RuntimeError BW_LSHIFT_X8(MyStack* stack) {
        uint8_t b = stack->popU8();
        uint8_t a = stack->popU8();
        stack->pushU8(a << b);
        return RTE_SUCCESS;
    }
    RuntimeError BW_LSHIFT_X16(MyStack* stack) {
        uint16_t b = stack->popU16();
        uint16_t a = stack->popU16();
        stack->pushU16(a << b);
        return RTE_SUCCESS;
    }
    RuntimeError BW_LSHIFT_X32(MyStack* stack) {
        uint32_t b = stack->popU32();
        uint32_t a = stack->popU32();
        stack->pushU32(a << b);
        return RTE_SUCCESS;
    }
    RuntimeError BW_LSHIFT_X64(MyStack* stack) {
        uint64_t b = stack->popU64();
        uint64_t a = stack->popU64();
        stack->pushU64(a << b);
        return RTE_SUCCESS;
    }

    RuntimeError BW_RSHIFT_X8(MyStack* stack) {
        uint8_t b = stack->popU8();
        uint8_t a = stack->popU8();
        stack->pushU8(a >> b);
        return RTE_SUCCESS;
    }
    RuntimeError BW_RSHIFT_X16(MyStack* stack) {
        uint16_t b = stack->popU16();
        uint16_t a = stack->popU16();
        stack->pushU16(a >> b);
        return RTE_SUCCESS;
    }
    RuntimeError BW_RSHIFT_X32(MyStack* stack) {
        uint32_t b = stack->popU32();
        uint32_t a = stack->popU32();
        stack->pushU32(a >> b);
        return RTE_SUCCESS;
    }
    RuntimeError BW_RSHIFT_X64(MyStack* stack) {
        uint64_t b = stack->popU64();
        uint64_t a = stack->popU64();
        stack->pushU64(a >> b);
        return RTE_SUCCESS;
    }

    // Boolean operators
    RuntimeError LOGIC_AND(MyStack* stack) {
        uint8_t b = stack->popU8() != 0;
        uint8_t a = stack->popU8() != 0;
        stack->pushU8(a && b);
        return RTE_SUCCESS;
    }
    RuntimeError LOGIC_OR(MyStack* stack) {
        uint8_t b = stack->popU8() != 0;
        uint8_t a = stack->popU8() != 0;
        stack->pushU8(a || b);
        return RTE_SUCCESS;
    }
    RuntimeError LOGIC_XOR(MyStack* stack) {
        uint8_t b = stack->popU8() != 0;
        uint8_t a = stack->popU8() != 0;
        stack->pushU8(a ^ b);
        return RTE_SUCCESS;
    }
    RuntimeError LOGIC_NOT(MyStack* stack) {
        uint8_t a = stack->popU8() != 0;
        stack->pushU8(!a);
        return RTE_SUCCESS;
    }

    RuntimeError CMP_EQ_U8(MyStack* stack) {
        uint8_t b = stack->popU8();
        uint8_t a = stack->popU8();
        stack->pushU8(a == b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_EQ_U16(MyStack* stack) {
        uint16_t b = stack->popU16();
        uint16_t a = stack->popU16();
        stack->pushU8(a == b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_EQ_U32(MyStack* stack) {
        uint32_t b = stack->popU32();
        uint32_t a = stack->popU32();
        stack->pushU8(a == b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_EQ_U64(MyStack* stack) {
        uint64_t b = stack->popU64();
        uint64_t a = stack->popU64();
        stack->pushU8(a == b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_EQ_S8(MyStack* stack) {
        int8_t b = stack->popS8();
        int8_t a = stack->popS8();
        stack->pushU8(a == b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_EQ_S16(MyStack* stack) {
        int16_t b = stack->popS16();
        int16_t a = stack->popS16();
        stack->pushU8(a == b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_EQ_S32(MyStack* stack) {
        int32_t b = stack->popS32();
        int32_t a = stack->popS32();
        stack->pushU8(a == b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_EQ_S64(MyStack* stack) {
        int64_t b = stack->popS64();
        int64_t a = stack->popS64();
        stack->pushU8(a == b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_EQ_F32(MyStack* stack) {
        float b = stack->popF32();
        float a = stack->popF32();
        stack->pushU8(a == b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_EQ_F64(MyStack* stack) {
        double b = stack->popF64();
        double a = stack->popF64();
        stack->pushU8(a == b);
        return RTE_SUCCESS;
    }

    RuntimeError CMP_NEQ_U8(MyStack* stack) {
        uint8_t b = stack->popU8();
        uint8_t a = stack->popU8();
        stack->pushU8(a != b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_NEQ_U16(MyStack* stack) {
        uint16_t b = stack->popU16();
        uint16_t a = stack->popU16();
        stack->pushU8(a != b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_NEQ_U32(MyStack* stack) {
        uint32_t b = stack->popU32();
        uint32_t a = stack->popU32();
        stack->pushU8(a != b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_NEQ_U64(MyStack* stack) {
        uint64_t b = stack->popU64();
        uint64_t a = stack->popU64();
        stack->pushU8(a != b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_NEQ_S8(MyStack* stack) {
        int8_t b = stack->popS8();
        int8_t a = stack->popS8();
        stack->pushU8(a != b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_NEQ_S16(MyStack* stack) {
        int16_t b = stack->popS16();
        int16_t a = stack->popS16();
        stack->pushU8(a != b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_NEQ_S32(MyStack* stack) {
        int32_t b = stack->popS32();
        int32_t a = stack->popS32();
        stack->pushU8(a != b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_NEQ_S64(MyStack* stack) {
        int64_t b = stack->popS64();
        int64_t a = stack->popS64();
        stack->pushU8(a != b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_NEQ_F32(MyStack* stack) {
        float b = stack->popF32();
        float a = stack->popF32();
        stack->pushU8(a != b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_NEQ_F64(MyStack* stack) {
        double b = stack->popF64();
        double a = stack->popF64();
        stack->pushU8(a != b);
        return RTE_SUCCESS;
    }

    RuntimeError CMP_GT_U8(MyStack* stack) {
        uint8_t b = stack->popU8();
        uint8_t a = stack->popU8();
        stack->pushU8(a > b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_GT_U16(MyStack* stack) {
        uint16_t b = stack->popU16();
        uint16_t a = stack->popU16();
        stack->pushU8(a > b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_GT_U32(MyStack* stack) {
        uint32_t b = stack->popU32();
        uint32_t a = stack->popU32();
        stack->pushU8(a > b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_GT_U64(MyStack* stack) {
        uint64_t b = stack->popU64();
        uint64_t a = stack->popU64();
        stack->pushU8(a > b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_GT_S8(MyStack* stack) {
        int8_t b = stack->popS8();
        int8_t a = stack->popS8();
        stack->pushU8(a > b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_GT_S16(MyStack* stack) {
        int16_t b = stack->popS16();
        int16_t a = stack->popS16();
        stack->pushU8(a > b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_GT_S32(MyStack* stack) {
        int32_t b = stack->popS32();
        int32_t a = stack->popS32();
        stack->pushU8(a > b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_GT_S64(MyStack* stack) {
        int64_t b = stack->popS64();
        int64_t a = stack->popS64();
        stack->pushU8(a > b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_GT_F32(MyStack* stack) {
        float b = stack->popF32();
        float a = stack->popF32();
        stack->pushU8(a > b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_GT_F64(MyStack* stack) {
        double b = stack->popF64();
        double a = stack->popF64();
        stack->pushU8(a > b);
        return RTE_SUCCESS;
    }

    RuntimeError CMP_LT_U8(MyStack* stack) {
        uint8_t b = stack->popU8();
        uint8_t a = stack->popU8();
        stack->pushU8(a < b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_LT_U16(MyStack* stack) {
        uint16_t b = stack->popU16();
        uint16_t a = stack->popU16();
        stack->pushU8(a < b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_LT_U32(MyStack* stack) {
        uint32_t b = stack->popU32();
        uint32_t a = stack->popU32();
        stack->pushU8(a < b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_LT_U64(MyStack* stack) {
        uint64_t b = stack->popU64();
        uint64_t a = stack->popU64();
        stack->pushU8(a < b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_LT_S8(MyStack* stack) {
        int8_t b = stack->popS8();
        int8_t a = stack->popS8();
        stack->pushU8(a < b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_LT_S16(MyStack* stack) {
        int16_t b = stack->popS16();
        int16_t a = stack->popS16();
        stack->pushU8(a < b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_LT_S32(MyStack* stack) {
        int32_t b = stack->popS32();
        int32_t a = stack->popS32();
        stack->pushU8(a < b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_LT_S64(MyStack* stack) {
        int64_t b = stack->popS64();
        int64_t a = stack->popS64();
        stack->pushU8(a < b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_LT_F32(MyStack* stack) {
        float b = stack->popF32();
        float a = stack->popF32();
        stack->pushU8(a < b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_LT_F64(MyStack* stack) {
        double b = stack->popF64();
        double a = stack->popF64();
        stack->pushU8(a < b);
        return RTE_SUCCESS;
    }

    RuntimeError CMP_GTE_U8(MyStack* stack) {
        uint8_t b = stack->popU8();
        uint8_t a = stack->popU8();
        stack->pushU8(a >= b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_GTE_U16(MyStack* stack) {
        uint16_t b = stack->popU16();
        uint16_t a = stack->popU16();
        stack->pushU8(a >= b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_GTE_U32(MyStack* stack) {
        uint32_t b = stack->popU32();
        uint32_t a = stack->popU32();
        stack->pushU8(a >= b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_GTE_U64(MyStack* stack) {
        uint64_t b = stack->popU64();
        uint64_t a = stack->popU64();
        stack->pushU8(a >= b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_GTE_S8(MyStack* stack) {
        int8_t b = stack->popS8();
        int8_t a = stack->popS8();
        stack->pushU8(a >= b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_GTE_S16(MyStack* stack) {
        int16_t b = stack->popS16();
        int16_t a = stack->popS16();
        stack->pushU8(a >= b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_GTE_S32(MyStack* stack) {
        int32_t b = stack->popS32();
        int32_t a = stack->popS32();
        stack->pushU8(a >= b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_GTE_S64(MyStack* stack) {
        int64_t b = stack->popS64();
        int64_t a = stack->popS64();
        stack->pushU8(a >= b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_GTE_F32(MyStack* stack) {
        float b = stack->popF32();
        float a = stack->popF32();
        stack->pushU8(a >= b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_GTE_F64(MyStack* stack) {
        double b = stack->popF64();
        double a = stack->popF64();
        stack->pushU8(a >= b);
        return RTE_SUCCESS;
    }

    RuntimeError CMP_LTE_U8(MyStack* stack) {
        uint8_t b = stack->popU8();
        uint8_t a = stack->popU8();
        stack->pushU8(a <= b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_LTE_U16(MyStack* stack) {
        uint16_t b = stack->popU16();
        uint16_t a = stack->popU16();
        stack->pushU8(a <= b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_LTE_U32(MyStack* stack) {
        uint32_t b = stack->popU32();
        uint32_t a = stack->popU32();
        stack->pushU8(a <= b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_LTE_U64(MyStack* stack) {
        uint64_t b = stack->popU64();
        uint64_t a = stack->popU64();
        stack->pushU8(a <= b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_LTE_S8(MyStack* stack) {
        int8_t b = stack->popS8();
        int8_t a = stack->popS8();
        stack->pushU8(a <= b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_LTE_S16(MyStack* stack) {
        int16_t b = stack->popS16();
        int16_t a = stack->popS16();
        stack->pushU8(a <= b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_LTE_S32(MyStack* stack) {
        int32_t b = stack->popS32();
        int32_t a = stack->popS32();
        stack->pushU8(a <= b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_LTE_S64(MyStack* stack) {
        int64_t b = stack->popS64();
        int64_t a = stack->popS64();
        stack->pushU8(a <= b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_LTE_F32(MyStack* stack) {
        float b = stack->popF32();
        float a = stack->popF32();
        stack->pushU8(a <= b);
        return RTE_SUCCESS;
    }
    RuntimeError CMP_LTE_F64(MyStack* stack) {
        double b = stack->popF64();
        double a = stack->popF64();
        stack->pushU8(a <= b);
        return RTE_SUCCESS;
    }

    // Comparison operators
    RuntimeError handle_CMP_EQ(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = program[index++];
        switch (data_type) {
            case BOOL:
            case U8: return CMP_EQ_U8(stack);
            case U16: return CMP_EQ_U16(stack);
            case U32: return CMP_EQ_U32(stack);
            case U64: return CMP_EQ_U64(stack);
            case S8: return CMP_EQ_S8(stack);
            case S16: return CMP_EQ_S16(stack);
            case S32: return CMP_EQ_S32(stack);
            case S64: return CMP_EQ_S64(stack);
            case F32: return CMP_EQ_F32(stack);
            case F64: return CMP_EQ_F64(stack);
            default: return RTE_INVALID_DATA_TYPE;
        }
    }
    RuntimeError handle_CMP_NEQ(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = program[index++];
        switch (data_type) {
            case BOOL:
            case U8: return CMP_NEQ_U8(stack);
            case U16: return CMP_NEQ_U16(stack);
            case U32: return CMP_NEQ_U32(stack);
            case U64: return CMP_NEQ_U64(stack);
            case S8: return CMP_NEQ_S8(stack);
            case S16: return CMP_NEQ_S16(stack);
            case S32: return CMP_NEQ_S32(stack);
            case S64: return CMP_NEQ_S64(stack);
            case F32: return CMP_NEQ_F32(stack);
            case F64: return CMP_NEQ_F64(stack);
            default: return RTE_INVALID_DATA_TYPE;
        }
    }
    RuntimeError handle_CMP_GT(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = program[index++];
        switch (data_type) {
            case BOOL:
            case U8: return CMP_GT_U8(stack);
            case U16: return CMP_GT_U16(stack);
            case U32: return CMP_GT_U32(stack);
            case U64: return CMP_GT_U64(stack);
            case S8: return CMP_GT_S8(stack);
            case S16: return CMP_GT_S16(stack);
            case S32: return CMP_GT_S32(stack);
            case S64: return CMP_GT_S64(stack);
            case F32: return CMP_GT_F32(stack);
            case F64: return CMP_GT_F64(stack);
            default: return RTE_INVALID_DATA_TYPE;
        }
    }
    RuntimeError handle_CMP_GTE(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = program[index++];
        switch (data_type) {
            case BOOL:
            case U8: return CMP_GTE_U8(stack);
            case U16: return CMP_GTE_U16(stack);
            case U32: return CMP_GTE_U32(stack);
            case U64: return CMP_GTE_U64(stack);
            case S8: return CMP_GTE_S8(stack);
            case S16: return CMP_GTE_S16(stack);
            case S32: return CMP_GTE_S32(stack);
            case S64: return CMP_GTE_S64(stack);
            case F32: return CMP_GTE_F32(stack);
            case F64: return CMP_GTE_F64(stack);
            default: return RTE_INVALID_DATA_TYPE;
        }
    }
    RuntimeError handle_CMP_LT(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = program[index++];
        switch (data_type) {
            case BOOL:
            case U8: return CMP_LT_U8(stack);
            case U16: return CMP_LT_U16(stack);
            case U32: return CMP_LT_U32(stack);
            case U64: return CMP_LT_U64(stack);
            case S8: return CMP_LT_S8(stack);
            case S16: return CMP_LT_S16(stack);
            case S32: return CMP_LT_S32(stack);
            case S64: return CMP_LT_S64(stack);
            case F32: return CMP_LT_F32(stack);
            case F64: return CMP_LT_F64(stack);
            default: return RTE_INVALID_DATA_TYPE;
        }
    }
    RuntimeError handle_CMP_LTE(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 1;
        if (index + size > program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        uint8_t data_type = program[index++];
        switch (data_type) {
            case BOOL:
            case U8: return CMP_LTE_U8(stack);
            case U16: return CMP_LTE_U16(stack);
            case U32: return CMP_LTE_U32(stack);
            case U64: return CMP_LTE_U64(stack);
            case S8: return CMP_LTE_S8(stack);
            case S16: return CMP_LTE_S16(stack);
            case S32: return CMP_LTE_S32(stack);
            case S64: return CMP_LTE_S64(stack);
            case F32: return CMP_LTE_F32(stack);
            case F64: return CMP_LTE_F64(stack);
            default: return RTE_INVALID_DATA_TYPE;
        }
    }


    RuntimeError handle_JMP(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 2;
        if (index + size > program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        U8A_to_U16 cvt;
        cvt.U8A[1] = program[index];
        cvt.U8A[0] = program[index + 1];
        index = cvt.U16;
        if (index >= program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        return RTE_SUCCESS;
    }
    RuntimeError handle_JMP_IF(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 2;
        if (index + size > program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        U8A_to_U16 cvt;
        cvt.U8A[1] = program[index];
        cvt.U8A[0] = program[index + 1];
        if (stack->popBool()) {
            index = cvt.U16;
            if (index >= program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        }
        return RTE_SUCCESS;
    }
    RuntimeError handle_JMP_IF_NOT(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 2;
        if (index + size > program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        U8A_to_U16 cvt;
        cvt.U8A[1] = program[index];
        cvt.U8A[0] = program[index + 1];
        if (!stack->popBool()) {
            index = cvt.U16;
            if (index >= program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        }
        return RTE_SUCCESS;
    }

    RuntimeError handle_CALL(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 2;
        if (index + size > program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        U8A_to_U16 cvt;
        cvt.U8A[1] = program[index];
        cvt.U8A[0] = program[index + 1];
        index = cvt.U16;
        if (index >= program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        RuntimeError call_store_status = stack->pushCall(index);
        if (call_store_status != RTE_SUCCESS) return call_store_status;
        if (index >= program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        return RTE_SUCCESS;
    }
    RuntimeError handle_CALL_IF(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 2;
        if (index + size > program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        U8A_to_U16 cvt;
        cvt.U8A[1] = program[index];
        cvt.U8A[0] = program[index + 1];
        if (stack->popBool()) {
            index = cvt.U16;
            if (index == 0 || index >= program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
            RuntimeError call_store_status = stack->pushCall(index);
            if (call_store_status != RTE_SUCCESS) return call_store_status;
            if (index >= program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        }
        return RTE_SUCCESS;
    }
    RuntimeError handle_CALL_IF_NOT(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        static uint16_t size = 2;
        if (index + size > program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        U8A_to_U16 cvt;
        cvt.U8A[1] = program[index];
        cvt.U8A[0] = program[index + 1];
        if (!stack->popBool()) {
            index = cvt.U16;
            if (index == 0 || index >= program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
            RuntimeError call_store_status = stack->pushCall(index);
            if (call_store_status != RTE_SUCCESS) return call_store_status;
            if (index >= program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        }
        return RTE_SUCCESS;
    }
    RuntimeError handle_RET(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        if (stack->call_stack->size() == 0) return RTE_CALL_STACK_UNDERFLOW;
        uint16_t ret_index = stack->popCall();
        index = ret_index;
        if (index >= program_size) return RTE_PROGRAM_POINTER_OUT_OF_BOUNDS;
        return RTE_SUCCESS;
    }


    RuntimeError handle_EXIT(MyStack* stack, uint8_t* program, uint16_t program_size, uint16_t& index) {
        if (index >= program_size) return RTE_SUCCESS;
        uint8_t exit_code = program[index++];
        return (RuntimeError) exit_code;
    }

} PLCMethods;