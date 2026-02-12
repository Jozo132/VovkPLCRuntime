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
const selectedPort = getArg('--port')
const selectedEnv = getArg('--env')
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

    // ── Push u8 ────────────────────────────────────────────────────────
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

    // ── Push u32 ───────────────────────────────────────────────────────
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

    // ── Push f32 ───────────────────────────────────────────────────────
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

    // ── ADD u8 ─────────────────────────────────────────────────────────
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

    // ── ADD u32 ────────────────────────────────────────────────────────
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

    // ── ADD f32 ────────────────────────────────────────────────────────
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

    // ── MUL u32 ────────────────────────────────────────────────────────
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

    // ── MUL f32 ────────────────────────────────────────────────────────
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

    // ── DIV u32 ────────────────────────────────────────────────────────
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

    // ── DIV f32 ────────────────────────────────────────────────────────
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

    // ── CMP (compare u8) ──────────────────────────────────────────────
    defs.push({
        name: 'cmp',
        category: 'CMP',
        description: 'u8.const × 2 + u8.cmp_eq/gt/lt × 4, then drop',
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
        description: 'u8.const + u8.and/u8.or × 4, then drop',
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
        description: 'u8.const + bw.and.x8/bw.or.x8 × 4, then drop',
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

    // ── JMP (unconditional forward jump) ───────────────────────────────
    defs.push({
        name: 'jmp',
        category: 'JMP',
        description: 'jmp forward to exit, skipping NOPs',
        source: [
            'jmp end',
            'nop',
            'nop',
            'end:',
            'exit',
        ].join('\n'),
        target_ops: 1,
    })

    // ── LOAD/STORE memory ──────────────────────────────────────────────
    defs.push({
        name: 'load_store',
        category: 'LOAD',
        description: 'u8.load_from + u8.move_to × 2',
        source: [
            'u8.load_from 192',   // Load from marker area
            'u8.move_to 193',     // Store to adjacent
            'u8.load_from 193',
            'u8.move_to 194',
            'exit',
        ].join('\n'),
        target_ops: 4,
    })

    // ── STACK ops (copy + drop) ────────────────────────────────────────
    // Note: .swap is not available in PLCASM compiler — using copy + drop
    defs.push({
        name: 'stack_ops',
        category: 'STACK_OP',
        description: 'u8.const + u8.copy + u8.drop × 3 rounds',
        source: [
            'u8.const 42',
            'u8.copy',       // DUP: stack = [42, 42]
            'u8.drop',       // DROP: stack = [42]
            'u8.copy',
            'u8.drop',
            'u8.copy',
            'u8.drop',
            'u8.drop',       // Final cleanup
            'exit',
        ].join('\n'),
        target_ops: 6,  // 3 copies + 3 drops (the working ones, not the final cleanup)
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

const NUM_WARMUP_MS = 200         // Let program run this long before resetting health
const SETTLE_TIME_MS = 200        // Wait between test cases for serial to flush

async function runCalibration(client, deviceInfo) {
    const programs = generateCalibrationPrograms()
    const results = []
    // Compute measurement time from requested iterations:
    // Assuming ~1-50µs per cycle, N iterations take roughly N*50µs in worst case.
    // We use a generous delay to ensure enough cycles execute.
    const measureMs = Math.max(500, measureIterations)

    console.log(`\n${BOLD}Running calibration (${programs.length} test programs, ~${measureMs}ms measurement window each)${RESET}\n`)

    for (let i = 0; i < programs.length; i++) {
        const prog = programs[i]
        process.stdout.write(`  [${String(i + 1).padStart(2)}/${programs.length}] ${prog.name.padEnd(20)} `)

        // Stop any previously running program and let things settle
        await client.stopProgram()
        await delay(150)
        client.drain()

        // Upload calibration program with retry
        let uploaded = false
        for (let attempt = 0; attempt < 3; attempt++) {
            uploaded = await client.downloadProgram(prog.bytecode)
            if (uploaded) break
            process.stdout.write(`${YELLOW}retry${RESET} `)
            await delay(300)
            client.drain()
        }
        if (!uploaded) {
            console.log(`${RED}FAIL (upload)${RESET}`)
            results.push({ ...prog, error: 'Upload failed' })
            await delay(200)
            continue
        }
        await delay(100)

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

    // Compute per-category costs by subtracting baseline and dividing by target_ops
    const categories = {}
    for (const r of results) {
        if (r.error) continue
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
        // Estimate unmeasured categories from measured ones
        if (cat === 'PUSH_U16') return Math.round((categories['PUSH_U8'] || 0) * 1.3)
        if (cat === 'PUSH_U64') return Math.round((categories['PUSH_U32'] || 0) * 1.8)
        if (cat === 'PUSH_F64') return Math.round((categories['PUSH_F32'] || 0) * 2.2)
        if (cat === 'ADD_U16') return Math.round((categories['ADD_U8'] || 0) * 1.2)
        if (cat === 'ADD_F64') return Math.round((categories['ADD_F32'] || 0) * 2.5)
        if (cat === 'MUL_U8') return Math.round((categories['MUL_U32'] || 0) * 0.5)
        if (cat === 'MUL_F64') return Math.round((categories['MUL_F32'] || 0) * 2.5)
        if (cat === 'DIV_U8') return Math.round((categories['DIV_U32'] || 0) * 0.6)
        if (cat === 'DIV_F64') return Math.round((categories['DIV_F32'] || 0) * 2.5)
        if (cat === 'JMP_COND') return Math.round((categories['JMP'] || 0) * 1.5)
        if (cat === 'CALL') return Math.round((categories['JMP'] || 0) * 2)
        if (cat === 'RET') return Math.round((categories['JMP'] || 0) * 1.5)
        if (cat === 'STORE') return categories['LOAD'] || 0
        if (cat === 'BIT_RW') return Math.round((categories['LOAD'] || 0) * 0.8)
        if (cat === 'TIMER') return Math.round((categories['LOAD'] || 0) * 3)
        if (cat === 'COUNTER') return Math.round((categories['LOAD'] || 0) * 2.5)
        if (cat === 'SQRT') return Math.round((categories['MUL_F32'] || 0) * 3)
        if (cat === 'TRIG') return Math.round((categories['MUL_F32'] || 0) * 5)
        if (cat === 'CVT') return Math.round(dispatch_ns * 2)
        if (cat === 'STR_OP') return Math.round(dispatch_ns * 10)
        if (cat === 'FFI') return Math.round(dispatch_ns * 8)
        if (cat === 'EXIT') return Math.round(dispatch_ns * 0.5)
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
    console.log(`\n${BOLD}═══ JSON Profile ═══${RESET}\n`)
    console.log(JSON.stringify(profile, null, 2))
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
        console.log(`\n${BOLD}Device Info:${RESET}`)
        console.log(`  Runtime:    ${deviceInfo.runtime}`)
        console.log(`  Arch:       ${deviceInfo.arch}`)
        console.log(`  Version:    ${deviceInfo.version} (build ${deviceInfo.build})`)
        console.log(`  Memory:     ${deviceInfo.memory_size} bytes`)
        console.log(`  Program:    ${deviceInfo.program_size} bytes`)
        console.log(`  Stack:      ${deviceInfo.stack_size} bytes`)
        console.log(`  Markers:    offset=${deviceInfo.marker_offset} count=${deviceInfo.marker_count}`)
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
    const results = await runCalibration(client, deviceInfo)

    // Step 8: Stop any remaining program
    await client.stopProgram()

    // Step 9: Compute profile
    const profile = computeProfile(results, deviceInfo, env.name)

    // Step 10: Output
    if (profile) {
        outputCppProfile(profile)

        // Save JSON to file
        const outDir = path.resolve(__dirname, 'calibration/results')
        if (!fs.existsSync(outDir)) fs.mkdirSync(outDir, { recursive: true })
        const outPath = path.join(outDir, `${env.name}_profile.json`)
        fs.writeFileSync(outPath, JSON.stringify(profile, null, 2))
        console.log(`\n${GREEN}Profile saved to: ${outPath}${RESET}`)

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
