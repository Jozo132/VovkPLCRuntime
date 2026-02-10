// runtime-datablock.h - 2026-02-10
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

// ============================================================================
// DataBlock (DB) Management
// ============================================================================
//
// DataBlocks provide named memory regions within the PLC data memory area.
// They are aligned to the END of the memory area, with a fixed-size lookup
// table occupying the very last bytes and actual DB data sitting just before it.
//
// Memory layout with DataBlocks:
//   [System][Inputs][Outputs][Markers][Timers][Counters]..free..[DB data][DB Lookup Table]
//   ^--- 0                                                                          ^--- PLCRUNTIME_MAX_MEMORY_SIZE
//
// DB Lookup Table Entry (6 bytes each):
//   { db: u16, offset: u16, size: u16 }
//   - db = 0 means the slot is unused
//   - offset = absolute memory address of the DB data region
//   - size = number of bytes in this DB
//
// The lookup table sits at the very end of memory, and DB data regions
// are allocated downward from the bottom of the lookup table.
//
// Default: 16 datablock slots (configurable via PLCRUNTIME_NUM_OF_DATABLOCKS)
// Total lookup table size: 16 * 6 = 96 bytes
//
// DataBlocks are:
//  - Declared by runtime bytecode on P_First_Cycle via CONFIG_DB instruction
//  - Managed over Serial API for live updates when program changes
//  - Migrated (relocated) for seamless program patching

#ifndef PLCRUNTIME_NUM_OF_DATABLOCKS
#define PLCRUNTIME_NUM_OF_DATABLOCKS 16
#endif // PLCRUNTIME_NUM_OF_DATABLOCKS

#define PLCRUNTIME_DB_ENTRY_SIZE 6  // sizeof(u16 db + u16 offset + u16 size)
#define PLCRUNTIME_DB_TABLE_SIZE (PLCRUNTIME_NUM_OF_DATABLOCKS * PLCRUNTIME_DB_ENTRY_SIZE)

// The DB lookup table starts at this offset (end of memory minus table)
#define PLCRUNTIME_DB_TABLE_OFFSET (PLCRUNTIME_MAX_MEMORY_SIZE - PLCRUNTIME_DB_TABLE_SIZE)

// ============================================================================
// DataBlock Manager
// ============================================================================
// Operates directly on the runtime memory[] array. No separate allocation.
// The manager reads/writes the DB lookup table at the end of memory and
// manages DB data regions that grow downward from the table.

struct DataBlockManager {
    u8* memory;           // Pointer to runtime memory[]
    u32 memory_size;      // PLCRUNTIME_MAX_MEMORY_SIZE
    u16 num_slots;        // PLCRUNTIME_NUM_OF_DATABLOCKS
    u16 table_offset;     // Start of the DB lookup table in memory
    u16 user_area_end;    // End of the "regular" memory areas (counters end), start of free region

    // Initialize the manager (call once after memory is available)
    void init(u8* mem, u32 mem_size, u16 slots, u16 area_end) {
        memory = mem;
        memory_size = mem_size;
        num_slots = slots;
        table_offset = (u16)(mem_size - (u32)slots * PLCRUNTIME_DB_ENTRY_SIZE);
        user_area_end = area_end;
    }

    // Format (clear) all DB entries - sets all slots to unused (db=0)
    void format() {
        for (u16 i = 0; i < num_slots; i++) {
            u16 entry_addr = table_offset + i * PLCRUNTIME_DB_ENTRY_SIZE;
            write_u16(memory + entry_addr + 0, 0); // db = 0 (unused)
            write_u16(memory + entry_addr + 2, 0); // offset = 0
            write_u16(memory + entry_addr + 4, 0); // size = 0
        }
    }

    // ========================================================================
    // Lookup table accessors
    // ========================================================================

    // Read a DB entry by slot index
    // Returns false if slot index is out of range
    bool getEntry(u16 slot, u16& db, u16& offset, u16& size) const {
        if (slot >= num_slots) return false;
        u16 entry_addr = table_offset + slot * PLCRUNTIME_DB_ENTRY_SIZE;
        db     = read_u16(memory + entry_addr + 0);
        offset = read_u16(memory + entry_addr + 2);
        size   = read_u16(memory + entry_addr + 4);
        return true;
    }

    // Write a DB entry by slot index
    bool setEntry(u16 slot, u16 db, u16 offset, u16 size) {
        if (slot >= num_slots) return false;
        u16 entry_addr = table_offset + slot * PLCRUNTIME_DB_ENTRY_SIZE;
        write_u16(memory + entry_addr + 0, db);
        write_u16(memory + entry_addr + 2, offset);
        write_u16(memory + entry_addr + 4, size);
        return true;
    }

    // Find the slot index for a given DB number
    // Returns -1 if not found
    i16 findSlot(u16 db_number) const {
        if (db_number == 0) return -1; // DB 0 is reserved as "unused"
        for (u16 i = 0; i < num_slots; i++) {
            u16 db, offset, size;
            getEntry(i, db, offset, size);
            if (db == db_number) return (i16)i;
        }
        return -1;
    }

    // Find first unused slot
    // Returns -1 if all slots are occupied
    i16 findFreeSlot() const {
        for (u16 i = 0; i < num_slots; i++) {
            u16 db, offset, size;
            getEntry(i, db, offset, size);
            if (db == 0) return (i16)i;
        }
        return -1;
    }

    // Count active (non-zero db) entries
    u16 activeCount() const {
        u16 count = 0;
        for (u16 i = 0; i < num_slots; i++) {
            u16 db, offset, size;
            getEntry(i, db, offset, size);
            if (db != 0) count++;
        }
        return count;
    }

    // ========================================================================
    // Memory region management
    // ========================================================================

    // Calculate the lowest address currently occupied by any DB data region.
    // This is the "watermark" below which all DB data lives.
    // Returns table_offset if no DBs are allocated.
    u16 lowestAllocatedAddress() const {
        u16 lowest = table_offset;
        for (u16 i = 0; i < num_slots; i++) {
            u16 db, offset, size;
            getEntry(i, db, offset, size);
            if (db != 0 && offset < lowest) {
                lowest = offset;
            }
        }
        return lowest;
    }

    // Calculate total bytes used by all DB data regions
    u16 totalDataUsed() const {
        u16 total = 0;
        for (u16 i = 0; i < num_slots; i++) {
            u16 db, offset, size;
            getEntry(i, db, offset, size);
            if (db != 0) total += size;
        }
        return total;
    }

    // Calculate free space available for new DB allocations.
    // Free space = gap between user_area_end and the lowest DB data address.
    u16 freeSpace() const {
        u16 lowest = lowestAllocatedAddress();
        if (lowest <= user_area_end) return 0;
        return lowest - user_area_end;
    }

    // ========================================================================
    // DataBlock Declaration (allocate)
    // ========================================================================

    // Declare a new DataBlock. Allocates data region growing downward from
    // the lookup table (or below existing DBs).
    // Returns the slot index on success, -1 on failure.
    // Failure reasons: db_number is 0, already exists, no free slot, not enough space.
    i16 declare(u16 db_number, u16 size) {
        if (db_number == 0) return -1;
        if (size == 0) return -1;

        // Check if already declared
        i16 existing = findSlot(db_number);
        if (existing >= 0) return -1; // Already exists

        // Find a free slot
        i16 slot = findFreeSlot();
        if (slot < 0) return -1; // No free slot

        // Calculate allocation address (grow downward)
        u16 lowest = lowestAllocatedAddress();
        if (size > lowest || (lowest - size) < user_area_end) return -1; // Not enough space
        u16 new_offset = lowest - size;

        // Zero-fill the new DB data region
        for (u16 i = 0; i < size; i++) {
            memory[new_offset + i] = 0;
        }

        // Write the entry
        setEntry((u16)slot, db_number, new_offset, size);
        return slot;
    }

    // ========================================================================
    // DataBlock Removal
    // ========================================================================

    // Remove a DataBlock by DB number, freeing its slot.
    // Note: This does NOT compact - it just marks the slot as unused.
    // Data region remains until compacted or overwritten.
    // Returns true on success.
    bool remove(u16 db_number) {
        i16 slot = findSlot(db_number);
        if (slot < 0) return false;
        setEntry((u16)slot, 0, 0, 0);
        return true;
    }

    // ========================================================================
    // DataBlock Read/Write (safe, range-checked)
    // ========================================================================

    // Read data from a DataBlock at a relative offset within the DB.
    // Returns false on error (DB not found, out of range).
    bool readDB(u16 db_number, u16 db_offset, u8* dest, u16 count) const {
        i16 slot = findSlot(db_number);
        if (slot < 0) return false;

        u16 db, base_offset, db_size;
        getEntry((u16)slot, db, base_offset, db_size);

        // Range check: db_offset + count must fit within the DB
        if ((u32)db_offset + (u32)count > (u32)db_size) return false;

        // Absolute memory range check
        u32 abs_addr = (u32)base_offset + (u32)db_offset;
        if (abs_addr + count > memory_size) return false;

        for (u16 i = 0; i < count; i++) {
            dest[i] = memory[abs_addr + i];
        }
        return true;
    }

    // Write data to a DataBlock at a relative offset within the DB.
    // Returns false on error (DB not found, out of range).
    bool writeDB(u16 db_number, u16 db_offset, const u8* src, u16 count) {
        i16 slot = findSlot(db_number);
        if (slot < 0) return false;

        u16 db, base_offset, db_size;
        getEntry((u16)slot, db, base_offset, db_size);

        // Range check
        if ((u32)db_offset + (u32)count > (u32)db_size) return false;

        u32 abs_addr = (u32)base_offset + (u32)db_offset;
        if (abs_addr + count > memory_size) return false;

        for (u16 i = 0; i < count; i++) {
            memory[abs_addr + i] = src[i];
        }
        return true;
    }

    // Get the absolute memory address for a DB + relative offset.
    // Returns 0xFFFF on error. Useful for bytecode instructions that
    // need to resolve DB addresses to flat memory addresses.
    u16 resolveAddress(u16 db_number, u16 db_offset) const {
        i16 slot = findSlot(db_number);
        if (slot < 0) return 0xFFFF;

        u16 db, base_offset, db_size;
        getEntry((u16)slot, db, base_offset, db_size);

        if (db_offset >= db_size) return 0xFFFF;
        return base_offset + db_offset;
    }

    // ========================================================================
    // DataBlock Migration
    // ========================================================================

    // Migrate a DataBlock to a new absolute offset in memory.
    // Copies the DB data from old location to new location and updates
    // the lookup table entry. The caller must ensure the destination
    // region is free (not overlapping other DBs or reserved areas).
    //
    // target_offset: the new absolute memory address for the DB data.
    // Returns true on success.
    bool migrate(u16 db_number, u16 target_offset) {
        i16 slot = findSlot(db_number);
        if (slot < 0) return false;

        u16 db, old_offset, db_size;
        getEntry((u16)slot, db, old_offset, db_size);

        // Validate target range
        if ((u32)target_offset + (u32)db_size > (u32)table_offset) return false;
        if (target_offset < user_area_end) return false;

        // Check target doesn't overlap any other active DB
        for (u16 i = 0; i < num_slots; i++) {
            if (i == (u16)slot) continue; // Skip self
            u16 other_db, other_offset, other_size;
            getEntry(i, other_db, other_offset, other_size);
            if (other_db == 0) continue;
            // Check for overlap: [target_offset, target_offset+db_size) vs [other_offset, other_offset+other_size)
            u16 target_end = target_offset + db_size;
            u16 other_end = other_offset + other_size;
            if (target_offset < other_end && target_end > other_offset) return false; // Overlap
        }

        // Copy data (handle overlapping regions safely with direction check)
        if (target_offset < old_offset) {
            // Copy forward (low to high)
            for (u16 i = 0; i < db_size; i++) {
                memory[target_offset + i] = memory[old_offset + i];
            }
        } else if (target_offset > old_offset) {
            // Copy backward (high to low)
            for (i16 i = (i16)db_size - 1; i >= 0; i--) {
                memory[target_offset + (u16)i] = memory[old_offset + (u16)i];
            }
        }
        // If target == old, no copy needed

        // Zero-fill old location (only the parts that don't overlap with new)
        if (target_offset != old_offset) {
            u16 old_end = old_offset + db_size;
            u16 new_end = target_offset + db_size;
            for (u16 addr = old_offset; addr < old_end; addr++) {
                // Only zero if not in new region
                if (addr < target_offset || addr >= new_end) {
                    memory[addr] = 0;
                }
            }
        }

        // Update the lookup table entry
        setEntry((u16)slot, db_number, target_offset, db_size);
        return true;
    }

    // Compact all DBs - moves all active DBs to be contiguous, packed
    // tightly against the lookup table. This maximizes free space.
    // Returns the new lowest allocated address.
    u16 compact() {
        // Sort active entries by current offset (descending - highest first)
        // Then pack them downward from the table
        u16 pack_addr = table_offset;

        // Simple approach: repeatedly find the DB with the highest offset
        // and pack it just below the current pack_addr.
        // Use a visited flag via a small bitmask.
        u32 visited = 0; // Supports up to 32 slots

        for (u16 pass = 0; pass < num_slots; pass++) {
            i16 best_slot = -1;
            u16 best_offset = 0;

            for (u16 i = 0; i < num_slots && i < 32; i++) {
                if (visited & ((u32)1 << i)) continue;
                u16 db, offset, size;
                getEntry(i, db, offset, size);
                if (db == 0) {
                    visited |= ((u32)1 << i);
                    continue;
                }
                if (best_slot < 0 || offset > best_offset) {
                    best_slot = (i16)i;
                    best_offset = offset;
                }
            }

            if (best_slot < 0) break; // No more active DBs
            visited |= ((u32)1 << (u16)best_slot);

            u16 db, offset, size;
            getEntry((u16)best_slot, db, offset, size);

            u16 target = pack_addr - size;
            if (target != offset) {
                // Move the data
                if (target < offset) {
                    for (u16 j = 0; j < size; j++)
                        memory[target + j] = memory[offset + j];
                } else {
                    for (i16 j = (i16)size - 1; j >= 0; j--)
                        memory[target + (u16)j] = memory[offset + (u16)j];
                }
                // Update entry
                setEntry((u16)best_slot, db, target, size);
            }

            pack_addr = target;
        }

        return pack_addr;
    }

    // ========================================================================
    // Memory Area Access (safe, range-checked)
    // ========================================================================
    // These helpers provide safe read/write for named memory areas with
    // bounds checking, including DB-aware access.

    // Read bytes from an absolute memory address with bounds checking.
    // Validates against PLCRUNTIME_MAX_MEMORY_SIZE.
    bool safeRead(u16 address, u8* dest, u16 count) const {
        if ((u32)address + (u32)count > memory_size) return false;
        for (u16 i = 0; i < count; i++) {
            dest[i] = memory[address + i];
        }
        return true;
    }

    // Write bytes to an absolute memory address with bounds checking.
    bool safeWrite(u16 address, const u8* src, u16 count) {
        if ((u32)address + (u32)count > memory_size) return false;
        for (u16 i = 0; i < count; i++) {
            memory[address + i] = src[i];
        }
        return true;
    }
};
