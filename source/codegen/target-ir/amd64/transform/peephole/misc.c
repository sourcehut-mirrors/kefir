/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#define KEFIR_CODEGEN_TARGET_IR_AMD64_PEEPHOLE_INTERNAL
#include "kefir/codegen/target-ir/amd64/transform.h"
#include "kefir/codegen/target-ir/amd64/code.h"
#include "kefir/codegen/target-ir/tie.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_target_ir_amd64_peephole_reduce_variant(
    struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
    const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    kefir_result_t res;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t use_instr_ref, instr_ref = instr->instr_ref;
    kefir_codegen_target_ir_value_ref_t used_value_ref;

    const struct kefir_codegen_target_ir_value_type *output_type;
    kefir_codegen_target_ir_value_ref_t output_value_ref;
    REQUIRE_OK(
        kefir_codegen_target_ir_code_instruction_output(code, instr->instr_ref, 0, &output_value_ref, &output_type));

    kefir_size_t found_variants = 0;
    kefir_codegen_target_ir_operand_variant_t max_use_variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT;
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, instr_ref, &use_instr_ref, &used_value_ref);
         res == KEFIR_OK; res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
        const struct kefir_codegen_target_ir_instruction *use_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, use_instr_ref, &use_instr));
        if (use_instr->operation.opcode == code->klass->phi_opcode ||
            use_instr->operation.opcode == code->klass->inline_asm_opcode ||
            use_instr->operation.opcode == code->klass->upsilon_opcode ||
            used_value_ref.aspect != KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)) {
            max_use_variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT;
            break;
        }

        for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
            if (use_instr->operation.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                use_instr->operation.parameters[i].direct.value_ref.instr_ref == instr_ref &&
                !use_instr->operation.parameters[i].direct.tied &&
                (use_instr->operation.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||
                 use_instr->operation.parameters[i].direct.variant >= max_use_variant)) {
                max_use_variant = use_instr->operation.parameters[i].direct.variant;
                found_variants++;
            } else if (use_instr->operation.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                       use_instr->operation.parameters[i].direct.value_ref.instr_ref == instr_ref &&
                       use_instr->operation.parameters[i].direct.tied) {
                max_use_variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT;
                break;
            } else if (use_instr->operation.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT &&
                       use_instr->operation.parameters[i].indirect.type ==
                           KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS &&
                       use_instr->operation.parameters[i].indirect.base.value_ref.instr_ref == instr_ref) {
                max_use_variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT;
                break;
            } else if (use_instr->operation.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT &&
                       use_instr->operation.parameters[i].indirect.index_type ==
                           KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_VALUE_REF &&
                       use_instr->operation.parameters[i].indirect.index.value_ref.instr_ref == instr_ref) {
                max_use_variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT;
                break;
            }
        }
        if (max_use_variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT) {
            break;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    if (max_use_variant != KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT && found_variants > 0 &&
        max_use_variant < output_type->variant) {
        struct kefir_codegen_target_ir_value_type value_type = *output_type;
        value_type.variant = max_use_variant;

        struct kefir_codegen_target_ir_tie_classification classification;
        REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

        struct kefir_codegen_target_ir_operation oper = instr->operation;
        for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
            if (oper.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF) {
                oper.parameters[i].direct.variant = max_use_variant;
            } else if (oper.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT) {
                oper.parameters[i].indirect.variant = max_use_variant;
            } else if (oper.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_EXTERNAL ||
                       oper.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_NATIVE ||
                       oper.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_BLOCK_REF) {
                oper.parameters[i].rip_indirection.variant = max_use_variant;
            } else if (oper.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER) {
                oper.parameters[i].immediate.variant = max_use_variant;
            }
        }
        kefir_codegen_target_ir_instruction_ref_t replacement_ref;
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref, &oper, &replacement_ref));
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_aspect(
            code,
            (kefir_codegen_target_ir_value_ref_t) {.instr_ref = replacement_ref,
                                                   .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)},
            &value_type));
        *replaced = true;
    }
    return KEFIR_OK;
}