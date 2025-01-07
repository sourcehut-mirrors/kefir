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

#include "kefir/ir/mnemonic.h"

const char *kefir_iropcode_mnemonic(kefir_iropcode_t opcode) {
    switch (opcode) {
#define KEFIR_IR_OPCODES_SYMBOL(_id, _mnemonic, _type) \
    case KEFIR_IROPCODE_##_id:                         \
        return _mnemonic;
        KEFIR_IR_OPCODE_DEFS(KEFIR_IR_OPCODES_SYMBOL, )
#undef KEFIR_IR_OPCODES_SYMBOL
        default:
            break;
    }
    return NULL;
}
