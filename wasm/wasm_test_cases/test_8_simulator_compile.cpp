#define __SIMULATOR__
#define __RUNTIME_DEBUG__
#define __RUNTIME_FULL_UNIT_TEST___

#include "common.h"

#include "../../src/VovkPLCRuntime.h"

void custom_test();

WASM_EXPORT void doSomething() {
    custom_test();
}

const char* testAssembly = R"(
# This is a comment
// This is also a comment

/* 
    This is the same program written in JavaScript:
    let sum = 0;
    for (let i = 0; i < 10; i++) {
        sum += 10;
    }
    return sum;
*/

################# Define constant values
const   sum = 0     // set "sum" to memory index 0
const   index = 1   // set "i"   to memory index 1
const   incr = 10

################# Set initial values
ptr.const  sum
u8.const  0
u8.move
ptr.const  index // set u8 at address 1 to value 0
u8.const  0
u8.move

################# Run test for loop
loop:
    ################# jump to end if "i" >= 10
    ptr.const  index
    u8.load
    u8.const   10
    u8.cmp_lt
    jmp_if_not end 
    jmp end

    ################# sum += 10
    ptr.const  sum
    ptr.copy
    u8.load
    u8.const   incr 
    u8.add
    u8.move

    ################# i++
    ptr.const  index
    ptr.copy
    u8.load
    u8.const   1
    u8.add
    u8.move

    jmp loop 

################# exit the program and return the remaining stack
end:  
    ptr.const sum
    u8.load
// Should return 100 (0x64 in hex) 

// Test type conversion and overflow handling
u8.const 130
cvt u8 i8
cvt i8 f32
cvt f32 u8 

// Test bit operations
u8.const 0
u8.set 0
u8.set 1
u8.set 2
u8.set 3
u8.set 4
u8.set 5
u8.set 6
u8.set 7

u8.const 255
u8.get 7

// Expected leftover stack after test: [64 82 FF 01]

########## Run recursive function test with a parameter (count down from 5)

u8.const 5
call FunctionTest
u8.drop

// Simple PLC logic test
    u8.readBit 10.0
    // u8.not
    u8.writeBit 20.0

// Previous way of doing the same task
    ptr.const 20
    ptr.copy
    u8.load
    ptr.const 10
    u8.load
    u8.get 0
    // u8.not

    jump_if_not RESET
    u8.set 0
    jump END

    RESET:
    u8.rset 0

    END:
    u8.move
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
    if (compileAssembly()) return;
}
