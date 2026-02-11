import VovkPLC from '../dist/VovkPLC.js'
const runtime = new VovkPLC()
await runtime.init()

// Test 1: PLCScript block with an error - does the project compiler surface it?
console.log('=== Test 1: PLCScript error in project ===')
const projectWithError = `
VOVKPLCPROJECT TestProject
VERSION 1.0.0

MEMORY
  OFFSET 0
  AVAILABLE 1024
  S 64
  X 64
  Y 64
  M 512
END_MEMORY

PROGRAM main
  BLOCK LANG=PLCSCRIPT Main
    let x: i32 = 42
    let y: i32 = x + undeclared_var
  END_BLOCK
END_PROGRAM
`
let result = await runtime.compileProject(projectWithError)
console.log('Result keys:', Object.keys(result))
console.log('Bytecode:', result.bytecode)
console.log('Problem:', JSON.stringify(result.problem, null, 2))
console.log('Output:', result.output)

// Test 2: PLCScript block with a warning - does the project compiler surface it?
console.log('\n=== Test 2: PLCScript warning in project ===')
const projectWithWarning = `
VOVKPLCPROJECT TestProject
VERSION 1.0.0

MEMORY
  OFFSET 0
  AVAILABLE 1024
  S 64
  X 64
  Y 64
  M 512
END_MEMORY

PROGRAM main
  BLOCK LANG=PLCSCRIPT Main
    let x: i32 = 42
    if (x > 10) {
      x = 100
    } else {
    }
  END_BLOCK
END_PROGRAM
`
result = await runtime.compileProject(projectWithWarning)
console.log('Result keys:', Object.keys(result))
console.log('Bytecode:', result.bytecode ? 'yes' : 'no')
console.log('Problem:', JSON.stringify(result.problem, null, 2))
console.log('Output:', result.output ? 'yes' : 'no')

// Test 3: Use lintPLCScript directly - does it return problems?
console.log('\n=== Test 3: lintPLCScript direct ===')
const lintResult = await runtime.lintPLCScript('let x: i32 = 42\nlet y: i32 = x + foo\n')
console.log('Problems:', JSON.stringify(lintResult.problems, null, 2))
console.log('Symbols:', JSON.stringify(lintResult.symbols, null, 2))

process.exit(0)
