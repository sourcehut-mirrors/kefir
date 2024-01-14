/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#ifndef KEFIR_IR_OPCODES_H_
#define KEFIR_IR_OPCODES_H_

#include "kefir/core/basic-types.h"
#include "kefir/ir/opcode_defs.h"

typedef enum kefir_iropcode {
#define KEFIR_IR_OPCODES_ENUM(_id, _mnemonic, _type) KEFIR_IROPCODE_##_id

    KEFIR_IR_OPCODE_DEFS(KEFIR_IR_OPCODES_ENUM, COMMA)
#undef KEFIR_IR_OPCODES_ENUM
} kefir_iropcode_t;

#endif
