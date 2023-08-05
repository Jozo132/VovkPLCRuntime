// runtime-instructions.h - 1.0.0 - 2022-12-11
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

#include "runtime-instructions.h"

#ifdef __RUNTIME_DEBUG__
const char* const unknows_error_code_str PROGMEM = "UNKNOWN_ERROR_CODE";
char runtime_error_msg[40];
const char* RUNTIME_ERROR_NAME(RuntimeError error) {
    char* output = runtime_error_msg;
    if (error > SIZE_OF_ARRAY(RuntimeErrorNames)) fstrcpy(output, unknows_error_code_str);
    else fstrcpy(output, RuntimeErrorNames[error]);
    return output;
}

void logRuntimeErrorList() {
    Serial.println(F("RuntimeError Status List:\n")); // "    %2d: %s"
    for (uint32_t i = 0; i < SIZE_OF_ARRAY(RuntimeErrorNames); i++) {
        Serial.print(F("    "));
        if (i < 10) Serial.print(' ');
        Serial.print(i);
        Serial.print(F(": "));
        Serial.println(RuntimeErrorNames[i]);
    }
    Serial.println();
}
#endif



bool OPCODE_EXISTS(PLCRuntimeInstructionSet opcode) {
    switch (opcode) {
        case NOP:
        case type_bool:
        case type_uint8_t:
        case type_int8_t:
        case type_uint16_t:
        case type_int16_t:
        case type_uint32_t:
        case type_int32_t:
        case type_float:
#ifdef USE_X64_OPS
        case type_uint64_t:
        case type_int64_t:
        case type_double:
#endif
        case MOV:
        case CVT:
        case PUT:
        case GET:
        case COPY:
        case SWAP:
        case DROP:
        case CLEAR:
        case ADD:
        case SUB:
        case MUL:
        case DIV:
            /* TODO: */
            // case MOD:
            // case POW:
            // case SQRT:
            // case MIN:
            // case MAX:
            // case ABS:
            // case MAP:
            // case CON:
            // case RAND:
            // case RAND1:
            // case RAND2:
            // case LN:
            // case LOG10:
            // case LOG2:
            // case EXP:
            // case SIN:
            // case COS:
            // case TAN:
            // case ASIN:
            // case ACOS:
            // case ATAN:
            // case ATAN2:
            // case SINH:
            // case COSH:
            // case TANH:
            // case ASINH:
            // case ACOSH:
            // case ATANH:
        case GET_X8_B0:
        case GET_X8_B1:
        case GET_X8_B2:
        case GET_X8_B3:
        case GET_X8_B4:
        case GET_X8_B5:
        case GET_X8_B6:
        case GET_X8_B7:
        case SET_X8_B0:
        case SET_X8_B1:
        case SET_X8_B2:
        case SET_X8_B3:
        case SET_X8_B4:
        case SET_X8_B5:
        case SET_X8_B6:
        case SET_X8_B7:
        case RSET_X8_B0:
        case RSET_X8_B1:
        case RSET_X8_B2:
        case RSET_X8_B3:
        case RSET_X8_B4:
        case RSET_X8_B5:
        case RSET_X8_B6:
        case RSET_X8_B7:
        case READ_X8_B0:
        case READ_X8_B1:
        case READ_X8_B2:
        case READ_X8_B3:
        case READ_X8_B4:
        case READ_X8_B5:
        case READ_X8_B6:
        case READ_X8_B7:
        case WRITE_X8_B0:
        case WRITE_X8_B1:
        case WRITE_X8_B2:
        case WRITE_X8_B3:
        case WRITE_X8_B4:
        case WRITE_X8_B5:
        case WRITE_X8_B6:
        case WRITE_X8_B7:
        case WRITE_S_X8_B0:
        case WRITE_S_X8_B1:
        case WRITE_S_X8_B2:
        case WRITE_S_X8_B3:
        case WRITE_S_X8_B4:
        case WRITE_S_X8_B5:
        case WRITE_S_X8_B6:
        case WRITE_S_X8_B7:
        case WRITE_R_X8_B0:
        case WRITE_R_X8_B1:
        case WRITE_R_X8_B2:
        case WRITE_R_X8_B3:
        case WRITE_R_X8_B4:
        case WRITE_R_X8_B5:
        case WRITE_R_X8_B6:
        case WRITE_R_X8_B7:
        case WRITE_INV_X8_B0:
        case WRITE_INV_X8_B1:
        case WRITE_INV_X8_B2:
        case WRITE_INV_X8_B3:
        case WRITE_INV_X8_B4:
        case WRITE_INV_X8_B5:
        case WRITE_INV_X8_B6:
        case WRITE_INV_X8_B7:

        case BW_AND_X8:
        case BW_AND_X16:
        case BW_AND_X32:
        case BW_OR_X8:
        case BW_OR_X16:
        case BW_OR_X32:
        case BW_XOR_X8:
        case BW_XOR_X16:
        case BW_XOR_X32:
        case BW_NOT_X8:
        case BW_NOT_X16:
        case BW_NOT_X32:
        case BW_LSHIFT_X8:
        case BW_LSHIFT_X16:
        case BW_LSHIFT_X32:
        case BW_RSHIFT_X8:
        case BW_RSHIFT_X16:
        case BW_RSHIFT_X32:
#ifdef USE_X64_OPS
        case BW_AND_X64:
        case BW_OR_X64:
        case BW_XOR_X64:
        case BW_NOT_X64:
        case BW_LSHIFT_X64:
        case BW_RSHIFT_X64:
#endif
        case LOGIC_AND:
        case LOGIC_OR:
        case LOGIC_XOR:
        case LOGIC_NOT:
        case CMP_EQ:
        case CMP_NEQ:
        case CMP_GT:
        case CMP_LT:
        case CMP_GTE:
        case CMP_LTE:
        case JMP:
        case JMP_IF:
        case JMP_IF_NOT:
        case CALL:
        case CALL_IF:
        case CALL_IF_NOT:
        case RET:
        case RET_IF:
        case RET_IF_NOT:
        case EXIT: return true;
        default: break;
    }
    return false;
}

#ifdef __RUNTIME_DEBUG__
const FSH* OPCODE_NAME(PLCRuntimeInstructionSet opcode) {
    switch (opcode) {
        case NOP: return F("NOP");
        case type_bool: return F("PUSH boolean");
        case type_uint8_t: return F("PUSH uint8_t");
        case type_int8_t: return F("PUSH int8_t");
        case type_uint16_t: return F("PUSH uint16_t");
        case type_int16_t: return F("PUSH int16_t");
        case type_uint32_t: return F("PUSH uint32_t");
        case type_int32_t: return F("PUSH int32_t");
        case type_float: return F("PUSH float");
#ifdef USE_X64_OPS
        case type_uint64_t: return F("PUSH uint64_t");
        case type_int64_t: return F("PUSH int64_t");
        case type_double: return F("PUSH double");
#endif
        case MOV: return F("MOV");
        case CVT: return F("CVT");
        case PUT: return F("PUT");
        case GET: return F("GET");
        case COPY: return F("COPY");
        case SWAP: return F("SWAP");
        case DROP: return F("DROP");
        case CLEAR: return F("CLEAR");
        case ADD: return F("ADD");
        case SUB: return F("SUB");
        case MUL: return F("MUL");
        case DIV: return F("DIV");
            /* TODO: */
            // case MOD: return F("MOD");
            // case POW: return F("POW");
            // case SQRT: return F("SQRT");
            // case MIN: return F("MIN");
            // case MAX: return F("MAX");
            // case ABS: return F("ABS");
            // case MAP: return F("MAP");
            // case CON: return F("CON");
            // case RAND: return F("RAND");
            // case RAND1: return F("RAND1");
            // case RAND2: return F("RAND2");
            // case LN: return F("LN");
            // case LOG10: return F("LOG10");
            // case LOG2: return F("LOG2");
            // case EXP: return F("EXP");
            // case SIN: return F("SIN");
            // case COS: return F("COS");
            // case TAN: return F("TAN");
            // case ASIN: return F("ASIN");
            // case ACOS: return F("ACOS");
            // case ATAN: return F("ATAN");
            // case ATAN2: return F("ATAN2");
            // case SINH: return F("SINH");
            // case COSH: return F("COSH");
            // case TANH: return F("TANH");
            // case ASINH: return F("ASINH");
            // case ACOSH: return F("ACOSH");
            // case ATANH: return F("ATANH");>
        case GET_X8_B0: return F("GET_X8_B0");
        case GET_X8_B1: return F("GET_X8_B1");
        case GET_X8_B2: return F("GET_X8_B2");
        case GET_X8_B3: return F("GET_X8_B3");
        case GET_X8_B4: return F("GET_X8_B4");
        case GET_X8_B5: return F("GET_X8_B5");
        case GET_X8_B6: return F("GET_X8_B6");
        case GET_X8_B7: return F("GET_X8_B7");
        case SET_X8_B0: return F("SET_X8_B0");
        case SET_X8_B1: return F("SET_X8_B1");
        case SET_X8_B2: return F("SET_X8_B2");
        case SET_X8_B3: return F("SET_X8_B3");
        case SET_X8_B4: return F("SET_X8_B4");
        case SET_X8_B5: return F("SET_X8_B5");
        case SET_X8_B6: return F("SET_X8_B6");
        case SET_X8_B7: return F("SET_X8_B7");
        case RSET_X8_B0: return F("RSET_X8_B0");
        case RSET_X8_B1: return F("RSET_X8_B1");
        case RSET_X8_B2: return F("RSET_X8_B2");
        case RSET_X8_B3: return F("RSET_X8_B3");
        case RSET_X8_B4: return F("RSET_X8_B4");
        case RSET_X8_B5: return F("RSET_X8_B5");
        case RSET_X8_B6: return F("RSET_X8_B6");
        case RSET_X8_B7: return F("RSET_X8_B7");
        case READ_X8_B0: return F("READ_X8_B0");
        case READ_X8_B1: return F("READ_X8_B1");
        case READ_X8_B2: return F("READ_X8_B2");
        case READ_X8_B3: return F("READ_X8_B3");
        case READ_X8_B4: return F("READ_X8_B4");
        case READ_X8_B5: return F("READ_X8_B5");
        case READ_X8_B6: return F("READ_X8_B6");
        case READ_X8_B7: return F("READ_X8_B7");
        case WRITE_X8_B0: return F("WRITE_X8_B0");
        case WRITE_X8_B1: return F("WRITE_X8_B1");
        case WRITE_X8_B2: return F("WRITE_X8_B2");
        case WRITE_X8_B3: return F("WRITE_X8_B3");
        case WRITE_X8_B4: return F("WRITE_X8_B4");
        case WRITE_X8_B5: return F("WRITE_X8_B5");
        case WRITE_X8_B6: return F("WRITE_X8_B6");
        case WRITE_X8_B7: return F("WRITE_X8_B7");
        case WRITE_S_X8_B0: return F("WRITE_S_X8_B0");
        case WRITE_S_X8_B1: return F("WRITE_S_X8_B1");
        case WRITE_S_X8_B2: return F("WRITE_S_X8_B2");
        case WRITE_S_X8_B3: return F("WRITE_S_X8_B3");
        case WRITE_S_X8_B4: return F("WRITE_S_X8_B4");
        case WRITE_S_X8_B5: return F("WRITE_S_X8_B5");
        case WRITE_S_X8_B6: return F("WRITE_S_X8_B6");
        case WRITE_S_X8_B7: return F("WRITE_S_X8_B7");
        case WRITE_R_X8_B0: return F("WRITE_R_X8_B0");
        case WRITE_R_X8_B1: return F("WRITE_R_X8_B1");
        case WRITE_R_X8_B2: return F("WRITE_R_X8_B2");
        case WRITE_R_X8_B3: return F("WRITE_R_X8_B3");
        case WRITE_R_X8_B4: return F("WRITE_R_X8_B4");
        case WRITE_R_X8_B5: return F("WRITE_R_X8_B5");
        case WRITE_R_X8_B6: return F("WRITE_R_X8_B6");
        case WRITE_R_X8_B7: return F("WRITE_R_X8_B7");
        case WRITE_INV_X8_B0: return F("WRITE_INV_X8_B0");
        case WRITE_INV_X8_B1: return F("WRITE_INV_X8_B1");
        case WRITE_INV_X8_B2: return F("WRITE_INV_X8_B2");
        case WRITE_INV_X8_B3: return F("WRITE_INV_X8_B3");
        case WRITE_INV_X8_B4: return F("WRITE_INV_X8_B4");
        case WRITE_INV_X8_B5: return F("WRITE_INV_X8_B5");
        case WRITE_INV_X8_B6: return F("WRITE_INV_X8_B6");
        case WRITE_INV_X8_B7: return F("WRITE_INV_X8_B7");

        case BW_AND_X8: return F("BW_AND_X8");
        case BW_AND_X16: return F("BW_AND_X16");
        case BW_AND_X32: return F("BW_AND_X32");
        case BW_OR_X8: return F("BW_OR_X8");
        case BW_OR_X16: return F("BW_OR_X16");
        case BW_OR_X32: return F("BW_OR_X32");
        case BW_XOR_X8: return F("BW_XOR_X8");
        case BW_XOR_X16: return F("BW_XOR_X16");
        case BW_XOR_X32: return F("BW_XOR_X32");
        case BW_NOT_X8: return F("BW_NOT_X8");
        case BW_NOT_X16: return F("BW_NOT_X16");
        case BW_NOT_X32: return F("BW_NOT_X32");
        case BW_LSHIFT_X8: return F("BW_LSHIFT_X8");
        case BW_LSHIFT_X16: return F("BW_LSHIFT_X16");
        case BW_LSHIFT_X32: return F("BW_LSHIFT_X32");
        case BW_RSHIFT_X8: return F("BW_RSHIFT_X8");
        case BW_RSHIFT_X16: return F("BW_RSHIFT_X16");
        case BW_RSHIFT_X32: return F("BW_RSHIFT_X32");
#ifdef USE_X64_OPS
        case BW_AND_X64: return F("BW_AND_X64");
        case BW_OR_X64: return F("BW_OR_X64");
        case BW_XOR_X64: return F("BW_XOR_X64");
        case BW_NOT_X64: return F("BW_NOT_X64");
        case BW_LSHIFT_X64: return F("BW_LSHIFT_X64");
        case BW_RSHIFT_X64: return F("BW_RSHIFT_X64");
#endif
        case LOGIC_AND: return F("LOGIC_AND");
        case LOGIC_OR: return F("LOGIC_OR");
        case LOGIC_XOR: return F("LOGIC_XOR");
        case LOGIC_NOT: return F("LOGIC_NOT");
        case CMP_EQ: return F("CMP_EQ");
        case CMP_NEQ: return F("CMP_NEQ");
        case CMP_GT: return F("CMP_GT");
        case CMP_LT: return F("CMP_LT");
        case CMP_GTE: return F("CMP_GTE");
        case CMP_LTE: return F("CMP_LTE");
        case JMP: return F("JMP");
        case JMP_IF: return F("JMP_IF");
        case JMP_IF_NOT: return F("JMP_IF_NOT");
        case CALL: return F("CALL");
        case CALL_IF: return F("CALL_IF");
        case CALL_IF_NOT: return F("CALL_IF_NOT");
        case RET: return F("RET");
        case RET_IF: return F("RET_IF");
        case RET_IF_NOT: return F("RET_IF_NOT");
        case EXIT: return F("EXIT");
        default: break;
    }
    return F("UNKNOWN OPCODE");
}
#else
const FSH* OPCODE_NAME(PLCRuntimeInstructionSet opcode) {
    bool exists = OPCODE_EXISTS(opcode);
    if (exists) return F("OPTIMIZED OUT");
    return F("UNKNOWN OPCODE");
}
#endif

uint8_t OPCODE_SIZE(PLCRuntimeInstructionSet opcode) {
    switch (opcode) {
        case NOP: return 1;
        case type_bool: return 2;
        case type_uint8_t: return 2;
        case type_int8_t: return 2;
        case type_uint16_t: return 3;
        case type_int16_t: return 3;
        case type_uint32_t: return 5;
        case type_int32_t: return 5;
        case type_float: return 5;
#ifdef USE_X64_OPS
        case type_uint64_t: return 9;
        case type_int64_t: return 9;
        case type_double: return 9;
#endif
        case MOV: return 8;
        case CVT: return 3;
        case PUT: return 4;
        case GET: return 4;
        case COPY: return 2;
        case SWAP: return 2;
        case DROP: return 2;
        case CLEAR: return 1;
        case ADD: return 2;
        case SUB: return 2;
        case MUL: return 2;
        case DIV: return 2;
            // case MOD: return 2;
            // case POW: return 2;
            // case SQRT: return 2;
            // case MIN: return 2;
            // case MAX: return 2;
            // case ABS: return 2;
            // case MAP: return 2;
            // case CON: return 2;
            // case RAND: return 2;
            // case RAND1: return 2;
            // case RAND2: return 2;
            // case LN: return 2;
            // case LOG10: return 2;
            // case LOG2: return 2;
            // case EXP: return 2;
            // case SIN: return 2;
            // case COS: return 2;
            // case TAN: return 2;
            // case ASIN: return 2;
            // case ACOS: return 2;
            // case ATAN: return 2;
            // case ATAN2: return 2;
            // case SINH: return 2;
            // case COSH: return 2;
            // case TANH: return 2;
            // case ASINH: return 2;
            // case ACOSH: return 2;
            // case ATANH: return 2;

        case GET_X8_B0:
        case GET_X8_B1:
        case GET_X8_B2:
        case GET_X8_B3:
        case GET_X8_B4:
        case GET_X8_B5:
        case GET_X8_B6:
        case GET_X8_B7:
        case SET_X8_B0:
        case SET_X8_B1:
        case SET_X8_B2:
        case SET_X8_B3:
        case SET_X8_B4:
        case SET_X8_B5:
        case SET_X8_B6:
        case SET_X8_B7:
        case RSET_X8_B0:
        case RSET_X8_B1:
        case RSET_X8_B2:
        case RSET_X8_B3:
        case RSET_X8_B4:
        case RSET_X8_B5:
        case RSET_X8_B6:
        case RSET_X8_B7: return 1;
        case READ_X8_B0:
        case READ_X8_B1:
        case READ_X8_B2:
        case READ_X8_B3:
        case READ_X8_B4:
        case READ_X8_B5:
        case READ_X8_B6:
        case READ_X8_B7:
        case WRITE_X8_B0:
        case WRITE_X8_B1:
        case WRITE_X8_B2:
        case WRITE_X8_B3:
        case WRITE_X8_B4:
        case WRITE_X8_B5:
        case WRITE_X8_B6:
        case WRITE_X8_B7:
        case WRITE_S_X8_B0:
        case WRITE_S_X8_B1:
        case WRITE_S_X8_B2:
        case WRITE_S_X8_B3:
        case WRITE_S_X8_B4:
        case WRITE_S_X8_B5:
        case WRITE_S_X8_B6:
        case WRITE_S_X8_B7:
        case WRITE_R_X8_B0:
        case WRITE_R_X8_B1:
        case WRITE_R_X8_B2:
        case WRITE_R_X8_B3:
        case WRITE_R_X8_B4:
        case WRITE_R_X8_B5:
        case WRITE_R_X8_B6:
        case WRITE_R_X8_B7:
        case WRITE_INV_X8_B0:
        case WRITE_INV_X8_B1:
        case WRITE_INV_X8_B2:
        case WRITE_INV_X8_B3:
        case WRITE_INV_X8_B4:
        case WRITE_INV_X8_B5:
        case WRITE_INV_X8_B6:
        case WRITE_INV_X8_B7: return 3;

        case BW_AND_X8:
        case BW_AND_X16:
        case BW_AND_X32:
        case BW_OR_X8:
        case BW_OR_X16:
        case BW_OR_X32:
        case BW_XOR_X8:
        case BW_XOR_X16:
        case BW_XOR_X32:
        case BW_NOT_X8:
        case BW_NOT_X16:
        case BW_NOT_X32:
        case BW_LSHIFT_X8:
        case BW_LSHIFT_X16:
        case BW_LSHIFT_X32:
        case BW_RSHIFT_X8:
        case BW_RSHIFT_X16:
        case BW_RSHIFT_X32: return 1;
#ifdef USE_X64_OPS
        case BW_AND_X64:
        case BW_OR_X64:
        case BW_XOR_X64:
        case BW_NOT_X64:
        case BW_LSHIFT_X64:
        case BW_RSHIFT_X64: return 1;
#endif
        case LOGIC_AND:
        case LOGIC_OR:
        case LOGIC_XOR:
        case LOGIC_NOT: return 1;
        case CMP_EQ:
        case CMP_NEQ:
        case CMP_GT:
        case CMP_LT:
        case CMP_GTE:
        case CMP_LTE: return 2;

        case JMP:
        case JMP_IF:
        case JMP_IF_NOT:
        case CALL:
        case CALL_IF:
        case CALL_IF_NOT: return 3;

        case RET:
        case RET_IF:
        case RET_IF_NOT:
        case EXIT: return 1;
        default: break;
    }
    return 0;
}


#ifdef __RUNTIME_DEBUG__
void logRuntimeInstructionSet() {
    Serial.println(F("Bytecode Instruction Set:"));
    uint32_t position = 0;
    bool was_valid = true;
    for (int opcode = 0x00; opcode < 256; opcode++) {
        bool is_valid = OPCODE_EXISTS((PLCRuntimeInstructionSet) opcode);
        if ((is_valid && !was_valid) || opcode < 0x02) Serial.println();
        was_valid = is_valid;
        if (is_valid) {
            position = Serial.print(F("    0x"));
            if (opcode < 0x10) position += Serial.print('0');
            position += Serial.print(opcode, HEX);
            position += Serial.print(F(":  "));
            position += Serial.print(OPCODE_NAME((PLCRuntimeInstructionSet) opcode));
            uint8_t size = OPCODE_SIZE((PLCRuntimeInstructionSet) opcode);
            if (size > 0) {
                for (; position < 26; position++) Serial.print(' ');
                Serial.print(F("("));
                Serial.print(size);
                Serial.print(F(" byte"));
                if (size > 1) Serial.print('s');
                Serial.print(')');
            }
            Serial.println();
        }
        if (opcode == 0xFF) break;
    }
    Serial.println();
}

#else
void logRuntimeInstructionSet() {}
#endif