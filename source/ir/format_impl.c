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
#define KEFIR_IR_OPCODES_SYMBOL_IMPL2(_id, _mnemonic, _type) \
    case KEFIR_IR_OPCODE_##_id:                              \
        return kefir_ir_format_instr_##_type(json, module, instr);
#define TYPE_FMT_index u64
#define TYPE_FMT_u64 u64
#define TYPE_FMT_phi_ref u64
#define TYPE_FMT_inline_asm u64
#define TYPE_FMT_ref1 none
#define TYPE_FMT_ref2 none
#define TYPE_FMT_ref3_cond condition
#define TYPE_FMT_ref4_compare compare
#define TYPE_FMT_compare_ref2 compare
#define TYPE_FMT_ref_offset i64
#define TYPE_FMT_typeref typeref
#define TYPE_FMT_typed_ref1 typeref
#define TYPE_FMT_typed_ref2 typeref
#define TYPE_FMT_branch branch
#define TYPE_FMT_call_ref funcref
#define TYPE_FMT_immediate immediate
#define TYPE_FMT_none none
#define TYPE_FMT_variable identifier
#define TYPE_FMT_type typeref
#define TYPE_FMT_load_mem memflags
#define TYPE_FMT_store_mem memflags
#define TYPE_FMT_bitfield u32
#define TYPE_FMT_stack_alloc boolean
#define TYPE_FMT_atomic_op memory_order
#define TYPE_FMT_atomic_typeref atomic_typeref
#define TYPE_FMT_localvar localvar
#define TYPE_FMT_overflow_arith overflow_arith
#define TYPE_FMT_branch_compare branch_compare
#define KEFIR_IR_OPCODES_SYMBOL_IMPL(_id, _mnemonic, _type) KEFIR_IR_OPCODES_SYMBOL_IMPL2(_id, _mnemonic, _type)
#define KEFIR_IR_OPCODES_SYMBOL(_id, _mnemonic, _type) KEFIR_IR_OPCODES_SYMBOL_IMPL(_id, _mnemonic, TYPE_FMT_##_type)
        KEFIR_IR_GENERIC_OPCODE_DEFS(KEFIR_IR_OPCODES_SYMBOL, )
        KEFIR_IR_SPECIAL_OPCODE_DEFS(KEFIR_IR_OPCODES_SYMBOL, )
    }
    return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unknown opcode");
}
