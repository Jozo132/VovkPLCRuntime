// VovkPLCRuntime.h - 2022-12-11
//
// Copyright (c) 2022 J.Vovk
//
// This file is part of VovkPLCRuntime.
//
// VovkPLCRuntime is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// VovkPLCRuntime is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with VovkPLCRuntime.  If not, see <https://www.gnu.org/licenses/>.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "tools/runtime-lib.h"
#include "tools/runtime-test.h"
#include "tools/assembly/plcasm-compiler.h"
#include "tools/assembly/plcasm-linter.h"
#include "tools/assembly/stl-compiler.h"
#include "tools/assembly/stl-linter.h"
#include "tools/assembly/ladder-compiler.h"
#include "tools/assembly/ladder-linter.h"
#include "tools/assembly/plcscript-linter.h"
#include "tools/assembly/st-compiler.h"
#include "tools/assembly/st-linter.h"
#include "tools/assembly/project-compiler.h"
#include "tools/assembly/wcet-analysis.h"

// Modbus RTU RS485 (enable with #define PLCRUNTIME_MODBUS_RTU before this include)
// Note: plc-modbus-rtu.h and plc-comms-manager.h are included internally by runtime-lib.h
// The re-include here is safe (#pragma once) and makes IDE discovery easier
#include "tools/transport/plc-modbus-rtu.h"
#include "tools/transport/plc-comms-manager.h"