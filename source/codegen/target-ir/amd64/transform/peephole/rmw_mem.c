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
#include "kefir/codegen/target-ir/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_codegen_target_ir_amd64_peephole_rmw_mem(
    struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
    const struct kefir_codegen_target_ir_instruction *load_instr, kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(load_instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_codegen_target_ir_code_attribute_iterator attr_iter;
    REQUIRE(kefir_codegen_target_ir_code_instruction_attribute_iter(code, &attr_iter, load_instr->instr_ref, NULL) ==
                KEFIR_ITERATOR_END,
            KEFIR_OK);

    struct kefir_codegen_target_ir_tie_classification load_classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, load_instr->instr_ref, &load_classification));
    REQUIRE(load_classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE, KEFIR_OK);

    kefir_codegen_target_ir_value_ref_t load_output_value_ref = load_classification.operands[0].output;
    REQUIRE(load_output_value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), KEFIR_OK);

    const struct kefir_codegen_target_ir_value_type *load_output_value_type;
    kefir_result_t res = kefir_codegen_target_ir_code_value_props(code, load_output_value_ref, &load_output_value_type);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);

    const struct kefir_codegen_target_ir_operand *load_input_param =
        &load_instr->operation.parameters[load_classification.operands[1].read_index];
    REQUIRE(load_input_param->type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT &&
                load_output_value_type->constraint.type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT,
            KEFIR_OK);

    kefir_codegen_target_ir_instruction_ref_t modify_instr_ref;
    REQUIRE_OK(kefir_codegen_target_ir_code_get_single_user(code, load_output_value_ref, &modify_instr_ref));
    REQUIRE(modify_instr_ref != KEFIR_ID_NONE &&
                kefir_codegen_target_ir_code_control_next(code, load_instr->instr_ref) == modify_instr_ref,
            KEFIR_OK);

    REQUIRE(kefir_codegen_target_ir_code_instruction_attribute_iter(code, &attr_iter, modify_instr_ref, NULL) ==
                KEFIR_ITERATOR_END,
            KEFIR_OK);

    const struct kefir_codegen_target_ir_instruction *modify_instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, modify_instr_ref, &modify_instr));

    switch (modify_instr->operation.opcode) {
        case KEFIR_TARGET_IR_AMD64_OPCODE(add):
        case KEFIR_TARGET_IR_AMD64_OPCODE(sub):
        case KEFIR_TARGET_IR_AMD64_OPCODE(and):
        case KEFIR_TARGET_IR_AMD64_OPCODE(or):
        case KEFIR_TARGET_IR_AMD64_OPCODE(xor):
        case KEFIR_TARGET_IR_AMD64_OPCODE(shl):
        case KEFIR_TARGET_IR_AMD64_OPCODE(shr):
        case KEFIR_TARGET_IR_AMD64_OPCODE(sar):
            // Intentionally left blank
            break;

        default:
            return KEFIR_OK;
    }

    struct kefir_codegen_target_ir_tie_classification modify_classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, modify_instr_ref, &modify_classification));
    REQUIRE(modify_classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                modify_classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE,
            KEFIR_OK);

    const struct kefir_codegen_target_ir_operand *modify_input_param =
        &modify_instr->operation.parameters[modify_classification.operands[0].read_index];
    REQUIRE(modify_input_param->type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                modify_input_param->direct.value_ref.instr_ref == load_output_value_ref.instr_ref &&
                modify_input_param->direct.value_ref.aspect == load_output_value_ref.aspect &&
                !modify_input_param->direct.tied &&
                modify_input_param->direct.variant == load_output_value_type->variant,
            KEFIR_OK);

    kefir_codegen_target_ir_value_ref_t modify_output_value_ref = modify_classification.operands[0].output;
    REQUIRE(modify_output_value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), KEFIR_OK);
    const struct kefir_codegen_target_ir_value_type *modify_output_value_type;
    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, modify_output_value_ref, &modify_output_value_type));

    kefir_codegen_target_ir_instruction_ref_t store_instr_ref;
    REQUIRE_OK(kefir_codegen_target_ir_code_get_single_user(code, modify_output_value_ref, &store_instr_ref));
    REQUIRE(store_instr_ref != KEFIR_ID_NONE &&
                kefir_codegen_target_ir_code_control_next(code, modify_instr_ref) == store_instr_ref,
            KEFIR_OK);

    REQUIRE(kefir_codegen_target_ir_code_instruction_attribute_iter(code, &attr_iter, store_instr_ref, NULL) ==
                KEFIR_ITERATOR_END,
            KEFIR_OK);

    const struct kefir_codegen_target_ir_instruction *store_instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, store_instr_ref, &store_instr));
    REQUIRE(
        store_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(mov) &&
            store_instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT &&
            kefir_codegen_target_ir_indirect_operand_same(load_input_param, &store_instr->operation.parameters[0]) &&
            store_instr->operation.parameters[1].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            store_instr->operation.parameters[1].direct.value_ref.instr_ref == modify_output_value_ref.instr_ref &&
            store_instr->operation.parameters[1].direct.value_ref.aspect == modify_output_value_ref.aspect &&
            !store_instr->operation.parameters[1].direct.tied &&
            store_instr->operation.parameters[1].direct.variant == modify_output_value_type->variant,
        KEFIR_OK);

    struct kefir_codegen_target_ir_operation oper = {
        .opcode = modify_instr->operation.opcode,
        .parameters[0] = *load_input_param,
        .parameters[1] = modify_instr->operation.parameters[modify_classification.operands[1].read_index]};
    struct kefir_codegen_target_ir_instruction_metadata metadata = modify_instr->metadata;
    kefir_codegen_target_ir_instruction_ref_t replacement_ref;
    REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, load_instr->block_ref, load_instr->instr_ref,
                                                            &oper, &metadata, &replacement_ref));
    REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(
        mem, code,
        (struct kefir_codegen_target_ir_value_ref) {.instr_ref = replacement_ref,
                                                    .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_INDIRECT_OUTPUT(0)},
        &(struct kefir_codegen_target_ir_value_type) {
            .kind = KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_INDIRECT,
            .variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT,
            .metadata = {.value_ref = KEFIR_CODEGEN_TARGET_IR_METADATA_VALUE_REF_NONE}}));
    REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, store_instr_ref));
    *replaced = true;

    return KEFIR_OK;
}
