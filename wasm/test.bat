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
@echo Starting WASM tests...
@echo off


rem try to execute 'cd wasm/wasm_test_cases', if failed do nothing
cd wasm\wasm_test_cases 2>nul
cd wasm_test_cases 2>nul

node compile_tests.js      || goto :error

cd ..
@echo on
@echo Done.
@echo off

goto :EOF

@REM Compile Windows EXE (TEST)
@REM clang++ -Wall -std=c++11 simulator.cpp -o simulator.exe

:error

cd ..
echo Failed with error %errorlevel%