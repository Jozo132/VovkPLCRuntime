@echo off
rem build.bat - 2022-12-11
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


rem try to execute 'cd wasm', if failed do nothing
cd wasm 2>nul

IF not exist build (mkdir build)

rem Check for --safe flag
set SAFE_MODE=0
for %%a in (%*) do (
    if "%%a"=="--safe" set SAFE_MODE=1
)

if %SAFE_MODE%==1 (
    rem Build safe mode version (with PLCRUNTIME_SAFE_MODE bounds checks)
    clang++ --target=wasm32-undefined-undefined-wasm -Wall -std=c++11 -nostdlib -O3 -D __WASM__ -D PLCRUNTIME_SAFE_MODE -c VovkPLC.cpp -o build/VovkPLC.o        || goto :error
) else (
    rem Build production version (no bounds checks)
    clang++ --target=wasm32-undefined-undefined-wasm -Wall -std=c++11 -nostdlib -O3 -D __WASM__ -c VovkPLC.cpp -o build/VovkPLC.o        || goto :error
)

@echo on
@echo Building...
@echo off

wasm-ld --no-entry --export-dynamic --allow-undefined --lto-O3 build/VovkPLC.o -o dist/VovkPLC.wasm      || goto :error

@echo on
@echo Done.
@echo off

goto :EOF

@REM Compile Windows EXE (TEST)
@REM clang++ -Wall -std=c++11 VovkPLC.cpp -o VovkPLC.exe

:error
echo Failed with error %errorlevel%