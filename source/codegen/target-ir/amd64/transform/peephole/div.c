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
#include "kefir/codegen/target-ir/transform.h"
#include "kefir/codegen/target-ir/tie.h"
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/codegen/target-ir/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_target_ir_amd64_peephole_div(struct kefir_mem *mem,
                                                          struct kefir_codegen_target_ir_code *code,
                                                          const struct kefir_codegen_target_ir_instruction *instr,
                                                          kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
    REQUIRE(classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
                classification.classification.operands[1].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
                classification.classification.operands[2].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
                classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                classification.operands[2].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                instr->operation.parameters[classification.operands[0].read_index].type ==
                    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER &&
                instr->operation.parameters[classification.operands[1].read_index].type ==
                    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                instr->operation.parameters[classification.operands[2].read_index].type ==
                    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
            KEFIR_OK);

    kefir_result_t res;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
    kefir_codegen_target_ir_value_ref_t used_value_ref;
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, instr_ref, &use_instr_ref, &used_value_ref);
         res == KEFIR_OK; res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
        REQUIRE(used_value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), KEFIR_OK);
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    struct kefir_codegen_target_ir_operation oper = instr->operation;
    for (kefir_uint32_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT; i++) {
        if (oper.parameters[classification.operands[0].read_index].immediate.uint_immediate == (1ull << i)) {
            const struct kefir_codegen_target_ir_value_type *output_value_type, *arg1_value_type, *arg2_value_type;
            kefir_codegen_target_ir_value_ref_t output_value_ref = {
                .instr_ref = instr_ref, .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)};
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, output_value_ref, &output_value_type));
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(
                code, oper.parameters[classification.operands[1].read_index].direct.value_ref, &arg1_value_type));
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(
                code, oper.parameters[classification.operands[2].read_index].direct.value_ref, &arg2_value_type));
            struct kefir_codegen_target_ir_value_type arg1_value_type_copy = *arg1_value_type;
            struct kefir_codegen_target_ir_value_type arg2_value_type_copy = *arg2_value_type;

            kefir_codegen_target_ir_instruction_ref_t single_user_ref;
            REQUIRE_OK(kefir_codegen_target_ir_code_get_single_user(
                code, oper.parameters[classification.operands[1].read_index].direct.value_ref, &single_user_ref));
            if (single_user_ref == instr_ref) {
                arg1_value_type_copy.constraint.type = KEFIR_CODEGEN_TARGET_IR_ALLOCATION_NO_CONSTRAINT;
            }
            REQUIRE_OK(kefir_codegen_target_ir_code_get_single_user(
                code, oper.parameters[classification.operands[2].read_index].direct.value_ref, &single_user_ref));
            if (single_user_ref == instr_ref) {
                arg2_value_type_copy.constraint.type = KEFIR_CODEGEN_TARGET_IR_ALLOCATION_NO_CONSTRAINT;
            }

            kefir_codegen_target_ir_value_ref_t shr_value_ref = {.aspect =
                                                                     KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)};
            struct kefir_codegen_target_ir_instruction_metadata metadata = instr->metadata;
            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(
                mem, code, instr->block_ref, instr_ref,
                &(struct kefir_codegen_target_ir_operation) {
                    .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(shr),
                    .parameters[0] = oper.parameters[classification.operands[1].read_index],
                    .parameters[1] = {.type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
                                      .immediate = {.int_immediate = i,
                                                    .variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT}}},
                &metadata, &shr_value_ref.instr_ref));

            REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(
                mem, code, shr_value_ref,
                &(struct kefir_codegen_target_ir_value_type) {
                    .kind = output_value_type->kind,
                    .metadata = output_value_type->metadata,
                    .variant = output_value_type->variant != KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                                   ? output_value_type->variant
                                   : KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT}));
            REQUIRE_OK(kefir_codegen_target_ir_add_produced_resource_aspects(mem, code, shr_value_ref.instr_ref));
            REQUIRE_OK(kefir_codegen_target_ir_code_replace_value(mem, code, shr_value_ref, output_value_ref));
            REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, instr_ref));
            *replaced = true;

            REQUIRE_OK(kefir_codegen_target_ir_code_replace_aspect(
                code, oper.parameters[classification.operands[1].read_index].direct.value_ref, &arg1_value_type_copy));
            REQUIRE_OK(kefir_codegen_target_ir_code_replace_aspect(
                code, oper.parameters[classification.operands[2].read_index].direct.value_ref, &arg2_value_type_copy));
            break;
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_peephole_idiv(struct kefir_mem *mem,
                                                           struct kefir_codegen_target_ir_code *code,
                                                           const struct kefir_codegen_target_ir_instruction *instr,
                                                           kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
    REQUIRE(classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
                classification.classification.operands[1].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
                classification.classification.operands[2].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
                classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                classification.operands[2].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                instr->operation.parameters[classification.operands[0].read_index].type ==
                    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER &&
                instr->operation.parameters[classification.operands[1].read_index].type ==
                    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                instr->operation.parameters[classification.operands[2].read_index].type ==
                    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
            KEFIR_OK);

    kefir_result_t res;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
    kefir_codegen_target_ir_value_ref_t used_value_ref;
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, instr_ref, &use_instr_ref, &used_value_ref);
         res == KEFIR_OK; res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
        REQUIRE(used_value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), KEFIR_OK);
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    struct kefir_codegen_target_ir_operation oper = instr->operation;
    for (kefir_uint32_t i = 0; i < sizeof(kefir_uint32_t) * CHAR_BIT - 1; i++) {
        kefir_int64_t imm = oper.parameters[classification.operands[0].read_index].immediate.int_immediate;
        kefir_bool_t neg = false;
        if (imm < 0) {
            imm *= -1;
            neg = true;
        }
        if (imm == (1ll << i)) {
            const struct kefir_codegen_target_ir_value_type *output_value_type, *arg1_value_type, *arg2_value_type;
            kefir_codegen_target_ir_value_ref_t output_value_ref = {
                .instr_ref = instr_ref, .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)};
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, output_value_ref, &output_value_type));
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(
                code, oper.parameters[classification.operands[1].read_index].direct.value_ref, &arg1_value_type));
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(
                code, oper.parameters[classification.operands[2].read_index].direct.value_ref, &arg2_value_type));
            struct kefir_codegen_target_ir_value_type arg1_value_type_copy = *arg1_value_type;
            struct kefir_codegen_target_ir_value_type arg2_value_type_copy = *arg2_value_type;
            struct kefir_codegen_target_ir_value_type output_value_type_copy = *output_value_type;

            kefir_codegen_target_ir_operand_variant_t variant =
                oper.parameters[classification.operands[1].read_index].direct.variant !=
                        KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                    ? oper.parameters[classification.operands[1].read_index].direct.variant
                    : KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT;

            struct kefir_codegen_target_ir_use_iterator use_iter;
            kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
            kefir_codegen_target_ir_value_ref_t used_value_ref;
            for (res = kefir_codegen_target_ir_code_use_iter(
                     code, &use_iter, oper.parameters[classification.operands[1].read_index].direct.value_ref.instr_ref,
                     &use_instr_ref, &used_value_ref);
                 res == KEFIR_OK;
                 res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
                const struct kefir_codegen_target_ir_instruction *user_instr;
                REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, use_instr_ref, &user_instr));
                if (user_instr->block_ref == KEFIR_ID_NONE) {
                    continue;
                }

                REQUIRE(use_instr_ref == instr_ref ||
                            use_instr_ref ==
                                oper.parameters[classification.operands[2].read_index].direct.value_ref.instr_ref,
                        KEFIR_OK);
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
            arg1_value_type_copy.constraint.type = KEFIR_CODEGEN_TARGET_IR_ALLOCATION_NO_CONSTRAINT;

            kefir_codegen_target_ir_instruction_ref_t single_user_ref;
            REQUIRE_OK(kefir_codegen_target_ir_code_get_single_user(
                code, oper.parameters[classification.operands[2].read_index].direct.value_ref, &single_user_ref));
            if (single_user_ref == instr_ref) {
                arg2_value_type_copy.constraint.type = KEFIR_CODEGEN_TARGET_IR_ALLOCATION_NO_CONSTRAINT;
            }

            kefir_codegen_target_ir_block_ref_t block_ref = instr->block_ref;
            struct kefir_codegen_target_ir_instruction_metadata metadata = instr->metadata;

            kefir_codegen_target_ir_instruction_ref_t test_instr_ref;
            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(
                mem, code, block_ref, instr_ref,
                &(struct kefir_codegen_target_ir_operation) {
                    .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(test),
                    .parameters[0] = oper.parameters[classification.operands[1].read_index],
                    .parameters[1] = oper.parameters[classification.operands[1].read_index]},
                &metadata, &test_instr_ref));

            REQUIRE_OK(kefir_codegen_target_ir_add_produced_resource_aspects(mem, code, test_instr_ref));

            kefir_codegen_target_ir_value_ref_t lea_value_ref = {.aspect =
                                                                     KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)};
            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(
                mem, code, block_ref, test_instr_ref,
                &(struct kefir_codegen_target_ir_operation) {
                    .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(lea),
                    .parameters[0] = {.type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT,
                                      .indirect = {.type = KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS,
                                                   .index_type = KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_NONE,
                                                   .base =
                                                       {
                                                           .value_ref =
                                                               oper.parameters[classification.operands[1].read_index]
                                                                   .direct.value_ref,
                                                       },
                                                   .offset = (1ll << i) - 1,
                                                   .variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT}}},
                &metadata, &lea_value_ref.instr_ref));
            REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(
                mem, code, lea_value_ref,
                &(struct kefir_codegen_target_ir_value_type) {
                    .kind = KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE,
                    .metadata.value_ref = KEFIR_CODEGEN_TARGET_IR_METADATA_VALUE_REF_NONE,
                    .variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT}));
            REQUIRE_OK(kefir_codegen_target_ir_add_produced_resource_aspects(mem, code, lea_value_ref.instr_ref));

            kefir_codegen_target_ir_value_ref_t cmovns_value_ref = {.aspect =
                                                                        KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)};
            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(
                mem, code, block_ref, lea_value_ref.instr_ref,
                &(struct kefir_codegen_target_ir_operation) {
                    .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(cmovns),
                    .parameters[0] = {.type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                                      .direct = {.value_ref = lea_value_ref,
                                                 .variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT}},
                    .parameters[1] = {.type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                                      .direct = {.value_ref = oper.parameters[classification.operands[1].read_index]
                                                                  .direct.value_ref,
                                                 .variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT}},
                    .parameters[2] = {.type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                                      .direct = {.value_ref = {.instr_ref = test_instr_ref,
                                                               .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_RESOURCE(
                                                                   KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF)},
                                                 .variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT}}},
                &metadata, &cmovns_value_ref.instr_ref));
            REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(
                mem, code, cmovns_value_ref,
                &(struct kefir_codegen_target_ir_value_type) {
                    .kind = KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE,
                    .metadata.value_ref = KEFIR_CODEGEN_TARGET_IR_METADATA_VALUE_REF_NONE,
                    .variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT}));
            REQUIRE_OK(kefir_codegen_target_ir_add_produced_resource_aspects(mem, code, cmovns_value_ref.instr_ref));

            kefir_codegen_target_ir_value_ref_t sar_value_ref = {.aspect =
                                                                     KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)};
            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(
                mem, code, block_ref, cmovns_value_ref.instr_ref,
                &(struct kefir_codegen_target_ir_operation) {
                    .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(sar),
                    .parameters[0] = {.type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                                      .direct = {.value_ref = cmovns_value_ref, .variant = variant}},
                    .parameters[1] = {.type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
                                      .immediate = {.int_immediate = i, .variant = variant}}},
                &metadata, &sar_value_ref.instr_ref));
            REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(
                mem, code, sar_value_ref,
                &(struct kefir_codegen_target_ir_value_type) {
                    .kind = output_value_type_copy.kind,
                    .metadata.value_ref = neg ? KEFIR_CODEGEN_TARGET_IR_METADATA_VALUE_REF_NONE
                                              : output_value_type_copy.metadata.value_ref,
                    .variant = variant}));
            REQUIRE_OK(kefir_codegen_target_ir_add_produced_resource_aspects(mem, code, sar_value_ref.instr_ref));

            kefir_codegen_target_ir_value_ref_t result_ref = sar_value_ref;
            if (neg) {
                kefir_codegen_target_ir_value_ref_t neg_value_ref = {
                    .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)};
                REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(
                    mem, code, block_ref, sar_value_ref.instr_ref,
                    &(struct kefir_codegen_target_ir_operation) {
                        .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(neg),
                        .parameters[0] = {.type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                                          .direct = {.value_ref = sar_value_ref, .variant = variant}}},
                    &metadata, &neg_value_ref.instr_ref));
                REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(
                    mem, code, neg_value_ref,
                    &(struct kefir_codegen_target_ir_value_type) {
                        .kind = output_value_type_copy.kind,
                        .metadata.value_ref = output_value_type_copy.metadata.value_ref,
                        .variant = variant}));
                REQUIRE_OK(kefir_codegen_target_ir_add_produced_resource_aspects(mem, code, neg_value_ref.instr_ref));
                result_ref = neg_value_ref;
            }

            REQUIRE_OK(kefir_codegen_target_ir_code_replace_value(mem, code, result_ref, output_value_ref));
            REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, instr_ref));
            *replaced = true;

            REQUIRE_OK(kefir_codegen_target_ir_code_replace_aspect(
                code, oper.parameters[classification.operands[1].read_index].direct.value_ref, &arg1_value_type_copy));
            REQUIRE_OK(kefir_codegen_target_ir_code_replace_aspect(
                code, oper.parameters[classification.operands[2].read_index].direct.value_ref, &arg2_value_type_copy));
            break;
        }
    }
    return KEFIR_OK;
}
