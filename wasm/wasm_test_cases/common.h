#pragma once


// #define PLCRUNTIME_NUM_OF_INPUTS    16
// #define PLCRUNTIME_NUM_OF_OUTPUTS   16
// #define PLCRUNTIME_MAX_STACK_SIZE   64
// #define PLCRUNTIME_MAX_MEMORY_SIZE  64
// #define PLCRUNTIME_MAX_PROGRAM_SIZE 84

#define USE_X64_OPS

#include "../../src/tools/assembly/wasm/wasm.h"

WASM_EXPORT void doNothing() {
    // Do nothing
}