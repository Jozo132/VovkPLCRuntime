#define __SIMULATOR__
#define __RUNTIME_DEBUG__
#define __RUNTIME_FULL_UNIT_TEST___

#include "common.h"

#include "../../../lib/VovkPLCRuntime/src/VovkPLCRuntime.h"

void custom_test();

WASM_EXPORT void doSomething() {
    custom_test();
}

const char* testAssembly = R"(
################# Define constant values
const   sum = 0     // set "sum" to memory index 0
const   index = 1   // set "i"   to memory index 1
const   incr = 10

################# Set initial values
u8.const  0
u8.store    sum   // set u8 at address 0 to value 0
u8.const  0
u8.store    index // set u8 at address 1 to value 0

################# Run test for loop
loop:
    ################# jump to end if "i" >= 10
    u8.load    index
    u8.const   10
    u8.cmp_lt
    jmp_if_not end 

    ################# sum += 10
    u8.load    sum
    u8.const   incr 
    u8.add
    u8.store   sum 

    ################# i++
    u8.load    index
    u8.const   1
    u8.add
    u8.store   index

    jmp loop 

################# exit the program and return the remaining stack
end:  
    u8.load sum
// Should return 100 (0x64 in hex) 

// Test type conversion and overflow handling
u8.const 130
cvt u8 i8
cvt i8 f32
cvt f32 u8 

// Test bit operations
u8.const 0
u8.0.set
u8.1.set
u8.2.set
u8.3.set
u8.4.set
u8.5.set
u8.6.set
u8.7.set

u8.const 255
u8.7.get

// Expected leftover stack after test: [64 82 FF 01]

########## Run recursive function test with a parameter (count down from 5)

u8.const 3
call FunctionTest
u8.drop
u8.const 5
call FunctionTest
u8.drop
exit


FunctionTest:
        // At the top of the stack should be the input value
        
        // Make a copy to check the if statement for 0
        u8.copy 
        u8.const 0
        u8.cmp_eq
        ret_if
        u8.const 1
        u8.sub
        call FunctionTest
        ret


        
)";

void custom_test() {
    set_assembly_string((char*) testAssembly);
    if (!compileTest()) return;
    verifyCode();
}
