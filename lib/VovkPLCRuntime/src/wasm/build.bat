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

set NAME=simulator
set BUILD_DIR=build

IF not exist %BUILD_DIR% (mkdir %BUILD_DIR%)

@REM clang++ -Wall -std=c++11 %NAME%.cpp -o %NAME%.exe
clang++ --target=wasm32 -nostdlib -D __WASM__ -O3 -c %NAME%.cpp -o %BUILD_DIR%/%NAME%.o
wasm-ld --no-entry --export-all --lto-O3 --allow-undefined --import-memory %BUILD_DIR%/%NAME%.o -o %NAME%.wasm
