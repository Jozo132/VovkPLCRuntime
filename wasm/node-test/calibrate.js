#!/usr/bin/env node
// calibrate.js - WCET calibration runner for real hardware targets
//
// This script connects to a VovkPLCRuntime device via serial, uploads
// calibration bytecode programs for each opcode category, measures
// cycle times, and outputs a target profile for use in WCET analysis.
//
// Usage:
//   npm run calibrate                        # Interactive mode
//   npm run calibrate -- --port COM3         # Specify serial port
//   npm run calibrate -- --env stm32f401     # Specify PlatformIO environment
//   npm run calibrate -- --skip-build        # Skip build & upload (device already flashed)
//   npm run calibrate -- --skip-upload       # Build but don't upload
//   npm run calibrate -- --baud 115200       # Specify baud rate (default: 115200)
//   npm run calibrate -- --iterations N      # Measurement cycles (default: 500)
//   npm run calibrate -- --help              # Show help
//
// The calibration flow:
//   1. Select PlatformIO environment (interactive or --env flag)
//   2. Build the calibration firmware
//   3. Upload to the connected device
//   4. Connect via serial
//   5. Query device info (PI command)
//   6. Reset health counters (RH command)
//   7. For each opcode category:
//      a. Stop any running program (PS)
//      b. Upload a calibration bytecode program (PD command)
//      c. Start execution (PR command)
//      d. Wait for N cycles to execute
//      e. Read health stats (PH command) to get cycle time
//      f. Stop program (PS command)
//      g. Reset health counters for next test (RH command)
//   8. Calculate per-opcode nanosecond costs
//   9. Output the target profile as a C++ struct for wcet-targets.h
//      and as JSON for programmatic use
//
// Serial protocol format (matches VovkPLC runtime transport):
//   - Command prefix: 2 raw ASCII chars (e.g., 'P','I')
//   - Payload: hex-encoded byte pairs
//   - CRC8: 1 hex-encoded byte (polynomial 0x31, chained across all parts)
//   - Terminator: '\n'
//   Example: "PI<crc8_hex>\n", "PD<size_hex_u32><data_hex><crc8_hex>\n"

import { SerialPort } from 'serialport'
import { execSync, spawnSync } from 'child_process'
import readline from 'readline'
import path from 'path'
import fs from 'fs'
import { fileURLToPath } from 'url'
import VovkPLC from '../dist/VovkPLC.js'

// Runtime feature flags (matches runtime-types.h PLCRUNTIME_FLAG_* and VovkPLC.RUNTIME_FLAGS)
const RUNTIME_FLAGS = {
    LITTLE_ENDIAN: 0x0001,
    STRINGS:       0x0002,
    COUNTERS:      0x0004,
    TIMERS:        0x0008,
    FFI:           0x0010,
    X64_OPS:       0x0020,
    SAFE_MODE:     0x0040,
    TRANSPORT:     0x0080,
    FLOAT_OPS:     0x0100,
    ADVANCED_MATH: 0x0200,
    OPS_32BIT:     0x0400,
    CVT:           0x0800,
    STACK_OPS:     0x1000,
    BITWISE_OPS:   0x2000,
}

/** Decode a hex flags string (e.g. '3FFF') into a feature-boolean object */
function decodeRuntimeFlags(hexStr) {
    const flags = parseInt(hexStr, 16) || 0
    return {
        raw: flags,
        littleEndian: !!(flags & RUNTIME_FLAGS.LITTLE_ENDIAN),
        strings:      !!(flags & RUNTIME_FLAGS.STRINGS),
        counters:     !!(flags & RUNTIME_FLAGS.COUNTERS),
        timers:       !!(flags & RUNTIME_FLAGS.TIMERS),
        ffi:          !!(flags & RUNTIME_FLAGS.FFI),
        x64Ops:       !!(flags & RUNTIME_FLAGS.X64_OPS),
        safeMode:     !!(flags & RUNTIME_FLAGS.SAFE_MODE),
        transport:    !!(flags & RUNTIME_FLAGS.TRANSPORT),
        floatOps:     !!(flags & RUNTIME_FLAGS.FLOAT_OPS),
        advancedMath: !!(flags & RUNTIME_FLAGS.ADVANCED_MATH),
        ops32bit:     !!(flags & RUNTIME_FLAGS.OPS_32BIT),
        cvt:          !!(flags & RUNTIME_FLAGS.CVT),
        stackOps:     !!(flags & RUNTIME_FLAGS.STACK_OPS),
        bitwiseOps:   !!(flags & RUNTIME_FLAGS.BITWISE_OPS),
    }
}

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

// ============================================================================
// ANSI Colors
// ============================================================================

const RED = '\x1b[31m'
const GREEN = '\x1b[32m'
const YELLOW = '\x1b[33m'
const CYAN = '\x1b[36m'
const DIM = '\x1b[2m'
const BOLD = '\x1b[1m'
const RESET = '\x1b[0m'

// ============================================================================
// CLI argument parsing
// ============================================================================

const args = process.argv.slice(2)
const getArg = (name) => {
    const idx = args.findIndex(a => a === name || a.startsWith(name + '='))
    if (idx < 0) return null
    if (args[idx].includes('=')) return args[idx].split('=')[1]
    return args[idx + 1] || true
}

const showHelp = args.includes('--help') || args.includes('-h')
const doUpload = args.includes('--upload')
const doPatch = args.includes('--patch')
const doJson = args.includes('--json')

// npm may strip --flag prefixes and pass values as bare positional args.
// Detect positional args: COM ports match /^COM\d+$/i, env names are the rest.
const positionalArgs = args.filter(a => !a.startsWith('-') && !args.some((b, i) => b.startsWith('--') && args[i + 1] === a))
let selectedPort = getArg('--port')
let selectedEnv = getArg('--env')
if (!selectedPort || !selectedEnv) {
    for (const arg of positionalArgs) {
        if (!selectedPort && /^COM\d+$/i.test(arg)) { selectedPort = arg; continue }
        if (!selectedEnv && !selectedPort && /^\d+$/.test(arg)) continue // skip bare numbers unless it's an env
        if (!selectedEnv && arg.length > 2 && !arg.startsWith('-')) { selectedEnv = arg }
    }
}

const baudRate = parseInt(getArg('--baud') || '115200', 10)
const measureIterations = parseInt(getArg('--iterations') || '500', 10)

if (showHelp) {
    console.log(`
${BOLD}WCET Calibration Runner${RESET}

Measures real opcode execution times on hardware targets for WCET analysis.
By default, connects directly to the device and runs calibration (no build/upload).

${CYAN}Usage:${RESET}
  npm run calibrate                        Interactive mode
  npm run calibrate -- --port COM3         Specify serial port
  npm run calibrate -- --env stm32f401     Specify PlatformIO environment
  npm run calibrate -- --upload            Build & upload firmware before calibrating
  npm run calibrate -- --patch             Patch calibration results into wcet-targets.h
  npm run calibrate -- --json              Output JSON profile and save to file
  npm run calibrate -- --baud 115200       Serial baud rate
  npm run calibrate -- --iterations 500    Measurement cycles per test

${CYAN}Requirements:${RESET}
  - serialport npm package (npm install serialport)
  - Target device connected via USB/serial
  - Calibration firmware already flashed (or use --upload)

${CYAN}Output:${RESET}
  - Per-opcode-category nanosecond costs
  - C++ target profile struct for wcet-targets.h (auto-patched with --patch)
  - JSON profile saved to calibration/results/
`)
    process.exit(0)
}

// ============================================================================
// PlatformIO helpers
// ============================================================================

const platformTesterDir = path.resolve(__dirname, 'calibration/platform_tester')

function parseEnvironments() {
    const iniPath = path.join(platformTesterDir, 'platformio.ini')
    if (!fs.existsSync(iniPath)) {
        console.error(`${RED}Error: platformio.ini not found at ${iniPath}${RESET}`)
        process.exit(1)
    }
    const content = fs.readFileSync(iniPath, 'utf-8')
    const envs = []
    const lines = content.split('\n')
    let currentEnv = null

    for (const line of lines) {
        const t = line.trim()
        const m = t.match(/^\[env:([^\]]+)\]/)
        if (m) {
            if (currentEnv) envs.push(currentEnv)
            currentEnv = { name: m[1], platform: '', board: '', monitor_speed: '115200' }
            continue
        }
        if (currentEnv && !t.startsWith('[') && t.includes('=')) {
            const [key, ...vp] = t.split('=')
            const val = vp.join('=').trim().split(';')[0].trim()
            const k = key.trim()
            if (k === 'platform') currentEnv.platform = val
            if (k === 'board') currentEnv.board = val
            if (k === 'monitor_speed') currentEnv.monitor_speed = val
        }
    }
    if (currentEnv) envs.push(currentEnv)
    return envs
}

// ============================================================================
// Interactive prompts
// ============================================================================

function createPrompt() {
    return readline.createInterface({ input: process.stdin, output: process.stdout })
}

async function askQuestion(rl, question) {
    return new Promise(resolve => rl.question(question, resolve))
}

async function selectEnvironment(environments) {
    if (selectedEnv) {
        const env = environments.find(e => e.name.toLowerCase().includes(selectedEnv.toLowerCase()))
        if (!env) {
            console.error(`${RED}No environment matching '${selectedEnv}'${RESET}`)
            process.exit(1)
        }
        return env
    }

    const rl = createPrompt()
    console.log(`\n${BOLD}Available calibration targets:${RESET}\n`)

    let lastPlatform = ''
    environments.forEach((env, i) => {
        if (env.platform !== lastPlatform) {
            lastPlatform = env.platform
            console.log(`  ${CYAN}--- ${env.platform.toUpperCase()} ---${RESET}`)
        }
        console.log(`  ${BOLD}${String(i).padStart(2)}${RESET}  ${env.name}  ${DIM}(${env.board})${RESET}`)
    })

    const answer = await askQuestion(rl, `\n${YELLOW}Select target [0-${environments.length - 1}]: ${RESET}`)
    rl.close()

    const idx = parseInt(answer, 10)
    if (isNaN(idx) || idx < 0 || idx >= environments.length) {
        console.error(`${RED}Invalid selection${RESET}`)
        process.exit(1)
    }
    return environments[idx]
}

async function selectSerialPort() {
    if (selectedPort) return selectedPort

    let ports
    try {
        ports = await SerialPort.list()
    } catch (err) {
        console.error(`${RED}Error listing serial ports: ${err.message}${RESET}`)
        console.error(`${YELLOW}Install serialport: npm install serialport${RESET}`)
        process.exit(1)
    }

    if (ports.length === 0) {
        console.error(`${RED}No serial ports found. Is the device connected?${RESET}`)
        process.exit(1)
    }

    if (ports.length === 1) {
        console.log(`${GREEN}Auto-selected port: ${ports[0].path}${RESET}`)
        return ports[0].path
    }

    const rl = createPrompt()
    console.log(`\n${BOLD}Available serial ports:${RESET}\n`)
    ports.forEach((p, i) => {
        const info = [p.manufacturer, p.serialNumber].filter(Boolean).join(' ')
        console.log(`  ${BOLD}${i}${RESET}  ${p.path}  ${DIM}${info}${RESET}`)
    })

    const answer = await askQuestion(rl, `\n${YELLOW}Select port [0-${ports.length - 1}]: ${RESET}`)
    rl.close()

    const idx = parseInt(answer, 10)
    if (isNaN(idx) || idx < 0 || idx >= ports.length) {
        console.error(`${RED}Invalid selection${RESET}`)
        process.exit(1)
    }
    return ports[idx].path
}

// ============================================================================
// CRC8 calculation (matches VovkPLC protocol — polynomial 0x31 with chaining)
// ============================================================================

function crc8(data, crc = 0) {
    for (let i = 0; i < data.length; i++) {
        crc ^= data[i]
        for (let k = 0; k < 8; k++)
            crc = (crc & 0x80) ? ((crc << 1) ^ 0x31) : (crc << 1)
        crc &= 0xFF
    }
    return crc
}

/** Parse hex string to byte array: "5049" → [0x50, 0x49] */
function parseHex(hexStr) {
    const hex = hexStr.replace(/[^0-9a-fA-F]/g, '')
    const bytes = []
    for (let i = 0; i + 1 < hex.length; i += 2)
        bytes.push(parseInt(hex.substring(i, i + 2), 16))
    return bytes
}

/** Convert string to hex of ASCII codes: "PI" → "5049" */
function stringToHex(str) {
    return [...str].map(c => c.charCodeAt(0).toString(16).padStart(2, '0')).join('')
}

// ============================================================================
// Serial protocol command builders
// Matches VovkPLC.js buildCommand format:
//   - Simple commands: "<CMD><crc8_hex>" (e.g., "PI02")
//   - PD: "PD<size_hex_u32><data_hex><crc8_hex>"
//   - All commands sent uppercase with "\n" terminator
// ============================================================================

/** Build a simple 2-char command (PI, PH, RH, PR, PS, PU, RS, etc.) */
function buildSimpleCommand(cmd) {
    const cmdBytes = parseHex(stringToHex(cmd))
    const checksum = crc8(cmdBytes)
    return (cmd + checksum.toString(16).padStart(2, '0')).toUpperCase()
}

/** Build PD (program download) command with chained CRC8 */
function buildProgramDownload(bytecode) {
    const cmd = 'PD'
    let checksum = crc8(parseHex(stringToHex(cmd)))
    const sizeHex = bytecode.length.toString(16).padStart(8, '0')
    checksum = crc8(parseHex(sizeHex), checksum)
    checksum = crc8(bytecode, checksum)
    const dataHex = bytecode.map(b => b.toString(16).padStart(2, '0')).join('')
    return (cmd + sizeHex + dataHex + checksum.toString(16).padStart(2, '0')).toUpperCase()
}

// ============================================================================
// Serial client — communicates with VovkPLC runtime over serial transport
// ============================================================================

class PLCSerialClient {
    constructor(portPath, baud = 115200) {
        this.portPath = portPath
        this.baud = baud
        this.port = null
        this.buffer = ''
        this.lineQueue = []       // Accumulated complete lines
        this.lineResolve = null   // Pending line reader resolve
        this.lineTimeout = null   // Pending line reader timeout
    }

    async connect() {
        return new Promise((resolve, reject) => {
            this.port = new SerialPort({
                path: this.portPath,
                baudRate: this.baud,
                autoOpen: false
            })

            this.port.on('data', (chunk) => {
                this.buffer += chunk.toString('utf8')
                // Process complete lines from the buffer
                while (true) {
                    const nlIdx = this.buffer.indexOf('\n')
                    if (nlIdx < 0) break
                    const line = this.buffer.substring(0, nlIdx).replace(/\r/g, '').trim()
                    this.buffer = this.buffer.substring(nlIdx + 1)
                    if (line.length === 0) continue
                    if (this.lineResolve) {
                        clearTimeout(this.lineTimeout)
                        const resolve = this.lineResolve
                        this.lineResolve = null
                        this.lineTimeout = null
                        resolve(line)
                    } else {
                        this.lineQueue.push(line)
                    }
                }
            })

            this.port.on('error', (err) => {
                if (this.lineResolve) {
                    clearTimeout(this.lineTimeout)
                    this.lineResolve(null)
                    this.lineResolve = null
                }
                reject(err)
            })

            this.port.open((err) => {
                if (err) return reject(err)
                // Set DTR high — STM32 USB CDC requires DTR for Serial to become active
                this.port.set({ dtr: true, rts: true }, () => {
                    // Wait for device to be ready after opening (USB CDC handshake)
                    setTimeout(resolve, 2500)
                })
            })
        })
    }

    async close() {
        if (this.port && this.port.isOpen) {
            return new Promise(resolve => this.port.close(resolve))
        }
    }

    /** Check if the serial port is connected and open */
    get isConnected() {
        return this.port != null && this.port.isOpen
    }

    /** Drain the serial line queue and internal buffer */
    drain() {
        this.lineQueue.length = 0
        this.buffer = ''
    }

    /** Wait for a complete line from the device with timeout */
    async readLine(timeoutMs = 5000) {
        // Check if we already have a queued line
        if (this.lineQueue.length > 0) return this.lineQueue.shift()
        return new Promise((resolve) => {
            this.lineTimeout = setTimeout(() => {
                this.lineResolve = null
                this.lineTimeout = null
                resolve(null)
            }, timeoutMs)
            this.lineResolve = resolve
        })
    }

    /** Write data to serial port and wait for it to flush */
    async write(data) {
        return new Promise((resolve, reject) => {
            this.port.write(data, (err) => {
                if (err) return reject(err)
                this.port.drain((err2) => {
                    if (err2) return reject(err2)
                    resolve()
                })
            })
        })
    }

    /** Send a simple 2-char command and read one response line */
    async sendSimple(cmd, timeoutMs = 5000) {
        this.drain()
        const frame = buildSimpleCommand(cmd) + '\n'
        await this.write(frame)
        return this.readLine(timeoutMs)
    }

    /** Ping: send '?\n' and check for <VovkPLC> response */
    async ping() {
        this.drain()
        await this.write('?\n')
        const response = await this.readLine(3000)
        return response && response.includes('VovkPLC')
    }

    /** PI: Get device info — firmware responds with "PLC INFO - [VovkPLCRuntime,...]" */
    async getDeviceInfo() {
        const response = await this.sendSimple('PI', 5000)
        if (!response) return null

        // Extract bracketed content: "PLC INFO - [VovkPLCRuntime,...,device]"
        const start = response.indexOf('[')
        const end = response.indexOf(']')
        if (start < 0 || end < 0 || start >= end) return null

        const parts = response.substring(start + 1, end).split(',')
        if (parts.length < 10) return null

        return {
            raw: response.substring(start, end + 1),
            runtime: parts[0],
            arch: parts[1],
            version: `${parts[2]}.${parts[3]}.${parts[4]}`,
            build: parseInt(parts[5]),
            timestamp: parts[6],
            stack_size: parseInt(parts[7]),
            memory_size: parseInt(parts[8]),
            program_size: parseInt(parts[9]),
            system_offset: parseInt(parts[10] || '0'),
            system_count: parseInt(parts[11] || '0'),
            input_offset: parseInt(parts[12] || '0'),
            input_count: parseInt(parts[13] || '0'),
            output_offset: parseInt(parts[14] || '0'),
            output_count: parseInt(parts[15] || '0'),
            marker_offset: parseInt(parts[16] || '0'),
            marker_count: parseInt(parts[17] || '0'),
            flags: parts[parts.length - 2] || '0000',
            device_name: parts[parts.length - 1] || 'Unknown'
        }
    }

    /** PH: Get health stats — firmware responds with "PH<13×u32_hex>" */
    async getHealth() {
        const response = await this.sendSimple('PH', 5000)
        if (!response) return null

        // Find the PH prefix and extract hex data
        const phIdx = response.indexOf('PH')
        if (phIdx < 0) return null
        const hex = response.substring(phIdx + 2)
        if (hex.length < 104) return null

        const readU32 = (offset) => parseInt(hex.substring(offset, offset + 8), 16) >>> 0
        return {
            last_cycle_us: readU32(0),
            min_cycle_us: readU32(8),
            max_cycle_us: readU32(16),
            last_ram_free: readU32(24),
            min_ram_free: readU32(32),
            max_ram_free: readU32(40),
            total_ram: readU32(48),
            last_period_us: readU32(56),
            min_period_us: readU32(64),
            max_period_us: readU32(72),
            last_jitter_us: readU32(80),
            min_jitter_us: readU32(88),
            max_jitter_us: readU32(96),
        }
    }

    /** RH: Reset health counters — firmware responds with "OK HEALTH RESET" */
    async resetHealth() {
        const response = await this.sendSimple('RH', 3000)
        return response !== null && response.includes('OK')
    }

    /** PD: Download program to device — firmware responds with "PROGRAM DOWNLOAD COMPLETE" */
    async downloadProgram(bytecode) {
        this.drain()
        const frame = buildProgramDownload(bytecode) + '\n'
        await this.write(frame)
        const response = await this.readLine(10000)
        return response !== null && response.includes('DOWNLOAD COMPLETE')
    }

    /** PR: Start program execution — runtime sets is_running = true */
    async startProgram() {
        const response = await this.sendSimple('PR', 3000)
        return response !== null && response.includes('OK')
    }

    /** PS: Stop program execution — runtime sets is_running = false */
    async stopProgram() {
        const response = await this.sendSimple('PS', 3000)
        return response !== null && response.includes('OK')
    }

    /** RS: Reset PLC (clears program, resets state) */
    async plcReset() {
        const response = await this.sendSimple('RS', 3000)
        return response !== null && response.includes('OK')
    }
}

// ============================================================================
// PLCASM-based calibration program definitions
// ============================================================================
// Each program is defined as PLCASM assembly source code, compiled at runtime
// via the VovkPLC.js WASM compiler. This ensures the bytecode is always
// consistent with the current instruction set architecture — if opcode values
// change, the programs automatically adapt.
//
// Structure of each test:
//   1. Push setup values onto stack
//   2. Execute the target opcode(s) N times
//   3. Clean up the stack (drop results)
//   4. EXIT
//
// The runtime measures the full cycle time. By comparing against a baseline
// (EXIT-only), we isolate per-opcode cost.

/** VovkPLC WASM compiler instance, initialized once in main() */
let plc = null

/**
 * Initialize the VovkPLC WASM compiler for PLCASM compilation.
 * Must be called before generateCalibrationPrograms().
 */
async function initCompiler() {
    const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')
    if (!fs.existsSync(wasmPath)) {
        console.error(`${RED}WASM file not found: ${wasmPath}${RESET}`)
        console.error('Run "npm run build" first.')
        process.exit(1)
    }
    plc = new VovkPLC()
    await plc.initialize(wasmPath, false, true) // debug=false, silent=true

    // Register the same FFI functions that the firmware has so the PLCASM
    // compiler can resolve `ffi F_add ...` instructions during compilation.
    // The actual callbacks here are dummies — only the signature matters for
    // bytecode generation (param count, types, return address).
    plc.registerFFI('F_add',      'i32,i32->i32',      'Add two i32',        (a, b) => a + b)
    plc.registerFFI('F_mul',      'i32,i32->i32',      'Multiply two i32',   (a, b) => a * b)
    plc.registerFFI('F_lerp',     'f32,f32,f32->f32',  'Linear interpolate', (a, b, t) => a + (b - a) * t)
    plc.registerFFI('F_in_range', 'i32,i32,i32->bool', 'Range check',        (v, lo, hi) => v >= lo && v <= hi)
}

/**
 * Compile a PLCASM source string to a bytecode array via VovkPLC WASM compiler.
 * @param {string} source - PLCASM assembly source
 * @returns {number[]} - Array of bytecode bytes
 */
function compilePLCASM(source) {
    if (!plc) throw new Error('VovkPLC compiler not initialized — call initCompiler() first')
    const result = plc.compilePLCASM(source)
    // result.output is a continuous hex string: "032AFF" (2 hex chars per byte, no spaces)
    const hex = result.output
    const bytes = []
    for (let i = 0; i < hex.length; i += 2) {
        bytes.push(parseInt(hex.substring(i, i + 2), 16))
    }
    return bytes
}

/**
 * Calibration program definition using PLCASM source code.
 * @typedef {{
 *   name: string,
 *   category: string,
 *   description: string,
 *   source: string,
 *   target_ops: number,
 *   requires_flags?: number,
 * }} CalibrationProgramDef
 */

/**
 * Generate calibration programs by compiling PLCASM assembly for each opcode
 * category. Returns bytecode arrays ready for downloading to the device.
 *
 * @returns {{ name: string, category: string, description: string, bytecode: number[], target_ops: number }[]}
 */
function generateCalibrationPrograms() {
    /** @type {CalibrationProgramDef[]} */
    const defs = []

    // ── Baseline: just EXIT ────────────────────────────────────────────
    defs.push({
        name: 'baseline',
        category: 'DISPATCH',
        description: 'EXIT only — measures interpreter dispatch overhead',
        source: 'exit',
        target_ops: 0,
    })

    // ── PUSH_U8 ────────────────────────────────────────────────────────
    defs.push({
        name: 'push_u8',
        category: 'PUSH_U8',
        description: 'u8.const × 8 + u8.drop × 8',
        source: [
            'u8.const 66', 'u8.const 66', 'u8.const 66', 'u8.const 66',
            'u8.const 66', 'u8.const 66', 'u8.const 66', 'u8.const 66',
            'u8.drop', 'u8.drop', 'u8.drop', 'u8.drop',
            'u8.drop', 'u8.drop', 'u8.drop', 'u8.drop',
            'exit',
        ].join('\n'),
        target_ops: 8,
    })

    // ── PUSH_U16 ───────────────────────────────────────────────────────
    defs.push({
        name: 'push_u16',
        category: 'PUSH_U16',
        description: 'u16.const × 4 + u16.drop × 4',
        source: [
            'u16.const 1000', 'u16.const 2000', 'u16.const 3000', 'u16.const 4000',
            'u16.drop', 'u16.drop', 'u16.drop', 'u16.drop',
            'exit',
        ].join('\n'),
        target_ops: 4,
    })

    // ── PUSH_U32 ───────────────────────────────────────────────────────
    defs.push({
        name: 'push_u32',
        category: 'PUSH_U32',
        description: 'u32.const × 4 + u32.drop × 4',
        source: [
            'u32.const 256', 'u32.const 512', 'u32.const 768', 'u32.const 1024',
            'u32.drop', 'u32.drop', 'u32.drop', 'u32.drop',
            'exit',
        ].join('\n'),
        target_ops: 4,
    })

    // ── PUSH_F32 ───────────────────────────────────────────────────────
    defs.push({
        name: 'push_f32',
        category: 'PUSH_F32',
        description: 'f32.const × 4 + f32.drop × 4',
        source: [
            'f32.const 10.0', 'f32.const 20.0', 'f32.const 30.0', 'f32.const 40.0',
            'f32.drop', 'f32.drop', 'f32.drop', 'f32.drop',
            'exit',
        ].join('\n'),
        target_ops: 4,
    })

    // ── PUSH_U64 ───────────────────────────────────────────────────────
    defs.push({
        name: 'push_u64',
        category: 'PUSH_U64',
        description: 'u64.const × 2 + u64.drop × 2',
        source: [
            'u64.const 999999', 'u64.const 888888',
            'u64.drop', 'u64.drop',
            'exit',
        ].join('\n'),
        target_ops: 2,
        requires_flags: RUNTIME_FLAGS.X64_OPS,
    })

    // ── PUSH_F64 ───────────────────────────────────────────────────────
    defs.push({
        name: 'push_f64',
        category: 'PUSH_F64',
        description: 'f64.const × 2 + f64.drop × 2',
        source: [
            'f64.const 3.14159', 'f64.const 2.71828',
            'f64.drop', 'f64.drop',
            'exit',
        ].join('\n'),
        target_ops: 2,
        requires_flags: RUNTIME_FLAGS.X64_OPS,
    })

    // ── ADD_U8 ─────────────────────────────────────────────────────────
    defs.push({
        name: 'add_u8',
        category: 'ADD_U8',
        description: 'u8.const × 2 + u8.add × 4, then drop',
        source: [
            'u8.const 10', 'u8.const 20', 'u8.add',
            'u8.const 10', 'u8.const 20', 'u8.add',
            'u8.const 10', 'u8.const 20', 'u8.add',
            'u8.const 10', 'u8.const 20', 'u8.add',
            'u8.drop', 'u8.drop', 'u8.drop', 'u8.drop',
            'exit',
        ].join('\n'),
        target_ops: 4,
    })

    // ── ADD_U16 ────────────────────────────────────────────────────────
    defs.push({
        name: 'add_u16',
        category: 'ADD_U16',
        description: 'u16.const × 2 + u16.add × 2, then drop',
        source: [
            'u16.const 100', 'u16.const 200', 'u16.add',
            'u16.const 300', 'u16.const 400', 'u16.add',
            'u16.drop', 'u16.drop',
            'exit',
        ].join('\n'),
        target_ops: 2,
    })

    // ── ADD_U32 ────────────────────────────────────────────────────────
    defs.push({
        name: 'add_u32',
        category: 'ADD_U32',
        description: 'u32.const × 2 + u32.add × 4, then drop',
        source: [
            'u32.const 10', 'u32.const 20', 'u32.add',
            'u32.const 10', 'u32.const 20', 'u32.add',
            'u32.const 10', 'u32.const 20', 'u32.add',
            'u32.const 10', 'u32.const 20', 'u32.add',
            'u32.drop', 'u32.drop', 'u32.drop', 'u32.drop',
            'exit',
        ].join('\n'),
        target_ops: 4,
    })

    // ── ADD_F32 ────────────────────────────────────────────────────────
    defs.push({
        name: 'add_f32',
        category: 'ADD_F32',
        description: 'f32.const × 2 + f32.add × 2, then drop',
        source: [
            'f32.const 10.0', 'f32.const 20.0', 'f32.add',
            'f32.const 10.0', 'f32.const 20.0', 'f32.add',
            'f32.drop', 'f32.drop',
            'exit',
        ].join('\n'),
        target_ops: 2,
    })

    // ── ADD_F64 ────────────────────────────────────────────────────────
    defs.push({
        name: 'add_f64',
        category: 'ADD_F64',
        description: 'f64.const × 2 + f64.add × 2, then drop',
        source: [
            'f64.const 10.0', 'f64.const 20.0', 'f64.add',
            'f64.const 10.0', 'f64.const 20.0', 'f64.add',
            'f64.drop', 'f64.drop',
            'exit',
        ].join('\n'),
        target_ops: 2,
        requires_flags: RUNTIME_FLAGS.X64_OPS,
    })

    // ── MUL_U8 ─────────────────────────────────────────────────────────
    defs.push({
        name: 'mul_u8',
        category: 'MUL_U8',
        description: 'u8.const × 2 + u8.mul × 2, then drop',
        source: [
            'u8.const 3', 'u8.const 5', 'u8.mul',
            'u8.const 2', 'u8.const 7', 'u8.mul',
            'u8.drop', 'u8.drop',
            'exit',
        ].join('\n'),
        target_ops: 2,
    })

    // ── MUL_U32 ────────────────────────────────────────────────────────
    defs.push({
        name: 'mul_u32',
        category: 'MUL_U32',
        description: 'u32.const × 2 + u32.mul × 2, then drop',
        source: [
            'u32.const 7', 'u32.const 3', 'u32.mul',
            'u32.const 5', 'u32.const 2', 'u32.mul',
            'u32.drop', 'u32.drop',
            'exit',
        ].join('\n'),
        target_ops: 2,
    })

    // ── MUL_F32 ────────────────────────────────────────────────────────
    defs.push({
        name: 'mul_f32',
        category: 'MUL_F32',
        description: 'f32.const × 2 + f32.mul × 2, then drop',
        source: [
            'f32.const 10.0', 'f32.const 3.0', 'f32.mul',
            'f32.const 10.0', 'f32.const 3.0', 'f32.mul',
            'f32.drop', 'f32.drop',
            'exit',
        ].join('\n'),
        target_ops: 2,
    })

    // ── MUL_F64 ────────────────────────────────────────────────────────
    defs.push({
        name: 'mul_f64',
        category: 'MUL_F64',
        description: 'f64.const × 2 + f64.mul × 2, then drop',
        source: [
            'f64.const 10.0', 'f64.const 3.0', 'f64.mul',
            'f64.const 10.0', 'f64.const 3.0', 'f64.mul',
            'f64.drop', 'f64.drop',
            'exit',
        ].join('\n'),
        target_ops: 2,
        requires_flags: RUNTIME_FLAGS.X64_OPS,
    })

    // ── DIV_U8 ─────────────────────────────────────────────────────────
    defs.push({
        name: 'div_u8',
        category: 'DIV_U8',
        description: 'u8.const × 2 + u8.div × 2, then drop',
        source: [
            'u8.const 100', 'u8.const 7', 'u8.div',
            'u8.const 50', 'u8.const 3', 'u8.div',
            'u8.drop', 'u8.drop',
            'exit',
        ].join('\n'),
        target_ops: 2,
    })

    // ── DIV_U32 ────────────────────────────────────────────────────────
    defs.push({
        name: 'div_u32',
        category: 'DIV_U32',
        description: 'u32.const × 2 + u32.div × 2, then drop',
        source: [
            'u32.const 100', 'u32.const 7', 'u32.div',
            'u32.const 50', 'u32.const 3', 'u32.div',
            'u32.drop', 'u32.drop',
            'exit',
        ].join('\n'),
        target_ops: 2,
    })

    // ── DIV_F32 ────────────────────────────────────────────────────────
    defs.push({
        name: 'div_f32',
        category: 'DIV_F32',
        description: 'f32.const × 2 + f32.div × 2, then drop',
        source: [
            'f32.const 100.0', 'f32.const 7.0', 'f32.div',
            'f32.const 50.0', 'f32.const 3.0', 'f32.div',
            'f32.drop', 'f32.drop',
            'exit',
        ].join('\n'),
        target_ops: 2,
    })

    // ── DIV_F64 ────────────────────────────────────────────────────────
    defs.push({
        name: 'div_f64',
        category: 'DIV_F64',
        description: 'f64.const × 2 + f64.div × 2, then drop',
        source: [
            'f64.const 100.0', 'f64.const 7.0', 'f64.div',
            'f64.const 50.0', 'f64.const 3.0', 'f64.div',
            'f64.drop', 'f64.drop',
            'exit',
        ].join('\n'),
        target_ops: 2,
        requires_flags: RUNTIME_FLAGS.X64_OPS,
    })

    // ── CMP (compare) ─────────────────────────────────────────────────
    defs.push({
        name: 'cmp',
        category: 'CMP',
        description: 'u8.cmp_eq/gt/lt × 4, then drop',
        source: [
            'u8.const 10', 'u8.const 10', 'u8.cmp_eq',
            'u8.const 10', 'u8.const 20', 'u8.cmp_gt',
            'u8.const 20', 'u8.const 10', 'u8.cmp_lt',
            'u8.const 10', 'u8.const 10', 'u8.cmp_eq',
            'u8.drop', 'u8.drop', 'u8.drop', 'u8.drop',
            'exit',
        ].join('\n'),
        target_ops: 4,
    })

    // ── LOGIC (boolean AND/OR) ─────────────────────────────────────────
    defs.push({
        name: 'logic',
        category: 'LOGIC',
        description: 'u8.and/u8.or × 4, then drop',
        source: [
            'u8.const 1', 'u8.const 1', 'u8.and',
            'u8.const 0', 'u8.or',
            'u8.const 1', 'u8.and',
            'u8.const 0', 'u8.or',
            'u8.drop',
            'exit',
        ].join('\n'),
        target_ops: 4,
    })

    // ── BITWISE (AND/OR on u8) ─────────────────────────────────────────
    defs.push({
        name: 'bitwise',
        category: 'BITWISE',
        description: 'bw.and.x8/bw.or.x8 × 4, then drop',
        source: [
            'u8.const 255', 'u8.const 15', 'bw.and.x8',
            'u8.const 240', 'bw.or.x8',
            'u8.const 15', 'bw.and.x8',
            'u8.const 240', 'bw.or.x8',
            'u8.drop',
            'exit',
        ].join('\n'),
        target_ops: 4,
    })

    // ── JMP (unconditional) ────────────────────────────────────────────
    defs.push({
        name: 'jmp',
        category: 'JMP',
        description: 'jmp forward × 4',
        source: [
            'jmp j1',
            'nop',
            'j1: jmp j2',
            'nop',
            'j2: jmp j3',
            'nop',
            'j3: jmp j4',
            'nop',
            'j4: exit',
        ].join('\n'),
        target_ops: 4,
    })

    // ── JMP_COND (conditional jump) ────────────────────────────────────
    defs.push({
        name: 'jmp_cond',
        category: 'JMP_COND',
        description: 'jmp_if × 2 + jmp_if_not × 2',
        source: [
            // jmp_if with true — takes the jump
            'u8.const 1', 'jmp_if jc1',
            'nop',
            'jc1:',
            // jmp_if_not with false — takes the jump
            'u8.const 0', 'jmp_if_not jc2',
            'nop',
            'jc2:',
            // jmp_if with false — doesn't jump (still executed)
            'u8.const 0', 'jmp_if jc3',
            'jc3:',
            // jmp_if_not with true — doesn't jump (still executed)
            'u8.const 1', 'jmp_if_not jc4',
            'jc4:',
            'exit',
        ].join('\n'),
        target_ops: 4,
    })

    // ── CALL (subroutine call) ─────────────────────────────────────────
    defs.push({
        name: 'call_ret',
        category: 'CALL',
        description: 'call sub × 4, sub does ret immediately',
        source: [
            'call sub',
            'call sub',
            'call sub',
            'call sub',
            'exit',
            'sub: ret',
        ].join('\n'),
        target_ops: 4,
    })

    // ── RET (subroutine return) ────────────────────────────────────────
    // Note: measures ret by calling a subroutine that does some work + ret
    // The call overhead is subtracted by comparing against the CALL test
    defs.push({
        name: 'ret',
        category: 'RET',
        description: 'call + ret × 4 (ret cost isolated from call)',
        source: [
            'call sub',
            'call sub',
            'call sub',
            'call sub',
            'exit',
            'sub: nop', // Extra NOP so ret cost is distinguishable
            'ret',
        ].join('\n'),
        target_ops: 4,
    })

    // ── LOAD (memory read) ─────────────────────────────────────────────
    defs.push({
        name: 'load',
        category: 'LOAD',
        description: 'u8.load_from × 4, then drop',
        source: [
            'u8.load_from 192',
            'u8.load_from 193',
            'u8.load_from 194',
            'u8.load_from 195',
            'u8.drop', 'u8.drop', 'u8.drop', 'u8.drop',
            'exit',
        ].join('\n'),
        target_ops: 4,
    })

    // ── STORE (memory write) ───────────────────────────────────────────
    defs.push({
        name: 'store',
        category: 'STORE',
        description: 'u8.const + u8.move_to × 4',
        source: [
            'u8.const 42', 'u8.move_to 192',
            'u8.const 43', 'u8.move_to 193',
            'u8.const 44', 'u8.move_to 194',
            'u8.const 45', 'u8.move_to 195',
            'exit',
        ].join('\n'),
        target_ops: 4,
    })

    // ── STACK_OP (copy + drop) ─────────────────────────────────────────
    defs.push({
        name: 'stack_ops',
        category: 'STACK_OP',
        description: 'u8.copy + u8.drop × 3 rounds',
        source: [
            'u8.const 42',
            'u8.copy', 'u8.drop',
            'u8.copy', 'u8.drop',
            'u8.copy', 'u8.drop',
            'u8.drop',
            'exit',
        ].join('\n'),
        target_ops: 6,
    })

    // ── BIT_RW (bit read/write on memory) ──────────────────────────────
    defs.push({
        name: 'bit_rw',
        category: 'BIT_RW',
        description: 'u8.readBit + u8.writeBitOn/Off × 4',
        source: [
            'u8.readBit 192.0',    // read bit 0 of addr 192
            'u8.drop',
            'u8.writeBitOn 192.1', // set bit 1
            'u8.writeBitOff 192.2', // reset bit 2
            'u8.readBit 192.3',
            'u8.drop',
            'exit',
        ].join('\n'),
        target_ops: 4,
    })

    // ── TIMER (timer on-delay) ─────────────────────────────────────────
    // Timers need a bool enable on stack and write to specific addresses.
    // We use .runtime_config to set up timer/counter memory,
    // then measure the timer instruction overhead (not actual timing).
    defs.push({
        name: 'timer',
        category: 'TIMER',
        description: 'ton × 2 (measures instruction overhead, not actual delay)',
        source: [
            '.runtime_config timer_offset 64 counter_offset 128',
            'u8.const 0',   // enable = false (so timer doesn't actually run)
            'ton 0 #1000',  // timer 0, preset 1000ms
            'u8.drop',      // drop Q output
            'u8.const 0',
            'ton 1 #1000',
            'u8.drop',
            'exit',
        ].join('\n'),
        target_ops: 2,
    })

    // ── COUNTER (count up) ─────────────────────────────────────────────
    defs.push({
        name: 'counter',
        category: 'COUNTER',
        description: 'ctu × 2 (measures instruction overhead)',
        source: [
            '.runtime_config timer_offset 64 counter_offset 128',
            'u8.const 0',   // CU = false
            'u8.const 0',   // R = false
            'ctu 0 #10',    // counter 0, preset 10
            'u8.drop',      // drop Q output
            'u8.const 0',
            'u8.const 0',
            'ctu 1 #10',
            'u8.drop',
            'exit',
        ].join('\n'),
        target_ops: 2,
    })

    // ── SQRT (square root) ─────────────────────────────────────────────
    defs.push({
        name: 'sqrt',
        category: 'SQRT',
        description: 'f32.sqrt × 4',
        source: [
            'f32.const 144.0', 'f32.sqrt',
            'f32.const 256.0', 'f32.sqrt',
            'f32.const 625.0', 'f32.sqrt',
            'f32.const 10000.0', 'f32.sqrt',
            'f32.drop', 'f32.drop', 'f32.drop', 'f32.drop',
            'exit',
        ].join('\n'),
        target_ops: 4,
    })

    // ── TRIG (sin/cos) ─────────────────────────────────────────────────
    defs.push({
        name: 'trig',
        category: 'TRIG',
        description: 'f32.sin × 2 + f32.cos × 2',
        source: [
            'f32.const 1.0', 'f32.sin',
            'f32.const 2.0', 'f32.cos',
            'f32.const 0.5', 'f32.sin',
            'f32.const 3.0', 'f32.cos',
            'f32.drop', 'f32.drop', 'f32.drop', 'f32.drop',
            'exit',
        ].join('\n'),
        target_ops: 4,
    })

    // ── CVT (type conversion) ──────────────────────────────────────────
    defs.push({
        name: 'cvt',
        category: 'CVT',
        description: 'cvt u8→u32, u32→f32, f32→u8, u8→f64 × 1 each',
        source: [
            'u8.const 42',
            'cvt u8 u32',    // u8 → u32
            'cvt u32 f32',   // u32 → f32
            'cvt f32 u8',    // f32 → u8
            'cvt u8 f64',    // u8 → f64
            'f64.drop',
            'exit',
        ].join('\n'),
        target_ops: 4,
    })

    // ── STR_OP (string operations) ─────────────────────────────────────
    // Uses str.init + str.len + str.char + str.clear on a memory region
    defs.push({
        name: 'str_op',
        category: 'STR_OP',
        description: 'str.init + str.len + str.char + str.clear',
        source: [
            'u16.const 32',       // Capacity for str.init
            'str.init 192',       // Init str8 at addr 192
            'u8.const 65',        // 'A'
            'str.char 192',       // Append char
            'str.len 192',        // Get length → push u16
            'u16.drop',
            'str.clear 192',      // Clear
            'exit',
        ].join('\n'),
        target_ops: 4,
    })

    // ── FFI (foreign function interface) ────────────────────────────────
    // The firmware registers F_add, F_mul, F_lerp, F_in_range.
    // We measure the full call + marshalling + return overhead.
    // Memory layout: M(192+), i32 = 4 bytes, f32 = 4 bytes
    defs.push({
        name: 'ffi',
        category: 'FFI',
        description: 'ffi F_add × 2 + ffi F_mul × 2 (i32,i32→i32)',
        source: [
            // Set up params: M+0=10, M+4=20 (i32 values)
            'i32.const 10', 'i32.move_to 192',
            'i32.const 20', 'i32.move_to 196',
            // Call F_add(M+192, M+196) → result at M+200
            'ffi F_add 192 196 200',
            'ffi F_add 192 196 200',
            // Call F_mul(M+192, M+196) → result at M+200
            'ffi F_mul 192 196 200',
            'ffi F_mul 192 196 200',
            'exit',
        ].join('\n'),
        target_ops: 4,
    })

    // ── EXIT ───────────────────────────────────────────────────────────
    // EXIT is inherently measured by the baseline, but we isolate it
    // more precisely with a program that has extra instructions before EXIT.
    defs.push({
        name: 'exit_cost',
        category: 'EXIT',
        description: 'nop + exit (EXIT cost = total - nop overhead)',
        source: [
            'nop',
            'nop',
            'exit',
        ].join('\n'),
        target_ops: 1,  // 1 EXIT, nops are ~0 cost
    })

    // ── Compile all programs ───────────────────────────────────────────
    const programs = []
    console.log(`\n${BOLD}Compiling ${defs.length} calibration programs via PLCASM...${RESET}`)
    for (const def of defs) {
        try {
            const bytecode = compilePLCASM(def.source)
            programs.push({
                name: def.name,
                category: def.category,
                description: def.description,
                bytecode,
                target_ops: def.target_ops,
            })
            console.log(`  ${GREEN}✓${RESET} ${def.name.padEnd(14)} ${String(bytecode.length).padStart(3)} bytes  ${DIM}${def.description}${RESET}`)
        } catch (err) {
            console.error(`  ${RED}✗${RESET} ${def.name.padEnd(14)} ${RED}FAIL: ${err.message}${RESET}`)
            console.error(`${DIM}Source:\n${def.source}${RESET}`)
            process.exit(1)
        }
    }
    console.log(`${GREEN}All ${programs.length} programs compiled successfully${RESET}`)

    return programs
}

// ============================================================================
// Calibration execution
// ============================================================================

const NUM_WARMUP_MS = 100         // Let program run this long before resetting health
const SETTLE_TIME_MS = 50         // Wait between test cases for serial to flush

async function runCalibration(client, deviceInfo, portPath) {
    const programs = generateCalibrationPrograms()
    const results = []
    // Decode device feature flags from PI response
    const deviceFlagsRaw = parseInt(deviceInfo?.flags || '0', 16) || 0
    // Compute measurement time from requested iterations:
    // Assuming ~1-50µs per cycle, N iterations take roughly N*50µs in worst case.
    // We use a generous delay to ensure enough cycles execute.
    const measureMs = Math.max(500, measureIterations)

    console.log(`\n${BOLD}Running calibration (${programs.length} test programs, ~${measureMs}ms measurement window each)${RESET}\n`)

    for (let i = 0; i < programs.length; i++) {
        const prog = programs[i]
        process.stdout.write(`  [${String(i + 1).padStart(2)}/${programs.length}] ${prog.name.padEnd(20)} `)

        // Skip tests that require features the device doesn't support
        if (prog.requires_flags && (deviceFlagsRaw & prog.requires_flags) !== prog.requires_flags) {
            const missing = []
            for (const [name, bit] of Object.entries(RUNTIME_FLAGS)) {
                if ((prog.requires_flags & bit) && !(deviceFlagsRaw & bit)) missing.push(name)
            }
            console.log(`${YELLOW}SKIP (device lacks: ${missing.join(', ')})${RESET}`)
            results.push({ ...prog, skipped: true, skip_reason: `Missing flags: ${missing.join(', ')}` })
            continue
        }

        // Check if device is still connected — if not, try to reconnect
        if (!client.isConnected) {
            process.stdout.write(`${YELLOW}disconnected (port.isOpen=${client.port?.isOpen}, port=${!!client.port}) — reconnecting...${RESET} `)
            try {
                await client.close()
            } catch (_) { /* ignore */ }
            await delay(2000) // Wait for device to recover (watchdog/power cycle)
            try {
                client = new PLCSerialClient(portPath, baudRate)
                await client.connect()
                await client.resetHealth()
                process.stdout.write(`${GREEN}OK${RESET} `)
            } catch (_) {
                console.log(`${RED}FAIL (reconnect failed — device may need power cycle)${RESET}`)
                results.push({ ...prog, error: 'Device disconnected' })
                continue
            }
        }

        // Stop any previously running program and let things settle
        await client.stopProgram()
        await delay(50)
        client.drain()

        // Upload calibration program with retry + PLC reset on failure
        let uploaded = false
        for (let attempt = 0; attempt < 3; attempt++) {
            if (!client.isConnected) break
            uploaded = await client.downloadProgram(prog.bytecode)
            if (uploaded) break
            process.stdout.write(`${YELLOW}retry${RESET} `)
            await client.plcReset()
            await delay(200)
            client.drain()
        }
        if (!uploaded) {
            console.log(`${RED}FAIL (upload — device may have crashed)${RESET}`)
            results.push({ ...prog, error: 'Upload failed' })
            // Try to recover for the next test
            try { await client.plcReset() } catch (_) { /* ignore */ }
            await delay(200)
            client.drain()
            continue
        }
        await delay(50)

        // Start program execution (sets is_running = true)
        const started = await client.startProgram()
        if (!started) {
            console.log(`${RED}FAIL (start)${RESET}`)
            results.push({ ...prog, error: 'Start failed' })
            continue
        }

        // Warm-up phase: let the program run to fill caches, stabilize timing
        await delay(NUM_WARMUP_MS)

        // Reset health counters to start fresh measurement.
        // The min value must reflect a proper steady-state cycle, not a
        // startup transient, so we wait a short period after resetting
        // to let at least a few full cycles execute before the
        // measurement window begins.
        const healthReset = await client.resetHealth()
        if (!healthReset) {
            console.log(`${YELLOW}WARN (health reset)${RESET}`)
        }
        await delay(50)  // Let a few proper cycles run so min is valid

        // Measurement phase: let program run for many cycles
        await delay(measureMs)

        // Read health stats (min/max/last cycle times)
        const health = await client.getHealth()

        // Stop program
        await client.stopProgram()
        await delay(SETTLE_TIME_MS)

        if (!health) {
            console.log(`${RED}FAIL (health read)${RESET}`)
            results.push({ ...prog, error: 'Health read failed' })
            continue
        }

        const last_us = health.last_cycle_us
        const min_us = health.min_cycle_us
        const max_us = health.max_cycle_us

        // Convert to nanoseconds
        const min_ns = min_us * 1000
        const max_ns = max_us * 1000
        const avg_ns = last_us * 1000

        results.push({
            ...prog,
            min_ns,
            max_ns,
            avg_ns,
            min_us,
            max_us,
            last_us,
        })

        console.log(`${GREEN}OK${RESET}  min=${min_us}µs  max=${max_us}µs  last=${last_us}µs`)
    }

    return results
}

function delay(ms) {
    return new Promise(resolve => setTimeout(resolve, ms))
}

// ============================================================================
// Profile computation from raw results
// ============================================================================

function computeProfile(results, deviceInfo, envName) {
    // Find baseline (dispatch overhead)
    const baseline = results.find(r => r.name === 'baseline')
    if (!baseline || baseline.error) {
        console.error(`${RED}Baseline measurement failed — cannot compute profile${RESET}`)
        return null
    }

    const dispatch_ns = baseline.min_ns
    console.log(`\n${CYAN}Baseline (dispatch + EXIT): ${dispatch_ns}ns${RESET}`)

    /**
     * Estimate a missing category from measured ones.
     * Used when a test crashed the device (e.g. 64-bit ops on some MCUs).
     */
    function estimateCategory(cat, measured, dispatch) {
        // 64-bit push/arithmetic — estimate from 32-bit counterpart × 2
        if (cat === 'PUSH_U64') return Math.round((measured['PUSH_U32'] || dispatch) * 2)
        if (cat === 'PUSH_F64') return Math.round((measured['PUSH_F32'] || dispatch) * 2.2)
        if (cat === 'ADD_F64') return Math.round((measured['ADD_F32'] || dispatch) * 2.5)
        if (cat === 'MUL_F64') return Math.round((measured['MUL_F32'] || dispatch) * 2.5)
        if (cat === 'DIV_F64') return Math.round((measured['DIV_F32'] || dispatch) * 2.5)
        return null
    }

    // Compute per-category costs by subtracting baseline and dividing by target_ops
    const categories = {}
    for (const r of results) {
        if (r.error || r.skipped) continue
        if (r.name === 'baseline') {
            categories['DISPATCH'] = dispatch_ns
            continue
        }

        const total_ns = r.min_ns
        // Subtract baseline for the EXIT instruction that's in every program
        const adjusted_ns = Math.max(0, total_ns - dispatch_ns)
        const per_op_ns = r.target_ops ? Math.round(adjusted_ns / r.target_ops) : adjusted_ns

        categories[r.category] = per_op_ns
    }

    // Map to the C++ category array order
    const categoryOrder = [
        'DISPATCH', 'PUSH_U8', 'PUSH_U16', 'PUSH_U32', 'PUSH_F32', 'PUSH_U64', 'PUSH_F64',
        'ADD_U8', 'ADD_U16', 'ADD_U32', 'ADD_F32', 'ADD_F64',
        'MUL_U8', 'MUL_U32', 'MUL_F32', 'MUL_F64',
        'DIV_U8', 'DIV_U32', 'DIV_F32', 'DIV_F64',
        'CMP', 'LOGIC', 'BITWISE', 'JMP', 'JMP_COND', 'CALL', 'RET',
        'LOAD', 'STORE', 'STACK_OP', 'BIT_RW', 'TIMER', 'COUNTER',
        'SQRT', 'TRIG', 'CVT', 'STR_OP', 'FFI', 'EXIT',
    ]

    const ns_array = categoryOrder.map(cat => {
        if (categories[cat] !== undefined) return categories[cat]
        // Fallback estimation for categories that crashed the device (e.g. 64-bit on some MCUs)
        const est = estimateCategory(cat, categories, dispatch_ns)
        if (est !== null) {
            console.warn(`  ${YELLOW}⚠ ${cat}: no measurement — estimated ${est}ns${RESET}`)
            return est
        }
        console.warn(`  ${YELLOW}⚠ ${cat}: no measurement — defaulting to 0${RESET}`)
        return 0
    })

    // Detect architecture from device info
    const arch = deviceInfo?.arch || 'unknown'
    const archLower = arch.toLowerCase()
    let arch_name = archLower
    let arch_comment = envName || 'unknown target'

    return {
        env_name: envName || '',
        name: envName || `${arch} device`,
        arch_name,
        arch: arch,
        clock_mhz: 0,  // Will be filled from env name parsing
        capabilities: 0,
        ns_per_category: ns_array,
        categories: Object.fromEntries(categoryOrder.map((cat, i) => [cat, ns_array[i]])),
        raw_results: results.filter(r => !r.error).map(r => ({
            name: r.name, category: r.category,
            min_us: r.min_us, max_us: r.max_us, last_us: r.last_us,
        })),
    }
}

// ============================================================================
// Environment-to-profile metadata mapping
// ============================================================================

// Maps PIO env names to their wcet-targets.h profile metadata.
// This defines the canonical name, architecture, clock, and capabilities for each target.
const ENV_PROFILE_META = {
    avr_uno_16mhz:              { name: 'ATmega328P @16MHz (Arduino UNO/Nano)',  arch_name: 'avr',         arch_id: 'WCET_ARCH_AVR',         clock_mhz: 16,  caps: 'WCET_CAP_NONE',                                                                                            section: 'AVR 8-bit' },
    avr_nano_16mhz:             { name: 'ATmega328P @16MHz (Arduino UNO/Nano)',  arch_name: 'avr',         arch_id: 'WCET_ARCH_AVR',         clock_mhz: 16,  caps: 'WCET_CAP_NONE',                                                                                            section: 'AVR 8-bit' },
    stm32f103_bluepill_72mhz:   { name: 'STM32F103 BluePill @72MHz',             arch_name: 'cortex-m3',   arch_id: 'WCET_ARCH_CORTEX_M3',   clock_mhz: 72,  caps: 'WCET_CAP_HW_DIV | WCET_CAP_HW_MUL32',                                                                      section: 'Cortex-M3' },
    stm32f401_blackpill_84mhz:  { name: 'STM32F401 BlackPill @84MHz',            arch_name: 'cortex-m4f',  arch_id: 'WCET_ARCH_CORTEX_M4F',  clock_mhz: 84,  caps: 'WCET_CAP_HW_FPU_SP | WCET_CAP_HW_DIV | WCET_CAP_HW_MUL32',                                                section: 'Cortex-M4F (reference target)' },
    stm32f411_blackpill_100mhz: { name: 'STM32F411 BlackPill @100MHz',           arch_name: 'cortex-m4f',  arch_id: 'WCET_ARCH_CORTEX_M4F',  clock_mhz: 100, caps: 'WCET_CAP_HW_FPU_SP | WCET_CAP_HW_DIV | WCET_CAP_HW_MUL32',                                                section: 'Cortex-M4F (faster)' },
    stm32h743_devebox_480mhz:   { name: 'STM32H743 @480MHz',                     arch_name: 'cortex-m7',   arch_id: 'WCET_ARCH_CORTEX_M7',   clock_mhz: 480, caps: 'WCET_CAP_HW_FPU_SP | WCET_CAP_HW_FPU_DP | WCET_CAP_HW_DIV | WCET_CAP_HW_MUL32 | WCET_CAP_CACHE | WCET_CAP_DSP', section: 'Cortex-M7' },
    stm32g431_blackpill_170mhz: { name: 'STM32G431 BlackPill @170MHz',           arch_name: 'cortex-m4f',  arch_id: 'WCET_ARCH_CORTEX_M4F',  clock_mhz: 170, caps: 'WCET_CAP_HW_FPU_SP | WCET_CAP_HW_DIV | WCET_CAP_HW_MUL32',                                                section: 'Cortex-M4F (G4)' },
    esp32_wemos_240mhz:         { name: 'ESP32 @240MHz',                          arch_name: 'xtensa-lx6',  arch_id: 'WCET_ARCH_XTENSA_LX6',  clock_mhz: 240, caps: 'WCET_CAP_HW_MUL32',                                                                                       section: 'ESP32 (Xtensa LX6)' },
    esp32c3_supermini_160mhz:   { name: 'ESP32-C3 @160MHz',                       arch_name: 'riscv32-imc', arch_id: 'WCET_ARCH_RISCV32',     clock_mhz: 160, caps: 'WCET_CAP_HW_MUL32',                                                                                       section: 'ESP32-C3 (RISC-V)' },
    esp32s3_zero_240mhz:        { name: 'ESP32-S3 @240MHz',                       arch_name: 'xtensa-lx7',  arch_id: 'WCET_ARCH_XTENSA_LX7',  clock_mhz: 240, caps: 'WCET_CAP_HW_FPU_SP | WCET_CAP_HW_MUL32',                                                                  section: 'ESP32-S3 (Xtensa LX7)' },
    esp8266_d1mini_80mhz:       { name: 'ESP8266 @80MHz',                         arch_name: 'xtensa-lx106',arch_id: 'WCET_ARCH_XTENSA_LX106',clock_mhz: 80,  caps: 'WCET_CAP_NONE',                                                                                            section: 'ESP8266 (Xtensa LX106)' },
    rp2040_pico_133mhz:         { name: 'RP2040 Pico @133MHz',                    arch_name: 'cortex-m0+',  arch_id: 'WCET_ARCH_CORTEX_M0P',  clock_mhz: 133, caps: 'WCET_CAP_NONE',                                                                                            section: 'RP2040 (Cortex-M0+)' },
    rp2350_pico2_150mhz:        { name: 'RP2350 Pico2 @150MHz',                   arch_name: 'cortex-m33',  arch_id: 'WCET_ARCH_CORTEX_M33',  clock_mhz: 150, caps: 'WCET_CAP_HW_FPU_SP | WCET_CAP_HW_DIV | WCET_CAP_HW_MUL32 | WCET_CAP_DSP',                                 section: 'RP2350 (Cortex-M33)' },
    ra4m1_uno_r4_48mhz:         { name: 'RA4M1 Arduino UNO R4 @48MHz',            arch_name: 'cortex-m4f',  arch_id: 'WCET_ARCH_CORTEX_M4F',  clock_mhz: 48,  caps: 'WCET_CAP_HW_FPU_SP | WCET_CAP_HW_DIV | WCET_CAP_HW_MUL32',                                                section: 'RA4M1 (Cortex-M4F)' },
    nrf52840_nicenano_64mhz:    { name: 'nRF52840 @64MHz',                        arch_name: 'cortex-m4f',  arch_id: 'WCET_ARCH_CORTEX_M4F',  clock_mhz: 64,  caps: 'WCET_CAP_HW_FPU_SP | WCET_CAP_HW_DIV | WCET_CAP_HW_MUL32',                                                section: 'nRF52840 (Cortex-M4F)' },
}

// ============================================================================
// Output formatters
// ============================================================================

function generateCppEntry(profile) {
    const ns = profile.ns_per_category
    const pad = (n) => String(n).padStart(5)
    const meta = ENV_PROFILE_META[profile.env_name]
    const name = meta ? meta.name : profile.name
    const arch_name = meta ? meta.arch_name : profile.arch_name
    const arch_id = meta ? meta.arch_id : 'WCET_ARCH_UNKNOWN'
    const clock_mhz = meta ? meta.clock_mhz : (profile.clock_mhz || 0)
    const caps = meta ? meta.caps : 'WCET_CAP_NONE'
    const section = meta ? meta.section : profile.name

    const lines = []
    lines.push(`    // @WCET_PROFILE@ ${name}`)
    lines.push(`    // ${section}`)
    lines.push(`    {`)
    lines.push(`        "${name}",`)
    lines.push(`        "${arch_name}", ${arch_id}, ${clock_mhz},`)
    lines.push(`        ${caps},`)
    lines.push(`        {`)
    lines.push(`            // dispatch, pu8, pu16, pu32, pf32, pu64, pf64,`)
    lines.push(`            ${pad(ns[0])}, ${pad(ns[1])}, ${pad(ns[2])}, ${pad(ns[3])}, ${pad(ns[4])}, ${pad(ns[5])}, ${pad(ns[6])},`)
    lines.push(`            // add8, add16, add32, addf32, addf64,`)
    lines.push(`            ${pad(ns[7])}, ${pad(ns[8])}, ${pad(ns[9])}, ${pad(ns[10])}, ${pad(ns[11])},`)
    lines.push(`            // mul8, mul32, mulf32, mulf64,`)
    lines.push(`            ${pad(ns[12])}, ${pad(ns[13])}, ${pad(ns[14])}, ${pad(ns[15])},`)
    lines.push(`            // div8, div32, divf32, divf64,`)
    lines.push(`            ${pad(ns[16])}, ${pad(ns[17])}, ${pad(ns[18])}, ${pad(ns[19])},`)
    lines.push(`            // cmp, logic, bitwise, jmp, jmp_cond, call, ret,`)
    lines.push(`            ${pad(ns[20])}, ${pad(ns[21])}, ${pad(ns[22])}, ${pad(ns[23])}, ${pad(ns[24])}, ${pad(ns[25])}, ${pad(ns[26])},`)
    lines.push(`            // load, store, stack_op, bit_rw, timer, counter,`)
    lines.push(`            ${pad(ns[27])}, ${pad(ns[28])}, ${pad(ns[29])}, ${pad(ns[30])}, ${pad(ns[31])}, ${pad(ns[32])},`)
    lines.push(`            // sqrt, trig, cvt, str_op, ffi, exit`)
    lines.push(`            ${pad(ns[33])}, ${pad(ns[34])}, ${pad(ns[35])}, ${pad(ns[36])}, ${pad(ns[37])}, ${pad(ns[38])}`)
    lines.push(`        }`)
    lines.push(`    },`)
    lines.push(`    // @WCET_PROFILE_END@`)
    return lines.join('\n')
}

function outputCppProfile(profile) {
    console.log(`\n${BOLD}═══ C++ Target Profile (paste into wcet-targets.h) ═══${RESET}\n`)
    console.log(generateCppEntry(profile))
    if (doJson) {
        console.log(`\n${BOLD}═══ JSON Profile ═══${RESET}\n`)
        console.log(JSON.stringify(profile, null, 2))
    }
}

// ============================================================================
// Patch wcet-targets.h with calibration results
// ============================================================================

function patchWcetTargets(profile) {
    const wcetPath = path.resolve(__dirname, '../../src/tools/assembly/wcet-targets.h')
    if (!fs.existsSync(wcetPath)) {
        console.error(`${RED}wcet-targets.h not found at: ${wcetPath}${RESET}`)
        return false
    }

    const meta = ENV_PROFILE_META[profile.env_name]
    const targetName = meta ? meta.name : profile.name

    const originalContent = fs.readFileSync(wcetPath, 'utf-8')
    let content = originalContent
    const newEntry = generateCppEntry(profile)

    // Find existing entry by marker comment: `// @WCET_PROFILE@ <name>`
    const startMarker = `// @WCET_PROFILE@ ${targetName}`
    const endMarker = `// @WCET_PROFILE_END@`

    let action = ''
    let oldBlock = null
    const startIdx = content.indexOf(startMarker)
    if (startIdx !== -1) {
        // Find the matching end marker after the start
        const endIdx = content.indexOf(endMarker, startIdx)
        if (endIdx !== -1) {
            const blockEnd = endIdx + endMarker.length
            oldBlock = content.substring(startIdx, blockEnd)
            // Replace: find the start of the line containing the start marker
            const lineStart = content.lastIndexOf('\n', startIdx) + 1
            // Find end of line containing end marker
            const lineEnd = content.indexOf('\n', blockEnd)
            const replaceEnd = lineEnd !== -1 ? lineEnd : content.length
            content = content.substring(0, lineStart) + newEntry + content.substring(replaceEnd)
            action = 'replaced'
        }
    }

    if (!action) {
        // Entry not found — insert before the sentinel/closing `};` of the array
        const arrayMarker = 'g_wcet_target_profiles[] = {'
        const arrayStart = content.indexOf(arrayMarker)
        if (arrayStart === -1) {
            console.error(`${RED}Could not find g_wcet_target_profiles array in wcet-targets.h${RESET}`)
            return false
        }
        const arrayEnd = content.indexOf('};', arrayStart + arrayMarker.length)
        if (arrayEnd === -1) {
            console.error(`${RED}Could not find end of g_wcet_target_profiles array${RESET}`)
            return false
        }

        // Insert new entry right after the opening brace line
        const openBraceEnd = content.indexOf('\n', arrayStart) + 1
        content = content.substring(0, openBraceEnd) + '\n' + newEntry + '\n' + content.substring(openBraceEnd)
        action = 'added'
    }

    // Show measurement diff
    const categoryNames = [
        'DISPATCH', 'PUSH_U8', 'PUSH_U16', 'PUSH_U32', 'PUSH_F32', 'PUSH_U64', 'PUSH_F64',
        'ADD_U8', 'ADD_U16', 'ADD_U32', 'ADD_F32', 'ADD_F64',
        'MUL_U8', 'MUL_U32', 'MUL_F32', 'MUL_F64',
        'DIV_U8', 'DIV_U32', 'DIV_F32', 'DIV_F64',
        'CMP', 'LOGIC', 'BITWISE', 'JMP', 'JMP_COND', 'CALL', 'RET',
        'LOAD', 'STORE', 'STACK_OP', 'BIT_RW', 'TIMER', 'COUNTER',
        'SQRT', 'TRIG', 'CVT', 'STR_OP', 'FFI', 'EXIT',
    ]

    // Extract ns_per_category values from an existing C++ entry block
    function parseEntryValues(block) {
        if (!block) return null
        // The ns array is the inner `{ ... }` nested inside the struct `{ ... },`
        // Find the line with `// dispatch` which starts the ns array data
        const dispatchIdx = block.indexOf('// dispatch')
        if (dispatchIdx === -1) return null
        // Extract everything from that point to the closing `}` of the inner array
        const afterDispatch = block.substring(dispatchIdx)
        const closeBrace = afterDispatch.indexOf('}')
        if (closeBrace === -1) return null
        const numBlock = afterDispatch.substring(0, closeBrace)
        const nums = numBlock.replace(/\/\/[^\n]*/g, '').match(/\d+/g)
        return nums ? nums.map(Number) : null
    }

    const oldValues = oldBlock ? parseEntryValues(oldBlock) : null
    const newValues = profile.ns_per_category

    console.log(`\n${BOLD}═══ wcet-targets.h: ${targetName} ═══${RESET}\n`)

    if (action === 'added') {
        console.log(`  ${GREEN}New entry (no previous values)${RESET}\n`)
        for (let i = 0; i < categoryNames.length; i++) {
            const name = categoryNames[i].padEnd(12)
            console.log(`  ${GREEN}${name} ${String(newValues[i]).padStart(6)}ns${RESET}`)
        }
    } else {
        // Show per-category comparison
        let anyDiff = false
        for (let i = 0; i < categoryNames.length; i++) {
            const name = categoryNames[i].padEnd(12)
            const ov = oldValues ? oldValues[i] : null
            const nv = newValues[i]
            if (ov === null || ov === undefined) {
                console.log(`  ${GREEN}${name}        → ${String(nv).padStart(6)}ns  (new)${RESET}`)
                anyDiff = true
            } else if (ov === nv) {
                console.log(`  ${DIM}${name} ${String(nv).padStart(6)}ns  (unchanged)${RESET}`)
            } else {
                const delta = nv - ov
                const pct = ov !== 0 ? ((delta / ov) * 100).toFixed(1) : '∞'
                const sign = delta > 0 ? '+' : ''
                const color = delta > 0 ? YELLOW : GREEN
                console.log(`  ${color}${name} ${String(ov).padStart(6)} → ${String(nv).padStart(6)}ns  (${sign}${delta}ns, ${sign}${pct}%)${RESET}`)
                anyDiff = true
            }
        }
        if (!anyDiff) {
            console.log(`\n${DIM}  All values unchanged${RESET}`)
        }
    }

    fs.writeFileSync(wcetPath, content)
    const verb = action === 'replaced' ? 'Patched existing' : 'Added new'
    console.log(`\n${GREEN}${verb} entry "${targetName}" in wcet-targets.h${RESET}`)
    return true
}

// ============================================================================
// PlatformIO build & upload
// ============================================================================

function findPioExecutable() {
    const home = process.env.HOME || process.env.USERPROFILE
    const paths = process.platform === 'win32' ? [
        path.join(home, '.platformio', 'penv', 'Scripts', 'platformio.exe'),
        path.join(home, '.platformio', 'penv', 'Scripts', 'pio.exe'),
    ] : [
        path.join(home, '.platformio', 'penv', 'bin', 'platformio'),
        path.join(home, '.platformio', 'penv', 'bin', 'pio'),
    ]

    for (const p of paths) {
        if (fs.existsSync(p)) return p
    }
    return 'platformio'
}

function pioUpload(envName) {
    const pio = findPioExecutable()
    console.log(`\n${CYAN}Building & uploading calibration firmware for ${envName}...${RESET}`)
    const result = spawnSync(`"${pio}"`, ['run', '-e', envName, '-t', 'upload'], {
        cwd: platformTesterDir, stdio: 'inherit', shell: true
    })
    if (result.status !== 0) {
        console.error(`${RED}Build/upload failed${RESET}`)
        process.exit(1)
    }
}

// ============================================================================
// Main
// ============================================================================

async function main() {
    console.log(`\n${BOLD}╔══════════════════════════════════════════════════╗${RESET}`)
    console.log(`${BOLD}║       VovkPLC WCET Calibration Runner            ║${RESET}`)
    console.log(`${BOLD}╚══════════════════════════════════════════════════╝${RESET}`)

    // Step 0: Initialize PLCASM compiler (WASM module)
    console.log(`\n${CYAN}Initializing PLCASM compiler...${RESET}`)
    await initCompiler()
    console.log(`${GREEN}Compiler ready${RESET}`)

    // Step 1: Select PlatformIO environment
    const environments = parseEnvironments()
    const env = await selectEnvironment(environments)
    console.log(`\n${GREEN}Selected: ${env.name} (${env.platform} / ${env.board})${RESET}`)

    // Step 2: Build & upload (only when --upload is passed)
    if (doUpload) {
        pioUpload(env.name)
        console.log(`${GREEN}Upload complete. Waiting for device to reboot...${RESET}`)
        await delay(5000) // USB CDC devices need time to re-enumerate after DFU upload
    }

    // Step 3: Select serial port
    const portPath = await selectSerialPort()
    console.log(`\n${CYAN}Connecting to ${portPath} @ ${baudRate} baud...${RESET}`)

    // Step 4: Connect
    const client = new PLCSerialClient(portPath, baudRate)
    try {
        await client.connect()
    } catch (err) {
        console.error(`${RED}Connection failed: ${err.message}${RESET}`)
        process.exit(1)
    }

    // Ping with retries (device may need time after boot / sends startup banner)
    let alive = false
    // First drain any startup banner the device auto-sends
    await delay(1000)
    client.drain()
    for (let attempt = 0; attempt < 5; attempt++) {
        alive = await client.ping()
        if (alive) break
        process.stdout.write(`${DIM}.${RESET}`)
        await delay(1000)
    }
    if (!alive) {
        console.error(`\n${RED}Device not responding to ping${RESET}`)
        await client.close()
        process.exit(1)
    }
    console.log(`${GREEN}Device connected!${RESET}`)

    // Step 5: Get device info
    const deviceInfo = await client.getDeviceInfo()
    if (deviceInfo) {
        const decodedFlags = decodeRuntimeFlags(deviceInfo.flags)
        console.log(`\n${BOLD}Device Info:${RESET}`)
        console.log(`  Runtime:    ${deviceInfo.runtime}`)
        console.log(`  Arch:       ${deviceInfo.arch}`)
        console.log(`  Version:    ${deviceInfo.version} (build ${deviceInfo.build})`)
        console.log(`  Memory:     ${deviceInfo.memory_size} bytes`)
        console.log(`  Program:    ${deviceInfo.program_size} bytes`)
        console.log(`  Stack:      ${deviceInfo.stack_size} bytes`)
        console.log(`  Markers:    offset=${deviceInfo.marker_offset} count=${deviceInfo.marker_count}`)
        console.log(`  Flags:      0x${deviceInfo.flags} → ${Object.entries(decodedFlags).filter(([k, v]) => k !== 'raw' && v).map(([k]) => k).join(', ') || 'none'}`)
        console.log(`  Device:     ${deviceInfo.device_name}`)
    } else {
        console.log(`${YELLOW}Could not read device info (PI) — continuing anyway${RESET}`)
    }

    // Step 6: Initial health reset
    const rhOk = await client.resetHealth()
    if (rhOk) {
        console.log(`${DIM}Health counters reset${RESET}`)
    }

    // Step 7: Run calibration
    const results = await runCalibration(client, deviceInfo, portPath)

    // Step 8: Stop any remaining program
    await client.stopProgram()

    // Step 9: Compute profile
    const profile = computeProfile(results, deviceInfo, env.name)

    // Step 10: Output
    if (profile) {
        outputCppProfile(profile)

        // Save JSON to file (only with --json)
        if (doJson) {
            const outDir = path.resolve(__dirname, 'calibration/results')
            if (!fs.existsSync(outDir)) fs.mkdirSync(outDir, { recursive: true })
            const outPath = path.join(outDir, `${env.name}_profile.json`)
            fs.writeFileSync(outPath, JSON.stringify(profile, null, 2))
            console.log(`\n${GREEN}Profile saved to: ${outPath}${RESET}`)
        }

        // Patch wcet-targets.h if requested
        if (doPatch) {
            patchWcetTargets(profile)
        }
    }

    // Cleanup
    await client.close()
    console.log(`\n${GREEN}Calibration complete.${RESET}\n`)
}

main().catch(err => {
    console.error(`${RED}Fatal error: ${err.message || err}${RESET}`)
    process.exit(1)
})
