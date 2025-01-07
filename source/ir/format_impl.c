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

#include "kefir/ir/format_impl.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

kefir_result_t kefir_ir_format_instr(struct kefir_json_output *json, const struct kefir_ir_module *module,
                                     const struct kefir_irinstr *instr) {
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid json output"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR module"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR instruction"));
    switch (instr->opcode) {
#define KEFIR_IR_OPCODES_SYMBOL(_id, _mnemonic, _type) \
    case KEFIR_IROPCODE_##_id:                         \
        return kefir_ir_format_instr_##_type(json, module, instr);
        KEFIR_IR_OPCODE_DEFS(KEFIR_IR_OPCODES_SYMBOL, )
#undef KEFIR_IR_OPCODES_SYMBOL
    }
    return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unknown opcode");
}
