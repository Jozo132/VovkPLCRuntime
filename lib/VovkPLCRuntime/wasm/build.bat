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


set C_INCLUDE_PATH="C:\MinGW\include"
@REM set CPLUS_INCLUDE_PATH="C:\MinGW\include"

IF not exist build (mkdir build)



@REM clang++ -Wall -std=c++11 simulator.cpp -o simulator.exe

@echo on
@echo Compiling dependencies:
@echo     - "walloc.o"
@echo off

clang -DNDEBUG -Oz --target=wasm32 -nostdlib -c include/walloc.c -o build/walloc.o      || goto :error

@echo on
@echo     - "simulator.o"
@echo off

clang++ --target=wasm32 -Wall -std=c++11 -nostdlib -D __WASM__ -O0 -I %C_INCLUDE_PATH% -c simulator.cpp -o build/simulator.o        || goto :error

@echo on
@echo Building "simulator.wasm" ...
@echo off

wasm-ld --no-entry --export-dynamic --allow-undefined --lto-O0 --import-memory --demangle build/simulator.o build/walloc.o -o simulator.wasm      || goto :error

@echo on
@echo Done.
@echo off

goto :EOF

:error
echo Failed with error %errorlevel%