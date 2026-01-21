// network-ir.h - Network Intermediate Representation for LADDER ⇄ STL
// Treats LADDER and STL as two serializations of the same "boolean network + actions" concept
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

#include "wasm/wasm.h"
#include "./../runtime-types.h"

// ============================================================================
// Network IR - Intermediate Representation for PLC Logic
// ============================================================================
//
// A Network (rung) is:
//   - A boolean condition (BoolExpr tree) - equivalent to Siemens RLO building
//   - One or more actions (coils, set/reset, timers/counters)
//
// This IR is round-trip safe for LADDER ⇄ STL conversion.
//
// Key concepts:
//   - LADDER renders from BoolExpr as series/parallel wiring
//   - STL renders by emitting RLO-building instructions, then actions
//   - Timers/counters/edge detection are pass-through expression nodes
//     (they consume IN, produce Q on the stack)
//
// ============================================================================

namespace nir {

// ============ Constants ============

static const uint32_t NIR_NONE = 0xFFFFFFFF;

// Fixed sizes - no heap allocation
static const uint32_t NIR_MAX_SYMBOLS  = 512;
static const uint32_t NIR_MAX_EXPR     = 2048;
static const uint32_t NIR_MAX_CHILDREN = 4096;
static const uint32_t NIR_MAX_CALLS    = 256;
static const uint32_t NIR_MAX_ACTIONS  = 512;
static const uint32_t NIR_MAX_NETWORKS = 128;
static const uint32_t NIR_NAME_MAX     = 32;
static const uint32_t NIR_PRESET_POOL_MAX = 2048;

// ============ Symbol Types ============

enum SymbolType : uint8_t {
    SYM_UNKNOWN = 0,
    SYM_BOOL    = 1,    // Bit address (I0.0, Q0.0, M0.0, etc.)
    SYM_U8      = 2,
    SYM_U16     = 3,
    SYM_U32     = 4,
    SYM_I8      = 5,
    SYM_I16     = 6,
    SYM_I32     = 7,
    SYM_F32     = 8,
    SYM_TIMER   = 10,   // Timer instance (T0, T1, etc.)
    SYM_COUNTER = 11,   // Counter instance (C0, C1, etc.)
    SYM_EDGE    = 12,   // Edge state bit (auto-generated)
};

// ============ Expression Kinds ============

enum ExprKind : uint8_t {
    EXPR_NONE      = 0,
    EXPR_LEAF      = 1,   // Symbol reference
    EXPR_NOT       = 2,   // Logical NOT
    EXPR_AND       = 3,   // Logical AND (series in ladder)
    EXPR_OR        = 4,   // Logical OR (parallel in ladder)
    EXPR_XOR       = 5,   // Logical XOR
    EXPR_CALL_BOOL = 6,   // Pass-through block (TON/TOF/TP/CTU/CTD/FP/FN)
};

// ============ Call Operations (for EXPR_CALL_BOOL) ============

enum CallOp : uint8_t {
    CALL_NONE = 0,
    CALL_FP   = 1,   // Rising edge (positive)
    CALL_FN   = 2,   // Falling edge (negative)
    CALL_FX   = 3,   // Any edge (change detect) - VovkPLCRuntime extension
    CALL_TON  = 10,  // Timer On-Delay
    CALL_TOF  = 11,  // Timer Off-Delay
    CALL_TP   = 12,  // Timer Pulse
    CALL_CTU  = 20,  // Counter Up
    CALL_CTD  = 21,  // Counter Down
    CALL_CTUD = 22,  // Counter Up/Down
};

// ============ Action Kinds ============

enum ActionKind : uint8_t {
    ACT_NONE   = 0,
    ACT_ASSIGN = 1,   // = symbol (assign coil)
    ACT_SET    = 2,   // S symbol (set coil)
    ACT_RESET  = 3,   // R symbol (reset coil)
};

// ============ Preset Kinds ============

enum PresetKind : uint8_t {
    PRESET_NONE    = 0,
    PRESET_U32     = 1,   // Numeric value (ms for timers, count for counters)
    PRESET_SYMBOL  = 2,   // From symbol
    PRESET_LITERAL = 3,   // Time literal string "T#5s" stored in pool
};

// ============ Expression Flags ============

enum ExprFlags : uint8_t {
    EXPR_FLAG_NONE     = 0,
    EXPR_FLAG_NON_PURE = 1,  // Has side effects (timers, counters, edge detection)
    EXPR_FLAG_INVERTED = 2,  // For LEAF: normally closed contact (AN/ON)
};

// ============ Structures ============

// Symbol table entry
struct Symbol {
    char name[NIR_NAME_MAX];  // Null-terminated symbol name
    uint8_t type;             // SymbolType
    uint8_t flags;            // Reserved
    uint16_t address;         // Byte address (for memory-mapped symbols)
    uint8_t bit;              // Bit offset (for bit addresses)
    uint8_t reserved[3];
    
    void clear() {
        for (int i = 0; i < NIR_NAME_MAX; i++) name[i] = '\0';
        type = SYM_UNKNOWN;
        flags = 0;
        address = 0;
        bit = 0;
    }
};

// Expression node
struct Expr {
    uint8_t kind;       // ExprKind
    uint8_t flags;      // ExprFlags
    uint8_t reserved8a;
    uint8_t reserved8b;
    
    uint32_t a;         // Depends on kind (see below)
    uint32_t b;         // Depends on kind
    
    uint32_t child_ofs; // For AND/OR/XOR: first child index in children array
    uint16_t child_cnt; // Number of children
    uint16_t reserved16;
    
    // Field usage by kind:
    // EXPR_LEAF:      a = symbol_index, flags.INVERTED for normally closed
    // EXPR_NOT:       a = child_expr_index
    // EXPR_AND/OR/XOR: child_ofs, child_cnt
    // EXPR_CALL_BOOL: a = call_index (into calls array)
    
    void clear() {
        kind = EXPR_NONE;
        flags = 0;
        reserved8a = 0;
        reserved8b = 0;
        a = NIR_NONE;
        b = NIR_NONE;
        child_ofs = 0;
        child_cnt = 0;
        reserved16 = 0;
    }
};

// Child reference for AND/OR/XOR expressions
struct Child {
    uint32_t expr_index;
};

// Call parameters for pass-through blocks (EXPR_CALL_BOOL)
struct CallBool {
    uint8_t op;            // CallOp
    uint8_t preset_kind;   // PresetKind
    uint16_t preset_str_ofs; // Offset into preset pool (for PRESET_LITERAL)
    
    uint32_t instance_sym; // Timer/Counter/Edge instance symbol index
    uint32_t in_expr;      // Input expression index (IN, CU, CD)
    uint32_t aux_expr;     // Auxiliary input (R for CTU, LD for CTD, NIR_NONE if unused)
    
    uint32_t preset_u32;   // Preset value (ms for timers, PV for counters)
    uint32_t preset_sym;   // Preset symbol index (if PRESET_SYMBOL, else NIR_NONE)
    
    void clear() {
        op = CALL_NONE;
        preset_kind = PRESET_NONE;
        preset_str_ofs = 0;
        instance_sym = NIR_NONE;
        in_expr = NIR_NONE;
        aux_expr = NIR_NONE;
        preset_u32 = 0;
        preset_sym = NIR_NONE;
    }
};

// Action (coil operation)
struct Action {
    uint8_t kind;       // ActionKind
    uint8_t flags;      // Reserved
    uint16_t reserved16;
    
    uint32_t target_sym; // Target symbol index (coil address)
    
    void clear() {
        kind = ACT_NONE;
        flags = 0;
        reserved16 = 0;
        target_sym = NIR_NONE;
    }
};

// Network (rung)
struct Network {
    uint32_t condition_expr; // Index into exprs array (NIR_NONE = always true)
    uint32_t action_ofs;     // First action index in actions array
    uint16_t action_cnt;     // Number of actions
    uint16_t flags;          // Reserved
    
    // Optional metadata
    uint16_t comment_ofs;    // Offset into comment pool (0 = no comment)
    uint16_t comment_len;    // Comment length
    
    void clear() {
        condition_expr = NIR_NONE;
        action_ofs = 0;
        action_cnt = 0;
        flags = 0;
        comment_ofs = 0;
        comment_len = 0;
    }
};

// ============ Global IR Storage ============

struct Store {
    // Counts
    uint32_t sym_count;
    uint32_t expr_count;
    uint32_t child_count;
    uint32_t call_count;
    uint32_t action_count;
    uint32_t network_count;
    
    // Arenas
    Symbol   symbols[NIR_MAX_SYMBOLS];
    Expr     exprs[NIR_MAX_EXPR];
    Child    children[NIR_MAX_CHILDREN];
    CallBool calls[NIR_MAX_CALLS];
    Action   actions[NIR_MAX_ACTIONS];
    Network  networks[NIR_MAX_NETWORKS];
    
    // String pools
    uint16_t preset_pool_len;
    char     preset_pool[NIR_PRESET_POOL_MAX];
    
    // Error state
    bool has_error;
    char error_msg[256];
    
    void reset() {
        sym_count = 0;
        expr_count = 0;
        child_count = 0;
        call_count = 0;
        action_count = 0;
        network_count = 0;
        preset_pool_len = 0;
        has_error = false;
        error_msg[0] = '\0';
    }
    
    void setError(const char* msg) {
        if (has_error) return;
        has_error = true;
        int i = 0;
        while (msg[i] && i < 255) {
            error_msg[i] = msg[i];
            i++;
        }
        error_msg[i] = '\0';
    }
    
    // ============ Symbol Management ============
    
    uint32_t findSymbol(const char* name) {
        for (uint32_t i = 0; i < sym_count; i++) {
            bool match = true;
            for (int j = 0; j < NIR_NAME_MAX && match; j++) {
                if (symbols[i].name[j] != name[j]) match = false;
                if (name[j] == '\0') break;
            }
            if (match) return i;
        }
        return NIR_NONE;
    }
    
    uint32_t addSymbol(const char* name, uint8_t type = SYM_BOOL) {
        uint32_t existing = findSymbol(name);
        if (existing != NIR_NONE) return existing;
        
        if (sym_count >= NIR_MAX_SYMBOLS) {
            setError("Symbol table full");
            return NIR_NONE;
        }
        
        uint32_t idx = sym_count++;
        symbols[idx].clear();
        int i = 0;
        while (name[i] && i < NIR_NAME_MAX - 1) {
            symbols[idx].name[i] = name[i];
            i++;
        }
        symbols[idx].name[i] = '\0';
        symbols[idx].type = type;
        
        // Parse address from name (e.g., "I0.0" -> address=0, bit=0)
        parseAddress(name, symbols[idx]);
        
        return idx;
    }
    
    void parseAddress(const char* name, Symbol& sym) {
        // Detect symbol type from prefix
        char prefix = name[0];
        if (prefix >= 'a' && prefix <= 'z') prefix -= 32; // uppercase
        
        switch (prefix) {
            case 'I': case 'X': // Input
            case 'Q': case 'Y': // Output
            case 'M':           // Marker/Memory
            case 'S':           // System
            case 'C':           // Control or Counter
                sym.type = SYM_BOOL;
                break;
            case 'T':           // Timer
                sym.type = SYM_TIMER;
                break;
        }
        
        // Parse byte.bit format (e.g., "0.0" after prefix)
        int pos = 1;
        uint16_t byte_addr = 0;
        uint8_t bit_addr = 0;
        
        // Skip non-digits
        while (name[pos] && (name[pos] < '0' || name[pos] > '9')) pos++;
        
        // Parse byte address
        while (name[pos] >= '0' && name[pos] <= '9') {
            byte_addr = byte_addr * 10 + (name[pos] - '0');
            pos++;
        }
        
        // Check for dot separator
        if (name[pos] == '.') {
            pos++;
            // Parse bit address
            while (name[pos] >= '0' && name[pos] <= '9') {
                bit_addr = bit_addr * 10 + (name[pos] - '0');
                pos++;
            }
        }
        
        sym.address = byte_addr;
        sym.bit = bit_addr;
    }
    
    // ============ Expression Building ============
    
    uint32_t addLeaf(uint32_t sym_idx, bool inverted = false) {
        if (expr_count >= NIR_MAX_EXPR) {
            setError("Expression arena full");
            return NIR_NONE;
        }
        uint32_t idx = expr_count++;
        exprs[idx].clear();
        exprs[idx].kind = EXPR_LEAF;
        exprs[idx].a = sym_idx;
        if (inverted) exprs[idx].flags |= EXPR_FLAG_INVERTED;
        return idx;
    }
    
    uint32_t addNot(uint32_t child_expr) {
        if (expr_count >= NIR_MAX_EXPR) {
            setError("Expression arena full");
            return NIR_NONE;
        }
        uint32_t idx = expr_count++;
        exprs[idx].clear();
        exprs[idx].kind = EXPR_NOT;
        exprs[idx].a = child_expr;
        return idx;
    }
    
    uint32_t addAnd(uint32_t* children, uint16_t count) {
        if (expr_count >= NIR_MAX_EXPR || child_count + count > NIR_MAX_CHILDREN) {
            setError("Expression arena full");
            return NIR_NONE;
        }
        uint32_t idx = expr_count++;
        exprs[idx].clear();
        exprs[idx].kind = EXPR_AND;
        exprs[idx].child_ofs = child_count;
        exprs[idx].child_cnt = count;
        
        for (uint16_t i = 0; i < count; i++) {
            this->children[child_count++].expr_index = children[i];
        }
        return idx;
    }
    
    uint32_t addOr(uint32_t* children, uint16_t count) {
        if (expr_count >= NIR_MAX_EXPR || child_count + count > NIR_MAX_CHILDREN) {
            setError("Expression arena full");
            return NIR_NONE;
        }
        uint32_t idx = expr_count++;
        exprs[idx].clear();
        exprs[idx].kind = EXPR_OR;
        exprs[idx].child_ofs = child_count;
        exprs[idx].child_cnt = count;
        
        for (uint16_t i = 0; i < count; i++) {
            this->children[child_count++].expr_index = children[i];
        }
        return idx;
    }
    
    uint32_t addCallBool(CallOp op, uint32_t in_expr, uint32_t instance_sym,
                         uint32_t preset_u32 = 0, uint32_t aux_expr = NIR_NONE) {
        if (expr_count >= NIR_MAX_EXPR || call_count >= NIR_MAX_CALLS) {
            setError("Expression/call arena full");
            return NIR_NONE;
        }
        
        // Add call parameters
        uint32_t call_idx = call_count++;
        calls[call_idx].clear();
        calls[call_idx].op = op;
        calls[call_idx].in_expr = in_expr;
        calls[call_idx].instance_sym = instance_sym;
        calls[call_idx].preset_u32 = preset_u32;
        calls[call_idx].preset_kind = PRESET_U32;
        calls[call_idx].aux_expr = aux_expr;
        
        // Add expression node
        uint32_t expr_idx = expr_count++;
        exprs[expr_idx].clear();
        exprs[expr_idx].kind = EXPR_CALL_BOOL;
        exprs[expr_idx].flags = EXPR_FLAG_NON_PURE;
        exprs[expr_idx].a = call_idx;
        
        return expr_idx;
    }
    
    // ============ Action Building ============
    
    uint32_t addAction(ActionKind kind, uint32_t target_sym) {
        if (action_count >= NIR_MAX_ACTIONS) {
            setError("Action arena full");
            return NIR_NONE;
        }
        uint32_t idx = action_count++;
        actions[idx].clear();
        actions[idx].kind = kind;
        actions[idx].target_sym = target_sym;
        return idx;
    }
    
    // ============ Network Building ============
    
    uint32_t addNetwork(uint32_t condition_expr, uint32_t action_start, uint16_t action_count) {
        if (network_count >= NIR_MAX_NETWORKS) {
            setError("Network arena full");
            return NIR_NONE;
        }
        uint32_t idx = network_count++;
        networks[idx].clear();
        networks[idx].condition_expr = condition_expr;
        networks[idx].action_ofs = action_start;
        networks[idx].action_cnt = action_count;
        return idx;
    }
    
    // ============ Preset Pool ============
    
    uint16_t addPresetLiteral(const char* literal) {
        uint16_t start = preset_pool_len;
        int i = 0;
        while (literal[i] && preset_pool_len < NIR_PRESET_POOL_MAX - 1) {
            preset_pool[preset_pool_len++] = literal[i++];
        }
        preset_pool[preset_pool_len++] = '\0';
        return start;
    }
};

// Global store instance
static Store g_store;

} // namespace nir

// ============ WASM Exports ============

extern "C" {

WASM_EXPORT void nir_reset() {
    nir::g_store.reset();
}

WASM_EXPORT uint32_t nir_get_symbol_count()  { return nir::g_store.sym_count; }
WASM_EXPORT uint32_t nir_get_expr_count()    { return nir::g_store.expr_count; }
WASM_EXPORT uint32_t nir_get_child_count()   { return nir::g_store.child_count; }
WASM_EXPORT uint32_t nir_get_call_count()    { return nir::g_store.call_count; }
WASM_EXPORT uint32_t nir_get_action_count()  { return nir::g_store.action_count; }
WASM_EXPORT uint32_t nir_get_network_count() { return nir::g_store.network_count; }

WASM_EXPORT nir::Symbol*   nir_get_symbols_ptr()  { return nir::g_store.symbols; }
WASM_EXPORT nir::Expr*     nir_get_exprs_ptr()    { return nir::g_store.exprs; }
WASM_EXPORT nir::Child*    nir_get_children_ptr() { return nir::g_store.children; }
WASM_EXPORT nir::CallBool* nir_get_calls_ptr()    { return nir::g_store.calls; }
WASM_EXPORT nir::Action*   nir_get_actions_ptr()  { return nir::g_store.actions; }
WASM_EXPORT nir::Network*  nir_get_networks_ptr() { return nir::g_store.networks; }

WASM_EXPORT const char* nir_get_preset_pool_ptr() { return nir::g_store.preset_pool; }
WASM_EXPORT uint16_t    nir_get_preset_pool_len() { return nir::g_store.preset_pool_len; }

WASM_EXPORT bool        nir_has_error()     { return nir::g_store.has_error; }
WASM_EXPORT const char* nir_get_error()     { return nir::g_store.error_msg; }

// Struct size exports for JS memory access
WASM_EXPORT uint32_t nir_sizeof_symbol()   { return sizeof(nir::Symbol); }
WASM_EXPORT uint32_t nir_sizeof_expr()     { return sizeof(nir::Expr); }
WASM_EXPORT uint32_t nir_sizeof_child()    { return sizeof(nir::Child); }
WASM_EXPORT uint32_t nir_sizeof_call()     { return sizeof(nir::CallBool); }
WASM_EXPORT uint32_t nir_sizeof_action()   { return sizeof(nir::Action); }
WASM_EXPORT uint32_t nir_sizeof_network()  { return sizeof(nir::Network); }

} // extern "C"

#endif // __WASM__
