// wcet-targets.h - WCET target device calibration profiles
//
// Copyright (c) 2026 J.Vovk
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

#ifdef __WASM__

// ============================================================================
// WCET Target Profile System
// ============================================================================
//
// Each calibrated target has a profile containing:
//   - Architecture identifier (e.g., "cortex-m4", "avr", "xtensa")
//   - CPU clock frequency (MHz)
//   - Hardware capabilities (FPU, 64-bit, etc.)
//   - Per-opcode-category cycle costs measured on real hardware
//   - Dispatch overhead (the constant cost of the interpreter loop per instruction)
//
// The calibration flow:
//   1. `npm run calibrate` builds firmware, uploads, runs opcode benchmarks
//   2. Results are printed as a target profile entry
//   3. User pastes the entry into the LUT below
//   4. WCET analysis selects the best matching profile for a given target
//
// If no exact match exists, the system falls back to:
//   1. Same architecture, closest clock speed
//   2. Same architecture family (e.g., any cortex-m for cortex-m4)
//   3. Default abstract cycle units (the existing CU model)

// ============================================================================
// Architecture identifiers
// ============================================================================

#define WCET_ARCH_UNKNOWN       0
#define WCET_ARCH_AVR           1   // ATmega328P, ATmega2560 (8-bit)
#define WCET_ARCH_CORTEX_M0     2   // RP2040, SAMD21
#define WCET_ARCH_CORTEX_M3     3   // STM32F103 (BluePill)
#define WCET_ARCH_CORTEX_M4     4   // STM32F401/F411/G431, nRF52840
#define WCET_ARCH_CORTEX_M4F    5   // Cortex-M4 with HW FPU
#define WCET_ARCH_CORTEX_M7     6   // STM32H743
#define WCET_ARCH_CORTEX_M33    7   // RP2350
#define WCET_ARCH_XTENSA_LX6    8   // ESP32
#define WCET_ARCH_XTENSA_LX7    9   // ESP32-S3
#define WCET_ARCH_RISCV_32     10   // ESP32-C3
#define WCET_ARCH_XTENSA_LX106 11   // ESP8266
#define WCET_ARCH_RENESAS_M4   12   // RA4M1 (Arduino UNO R4)

// ============================================================================
// Hardware capability flags
// ============================================================================

#define WCET_CAP_NONE          0x00
#define WCET_CAP_HW_FPU_SP    0x01  // Single-precision HW FPU
#define WCET_CAP_HW_FPU_DP    0x02  // Double-precision HW FPU
#define WCET_CAP_HW_DIV       0x04  // Hardware integer divider
#define WCET_CAP_HW_MUL32     0x08  // Single-cycle 32-bit multiply
#define WCET_CAP_64BIT        0x10  // Native 64-bit arithmetic
#define WCET_CAP_CACHE         0x20  // Instruction/data cache present
#define WCET_CAP_DSP           0x40  // DSP extensions (MAC, SIMD)

// ============================================================================
// Opcode category indices for the per-category nanosecond cost array
// ============================================================================

#define WCET_CAT_DISPATCH       0   // Interpreter dispatch overhead per instruction
#define WCET_CAT_PUSH_U8        1   // Push 1-byte immediate
#define WCET_CAT_PUSH_U16       2   // Push 2-byte immediate
#define WCET_CAT_PUSH_U32       3   // Push 4-byte immediate
#define WCET_CAT_PUSH_F32       4   // Push 4-byte float immediate
#define WCET_CAT_PUSH_U64       5   // Push 8-byte immediate
#define WCET_CAT_PUSH_F64       6   // Push 8-byte double immediate
#define WCET_CAT_ADD_U8         7   // ADD u8
#define WCET_CAT_ADD_U16        8   // ADD u16
#define WCET_CAT_ADD_U32        9   // ADD u32
#define WCET_CAT_ADD_F32       10   // ADD f32
#define WCET_CAT_ADD_F64       11   // ADD f64
#define WCET_CAT_MUL_U8        12   // MUL u8
#define WCET_CAT_MUL_U32       13   // MUL u32
#define WCET_CAT_MUL_F32       14   // MUL f32
#define WCET_CAT_MUL_F64       15   // MUL f64
#define WCET_CAT_DIV_U8        16   // DIV u8
#define WCET_CAT_DIV_U32       17   // DIV u32
#define WCET_CAT_DIV_F32       18   // DIV f32
#define WCET_CAT_DIV_F64       19   // DIV f64
#define WCET_CAT_CMP            20  // Comparison (any type)
#define WCET_CAT_LOGIC          21  // Logical AND/OR/XOR/NOT
#define WCET_CAT_BITWISE        22  // Bitwise operations
#define WCET_CAT_JMP            23  // Unconditional jump
#define WCET_CAT_JMP_COND       24  // Conditional jump
#define WCET_CAT_CALL           25  // CALL instruction
#define WCET_CAT_RET            26  // RET instruction
#define WCET_CAT_LOAD           27  // Memory load (bounds-checked)
#define WCET_CAT_STORE          28  // Memory store (bounds-checked)
#define WCET_CAT_STACK_OP       29  // COPY/SWAP/DROP/PICK/POKE
#define WCET_CAT_BIT_RW         30  // Bit read/write with address
#define WCET_CAT_TIMER          31  // Timer operations (TON/TOF/TP)
#define WCET_CAT_COUNTER        32  // Counter operations (CTU/CTD)
#define WCET_CAT_SQRT           33  // Square root
#define WCET_CAT_TRIG           34  // Trigonometric (SIN/COS)
#define WCET_CAT_CVT            35  // Type conversion
#define WCET_CAT_STR_OP         36  // String operations
#define WCET_CAT_FFI            37  // FFI call
#define WCET_CAT_EXIT           38  // EXIT instruction
#define WCET_CAT_COUNT          39  // Total number of categories

// ============================================================================
// Target profile structure
// ============================================================================

struct WCETTargetProfile {
    const char* name;                       // Human-readable name, e.g. "STM32F401 BlackPill @84MHz"
    const char* arch_name;                  // Architecture string, e.g. "cortex-m4f"
    u8  arch_id;                            // WCET_ARCH_* constant
    u16 clock_mhz;                         // CPU clock frequency in MHz
    u8  capabilities;                       // WCET_CAP_* flags OR'd together
    u16 ns_per_category[WCET_CAT_COUNT];   // Nanoseconds per opcode category (BCET)
    //   Index 0 (DISPATCH) is added to every instruction
    //   Other indices are the opcode-specific cost ON TOP of dispatch
};

// ============================================================================
// Calibrated target profiles lookup table
// ============================================================================
// Each entry is measured via `npm run calibrate` on real hardware.
// To add a new target:
//   1. Run `npm run calibrate` with the target connected
//   2. Copy the output profile entry into this array
//   3. Rebuild WASM with `npm run build`
//
// The ns_per_category values are MEASURED nanoseconds per instruction
// for each opcode category. Index 0 (DISPATCH) is the base overhead.
//
// Placeholder entries below — replace with real measurements.
// Run: npm run calibrate -- --patch
// to add real calibration entries from hardware measurements.

static const WCETTargetProfile g_wcet_target_profiles[] = {

    // @WCET_PROFILE@ STM32F401 BlackPill @84MHz
    // Cortex-M4F (reference target)
    {
        "STM32F401 BlackPill @84MHz",
        "cortex-m4f", WCET_ARCH_CORTEX_M4F, 84,
        WCET_CAP_HW_FPU_SP | WCET_CAP_HW_DIV | WCET_CAP_HW_MUL32,
        {
            // dispatch, pu8, pu16, pu32, pf32, pu64, pf64,
            14000,  2375,  3088,  3750,  4000,  6750,  8800,
            // add8, add16, add32, addf32, addf64,
             5250,  6300,  9500, 10000, 25000,
            // mul8, mul32, mulf32, mulf64,
             4750,  9500, 10000, 25000,
            // div8, div32, divf32, divf64,
             6000, 10000, 10000, 25000,
            // cmp, logic, bitwise, jmp, jmp_cond, call, ret,
             5250,  3000,  3000,  1000,  1500,  2000,  1500,
            // load, store, stack_op, bit_rw, timer, counter,
             2000,  2000,  1833,  1600,  6000,  5000,
            // sqrt, trig, cvt, str_op, ffi, exit
            30000, 50000, 28000, 140000, 112000,  7000
        }
    },
    // @WCET_PROFILE_END@

    { "", "", 0, 0, 0, { 0 } }, // sentinel — ignored by lookup (empty name)
};

static const u32 g_wcet_target_profile_count = sizeof(g_wcet_target_profiles) / sizeof(g_wcet_target_profiles[0]);

// ============================================================================
// Default fallback profile (abstract cycle units, no real calibration)
// ============================================================================
// Used when no target is specified or no matching profile is found.
// Values are in abstract CU (1 CU ≈ 1 register operation).

static const WCETTargetProfile g_wcet_default_profile = {
    "Default (abstract cycle units)",
    "unknown", WCET_ARCH_UNKNOWN, 0, WCET_CAP_NONE,
    { 0 }  // All zeros — signals to use the abstract CU model from wcet-costs.h
};

// ============================================================================
// Target profile selection
// ============================================================================

// Active target profile (set by wcet_select_target)
static const WCETTargetProfile* g_wcet_active_profile = &g_wcet_default_profile;
static u8  g_wcet_target_match_quality = 0;  // 0=default, 1=family, 2=arch, 3=exact
static char g_wcet_target_match_reason[128] = "No target selected";

// Select a target profile by architecture ID and clock speed.
// Returns: match quality (0-3)
inline u8 wcet_select_target(u8 arch_id, u16 clock_mhz, u8 capabilities) {
    const WCETTargetProfile* best = nullptr;
    u8 best_quality = 0;
    u32 best_clock_diff = 0xFFFFFFFF;

    for (u32 i = 0; i < g_wcet_target_profile_count; i++) {
        const WCETTargetProfile* p = &g_wcet_target_profiles[i];

        // Exact architecture match
        if (p->arch_id == arch_id) {
            u32 clock_diff = (u32)(clock_mhz > p->clock_mhz ? clock_mhz - p->clock_mhz : p->clock_mhz - clock_mhz);

            // Exact clock match with same capabilities = perfect match
            if (clock_diff == 0 && (p->capabilities & capabilities) == capabilities) {
                best = p;
                best_quality = 3;
                best_clock_diff = 0;
                break;
            }

            // Same arch, closest clock
            if (best_quality < 2 || (best_quality == 2 && clock_diff < best_clock_diff)) {
                best = p;
                best_quality = 2;
                best_clock_diff = clock_diff;
            }
        }

        // Architecture family match (e.g., cortex-m4 matches cortex-m4f)
        if (best_quality < 2) {
            bool family_match = false;
            // Cortex-M family (M0, M0+, M3, M4, M4F, M7, M33 are all ARM)
            if (arch_id >= WCET_ARCH_CORTEX_M0 && arch_id <= WCET_ARCH_CORTEX_M33 &&
                p->arch_id >= WCET_ARCH_CORTEX_M0 && p->arch_id <= WCET_ARCH_CORTEX_M33) {
                family_match = true;
            }
            // Xtensa family (LX6, LX7, LX106)
            if ((arch_id == WCET_ARCH_XTENSA_LX6 || arch_id == WCET_ARCH_XTENSA_LX7 || arch_id == WCET_ARCH_XTENSA_LX106) &&
                (p->arch_id == WCET_ARCH_XTENSA_LX6 || p->arch_id == WCET_ARCH_XTENSA_LX7 || p->arch_id == WCET_ARCH_XTENSA_LX106)) {
                family_match = true;
            }

            if (family_match) {
                u32 clock_diff = (u32)(clock_mhz > p->clock_mhz ? clock_mhz - p->clock_mhz : p->clock_mhz - clock_mhz);
                if (best_quality < 1 || (best_quality == 1 && clock_diff < best_clock_diff)) {
                    best = p;
                    best_quality = 1;
                    best_clock_diff = clock_diff;
                }
            }
        }
    }

    if (best) {
        g_wcet_active_profile = best;
        g_wcet_target_match_quality = best_quality;
        const char* quality_str = best_quality == 3 ? "exact" : best_quality == 2 ? "arch" : "family";
        snprintf(g_wcet_target_match_reason, sizeof(g_wcet_target_match_reason),
                 "%s match: %s (%s @%uMHz)", quality_str, best->name, best->arch_name, best->clock_mhz);
    } else {
        g_wcet_active_profile = &g_wcet_default_profile;
        g_wcet_target_match_quality = 0;
        snprintf(g_wcet_target_match_reason, sizeof(g_wcet_target_match_reason),
                 "No matching profile for arch=%u clock=%uMHz", arch_id, clock_mhz);
    }

    return best_quality;
}

// Select target profile by name (partial match, case-insensitive)
inline u8 wcet_select_target_by_name(const char* name) {
    if (!name || !name[0]) {
        g_wcet_active_profile = &g_wcet_default_profile;
        g_wcet_target_match_quality = 0;
        snprintf(g_wcet_target_match_reason, sizeof(g_wcet_target_match_reason), "No target name specified");
        return 0;
    }

    // Case-insensitive partial match on name or arch_name
    for (u32 i = 0; i < g_wcet_target_profile_count; i++) {
        const WCETTargetProfile* p = &g_wcet_target_profiles[i];

        // Check name match
        const char* haystack = p->name;
        const char* needle = name;
        bool found = false;

        for (u32 h = 0; haystack[h]; h++) {
            bool match = true;
            for (u32 n = 0; needle[n]; n++) {
                char a = haystack[h + n];
                char b = needle[n];
                if (a >= 'A' && a <= 'Z') a += 32;
                if (b >= 'A' && b <= 'Z') b += 32;
                if (a != b) { match = false; break; }
            }
            if (match) { found = true; break; }
        }

        if (!found) {
            // Try arch_name
            haystack = p->arch_name;
            for (u32 h = 0; haystack[h]; h++) {
                bool match = true;
                for (u32 n = 0; needle[n]; n++) {
                    char a = haystack[h + n];
                    char b = needle[n];
                    if (a >= 'A' && a <= 'Z') a += 32;
                    if (b >= 'A' && b <= 'Z') b += 32;
                    if (a != b) { match = false; break; }
                }
                if (match) { found = true; break; }
            }
        }

        if (found) {
            g_wcet_active_profile = p;
            g_wcet_target_match_quality = 2;  // At least arch-level match
            snprintf(g_wcet_target_match_reason, sizeof(g_wcet_target_match_reason),
                     "name match: %s (%s @%uMHz)", p->name, p->arch_name, p->clock_mhz);
            return 2;
        }
    }

    g_wcet_active_profile = &g_wcet_default_profile;
    g_wcet_target_match_quality = 0;
    snprintf(g_wcet_target_match_reason, sizeof(g_wcet_target_match_reason),
             "No profile matching '%s'", name);
    return 0;
}

// Map an opcode to its calibration category index
inline u8 wcet_opcode_to_category(u8 opcode, u8 type_arg) {
    switch (opcode) {
        case type_bool: case type_u8: case type_i8: case type_char:
            return WCET_CAT_PUSH_U8;
        case type_u16: case type_i16: case type_pointer:
            return WCET_CAT_PUSH_U16;
        case type_u32: case type_i32:
            return WCET_CAT_PUSH_U32;
        case type_f32:
            return WCET_CAT_PUSH_F32;
        case type_u64: case type_i64:
            return WCET_CAT_PUSH_U64;
        case type_f64:
            return WCET_CAT_PUSH_F64;

        case ADD: case SUB: case NEG: case ABS:
            switch (type_arg) {
                case type_u8: case type_i8: return WCET_CAT_ADD_U8;
                case type_u16: case type_i16: return WCET_CAT_ADD_U16;
                case type_u32: case type_i32: return WCET_CAT_ADD_U32;
                case type_f32: return WCET_CAT_ADD_F32;
                case type_f64: return WCET_CAT_ADD_F64;
                default: return WCET_CAT_ADD_U32;
            }

        case MUL:
            switch (type_arg) {
                case type_u8: case type_i8: return WCET_CAT_MUL_U8;
                case type_u32: case type_i32: return WCET_CAT_MUL_U32;
                case type_f32: return WCET_CAT_MUL_F32;
                case type_f64: return WCET_CAT_MUL_F64;
                default: return WCET_CAT_MUL_U32;
            }

        case DIV: case MOD:
            switch (type_arg) {
                case type_u8: case type_i8: return WCET_CAT_DIV_U8;
                case type_u32: case type_i32: return WCET_CAT_DIV_U32;
                case type_f32: return WCET_CAT_DIV_F32;
                case type_f64: return WCET_CAT_DIV_F64;
                default: return WCET_CAT_DIV_U32;
            }

        case POW: return WCET_CAT_TRIG; // Expensive math, group with trig
        case SQRT: return WCET_CAT_SQRT;
        case SIN: case COS: return WCET_CAT_TRIG;

        case CMP_EQ: case CMP_NEQ: case CMP_GT: case CMP_LT:
        case CMP_GTE: case CMP_LTE:
            return WCET_CAT_CMP;

        case LOGIC_AND: case LOGIC_OR: case LOGIC_XOR: case LOGIC_NOT:
            return WCET_CAT_LOGIC;

        case BW_AND_X8: case BW_OR_X8: case BW_XOR_X8: case BW_NOT_X8:
        case BW_LSHIFT_X8: case BW_RSHIFT_X8:
        case BW_AND_X16: case BW_OR_X16: case BW_XOR_X16: case BW_NOT_X16:
        case BW_LSHIFT_X16: case BW_RSHIFT_X16:
        case BW_AND_X32: case BW_OR_X32: case BW_XOR_X32: case BW_NOT_X32:
        case BW_LSHIFT_X32: case BW_RSHIFT_X32:
        case BW_AND_X64: case BW_OR_X64: case BW_XOR_X64: case BW_NOT_X64:
        case BW_LSHIFT_X64: case BW_RSHIFT_X64:
            return WCET_CAT_BITWISE;

        case JMP: case JMP_REL:
            return WCET_CAT_JMP;
        case JMP_IF: case JMP_IF_NOT: case JMP_IF_REL: case JMP_IF_NOT_REL:
            return WCET_CAT_JMP_COND;
        case CALL: case CALL_IF: case CALL_IF_NOT:
        case CALL_REL: case CALL_IF_REL: case CALL_IF_NOT_REL:
            return WCET_CAT_CALL;
        case RET: case RET_IF: case RET_IF_NOT:
            return WCET_CAT_RET;

        case LOAD: case LOAD_FROM:
            return WCET_CAT_LOAD;
        case MOVE: case MOVE_TO: case MOVE_COPY: case MEM_FILL:
        case INC_MEM: case DEC_MEM:
            return WCET_CAT_STORE;

        case COPY: case SWAP: case DROP: case CLEAR: case PICK: case POKE:
            return WCET_CAT_STACK_OP;

        case GET_X8_B0: case GET_X8_B1: case GET_X8_B2: case GET_X8_B3:
        case GET_X8_B4: case GET_X8_B5: case GET_X8_B6: case GET_X8_B7:
        case SET_X8_B0: case SET_X8_B1: case SET_X8_B2: case SET_X8_B3:
        case SET_X8_B4: case SET_X8_B5: case SET_X8_B6: case SET_X8_B7:
        case RSET_X8_B0: case RSET_X8_B1: case RSET_X8_B2: case RSET_X8_B3:
        case RSET_X8_B4: case RSET_X8_B5: case RSET_X8_B6: case RSET_X8_B7:
        case READ_X8_B0: case READ_X8_B1: case READ_X8_B2: case READ_X8_B3:
        case READ_X8_B4: case READ_X8_B5: case READ_X8_B6: case READ_X8_B7:
        case WRITE_X8_B0: case WRITE_X8_B1: case WRITE_X8_B2: case WRITE_X8_B3:
        case WRITE_X8_B4: case WRITE_X8_B5: case WRITE_X8_B6: case WRITE_X8_B7:
            return WCET_CAT_BIT_RW;

        case TON_CONST: case TON_MEM: case TOF_CONST: case TOF_MEM:
        case TP_CONST: case TP_MEM:
            return WCET_CAT_TIMER;

        case CTU_CONST: case CTU_MEM: case CTD_CONST: case CTD_MEM:
            return WCET_CAT_COUNTER;

        case CVT:
            return WCET_CAT_CVT;

        case STR_LEN: case STR_CAP: case STR_GET: case STR_SET:
        case STR_CLEAR: case STR_CMP: case STR_EQ: case STR_CONCAT:
        case STR_COPY: case STR_SUBSTR: case STR_FIND: case STR_CHAR:
        case STR_TO_NUM: case STR_FROM_NUM: case STR_INIT:
        case CSTR_LIT: case CSTR_CPY: case CSTR_EQ: case CSTR_CAT:
            return WCET_CAT_STR_OP;

        case FFI_CALL: case FFI_CALL_STACK:
            return WCET_CAT_FFI;

        case EXIT:
            return WCET_CAT_EXIT;

        case NOP: case LANG: case COMMENT: case CONFIG_DB: case CONFIG_TC:
            return WCET_CAT_DISPATCH;  // Minimal cost, just dispatch

        default:
            return WCET_CAT_DISPATCH;
    }
}

// Get nanosecond cost for an opcode using the active target profile.
// Returns dispatch + opcode-specific cost (total ns per instruction).
// If no target profile is active (all zeros), returns 0 to signal
// "use abstract CU model instead".
inline u32 wcet_target_ns_for_opcode(u8 opcode, u8 type_arg) {
    const WCETTargetProfile* p = g_wcet_active_profile;
    if (!p || p->ns_per_category[WCET_CAT_DISPATCH] == 0) return 0;

    u8 cat = wcet_opcode_to_category(opcode, type_arg);
    u32 dispatch = p->ns_per_category[WCET_CAT_DISPATCH];
    u32 opcode_ns = p->ns_per_category[cat];

    return dispatch + opcode_ns;
}

// ============================================================================
// WASM-exported list of profile names and count
// ============================================================================

inline u32 wcet_get_target_count() {
    return g_wcet_target_profile_count;
}

inline const char* wcet_get_target_name(u32 index) {
    if (index >= g_wcet_target_profile_count) return "";
    return g_wcet_target_profiles[index].name;
}

inline const char* wcet_get_target_arch(u32 index) {
    if (index >= g_wcet_target_profile_count) return "";
    return g_wcet_target_profiles[index].arch_name;
}

inline u16 wcet_get_target_clock(u32 index) {
    if (index >= g_wcet_target_profile_count) return 0;
    return g_wcet_target_profiles[index].clock_mhz;
}

inline u8 wcet_get_target_caps(u32 index) {
    if (index >= g_wcet_target_profile_count) return 0;
    return g_wcet_target_profiles[index].capabilities;
}

inline u8 wcet_get_match_quality() {
    return g_wcet_target_match_quality;
}

inline const char* wcet_get_match_reason() {
    return g_wcet_target_match_reason;
}

#endif // __WASM__
