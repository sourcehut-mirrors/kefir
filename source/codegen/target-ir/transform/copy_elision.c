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

#include "kefir/codegen/target-ir/transform.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t do_copy_elision(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_block_ref_t block_ref) {
    for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(code, block_ref);
        instr_ref != KEFIR_ID_NONE;) {
        const struct kefir_codegen_target_ir_instruction *assign_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &assign_instr));
        instr_ref = kefir_codegen_target_ir_code_control_next(code, instr_ref);
        if (assign_instr->operation.opcode != code->klass->assign_opcode ||
            assign_instr->operation.parameters[0].direct.value_ref.aspect != KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0) ||
            assign_instr->operation.parameters[0].type != KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF) {
            continue;
        }

        const struct kefir_codegen_target_ir_value_type *assign_value_type, *assigned_value_type;
        REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, (kefir_codegen_target_ir_value_ref_t) {
            .instr_ref = assign_instr->instr_ref,
            .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
        }, &assign_value_type));
        if (assign_value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
            continue;
        }

        REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, assign_instr->operation.parameters[0].direct.value_ref, &assigned_value_type));
        if (assigned_value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
            continue;
        }

        if (assign_value_type->kind != assigned_value_type->kind) {
            continue;
        }

        kefir_bool_t elide_copy = false;
        switch (assign_value_type->kind) {
            case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_UNSPECIFIED:
            case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE:
            case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT:
            case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLAGS:
                elide_copy = true;
                break;

            case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_INDIRECT:
            case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_SPILL_SPACE:
            case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE:
            case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_EXTERNAL_MEMORY:
                // Intentionally left blank
                break;
        }

        if (elide_copy) {
            REQUIRE_OK(kefir_codegen_target_ir_code_replace_instruction(mem, code, assign_instr->operation.parameters[0].direct.value_ref.instr_ref, assign_instr->instr_ref));
            REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, assign_instr->instr_ref));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_transform_copy_elision(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        REQUIRE_OK(do_copy_elision(mem, code, block_ref));
    }
    return KEFIR_OK;
}
