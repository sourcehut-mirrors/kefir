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

#define KEFIR_CODEGEN_TARGET_IR_AMD64_PEEPHOLE_INTERNAL
#include "kefir/codegen/target-ir/amd64/transform.h"
#include "kefir/codegen/target-ir/amd64/code.h"
#include "kefir/codegen/target-ir/amd64/util.h"
#include "kefir/codegen/target-ir/tie.h"
#include "kefir/codegen/target-ir/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_target_ir_amd64_peephole_imul(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));
    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;

    if (classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
            classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            instr->operation.parameters[classification.operands[0].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            !instr->operation.parameters[classification.operands[0].read_index].direct.tied &&
            (instr->operation.parameters[classification.operands[0].read_index].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||
            instr->operation.parameters[classification.operands[0].read_index].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT ||
            instr->operation.parameters[classification.operands[0].read_index].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT) &&
            instr->operation.parameters[classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF) {

        kefir_int64_t rhs_value = 0, lhs_value = 0;
        kefir_bool_t has_rhs_value = false, has_lhs_value = false;
        kefir_result_t res = kefir_codegen_target_ir_amd64_match_immediate(code, instr->operation.parameters[classification.operands[1].read_index].direct.value_ref, &rhs_value);
        if (res != KEFIR_NO_MATCH) {
            REQUIRE_OK(res);
            has_rhs_value = true;
        }
        res = kefir_codegen_target_ir_amd64_match_immediate(code, instr->operation.parameters[classification.operands[0].read_index].direct.value_ref, &lhs_value);
        if (res != KEFIR_NO_MATCH) {
            REQUIRE_OK(res);
            has_lhs_value = true;
        }

        if (has_rhs_value) {
            REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref, &(struct kefir_codegen_target_ir_operation) {
                .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(imul3),
                .parameters[0] = instr->operation.parameters[classification.operands[0].read_index],
                .parameters[1] = {
                    .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
                    .immediate = {
                        .uint_immediate = rhs_value,
                        .variant = instr->operation.parameters[classification.operands[1].read_index].direct.variant
                    }
                }
            }));
            *replaced = true;
            return KEFIR_OK;
        } else if (has_lhs_value && instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(imul)) {
            REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref, &(struct kefir_codegen_target_ir_operation) {
                .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(imul3),
                .parameters[0] = instr->operation.parameters[classification.operands[1].read_index],
                .parameters[1] = {
                    .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
                    .immediate = {
                        .uint_immediate = lhs_value,
                        .variant = instr->operation.parameters[classification.operands[0].read_index].direct.variant
                    }
                }
            }));
            *replaced = true;
            return KEFIR_OK;
        }
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_peephole_imul3(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
    REQUIRE(classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE &&
            classification.classification.operands[1].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
            classification.classification.operands[2].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
            classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            classification.operands[2].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            instr->operation.parameters[classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            !instr->operation.parameters[classification.operands[1].read_index].direct.tied &&
            instr->operation.parameters[classification.operands[2].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER, KEFIR_OK);

    kefir_result_t res;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
    kefir_codegen_target_ir_value_ref_t used_value_ref;
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, instr_ref, &use_instr_ref, &used_value_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
        REQUIRE(used_value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), KEFIR_OK);
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    switch (instr->operation.parameters[classification.operands[2].read_index].immediate.int_immediate) {
        case 0: {
            kefir_codegen_target_ir_value_ref_t zero_ref = {
                .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
            };

            struct kefir_codegen_target_ir_instruction_metadata metadata = instr->metadata;
            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, instr->block_ref, instr_ref, 
                &(struct kefir_codegen_target_ir_operation) {
                    .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(mov),
                    .parameters[0] = {
                        .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
                        .immediate.int_immediate = 0,
                        .immediate.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                    }
                }, &metadata, &zero_ref.instr_ref));

            const struct kefir_codegen_target_ir_value_type *output_value_type;
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, classification.operands[0].output,
                &output_value_type));
            struct kefir_codegen_target_ir_value_type zero_value_type = *output_value_type;
            REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, zero_ref, &zero_value_type));
            REQUIRE_OK(kefir_codegen_target_ir_add_produced_resource_aspects(mem, code, zero_ref.instr_ref));

            REQUIRE_OK(kefir_codegen_target_ir_code_replace_value(mem, code, zero_ref,
                classification.operands[0].output));
            REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, instr_ref));
            *replaced = true;
        } break;

        case 1:
            REQUIRE_OK(kefir_codegen_target_ir_code_replace_value(mem, code, instr->operation.parameters[classification.operands[1].read_index].direct.value_ref,
                classification.operands[0].output));
            REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, instr_ref));
            *replaced = true;
            break;

        default:
            for (kefir_uint32_t i = 1; i < sizeof(kefir_uint32_t) * CHAR_BIT - 1; i++) {
                if (instr->operation.parameters[classification.operands[2].read_index].immediate.int_immediate == (1 << i)) {
                    kefir_codegen_target_ir_value_ref_t shift_ref = {
                        .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
                    };

                    struct kefir_codegen_target_ir_instruction_metadata metadata = instr->metadata;
                    REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, instr->block_ref, instr_ref, 
                        &(struct kefir_codegen_target_ir_operation) {
                            .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(shl),
                            .parameters[0] = instr->operation.parameters[classification.operands[1].read_index],
                            .parameters[1] = {
                                .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
                                .immediate.int_immediate = i,
                                .immediate.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                            }
                        }, &metadata, &shift_ref.instr_ref));

                    const struct kefir_codegen_target_ir_value_type *output_value_type;
                    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, classification.operands[0].output,
                        &output_value_type));
                    struct kefir_codegen_target_ir_value_type shift_value_type = *output_value_type;
                    REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, shift_ref, &shift_value_type));

                    REQUIRE_OK(kefir_codegen_target_ir_add_produced_resource_aspects(mem, code, shift_ref.instr_ref));

                    REQUIRE_OK(kefir_codegen_target_ir_code_replace_value(mem, code, shift_ref,
                        classification.operands[0].output));
                    REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, instr_ref));
                    *replaced = true;
                    break;
                }
            }
            break;
    }

    return KEFIR_OK;
}
