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

NAME=simulator
BUILD_DIR=build
INCLUDE="C:\MinGW\include"

clang++ --target=wasm32	-nostdlib -O3 -o $BUILD_DIR/$NAME.o -c -I $INCLUDE $NAME.cpp
wasm-ld --no-entry --export-all --lto-O3 --allow-undefined --import-memory $BUILD_DIR/$NAME.o -o $NAME.wasm
