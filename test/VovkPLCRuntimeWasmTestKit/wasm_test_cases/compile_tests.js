// @ts-check
"use strict"

// Compile all '*.cpp' files in the current directory like above but using Node.JS and process execution instead of a batch file.
// The compilation will be executed in parallel with async await.

const fs = require('fs')
const path = require('path')
const { exec } = require('child_process')


/** @param { string } file_name */
const readFile = file_name => new Promise((resolve, reject) => {
    fs.readFile(file_name, (error, data) => {
        if (error) reject(error)
        else resolve(data)
    })
})

/** @param { string } file_name */
const getFileSize = file_name => new Promise((resolve, reject) => {
    fs.stat(file_name, (error, stats) => {
        if (error) reject(error)
        else resolve(stats.size)
    })
})

/** @param { string } command * @param { any } [options] */
const exec_async = (command, options) => new Promise((resolve, reject) => {
    exec(command, options, (error, stdout, stderr) => {
        if (error) reject(error)
        else resolve({ stdout, stderr })
    })
})

/** @param { string } input * @param { string } output */
const CC = (input, output) => `clang++ --target=wasm32 -Wall -std=c++11 -nostdlib -D __WASM__ -O3 -c "${input}" -o "${output}"`

/** @param { string } input * @param { string } output */
const LD = (input, output) => `wasm-ld --no-entry --export-dynamic --allow-undefined --lto-O3 "${input}" -o "${output}"`

/** @param { string } file_name */
const compile_and_build_wasm = async (file_name, padding = 15) => new Promise(async (resolve, reject) => {
    const input = `${file_name}.cpp`
    const output = `build/${file_name}.o`
    const wasm = `debug/${file_name}.wasm`
    const cc = CC(input, output)
    const ld = LD(output, wasm)

    try {
        let t = +new Date()
        const result = await exec_async(cc)
        const cc_t = +new Date() - t
        if (result.stderr) {
            const msg = `Compiling ${file_name}.o error: "${result.stderr}"`
            console.log(msg)
            reject(msg)
            return
        }
        t = +new Date()
        const result2 = await exec_async(ld)
        const ld_t = +new Date() - t
        if (result2.stderr) {
            const msg = `Building debug/${file_name}.wasm error: "${result2.stderr}"`
            console.log(msg)
            reject(msg)
            return
        }
        const success_msg = `Compiled ${file_name}.wasm`
        const fileSize = await getFileSize(wasm) * 0.001
        console.log(`${success_msg.padEnd(padding + 20, ' ')} - CC took ${`${cc_t} ms`.padEnd(10, ' ')}   LD took ${`${ld_t} ms`.padEnd(10, ' ')}  File size: ${fileSize.toFixed(1).padStart(6, ' ')} KB`)
        resolve(true)
    } catch (error) {
        reject(error)
    }
})


let message = ''
/** @param { number | number[] | string } charcode */
const console_print = charcode => {
    if (typeof charcode === 'string') {
        if (charcode.length === 0) return
        if (charcode.length === 1) return console_print(charcode.charCodeAt(0))
        return charcode.split('').forEach(console_print)
    }
    if (Array.isArray(charcode)) return charcode.forEach(console_print)
    const char = String.fromCharCode(charcode)
    if (char === '\0' || char === '\n') {
        console.log(message)
        message = ''
    } else message += char
}

/** @param { string } file_name * @returns { Promise<any> } */
const instantiate_wasm = async file_name => {
    const wasmImports = {
        env: {
            stdout: (charcode) => { /* console_print(charcode) */ }, // Skip logging to console
        }
    }
    const wasmBuffer = await readFile(file_name)
    const wasmModule = await WebAssembly.compile(wasmBuffer)
    const wasmInstance = await WebAssembly.instantiate(wasmModule, wasmImports)
    return wasmInstance
}

const main = async () => {
    const all_files = fs.readdirSync(__dirname)
    const file_names = all_files.filter(file => file.endsWith('.cpp')).map(file => file.slice(0, -4))
    if (!file_names.length) throw new Error('No "*.cpp" files found')
    const head_1 = `################ Compiling ${file_names.length} test source file${file_names.length === 1 ? '' : 's'} ################`
    console.log(head_1)
    const build_dir_exists = fs.existsSync(path.join(__dirname, 'build'))
    if (!build_dir_exists) {
        console.log('Creating "build" directory...')
        fs.mkdirSync(path.join(__dirname, 'build'))
    }
    const debug_dir_exists = fs.existsSync(path.join(__dirname, 'debug'))
    if (!debug_dir_exists) {
        console.log('Creating "debug" directory...')
        fs.mkdirSync(path.join(__dirname, 'debug'))
    }
    let longset_file_name_length = 0
    for (const file_name of file_names) {
        if (file_name.length > longset_file_name_length) longset_file_name_length = file_name.length
    }
    const t = +new Date()
    const promises = file_names.map(file_name => compile_and_build_wasm(file_name, longset_file_name_length))
    await Promise.all(promises)
    const compilation_time = +new Date() - t
    console.log(`Compilation finished in ${compilation_time} ms`)

    const bg_red = '\x1b[41m'
    const bg_yellow = '\x1b[43m'
    const bg_green = '\x1b[42m'
    const bg_gray = '\x1b[100m'
    const bg_blue = '\x1b[44m'
    const fg_white = '\x1b[37m'
    const c_reset = '\x1b[0m'

    const G = bg_gray
    const C = bg_blue + fg_white
    const R = c_reset

    console.log(`################ Testing WASM file${file_names.length === 1 ? '' : 's'} ################`.padEnd(head_1.length, '#'))
    console.log(`Explanation:`)
    console.log(`   info :  int function(void)  - a function to calculate memory increase since last info call - should not increase memory`)
    console.log(`   ${G}empty${R}:  void function(void) - a function that does nothing - should not increase memory`)
    console.log(`   ${C}job${R}  :  void function(void) - a function that does some kind of work - can increase memory (malloc) and/or free it (free),`)
    console.log(`                                     repeated calls can also reuse the memory that was set free`)
    console.log(` The table cell format is "${fg_white + bg_red}+diff${c_reset} (used/allocated)"`)
    console.log()

    const table_offset = longset_file_name_length + 2
    const col_width = 18
    const col_pad_number = x => {
        let isNumber = false
        if (Array.isArray(x)) {
            isNumber = true
            const diff = x[0]
            const used = x[1]
            const allocated = x[2]
            const alloc_change = x[3]
            const bg = diff > 0 ? bg_red : diff < 0 ? bg_green : alloc_change > 0 ? bg_yellow : ''
            const output = fg_white + bg + `${bg === bg_red ? '+' : bg === bg_yellow ? '~' : ''}${bg === bg_yellow ? alloc_change : diff}${c_reset} (${used}/${allocated})`
            x = output
        }
        let addiding_spaces = 0
        if (typeof x === 'string' && x.length > 0) {
            // Calculate how many spaces to add to the end of the string
            const chars = x.split('')
            let found = false
            for (let i = 0; i < chars.length; i++) {
                const char = chars[i]
                if (char === '\x1b') {
                    found = true
                } else if (found) {
                    if (char === 'm') found = false
                } else continue
                addiding_spaces++
            }
        }
        x = x.trim()
        const strlen = x.length - addiding_spaces
        // Target width is col_width
        if (!isNumber) {
            // Align to center by adding spaces to the end of the string
            const spaces = Math.floor((col_width - strlen) / 2)
            x = x + ''.padEnd(spaces, ' ')
        }
        return `${x}`.padStart(col_width + addiding_spaces, ' ')
    }

    console.log(`${''.padEnd(table_offset, ' ')}   ${['2x info', `${G}empty${R} + 2x info`, `${C}job${R} + 2x info`, '2x info', `${C}job${R} 2x info`, `${C}job${R} 2x info`, `${C}job${R} 2x info`, '2x info'].map(col_pad_number).join('   ')}`)
    const print_row_border = () => console.log(`${''.padEnd(table_offset, ' ')} ${new Array(4 + 8 * col_width + 7 * 3).fill('-').join('')}`)
    for (let i = 0; i < file_names.length; i++) {
        print_row_border()
        const file_name = file_names[i]
        const wasm = `debug/${file_name}.wasm`
        const wasmInstance = await instantiate_wasm(wasm)
        const { get_free_memory, get_used_memory, get_total_memory, doNothing, doSomething } = wasmInstance.exports
        if (!get_free_memory) throw new Error(`get_free_memory() not found in ${wasm}`)
        if (!get_used_memory) throw new Error(`get_used_memory() not found in ${wasm}`)
        if (!get_total_memory) throw new Error(`get_total_memory() not found in ${wasm}`)
        if (!doNothing) throw new Error(`doNothing() not found in ${wasm}`)
        if (!doSomething) throw new Error(`doSomething() not found in ${wasm}`)
        const usage_allocation_changes = []
        let last_usage = 0
        let total_usage = 0;
        let last_allocated = 0
        let total_allocated = 0;
        let total_allocated_last = 0;
        const log_memory = () => {
            const new_usage = get_used_memory()
            const new_allocated = get_total_memory()
            const change_usage = new_usage - last_usage
            const change_allocated = new_allocated - last_allocated
            const change_allocated_last = new_allocated - total_allocated_last

            last_usage = new_usage
            last_allocated = new_allocated
            total_allocated_last = new_allocated
            total_usage += change_usage
            total_allocated += change_allocated
            usage_allocation_changes.push([change_usage, total_usage, total_allocated, change_allocated_last])
        }
        // Begin testing
        log_memory() // Log initial memory usage
        const runtime_leak = usage_allocation_changes[usage_allocation_changes.length - 1][0] > 0 // If the memory usage increased after the 3rd log_memory() call, then there is a runtime memory leak
        doNothing()  // Do literally an empty function call
        log_memory() // Log memory usage after empty function call (this will double the any runtime function call memory leak if there is any, otherwise it will stay the same as it should)
        doSomething() // Do a function call that does some computation and data manipulation, this can change the memory usage depending on the implementation of doSomething()
        log_memory() // Log memory usage after doSomething() function call
        log_memory() // Log memory usage again to make sure it's not changing
        doSomething() // Do another doSomething() function call to make sure nothing strange is happening
        log_memory() // Log memory usage after doSomething() function call
        doSomething() // And another doSomething() function call to make sure nothing strange is happening
        log_memory() // Log memory usage after doSomething() function call
        doSomething() // And another doSomething() function call to make sure nothing strange is happening
        log_memory() // Log memory usage after doSomething() function call
        log_memory() // Log memory usage again to make sure it's not changing
        // End testing
        const addiding_spaces = (runtime_leak ? bg_red.length : 0) + c_reset.length
        console.log(`${`${runtime_leak ? bg_red : ''}${file_name}${c_reset} `.padEnd(longset_file_name_length + 2 + addiding_spaces, '-')} | ${usage_allocation_changes.map(col_pad_number).join(' | ')} | bytes`)
    }
    print_row_border()
    console.log(`Testing finished.`)
}

main().catch(error => {
    console.error(error)
})



