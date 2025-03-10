#!/bin/bash
# build.sh - 1.0.0 - 2022-12-11
#
# Copyright (c) 2022 J.Vovk
#
# This file is part of VovkPLCRuntime.
#
# VovkPLCRuntime is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# VovkPLCRuntime is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with VovkPLCRuntime.  If not, see <https://www.gnu.org/licenses/>.
#
# SPDX-License-Identifier: GPL-3.0-or-later
set -e

# Set variable 'target' relative path to './wasm'

echo "Compiling..."
# try to cd, if failed do nothing
cd wasm 2>/dev/null || true 
mkdir -p build
clang++ --target=wasm32-undefined-undefined-wasm -Wall -std=c++11 -nostdlib -O3 -D __WASM__ -c VovkPLC.cpp -o build/VovkPLC.o
echo "Building..."
wasm-ld --no-entry --export-dynamic --allow-undefined --lto-O3 build/VovkPLC.o -o VovkPLC.wasm
echo "Done."