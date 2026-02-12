// wcet-analysis.h - WCET (Worst-Case Execution Time) Analysis
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

#include "wcet-cfg.h"
#include "wcet-targets.h"

// ============================================================================
// WCET Analysis Report — all fields are plain globals for WASM safety
// ============================================================================

struct WCETOpcodeFreq {
    u8 opcode;
    u16 count;
    u32 total_bcet;
    u32 total_wcet;
};

// Global report fields (no struct wrapping — just plain globals)
u32  g_wcet_bytecode_size       = 0;
u32  g_wcet_rpt_instruction_count = 0;
u32  g_wcet_unique_opcodes      = 0;
u32  g_wcet_rpt_block_count     = 0;
u32  g_wcet_rpt_edge_count      = 0;
u32  g_wcet_rpt_loop_count      = 0;
u32  g_wcet_rpt_unreachable     = 0;
u32  g_wcet_rpt_dead_bytes      = 0;
u32  g_wcet_bcet_cycles         = 0;
u32  g_wcet_wcet_cycles         = 0;
u32  g_wcet_bcet_instructions   = 0;
u32  g_wcet_wcet_instructions   = 0;
i32  g_wcet_max_stack_depth     = 0;
i32  g_wcet_min_stack_depth     = 0;
i32  g_wcet_stack_at_exit       = 0;
u32  g_wcet_max_call_depth      = 0;
u8   g_wcet_has_recursion       = 0;
u32  g_wcet_total_paths         = 0;
u32  g_wcet_shortest_path       = 0;
u32  g_wcet_longest_path        = 0;
u32  g_wcet_warning_count       = 0;
u8   g_wcet_warn_dead_code      = 0;
u8   g_wcet_warn_infinite_loop  = 0;
u8   g_wcet_warn_stack_overflow = 0;
u8   g_wcet_warn_stack_underflow = 0;
u8   g_wcet_warn_unbalanced     = 0;

WCETOpcodeFreq g_wcet_opcode_freq[32];
u8   g_wcet_opcode_freq_count   = 0;

// Target profile nanosecond estimates (populated when a target is active)
u32  g_wcet_bcet_ns             = 0;  // Best-case nanoseconds (0 = no target)
u32  g_wcet_wcet_ns             = 0;  // Worst-case nanoseconds (0 = no target)
u8   g_wcet_has_target          = 0;  // 1 if a calibrated target profile is active

// ============================================================================
// Report reset
// ============================================================================
void wcet_report_reset() {
    g_wcet_bytecode_size = 0;
    g_wcet_rpt_instruction_count = 0;
    g_wcet_unique_opcodes = 0;
    g_wcet_rpt_block_count = 0;
    g_wcet_rpt_edge_count = 0;
    g_wcet_rpt_loop_count = 0;
    g_wcet_rpt_unreachable = 0;
    g_wcet_rpt_dead_bytes = 0;
    g_wcet_bcet_cycles = 0;
    g_wcet_wcet_cycles = 0;
    g_wcet_bcet_instructions = 0;
    g_wcet_wcet_instructions = 0;
    g_wcet_max_stack_depth = 0;
    g_wcet_min_stack_depth = 0;
    g_wcet_stack_at_exit = 0;
    g_wcet_max_call_depth = 0;
    g_wcet_has_recursion = 0;
    g_wcet_total_paths = 0;
    g_wcet_shortest_path = 0;
    g_wcet_longest_path = 0;
    g_wcet_warning_count = 0;
    g_wcet_warn_dead_code = 0;
    g_wcet_warn_infinite_loop = 0;
    g_wcet_warn_stack_overflow = 0;
    g_wcet_warn_stack_underflow = 0;
    g_wcet_warn_unbalanced = 0;
    g_wcet_opcode_freq_count = 0;
    g_wcet_bcet_ns = 0;
    g_wcet_wcet_ns = 0;
    g_wcet_has_target = 0;
    for (int i = 0; i < 32; i++) {
        g_wcet_opcode_freq[i].opcode = 0;
        g_wcet_opcode_freq[i].count = 0;
        g_wcet_opcode_freq[i].total_bcet = 0;
        g_wcet_opcode_freq[i].total_wcet = 0;
    }
}

// ============================================================================
// Sub-analyses — all operate on globals
// ============================================================================

void wcet_analyze_opcode_frequency() {
    u16 freq[256];
    u32 bcet_total[256];
    u32 wcet_total[256];
    bool seen[256];
    for (int i = 0; i < 256; i++) { freq[i] = 0; bcet_total[i] = 0; wcet_total[i] = 0; seen[i] = false; }

    for (u32 i = 0; i < g_wcet_instruction_count; i++) {
        u8 op = g_wcet_instructions[i].opcode;
        freq[op]++;
        bcet_total[op] += g_wcet_instructions[i].cost.bcet;
        wcet_total[op] += g_wcet_instructions[i].cost.wcet;
        seen[op] = true;
    }

    g_wcet_unique_opcodes = 0;
    for (int i = 0; i < 256; i++) { if (seen[i]) g_wcet_unique_opcodes++; }

    g_wcet_opcode_freq_count = 0;
    for (int op = 0; op < 256; op++) {
        if (freq[op] == 0) continue;
        if (g_wcet_opcode_freq_count < 32) {
            WCETOpcodeFreq& f = g_wcet_opcode_freq[g_wcet_opcode_freq_count++];
            f.opcode = (u8)op;
            f.count = freq[op];
            f.total_bcet = bcet_total[op];
            f.total_wcet = wcet_total[op];
        }
    }

    // Sort by total_wcet descending
    for (u8 i = 1; i < g_wcet_opcode_freq_count; i++) {
        WCETOpcodeFreq tmp = g_wcet_opcode_freq[i];
        u8 j = i;
        while (j > 0 && g_wcet_opcode_freq[j - 1].total_wcet < tmp.total_wcet) {
            g_wcet_opcode_freq[j] = g_wcet_opcode_freq[j - 1];
            j--;
        }
        g_wcet_opcode_freq[j] = tmp;
    }
}

void wcet_analyze_paths() {
    if (g_wcet_block_count == 0) return;

    u32 entry = 0;
    for (u32 b = 0; b < g_wcet_block_count; b++) {
        if (g_wcet_blocks[b].is_entry) { entry = b; break; }
    }

    u32 best_bcet = 0xFFFFFFFF;
    u32 best_wcet = 0;
    u32 best_bcet_instrs = 0xFFFFFFFF;
    u32 best_wcet_instrs = 0;
    u32 shortest_path_len = 0xFFFFFFFF;
    u32 longest_path_len = 0;
    u32 path_count = 0;

    struct PathFrame {
        u32 block_id;
        u32 edge_idx;
        u32 cumulative_bcet;
        u32 cumulative_wcet;
        u32 cumulative_instrs;
        u32 path_length;
    };

    PathFrame path_stack[WCET_MAX_BLOCKS];
    bool on_path[WCET_MAX_BLOCKS];
    u32 visit_count[WCET_MAX_BLOCKS];
    for (u32 i = 0; i < WCET_MAX_BLOCKS; i++) { on_path[i] = false; visit_count[i] = 0; }

    u32 top = 0;
    path_stack[0] = { entry, 0, g_wcet_blocks[entry].bcet, g_wcet_blocks[entry].wcet,
                      g_wcet_blocks[entry].instr_count, 1 };
    on_path[entry] = true;
    visit_count[entry] = 1;

    u32 max_search = 10000;
    u32 iterations = 0;

    while (top < WCET_MAX_BLOCKS && iterations < max_search) {
        iterations++;
        PathFrame& frame = path_stack[top];
        WCETBlock& block = g_wcet_blocks[frame.block_id];

        if (block.is_exit || block.is_return) {
            path_count++;
            if (frame.cumulative_bcet < best_bcet) { best_bcet = frame.cumulative_bcet; best_bcet_instrs = frame.cumulative_instrs; }
            if (frame.cumulative_wcet > best_wcet) { best_wcet = frame.cumulative_wcet; best_wcet_instrs = frame.cumulative_instrs; }
            if (frame.path_length < shortest_path_len) shortest_path_len = frame.path_length;
            if (frame.path_length > longest_path_len) longest_path_len = frame.path_length;
            on_path[frame.block_id] = false;
            visit_count[frame.block_id]--;
            if (top == 0) break;
            top--;
            continue;
        }

        bool found_next = false;
        u32 ei = frame.edge_idx;
        while (ei < g_wcet_edge_count) {
            frame.edge_idx = ei + 1;
            if (g_wcet_edges[ei].from_block != frame.block_id) { ei++; continue; }
            u32 next = g_wcet_edges[ei].to_block;
            u32 max_visits = 1;
            if (g_wcet_edges[ei].is_back_edge || on_path[next]) {
                for (u32 l = 0; l < g_wcet_loop_count; l++) {
                    if (g_wcet_loops[l].header_block == next) { max_visits = g_wcet_loops[l].estimated_max_iterations; break; }
                }
                if (max_visits > 3) max_visits = 3;
            }
            if (visit_count[next] >= max_visits) { ei++; continue; }
            WCETBlock& nb = g_wcet_blocks[next];
            on_path[next] = true;
            visit_count[next]++;
            top++;
            path_stack[top] = { next, 0, frame.cumulative_bcet + nb.bcet, frame.cumulative_wcet + nb.wcet,
                                frame.cumulative_instrs + nb.instr_count, frame.path_length + 1 };
            found_next = true;
            break;
        }

        if (!found_next) {
            if (!block.is_exit && !block.is_return) {
                path_count++;
                if (frame.cumulative_bcet < best_bcet) { best_bcet = frame.cumulative_bcet; best_bcet_instrs = frame.cumulative_instrs; }
                if (frame.cumulative_wcet > best_wcet) { best_wcet = frame.cumulative_wcet; best_wcet_instrs = frame.cumulative_instrs; }
                if (frame.path_length < shortest_path_len) shortest_path_len = frame.path_length;
                if (frame.path_length > longest_path_len) longest_path_len = frame.path_length;
            }
            on_path[frame.block_id] = false;
            visit_count[frame.block_id]--;
            if (top == 0) break;
            top--;
        }
    }

    g_wcet_bcet_cycles = (best_bcet == 0xFFFFFFFF) ? 0 : best_bcet;
    g_wcet_wcet_cycles = best_wcet;
    g_wcet_bcet_instructions = (best_bcet_instrs == 0xFFFFFFFF) ? 0 : best_bcet_instrs;
    g_wcet_wcet_instructions = best_wcet_instrs;
    g_wcet_total_paths = path_count;
    g_wcet_shortest_path = (shortest_path_len == 0xFFFFFFFF) ? 0 : shortest_path_len;
    g_wcet_longest_path = longest_path_len;
}

void wcet_analyze_stack_depth(u32 max_stack_size) {
    if (g_wcet_block_count == 0) return;

    i32 max_depth_at_entry[WCET_MAX_BLOCKS];
    i32 min_depth_at_entry[WCET_MAX_BLOCKS];
    bool processed[WCET_MAX_BLOCKS];
    for (u32 i = 0; i < WCET_MAX_BLOCKS; i++) {
        max_depth_at_entry[i] = -1;
        min_depth_at_entry[i] = 0x7FFFFFFF;
        processed[i] = false;
    }

    u32 entry = 0;
    for (u32 b = 0; b < g_wcet_block_count; b++) {
        if (g_wcet_blocks[b].is_entry) { entry = b; break; }
    }
    max_depth_at_entry[entry] = 0;
    min_depth_at_entry[entry] = 0;

    u32 worklist[WCET_MAX_BLOCKS];
    u32 wl_head = 0, wl_tail = 0;
    worklist[wl_tail++] = entry;

    i32 global_max = 0;
    i32 global_min = 0;
    i32 exit_stack = 0;
    u32 max_iter = g_wcet_block_count * 10;
    u32 iter = 0;

    while (wl_head < wl_tail && iter < max_iter) {
        iter++;
        u32 b = worklist[wl_head++];
        WCETBlock& block = g_wcet_blocks[b];
        if (block.is_unreachable) continue;

        i32 entry_max = max_depth_at_entry[b];
        i32 entry_min = min_depth_at_entry[b];
        i32 peak = entry_max + block.max_stack_growth;
        if (peak > global_max) global_max = peak;

        i32 exit_max = entry_max + block.stack_delta;
        i32 exit_min = entry_min + block.stack_delta;
        if (exit_min < global_min) global_min = exit_min;

        if (block.is_exit || block.is_return) exit_stack = exit_max;

        for (u32 e = 0; e < g_wcet_edge_count; e++) {
            if (g_wcet_edges[e].from_block != b) continue;
            u32 succ = g_wcet_edges[e].to_block;
            bool changed = false;
            if (exit_max > max_depth_at_entry[succ]) { max_depth_at_entry[succ] = exit_max; changed = true; }
            if (exit_min < min_depth_at_entry[succ]) { min_depth_at_entry[succ] = exit_min; changed = true; }
            if (changed) {
                if (processed[succ]) {
                    i32 diff = max_depth_at_entry[succ] - min_depth_at_entry[succ];
                    if (diff > 1 || diff < -1) g_wcet_warn_unbalanced = 1;
                }
                if (wl_tail < WCET_MAX_BLOCKS) worklist[wl_tail++] = succ;
            }
            processed[succ] = true;
        }
    }

    g_wcet_max_stack_depth = global_max;
    g_wcet_min_stack_depth = global_min;
    g_wcet_stack_at_exit = exit_stack;
    g_wcet_warn_stack_overflow = (global_max > (i32)max_stack_size) ? 1 : 0;
    g_wcet_warn_stack_underflow = (global_min < 0) ? 1 : 0;
}

void wcet_analyze_call_depth() {
    if (g_wcet_block_count == 0) return;

    u32 max_depth = 0;
    for (u32 e = 0; e < g_wcet_edge_count; e++) {
        if (g_wcet_edges[e].type == EDGE_CALL && g_wcet_edges[e].is_back_edge) {
            g_wcet_has_recursion = 1;
        }
    }

    struct CallFrame { u32 block_id; u32 call_depth; };
    CallFrame queue[WCET_MAX_BLOCKS];
    u32 q_head = 0, q_tail = 0;

    u32 entry = 0;
    for (u32 b = 0; b < g_wcet_block_count; b++) {
        if (g_wcet_blocks[b].is_entry) { entry = b; break; }
    }

    bool visited[WCET_MAX_BLOCKS];
    for (u32 i = 0; i < WCET_MAX_BLOCKS; i++) visited[i] = false;
    queue[q_tail++] = { entry, 0 };
    visited[entry] = true;

    while (q_head < q_tail) {
        CallFrame frame = queue[q_head++];
        if (frame.call_depth > max_depth) max_depth = frame.call_depth;
        for (u32 e = 0; e < g_wcet_edge_count; e++) {
            if (g_wcet_edges[e].from_block != frame.block_id) continue;
            u32 next = g_wcet_edges[e].to_block;
            if (visited[next]) continue;
            visited[next] = true;
            u32 next_depth = frame.call_depth;
            if (g_wcet_edges[e].type == EDGE_CALL) next_depth++;
            if (q_tail < WCET_MAX_BLOCKS) queue[q_tail++] = { next, next_depth };
        }
    }
    g_wcet_max_call_depth = max_depth;
}

void wcet_analyze_loop_termination() {
    for (u32 l = 0; l < g_wcet_loop_count; l++) {
        u32 header = g_wcet_loops[l].header_block;
        u32 back = g_wcet_loops[l].back_edge_block;
        bool has_exit = false;
        for (u32 e = 0; e < g_wcet_edge_count; e++) {
            if (g_wcet_edges[e].from_block == header && g_wcet_edges[e].to_block != back && !g_wcet_edges[e].is_back_edge) { has_exit = true; break; }
            if (g_wcet_edges[e].from_block == back && g_wcet_edges[e].to_block != header) { has_exit = true; break; }
        }
        if (!has_exit) g_wcet_warn_infinite_loop = 1;
    }
}

void wcet_count_warnings() {
    g_wcet_warning_count = 0;
    if (g_wcet_warn_dead_code) g_wcet_warning_count++;
    if (g_wcet_warn_infinite_loop) g_wcet_warning_count++;
    if (g_wcet_warn_stack_overflow) g_wcet_warning_count++;
    if (g_wcet_warn_stack_underflow) g_wcet_warning_count++;
    if (g_wcet_warn_unbalanced) g_wcet_warning_count++;
    if (g_wcet_has_recursion) g_wcet_warning_count++;
}

// ============================================================================
// Main entry point
// ============================================================================
bool wcet_analyze(const u8* bytecode, u32 length, u32 max_stack_size) {
    wcet_report_reset();
    if (length == 0) return false;

    g_wcet_bytecode_size = length;

    if (!wcet_cfg_build(bytecode, length)) return false;

    g_wcet_rpt_instruction_count = g_wcet_instruction_count;
    g_wcet_rpt_block_count = g_wcet_block_count;
    g_wcet_rpt_edge_count = g_wcet_edge_count;
    g_wcet_rpt_loop_count = g_wcet_loop_count;
    g_wcet_rpt_unreachable = g_wcet_unreachable_block_count;
    g_wcet_rpt_dead_bytes = g_wcet_dead_code_bytes;
    g_wcet_warn_dead_code = (g_wcet_unreachable_block_count > 0) ? 1 : 0;

    wcet_analyze_opcode_frequency();
    wcet_analyze_paths();
    wcet_analyze_stack_depth(max_stack_size);
    wcet_analyze_call_depth();
    wcet_analyze_loop_termination();
    wcet_count_warnings();

    // Compute nanosecond estimates if a calibrated target profile is active
    if (g_wcet_active_profile->clock_mhz > 0) {
        g_wcet_has_target = 1;

        // Walk the BCET/WCET paths using per-instruction ns costs
        // For simplicity, sum ns contributions across all instructions per block,
        // then use the same path analysis results (bcet/wcet are proportional)
        u32 total_wcet_ns = 0;

        // Sum ns for each instruction in the program
        for (u32 i = 0; i < g_wcet_instruction_count; i++) {
            u8 opcode = g_wcet_instructions[i].opcode;
            u8 type_arg = g_wcet_instructions[i].type_arg;
            u32 ns = wcet_target_ns_for_opcode(opcode, type_arg);
            // Track per-instruction ns (we'll scale by path ratio)
            total_wcet_ns += ns;
        }

        // Use the ratio of WCET/BCET cycles to scale ns estimates
        if (g_wcet_wcet_cycles > 0 && g_wcet_bcet_cycles > 0) {
            // Total ns is for ALL instructions; scale by path coverage
            u32 total_instructions = g_wcet_rpt_instruction_count;
            if (total_instructions > 0) {
                u32 ns_per_instruction_avg = total_wcet_ns / total_instructions;
                g_wcet_bcet_ns = ns_per_instruction_avg * g_wcet_bcet_instructions;
                g_wcet_wcet_ns = ns_per_instruction_avg * g_wcet_wcet_instructions;
            }
        } else if (g_wcet_wcet_instructions > 0) {
            g_wcet_wcet_ns = total_wcet_ns;
            g_wcet_bcet_ns = total_wcet_ns;
        }
    }

    return true;
}

// ============================================================================
// Print report to stdout
// ============================================================================
void wcet_print_report() {
    printf("\n");
    printf("+--------------------------------------------------------------+\n");
    printf("|             WCET Analysis Report                             |\n");
    printf("+--------------------------------------------------------------+\n");

    printf("| Program Metrics                                              |\n");
    printf("|   Bytecode size:       %6u bytes                          |\n", g_wcet_bytecode_size);
    printf("|   Instructions:        %6u                                |\n", g_wcet_rpt_instruction_count);
    printf("|   Unique opcodes:      %6u                                |\n", g_wcet_unique_opcodes);
    printf("|   Basic blocks:        %6u                                |\n", g_wcet_rpt_block_count);
    printf("|   CFG edges:           %6u                                |\n", g_wcet_rpt_edge_count);
    printf("|   Loops detected:      %6u                                |\n", g_wcet_rpt_loop_count);
    printf("+--------------------------------------------------------------+\n");

    printf("| Timing Analysis (cycle units)                                |\n");
    printf("|   BCET (best case):    %6u CU  (%u instructions)         \n", g_wcet_bcet_cycles, g_wcet_bcet_instructions);
    printf("|   WCET (worst case):   %6u CU  (%u instructions)         \n", g_wcet_wcet_cycles, g_wcet_wcet_instructions);
    if (g_wcet_bcet_cycles > 0) {
        u32 ratio = (g_wcet_wcet_cycles * 100) / g_wcet_bcet_cycles;
        printf("|   WCET/BCET ratio:     %3u.%02u%%                              \n", ratio / 100, ratio % 100);
    }
    printf("+--------------------------------------------------------------+\n");

    if (g_wcet_has_target) {
        printf("| Target Profile: %-42s |\n", g_wcet_active_profile->name);
        printf("|   Architecture:    %-40s |\n", g_wcet_active_profile->arch_name);
        printf("|   Clock:           %6u MHz                                |\n", g_wcet_active_profile->clock_mhz);
        printf("|   Match quality:   %6u (%s)\n", g_wcet_target_match_quality, g_wcet_target_match_reason);
        printf("|   BCET (estimated): %6u ns  (%.3f us)                   \n", g_wcet_bcet_ns, (float)g_wcet_bcet_ns / 1000.0f);
        printf("|   WCET (estimated): %6u ns  (%.3f us)                   \n", g_wcet_wcet_ns, (float)g_wcet_wcet_ns / 1000.0f);
        printf("+--------------------------------------------------------------+\n");
    }

    printf("| Stack Analysis                                               |\n");
    printf("|   Max stack depth:     %6d bytes                          |\n", g_wcet_max_stack_depth);
    printf("|   Stack at exit:       %6d bytes                          |\n", g_wcet_stack_at_exit);
    printf("|   Max call depth:      %6u                                |\n", g_wcet_max_call_depth);
    printf("+--------------------------------------------------------------+\n");

    printf("| Path Analysis                                                |\n");
    printf("|   Execution paths:     %6u                                |\n", g_wcet_total_paths);
    printf("|   Shortest path:       %6u blocks                         |\n", g_wcet_shortest_path);
    printf("|   Longest path:        %6u blocks                         |\n", g_wcet_longest_path);
    printf("+--------------------------------------------------------------+\n");

    if (g_wcet_warn_dead_code || g_wcet_warn_infinite_loop || g_wcet_warn_stack_overflow ||
        g_wcet_warn_stack_underflow || g_wcet_warn_unbalanced || g_wcet_has_recursion) {
        printf("| ! Warnings (%u)                                             \n", g_wcet_warning_count);
        if (g_wcet_warn_dead_code)
            printf("|   DEAD_CODE: %u unreachable blocks (%u bytes)              \n",
                   g_wcet_rpt_unreachable, g_wcet_rpt_dead_bytes);
        if (g_wcet_warn_infinite_loop)
            printf("|   INFINITE_LOOP: Loop with no exit path detected            |\n");
        if (g_wcet_warn_stack_overflow)
            printf("|   STACK_OVERFLOW: Max depth %d may exceed limit             \n", g_wcet_max_stack_depth);
        if (g_wcet_warn_stack_underflow)
            printf("|   STACK_UNDERFLOW: Stack depth may go negative              |\n");
        if (g_wcet_warn_unbalanced)
            printf("|   UNBALANCED_STACK: Different depths at merge points        |\n");
        if (g_wcet_has_recursion)
            printf("|   RECURSION: Recursive call pattern detected                |\n");
    } else {
        printf("| No warnings                                                  |\n");
    }
    printf("+--------------------------------------------------------------+\n");

    printf("| Top Opcodes by WCET Contribution                             |\n");
    u8 show_count = g_wcet_opcode_freq_count < 10 ? g_wcet_opcode_freq_count : 10;
    for (u8 i = 0; i < show_count; i++) {
        const WCETOpcodeFreq& f = g_wcet_opcode_freq[i];
        if (f.count == 0) break;
        const char* name = (const char*)OPCODE_NAME((PLCRuntimeInstructionSet)f.opcode);
        char name_buf[20];
        int j = 0;
        if (name) { while (name[j] && j < 19) { name_buf[j] = name[j]; j++; } }
        name_buf[j] = '\0';
        printf("|   %-16s  x%-4u  BCET:%5u  WCET:%5u                \n",
               name_buf, f.count, f.total_bcet, f.total_wcet);
    }
    printf("+--------------------------------------------------------------+\n\n");
}

#endif // __WASM__
