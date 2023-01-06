// @echo off
// rem build_test.bat - 1.0.0 - 2022-12-11
// rem
// rem Copyright (c) 2022 J.Vovk
// rem
// rem This file is part of VovkPLCRuntime.
// rem
// rem VovkPLCRuntime is free software: you can redistribute it and/or modify
// rem it under the terms of the GNU General Public License as published by
// rem the Free Software Foundation, either version 3 of the License, or
// rem (at your option) any later version.
// rem
// rem VovkPLCRuntime is distributed in the hope that it will be useful,
// rem but WITHOUT ANY WARRANTY; without even the implied warranty of
// rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// rem GNU General Public License for more details.
// rem
// rem You should have received a copy of the GNU General Public License
// rem along with VovkPLCRuntime.  If not, see <https://www.gnu.org/licenses/>.
// rem
// rem SPDX-License-Identifier: GPL-3.0-or-later

// @echo on
// @echo Compiling...
// @echo off

// IF not exist build (mkdir build)
// clang++ --target=wasm32-undefined-undefined-wasm -Wall -std=c++11 -nostdlib -D __WASM__ -O3 -c test.cpp -o build/test.o        || goto :error

// @echo on
// @echo Building...
// @echo off

// wasm-ld --no-entry --export-dynamic --allow-undefined --lto-O3 build/test.o -o test.wasm      || goto :error

// @echo on
// @echo Done.
// @echo off

// goto :EOF

// :error
// echo Failed with error %errorlevel%

// @ts-check
"use strict"

// Compile all '*.cpp' files in the current directory like above but using Node.JS and process execution instead of a batch file.
// The compilation will be executed in parallel with async await.

const fs = require('fs')
const path = require('path')
const { exec } = require('child_process')

const exec_async = (command, options) => new Promise((resolve, reject) => {
    exec(command, options, (error, stdout, stderr) => {
        if (error) reject(error)
        else resolve({ stdout, stderr })
    })
})

const CC = (input, output) => `clang++ --target=wasm32-undefined-undefined-wasm -Wall -std=c++11 -nostdlib -D __WASM__ -O3 -c "${input}" -o "${output}"`
const LD = (input, output) => `wasm-ld --no-entry --export-dynamic --allow-undefined --lto-O3 "${input}" -o "${output}"`

const main = async () => {
    const all_files = fs.readdirSync(__dirname)
    const file_names = all_files.filter(file => file.endsWith('.cpp')).map(file => file.slice(0, -4))
    if (!file_names.length) throw new Error('No "*.cpp" files found')
    const build_dir_exists = fs.existsSync(path.join(__dirname, 'build'))
    if (!build_dir_exists) {
        console.log('Creating "build" directory...')
        fs.mkdirSync(path.join(__dirname, 'build'))
    }

    const commands = []
    let longset_file_name_length = 0
    for (const file_name of file_names) {
        if (file_name.length > longset_file_name_length) longset_file_name_length = file_name.length
        const input = `${file_name}.cpp`
        const output = `build/${file_name}.o`
        const wasm = `${file_name}.wasm`

        commands.push({
            name: file_name,
            CC: CC(input, output),
            LD: LD(output, wasm)
        })
    }

    console.log(`Compiling ${commands.length} test source file${commands.length === 1 ? '' : 's'}...`)
    const promises = []
    for (const command of commands) {
        const { name, CC, LD } = command
        const promise = new Promise(async (resolve, reject) => {
            try {
                let t = +new Date()
                const result = await exec_async(CC)
                t = +new Date() - t
                if (result.stderr) {
                    const msg = `Compiling ${name}.o error: "${result.stderr}"`
                    console.log(msg)
                    reject(msg)
                    return
                }
                console.log(`${`Compiled ${name}.o`.padEnd(longset_file_name_length + 8, ' ')} [ took ${t} ms ]`)
                t = +new Date()
                const result2 = await exec_async(LD)
                t = +new Date() - t
                if (result2.stderr) {
                    const msg = `Building ${name}.wasm error: "${result2.stderr}"`
                    console.log(msg)
                    reject(msg)
                    return
                }
                console.log(`${`Built ${name}.wasm`.padEnd(longset_file_name_length + 8, ' ')} [ took ${t} ms ]`)
                resolve(true)
            } catch (error) {
                reject(error)
            }
        })
        promises.push(promise)
    }
    await Promise.all(promises)
    console.log('Done.')
}

main().catch(error => {
    console.error(error)
})



