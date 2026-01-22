// ladder-compiler.h - Ladder Logic ⇄ STL Transpiler using Network IR
// Bidirectional conversion between Ladder Logic and Siemens-style STL
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

#include "network-ir.h"
#include "stl-compiler.h"

// ============================================================================
// Ladder JSON to Network IR Parser
// ============================================================================
//
// Parses JSON Ladder representation into Network IR.
// JSON format:
//   {
//     "rungs": [
//       {
//         "comment": "optional comment",
//         "elements": [
//           { "type": "contact", "address": "I0.0", "inverted": false, "trigger": "normal" },
//           { "type": "contact", "address": "I0.1", "inverted": true, "trigger": "rising" },
//           { "type": "coil", "address": "Q0.0", "inverted": false },
//           { "type": "coil_set", "address": "M0.0" },
//           { "type": "coil_rset", "address": "M0.1" }
//         ],
//         "branches": [  // optional OR branches
//           { "elements": [...] }
//         ]
//       }
//     ]
//   }
//
// Element types:
//   - contact:    Normally open contact (A operand)
//   - contact_nc: Normally closed contact (AN operand)
//   - coil:       Output coil (= operand)
//   - coil_set:   Set coil (S operand)
//   - coil_rset:  Reset coil (R operand)
//   - coil_n:     Negated coil (inverted =)
//   - tap:        RLO passthrough to next rung (VovkPLCRuntime extension)
//
// Contact properties:
//   - address:  Symbol address (I0.0, M0.0, etc.)
//   - inverted: true for normally closed (AN)
//   - trigger:  "normal", "rising" (FP), "falling" (FN), "change" (FX)
//
// ============================================================================

class LadderJSONToIRParser {
public:
    const char* source;
    int length;
    int pos;
    
    char error_msg[256];
    bool has_error;
    
    // Edge memory counter for auto-generated edge state bits
    // Start at bit 800 = M100.0 to avoid conflict with user memory
    int edge_mem_counter;
    
    LadderJSONToIRParser() { reset(); }
    
    void reset() {
        source = nullptr;
        length = 0;
        pos = 0;
        error_msg[0] = '\0';
        has_error = false;
        edge_mem_counter = 800; // Start at M100.0 (bit 800 = 100*8)
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
    
    // ============ JSON Lexer Helpers ============
    
    char peek(int offset = 0) {
        int p = pos + offset;
        return (p < length) ? source[p] : '\0';
    }
    
    char advance() {
        if (pos >= length) return '\0';
        return source[pos++];
    }
    
    void skipWhitespace() {
        while (pos < length) {
            char c = peek();
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') advance();
            else break;
        }
    }
    
    bool expect(char c) {
        skipWhitespace();
        if (peek() == c) {
            advance();
            return true;
        }
        return false;
    }
    
    bool strMatch(const char* s) {
        skipWhitespace();
        int i = 0;
        while (s[i]) {
            if (peek(i) != s[i]) return false;
            i++;
        }
        // Advance past matched string
        for (int j = 0; j < i; j++) advance();
        return true;
    }
    
    bool strEqI(const char* a, const char* b) {
        while (*a && *b) {
            char ca = (*a >= 'A' && *a <= 'Z') ? *a + 32 : *a;
            char cb = (*b >= 'A' && *b <= 'Z') ? *b + 32 : *b;
            if (ca != cb) return false;
            a++; b++;
        }
        return *a == *b;
    }
    
    // Read a JSON string value (expects opening quote already consumed or at current pos)
    bool readString(char* buf, int maxLen) {
        skipWhitespace();
        if (peek() != '"') {
            setError("Expected string");
            return false;
        }
        advance(); // Skip opening quote
        
        int i = 0;
        while (i < maxLen - 1) {
            char c = peek();
            if (c == '\0') {
                setError("Unterminated string");
                return false;
            }
            if (c == '"') {
                advance(); // Skip closing quote
                break;
            }
            if (c == '\\') {
                advance();
                c = peek();
                // Handle escape sequences
                if (c == 'n') c = '\n';
                else if (c == 't') c = '\t';
                else if (c == 'r') c = '\r';
            }
            buf[i++] = advance();
        }
        buf[i] = '\0';
        return true;
    }
    
    // Read a JSON boolean value
    bool readBool(bool& value) {
        skipWhitespace();
        if (strMatch("true")) {
            value = true;
            return true;
        }
        if (strMatch("false")) {
            value = false;
            return true;
        }
        setError("Expected boolean");
        return false;
    }
    
    // Read a JSON number as u32
    bool readU32(uint32_t& value) {
        skipWhitespace();
        if (peek() < '0' || peek() > '9') {
            setError("Expected number");
            return false;
        }
        value = 0;
        while (peek() >= '0' && peek() <= '9') {
            value = value * 10 + (advance() - '0');
        }
        return true;
    }
    
    // Skip a JSON value (string, number, object, array, bool, null)
    void skipValue() {
        skipWhitespace();
        char c = peek();
        
        if (c == '"') {
            // String
            advance();
            while (peek() && peek() != '"') {
                if (peek() == '\\') advance();
                advance();
            }
            if (peek() == '"') advance();
        } else if (c == '{') {
            // Object
            advance();
            int depth = 1;
            while (peek() && depth > 0) {
                if (peek() == '{') depth++;
                else if (peek() == '}') depth--;
                else if (peek() == '"') {
                    advance();
                    while (peek() && peek() != '"') {
                        if (peek() == '\\') advance();
                        advance();
                    }
                }
                advance();
            }
        } else if (c == '[') {
            // Array
            advance();
            int depth = 1;
            while (peek() && depth > 0) {
                if (peek() == '[') depth++;
                else if (peek() == ']') depth--;
                else if (peek() == '"') {
                    advance();
                    while (peek() && peek() != '"') {
                        if (peek() == '\\') advance();
                        advance();
                    }
                }
                advance();
            }
        } else if ((c >= '0' && c <= '9') || c == '-') {
            // Number
            while (peek() && ((peek() >= '0' && peek() <= '9') || peek() == '.' || peek() == '-' || peek() == 'e' || peek() == 'E')) {
                advance();
            }
        } else if (c == 't' || c == 'f' || c == 'n') {
            // true, false, null
            while (peek() && peek() >= 'a' && peek() <= 'z') advance();
        }
    }
    
    // ============ Ladder Element Parsing ============
    
    struct LadderElement {
        char type[16];       // contact, coil, coil_set, coil_rset, etc.
        char address[32];    // I0.0, Q0.0, M0.0, etc.
        bool inverted;       // Normally closed for contacts, negated for coils
        char trigger[16];    // normal, rising, falling
        uint32_t preset;     // Timer/Counter preset value
        char preset_str[32]; // Timer preset as literal string (e.g. T#5s)
        uint32_t precompiled_expr; // For nested structures like OR blocks
        
        void clear() {
            type[0] = '\0';
            address[0] = '\0';
            inverted = false;
            trigger[0] = 'n'; trigger[1] = 'o'; trigger[2] = 'r'; trigger[3] = 'm';
            trigger[4] = 'a'; trigger[5] = 'l'; trigger[6] = '\0';
            preset = 0;
            preset_str[0] = '\0';
            precompiled_expr = nir::NIR_NONE;
        }
    };
    
    // ============ Deferred Actions from Nested Branches ============
    // Actions (coils, TAPs) found inside OR branches need to be collected
    // with their condition and processed after the main network is built.
    
    static const int MAX_DEFERRED_ACTIONS = 32;
    
    struct DeferredAction {
        char type[16];      // coil, coil_set, coil_rset, coil_n, tap
        char address[32];   // Output address
        bool inverted;
        uint32_t condition; // NIR expression for when this action should execute
        
        void clear() {
            type[0] = '\0';
            address[0] = '\0';
            inverted = false;
            condition = nir::NIR_NONE;
        }
    };
    
    // Global collection for deferred actions during parsing
    DeferredAction deferred_actions[MAX_DEFERRED_ACTIONS];
    int deferred_action_count = 0;
    
    void clearDeferredActions() {
        deferred_action_count = 0;
    }
    
    void addDeferredAction(const char* type, const char* address, bool inverted, uint32_t condition) {
        if (deferred_action_count >= MAX_DEFERRED_ACTIONS) return;
        DeferredAction& da = deferred_actions[deferred_action_count++];
        da.clear();
        
        int i = 0;
        while (type[i] && i < 15) { da.type[i] = type[i]; i++; }
        da.type[i] = '\0';
        
        i = 0;
        while (address[i] && i < 31) { da.address[i] = address[i]; i++; }
        da.address[i] = '\0';
        
        da.inverted = inverted;
        da.condition = condition;
    };

    // Helper to generate unique edge memory address
    void generateEdgeAddress(char* edge_mem) {
        int byte_addr = edge_mem_counter / 8;
        int bit_addr = edge_mem_counter % 8;
        edge_mem_counter++;
        
        int idx = 0;
        edge_mem[idx++] = 'M';
        int hundreds = byte_addr / 100;
        int tens = (byte_addr / 10) % 10;
        int ones = byte_addr % 10;
        if (hundreds > 0) edge_mem[idx++] = '0' + hundreds;
        if (hundreds > 0 || tens > 0) edge_mem[idx++] = '0' + tens;
        edge_mem[idx++] = '0' + ones;
        edge_mem[idx++] = '.';
        edge_mem[idx++] = '0' + bit_addr;
        edge_mem[idx] = '\0';
    }

    // Convert a single element to an expression (handles precompiled nested structures)
    // in_expr is the accumulated input expression for timers/counters (can be NIR_NONE)
    uint32_t elementToExpr(LadderElement& elem, uint32_t in_expr = nir::NIR_NONE) {
        if (elem.precompiled_expr != nir::NIR_NONE) return elem.precompiled_expr;
        
        // Handle Timers (multiple name formats supported)
        if (strEqI(elem.type, "timer_ton") || strEqI(elem.type, "timer_on") ||
            strEqI(elem.type, "timer_tof") || strEqI(elem.type, "timer_off") ||
            strEqI(elem.type, "timer_tp") || strEqI(elem.type, "timer_pulse")) {
            nir::CallOp op = nir::CALL_TON;
            if (strEqI(elem.type, "timer_tof") || strEqI(elem.type, "timer_off")) op = nir::CALL_TOF;
            else if (strEqI(elem.type, "timer_tp") || strEqI(elem.type, "timer_pulse")) op = nir::CALL_TP;
            
            uint32_t sym_idx = nir::g_store.addSymbol(elem.address, nir::SYM_TIMER);
            if (sym_idx == nir::NIR_NONE) return nir::NIR_NONE;
            
            uint32_t call_expr;
            if (elem.preset_str[0] != '\0') {
                call_expr = nir::g_store.addCallBoolLiteral(op, in_expr, sym_idx, elem.preset_str);
            } else {
                call_expr = nir::g_store.addCallBool(op, in_expr, sym_idx, elem.preset);
            }
            return call_expr;
        }
        
        // Handle Counters (multiple name formats supported)
        if (strEqI(elem.type, "counter_u") || strEqI(elem.type, "counter_up") ||
            strEqI(elem.type, "counter_d") || strEqI(elem.type, "counter_down")) {
            nir::CallOp op = (strEqI(elem.type, "counter_u") || strEqI(elem.type, "counter_up")) 
                            ? nir::CALL_CTU : nir::CALL_CTD;
            
            uint32_t sym_idx = nir::g_store.addSymbol(elem.address, nir::SYM_COUNTER);
            if (sym_idx == nir::NIR_NONE) return nir::NIR_NONE;
            
            uint32_t call_expr = nir::g_store.addCallBool(op, in_expr, sym_idx, elem.preset);
            return call_expr;
        }
        
        // Handle Contacts
        bool inverted = elem.inverted || strEqI(elem.type, "contact_nc");
        uint32_t sym_idx = nir::g_store.addSymbol(elem.address);
        if (sym_idx == nir::NIR_NONE) return nir::NIR_NONE;
        
        uint32_t leaf_expr = nir::g_store.addLeaf(sym_idx, inverted);
        
        // Handle edge detection
        if (strEqI(elem.trigger, "rising") || strEqI(elem.trigger, "positive")) {
            char edge_mem[16];
            generateEdgeAddress(edge_mem);
            uint32_t edge_sym = nir::g_store.addSymbol(edge_mem, nir::SYM_EDGE);
            leaf_expr = nir::g_store.addCallBool(nir::CALL_FP, leaf_expr, edge_sym);
        } else if (strEqI(elem.trigger, "falling") || strEqI(elem.trigger, "negative")) {
            char edge_mem[16];
            generateEdgeAddress(edge_mem);
            uint32_t edge_sym = nir::g_store.addSymbol(edge_mem, nir::SYM_EDGE);
            leaf_expr = nir::g_store.addCallBool(nir::CALL_FN, leaf_expr, edge_sym);
        } else if (strEqI(elem.trigger, "change") || strEqI(elem.trigger, "any")) {
            char edge_mem[16];
            generateEdgeAddress(edge_mem);
            uint32_t edge_sym = nir::g_store.addSymbol(edge_mem, nir::SYM_EDGE);
            leaf_expr = nir::g_store.addCallBool(nir::CALL_FX, leaf_expr, edge_sym);
        }
        return leaf_expr;
    }

    // Check if element type is a timer or counter
    bool isTimerOrCounter(const char* type) {
        return strEqI(type, "timer_ton") || strEqI(type, "timer_on") ||
               strEqI(type, "timer_tof") || strEqI(type, "timer_off") ||
               strEqI(type, "timer_tp") || strEqI(type, "timer_pulse") ||
               strEqI(type, "counter_u") || strEqI(type, "counter_up") ||
               strEqI(type, "counter_d") || strEqI(type, "counter_down");
    }
    
    // Check if element type is a coil
    bool isCoil(const char* type) {
        return strEqI(type, "coil") || strEqI(type, "coil_set") ||
               strEqI(type, "coil_rset") || strEqI(type, "coil_n");
    }

    // Structure to hold parsed branch info (coils, TAPs, etc.)
    struct BranchParseResult {
        uint32_t condition_expr;
        bool has_coils;
        bool has_tap;
        
        // Collected coils
        static const int MAX_BRANCH_COILS = 8;
        struct CoilInfo {
            char type[16];
            char address[32];
            bool inverted;
        } coils[MAX_BRANCH_COILS];
        int coil_count;
        
        void clear() {
            condition_expr = nir::NIR_NONE;
            has_coils = false;
            has_tap = false;
            coil_count = 0;
        }
    };

    // Parse elements array to expression AND collect coils/TAPs for compound branch detection
    bool parseElementsArrayWithActions(BranchParseResult& result) {
        result.clear();
        
        if (!expect('[')) { setError("Expected elements array"); return false; }
        
        uint32_t children[32];
        int count = 0;
        uint32_t accumulated_expr = nir::NIR_NONE;
        
        while (peek() != ']' && peek() != '\0') {
            if (count >= 32) { setError("Too many elements in nested block"); return false; }
            
            LadderElement elem;
            if (!parseElement(elem)) return false;
            
            // Handle coils - collect them for compound branch
            if (isCoil(elem.type)) {
                if (result.coil_count < BranchParseResult::MAX_BRANCH_COILS) {
                    BranchParseResult::CoilInfo& ci = result.coils[result.coil_count++];
                    int i = 0;
                    while (elem.type[i] && i < 15) { ci.type[i] = elem.type[i]; i++; }
                    ci.type[i] = '\0';
                    i = 0;
                    while (elem.address[i] && i < 31) { ci.address[i] = elem.address[i]; i++; }
                    ci.address[i] = '\0';
                    ci.inverted = elem.inverted;
                    result.has_coils = true;
                }
                skipWhitespace();
                if (peek() == ',') advance();
                continue;
            }
            
            // Handle TAP - mark branch as having TAP
            if (strEqI(elem.type, "tap")) {
                result.has_tap = true;
                skipWhitespace();
                if (peek() == ',') advance();
                continue;
            }
            
            // Handle contacts, OR blocks, and precompiled expressions
            if (strEqI(elem.type, "contact") || strEqI(elem.type, "contact_no") ||
                strEqI(elem.type, "contact_nc") || 
                strEqI(elem.type, "or") || elem.precompiled_expr != nir::NIR_NONE) {
                uint32_t e = elementToExpr(elem);
                if (e != nir::NIR_NONE) children[count++] = e;
            }
            // Handle timers and counters
            else if (isTimerOrCounter(elem.type)) {
                if (count > 0) {
                    if (count == 1) accumulated_expr = children[0];
                    else accumulated_expr = nir::g_store.addAnd(children, count);
                    count = 0;
                }
                
                uint32_t e = elementToExpr(elem, accumulated_expr);
                if (e != nir::NIR_NONE) children[count++] = e;
                accumulated_expr = nir::NIR_NONE;
            }
            
            skipWhitespace();
            if (peek() == ',') advance();
        }
        
        if (!expect(']')) { setError("Expected ] close elements"); return false; }
        
        if (count == 0) result.condition_expr = nir::NIR_NONE;
        else if (count == 1) result.condition_expr = children[0];
        else result.condition_expr = nir::g_store.addAnd(children, count);
        return true;
    }
    
    // Simple version that doesn't collect actions (for backward compat)
    bool parseElementsArrayToExpr(uint32_t& out_expr) {
        BranchParseResult result;
        if (!parseElementsArrayWithActions(result)) return false;
        out_expr = result.condition_expr;
        return true;
    }
    
    // Parse branches array - creates compound expressions for branches with coils/TAPs
    bool parseRecursiveBranches(uint32_t& out_expr) {
        if (!expect('[')) { setError("Expected branches array"); return false; }
        
        uint32_t children[16];
        int count = 0;
        
        while (peek() != ']' && peek() != '\0') {
            if (count >= 16) { setError("Too many branches"); return false; }
            
            BranchParseResult br_result;
            br_result.clear();
            
            skipWhitespace();
            // Support both formats:
            // 1. [ [...elements...], [...elements...] ]  (arrays directly)
            // 2. [ {elements: [...]}, {elements: [...]} ]  (objects with elements)
            if (peek() == '[') {
                // Branch is an array of elements directly
                if (!parseElementsArrayWithActions(br_result)) return false;
            } else if (peek() == '{') {
                // Branch is an object with elements key
                advance(); // skip '{'
            
                while (peek() != '}' && peek() != '\0') {
                    char key[32];
                    if (!readString(key, sizeof(key))) return false;
                    if (!expect(':')) { setError("Expected :"); return false; }
                    
                    if (strEqI(key, "elements")) {
                        if (!parseElementsArrayWithActions(br_result)) return false;
                    } else skipValue();
                    
                    skipWhitespace();
                    if (peek() == ',') advance();
                }
                if (!expect('}')) { setError("Expected } close branch"); return false; }
            } else {
                setError("Expected branch ([ or {)");
                return false;
            }
            
            // If branch has coils or TAP, create a compound expression
            if (br_result.has_coils || br_result.has_tap) {
                uint32_t comp_expr = nir::g_store.addCompoundBranch(br_result.condition_expr, br_result.has_tap);
                if (comp_expr == nir::NIR_NONE) return false;
                
                // Get the compound index from the expression
                nir::Expr& expr = nir::g_store.exprs[comp_expr];
                uint32_t comp_idx = expr.a;
                
                // Add inline actions
                for (int i = 0; i < br_result.coil_count; i++) {
                    BranchParseResult::CoilInfo& ci = br_result.coils[i];
                    nir::ActionKind kind = nir::ACT_ASSIGN;
                    if (strEqI(ci.type, "coil_set")) kind = nir::ACT_SET;
                    else if (strEqI(ci.type, "coil_rset")) kind = nir::ACT_RESET;
                    
                    uint32_t target_sym = nir::g_store.addSymbol(ci.address);
                    if (target_sym == nir::NIR_NONE) return false;
                    
                    nir::g_store.addCompoundAction(comp_idx, kind, target_sym);
                }
                
                children[count++] = comp_expr;
            } else if (br_result.condition_expr != nir::NIR_NONE) {
                children[count++] = br_result.condition_expr;
            }
            
            skipWhitespace();
            if (peek() == ',') advance();
        }
        
        if (!expect(']')) { setError("Expected ] close branches"); return false; }
        
        if (count == 0) out_expr = nir::NIR_NONE;
        else if (count == 1) out_expr = children[0];
        else out_expr = nir::g_store.addOr(children, count);
        return true;
    }
    
    bool parseElement(LadderElement& elem) {
        elem.clear();
        
        if (!expect('{')) {
            setError("Expected element object");
            return false;
        }
        
        while (peek() != '}' && peek() != '\0') {
            char key[32];
            if (!readString(key, sizeof(key))) return false;
            
            if (!expect(':')) {
                setError("Expected ':' after key");
                return false;
            }
            
            if (strEqI(key, "type")) {
                if (!readString(elem.type, sizeof(elem.type))) return false;
            } else if (strEqI(key, "branches")) {
                // Recursive OR branches within an element
                if (!parseRecursiveBranches(elem.precompiled_expr)) return false;
            } else if (strEqI(key, "address")) {
                if (!readString(elem.address, sizeof(elem.address))) return false;
            } else if (strEqI(key, "inverted")) {
                if (!readBool(elem.inverted)) return false;
            } else if (strEqI(key, "trigger")) {
                if (!readString(elem.trigger, sizeof(elem.trigger))) return false;
            } else if (strEqI(key, "preset")) {
                skipWhitespace();
                if (peek() == '"') {
                    if (!readString(elem.preset_str, sizeof(elem.preset_str))) return false;
                } else {
                    if (!readU32(elem.preset)) return false;
                }
            } else {
                // Skip unknown property
                skipValue();
            }
            
            // Skip comma if present
            skipWhitespace();
            if (peek() == ',') advance();
        }
        
        if (!expect('}')) {
            setError("Expected '}' to close element");
            return false;
        }
        
        return true;
    }
    
    // ============ Rung Parsing ============
    
    // Branches structure for OR logic
    static const int MAX_BRANCHES = 16;
    static const int MAX_BRANCH_ELEMENTS = 32;
    
    struct Branch {
        LadderElement elements[MAX_BRANCH_ELEMENTS];
        int element_count;
        
        void clear() { element_count = 0; }
    };
    
    bool parseBranch(Branch& branch) {
        branch.clear();
        
        if (!expect('{')) {
            setError("Expected branch object");
            return false;
        }
        
        while (peek() != '}' && peek() != '\0') {
            char key[32];
            if (!readString(key, sizeof(key))) return false;
            
            if (!expect(':')) {
                setError("Expected ':' after key");
                return false;
            }
            
            if (strEqI(key, "elements")) {
                if (!expect('[')) {
                    setError("Expected elements array in branch");
                    return false;
                }
                
                while (peek() != ']' && peek() != '\0') {
                    if (branch.element_count >= MAX_BRANCH_ELEMENTS) {
                        setError("Too many elements in branch");
                        return false;
                    }
                    
                    if (!parseElement(branch.elements[branch.element_count])) return false;
                    branch.element_count++;
                    
                    skipWhitespace();
                    if (peek() == ',') advance();
                }
                
                if (!expect(']')) {
                    setError("Expected ']' to close branch elements");
                    return false;
                }
            } else {
                skipValue();
            }
            
            skipWhitespace();
            if (peek() == ',') advance();
        }
        
        if (!expect('}')) {
            setError("Expected '}' to close branch");
            return false;
        }
        
        return true;
    }
    
    bool parseRung() {
        if (!expect('{')) {
            setError("Expected rung object");
            return false;
        }
        
        // Clear deferred actions from previous rung
        clearDeferredActions();
        
        // Collect elements for this rung
        static const int MAX_ELEMENTS = 64;
        LadderElement elements[MAX_ELEMENTS];
        int element_count = 0;
        
        // Branches for parallel (OR) logic
        Branch branches[MAX_BRANCHES];
        int branch_count = 0;
        
        while (peek() != '}' && peek() != '\0') {
            char key[32];
            if (!readString(key, sizeof(key))) return false;
            
            if (!expect(':')) {
                setError("Expected ':' after key");
                return false;
            }
            
            if (strEqI(key, "elements")) {
                // Parse elements array
                if (!expect('[')) {
                    setError("Expected elements array");
                    return false;
                }
                
                while (peek() != ']' && peek() != '\0') {
                    if (element_count >= MAX_ELEMENTS) {
                        setError("Too many elements in rung");
                        return false;
                    }
                    
                    // Check if this is a nested array (row of elements)
                    skipWhitespace();
                    if (peek() == '[') {
                        // Parse nested array as a row of elements
                        advance(); // skip '['
                        while (peek() != ']' && peek() != '\0') {
                            if (element_count >= MAX_ELEMENTS) {
                                setError("Too many elements in nested row");
                                return false;
                            }
                            
                            if (!parseElement(elements[element_count])) return false;
                            element_count++;
                            
                            skipWhitespace();
                            if (peek() == ',') advance();
                        }
                        if (!expect(']')) {
                            setError("Expected ']' to close nested row");
                            return false;
                        }
                    } else {
                        // Parse single element
                        if (!parseElement(elements[element_count])) return false;
                        element_count++;
                    }
                    
                    skipWhitespace();
                    if (peek() == ',') advance();
                }
                
                if (!expect(']')) {
                    setError("Expected ']' to close elements");
                    return false;
                }
            } else if (strEqI(key, "segments")) {
                // Parse segments array - each segment has elements
                if (!expect('[')) {
                    setError("Expected segments array");
                    return false;
                }
                
                while (peek() != ']' && peek() != '\0') {
                    // Each segment is an object with elements
                    if (!expect('{')) {
                        setError("Expected segment object");
                        return false;
                    }
                    
                    while (peek() != '}' && peek() != '\0') {
                        char segKey[32];
                        if (!readString(segKey, sizeof(segKey))) return false;
                        if (!expect(':')) { setError("Expected ':'"); return false; }
                        
                        if (strEqI(segKey, "elements")) {
                            // Parse elements array within segment
                            if (!expect('[')) {
                                setError("Expected elements array in segment");
                                return false;
                            }
                            
                            while (peek() != ']' && peek() != '\0') {
                                // Check if nested array
                                skipWhitespace();
                                if (peek() == '[') {
                                    advance();
                                    while (peek() != ']' && peek() != '\0') {
                                        if (element_count >= MAX_ELEMENTS) {
                                            setError("Too many elements");
                                            return false;
                                        }
                                        if (!parseElement(elements[element_count])) return false;
                                        element_count++;
                                        skipWhitespace();
                                        if (peek() == ',') advance();
                                    }
                                    if (!expect(']')) {
                                        setError("Expected ']' close nested");
                                        return false;
                                    }
                                } else {
                                    if (element_count >= MAX_ELEMENTS) {
                                        setError("Too many elements");
                                        return false;
                                    }
                                    if (!parseElement(elements[element_count])) return false;
                                    element_count++;
                                }
                                skipWhitespace();
                                if (peek() == ',') advance();
                            }
                            
                            if (!expect(']')) {
                                setError("Expected ']' close segment elements");
                                return false;
                            }
                        } else {
                            skipValue();
                        }
                        
                        skipWhitespace();
                        if (peek() == ',') advance();
                    }
                    
                    if (!expect('}')) {
                        setError("Expected '}' close segment");
                        return false;
                    }
                    
                    skipWhitespace();
                    if (peek() == ',') advance();
                }
                
                if (!expect(']')) {
                    setError("Expected ']' close segments");
                    return false;
                }
            } else if (strEqI(key, "comment")) {
                // Skip comment for now
                skipValue();
            } else if (strEqI(key, "branches")) {
                // Parse branches array for OR logic
                if (!expect('[')) {
                    setError("Expected branches array");
                    return false;
                }
                
                while (peek() != ']' && peek() != '\0') {
                    if (branch_count >= MAX_BRANCHES) {
                        setError("Too many branches in rung");
                        return false;
                    }
                    
                    if (!parseBranch(branches[branch_count])) return false;
                    branch_count++;
                    
                    skipWhitespace();
                    if (peek() == ',') advance();
                }
                
                if (!expect(']')) {
                    setError("Expected ']' to close branches");
                    return false;
                }
            } else {
                skipValue();
            }
            
            skipWhitespace();
            if (peek() == ',') advance();
        }
        
        if (!expect('}')) {
            setError("Expected '}' to close rung");
            return false;
        }
        
        // Now build Network IR from elements and branches
        return buildNetworkFromElements(elements, element_count, branches, branch_count);
    }

    // Build logic expression processing contacts and function blocks (timers/counters)
    uint32_t buildLogicExpression(LadderElement* elements, int count) {
        if (count == 0) return nir::NIR_NONE;
        
        static const int MAX_AND_CHILDREN = 64;
        uint32_t and_children[MAX_AND_CHILDREN];
        int and_count = 0;
        
        for (int i = 0; i < count; i++) {
            LadderElement& elem = elements[i];
            
            // Skip coils
            if (strEqI(elem.type, "coil") || strEqI(elem.type, "coil_set") || 
                strEqI(elem.type, "coil_rset") || strEqI(elem.type, "coil_n")) {
                continue;
            }
            
            // Handle elements with precompiled expressions (nested OR blocks)
            if (elem.precompiled_expr != nir::NIR_NONE || strEqI(elem.type, "or")) {
                uint32_t expr = elementToExpr(elem);
                if (expr != nir::NIR_NONE && and_count < MAX_AND_CHILDREN) {
                    and_children[and_count++] = expr;
                }
                continue;
            }
            
            // Handle Contacts
            if (strEqI(elem.type, "contact") || strEqI(elem.type, "contact_nc")) {
                bool inverted = elem.inverted || strEqI(elem.type, "contact_nc");
                
                uint32_t sym_idx = nir::g_store.addSymbol(elem.address);
                if (sym_idx == nir::NIR_NONE) return nir::NIR_NONE;
                
                uint32_t leaf_expr = nir::g_store.addLeaf(sym_idx, inverted);
                if (leaf_expr == nir::NIR_NONE) return nir::NIR_NONE;
                
                // Handle edge detection
                if (strEqI(elem.trigger, "rising") || strEqI(elem.trigger, "positive")) {
                    char edge_mem[16];
                    generateEdgeAddress(edge_mem);
                    uint32_t edge_sym = nir::g_store.addSymbol(edge_mem, nir::SYM_EDGE);
                    leaf_expr = nir::g_store.addCallBool(nir::CALL_FP, leaf_expr, edge_sym);
                } else if (strEqI(elem.trigger, "falling") || strEqI(elem.trigger, "negative")) {
                    char edge_mem[16];
                    generateEdgeAddress(edge_mem);
                    uint32_t edge_sym = nir::g_store.addSymbol(edge_mem, nir::SYM_EDGE);
                    leaf_expr = nir::g_store.addCallBool(nir::CALL_FN, leaf_expr, edge_sym);
                } else if (strEqI(elem.trigger, "change") || strEqI(elem.trigger, "any")) {
                    char edge_mem[16];
                    generateEdgeAddress(edge_mem);
                    uint32_t edge_sym = nir::g_store.addSymbol(edge_mem, nir::SYM_EDGE);
                    leaf_expr = nir::g_store.addCallBool(nir::CALL_FX, leaf_expr, edge_sym);
                }
                
                if (and_count < MAX_AND_CHILDREN) {
                    and_children[and_count++] = leaf_expr;
                }
            }
            // Handle Timers/Counters
            else if (strEqI(elem.type, "timer_ton") || strEqI(elem.type, "timer_on") ||
                     strEqI(elem.type, "timer_tof") || strEqI(elem.type, "timer_off") ||
                     strEqI(elem.type, "timer_tp") || strEqI(elem.type, "timer_pulse") ||
                     strEqI(elem.type, "counter_u") || strEqI(elem.type, "counter_up") ||
                     strEqI(elem.type, "counter_d") || strEqI(elem.type, "counter_down")) {
                
                // Collapse current AND terms to get the Input Expression
                uint32_t in_expr = nir::NIR_NONE;
                if (and_count == 1) {
                    in_expr = and_children[0];
                } else if (and_count > 1) {
                    in_expr = nir::g_store.addAnd(and_children, and_count);
                }
                
                // Reset AND accumulation
                and_count = 0;
                
                nir::CallOp op = nir::CALL_NONE;
                if (strEqI(elem.type, "timer_ton") || strEqI(elem.type, "timer_on")) op = nir::CALL_TON;
                else if (strEqI(elem.type, "timer_tof") || strEqI(elem.type, "timer_off")) op = nir::CALL_TOF;
                else if (strEqI(elem.type, "timer_tp") || strEqI(elem.type, "timer_pulse")) op = nir::CALL_TP;
                else if (strEqI(elem.type, "counter_u") || strEqI(elem.type, "counter_up")) op = nir::CALL_CTU;
                else if (strEqI(elem.type, "counter_d") || strEqI(elem.type, "counter_down")) op = nir::CALL_CTD;

                uint32_t sym_idx = nir::g_store.addSymbol(elem.address, (op >= nir::CALL_CTU) ? nir::SYM_COUNTER : nir::SYM_TIMER);
                
                uint32_t call_expr;
                if (elem.preset_str[0] != '\0') {
                    call_expr = nir::g_store.addCallBoolLiteral(op, in_expr, sym_idx, elem.preset_str);
                } else {
                    call_expr = nir::g_store.addCallBool(op, in_expr, sym_idx, elem.preset);
                }
                
                if (and_count < MAX_AND_CHILDREN) {
                    and_children[and_count++] = call_expr;
                }
            }
        }
        
        if (and_count == 1) return and_children[0];
        if (and_count > 1) return nir::g_store.addAnd(and_children, and_count);
        return nir::NIR_NONE;
    }
    
    bool buildNetworkFromElements(LadderElement* elements, int count, Branch* branches, int branch_count) {
        if (count == 0 && branch_count == 0) return true;
        
        // TAP splits elements into segments. Each segment before a TAP becomes its own network
        // with a TAP action to pass RLO forward. The last segment doesn't need TAP.
        //
        // Example: [contact A, coil X, tap, contact B, coil Y]
        // Becomes: Network 1: A -> X (with TAP action)
        //          Network 2: B -> Y (RLO continues from Network 1)
        //
        // We process segments by finding TAP elements
        
        // Find TAP positions
        int tap_positions[16];
        int tap_count = 0;
        for (int i = 0; i < count && tap_count < 16; i++) {
            if (strEqI(elements[i].type, "tap")) {
                tap_positions[tap_count++] = i;
            }
        }
        
        // If no TAPs, process normally (old behavior)
        if (tap_count == 0) {
            return buildNetworkSegment(elements, count, branches, branch_count, false);
        }
        
        // Process each segment
        int segment_start = 0;
        for (int t = 0; t <= tap_count; t++) {
            int segment_end = (t < tap_count) ? tap_positions[t] : count;
            int segment_len = segment_end - segment_start;
            
            if (segment_len > 0) {
                // For segments before the last, add TAP action
                bool add_tap = (t < tap_count);
                
                // Build this segment (only pass branches to first segment)
                Branch* seg_branches = (t == 0) ? branches : nullptr;
                int seg_branch_count = (t == 0) ? branch_count : 0;
                
                if (!buildNetworkSegment(&elements[segment_start], segment_len, seg_branches, seg_branch_count, add_tap)) {
                    return false;
                }
            }
            
            segment_start = segment_end + 1; // Skip the TAP element itself
        }
        
        return true;
    }
    
    bool buildNetworkSegment(LadderElement* elements, int count, Branch* branches, int branch_count, bool add_tap) {
        if (count == 0 && branch_count == 0 && !add_tap) return true;
        
        // Identify coils for action generation
        static const int MAX_COILS = 16;
        int coil_indices[MAX_COILS];
        int coil_count = 0;
        
        for (int i = 0; i < count; i++) {
            const char* type = elements[i].type;
            if (strEqI(type, "coil") || strEqI(type, "coil_set") || 
                strEqI(type, "coil_rset") || strEqI(type, "coil_n")) {
                if (coil_count < MAX_COILS) {
                    coil_indices[coil_count++] = i;
                }
            }
            // TAP elements are already handled by buildNetworkFromElements
        }
        
        // Build main branch logic expression
        uint32_t main_branch_expr = buildLogicExpression(elements, count);
        
        uint32_t condition_expr = nir::NIR_NONE;
        
        // If we have branches, build OR expression
        if (branch_count > 0) {
            uint32_t or_children[MAX_BRANCHES + 1];
            int or_count = 0;
            
            // Main elements form the first branch
            if (main_branch_expr != nir::NIR_NONE) {
                or_children[or_count++] = main_branch_expr;
            }
            
            // Add each branch as an OR alternative
            for (int b = 0; b < branch_count && or_count < MAX_BRANCHES + 1; b++) {
                Branch& br = branches[b];
                // Build this branch's expression
                uint32_t branch_expr = buildLogicExpression(br.elements, br.element_count);
                if (branch_expr != nir::NIR_NONE) {
                    or_children[or_count++] = branch_expr;
                }
            }
            
            // Create OR expression
            if (or_count == 1) {
                condition_expr = or_children[0];
            } else if (or_count > 1) {
                condition_expr = nir::g_store.addOr(or_children, or_count);
            }
        } else {
            condition_expr = main_branch_expr;
        }
        
        // Build actions from coils
        uint32_t action_start = nir::g_store.action_count;
        uint16_t action_cnt = 0;
        
        for (int i = 0; i < coil_count; i++) {
            LadderElement& elem = elements[coil_indices[i]];
            
            nir::ActionKind kind = nir::ACT_ASSIGN;
            if (strEqI(elem.type, "coil_set")) {
                kind = nir::ACT_SET;
            } else if (strEqI(elem.type, "coil_rset")) {
                kind = nir::ACT_RESET;
            }
            
            uint32_t target_sym = nir::g_store.addSymbol(elem.address);
            if (target_sym == nir::NIR_NONE) return false;
            
            uint32_t action_idx = nir::g_store.addAction(kind, target_sym);
            if (action_idx == nir::NIR_NONE) return false;
            action_cnt++;
        }
        
        // Add TAP action if requested (passthrough RLO to next segment/network)
        if (add_tap) {
            uint32_t action_idx = nir::g_store.addAction(nir::ACT_TAP, nir::NIR_NONE);
            if (action_idx == nir::NIR_NONE) return false;
            action_cnt++;
        }
        
        // NOTE: Deferred TAPs from nested branches do NOT modify condition_expr.
        // The branch expression already includes the TAP condition as part of the OR.
        // TAP inside a branch just marks "this is what I contribute to the OR".
        
        // Create network
        if (condition_expr != nir::NIR_NONE || action_cnt > 0) {
            nir::g_store.addNetwork(condition_expr, action_start, action_cnt);
        }
        
        // Process deferred coil actions - create separate networks for each
        for (int i = 0; i < deferred_action_count; i++) {
            DeferredAction& da = deferred_actions[i];
            
            // Skip TAPs - they don't generate separate networks
            // (the branch expression already includes the TAP condition)
            if (strEqI(da.type, "tap")) continue;
            
            // Create a network for this deferred coil
            nir::ActionKind kind = nir::ACT_ASSIGN;
            if (strEqI(da.type, "coil_set")) {
                kind = nir::ACT_SET;
            } else if (strEqI(da.type, "coil_rset")) {
                kind = nir::ACT_RESET;
            }
            
            uint32_t target_sym = nir::g_store.addSymbol(da.address);
            if (target_sym == nir::NIR_NONE) continue;
            
            uint32_t def_action_start = nir::g_store.action_count;
            uint32_t action_idx = nir::g_store.addAction(kind, target_sym);
            if (action_idx == nir::NIR_NONE) continue;
            
            // Use the deferred condition for this action
            nir::g_store.addNetwork(da.condition, def_action_start, 1);
        }
        
        // Clear deferred actions after processing
        clearDeferredActions();
        
        return true;
    }
    
    // ============ Main Parser ============
    
    bool parse(const char* src, int len) {
        reset();
        source = src;
        length = len;
        
        nir::g_store.reset();
        
        // Expect root object
        if (!expect('{')) {
            setError("Expected JSON object");
            return false;
        }
        
        while (peek() != '}' && peek() != '\0') {
            char key[32];
            if (!readString(key, sizeof(key))) return false;
            
            if (!expect(':')) {
                setError("Expected ':' after key");
                return false;
            }
            
            if (strEqI(key, "rungs") || strEqI(key, "networks")) {
                // Parse rungs/networks array
                if (!expect('[')) {
                    setError("Expected rungs/networks array");
                    return false;
                }
                
                while (peek() != ']' && peek() != '\0') {
                    if (!parseRung()) return false;
                    
                    skipWhitespace();
                    if (peek() == ',') advance();
                }
                
                if (!expect(']')) {
                    setError("Expected ']' to close rungs");
                    return false;
                }
            } else {
                // Skip unknown property
                skipValue();
            }
            
            skipWhitespace();
            if (peek() == ',') advance();
        }
        
        if (!expect('}')) {
            setError("Expected '}' to close root object");
            return false;
        }
        
        return !has_error;
    }
};


// ============================================================================
// STL to Network IR Parser
// ============================================================================
//
// Parses Siemens-style STL into Network IR representation.
// Supports: A, AN, O, ON, X, XN, NOT, SET, CLR, =, S, R, FP, FN
//           TON, TOF, TP, CTU, CTD and nesting with O( ... )
//
// ============================================================================

class STLToIRParser {
public:
    const char* source;
    int length;
    int pos;
    int line;
    int col;
    
    // Output buffer for error messages
    char error_msg[256];
    bool has_error;
    
    // RLO expression stack for nested logic
    static const int MAX_EXPR_STACK = 64;
    uint32_t expr_stack[MAX_EXPR_STACK];
    int expr_stack_top;
    
    // OR branch tracking
    static const int MAX_OR_BRANCHES = 32;
    uint32_t or_branches[MAX_OR_BRANCHES];
    int or_branch_count;
    bool in_or_block;
    
    // Current network actions
    static const int MAX_PENDING_ACTIONS = 32;
    uint32_t pending_actions[MAX_PENDING_ACTIONS];
    int pending_action_count;
    uint32_t action_start_idx;
    
    STLToIRParser() { reset(); }
    
    void reset() {
        source = nullptr;
        length = 0;
        pos = 0;
        line = 1;
        col = 1;
        error_msg[0] = '\0';
        has_error = false;
        expr_stack_top = 0;
        or_branch_count = 0;
        in_or_block = false;
        pending_action_count = 0;
        action_start_idx = 0;
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
    
    // ============ Lexer Helpers ============
    
    char peek(int offset = 0) {
        int p = pos + offset;
        return (p < length) ? source[p] : '\0';
    }
    
    char advance() {
        if (pos >= length) return '\0';
        char c = source[pos++];
        if (c == '\n') { line++; col = 1; }
        else col++;
        return c;
    }
    
    void skipWhitespace() {
        while (pos < length) {
            char c = peek();
            if (c == ' ' || c == '\t' || c == '\r') advance();
            else break;
        }
    }
    
    void skipLine() {
        while (pos < length && peek() != '\n') advance();
    }
    
    bool isAlpha(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }
    
    bool isDigit(char c) {
        return c >= '0' && c <= '9';
    }
    
    bool isAlphaNum(char c) {
        return isAlpha(c) || isDigit(c);
    }
    
    bool readToken(char* buf, int maxLen) {
        skipWhitespace();
        int i = 0;
        
        // Special handling for single-character operators: = S R
        char c = peek();
        if (c == '=' || c == ')') {
            buf[0] = advance();
            buf[1] = '\0';
            return true;
        }
        
        while (i < maxLen - 1) {
            char c = peek();
            if (isAlphaNum(c) || c == '.' || c == '#' || c == '_' || c == ':') {
                buf[i++] = advance();
            } else {
                break;
            }
        }
        buf[i] = '\0';
        return i > 0;
    }
    
    bool strEqI(const char* a, const char* b) {
        while (*a && *b) {
            char ca = (*a >= 'a' && *a <= 'z') ? *a - 32 : *a;
            char cb = (*b >= 'a' && *b <= 'z') ? *b - 32 : *b;
            if (ca != cb) return false;
            a++; b++;
        }
        return *a == *b;
    }
    
    // ============ Expression Stack ============
    
    void pushExpr(uint32_t expr) {
        if (expr_stack_top < MAX_EXPR_STACK) {
            expr_stack[expr_stack_top++] = expr;
        }
    }
    
    uint32_t popExpr() {
        if (expr_stack_top > 0) {
            return expr_stack[--expr_stack_top];
        }
        return nir::NIR_NONE;
    }
    
    uint32_t peekExpr() {
        if (expr_stack_top > 0) {
            return expr_stack[expr_stack_top - 1];
        }
        return nir::NIR_NONE;
    }
    
    // Combine top two expressions with AND
    void combineAnd() {
        if (expr_stack_top >= 2) {
            uint32_t b = popExpr();
            uint32_t a = popExpr();
            uint32_t children[2] = { a, b };
            uint32_t and_expr = nir::g_store.addAnd(children, 2);
            pushExpr(and_expr);
        }
    }
    
    // ============ Control Flow Detection ============
    
    bool hasControlFlow() {
        const char* p = source;
        char word[32];
        
        while (*p) {
            // Skip whitespace
            while (*p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) p++;
            if (!*p) break;
            
            // Skip comments
            if (*p == '/' && *(p+1) == '/') {
                while (*p && *p != '\n') p++;
                continue;
            }
            
            // Read word
            int i = 0;
            while (*p && i < 31 && (isAlpha(*p) || isDigit(*p) || *p == '_')) {
                word[i++] = (*p >= 'a' && *p <= 'z') ? *p - 32 : *p;
                p++;
            }
            word[i] = '\0';
            
            if (i == 0) { p++; continue; }
            
            // Check for control flow
            if (strEqI(word, "JU") || strEqI(word, "JC") || strEqI(word, "JCN") ||
                strEqI(word, "JMP") || strEqI(word, "JUMP") ||
                strEqI(word, "CALL") || strEqI(word, "LOOP") ||
                strEqI(word, "BE") || strEqI(word, "BEC") || strEqI(word, "BEU") ||
                strEqI(word, "RET") || strEqI(word, "RETURN")) {
                return true;
            }
            
            while (*p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') p++;
        }
        return false;
    }
    
    // ============ Main Parser ============
    
    bool parse(const char* src, int len) {
        reset();
        source = src;
        length = len;
        
        nir::g_store.reset();
        
        // Check for control flow first
        if (hasControlFlow()) {
            setError("Cannot convert to Network IR: STL contains control flow (JU/JC/JCN/CALL) not representable in LADDER");
            return false;
        }
        
        action_start_idx = 0;
        
        char token[64];
        char operand[64];
        
        while (pos < length) {
            skipWhitespace();
            if (pos >= length) break;
            
            // Handle newline - potential network boundary
            if (peek() == '\n') {
                advance();
                // Check if we have pending actions -> finalize network
                if (pending_action_count > 0 && expr_stack_top > 0) {
                    finalizeNetwork();
                }
                continue;
            }
            
            // Skip comments
            if (peek() == '/' && peek(1) == '/') {
                skipLine();
                continue;
            }
            
            // Skip labels (name followed by colon)
            int save_pos = pos;
            if (readToken(token, sizeof(token))) {
                skipWhitespace();
                if (peek() == ':') {
                    advance(); // Skip label
                    continue;
                }
                // Not a label, restore position
                pos = save_pos;
            }
            
            // Read instruction
            if (!readToken(token, sizeof(token))) {
                advance();
                continue;
            }
            
            // Uppercase token
            for (int i = 0; token[i]; i++) {
                if (token[i] >= 'a' && token[i] <= 'z') token[i] -= 32;
            }
            
            // Read operand if any
            operand[0] = '\0';
            skipWhitespace();
            
            // Check for opening parenthesis (nesting)
            if (peek() == '(') {
                advance();
                operand[0] = '(';
                operand[1] = '\0';
            } else if (peek() != '\n' && peek() != '/' && peek() != '\0') {
                readToken(operand, sizeof(operand));
            }
            
            // Process instruction
            if (!processInstruction(token, operand)) {
                return false;
            }
        }
        
        // Finalize any remaining network
        if (pending_action_count > 0 || expr_stack_top > 0) {
            finalizeNetwork();
        }
        
        return !has_error;
    }
    
    bool processInstruction(const char* instr, const char* operand) {
        // ============ Load instructions (contacts) ============
        if (strEqI(instr, "A") || strEqI(instr, "AN") ||
            strEqI(instr, "O") || strEqI(instr, "ON") ||
            strEqI(instr, "X") || strEqI(instr, "XN")) {
            
            bool inverted = (instr[1] == 'N' || instr[1] == 'n');
            bool is_or = (instr[0] == 'O' || instr[0] == 'o');
            bool is_xor = (instr[0] == 'X' || instr[0] == 'x');
            
            // Check for nesting start: O( or A(
            if (operand[0] == '(') {
                if (is_or) {
                    // Start OR block - save current expression
                    if (expr_stack_top > 0) {
                        or_branches[or_branch_count++] = popExpr();
                    }
                    in_or_block = true;
                }
                // A( starts AND nesting - we'll combine on )
                return true;
            }
            
            if (operand[0] == '\0') {
                setError("Missing operand for load instruction");
                return false;
            }
            
            // Create leaf expression
            uint32_t sym_idx = nir::g_store.addSymbol(operand);
            if (sym_idx == nir::NIR_NONE) return false;
            
            uint32_t leaf = nir::g_store.addLeaf(sym_idx, inverted);
            if (leaf == nir::NIR_NONE) return false;
            
            if (is_or && expr_stack_top > 0) {
                // OR with previous - collect as branch
                or_branches[or_branch_count++] = popExpr();
                or_branches[or_branch_count++] = leaf;
                
                // Create OR expression
                uint32_t or_expr = nir::g_store.addOr(or_branches, or_branch_count);
                or_branch_count = 0;
                pushExpr(or_expr);
            } else if (is_xor && expr_stack_top > 0) {
                // XOR - for now treat similar to OR
                uint32_t prev = popExpr();
                uint32_t children[2] = { prev, leaf };
                // TODO: Add proper XOR support
                uint32_t xor_expr = nir::g_store.addOr(children, 2);
                pushExpr(xor_expr);
            } else if (expr_stack_top > 0 && !in_or_block) {
                // AND with previous (series)
                pushExpr(leaf);
                combineAnd();
            } else {
                pushExpr(leaf);
            }
            
            return true;
        }
        
        // ============ Closing parenthesis ============
        if (strEqI(instr, ")") || operand[0] == ')') {
            if (in_or_block && or_branch_count > 0) {
                // Close OR block
                if (expr_stack_top > 0) {
                    or_branches[or_branch_count++] = popExpr();
                }
                uint32_t or_expr = nir::g_store.addOr(or_branches, or_branch_count);
                or_branch_count = 0;
                in_or_block = false;
                pushExpr(or_expr);
            }
            return true;
        }
        
        // ============ NOT instruction ============
        if (strEqI(instr, "NOT")) {
            if (expr_stack_top > 0) {
                uint32_t top = popExpr();
                uint32_t not_expr = nir::g_store.addNot(top);
                pushExpr(not_expr);
            }
            return true;
        }
        
        // ============ Edge detection ============
        if (strEqI(instr, "FP") || strEqI(instr, "FN")) {
            bool is_falling = (instr[1] == 'N' || instr[1] == 'n');
            nir::CallOp op = is_falling ? nir::CALL_FN : nir::CALL_FP;
            
            // FP/FN uses operand as edge state memory
            uint32_t state_sym = nir::g_store.addSymbol(operand, nir::SYM_EDGE);
            
            if (expr_stack_top > 0) {
                uint32_t in_expr = popExpr();
                uint32_t call_expr = nir::g_store.addCallBool(op, in_expr, state_sym);
                pushExpr(call_expr);
            }
            return true;
        }
        
        // ============ Timer instructions ============
        if (strEqI(instr, "TON") || strEqI(instr, "TOF") || strEqI(instr, "TP")) {
            nir::CallOp op = strEqI(instr, "TON") ? nir::CALL_TON :
                             strEqI(instr, "TOF") ? nir::CALL_TOF : nir::CALL_TP;
            
            // Parse: TON T0, T#50ms or TON T0, #50
            char instance[32];
            uint32_t preset_ms = 0;
            
            // operand contains instance name (T0)
            int i = 0;
            while (operand[i] && operand[i] != ',' && i < 31) {
                instance[i] = operand[i];
                i++;
            }
            instance[i] = '\0';
            
            // Skip comma and whitespace, read preset
            if (operand[i] == ',') {
                i++;
                while (operand[i] == ' ') i++;
                // Parse preset (skip T# prefix if present)
                if (operand[i] == 'T' || operand[i] == 't') i++;
                if (operand[i] == '#') i++;
                preset_ms = 0;
                while (operand[i] >= '0' && operand[i] <= '9') {
                    preset_ms = preset_ms * 10 + (operand[i] - '0');
                    i++;
                }
                // Check for ms/s suffix
                if ((operand[i] == 'm' || operand[i] == 'M') &&
                    (operand[i+1] == 's' || operand[i+1] == 'S')) {
                    // Already in ms
                } else if (operand[i] == 's' || operand[i] == 'S') {
                    preset_ms *= 1000; // Convert to ms
                }
            }
            
            uint32_t inst_sym = nir::g_store.addSymbol(instance, nir::SYM_TIMER);
            
            if (expr_stack_top > 0) {
                uint32_t in_expr = popExpr();
                uint32_t call_expr = nir::g_store.addCallBool(op, in_expr, inst_sym, preset_ms);
                pushExpr(call_expr);
            }
            return true;
        }
        
        // ============ Counter instructions ============
        if (strEqI(instr, "CTU") || strEqI(instr, "CTD")) {
            nir::CallOp op = strEqI(instr, "CTU") ? nir::CALL_CTU : nir::CALL_CTD;
            
            // Parse: CTU C0, #10, X0.1 (instance, PV, reset/load)
            char instance[32];
            uint32_t pv = 0;
            char aux_operand[32];
            aux_operand[0] = '\0';
            
            int i = 0;
            // Instance
            while (operand[i] && operand[i] != ',' && i < 31) {
                instance[i] = operand[i];
                i++;
            }
            instance[i] = '\0';
            
            // PV
            if (operand[i] == ',') {
                i++;
                while (operand[i] == ' ') i++;
                if (operand[i] == '#') i++;
                while (operand[i] >= '0' && operand[i] <= '9') {
                    pv = pv * 10 + (operand[i] - '0');
                    i++;
                }
            }
            
            // Aux input (reset for CTU, load for CTD)
            if (operand[i] == ',') {
                i++;
                while (operand[i] == ' ') i++;
                int j = 0;
                while (operand[i] && j < 31) {
                    aux_operand[j++] = operand[i++];
                }
                aux_operand[j] = '\0';
            }
            
            uint32_t inst_sym = nir::g_store.addSymbol(instance, nir::SYM_COUNTER);
            uint32_t aux_expr = nir::NIR_NONE;
            
            if (aux_operand[0]) {
                uint32_t aux_sym = nir::g_store.addSymbol(aux_operand);
                aux_expr = nir::g_store.addLeaf(aux_sym);
            }
            
            if (expr_stack_top > 0) {
                uint32_t in_expr = popExpr();
                uint32_t call_expr = nir::g_store.addCallBool(op, in_expr, inst_sym, pv, aux_expr);
                pushExpr(call_expr);
            }
            return true;
        }
        
        // ============ Output instructions (coils) ============
        if (strEqI(instr, "=") || strEqI(instr, "S") || strEqI(instr, "R")) {
            nir::ActionKind kind = strEqI(instr, "=") ? nir::ACT_ASSIGN :
                                   strEqI(instr, "S") ? nir::ACT_SET : nir::ACT_RESET;
            
            uint32_t target_sym = nir::g_store.addSymbol(operand);
            uint32_t action_idx = nir::g_store.addAction(kind, target_sym);
            
            if (action_idx != nir::NIR_NONE) {
                if (pending_action_count == 0) {
                    action_start_idx = action_idx;
                }
                pending_actions[pending_action_count++] = action_idx;
            }
            return true;
        }
        
        // ============ TAP (VovkPLCRuntime extension) ============
        if (strEqI(instr, "TAP")) {
            // TAP - passthrough RLO to next network
            uint32_t action_idx = nir::g_store.addAction(nir::ACT_TAP, nir::NIR_NONE);
            
            if (action_idx != nir::NIR_NONE) {
                if (pending_action_count == 0) {
                    action_start_idx = action_idx;
                }
                pending_actions[pending_action_count++] = action_idx;
            }
            return true;
        }
        
        // ============ RLO manipulation ============
        if (strEqI(instr, "SET") || strEqI(instr, "CLR")) {
            // SET = force RLO to 1, CLR = force RLO to 0
            // For IR: create a constant true/false leaf
            // Not commonly used in LADDER, skip for now
            return true;
        }
        
        // Unknown instruction - skip
        return true;
    }
    
    void finalizeNetwork() {
        uint32_t condition = (expr_stack_top > 0) ? popExpr() : nir::NIR_NONE;
        
        if (pending_action_count > 0 || condition != nir::NIR_NONE) {
            nir::g_store.addNetwork(condition, action_start_idx, pending_action_count);
        }
        
        // Reset for next network
        pending_action_count = 0;
        action_start_idx = nir::g_store.action_count;
        expr_stack_top = 0;
        or_branch_count = 0;
        in_or_block = false;
    }
};

// ============================================================================
// Network IR to STL Emitter
// ============================================================================

class IRToSTLEmitter {
public:
    static const int MAX_OUTPUT = 32768;
    char output[MAX_OUTPUT];
    int output_len;
    
    char error_msg[256];
    bool has_error;
    
    int indent_level;  // Track nesting depth for indentation
    
    IRToSTLEmitter() { reset(); }
    
    void reset() {
        output_len = 0;
        output[0] = '\0';
        error_msg[0] = '\0';
        has_error = false;
        indent_level = 0;
    }
    
    void emit(const char* s) {
        while (*s && output_len < MAX_OUTPUT - 1) {
            output[output_len++] = *s++;
        }
        output[output_len] = '\0';
    }
    
    void emitLine(const char* s) {
        emit(s);
        emit("\n");
    }
    
    void emitIndent() {
        for (int i = 0; i < indent_level * 4; i++) {
            if (output_len < MAX_OUTPUT - 1) {
                output[output_len++] = ' ';
            }
        }
        output[output_len] = '\0';
    }
    
    void emitIndentedLine(const char* s) {
        emitIndent();
        emit(s);
        emit("\n");
    }
    
    void emitInt(int val) {
        char buf[16];
        int i = 0;
        bool neg = val < 0;
        if (neg) val = -val;
        if (val == 0) buf[i++] = '0';
        else {
            while (val > 0) {
                buf[i++] = '0' + (val % 10);
                val /= 10;
            }
        }
        if (neg) buf[i++] = '-';
        while (i > 0) {
            output[output_len++] = buf[--i];
        }
        output[output_len] = '\0';
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
    
    // ============ Expression Emission ============
    
    void emitExpr(uint32_t expr_idx, bool first_in_and = true) {
        if (expr_idx == nir::NIR_NONE) return;
        
        nir::Expr& expr = nir::g_store.exprs[expr_idx];
        
        switch (expr.kind) {
            case nir::EXPR_LEAF:
                emitLeaf(expr, first_in_and);
                break;
                
            case nir::EXPR_NOT:
                emitIndentedLine("NOT");
                emitExpr(expr.a, true);
                break;
                
            case nir::EXPR_AND:
                for (uint16_t i = 0; i < expr.child_cnt; i++) {
                    uint32_t child_idx = nir::g_store.children[expr.child_ofs + i].expr_index;
                    nir::Expr& child = nir::g_store.exprs[child_idx];
                    
                    // If child is an OR or COMPOUND expression, wrap it with A( ... )
                    if (child.kind == nir::EXPR_OR || child.kind == nir::EXPR_COMPOUND) {
                        emitIndentedLine("A(");
                        indent_level++;
                        emitExpr(child_idx, true);
                        indent_level--;
                        emitIndentedLine(")");
                    } else {
                        emitExpr(child_idx, i == 0);
                    }
                }
                break;
                
            case nir::EXPR_OR:
                // First branch - emit directly (no wrapper needed for first branch)
                // The first branch establishes the RLO, subsequent branches OR with it
                if (expr.child_cnt > 0) {
                    uint32_t first = nir::g_store.children[expr.child_ofs].expr_index;
                    emitExpr(first, true);  // First branch emits its own A() if needed
                }
                // Remaining branches with O ... (simple) or O( ... ) (complex)
                for (uint16_t i = 1; i < expr.child_cnt; i++) {
                    uint32_t child_idx = nir::g_store.children[expr.child_ofs + i].expr_index;
                    nir::Expr& child = nir::g_store.exprs[child_idx];
                    
                    // Simple leaf can use plain O, complex needs O( ... )
                    if (child.kind == nir::EXPR_LEAF) {
                        bool inverted = (child.flags & nir::EXPR_FLAG_INVERTED) != 0;
                        emitIndent();
                        emit(inverted ? "ON " : "O ");
                        emit(nir::g_store.symbols[child.a].name);
                        emit("\n");
                    } else {
                        emitIndentedLine("O(");
                        indent_level++;
                        emitExpr(child_idx, true);
                        indent_level--;
                        emitIndentedLine(")");
                    }
                }
                break;
                
            case nir::EXPR_CALL_BOOL:
                emitCallBool(expr);
                break;
                
            case nir::EXPR_COMPOUND:
                emitCompoundBranch(expr);
                break;
        }
    }
    
    // Emit a compound branch (condition + inline actions + optional TAP)
    void emitCompoundBranch(nir::Expr& expr) {
        uint32_t comp_idx = expr.a;
        nir::CompoundBranch& comp = nir::g_store.compounds[comp_idx];
        
        // Emit the condition expression
        emitExpr(comp.condition_expr, true);
        
        // Emit inline actions
        for (int i = 0; i < comp.action_count; i++) {
            nir::CompoundBranch::InlineAction& act = comp.actions[i];
            const char* instr = act.kind == nir::ACT_ASSIGN ? "=" :
                                act.kind == nir::ACT_SET ? "S" : "R";
            emitIndent();
            emit(instr);
            emit(" ");
            emit(nir::g_store.symbols[act.target_sym].name);
            emit("\n");
        }
        
        // Emit TAP if present
        if (comp.has_tap) {
            emitIndentedLine("TAP");
        }
    }
    
    void emitLeaf(nir::Expr& expr, bool first_in_and) {
        bool inverted = (expr.flags & nir::EXPR_FLAG_INVERTED) != 0;
        
        // Determine instruction
        const char* instr = first_in_and ? (inverted ? "AN" : "A") : (inverted ? "AN" : "A");
        
        emitIndent();
        emit(instr);
        emit(" ");
        emit(nir::g_store.symbols[expr.a].name);
        emit("\n");
    }
    
    void emitCallBool(nir::Expr& expr) {
        uint32_t call_idx = expr.a;
        nir::CallBool& call = nir::g_store.calls[call_idx];
        
        // First emit the input expression
        emitExpr(call.in_expr, true);
        
        // Then emit the call instruction
        switch (call.op) {
            case nir::CALL_FP:
                emitIndent();
                emit("FP ");
                emit(nir::g_store.symbols[call.instance_sym].name);
                emit("\n");
                break;
                
            case nir::CALL_FN:
                emitIndent();
                emit("FN ");
                emit(nir::g_store.symbols[call.instance_sym].name);
                emit("\n");
                break;
                
            case nir::CALL_FX:
                emitIndent();
                emit("FX ");
                emit(nir::g_store.symbols[call.instance_sym].name);
                emit("\n");
                break;
                
            case nir::CALL_TON:
            case nir::CALL_TOF:
            case nir::CALL_TP: {
                const char* op = call.op == nir::CALL_TON ? "TON" :
                                 call.op == nir::CALL_TOF ? "TOF" : "TP";
                emitIndent();
                emit(op);
                emit(" ");
                emit(nir::g_store.symbols[call.instance_sym].name);
                emit(", ");
                if (call.preset_kind == nir::PRESET_LITERAL) {
                    emit(&nir::g_store.preset_pool[call.preset_str_ofs]);
                } else {
                    emit("T#");
                    emitInt(call.preset_u32);
                    emit("ms");
                }
                emit("\n");
                break;
            }
                
            case nir::CALL_CTU:
            case nir::CALL_CTD: {
                const char* op = call.op == nir::CALL_CTU ? "CTU" : "CTD";
                emitIndent();
                emit(op);
                emit(" ");
                emit(nir::g_store.symbols[call.instance_sym].name);
                emit(", #");
                emitInt(call.preset_u32);
                if (call.aux_expr != nir::NIR_NONE) {
                    emit(", ");
                    // Get the aux symbol name
                    nir::Expr& aux = nir::g_store.exprs[call.aux_expr];
                    if (aux.kind == nir::EXPR_LEAF) {
                        emit(nir::g_store.symbols[aux.a].name);
                    }
                }
                emit("\n");
                break;
            }
            
            default:
                break;
        }
    }
    
    // ============ Action Emission ============
    
    void emitAction(nir::Action& action) {
        if (action.kind == nir::ACT_TAP) {
            // TAP - passthrough RLO to next network (no operand)
            emitIndentedLine("TAP");
            return;
        }
        const char* instr = action.kind == nir::ACT_ASSIGN ? "=" :
                            action.kind == nir::ACT_SET ? "S" : "R";
        emitIndent();
        emit(instr);
        emit(" ");
        emit(nir::g_store.symbols[action.target_sym].name);
        emit("\n");
    }
    
    // ============ Network Emission ============
    
    void emitNetwork(nir::Network& net) {
        // Emit condition expression
        if (net.condition_expr != nir::NIR_NONE) {
            emitExpr(net.condition_expr, true);
        }
        
        // Emit actions
        for (uint16_t i = 0; i < net.action_cnt; i++) {
            emitAction(nir::g_store.actions[net.action_ofs + i]);
        }
        
        emit("\n"); // Blank line between networks
    }
    
    // ============ Full Program Emission ============
    
    bool emitAll() {
        reset();
        
        emit("// Generated from Network IR by VovkPLCRuntime\n\n");
        
        for (uint32_t i = 0; i < nir::g_store.network_count; i++) {
            emitNetwork(nir::g_store.networks[i]);
        }
        
        return !has_error;
    }
};

// ============================================================================
// Global Instances and WASM Exports
// ============================================================================

static LadderJSONToIRParser ladderJSONParser;
static STLToIRParser stlToIRParser;
static IRToSTLEmitter irToSTLEmitter;
static char ladder_input_buffer[32768];
static int ladder_input_length = 0;

extern "C" {

// ============ Ladder JSON to Network IR ============

WASM_EXPORT void ladder_load_from_stream() {
    streamRead(ladder_input_buffer, ladder_input_length, sizeof(ladder_input_buffer));
}

WASM_EXPORT bool ladder_compile() {
    // Parse JSON Ladder into Network IR, then emit to STL
    if (!ladderJSONParser.parse(ladder_input_buffer, ladder_input_length)) {
        return false;
    }
    return irToSTLEmitter.emitAll();
}

WASM_EXPORT const char* ladder_get_output() {
    return irToSTLEmitter.output;
}

WASM_EXPORT int ladder_get_output_length() {
    return irToSTLEmitter.output_len;
}

WASM_EXPORT void ladder_output_to_stream() {
    for (int i = 0; i < irToSTLEmitter.output_len; i++) {
        streamOut(irToSTLEmitter.output[i]);
    }
}

WASM_EXPORT bool ladder_has_error() {
    return ladderJSONParser.has_error || irToSTLEmitter.has_error;
}

WASM_EXPORT const char* ladder_get_error() {
    if (ladderJSONParser.has_error) return ladderJSONParser.error_msg;
    if (irToSTLEmitter.has_error) return irToSTLEmitter.error_msg;
    return "";
}

// ============ Full Pipeline: Ladder JSON -> IR -> STL -> PLCASM -> Bytecode ============

WASM_EXPORT bool ladder_compile_full() {
    // Step 1: Parse JSON Ladder to Network IR
    if (!ladderJSONParser.parse(ladder_input_buffer, ladder_input_length)) {
        return false;
    }
    
    // Step 2: Emit Network IR to canonical STL
    if (!irToSTLEmitter.emitAll()) {
        return false;
    }
    
    // Step 3: Compile STL to PLCASM
    stlCompiler.setSource(irToSTLEmitter.output, irToSTLEmitter.output_len);
    if (!stlCompiler.compile()) {
        return false;
    }
    
    // Step 4: Compile PLCASM to bytecode
    defaultCompiler.set_assembly_string((char*)stlCompiler.getOutput());
    return !defaultCompiler.compileAssembly(false, false);
}

// ============ STL to Network IR ============

WASM_EXPORT void stl_to_ir_load_from_stream() {
    streamRead(ladder_input_buffer, ladder_input_length, sizeof(ladder_input_buffer));
}

WASM_EXPORT bool stl_to_ir_parse() {
    return stlToIRParser.parse(ladder_input_buffer, ladder_input_length);
}

WASM_EXPORT bool stl_to_ir_has_error() {
    return stlToIRParser.has_error;
}

WASM_EXPORT const char* stl_to_ir_get_error() {
    return stlToIRParser.error_msg;
}

// ============ Network IR to STL ============

WASM_EXPORT bool ir_to_stl_emit() {
    return irToSTLEmitter.emitAll();
}

WASM_EXPORT const char* ir_to_stl_get_output() {
    return irToSTLEmitter.output;
}

WASM_EXPORT int ir_to_stl_get_output_length() {
    return irToSTLEmitter.output_len;
}

WASM_EXPORT void ir_to_stl_output_to_stream() {
    for (int i = 0; i < irToSTLEmitter.output_len; i++) {
        streamOut(irToSTLEmitter.output[i]);
    }
}

WASM_EXPORT bool ir_to_stl_has_error() {
    return irToSTLEmitter.has_error;
}

WASM_EXPORT const char* ir_to_stl_get_error() {
    return irToSTLEmitter.error_msg;
}

// ============ Round-trip: STL -> IR -> STL ============

WASM_EXPORT bool stl_roundtrip() {
    // Parse STL to IR
    if (!stlToIRParser.parse(ladder_input_buffer, ladder_input_length)) {
        return false;
    }
    // Emit IR to STL
    return irToSTLEmitter.emitAll();
}

// ============ Full Pipeline: STL -> IR -> STL -> PLCASM -> Bytecode ============

WASM_EXPORT bool stl_to_ir_compile_full() {
    // Step 1: Parse STL to Network IR
    if (!stlToIRParser.parse(ladder_input_buffer, ladder_input_length)) {
        return false;
    }
    
    // Step 2: Emit Network IR back to canonical STL
    if (!irToSTLEmitter.emitAll()) {
        return false;
    }
    
    // Step 3: Compile STL to PLCASM
    stlCompiler.setSource(irToSTLEmitter.output, irToSTLEmitter.output_len);
    if (!stlCompiler.compile()) {
        return false;
    }
    
    // Step 4: Compile PLCASM to bytecode
    defaultCompiler.set_assembly_string((char*)stlCompiler.getOutput());
    return !defaultCompiler.compileAssembly(false, false);
}

// ============ STL to Ladder (via IR) ============

WASM_EXPORT bool stl_to_ladder_convert() {
    // Parse STL into Network IR
    if (!stlToIRParser.parse(stl_source_buffer, stl_source_length)) {
        return false;
    }
    // The IR itself IS the "ladder" representation
    // Emit back to STL for now (visual ladder rendering is a JS concern)
    return irToSTLEmitter.emitAll();
}

WASM_EXPORT bool stl_to_ladder_has_error() {
    return stlToIRParser.has_error;
}

WASM_EXPORT const char* stl_to_ladder_get_error() {
    return stlToIRParser.error_msg;
}

WASM_EXPORT const char* stl_to_ladder_get_output() {
    return irToSTLEmitter.output;
}

WASM_EXPORT int stl_to_ladder_get_output_length() {
    return irToSTLEmitter.output_len;
}

WASM_EXPORT void stl_to_ladder_output_to_stream() {
    ir_to_stl_output_to_stream();
}

} // extern "C"

#endif // __WASM__
