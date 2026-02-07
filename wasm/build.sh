#!/bin/bash
# build.sh - 2022-12-11
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

# Check for --safe flag
SAFE_MODE=0
for arg in "$@"; do
    if [ "$arg" = "--safe" ]; then
        SAFE_MODE=1
    fi
done

if [ $SAFE_MODE -eq 1 ]; then
    # Build safe mode version (with PLCRUNTIME_SAFE_MODE bounds checks)
    clang++ --target=wasm32-undefined-undefined-wasm -Wall -std=c++11 -nostdlib -O3 -D __WASM__ -D PLCRUNTIME_SAFE_MODE -c VovkPLC.cpp -o build/VovkPLC.o
else
    # Build production version (no bounds checks)
    clang++ --target=wasm32-undefined-undefined-wasm -Wall -std=c++11 -nostdlib -O3 -D __WASM__ -c VovkPLC.cpp -o build/VovkPLC.o
fi

echo "Building..."
wasm-ld --no-entry --export-dynamic --allow-undefined --lto-O3 build/VovkPLC.o -o dist/VovkPLC.wasm
echo "Done."