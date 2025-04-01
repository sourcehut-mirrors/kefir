/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

    This file is part of Kefir project.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KEFIR_IR_OPCODE_DEFS_H_
#define KEFIR_IR_OPCODE_DEFS_H_

#include "kefir/optimizer/opcode_defs.h"

#define KEFIR_IR_OPCODES_REVISION 1
// clang-format off
#define KEFIR_IR_GENERIC_OPCODE_DEFS KEFIR_OPTIMIZER_OPCODE_DEFS
#define KEFIR_IR_SPECIAL_OPCODE_DEFS(OPCODE, SEPARATOR) \
    OPCODE(NOP, "nop", none) SEPARATOR \
    OPCODE(GET_LOCAL, "get_local", localvar) SEPARATOR \
    OPCODE(VSTACK_POP, "vstack_pop", none) SEPARATOR \
    OPCODE(VSTACK_PICK, "vstack_pick", u64) SEPARATOR \
    OPCODE(VSTACK_EXCHANGE, "vstack_exchange", u64)
// clang-format on

#endif
