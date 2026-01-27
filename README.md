 [![.github/workflows/main.yml](https://github.com/Jozo132/VovkPLCRuntime/actions/workflows/main.yml/badge.svg?branch=main&event=push)](https://github.com/Jozo132/VovkPLCRuntime/actions/workflows/main.yml)
# VovkPLCRuntime
### THIS LIBRARY IS A WORK IN PROGRESS AND IS NOT YET FULLY FUNCTIONAL <br> API IS SUBJECT TO CHANGE AT ANY TIME 


```txt
This is an (experimental) Arduino library that runs a PLC virtual machine.

This library was designed to allow you to run PLC logic on most Arduino boards, but you can also run it on a computer as a standalone program or in the browser using WebAssembly.
It is not yet recommended to use this library in a production environment, as it is still in development.

The library allows you to run, update and online-debug PLC programs. The runtime excutes the PLC bytecode reasonably fast.
Updating the running PLC programs is done almost instantly, without the need to recompile the Arduino sketch. 

I've written a custom assembly language for the PLC runtime, which is used to compile PLC logic to the PLC bytecode that the runtime understands. This assembly language is the base that allows more complex programming languages to be built on top of it.
The web interface is provided in the `./wasm/web-server-test` folder of this library.
The PLC runtime runs in a custom stack-based virtual machine that has its' own instruction set and memory model.

The runtime library just barely fits the Arduino Nano's 32KB of flash memory, so it is recommended to use a board with more memory if you plan to use this library in a more advanced project.

To use the provided web interface, you will need LLVM installed on your system to compile the library to WebAssembly (WASM) so it can be run in the browser.
You will also need Node.JS to run the web server that serves the web interface and Python to run the WASM build scripts.

My plan is to have the web interface running as a VS Code extension in the future.
```

## Usage
### Include the library
```cpp
#define __RUNTIME_PRODUCTION__
#define PLCRUNTIME_SERIAL_ENABLED
#include <VovkPLCRuntime.h>
VovkPLCRuntime runtime;
```
### Initialize the PLC runtime in the setup function
```cpp
void setup() {
    // ...
    runtime.initialize();
    // ...
}
```
### Run the PLC runtime in the loop function without blocking delays as quickly as possible for smooth operation
```cpp
void loop() {
    // 1. Read inputs
    bool input = digitalRead(2);
    runtime.setInputBit(0, 0, input); // Set input bit 0.0 to the value of the input

    // 2. Run the PLC program
    runtime.run();

    // 3. Write outputs
    bool output = runtime.getOutputBit(0, 0); // Get the value of output bit 0.0
    digitalWrite(3, output); // Write the output to the output pin

    // 4. Listen for external plc commands (default Serial)
    runtime.listen();
}
```

### How a PLC cycle works
- Read inputs
- Run PLC program
- Write outputs



## JavaScript/WASM usage (universal worker)
All JS/WASM APIs are exposed through a single worker-based runtime. This keeps the UI responsive and gives async access to the full WASM feature set.

```js
import VovkPLC from './wasm/dist/VovkPLC.js'

const worker = await VovkPLC.createWorker('./wasm/dist/VovkPLC.wasm')
worker.onStdout(msg => console.log(msg))

const problems = await worker.lintPLCASM(assembly)
await worker.downloadAssembly(assembly)
await worker.run()

// Raw WASM exports are available via callExport
await worker.callExport('run_unit_test')

// Optional: list export names
const exports = await worker.getExports()

await worker.terminate()
```

Notes:
- Browser builds must serve `VovkPLC.worker.js` alongside `VovkPLC.js` and `VovkPLC.wasm`.
- Node.js uses `worker_threads` automatically; pass a working WASM file path to `createWorker(...)`.

## Web interface and WebAssembly
#### Web PLC editor is still a work in progress but it's getting there
![image](https://github.com/user-attachments/assets/a797114d-4c84-46da-aaba-6b65d8215293)

#### Web PLC assembly compiler and simulator demo
![image](https://github.com/user-attachments/assets/1e4a46e2-03c2-4099-afae-28749961ab62)

#### Control system memory layout starting at address 0 (`P` are one-time periodic pulse bits, `S` are square wave bits)
![image](https://github.com/user-attachments/assets/e63234cc-44ce-465b-8ddd-cee1c6652924)

#### Hand written PLC assembly code with pointers, for-loops, functions and recursion:
```text
// This is a comment

/* 
    This is the same program written in JavaScript:
    let sum = 0;
    for (let i = 0; i < 10; i++) {
        sum += 10;
    }
    return sum;
*/

//################ Define constant values
const   sum = 0     // set "sum" to memory index 0
const   index = 1   // set "i"   to memory index 1
const   incr = 10

//################ Set initial values
ptr.const  sum
u8.const  0
u8.move
ptr.const  index // set u8 at address 1 to value 0
u8.const  0
u8.move

//################ Run test for loop
loop:
    //################ jump to end if "i" >= 10
    ptr.const  index
    u8.load
    u8.const   10
    u8.cmp_lt
    jmp_if_not end 
    jmp end

    //################ sum += 10
    ptr.const  sum
    ptr.copy
    u8.load
    u8.const   incr 
    u8.add
    u8.move

    //################ i++
    ptr.const  index
    ptr.copy
    u8.load
    u8.const   1
    u8.add
    u8.move

    jmp loop 

//################ exit the program and return the remaining stack
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

//######### Run recursive function test with a parameter (count down from 5)

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
```
#### Compiled and explained assembly bytecode from assembly:
```text
Compiling the assembly code...
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Using VovkPLCRuntime Library - Author J.Vovk <jozo132@gmail.com> ::
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
VovkPLCRuntime - PLCASM Compiler v1.0
Compiling assembly to bytecode ... finished in 0 ms
Labels 5:
    loop = 14
    end = 58
    RESET = 120
    END = 121
    FunctionTest = 124
Consts 3:
    sum <int> = 0 => 0
    index <int> = 1 => 1
    incr <int> = 10 => 10
Bytecode checksum C6, 140 bytes:
 01 00 00 03 00 12 03 01 00 01 03 00 12 03 01 00 01 11 03 03 0A D3 03 E2 00 3A E0 00 3A 01 00 00 4 01 11 03 03 0A 20 03 12 03 01 00 01 14 01 11 03 03 01 20 03 12 03 E0 00 0E 01 00 00 11 03 03 82 10 03 07 10 07 0B 10 0B 03 03 00 48 49 4A 4B 4C 4D 4E 4F 03 FF 47 03 05 E3 00 7C 16 03 58 00 0A 60 00 14 01 00 14 14 01 11 03 01 00 0A 11 03 40 E2 00 78 48 E0 00 79 50 12 03 FF 14 03 03 00 D0 03 E7 03 01 21 03 E3 00 7C E6 FF
Loading program with 140 bytes and checksum 0xC6 ...
Program[140] [01 00 00 03 00 12 03 01 00 01 03 00 12 03 01 00 01 11 03 03 0A D3 03 E2 00 3A E0 00 3A 01 00 00 4 01 11 03 03 0A 20 03 12 03 01 00 01 14 01 11 03 03 01 20 03 12 03 E0 00 0E 01 00 00 11 03 03 82 10 03 07 10 07 0B 10 0B 03 03 00 48 49 4A 4B 4C 4D 4E 4F 03 FF 47 03 05 E3 00 7C 16 03 58 00 0A 60 00 14 01 00 14 14 01 11 03 01 00 0A 11 03 40 E2 00 78 48 E0 00 79 50 12 03 FF 14 03 03 00 D0 03 E7 03 01 21 03 E3 00 7C E6 FF]
#### Program Explanation:
       0 [00 00] - 01: PUSH pointer --- [size  3] 010000
       3 [00 03] - 03: PUSH u8 -------- [size  2] 0300
       5 [00 05] - 12: MOVE ----------- [size  2] 1203
       7 [00 07] - 01: PUSH pointer --- [size  3] 010001
      10 [00 0A] - 03: PUSH u8 -------- [size  2] 0300
      12 [00 0C] - 12: MOVE ----------- [size  2] 1203
      14 [00 0E] - 01: PUSH pointer --- [size  3] 010001
      17 [00 11] - 11: LOAD ----------- [size  2] 1103
      19 [00 13] - 03: PUSH u8 -------- [size  2] 030A
      21 [00 15] - D3: CMP_LT --------- [size  2] D303
      23 [00 17] - E2: JMP_IF_NOT ----- [size  3] E2003A
      26 [00 1A] - E0: JMP ------------ [size  3] E0003A
      29 [00 1D] - 01: PUSH pointer --- [size  3] 010000
      32 [00 20] - 14: COPY ----------- [size  2] 1401
      34 [00 22] - 11: LOAD ----------- [size  2] 1103
      36 [00 24] - 03: PUSH u8 -------- [size  2] 030A
      38 [00 26] - 20: ADD ------------ [size  2] 2003
      40 [00 28] - 12: MOVE ----------- [size  2] 1203
      42 [00 2A] - 01: PUSH pointer --- [size  3] 010001
      45 [00 2D] - 14: COPY ----------- [size  2] 1401
      47 [00 2F] - 11: LOAD ----------- [size  2] 1103
      49 [00 31] - 03: PUSH u8 -------- [size  2] 0301
      51 [00 33] - 20: ADD ------------ [size  2] 2003
      53 [00 35] - 12: MOVE ----------- [size  2] 1203
      55 [00 37] - E0: JMP ------------ [size  3] E0000E
      58 [00 3A] - 01: PUSH pointer --- [size  3] 010000
      61 [00 3D] - 11: LOAD ----------- [size  2] 1103
      63 [00 3F] - 03: PUSH u8 -------- [size  2] 0382
      65 [00 41] - 10: CVT ------------ [size  3] 100307
      68 [00 44] - 10: CVT ------------ [size  3] 10070B
      71 [00 47] - 10: CVT ------------ [size  3] 100B03
      74 [00 4A] - 03: PUSH u8 -------- [size  2] 0300
      76 [00 4C] - 48: SET_X8_B0 ------ [size  1] 48
      77 [00 4D] - 49: SET_X8_B1 ------ [size  1] 49
      78 [00 4E] - 4A: SET_X8_B2 ------ [size  1] 4A
      79 [00 4F] - 4B: SET_X8_B3 ------ [size  1] 4B
      80 [00 50] - 4C: SET_X8_B4 ------ [size  1] 4C
      81 [00 51] - 4D: SET_X8_B5 ------ [size  1] 4D
      82 [00 52] - 4E: SET_X8_B6 ------ [size  1] 4E
      83 [00 53] - 4F: SET_X8_B7 ------ [size  1] 4F
      84 [00 54] - 03: PUSH u8 -------- [size  2] 03FF
      86 [00 56] - 47: GET_X8_B7 ------ [size  1] 47
      87 [00 57] - 03: PUSH u8 -------- [size  2] 0305
      89 [00 59] - E3: CALL ----------- [size  3] E3007C
      92 [00 5C] - 16: DROP ----------- [size  2] 1603
      94 [00 5E] - 58: READ_X8_B0 ----- [size  3] 58000A
      97 [00 61] - 60: WRITE_X8_B0 ---- [size  3] 600014
     100 [00 64] - 01: PUSH pointer --- [size  3] 010014
     103 [00 67] - 14: COPY ----------- [size  2] 1401
     105 [00 69] - 11: LOAD ----------- [size  2] 1103
     107 [00 6B] - 01: PUSH pointer --- [size  3] 01000A
     110 [00 6E] - 11: LOAD ----------- [size  2] 1103
     112 [00 70] - 40: GET_X8_B0 ------ [size  1] 40
     113 [00 71] - E2: JMP_IF_NOT ----- [size  3] E20078
     116 [00 74] - 48: SET_X8_B0 ------ [size  1] 48
     117 [00 75] - E0: JMP ------------ [size  3] E00079
     120 [00 78] - 50: RSET_X8_B0 ----- [size  1] 50
     121 [00 79] - 12: MOVE ----------- [size  2] 1203
     123 [00 7B] - FF: EXIT ----------- [size  1] FF
     124 [00 7C] - 14: COPY ----------- [size  2] 1403
     126 [00 7E] - 03: PUSH u8 -------- [size  2] 0300
     128 [00 80] - D0: CMP_EQ --------- [size  2] D003
     130 [00 82] - E7: RET_IF --------- [size  1] E7
     131 [00 83] - 03: PUSH u8 -------- [size  2] 0301
     133 [00 85] - 21: SUB ------------ [size  2] 2103
     135 [00 87] - E3: CALL ----------- [size  3] E3007C
     138 [00 8A] - E6: RET ------------ [size  1] E6
     139 [00 8B] - FF: EXIT ----------- [size  1] FF
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 00]  ---------------------  Stack( 2) [00 00]
    Step    3 [00 03]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 00]  ------------------------  Stack( 3) [00 00 00]
    Step    5 [00 05]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 0) []
    Step    7 [00 07]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 2) [00 01]
    Step   10 [00 0A]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 00]  ------------------------  Stack( 3) [00 01 00]
    Step   12 [00 0C]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 0) []
    Step   14 [00 0E]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 2) [00 01]
    Step   17 [00 11]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 1) [00]
    Step   19 [00 13]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 2) [00 0A]
    Step   21 [00 15]: Executed in [ 0.000 ms]  Opcode[D3]: CMP_LT  -------  [D3 03]  ------------------------  Stack( 1) [01]
    Step   23 [00 17]: Executed in [ 0.000 ms]  Opcode[E2]: JMP_IF_NOT  ---  [E2 00 3A]  ---------------------  Stack( 0) []
    Step   26 [00 1A]: Executed in [ 0.000 ms]  Opcode[E0]: JMP  ----------  [E0 00 3A]  ---------------------  Stack( 0) []
    Step   58 [00 3A]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 00]  ---------------------  Stack( 2) [00 00]
    Step   61 [00 3D]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 1) [00]
    Step   63 [00 3F]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 82]  ------------------------  Stack( 2) [00 82]
    Step   65 [00 41]: Executed in [ 0.000 ms]  Opcode[10]: CVT  ----------  [10 03 07]  ---------------------  Stack( 2) [00 82]
    Step   68 [00 44]: Executed in [ 0.000 ms]  Opcode[10]: CVT  ----------  [10 07 0B]  ---------------------  Stack( 5) [00 C2 FC 00 00]
    Step   71 [00 47]: Executed in [ 0.000 ms]  Opcode[10]: CVT  ----------  [10 0B 03]  ---------------------  Stack( 2) [00 82]
    Step   74 [00 4A]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 00]  ------------------------  Stack( 3) [00 82 00]
    Step   76 [00 4C]: Executed in [ 0.000 ms]  Opcode[48]: SET_X8_B0  ----  [48]  ---------------------------  Stack( 3) [00 82 01]
    Step   77 [00 4D]: Executed in [ 0.000 ms]  Opcode[49]: SET_X8_B1  ----  [49]  ---------------------------  Stack( 3) [00 82 03]
    Step   78 [00 4E]: Executed in [ 0.000 ms]  Opcode[4A]: SET_X8_B2  ----  [4A]  ---------------------------  Stack( 3) [00 82 07]
    Step   79 [00 4F]: Executed in [ 0.000 ms]  Opcode[4B]: SET_X8_B3  ----  [4B]  ---------------------------  Stack( 3) [00 82 0F]
    Step   80 [00 50]: Executed in [ 0.000 ms]  Opcode[4C]: SET_X8_B4  ----  [4C]  ---------------------------  Stack( 3) [00 82 1F]
    Step   81 [00 51]: Executed in [ 0.000 ms]  Opcode[4D]: SET_X8_B5  ----  [4D]  ---------------------------  Stack( 3) [00 82 3F]
    Step   82 [00 52]: Executed in [ 0.000 ms]  Opcode[4E]: SET_X8_B6  ----  [4E]  ---------------------------  Stack( 3) [00 82 7F]
    Step   83 [00 53]: Executed in [ 0.000 ms]  Opcode[4F]: SET_X8_B7  ----  [4F]  ---------------------------  Stack( 3) [00 82 FF]
    Step   84 [00 54]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 FF]  ------------------------  Stack( 4) [00 82 FF FF]
    Step   86 [00 56]: Executed in [ 0.000 ms]  Opcode[47]: GET_X8_B7  ----  [47]  ---------------------------  Stack( 4) [00 82 FF 01]
    Step   87 [00 57]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 05]  ------------------------  Stack( 5) [00 82 FF 01 05]
    Step   89 [00 59]: Executed in [ 0.000 ms]  Opcode[E3]: CALL  ---------  [E3 00 7C]  ---------------------  Stack( 5) [00 82 FF 01 05]
    Step  124 [00 7C]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 03]  ------------------------  Stack( 6) [00 82 FF 01 05 05]
    Step  126 [00 7E]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 00]  ------------------------  Stack( 7) [00 82 FF 01 05 05 00]
    Step  128 [00 80]: Executed in [ 0.000 ms]  Opcode[D0]: CMP_EQ  -------  [D0 03]  ------------------------  Stack( 6) [00 82 FF 01 05 00]
    Step  130 [00 82]: Executed in [ 0.000 ms]  Opcode[E7]: RET_IF  -------  [E7]  ---------------------------  Stack( 5) [00 82 FF 01 05]
    Step  131 [00 83]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 6) [00 82 FF 01 05 01]
    Step  133 [00 85]: Executed in [ 0.000 ms]  Opcode[21]: SUB  ----------  [21 03]  ------------------------  Stack( 5) [00 82 FF 01 04]
    Step  135 [00 87]: Executed in [ 0.000 ms]  Opcode[E3]: CALL  ---------  [E3 00 7C]  ---------------------  Stack( 5) [00 82 FF 01 04]
    Step  124 [00 7C]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 03]  ------------------------  Stack( 6) [00 82 FF 01 04 04]
    Step  126 [00 7E]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 00]  ------------------------  Stack( 7) [00 82 FF 01 04 04 00]
    Step  128 [00 80]: Executed in [ 0.000 ms]  Opcode[D0]: CMP_EQ  -------  [D0 03]  ------------------------  Stack( 6) [00 82 FF 01 04 00]
    Step  130 [00 82]: Executed in [ 0.000 ms]  Opcode[E7]: RET_IF  -------  [E7]  ---------------------------  Stack( 5) [00 82 FF 01 04]
    Step  131 [00 83]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 6) [00 82 FF 01 04 01]
    Step  133 [00 85]: Executed in [ 0.000 ms]  Opcode[21]: SUB  ----------  [21 03]  ------------------------  Stack( 5) [00 82 FF 01 03]
    Step  135 [00 87]: Executed in [ 0.000 ms]  Opcode[E3]: CALL  ---------  [E3 00 7C]  ---------------------  Stack( 5) [00 82 FF 01 03]
    Step  124 [00 7C]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 03]  ------------------------  Stack( 6) [00 82 FF 01 03 03]
    Step  126 [00 7E]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 00]  ------------------------  Stack( 7) [00 82 FF 01 03 03 00]
    Step  128 [00 80]: Executed in [ 0.000 ms]  Opcode[D0]: CMP_EQ  -------  [D0 03]  ------------------------  Stack( 6) [00 82 FF 01 03 00]
    Step  130 [00 82]: Executed in [ 0.000 ms]  Opcode[E7]: RET_IF  -------  [E7]  ---------------------------  Stack( 5) [00 82 FF 01 03]
    Step  131 [00 83]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 6) [00 82 FF 01 03 01]
    Step  133 [00 85]: Executed in [ 0.000 ms]  Opcode[21]: SUB  ----------  [21 03]  ------------------------  Stack( 5) [00 82 FF 01 02]
    Step  135 [00 87]: Executed in [ 0.000 ms]  Opcode[E3]: CALL  ---------  [E3 00 7C]  ---------------------  Stack( 5) [00 82 FF 01 02]
    Step  124 [00 7C]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 03]  ------------------------  Stack( 6) [00 82 FF 01 02 02]
    Step  126 [00 7E]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 00]  ------------------------  Stack( 7) [00 82 FF 01 02 02 00]
    Step  128 [00 80]: Executed in [ 0.000 ms]  Opcode[D0]: CMP_EQ  -------  [D0 03]  ------------------------  Stack( 6) [00 82 FF 01 02 00]
    Step  130 [00 82]: Executed in [ 0.000 ms]  Opcode[E7]: RET_IF  -------  [E7]  ---------------------------  Stack( 5) [00 82 FF 01 02]
    Step  131 [00 83]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 6) [00 82 FF 01 02 01]
    Step  133 [00 85]: Executed in [ 0.000 ms]  Opcode[21]: SUB  ----------  [21 03]  ------------------------  Stack( 5) [00 82 FF 01 01]
    Step  135 [00 87]: Executed in [ 0.000 ms]  Opcode[E3]: CALL  ---------  [E3 00 7C]  ---------------------  Stack( 5) [00 82 FF 01 01]
    Step  124 [00 7C]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 03]  ------------------------  Stack( 6) [00 82 FF 01 01 01]
    Step  126 [00 7E]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 00]  ------------------------  Stack( 7) [00 82 FF 01 01 01 00]
    Step  128 [00 80]: Executed in [ 0.000 ms]  Opcode[D0]: CMP_EQ  -------  [D0 03]  ------------------------  Stack( 6) [00 82 FF 01 01 00]
    Step  130 [00 82]: Executed in [ 0.000 ms]  Opcode[E7]: RET_IF  -------  [E7]  ---------------------------  Stack( 5) [00 82 FF 01 01]
    Step  131 [00 83]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 6) [00 82 FF 01 01 01]
    Step  133 [00 85]: Executed in [ 0.000 ms]  Opcode[21]: SUB  ----------  [21 03]  ------------------------  Stack( 5) [00 82 FF 01 00]
    Step  135 [00 87]: Executed in [ 0.000 ms]  Opcode[E3]: CALL  ---------  [E3 00 7C]  ---------------------  Stack( 5) [00 82 FF 01 00]
    Step  124 [00 7C]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 03]  ------------------------  Stack( 6) [00 82 FF 01 00 00]
    Step  126 [00 7E]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 00]  ------------------------  Stack( 7) [00 82 FF 01 00 00 00]
    Step  128 [00 80]: Executed in [ 0.000 ms]  Opcode[D0]: CMP_EQ  -------  [D0 03]  ------------------------  Stack( 6) [00 82 FF 01 00 01]
    Step  130 [00 82]: Executed in [ 0.000 ms]  Opcode[E7]: RET_IF  -------  [E7]  ---------------------------  Stack( 5) [00 82 FF 01 00]
    Step  138 [00 8A]: Executed in [ 0.000 ms]  Opcode[E6]: RET  ----------  [E6]  ---------------------------  Stack( 5) [00 82 FF 01 00]
    Step  138 [00 8A]: Executed in [ 0.000 ms]  Opcode[E6]: RET  ----------  [E6]  ---------------------------  Stack( 5) [00 82 FF 01 00]
    Step  138 [00 8A]: Executed in [ 0.000 ms]  Opcode[E6]: RET  ----------  [E6]  ---------------------------  Stack( 5) [00 82 FF 01 00]
    Step  138 [00 8A]: Executed in [ 0.000 ms]  Opcode[E6]: RET  ----------  [E6]  ---------------------------  Stack( 5) [00 82 FF 01 00]
    Step  138 [00 8A]: Executed in [ 0.000 ms]  Opcode[E6]: RET  ----------  [E6]  ---------------------------  Stack( 5) [00 82 FF 01 00]
    Step   92 [00 5C]: Executed in [ 0.000 ms]  Opcode[16]: DROP  ---------  [16 03]  ------------------------  Stack( 4) [00 82 FF 01]
    Step   94 [00 5E]: Executed in [ 0.000 ms]  Opcode[58]: READ_X8_B0  ---  [58 00 0A]  ---------------------  Stack( 5) [00 82 FF 01 00]
    Step   97 [00 61]: Executed in [ 0.000 ms]  Opcode[60]: WRITE_X8_B0  --  [60 00 14]  ---------------------  Stack( 4) [00 82 FF 01]
    Step  100 [00 64]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 14]  ---------------------  Stack( 6) [00 82 FF 01 00 14]
    Step  103 [00 67]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [00 82 FF 01 00 14 00 14]
    Step  105 [00 69]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [00 82 FF 01 00 14 00]
    Step  107 [00 6B]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 0A]  ---------------------  Stack( 9) [00 82 FF 01 00 14 00 00 0A]
    Step  110 [00 6E]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 8) [00 82 FF 01 00 14 00 00]
    Step  112 [00 70]: Executed in [ 0.000 ms]  Opcode[40]: GET_X8_B0  ----  [40]  ---------------------------  Stack( 8) [00 82 FF 01 00 14 00 00]
    Step  113 [00 71]: Executed in [ 0.000 ms]  Opcode[E2]: JMP_IF_NOT  ---  [E2 00 78]  ---------------------  Stack( 7) [00 82 FF 01 00 14 00]
    Step  120 [00 78]: Executed in [ 0.000 ms]  Opcode[50]: RSET_X8_B0  ---  [50]  ---------------------------  Stack( 7) [00 82 FF 01 00 14 00]
    Step  121 [00 79]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [00 82 FF 01]
    Step  123 [00 7B]: Executed in [ 0.000 ms]  Opcode[FF]: EXIT  ---------  [FF]  ---------------------------  Stack( 4) [00 82 FF 01]
Leftover Stack( 4) [00 82 FF 01]
Time to execute program: 0.000 ms
Debug failed with error: PROGRAM_EXITED
Runtime status: PROGRAM_EXITED
Program download string for Serial/UART/RS232: PD0000008C01000003001203010001030012030100011103030AD303E2003AE0003A01000014011103030A2003120301000114011103030120031203E0000E0100001103038210030710070B100B03030048494A4B4C4D4E4F03FF470305E3007C160358000A6000140100141401110301000A110340E2007848E00079501203FF14030300D003E703012103E3007CE6FFBA
```

#### Basic functional unit test results
```text
Running VovkPLCRuntime WebAssembly simulation unit test...
----------------------------------------------------------------------
Runtime Unit Test
----------------------------------------------------------------------
RuntimeError Status List:
     0: STATUS_SUCCESS
     1: UNKNOWN_INSTRUCTION
     2: INVALID_INSTRUCTION
     3: INVALID_DATA_TYPE
     4: STACK_OVERFLOW
     5: STACK_UNDERFLOW
     6: INVALID_STACK_SIZE
     7: EMPTY_STACK
     8: CALL_STACK_OVERFLOW
     9: CALL_STACK_UNDERFLOW
    10: INVALID_MEMORY_ADDRESS
    11: INVALID_MEMORY_SIZE
    12: UNDEFINED_STATE
    13: INVALID_PROGRAM_INDEX
    14: PROGRAM_SIZE_EXCEEDED
    15: PROGRAM_POINTER_OUT_OF_BOUNDS
    16: PROGRAM_EXITED
    17: EMPTY_PROGRAM
    18: NO_PROGRAM
    19: INVALID_CHECKSUM
    20: EXECUTION_ERROR
    21: EXECUTION_TIMEOUT
    22: MEMORY_ACCESS_ERROR
    23: PROGRAM_CYCLE_LIMIT_EXCEEDED
----------------------------------------------------------------------
Bytecode Instruction Set:
    0x00:  NOP            (1 byte)
    0x01:  PUSH pointer   (3 bytes)
    0x02:  PUSH boolean   (2 bytes)
    0x03:  PUSH u8        (2 bytes)
    0x04:  PUSH u16       (3 bytes)
    0x05:  PUSH u32       (5 bytes)
    0x06:  PUSH u64       (9 bytes)
    0x07:  PUSH i8        (2 bytes)
    0x08:  PUSH i16       (3 bytes)
    0x09:  PUSH i32       (5 bytes)
    0x0A:  PUSH i64       (9 bytes)
    0x0B:  PUSH f32       (5 bytes)
    0x0C:  PUSH f64       (9 bytes)
    0x10:  CVT            (3 bytes)
    0x11:  LOAD           (2 bytes)
    0x12:  MOVE           (2 bytes)
    0x13:  MOVE_COPY      (2 bytes)
    0x14:  COPY           (2 bytes)
    0x15:  SWAP           (3 bytes)
    0x16:  DROP           (2 bytes)
    0x17:  CLEAR          (1 byte)
    0x20:  ADD            (2 bytes)
    0x21:  SUB            (2 bytes)
    0x22:  MUL            (2 bytes)
    0x23:  DIV            (2 bytes)
    0x24:  MOD            (2 bytes)
    0x25:  POW            (2 bytes)
    0x26:  SQRT           (2 bytes)
    0x27:  NEG            (2 bytes)
    0x28:  ABS            (2 bytes)
    0x29:  SIN            (2 bytes)
    0x2A:  COS            (2 bytes)
    0x40:  GET_X8_B0      (1 byte)
    0x41:  GET_X8_B1      (1 byte)
    0x42:  GET_X8_B2      (1 byte)
    0x43:  GET_X8_B3      (1 byte)
    0x44:  GET_X8_B4      (1 byte)
    0x45:  GET_X8_B5      (1 byte)
    0x46:  GET_X8_B6      (1 byte)
    0x47:  GET_X8_B7      (1 byte)
    0x48:  SET_X8_B0      (1 byte)
    0x49:  SET_X8_B1      (1 byte)
    0x4A:  SET_X8_B2      (1 byte)
    0x4B:  SET_X8_B3      (1 byte)
    0x4C:  SET_X8_B4      (1 byte)
    0x4D:  SET_X8_B5      (1 byte)
    0x4E:  SET_X8_B6      (1 byte)
    0x4F:  SET_X8_B7      (1 byte)
    0x50:  RSET_X8_B0     (1 byte)
    0x51:  RSET_X8_B1     (1 byte)
    0x52:  RSET_X8_B2     (1 byte)
    0x53:  RSET_X8_B3     (1 byte)
    0x54:  RSET_X8_B4     (1 byte)
    0x55:  RSET_X8_B5     (1 byte)
    0x56:  RSET_X8_B6     (1 byte)
    0x57:  RSET_X8_B7     (1 byte)
    0x58:  READ_X8_B0     (3 bytes)
    0x59:  READ_X8_B1     (3 bytes)
    0x5A:  READ_X8_B2     (3 bytes)
    0x5B:  READ_X8_B3     (3 bytes)
    0x5C:  READ_X8_B4     (3 bytes)
    0x5D:  READ_X8_B5     (3 bytes)
    0x5E:  READ_X8_B6     (3 bytes)
    0x5F:  READ_X8_B7     (3 bytes)
    0x60:  WRITE_X8_B0    (3 bytes)
    0x61:  WRITE_X8_B1    (3 bytes)
    0x62:  WRITE_X8_B2    (3 bytes)
    0x63:  WRITE_X8_B3    (3 bytes)
    0x64:  WRITE_X8_B4    (3 bytes)
    0x65:  WRITE_X8_B5    (3 bytes)
    0x66:  WRITE_X8_B6    (3 bytes)
    0x67:  WRITE_X8_B7    (3 bytes)
    0x68:  WRITE_S_X8_B0  (3 bytes)
    0x69:  WRITE_S_X8_B1  (3 bytes)
    0x6A:  WRITE_S_X8_B2  (3 bytes)
    0x6B:  WRITE_S_X8_B3  (3 bytes)
    0x6C:  WRITE_S_X8_B4  (3 bytes)
    0x6D:  WRITE_S_X8_B5  (3 bytes)
    0x6E:  WRITE_S_X8_B6  (3 bytes)
    0x6F:  WRITE_S_X8_B7  (3 bytes)
    0x70:  WRITE_R_X8_B0  (3 bytes)
    0x71:  WRITE_R_X8_B1  (3 bytes)
    0x72:  WRITE_R_X8_B2  (3 bytes)
    0x73:  WRITE_R_X8_B3  (3 bytes)
    0x74:  WRITE_R_X8_B4  (3 bytes)
    0x75:  WRITE_R_X8_B5  (3 bytes)
    0x76:  WRITE_R_X8_B6  (3 bytes)
    0x77:  WRITE_R_X8_B7  (3 bytes)
    0x78:  WRITE_INV_X8_B0(3 bytes)
    0x79:  WRITE_INV_X8_B1(3 bytes)
    0x7A:  WRITE_INV_X8_B2(3 bytes)
    0x7B:  WRITE_INV_X8_B3(3 bytes)
    0x7C:  WRITE_INV_X8_B4(3 bytes)
    0x7D:  WRITE_INV_X8_B5(3 bytes)
    0x7E:  WRITE_INV_X8_B6(3 bytes)
    0x7F:  WRITE_INV_X8_B7(3 bytes)
    0xA0:  BW_AND_X8      (1 byte)
    0xA1:  BW_AND_X16     (1 byte)
    0xA2:  BW_AND_X32     (1 byte)
    0xA3:  BW_AND_X64     (1 byte)
    0xA4:  BW_OR_X8       (1 byte)
    0xA5:  BW_OR_X16      (1 byte)
    0xA6:  BW_OR_X32      (1 byte)
    0xA7:  BW_OR_X64      (1 byte)
    0xA8:  BW_XOR_X8      (1 byte)
    0xA9:  BW_XOR_X16     (1 byte)
    0xAA:  BW_XOR_X32     (1 byte)
    0xAB:  BW_XOR_X64     (1 byte)
    0xAC:  BW_NOT_X8      (1 byte)
    0xAD:  BW_NOT_X16     (1 byte)
    0xAE:  BW_NOT_X32     (1 byte)
    0xAF:  BW_NOT_X64     (1 byte)
    0xB0:  BW_LSHIFT_X8   (1 byte)
    0xB1:  BW_LSHIFT_X16  (1 byte)
    0xB2:  BW_LSHIFT_X32  (1 byte)
    0xB3:  BW_LSHIFT_X64  (1 byte)
    0xB4:  BW_RSHIFT_X8   (1 byte)
    0xB5:  BW_RSHIFT_X16  (1 byte)
    0xB6:  BW_RSHIFT_X32  (1 byte)
    0xB7:  BW_RSHIFT_X64  (1 byte)
    0xC0:  LOGIC_AND      (1 byte)
    0xC1:  LOGIC_OR       (1 byte)
    0xC2:  LOGIC_XOR      (1 byte)
    0xC3:  LOGIC_NOT      (1 byte)
    0xD0:  CMP_EQ         (2 bytes)
    0xD1:  CMP_NEQ        (2 bytes)
    0xD2:  CMP_GT         (2 bytes)
    0xD3:  CMP_LT         (2 bytes)
    0xD4:  CMP_GTE        (2 bytes)
    0xD5:  CMP_LTE        (2 bytes)
    0xE0:  JMP            (3 bytes)
    0xE1:  JMP_IF         (3 bytes)
    0xE2:  JMP_IF_NOT     (3 bytes)
    0xE3:  CALL           (3 bytes)
    0xE4:  CALL_IF        (3 bytes)
    0xE5:  CALL_IF_NOT    (3 bytes)
    0xE6:  RET            (1 byte)
    0xE7:  RET_IF         (1 byte)
    0xE8:  RET_IF_NOT     (1 byte)
    0xFF:  EXIT           (1 byte)
----------------------------------------------------------------------
######################################################################
Executing Runtime Unit Tests...
######################################################################
Running test: demo_uint8_t => (1 + 2) * 3
Program[10] [03 01 03 02 20 03 03 03 22 03]
#### Program Explanation:
       0 [00 00] - 03: PUSH u8 -------- [size  2] 0301
       2 [00 02] - 03: PUSH u8 -------- [size  2] 0302
       4 [00 04] - 20: ADD ------------ [size  2] 2003
       6 [00 06] - 03: PUSH u8 -------- [size  2] 0303
       8 [00 08] - 22: MUL ------------ [size  2] 2203
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 1) [01]
    Step    2 [00 02]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 02]  ------------------------  Stack( 2) [01 02]
    Step    4 [00 04]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 1) [03]
    Step    6 [00 06]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 03]  ------------------------  Stack( 2) [03 03]
    Step    8 [00 08]: Executed in [ 0.000 ms]  Opcode[22]: MUL  ----------  [22 03]  ------------------------  Stack( 1) [09]
Leftover Stack( 1) [09]
Time to execute program: 0.000 ms
Program result: 9
Expected result: 9
Test status: Passed
######################################################################
Running test: demo_uint16_t => (1 + 2) * 3
Program[13] [04 00 01 04 00 02 20 04 04 00 03 22 04]
#### Program Explanation:
       0 [00 00] - 04: PUSH u16 ------- [size  3] 040001
       3 [00 03] - 04: PUSH u16 ------- [size  3] 040002
       6 [00 06] - 20: ADD ------------ [size  2] 2004
       8 [00 08] - 04: PUSH u16 ------- [size  3] 040003
      11 [00 0B] - 22: MUL ------------ [size  2] 2204
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Executed in [ 0.000 ms]  Opcode[04]: PUSH u16  -----  [04 00 01]  ---------------------  Stack( 2) [00 01]
    Step    3 [00 03]: Executed in [ 0.000 ms]  Opcode[04]: PUSH u16  -----  [04 00 02]  ---------------------  Stack( 4) [00 01 00 02]
    Step    6 [00 06]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 04]  ------------------------  Stack( 2) [00 03]
    Step    8 [00 08]: Executed in [ 0.000 ms]  Opcode[04]: PUSH u16  -----  [04 00 03]  ---------------------  Stack( 4) [00 03 00 03]
    Step   11 [00 0B]: Executed in [ 0.000 ms]  Opcode[22]: MUL  ----------  [22 04]  ------------------------  Stack( 2) [00 09]
Leftover Stack( 2) [00 09]
Time to execute program: 0.000 ms
Program result: 9
Expected result: 9
Test status: Passed
######################################################################
Running test: demo_uint32_t => (1 + 2) * 3
Program[19] [05 00 00 00 01 05 00 00 00 02 20 05 05 00 00 00 03 22 05]
#### Program Explanation:
       0 [00 00] - 05: PUSH u32 ------- [size  5] 0500000001
       5 [00 05] - 05: PUSH u32 ------- [size  5] 0500000002
      10 [00 0A] - 20: ADD ------------ [size  2] 2005
      12 [00 0C] - 05: PUSH u32 ------- [size  5] 0500000003
      17 [00 11] - 22: MUL ------------ [size  2] 2205
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Executed in [ 0.000 ms]  Opcode[05]: PUSH u32  -----  [05 00 00 00 01]  ---------------  Stack( 4) [00 00 00 01]
    Step    5 [00 05]: Executed in [ 0.000 ms]  Opcode[05]: PUSH u32  -----  [05 00 00 00 02]  ---------------  Stack( 8) [00 00 00 01 00 00 00 02]
    Step   10 [00 0A]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 05]  ------------------------  Stack( 4) [00 00 00 03]
    Step   12 [00 0C]: Executed in [ 0.000 ms]  Opcode[05]: PUSH u32  -----  [05 00 00 00 03]  ---------------  Stack( 8) [00 00 00 03 00 00 00 03]
    Step   17 [00 11]: Executed in [ 0.000 ms]  Opcode[22]: MUL  ----------  [22 05]  ------------------------  Stack( 4) [00 00 00 09]
Leftover Stack( 4) [00 00 00 09]
Time to execute program: 0.000 ms
Program result: 9
Expected result: 9
Test status: Passed
######################################################################
Running test: demo_uint64_t => (1 + 2) * 3
Program[31] [06 00 00 00 00 00 00 00 01 06 00 00 00 00 00 00 00 02 20 06 06 00 00 00 00 00 00 00 03 22 06]
#### Program Explanation:
       0 [00 00] - 06: PUSH u64 ------- [size  9] 060000000000000001
       9 [00 09] - 06: PUSH u64 ------- [size  9] 060000000000000002
      18 [00 12] - 20: ADD ------------ [size  2] 2006
      20 [00 14] - 06: PUSH u64 ------- [size  9] 060000000000000003
      29 [00 1D] - 22: MUL ------------ [size  2] 2206
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Executed in [ 0.000 ms]  Opcode[06]: PUSH u64  -----  [06 00 00 00 00 00 00 00 01]  ---  Stack( 8) [00 00 00 00 00 00 00 01]
    Step    9 [00 09]: Executed in [ 0.000 ms]  Opcode[06]: PUSH u64  -----  [06 00 00 00 00 00 00 00 02]  ---  Stack(16) [00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 02]
    Step   18 [00 12]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 06]  ------------------------  Stack( 8) [00 00 00 00 00 00 00 03]
    Step   20 [00 14]: Executed in [ 0.000 ms]  Opcode[06]: PUSH u64  -----  [06 00 00 00 00 00 00 00 03]  ---  Stack(16) [00 00 00 00 00 00 00 03 00 00 00 00 00 00 00 03]
    Step   29 [00 1D]: Executed in [ 0.000 ms]  Opcode[22]: MUL  ----------  [22 06]  ------------------------  Stack( 8) [00 00 00 00 00 00 00 09]
Leftover Stack( 8) [00 00 00 00 00 00 00 09]
Time to execute program: 0.000 ms
Program result: 9
Expected result: 9
Test status: Passed
######################################################################
Running test: demo_int8_t => (1 - 2) * 3
Program[10] [07 01 07 02 21 07 07 03 22 07]
#### Program Explanation:
       0 [00 00] - 07: PUSH i8 -------- [size  2] 0701
       2 [00 02] - 07: PUSH i8 -------- [size  2] 0702
       4 [00 04] - 21: SUB ------------ [size  2] 2107
       6 [00 06] - 07: PUSH i8 -------- [size  2] 0703
       8 [00 08] - 22: MUL ------------ [size  2] 2207
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Executed in [ 0.000 ms]  Opcode[07]: PUSH i8  ------  [07 01]  ------------------------  Stack( 1) [01]
    Step    2 [00 02]: Executed in [ 0.000 ms]  Opcode[07]: PUSH i8  ------  [07 02]  ------------------------  Stack( 2) [01 02]
    Step    4 [00 04]: Executed in [ 0.000 ms]  Opcode[21]: SUB  ----------  [21 07]  ------------------------  Stack( 1) [FF]
    Step    6 [00 06]: Executed in [ 0.000 ms]  Opcode[07]: PUSH i8  ------  [07 03]  ------------------------  Stack( 2) [FF 03]
    Step    8 [00 08]: Executed in [ 0.000 ms]  Opcode[22]: MUL  ----------  [22 07]  ------------------------  Stack( 1) [FD]
Leftover Stack( 1) [FD]
Time to execute program: 0.000 ms
Program result: -3
Expected result: -3
Test status: Passed
######################################################################
Running test: demo_float => (0.1 + 0.2) * -1
Program[19] [0B 3D CC CC CD 0B 3E 4C CC CD 20 0B 0B BF 80 00 00 22 0B]
#### Program Explanation:
       0 [00 00] - 0B: PUSH f32 ------- [size  5] 0B3DCCCCCD
       5 [00 05] - 0B: PUSH f32 ------- [size  5] 0B3E4CCCCD
      10 [00 0A] - 20: ADD ------------ [size  2] 200B
      12 [00 0C] - 0B: PUSH f32 ------- [size  5] 0BBF800000
      17 [00 11] - 22: MUL ------------ [size  2] 220B
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Executed in [ 0.000 ms]  Opcode[0B]: PUSH f32  -----  [0B 3D CC CC CD]  ---------------  Stack( 4) [3D CC CC CD]
    Step    5 [00 05]: Executed in [ 0.000 ms]  Opcode[0B]: PUSH f32  -----  [0B 3E 4C CC CD]  ---------------  Stack( 8) [3D CC CC CD 3E 4C CC CD]
    Step   10 [00 0A]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 0B]  ------------------------  Stack( 4) [3E 99 99 9A]
    Step   12 [00 0C]: Executed in [ 0.000 ms]  Opcode[0B]: PUSH f32  -----  [0B BF 80 00 00]  ---------------  Stack( 8) [3E 99 99 9A BF 80 00 00]
    Step   17 [00 11]: Executed in [ 0.000 ms]  Opcode[22]: MUL  ----------  [22 0B]  ------------------------  Stack( 4) [BE 99 99 9A]
Leftover Stack( 4) [BE 99 99 9A]
Time to execute program: 0.000 ms
Program result: -0.3000000120
Expected result: -0.3000000120
Test status: Passed
######################################################################
Running test: demo_double => (0.1 + 0.2) * -1
Program[31] [0C 3F B9 99 99 99 99 99 9A 0C 3F C9 99 99 99 99 99 9A 20 0C 0C BF F0 00 00 00 00 00 00 22 0C]
#### Program Explanation:
       0 [00 00] - 0C: PUSH f64 ------- [size  9] 0C3FB999999999999A
       9 [00 09] - 0C: PUSH f64 ------- [size  9] 0C3FC999999999999A
      18 [00 12] - 20: ADD ------------ [size  2] 200C
      20 [00 14] - 0C: PUSH f64 ------- [size  9] 0CBFF0000000000000
      29 [00 1D] - 22: MUL ------------ [size  2] 220C
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Executed in [ 0.000 ms]  Opcode[0C]: PUSH f64  -----  [0C 3F B9 99 99 99 99 99 9A]  ---  Stack( 8) [3F B9 99 99 99 99 99 9A]
    Step    9 [00 09]: Executed in [ 0.000 ms]  Opcode[0C]: PUSH f64  -----  [0C 3F C9 99 99 99 99 99 9A]  ---  Stack(16) [3F B9 99 99 99 99 99 9A 3F C9 99 99 99 99 99 9A]
    Step   18 [00 12]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 0C]  ------------------------  Stack( 8) [3F D3 33 33 33 33 33 34]
    Step   20 [00 14]: Executed in [ 0.000 ms]  Opcode[0C]: PUSH f64  -----  [0C BF F0 00 00 00 00 00 00]  ---  Stack(16) [3F D3 33 33 33 33 33 34 BF F0 00 00 00 00 00 00]
    Step   29 [00 1D]: Executed in [ 0.000 ms]  Opcode[22]: MUL  ----------  [22 0C]  ------------------------  Stack( 8) [BF D3 33 33 33 33 33 34]
Leftover Stack( 8) [BF D3 33 33 33 33 33 34]
Time to execute program: 0.000 ms
Program result: -0.3000000000
Expected result: -0.3000000000
Test status: Passed
######################################################################
Running test: bitwise_and_X8
Program[5] [03 0F 03 55 A0]
#### Program Explanation:
       0 [00 00] - 03: PUSH u8 -------- [size  2] 030F
       2 [00 02] - 03: PUSH u8 -------- [size  2] 0355
       4 [00 04] - A0: BW_AND_X8 ------ [size  1] A0
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0F]  ------------------------  Stack( 1) [0F]
    Step    2 [00 02]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 55]  ------------------------  Stack( 2) [0F 55]
    Step    4 [00 04]: Executed in [ 0.000 ms]  Opcode[A0]: BW_AND_X8  ----  [A0]  ---------------------------  Stack( 1) [05]
Leftover Stack( 1) [05]
Time to execute program: 0.000 ms
Program result: 5
Expected result: 5
Test status: Passed
######################################################################
Running test: bitwise_and_X16
Program[7] [04 00 FF 04 F0 0F A1]
#### Program Explanation:
       0 [00 00] - 04: PUSH u16 ------- [size  3] 0400FF
       3 [00 03] - 04: PUSH u16 ------- [size  3] 04F00F
       6 [00 06] - A1: BW_AND_X16 ----- [size  1] A1
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Executed in [ 0.000 ms]  Opcode[04]: PUSH u16  -----  [04 00 FF]  ---------------------  Stack( 2) [00 FF]
    Step    3 [00 03]: Executed in [ 0.000 ms]  Opcode[04]: PUSH u16  -----  [04 F0 0F]  ---------------------  Stack( 4) [00 FF F0 0F]
    Step    6 [00 06]: Executed in [ 0.000 ms]  Opcode[A1]: BW_AND_X16  ---  [A1]  ---------------------------  Stack( 2) [00 0F]
Leftover Stack( 2) [00 0F]
Time to execute program: 0.000 ms
Program result: 15
Expected result: 15
Test status: Passed
######################################################################
Running test: logic_and => true && true
Program[5] [03 01 03 01 C0]
#### Program Explanation:
       0 [00 00] - 03: PUSH u8 -------- [size  2] 0301
       2 [00 02] - 03: PUSH u8 -------- [size  2] 0301
       4 [00 04] - C0: LOGIC_AND ------ [size  1] C0
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 1) [01]
    Step    2 [00 02]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 2) [01 01]
    Step    4 [00 04]: Executed in [ 0.000 ms]  Opcode[C0]: LOGIC_AND  ----  [C0]  ---------------------------  Stack( 1) [01]
Leftover Stack( 1) [01]
Time to execute program: 0.000 ms
Program result: 1
Expected result: 1
Test status: Passed
######################################################################
Running test: logic_or => true || false
Program[5] [03 01 03 00 C1]
#### Program Explanation:
       0 [00 00] - 03: PUSH u8 -------- [size  2] 0301
       2 [00 02] - 03: PUSH u8 -------- [size  2] 0300
       4 [00 04] - C1: LOGIC_OR ------- [size  1] C1
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 1) [01]
    Step    2 [00 02]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 00]  ------------------------  Stack( 2) [01 00]
    Step    4 [00 04]: Executed in [ 0.000 ms]  Opcode[C1]: LOGIC_OR  -----  [C1]  ---------------------------  Stack( 1) [01]
Leftover Stack( 1) [01]
Time to execute program: 0.000 ms
Program result: 1
Expected result: 1
Test status: Passed
######################################################################
Running test: cmp_eq_1 => 1 == 1
Program[6] [03 01 03 01 D0 02]
#### Program Explanation:
       0 [00 00] - 03: PUSH u8 -------- [size  2] 0301
       2 [00 02] - 03: PUSH u8 -------- [size  2] 0301
       4 [00 04] - D0: CMP_EQ --------- [size  2] D002
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 1) [01]
    Step    2 [00 02]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 2) [01 01]
    Step    4 [00 04]: Executed in [ 0.000 ms]  Opcode[D0]: CMP_EQ  -------  [D0 02]  ------------------------  Stack( 1) [01]
Leftover Stack( 1) [01]
Time to execute program: 0.000 ms
Program result: 1
Expected result: 1
Test status: Passed
######################################################################
Running test: cmp_eq_2 => 0.29 == 0.31
Program[12] [0B 3E 94 7A E1 0B 3E 9E B8 52 D0 0B]
#### Program Explanation:
       0 [00 00] - 0B: PUSH f32 ------- [size  5] 0B3E947AE1
       5 [00 05] - 0B: PUSH f32 ------- [size  5] 0B3E9EB852
      10 [00 0A] - D0: CMP_EQ --------- [size  2] D00B
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Executed in [ 0.000 ms]  Opcode[0B]: PUSH f32  -----  [0B 3E 94 7A E1]  ---------------  Stack( 4) [3E 94 7A E1]
    Step    5 [00 05]: Executed in [ 0.000 ms]  Opcode[0B]: PUSH f32  -----  [0B 3E 9E B8 52]  ---------------  Stack( 8) [3E 94 7A E1 3E 9E B8 52]
    Step   10 [00 0A]: Executed in [ 0.000 ms]  Opcode[D0]: CMP_EQ  -------  [D0 0B]  ------------------------  Stack( 1) [00]
Leftover Stack( 1) [00]
Time to execute program: 0.000 ms
Program result: 0
Expected result: 0
Test status: Passed
######################################################################
Running test: jump => push 1 and jump to exit
Program[14] [03 01 E0 00 0D 03 01 20 03 03 03 22 03 FF]
#### Program Explanation:
       0 [00 00] - 03: PUSH u8 -------- [size  2] 0301
       2 [00 02] - E0: JMP ------------ [size  3] E0000D
       5 [00 05] - 03: PUSH u8 -------- [size  2] 0301
       7 [00 07] - 20: ADD ------------ [size  2] 2003
       9 [00 09] - 03: PUSH u8 -------- [size  2] 0303
      11 [00 0B] - 22: MUL ------------ [size  2] 2203
      13 [00 0D] - FF: EXIT ----------- [size  1] FF
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 1) [01]
    Step    2 [00 02]: Executed in [ 0.000 ms]  Opcode[E0]: JMP  ----------  [E0 00 0D]  ---------------------  Stack( 1) [01]
    Step   13 [00 0D]: Executed in [ 0.000 ms]  Opcode[FF]: EXIT  ---------  [FF]  ---------------------------  Stack( 1) [01]
Leftover Stack( 1) [01]
Time to execute program: 0.000 ms
Debug failed with error: PROGRAM_EXITED
Program result: 1
Expected result: 1
Test status: Passed
######################################################################
Running test: jump_if => for loop sum
Program[66] [05 12 34 56 78 01 00 00 03 00 12 03 01 00 01 03 00 12 03 01 00 01 11 03 03 0A D3 03 E2 00 3C 01 00 01 14 01 11 03 03 01 20 03 12 03 01 00 00 14 01 11 03 03 0A 20 03 12 03 E0 00 13 01 00 00 11 03 FF]
#### Program Explanation:
       0 [00 00] - 05: PUSH u32 ------- [size  5] 0512345678
       5 [00 05] - 01: PUSH pointer --- [size  3] 010000
       8 [00 08] - 03: PUSH u8 -------- [size  2] 0300
      10 [00 0A] - 12: MOVE ----------- [size  2] 1203
      12 [00 0C] - 01: PUSH pointer --- [size  3] 010001
      15 [00 0F] - 03: PUSH u8 -------- [size  2] 0300
      17 [00 11] - 12: MOVE ----------- [size  2] 1203
      19 [00 13] - 01: PUSH pointer --- [size  3] 010001
      22 [00 16] - 11: LOAD ----------- [size  2] 1103
      24 [00 18] - 03: PUSH u8 -------- [size  2] 030A
      26 [00 1A] - D3: CMP_LT --------- [size  2] D303
      28 [00 1C] - E2: JMP_IF_NOT ----- [size  3] E2003C
      31 [00 1F] - 01: PUSH pointer --- [size  3] 010001
      34 [00 22] - 14: COPY ----------- [size  2] 1401
      36 [00 24] - 11: LOAD ----------- [size  2] 1103
      38 [00 26] - 03: PUSH u8 -------- [size  2] 0301
      40 [00 28] - 20: ADD ------------ [size  2] 2003
      42 [00 2A] - 12: MOVE ----------- [size  2] 1203
      44 [00 2C] - 01: PUSH pointer --- [size  3] 010000
      47 [00 2F] - 14: COPY ----------- [size  2] 1401
      49 [00 31] - 11: LOAD ----------- [size  2] 1103
      51 [00 33] - 03: PUSH u8 -------- [size  2] 030A
      53 [00 35] - 20: ADD ------------ [size  2] 2003
      55 [00 37] - 12: MOVE ----------- [size  2] 1203
      57 [00 39] - E0: JMP ------------ [size  3] E00013
      60 [00 3C] - 01: PUSH pointer --- [size  3] 010000
      63 [00 3F] - 11: LOAD ----------- [size  2] 1103
      65 [00 41] - FF: EXIT ----------- [size  1] FF
#### End of program explanation.
Starting detailed program debug...
    Step    0 [00 00]: Executed in [ 0.000 ms]  Opcode[05]: PUSH u32  -----  [05 12 34 56 78]  ---------------  Stack( 4) [12 34 56 78]
    Step    5 [00 05]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 00]  ---------------------  Stack( 6) [12 34 56 78 00 00]
    Step    8 [00 08]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 00]  ------------------------  Stack( 7) [12 34 56 78 00 00 00]
    Step   10 [00 0A]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   12 [00 0C]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   15 [00 0F]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 00]  ------------------------  Stack( 7) [12 34 56 78 00 01 00]
    Step   17 [00 11]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   19 [00 13]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   22 [00 16]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 5) [12 34 56 78 00]
    Step   24 [00 18]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 6) [12 34 56 78 00 0A]
    Step   26 [00 1A]: Executed in [ 0.000 ms]  Opcode[D3]: CMP_LT  -------  [D3 03]  ------------------------  Stack( 5) [12 34 56 78 01]
    Step   28 [00 1C]: Executed in [ 0.000 ms]  Opcode[E2]: JMP_IF_NOT  ---  [E2 00 3C]  ---------------------  Stack( 4) [12 34 56 78]
    Step   31 [00 1F]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   34 [00 22]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 00 01]
    Step   36 [00 24]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 00]
    Step   38 [00 26]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 00 01]
    Step   40 [00 28]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 01]
    Step   42 [00 2A]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   44 [00 2C]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 00]  ---------------------  Stack( 6) [12 34 56 78 00 00]
    Step   47 [00 2F]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 00 00 00]
    Step   49 [00 31]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 00]
    Step   51 [00 33]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 8) [12 34 56 78 00 00 00 0A]
    Step   53 [00 35]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 0A]
    Step   55 [00 37]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   57 [00 39]: Executed in [ 0.000 ms]  Opcode[E0]: JMP  ----------  [E0 00 13]  ---------------------  Stack( 4) [12 34 56 78]
    Step   19 [00 13]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   22 [00 16]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 5) [12 34 56 78 01]
    Step   24 [00 18]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 6) [12 34 56 78 01 0A]
    Step   26 [00 1A]: Executed in [ 0.000 ms]  Opcode[D3]: CMP_LT  -------  [D3 03]  ------------------------  Stack( 5) [12 34 56 78 01]
    Step   28 [00 1C]: Executed in [ 0.000 ms]  Opcode[E2]: JMP_IF_NOT  ---  [E2 00 3C]  ---------------------  Stack( 4) [12 34 56 78]
    Step   31 [00 1F]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   34 [00 22]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 00 01]
    Step   36 [00 24]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 01]
    Step   38 [00 26]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 01 01]
    Step   40 [00 28]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 02]
    Step   42 [00 2A]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   44 [00 2C]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 00]  ---------------------  Stack( 6) [12 34 56 78 00 00]
    Step   47 [00 2F]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 00 00 00]
    Step   49 [00 31]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 0A]
    Step   51 [00 33]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 8) [12 34 56 78 00 00 0A 0A]
    Step   53 [00 35]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 14]
    Step   55 [00 37]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   57 [00 39]: Executed in [ 0.000 ms]  Opcode[E0]: JMP  ----------  [E0 00 13]  ---------------------  Stack( 4) [12 34 56 78]
    Step   19 [00 13]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   22 [00 16]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 5) [12 34 56 78 02]
    Step   24 [00 18]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 6) [12 34 56 78 02 0A]
    Step   26 [00 1A]: Executed in [ 0.000 ms]  Opcode[D3]: CMP_LT  -------  [D3 03]  ------------------------  Stack( 5) [12 34 56 78 01]
    Step   28 [00 1C]: Executed in [ 0.000 ms]  Opcode[E2]: JMP_IF_NOT  ---  [E2 00 3C]  ---------------------  Stack( 4) [12 34 56 78]
    Step   31 [00 1F]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   34 [00 22]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 00 01]
    Step   36 [00 24]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 02]
    Step   38 [00 26]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 02 01]
    Step   40 [00 28]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 03]
    Step   42 [00 2A]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   44 [00 2C]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 00]  ---------------------  Stack( 6) [12 34 56 78 00 00]
    Step   47 [00 2F]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 00 00 00]
    Step   49 [00 31]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 14]
    Step   51 [00 33]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 8) [12 34 56 78 00 00 14 0A]
    Step   53 [00 35]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 1E]
    Step   55 [00 37]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   57 [00 39]: Executed in [ 0.000 ms]  Opcode[E0]: JMP  ----------  [E0 00 13]  ---------------------  Stack( 4) [12 34 56 78]
    Step   19 [00 13]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   22 [00 16]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 5) [12 34 56 78 03]
    Step   24 [00 18]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 6) [12 34 56 78 03 0A]
    Step   26 [00 1A]: Executed in [ 0.000 ms]  Opcode[D3]: CMP_LT  -------  [D3 03]  ------------------------  Stack( 5) [12 34 56 78 01]
    Step   28 [00 1C]: Executed in [ 0.000 ms]  Opcode[E2]: JMP_IF_NOT  ---  [E2 00 3C]  ---------------------  Stack( 4) [12 34 56 78]
    Step   31 [00 1F]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   34 [00 22]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 00 01]
    Step   36 [00 24]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 03]
    Step   38 [00 26]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 03 01]
    Step   40 [00 28]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 04]
    Step   42 [00 2A]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   44 [00 2C]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 00]  ---------------------  Stack( 6) [12 34 56 78 00 00]
    Step   47 [00 2F]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 00 00 00]
    Step   49 [00 31]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 1E]
    Step   51 [00 33]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 8) [12 34 56 78 00 00 1E 0A]
    Step   53 [00 35]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 28]
    Step   55 [00 37]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   57 [00 39]: Executed in [ 0.000 ms]  Opcode[E0]: JMP  ----------  [E0 00 13]  ---------------------  Stack( 4) [12 34 56 78]
    Step   19 [00 13]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   22 [00 16]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 5) [12 34 56 78 04]
    Step   24 [00 18]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 6) [12 34 56 78 04 0A]
    Step   26 [00 1A]: Executed in [ 0.000 ms]  Opcode[D3]: CMP_LT  -------  [D3 03]  ------------------------  Stack( 5) [12 34 56 78 01]
    Step   28 [00 1C]: Executed in [ 0.000 ms]  Opcode[E2]: JMP_IF_NOT  ---  [E2 00 3C]  ---------------------  Stack( 4) [12 34 56 78]
    Step   31 [00 1F]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   34 [00 22]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 00 01]
    Step   36 [00 24]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 04]
    Step   38 [00 26]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 04 01]
    Step   40 [00 28]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 05]
    Step   42 [00 2A]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   44 [00 2C]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 00]  ---------------------  Stack( 6) [12 34 56 78 00 00]
    Step   47 [00 2F]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 00 00 00]
    Step   49 [00 31]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 28]
    Step   51 [00 33]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 8) [12 34 56 78 00 00 28 0A]
    Step   53 [00 35]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 32]
    Step   55 [00 37]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   57 [00 39]: Executed in [ 0.000 ms]  Opcode[E0]: JMP  ----------  [E0 00 13]  ---------------------  Stack( 4) [12 34 56 78]
    Step   19 [00 13]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   22 [00 16]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 5) [12 34 56 78 05]
    Step   24 [00 18]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 6) [12 34 56 78 05 0A]
    Step   26 [00 1A]: Executed in [ 0.000 ms]  Opcode[D3]: CMP_LT  -------  [D3 03]  ------------------------  Stack( 5) [12 34 56 78 01]
    Step   28 [00 1C]: Executed in [ 0.000 ms]  Opcode[E2]: JMP_IF_NOT  ---  [E2 00 3C]  ---------------------  Stack( 4) [12 34 56 78]
    Step   31 [00 1F]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   34 [00 22]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 00 01]
    Step   36 [00 24]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 05]
    Step   38 [00 26]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 05 01]
    Step   40 [00 28]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 06]
    Step   42 [00 2A]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   44 [00 2C]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 00]  ---------------------  Stack( 6) [12 34 56 78 00 00]
    Step   47 [00 2F]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 00 00 00]
    Step   49 [00 31]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 32]
    Step   51 [00 33]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 8) [12 34 56 78 00 00 32 0A]
    Step   53 [00 35]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 3C]
    Step   55 [00 37]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   57 [00 39]: Executed in [ 0.000 ms]  Opcode[E0]: JMP  ----------  [E0 00 13]  ---------------------  Stack( 4) [12 34 56 78]
    Step   19 [00 13]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   22 [00 16]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 5) [12 34 56 78 06]
    Step   24 [00 18]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 6) [12 34 56 78 06 0A]
    Step   26 [00 1A]: Executed in [ 0.000 ms]  Opcode[D3]: CMP_LT  -------  [D3 03]  ------------------------  Stack( 5) [12 34 56 78 01]
    Step   28 [00 1C]: Executed in [ 0.000 ms]  Opcode[E2]: JMP_IF_NOT  ---  [E2 00 3C]  ---------------------  Stack( 4) [12 34 56 78]
    Step   31 [00 1F]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   34 [00 22]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 00 01]
    Step   36 [00 24]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 06]
    Step   38 [00 26]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 06 01]
    Step   40 [00 28]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 07]
    Step   42 [00 2A]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   44 [00 2C]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 00]  ---------------------  Stack( 6) [12 34 56 78 00 00]
    Step   47 [00 2F]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 00 00 00]
    Step   49 [00 31]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 3C]
    Step   51 [00 33]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 8) [12 34 56 78 00 00 3C 0A]
    Step   53 [00 35]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 46]
    Step   55 [00 37]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   57 [00 39]: Executed in [ 0.000 ms]  Opcode[E0]: JMP  ----------  [E0 00 13]  ---------------------  Stack( 4) [12 34 56 78]
    Step   19 [00 13]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   22 [00 16]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 5) [12 34 56 78 07]
    Step   24 [00 18]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 6) [12 34 56 78 07 0A]
    Step   26 [00 1A]: Executed in [ 0.000 ms]  Opcode[D3]: CMP_LT  -------  [D3 03]  ------------------------  Stack( 5) [12 34 56 78 01]
    Step   28 [00 1C]: Executed in [ 0.000 ms]  Opcode[E2]: JMP_IF_NOT  ---  [E2 00 3C]  ---------------------  Stack( 4) [12 34 56 78]
    Step   31 [00 1F]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   34 [00 22]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 00 01]
    Step   36 [00 24]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 07]
    Step   38 [00 26]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 07 01]
    Step   40 [00 28]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 08]
    Step   42 [00 2A]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   44 [00 2C]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 00]  ---------------------  Stack( 6) [12 34 56 78 00 00]
    Step   47 [00 2F]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 00 00 00]
    Step   49 [00 31]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 46]
    Step   51 [00 33]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 8) [12 34 56 78 00 00 46 0A]
    Step   53 [00 35]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 50]
    Step   55 [00 37]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   57 [00 39]: Executed in [ 0.000 ms]  Opcode[E0]: JMP  ----------  [E0 00 13]  ---------------------  Stack( 4) [12 34 56 78]
    Step   19 [00 13]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   22 [00 16]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 5) [12 34 56 78 08]
    Step   24 [00 18]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 6) [12 34 56 78 08 0A]
    Step   26 [00 1A]: Executed in [ 0.000 ms]  Opcode[D3]: CMP_LT  -------  [D3 03]  ------------------------  Stack( 5) [12 34 56 78 01]
    Step   28 [00 1C]: Executed in [ 0.000 ms]  Opcode[E2]: JMP_IF_NOT  ---  [E2 00 3C]  ---------------------  Stack( 4) [12 34 56 78]
    Step   31 [00 1F]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   34 [00 22]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 00 01]
    Step   36 [00 24]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 08]
    Step   38 [00 26]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 08 01]
    Step   40 [00 28]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 09]
    Step   42 [00 2A]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   44 [00 2C]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 00]  ---------------------  Stack( 6) [12 34 56 78 00 00]
    Step   47 [00 2F]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 00 00 00]
    Step   49 [00 31]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 50]
    Step   51 [00 33]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 8) [12 34 56 78 00 00 50 0A]
    Step   53 [00 35]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 5A]
    Step   55 [00 37]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   57 [00 39]: Executed in [ 0.000 ms]  Opcode[E0]: JMP  ----------  [E0 00 13]  ---------------------  Stack( 4) [12 34 56 78]
    Step   19 [00 13]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   22 [00 16]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 5) [12 34 56 78 09]
    Step   24 [00 18]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 6) [12 34 56 78 09 0A]
    Step   26 [00 1A]: Executed in [ 0.000 ms]  Opcode[D3]: CMP_LT  -------  [D3 03]  ------------------------  Stack( 5) [12 34 56 78 01]
    Step   28 [00 1C]: Executed in [ 0.000 ms]  Opcode[E2]: JMP_IF_NOT  ---  [E2 00 3C]  ---------------------  Stack( 4) [12 34 56 78]
    Step   31 [00 1F]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   34 [00 22]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 00 01]
    Step   36 [00 24]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 09]
    Step   38 [00 26]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 01]  ------------------------  Stack( 8) [12 34 56 78 00 01 09 01]
    Step   40 [00 28]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 01 0A]
    Step   42 [00 2A]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   44 [00 2C]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 00]  ---------------------  Stack( 6) [12 34 56 78 00 00]
    Step   47 [00 2F]: Executed in [ 0.000 ms]  Opcode[14]: COPY  ---------  [14 01]  ------------------------  Stack( 8) [12 34 56 78 00 00 00 00]
    Step   49 [00 31]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 5A]
    Step   51 [00 33]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 8) [12 34 56 78 00 00 5A 0A]
    Step   53 [00 35]: Executed in [ 0.000 ms]  Opcode[20]: ADD  ----------  [20 03]  ------------------------  Stack( 7) [12 34 56 78 00 00 64]
    Step   55 [00 37]: Executed in [ 0.000 ms]  Opcode[12]: MOVE  ---------  [12 03]  ------------------------  Stack( 4) [12 34 56 78]
    Step   57 [00 39]: Executed in [ 0.000 ms]  Opcode[E0]: JMP  ----------  [E0 00 13]  ---------------------  Stack( 4) [12 34 56 78]
    Step   19 [00 13]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 01]  ---------------------  Stack( 6) [12 34 56 78 00 01]
    Step   22 [00 16]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 5) [12 34 56 78 0A]
    Step   24 [00 18]: Executed in [ 0.000 ms]  Opcode[03]: PUSH u8  ------  [03 0A]  ------------------------  Stack( 6) [12 34 56 78 0A 0A]
    Step   26 [00 1A]: Executed in [ 0.000 ms]  Opcode[D3]: CMP_LT  -------  [D3 03]  ------------------------  Stack( 5) [12 34 56 78 00]
    Step   28 [00 1C]: Executed in [ 0.000 ms]  Opcode[E2]: JMP_IF_NOT  ---  [E2 00 3C]  ---------------------  Stack( 4) [12 34 56 78]
    Step   60 [00 3C]: Executed in [ 0.000 ms]  Opcode[01]: PUSH pointer  -  [01 00 00]  ---------------------  Stack( 6) [12 34 56 78 00 00]
    Step   63 [00 3F]: Executed in [ 0.000 ms]  Opcode[11]: LOAD  ---------  [11 03]  ------------------------  Stack( 5) [12 34 56 78 64]
    Step   65 [00 41]: Executed in [ 0.000 ms]  Opcode[FF]: EXIT  ---------  [FF]  ---------------------------  Stack( 5) [12 34 56 78 64]
Leftover Stack( 5) [12 34 56 78 64]
Time to execute program: 0.000 ms
Debug failed with error: PROGRAM_EXITED
Program result: 100
Expected result: 100
Test status: Passed
----------------------------------------------------------------------
######################################################################
Runtime Unit Tests Completed.
######################################################################
######################################################################
Report:
######################################################################
Test "demo_uint8_t => (1 + 2) * 3"      Passed - 0.000 ms
Test "demo_uint16_t => (1 + 2) * 3"     Passed - 0.000 ms
Test "demo_uint32_t => (1 + 2) * 3"     Passed - 0.000 ms
Test "demo_uint64_t => (1 + 2) * 3"     Passed - 0.000 ms
Test "demo_int8_t => (1 - 2) * 3"       Passed - 0.000 ms
Test "demo_float => (0.1 + 0.2) * -1"   Passed - 0.000 ms
Test "demo_double => (0.1 + 0.2) * -1"  Passed - 0.000 ms
Test "bitwise_and_X8"                   Passed - 0.000 ms
Test "bitwise_and_X16"                  Passed - 0.000 ms
Test "logic_and => true && true"        Passed - 0.000 ms
Test "logic_or => true || false"        Passed - 0.000 ms
Test "cmp_eq_1 => 1 == 1"               Passed - 0.000 ms
Test "cmp_eq_2 => 0.29 == 0.31"         Passed - 0.000 ms
Test "jump => push 1 and jump to exit"  Passed - 0.000 ms
Test "jump_if => for loop sum"          Passed - 0.000 ms
######################################################################
Runtime Unit Tests Report Completed.
######################################################################
Done.
```

