@echo off
rem build.bat - 1.0.0 - 2022-12-11
rem
rem Copyright (c) 2022 J.Vovk
rem
rem This file is part of VovkPLCRuntime.
rem
rem VovkPLCRuntime is free software: you can redistribute it and/or modify
rem it under the terms of the GNU General Public License as published by
rem the Free Software Foundation, either version 3 of the License, or
rem (at your option) any later version.
rem
rem VovkPLCRuntime is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem GNU General Public License for more details.
rem
rem You should have received a copy of the GNU General Public License
rem along with VovkPLCRuntime.  If not, see <https://www.gnu.org/licenses/>.
rem
rem SPDX-License-Identifier: GPL-3.0-or-later

@echo on
@echo Compiling...
@echo off


rem try to execute 'cd test/VovkPLCRuntimeWasmTestKit', if failed do nothing
cd test\VovkPLCRuntimeWasmTestKit 2>nul

IF not exist build (mkdir build)
clang++ --target=wasm32-undefined-undefined-wasm -Wall -std=c++11 -nostdlib -O3 -D __WASM__ -c simulator.cpp -o build/simulator.o        || goto :error

@echo on
@echo Building...
@echo off

wasm-ld --no-entry --export-dynamic --allow-undefined --lto-O3 build/simulator.o -o simulator.wasm      || goto :error

@echo on
@echo Done.
@echo off

goto :EOF

@REM Compile Windows EXE (TEST)
@REM clang++ -Wall -std=c++11 simulator.cpp -o simulator.exe

:error
echo Failed with error %errorlevel%