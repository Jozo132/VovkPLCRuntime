#pragma once

#include "../runtime-types.h"
#include "methods-timer.h" // For read_u32_from_mem, write_u32_to_mem helpers

// Counter Memory Layout (IEC 61131-3 CTU/CTD)
// [0..3] CV (u32) - Current Value
// [4]    Flags (u8)
//        Bit 0: Q (Output)
//        Bit 1: CU_OLD or CD_OLD (Edge detection for count input)

#define COUNTER_FLAG_Q 0x01
#define COUNTER_FLAG_INPUT_OLD 0x02

#define COUNTER_OFFSET_CV 0
#define COUNTER_OFFSET_FLAGS 4
#define COUNTER_STRUCT_SIZE 5

// Counter Structure Property Access
// These define the accessible properties of a counter for property access syntax
// Property: offset within struct, size in bytes (0 = bit), bit position (255 = not a bit)
// Usage: C0.Q, C0.CV, my_counter.Q, my_counter.CV
//
// Properties:
//   .Q   - Output bit (true when CV >= PV)
//   .CV  - Current Value (u32, read/write)
//   .IN  - Previous input state (bit, internal use)

// Property identifiers for counter struct
#define COUNTER_PROP_Q     0   // Q output bit - offset 4 (FLAGS), bit 0
#define COUNTER_PROP_CV    1   // Current Value - offset 0, u32
#define COUNTER_PROP_IN    2   // Previous IN state - offset 4 (FLAGS), bit 1

namespace PLCMethods {

    /**
     * CTU - Counter Up (IEC 61131-3)
     * 
     * Inputs (from stack, top to bottom):
     *   - CU (bool): Count Up input - counts on rising edge
     *   - R (bool): Reset input - resets CV to 0
     * 
     * Parameters:
     *   - PV (u32): Preset Value - threshold for Q output
     * 
     * Output (to stack):
     *   - Q (bool): Output - true when CV >= PV
     * 
     * Memory:
     *   - CV (u32): Current counter value (stored in counter struct)
     */
    void logic_CTU(u8* memory, MY_PTR_t counter_ptr, u32 PV, bool CU, bool R, bool& Q_out) {
        u8 flags = memory[counter_ptr + COUNTER_OFFSET_FLAGS];
        u32 cv = read_u32_from_mem(memory, counter_ptr + COUNTER_OFFSET_CV);
        
        bool cu_old = (flags & COUNTER_FLAG_INPUT_OLD) ? true : false;
        bool cu_rising_edge = CU && !cu_old;
        
        if (R) {
            // Reset takes priority
            cv = 0;
        } else if (cu_rising_edge) {
            // Count up on rising edge of CU
            if (cv < 0xFFFFFFFF) { // Prevent overflow
                cv++;
            }
        }
        
        // Output is true when CV >= PV
        Q_out = (cv >= PV);
        
        // Update flags
        flags = (flags & ~COUNTER_FLAG_INPUT_OLD) | (CU ? COUNTER_FLAG_INPUT_OLD : 0);
        flags = (flags & ~COUNTER_FLAG_Q) | (Q_out ? COUNTER_FLAG_Q : 0);
        
        write_u32_to_mem(memory, counter_ptr + COUNTER_OFFSET_CV, cv);
        memory[counter_ptr + COUNTER_OFFSET_FLAGS] = flags;
    }

    /**
     * CTD - Counter Down (IEC 61131-3)
     * 
     * Inputs (from stack, top to bottom):
     *   - CD (bool): Count Down input - counts down on rising edge
     *   - LD (bool): Load input - loads PV into CV
     * 
     * Parameters:
     *   - PV (u32): Preset Value - value to load when LD is true
     * 
     * Output (to stack):
     *   - Q (bool): Output - true when CV <= 0
     * 
     * Memory:
     *   - CV (u32): Current counter value (stored in counter struct)
     */
    void logic_CTD(u8* memory, MY_PTR_t counter_ptr, u32 PV, bool CD, bool LD, bool& Q_out) {
        u8 flags = memory[counter_ptr + COUNTER_OFFSET_FLAGS];
        u32 cv = read_u32_from_mem(memory, counter_ptr + COUNTER_OFFSET_CV);
        
        bool cd_old = (flags & COUNTER_FLAG_INPUT_OLD) ? true : false;
        bool cd_rising_edge = CD && !cd_old;
        
        if (LD) {
            // Load takes priority - load PV into CV
            cv = PV;
        } else if (cd_rising_edge) {
            // Count down on rising edge of CD
            if (cv > 0) { // Prevent underflow
                cv--;
            }
        }
        
        // Output is true when CV <= 0
        Q_out = (cv == 0);
        
        // Update flags
        flags = (flags & ~COUNTER_FLAG_INPUT_OLD) | (CD ? COUNTER_FLAG_INPUT_OLD : 0);
        flags = (flags & ~COUNTER_FLAG_Q) | (Q_out ? COUNTER_FLAG_Q : 0);
        
        write_u32_to_mem(memory, counter_ptr + COUNTER_OFFSET_CV, cv);
        memory[counter_ptr + COUNTER_OFFSET_FLAGS] = flags;
    }

    // CTU_CONST - Counter Up with constant preset value
    // Stack: [R, CU] (R on top, CU below) -> [Q]
    RuntimeError handle_CTU_CONST(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        MY_PTR_t counter_ptr = 0;
        RuntimeError err = ProgramExtract.type_pointer(program, prog_size, index, &counter_ptr);
        if (err != STATUS_SUCCESS) return err;
        counter_ptr = reverse_byte_order(counter_ptr);

        u32 PV = 0;
        err = ProgramExtract.type_u32(program, prog_size, index, &PV);
        if (err != STATUS_SUCCESS) return err;

        if (stack.size() < 2) return EMPTY_STACK;
        bool R = stack.pop_bool();  // Reset (top of stack)
        bool CU = stack.pop_bool(); // Count Up
        
        if (counter_ptr + COUNTER_STRUCT_SIZE > PLCRUNTIME_MAX_MEMORY_SIZE) return MEMORY_ACCESS_ERROR;

        bool Q = false;
        logic_CTU(memory, counter_ptr, PV, CU, R, Q);
        return stack.push_bool(Q);
    }

    // CTU_MEM - Counter Up with memory-based preset value
    // Stack: [R, CU] (R on top, CU below) -> [Q]
    RuntimeError handle_CTU_MEM(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        MY_PTR_t counter_ptr = 0;
        RuntimeError err = ProgramExtract.type_pointer(program, prog_size, index, &counter_ptr);
        if (err != STATUS_SUCCESS) return err;
        counter_ptr = reverse_byte_order(counter_ptr);

        MY_PTR_t pv_ptr = 0;
        err = ProgramExtract.type_pointer(program, prog_size, index, &pv_ptr);
        if (err != STATUS_SUCCESS) return err;
        pv_ptr = reverse_byte_order(pv_ptr);

        if (pv_ptr + 4 > PLCRUNTIME_MAX_MEMORY_SIZE) return MEMORY_ACCESS_ERROR;
        u32 PV = read_u32_from_mem(memory, pv_ptr);

        if (stack.size() < 2) return EMPTY_STACK;
        bool R = stack.pop_bool();  // Reset (top of stack)
        bool CU = stack.pop_bool(); // Count Up
        
        if (counter_ptr + COUNTER_STRUCT_SIZE > PLCRUNTIME_MAX_MEMORY_SIZE) return MEMORY_ACCESS_ERROR;

        bool Q = false;
        logic_CTU(memory, counter_ptr, PV, CU, R, Q);
        return stack.push_bool(Q);
    }

    // CTD_CONST - Counter Down with constant preset value
    // Stack: [LD, CD] (LD on top, CD below) -> [Q]
    RuntimeError handle_CTD_CONST(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        MY_PTR_t counter_ptr = 0;
        RuntimeError err = ProgramExtract.type_pointer(program, prog_size, index, &counter_ptr);
        if (err != STATUS_SUCCESS) return err;
        counter_ptr = reverse_byte_order(counter_ptr);

        u32 PV = 0;
        err = ProgramExtract.type_u32(program, prog_size, index, &PV);
        if (err != STATUS_SUCCESS) return err;

        if (stack.size() < 2) return EMPTY_STACK;
        bool LD = stack.pop_bool(); // Load (top of stack)
        bool CD = stack.pop_bool(); // Count Down
        
        if (counter_ptr + COUNTER_STRUCT_SIZE > PLCRUNTIME_MAX_MEMORY_SIZE) return MEMORY_ACCESS_ERROR;

        bool Q = false;
        logic_CTD(memory, counter_ptr, PV, CD, LD, Q);
        return stack.push_bool(Q);
    }

    // CTD_MEM - Counter Down with memory-based preset value
    // Stack: [LD, CD] (LD on top, CD below) -> [Q]
    RuntimeError handle_CTD_MEM(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        MY_PTR_t counter_ptr = 0;
        RuntimeError err = ProgramExtract.type_pointer(program, prog_size, index, &counter_ptr);
        if (err != STATUS_SUCCESS) return err;
        counter_ptr = reverse_byte_order(counter_ptr);

        MY_PTR_t pv_ptr = 0;
        err = ProgramExtract.type_pointer(program, prog_size, index, &pv_ptr);
        if (err != STATUS_SUCCESS) return err;
        pv_ptr = reverse_byte_order(pv_ptr);

        if (pv_ptr + 4 > PLCRUNTIME_MAX_MEMORY_SIZE) return MEMORY_ACCESS_ERROR;
        u32 PV = read_u32_from_mem(memory, pv_ptr);

        if (stack.size() < 2) return EMPTY_STACK;
        bool LD = stack.pop_bool(); // Load (top of stack)
        bool CD = stack.pop_bool(); // Count Down
        
        if (counter_ptr + COUNTER_STRUCT_SIZE > PLCRUNTIME_MAX_MEMORY_SIZE) return MEMORY_ACCESS_ERROR;

        bool Q = false;
        logic_CTD(memory, counter_ptr, PV, CD, LD, Q);
        return stack.push_bool(Q);
    }
}
