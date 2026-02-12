// wcet-cfg.h - Control Flow Graph builder for WCET analysis
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

#include "wcet-costs.h"

// ============================================================================
// Control Flow Graph (CFG) for bytecode analysis
// ============================================================================
// All state is kept in bare global arrays for WASM export stability.
// No classes, no constructors, no member references — just plain C-style data.

#define WCET_MAX_BLOCKS 512
#define WCET_MAX_EDGES 1024
#define WCET_MAX_INSTRUCTIONS 4096
#define WCET_MAX_LOOPS 64

// ============================================================================
// Type definitions
// ============================================================================

struct WCETInstruction {
    u32 offset;
    u8 opcode;
    u8 type_arg;
    u8 size;
    OpcodeCost cost;
    OpcodeStackEffect stack_effect;
};

struct WCETBlock {
    u32 id;
    u32 start_offset;
    u32 end_offset;
    u32 first_instr;
    u32 instr_count;
    u32 bcet;
    u32 wcet;
    i32 stack_delta;
    i32 max_stack_growth;
    u8 is_entry;
    u8 is_exit;
    u8 is_return;
    u8 has_call;
    u8 is_unreachable;
};

enum WCETEdgeType : u8 {
    EDGE_FALLTHROUGH = 0,
    EDGE_JUMP,
    EDGE_BRANCH_TRUE,
    EDGE_BRANCH_FALSE,
    EDGE_CALL,
    EDGE_RETURN,
};

struct WCETEdge {
    u32 from_block;
    u32 to_block;
    u8 type;
    u8 is_back_edge;
};

struct WCETLoop {
    u32 header_block;
    u32 back_edge_block;
    u32 estimated_max_iterations;
};

// ============================================================================
// Global CFG state — bare globals, no classes
// ============================================================================
WCETInstruction g_wcet_instructions[WCET_MAX_INSTRUCTIONS];
u32             g_wcet_instruction_count = 0;

WCETBlock       g_wcet_blocks[WCET_MAX_BLOCKS];
u32             g_wcet_block_count = 0;

WCETEdge        g_wcet_edges[WCET_MAX_EDGES];
u32             g_wcet_edge_count = 0;

WCETLoop        g_wcet_loops[WCET_MAX_LOOPS];
u32             g_wcet_loop_count = 0;

u32             g_wcet_unreachable_block_count = 0;
u32             g_wcet_dead_code_bytes = 0;

// ============================================================================
// Helpers
// ============================================================================
inline bool wcet_is_branch(u8 op) {
    return op == JMP || op == JMP_IF || op == JMP_IF_NOT ||
           op == JMP_REL || op == JMP_IF_REL || op == JMP_IF_NOT_REL;
}
inline bool wcet_is_call(u8 op) {
    return op == CALL || op == CALL_IF || op == CALL_IF_NOT ||
           op == CALL_REL || op == CALL_IF_REL || op == CALL_IF_NOT_REL;
}
inline bool wcet_is_conditional_branch(u8 op) {
    return op == JMP_IF || op == JMP_IF_NOT ||
           op == JMP_IF_REL || op == JMP_IF_NOT_REL;
}
inline bool wcet_is_conditional_call(u8 op) {
    return op == CALL_IF || op == CALL_IF_NOT ||
           op == CALL_IF_REL || op == CALL_IF_NOT_REL;
}

inline u32 wcet_resolve_target(const WCETInstruction& instr, const u8* bytecode) {
    u8 op = instr.opcode;
    u32 off = instr.offset;
    if (op == JMP || op == JMP_IF || op == JMP_IF_NOT ||
        op == CALL || op == CALL_IF || op == CALL_IF_NOT) {
        u16 target = (u16)bytecode[off + 1] | ((u16)bytecode[off + 2] << 8);
        return (u32)target;
    }
    if (op == JMP_REL || op == JMP_IF_REL || op == JMP_IF_NOT_REL ||
        op == CALL_REL || op == CALL_IF_REL || op == CALL_IF_NOT_REL) {
        i16 rel = (i16)((u16)bytecode[off + 1] | ((u16)bytecode[off + 2] << 8));
        i32 target = (i32)(off + instr.size) + (i32)rel;
        if (target < 0) return 0xFFFFFFFF;
        return (u32)target;
    }
    return 0xFFFFFFFF;
}

inline u32 wcet_find_block_by_offset(u32 offset) {
    for (u32 b = 0; b < g_wcet_block_count; b++) {
        if (g_wcet_blocks[b].start_offset == offset) return b;
    }
    return 0xFFFFFFFF;
}

inline void wcet_add_edge(u32 from, u32 to, u8 type) {
    if (g_wcet_edge_count < WCET_MAX_EDGES) {
        WCETEdge& e = g_wcet_edges[g_wcet_edge_count++];
        e.from_block = from;
        e.to_block = to;
        e.type = type;
        e.is_back_edge = 0;
    }
}

inline void wcet_compute_block_metrics(WCETBlock& block) {
    block.bcet = 0;
    block.wcet = 0;
    block.stack_delta = 0;
    block.max_stack_growth = 0;
    block.has_call = 0;
    i32 current_stack = 0;
    for (u32 i = 0; i < block.instr_count; i++) {
        const WCETInstruction& instr = g_wcet_instructions[block.first_instr + i];
        block.bcet += instr.cost.bcet;
        block.wcet += instr.cost.wcet;
        current_stack -= instr.stack_effect.pop_bytes;
        current_stack += instr.stack_effect.push_bytes;
        if (current_stack > block.max_stack_growth) block.max_stack_growth = current_stack;
        if (wcet_is_call(instr.opcode)) block.has_call = 1;
    }
    block.stack_delta = current_stack;
}

// ============================================================================
// CFG construction phases — all operate on globals
// ============================================================================

void wcet_cfg_reset() {
    g_wcet_instruction_count = 0;
    g_wcet_block_count = 0;
    g_wcet_edge_count = 0;
    g_wcet_loop_count = 0;
    g_wcet_unreachable_block_count = 0;
    g_wcet_dead_code_bytes = 0;
}

bool wcet_cfg_decode(const u8* bytecode, u32 length) {
    g_wcet_instruction_count = 0;
    u32 offset = 0;
    while (offset < length && g_wcet_instruction_count < WCET_MAX_INSTRUCTIONS) {
        WCETInstruction& instr = g_wcet_instructions[g_wcet_instruction_count];
        instr.offset = offset;
        instr.opcode = bytecode[offset];
        u8 op_size = OPCODE_SIZE((PLCRuntimeInstructionSet)instr.opcode);
        if (op_size == 0) {
            if (instr.opcode == COMMENT && offset + 1 < length) {
                op_size = 2 + bytecode[offset + 1];
            } else if (instr.opcode == CONFIG_DB && offset + 1 < length) {
                op_size = 2 + bytecode[offset + 1] * 4;
            } else if ((instr.opcode == CSTR_LIT || instr.opcode == CSTR_CAT) && offset + 4 < length) {
                u16 str_len = (u16)bytecode[offset + 3] | ((u16)bytecode[offset + 4] << 8);
                op_size = 5 + str_len;
            } else {
                op_size = 1;
            }
        }
        instr.size = op_size;
        instr.type_arg = 0;
        if (op_size >= 2 && offset + 1 < length) {
            u8 op = instr.opcode;
            if (op == CVT || op == LOAD || op == MOVE || op == MOVE_COPY ||
                op == LOAD_FROM || op == MOVE_TO || op == INC_MEM || op == DEC_MEM ||
                op == COPY || op == SWAP || op == DROP ||
                op == PICK || op == POKE ||
                op == ADD || op == SUB || op == MUL || op == DIV || op == MOD ||
                op == POW || op == SQRT || op == NEG || op == ABS ||
                op == SIN || op == COS ||
                op == CMP_EQ || op == CMP_NEQ || op == CMP_GT || op == CMP_LT ||
                op == CMP_GTE || op == CMP_LTE) {
                instr.type_arg = bytecode[offset + 1];
            }
        }
        instr.cost = wcet_opcode_cost(instr.opcode);
        instr.stack_effect = wcet_stack_effect(instr.opcode, instr.type_arg);
        g_wcet_instruction_count++;
        offset += op_size;
    }
    return true;
}

bool wcet_cfg_build_blocks(const u8* bytecode) {
    if (g_wcet_instruction_count == 0) return false;
    bool is_leader[WCET_MAX_INSTRUCTIONS];
    for (u32 i = 0; i < WCET_MAX_INSTRUCTIONS; i++) is_leader[i] = false;
    is_leader[0] = true;
    for (u32 i = 0; i < g_wcet_instruction_count; i++) {
        const WCETInstruction& instr = g_wcet_instructions[i];
        u8 op = instr.opcode;
        if (wcet_is_branch(op) || wcet_is_call(op) || op == RET ||
            op == RET_IF || op == RET_IF_NOT || op == EXIT) {
            if (i + 1 < g_wcet_instruction_count) is_leader[i + 1] = true;
        }
        if (wcet_is_branch(op) || wcet_is_call(op)) {
            u32 target = wcet_resolve_target(instr, bytecode);
            if (target != 0xFFFFFFFF) {
                for (u32 j = 0; j < g_wcet_instruction_count; j++) {
                    if (g_wcet_instructions[j].offset == target) { is_leader[j] = true; break; }
                }
            }
        }
    }
    g_wcet_block_count = 0;
    for (u32 i = 0; i < g_wcet_instruction_count && g_wcet_block_count < WCET_MAX_BLOCKS; i++) {
        if (!is_leader[i]) continue;
        WCETBlock& block = g_wcet_blocks[g_wcet_block_count];
        block.id = g_wcet_block_count;
        block.start_offset = g_wcet_instructions[i].offset;
        block.first_instr = i;
        block.is_entry = (i == 0) ? 1 : 0;
        block.is_exit = 0;
        block.is_return = 0;
        block.has_call = 0;
        block.is_unreachable = 1;
        u32 instr_end = i + 1;
        while (instr_end < g_wcet_instruction_count && !is_leader[instr_end]) instr_end++;
        block.instr_count = instr_end - i;
        if (instr_end < g_wcet_instruction_count) {
            block.end_offset = g_wcet_instructions[instr_end].offset;
        } else {
            const WCETInstruction& last = g_wcet_instructions[instr_end - 1];
            block.end_offset = last.offset + last.size;
        }
        wcet_compute_block_metrics(block);
        g_wcet_block_count++;
    }
    return g_wcet_block_count > 0;
}

bool wcet_cfg_build_edges(const u8* bytecode, u32 length) {
    g_wcet_edge_count = 0;
    (void)length;
    for (u32 b = 0; b < g_wcet_block_count; b++) {
        WCETBlock& block = g_wcet_blocks[b];
        u32 li = block.first_instr + block.instr_count - 1;
        const WCETInstruction& last = g_wcet_instructions[li];
        u8 op = last.opcode;
        if (op == EXIT) { block.is_exit = 1; continue; }
        if (op == RET)  { block.is_return = 1; continue; }
        if (op == JMP || op == JMP_REL) {
            u32 t = wcet_resolve_target(last, bytecode);
            u32 tb = wcet_find_block_by_offset(t);
            if (tb != 0xFFFFFFFF) wcet_add_edge(b, tb, EDGE_JUMP);
        } else if (wcet_is_conditional_branch(op)) {
            u32 t = wcet_resolve_target(last, bytecode);
            u32 tb = wcet_find_block_by_offset(t);
            if (tb != 0xFFFFFFFF) wcet_add_edge(b, tb, EDGE_BRANCH_TRUE);
            if (b + 1 < g_wcet_block_count) wcet_add_edge(b, b + 1, EDGE_BRANCH_FALSE);
        } else if (op == CALL || op == CALL_REL) {
            u32 t = wcet_resolve_target(last, bytecode);
            u32 tb = wcet_find_block_by_offset(t);
            if (tb != 0xFFFFFFFF) wcet_add_edge(b, tb, EDGE_CALL);
            block.has_call = 1;
            if (b + 1 < g_wcet_block_count) wcet_add_edge(b, b + 1, EDGE_RETURN);
        } else if (wcet_is_conditional_call(op)) {
            u32 t = wcet_resolve_target(last, bytecode);
            u32 tb = wcet_find_block_by_offset(t);
            if (tb != 0xFFFFFFFF) wcet_add_edge(b, tb, EDGE_CALL);
            block.has_call = 1;
            if (b + 1 < g_wcet_block_count) wcet_add_edge(b, b + 1, EDGE_BRANCH_FALSE);
        } else if (op == RET_IF || op == RET_IF_NOT) {
            block.is_return = 1;
            if (b + 1 < g_wcet_block_count) wcet_add_edge(b, b + 1, EDGE_BRANCH_FALSE);
        } else {
            if (b + 1 < g_wcet_block_count) wcet_add_edge(b, b + 1, EDGE_FALLTHROUGH);
            else block.is_exit = 1;
        }
    }
    return true;
}

void wcet_cfg_reachability() {
    bool visited[WCET_MAX_BLOCKS];
    u32 queue[WCET_MAX_BLOCKS];
    for (u32 i = 0; i < WCET_MAX_BLOCKS; i++) visited[i] = false;
    u32 q_head = 0, q_tail = 0;
    for (u32 b = 0; b < g_wcet_block_count; b++) {
        if (g_wcet_blocks[b].is_entry) { queue[q_tail++] = b; visited[b] = true; break; }
    }
    while (q_head < q_tail) {
        u32 cur = queue[q_head++];
        for (u32 e = 0; e < g_wcet_edge_count; e++) {
            if (g_wcet_edges[e].from_block == cur && !visited[g_wcet_edges[e].to_block]) {
                visited[g_wcet_edges[e].to_block] = true;
                queue[q_tail++] = g_wcet_edges[e].to_block;
            }
        }
    }
    g_wcet_unreachable_block_count = 0;
    g_wcet_dead_code_bytes = 0;
    for (u32 b = 0; b < g_wcet_block_count; b++) {
        g_wcet_blocks[b].is_unreachable = visited[b] ? 0 : 1;
        if (!visited[b]) {
            g_wcet_unreachable_block_count++;
            g_wcet_dead_code_bytes += g_wcet_blocks[b].end_offset - g_wcet_blocks[b].start_offset;
        }
    }
}

void wcet_cfg_detect_loops() {
    g_wcet_loop_count = 0;
    u8 color[WCET_MAX_BLOCKS]; // 0=WHITE, 1=GRAY, 2=BLACK
    for (u32 i = 0; i < g_wcet_block_count; i++) color[i] = 0;
    struct DFSFrame { u32 block_id; u32 edge_idx; };
    DFSFrame dfs_stack[WCET_MAX_BLOCKS];
    u32 dfs_top = 0;
    u32 entry = 0;
    for (u32 b = 0; b < g_wcet_block_count; b++) {
        if (g_wcet_blocks[b].is_entry) { entry = b; break; }
    }
    dfs_stack[0] = { entry, 0 };
    dfs_top = 1;
    color[entry] = 1; // GRAY
    while (dfs_top > 0) {
        DFSFrame& frame = dfs_stack[dfs_top - 1];
        bool found = false;
        while (frame.edge_idx < g_wcet_edge_count) {
            u32 e = frame.edge_idx++;
            if (g_wcet_edges[e].from_block != frame.block_id) continue;
            u32 t = g_wcet_edges[e].to_block;
            if (color[t] == 1) { // GRAY — back edge
                g_wcet_edges[e].is_back_edge = 1;
                if (g_wcet_loop_count < WCET_MAX_LOOPS) {
                    g_wcet_loops[g_wcet_loop_count].header_block = t;
                    g_wcet_loops[g_wcet_loop_count].back_edge_block = frame.block_id;
                    g_wcet_loops[g_wcet_loop_count].estimated_max_iterations = 100;
                    g_wcet_loop_count++;
                }
            } else if (color[t] == 0) { // WHITE
                color[t] = 1;
                dfs_stack[dfs_top++] = { t, 0 };
                found = true;
                break;
            }
        }
        if (!found) {
            color[frame.block_id] = 2; // BLACK
            dfs_top--;
        }
    }
}

// Full CFG construction pipeline
bool wcet_cfg_build(const u8* bytecode, u32 length) {
    wcet_cfg_reset();
    if (length == 0) return false;
    if (!wcet_cfg_decode(bytecode, length)) return false;
    if (!wcet_cfg_build_blocks(bytecode)) return false;
    if (!wcet_cfg_build_edges(bytecode, length)) return false;
    wcet_cfg_reachability();
    wcet_cfg_detect_loops();
    return true;
}

#endif // __WASM__
