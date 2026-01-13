#pragma once

#include "../runtime-interval.h" 
#include "../runtime-types.h"

union u8A_to_u32 { u8 u8A[4]; u32 val; };

// Timers Memory Layout
// [0..3] ET (u32)
// [4..7] StartTime (u32)
// [8]    Flags (u8)
//        Bit 0: Q
//        Bit 1: RUNNING
//        Bit 2: IN_OLD (Used for TP/TOF)

#define TIMER_FLAG_Q 0x01
#define TIMER_FLAG_RUNNING 0x02
#define TIMER_FLAG_IN_OLD 0x04

#define TIMER_OFFSET_ET 0
#define TIMER_OFFSET_START 4
#define TIMER_OFFSET_FLAGS 8
#define TIMER_STRUCT_SIZE 9

namespace PLCMethods {
    
    // Helper to read u32 from memory array
    inline u32 read_u32_from_mem(u8* memory, MY_PTR_t address) {
        u8A_to_u32 converter;
        converter.u8A[0] = memory[address + 0];
        converter.u8A[1] = memory[address + 1];
        converter.u8A[2] = memory[address + 2];
        converter.u8A[3] = memory[address + 3];
        return converter.val;
    }

    inline void write_u32_to_mem(u8* memory, MY_PTR_t address, u32 value) {
        u8A_to_u32 converter;
        converter.val = value;
        // The array might be optimized out if I assign to val and read val?
        // No, I need to write FROM val TO memory.
        memory[address + 0] = converter.u8A[0];
        memory[address + 1] = converter.u8A[1];
        memory[address + 2] = converter.u8A[2];
        memory[address + 3] = converter.u8A[3];
    }

    void logic_TON(u8* memory, MY_PTR_t timer_ptr, u32 PT, bool IN, bool& Q_out) {
        u8 flags = memory[timer_ptr + TIMER_OFFSET_FLAGS];
        u32 start_time = read_u32_from_mem(memory, timer_ptr + TIMER_OFFSET_START);
        u32 et = 0;

        // DEBUG
        // Serial.printf("TON: IN=%d, Flags=%02X, Start=%u, Now=%u, PT=%u\n", IN, flags, start_time, interval_millis_now, PT);

        if (IN) {
            if (!(flags & TIMER_FLAG_RUNNING)) {
                start_time = interval_millis_now;
                flags |= TIMER_FLAG_RUNNING;
                write_u32_to_mem(memory, timer_ptr + TIMER_OFFSET_START, start_time);
            }
            et = interval_millis_now - start_time;
            if (et >= PT) {
                flags |= TIMER_FLAG_Q;
                et = PT;
            } else {
                flags &= ~TIMER_FLAG_Q;
            }
        } else {
            flags &= ~(TIMER_FLAG_RUNNING | TIMER_FLAG_Q);
            et = 0;
        }

        write_u32_to_mem(memory, timer_ptr + TIMER_OFFSET_ET, et);
        memory[timer_ptr + TIMER_OFFSET_FLAGS] = flags;
        Q_out = (flags & TIMER_FLAG_Q) ? true : false;
    }

    void logic_TOF(u8* memory, MY_PTR_t timer_ptr, u32 PT, bool IN, bool& Q_out) {
        u8 flags = memory[timer_ptr + TIMER_OFFSET_FLAGS];
        
        bool prev_IN = (flags & TIMER_FLAG_IN_OLD) ? true : false;
        bool running = (flags & TIMER_FLAG_RUNNING) ? true : false;
        u32 start_time = read_u32_from_mem(memory, timer_ptr + TIMER_OFFSET_START);
        u32 et = 0;
        
        if (IN) {
            Q_out = true;
            running = false;
            et = 0;
        } else {
            if (prev_IN) { // Falling Edge
                running = true;
                start_time = interval_millis_now;
                write_u32_to_mem(memory, timer_ptr + TIMER_OFFSET_START, start_time);
            }
            
            if (running) {
                et = interval_millis_now - start_time;
                if (et >= PT) {
                    running = false;
                    et = PT;
                    Q_out = false;
                } else {
                    Q_out = true;
                }
            } else {
                et = PT; // Assume ET stays at PT when done? Or 0? TOF usually ET=PT when Q=0.
                Q_out = false;
            }
        }

        // Update Flags
        flags = (flags & ~TIMER_FLAG_IN_OLD) | (IN ? TIMER_FLAG_IN_OLD : 0);
        flags = (flags & ~TIMER_FLAG_RUNNING) | (running ? TIMER_FLAG_RUNNING : 0);
        flags = (flags & ~TIMER_FLAG_Q) | (Q_out ? TIMER_FLAG_Q : 0);

        write_u32_to_mem(memory, timer_ptr + TIMER_OFFSET_ET, et);
        memory[timer_ptr + TIMER_OFFSET_FLAGS] = flags;
    }

    void logic_TP(u8* memory, MY_PTR_t timer_ptr, u32 PT, bool IN, bool& Q_out) {
        u8 flags = memory[timer_ptr + TIMER_OFFSET_FLAGS];
        
        bool prev_IN = (flags & TIMER_FLAG_IN_OLD) ? true : false;
        bool running = (flags & TIMER_FLAG_RUNNING) ? true : false;
        u32 start_time = read_u32_from_mem(memory, timer_ptr + TIMER_OFFSET_START);
        u32 et = 0;

        if (!prev_IN && IN && !running) {
             running = true;
             start_time = interval_millis_now;
             write_u32_to_mem(memory, timer_ptr + TIMER_OFFSET_START, start_time);
        }

        if (running) {
            et = interval_millis_now - start_time;
            if (et >= PT) {
                running = false;
                et = PT;
            }
        } else {
            et = 0; // Or PT? Usually TP resets ET when not active and IN is false?
            // If IN is true but Pulse finished, ET = PT usually. 
            // If IN goes false, and Pulse finished, ET = 0?
            if (!IN) et = 0;
            else et = PT; // Holding logic
        }

        Q_out = running;

        flags = (flags & ~TIMER_FLAG_IN_OLD) | (IN ? TIMER_FLAG_IN_OLD : 0);
        flags = (flags & ~TIMER_FLAG_RUNNING) | (running ? TIMER_FLAG_RUNNING : 0);
        flags = (flags & ~TIMER_FLAG_Q) | (Q_out ? TIMER_FLAG_Q : 0);

        write_u32_to_mem(memory, timer_ptr + TIMER_OFFSET_ET, et);
        memory[timer_ptr + TIMER_OFFSET_FLAGS] = flags;
    }


    // TON_CONST
    RuntimeError handle_TON_CONST(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        MY_PTR_t timer_ptr = 0;
        RuntimeError err = ProgramExtract.type_pointer(program, prog_size, index, &timer_ptr);
        if (err != STATUS_SUCCESS) return err;

        u32 PT = 0;
        err = ProgramExtract.type_u32(program, prog_size, index, &PT);
        if (err != STATUS_SUCCESS) return err;

        if (stack.size() == 0) return EMPTY_STACK;
        bool IN = stack.pop_bool();
        if (timer_ptr + TIMER_STRUCT_SIZE > PLCRUNTIME_MAX_MEMORY_SIZE) return MEMORY_ACCESS_ERROR;

        bool Q = false;
        logic_TON(memory, timer_ptr, PT, IN, Q);
        return stack.push_bool(Q);
    }
    
    // TON_MEM
    RuntimeError handle_TON_MEM(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        MY_PTR_t timer_ptr = 0;
        RuntimeError err = ProgramExtract.type_pointer(program, prog_size, index, &timer_ptr);
        if (err != STATUS_SUCCESS) return err;

        MY_PTR_t pt_ptr = 0;
        err = ProgramExtract.type_pointer(program, prog_size, index, &pt_ptr);
        if (err != STATUS_SUCCESS) return err;
        
        if (pt_ptr + 4 > PLCRUNTIME_MAX_MEMORY_SIZE) return MEMORY_ACCESS_ERROR;
        u32 PT = read_u32_from_mem(memory, pt_ptr);

        if (stack.size() == 0) return EMPTY_STACK;
        bool IN = stack.pop_bool();
        if (timer_ptr + TIMER_STRUCT_SIZE > PLCRUNTIME_MAX_MEMORY_SIZE) return MEMORY_ACCESS_ERROR;

        bool Q = false;
        logic_TON(memory, timer_ptr, PT, IN, Q);
        return stack.push_bool(Q);
    }

    // TOF_CONST
    RuntimeError handle_TOF_CONST(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        MY_PTR_t timer_ptr = 0;
        RuntimeError err = ProgramExtract.type_pointer(program, prog_size, index, &timer_ptr);
        if (err != STATUS_SUCCESS) return err;

        u32 PT = 0;
        err = ProgramExtract.type_u32(program, prog_size, index, &PT);
        if (err != STATUS_SUCCESS) return err;

        if (stack.size() == 0) return EMPTY_STACK;
        bool IN = stack.pop_bool();
        if (timer_ptr + TIMER_STRUCT_SIZE > PLCRUNTIME_MAX_MEMORY_SIZE) return MEMORY_ACCESS_ERROR;

        bool Q = false;
        logic_TOF(memory, timer_ptr, PT, IN, Q);
        return stack.push_bool(Q);
    }

    // TOF_MEM
    RuntimeError handle_TOF_MEM(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        MY_PTR_t timer_ptr = 0;
        RuntimeError err = ProgramExtract.type_pointer(program, prog_size, index, &timer_ptr);
        if (err != STATUS_SUCCESS) return err;

        MY_PTR_t pt_ptr = 0;
        err = ProgramExtract.type_pointer(program, prog_size, index, &pt_ptr);
        if (err != STATUS_SUCCESS) return err;
        
        if (pt_ptr + 4 > PLCRUNTIME_MAX_MEMORY_SIZE) return MEMORY_ACCESS_ERROR;
        u32 PT = read_u32_from_mem(memory, pt_ptr);

        if (stack.size() == 0) return EMPTY_STACK;
        bool IN = stack.pop_bool();
        if (timer_ptr + TIMER_STRUCT_SIZE > PLCRUNTIME_MAX_MEMORY_SIZE) return MEMORY_ACCESS_ERROR;

        bool Q = false;
        logic_TOF(memory, timer_ptr, PT, IN, Q);
        return stack.push_bool(Q);
    }

    // TP_CONST
    RuntimeError handle_TP_CONST(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        MY_PTR_t timer_ptr = 0;
        RuntimeError err = ProgramExtract.type_pointer(program, prog_size, index, &timer_ptr);
        if (err != STATUS_SUCCESS) return err;

        u32 PT = 0;
        err = ProgramExtract.type_u32(program, prog_size, index, &PT);
        if (err != STATUS_SUCCESS) return err;

        if (stack.size() == 0) return EMPTY_STACK;
        bool IN = stack.pop_bool();
        if (timer_ptr + TIMER_STRUCT_SIZE > PLCRUNTIME_MAX_MEMORY_SIZE) return MEMORY_ACCESS_ERROR;

        bool Q = false;
        logic_TP(memory, timer_ptr, PT, IN, Q);
        return stack.push_bool(Q);
    }

    // TP_MEM
    RuntimeError handle_TP_MEM(RuntimeStack& stack, u8* memory, u8* program, u32 prog_size, u32& index) {
        MY_PTR_t timer_ptr = 0;
        RuntimeError err = ProgramExtract.type_pointer(program, prog_size, index, &timer_ptr);
        if (err != STATUS_SUCCESS) return err;

        MY_PTR_t pt_ptr = 0;
        err = ProgramExtract.type_pointer(program, prog_size, index, &pt_ptr);
        if (err != STATUS_SUCCESS) return err;
        
        if (pt_ptr + 4 > PLCRUNTIME_MAX_MEMORY_SIZE) return MEMORY_ACCESS_ERROR;
        u32 PT = read_u32_from_mem(memory, pt_ptr);

        if (stack.size() == 0) return EMPTY_STACK;
        bool IN = stack.pop_bool();
        if (timer_ptr + TIMER_STRUCT_SIZE > PLCRUNTIME_MAX_MEMORY_SIZE) return MEMORY_ACCESS_ERROR;

        bool Q = false;
        logic_TP(memory, timer_ptr, PT, IN, Q);
        return stack.push_bool(Q);
    }
}
