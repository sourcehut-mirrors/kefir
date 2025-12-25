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

#include "kefir/codegen/target-ir/split.h"
#include "kefir/codegen/target-ir/update.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_target_ir_split_live_range_after(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
    const struct kefir_codegen_target_ir_control_flow *control_flow, kefir_codegen_target_ir_block_ref_t block_ref, kefir_codegen_target_ir_value_ref_t value_ref, kefir_codegen_target_ir_instruction_ref_t split_after_instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));

    const struct kefir_codegen_target_ir_instruction *instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, value_ref.instr_ref, &instr));

    const struct kefir_codegen_target_ir_value_type *value_type;
    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, value_ref, &value_type));

    struct kefir_codegen_target_ir_instruction_metadata metadata = instr->metadata;
    struct kefir_codegen_target_ir_value_type value_type_copy = *value_type;

    kefir_codegen_target_ir_instruction_ref_t split_instr_ref;
    REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref,
        split_after_instr_ref,
        &(struct kefir_codegen_target_ir_operation) {
            .opcode = code->klass->assign_opcode,
            .parameters[0] = {
                .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                .direct.value_ref = value_ref,
                .direct.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
            }
        }, &metadata, &split_instr_ref));
    kefir_codegen_target_ir_value_ref_t split_value_ref = {
        .instr_ref = split_instr_ref,
        .aspect = value_ref.aspect
    };
    REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, split_value_ref, &value_type_copy));
    REQUIRE_OK(kefir_codegen_target_ir_partial_replace_value(mem, code, control_flow, split_value_ref, value_ref));

    return KEFIR_OK;
}