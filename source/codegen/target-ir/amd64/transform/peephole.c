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
#include "kefir/codegen/target-ir/transform.h"
#include "kefir/codegen/target-ir/tie.h"
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/codegen/target-ir/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_target_ir_amd64_peephole_const_operand(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t enable_lhs, kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr_ref, &classification));
    if (classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
        classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
        classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
        instr->operation.parameters[classification.operands[0].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
        !instr->operation.parameters[classification.operands[0].read_index].direct.tied &&
        instr->operation.parameters[classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF) {
        kefir_int64_t rhs_value = 0, lhs_value = 0;
        kefir_bool_t has_rhs_value = false, has_lhs_value = false;
        kefir_result_t res = kefir_codegen_target_ir_amd64_match_immediate(code, instr->operation.parameters[classification.operands[1].read_index].direct.value_ref, true, &rhs_value);
        if (res != KEFIR_NO_MATCH) {
            REQUIRE_OK(res);
            has_rhs_value = true;
        }
        res = kefir_codegen_target_ir_amd64_match_immediate(code, instr->operation.parameters[classification.operands[0].read_index].direct.value_ref, true, &lhs_value);
        if (res != KEFIR_NO_MATCH) {
            REQUIRE_OK(res);
            has_lhs_value = true;
        }
        if (has_rhs_value) {
            struct kefir_codegen_target_ir_operation oper = instr->operation;
            oper.parameters[classification.operands[1].read_index] = (struct kefir_codegen_target_ir_operand) {
                .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
                .immediate = {
                    .uint_immediate = rhs_value,
                    .variant = oper.parameters[classification.operands[1].read_index].direct.variant
                }
            };
            REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref, &oper, NULL));
            *replaced = true;
            return KEFIR_OK;
        } else if (enable_lhs && has_lhs_value) {
            struct kefir_codegen_target_ir_operation oper = instr->operation;
            oper.parameters[classification.operands[0].read_index] = instr->operation.parameters[classification.operands[1].read_index],
            oper.parameters[classification.operands[1].read_index] = (struct kefir_codegen_target_ir_operand) {
                .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
                .immediate = {
                    .uint_immediate = lhs_value,
                    .variant = instr->operation.parameters[classification.operands[0].read_index].direct.variant
                }
            };
            REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref, &oper, NULL));
            *replaced = true;
            return KEFIR_OK;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_indirect(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    kefir_bool_t replace = false;
    struct kefir_codegen_target_ir_operation oper = instr->operation;
    for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
        if (oper.parameters[i].type != KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT ||
            oper.parameters[i].indirect.type != KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS ||
            oper.parameters[i].indirect.base.value_ref.aspect != KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)) {
            continue;
        }

        const struct kefir_codegen_target_ir_instruction *base_instr, *index_instr = NULL;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, oper.parameters[i].indirect.base.value_ref.instr_ref, &base_instr));
        if (oper.parameters[i].indirect.index_type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_VALUE_REF) {
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, oper.parameters[i].indirect.index.value_ref.instr_ref, &index_instr));
        }

        if (base_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(lea) &&
            base_instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT &&
            base_instr->operation.parameters[0].indirect.type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS &&
            (base_instr->operation.parameters[0].indirect.index_type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_NONE ||
            oper.parameters[i].indirect.index_type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_NONE)) {
            const struct kefir_codegen_target_ir_value_type *value_type;
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, base_instr->operation.parameters[0].indirect.base.value_ref, &value_type));
            if (value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
                continue;
            }

            if (base_instr->operation.parameters[0].indirect.index_type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_VALUE_REF) {
                REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, base_instr->operation.parameters[0].indirect.index.value_ref, &value_type));
                if (value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
                    continue;
                }
            }

            kefir_codegen_target_ir_indirect_index_type_t index_type = oper.parameters[i].indirect.index_type;
            struct kefir_codegen_target_ir_operand_indirect_index index = oper.parameters[i].indirect.index;
            kefir_int64_t offset = oper.parameters[i].indirect.offset;
            kefir_codegen_target_ir_operand_variant_t variant = oper.parameters[i].indirect.variant;

            oper.parameters[i] = base_instr->operation.parameters[0];
            if (index_type != KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_NONE) {
                oper.parameters[i].indirect.index_type = index_type;
                oper.parameters[i].indirect.index = index;
            }
            oper.parameters[i].indirect.offset += offset;
            oper.parameters[i].indirect.variant = variant;
            replace = true;
        } else if (oper.parameters[i].indirect.index_type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_NONE &&
            base_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(add) &&
            base_instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            !base_instr->operation.parameters[0].segment.present &&
            (base_instr->operation.parameters[0].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||
            base_instr->operation.parameters[0].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT) &&
            base_instr->operation.parameters[1].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            !base_instr->operation.parameters[1].segment.present &&
            (base_instr->operation.parameters[1].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||
            base_instr->operation.parameters[1].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT)) {
            const struct kefir_codegen_target_ir_value_type *value_type;
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, base_instr->operation.parameters[0].direct.value_ref, &value_type));
            if (value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
                continue;
            }
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, base_instr->operation.parameters[1].direct.value_ref, &value_type));
            if (value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
                continue;
            }

            oper.parameters[i] = (struct kefir_codegen_target_ir_operand) {
                .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT,
                .indirect = {
                    .type = KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS,
                    .index_type = KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_VALUE_REF,
                    .base.value_ref = base_instr->operation.parameters[0].direct.value_ref,
                    .index.value_ref = base_instr->operation.parameters[1].direct.value_ref,
                    .index.scale = 1,
                    .offset = oper.parameters[i].indirect.offset,
                    .variant = oper.parameters[i].indirect.variant
                }
            };
            replace = true;
        } else if (index_instr != NULL && index_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(shl) &&
            index_instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            (index_instr->operation.parameters[0].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||
            index_instr->operation.parameters[0].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT ||
            index_instr->operation.parameters[0].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT) &&
            index_instr->operation.parameters[1].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER &&
            (1 << index_instr->operation.parameters[1].immediate.int_immediate) * oper.parameters[i].indirect.index.scale <= 8) {
            oper.parameters[i].indirect.index.value_ref = index_instr->operation.parameters[0].direct.value_ref;
            oper.parameters[i].indirect.index.scale = (1 << index_instr->operation.parameters[1].immediate.int_immediate) * oper.parameters[i].indirect.index.scale;
            replace = true;
        } else if ((base_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(add) || base_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(sub)) &&
            base_instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            !base_instr->operation.parameters[0].segment.present &&
            (base_instr->operation.parameters[0].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||
            base_instr->operation.parameters[0].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT) &&
            base_instr->operation.parameters[1].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER &&
            !base_instr->operation.parameters[1].segment.present) {
            const struct kefir_codegen_target_ir_value_type *value_type;
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, base_instr->operation.parameters[0].direct.value_ref, &value_type));
            if (value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
                continue;
            }

            kefir_int64_t value = kefir_codegen_target_ir_sign_extend(base_instr->operation.parameters[1].immediate.int_immediate, base_instr->operation.parameters[1].immediate.variant);
            if (base_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(sub)) {
                value *= -1;
            }

            oper.parameters[i].indirect.base.value_ref = base_instr->operation.parameters[0].direct.value_ref;
            oper.parameters[i].indirect.offset += value;
            replace = true;
        } else if (index_instr != NULL &&
            (index_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(add) || index_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(sub)) &&
            index_instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            !index_instr->operation.parameters[0].segment.present &&
            (index_instr->operation.parameters[0].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||
            index_instr->operation.parameters[0].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT) &&
            index_instr->operation.parameters[1].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER &&
            !index_instr->operation.parameters[1].segment.present) {
            const struct kefir_codegen_target_ir_value_type *value_type;
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, index_instr->operation.parameters[0].direct.value_ref, &value_type));
            if (value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
                continue;
            }

            kefir_int64_t value = kefir_codegen_target_ir_sign_extend(index_instr->operation.parameters[1].immediate.int_immediate, index_instr->operation.parameters[1].immediate.variant);
            if (index_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(sub)) {
                value *= -1;
            }

            oper.parameters[i].indirect.index.value_ref = index_instr->operation.parameters[0].direct.value_ref;
            oper.parameters[i].indirect.offset += value * oper.parameters[i].indirect.index.scale;
            replace = true;
        }
    }

    if (replace) {
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr->instr_ref, &oper, NULL));
        *replaced = true;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_untie(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    kefir_bool_t replace = false;
    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
    struct kefir_codegen_target_ir_operation oper = instr->operation;
    for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
        if (classification.classification.operands[i].class != KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE ||
            classification.operands[i].read_index == KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE ||
            oper.parameters[classification.operands[i].read_index].type != KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF ||
            !oper.parameters[classification.operands[i].read_index].direct.tied) {
            continue;
        }

        kefir_codegen_target_ir_value_ref_t output_value_ref = classification.operands[i].output;
        if (!KEFIR_CODEGEN_TARGET_IR_VALUE_IS_DIRECT_OUTPUT(output_value_ref.aspect)) {
            continue;
        }

        const struct kefir_codegen_target_ir_value_type *output_value_type;
        REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, output_value_ref, &output_value_type));
        if (output_value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
            continue;
        }

        kefir_bool_t ext_uses = false;
        if (output_value_type->variant != KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT &&
            output_value_type->variant != KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT &&
            (output_value_type->variant != KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT || output_value_type->kind != KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE)) {
            kefir_result_t res;
            struct kefir_codegen_target_ir_use_iterator use_iter;
            kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
            kefir_codegen_target_ir_value_ref_t used_value_ref;
            for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, output_value_ref.instr_ref, &use_instr_ref, &used_value_ref);
                res == KEFIR_OK && !ext_uses;
                res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
                if (used_value_ref.aspect != output_value_ref.aspect) {
                    continue;
                }

                const struct kefir_codegen_target_ir_instruction *user_instr;
                REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, use_instr_ref, &user_instr));
                if (user_instr->operation.opcode == code->klass->phi_opcode ||
                    user_instr->operation.opcode == code->klass->inline_asm_opcode ||
                    user_instr->block_ref == KEFIR_ID_NONE) {
                    ext_uses = true;
                    continue;
                }
                
                for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS && !ext_uses; i++) {
                    if (user_instr->operation.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                        user_instr->operation.parameters[i].direct.value_ref.instr_ref == output_value_ref.instr_ref &&
                        user_instr->operation.parameters[i].direct.value_ref.aspect == output_value_ref.aspect &&
                        (user_instr->operation.parameters[i].direct.tied ||
                        !(user_instr->operation.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT ||
                        user_instr->operation.parameters[i].direct.variant == output_value_type->variant ||
                        (user_instr->operation.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT &&
                        output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT) ||
                        (user_instr->operation.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT &&
                        output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT) ||
                        (user_instr->operation.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT &&
                        output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT)))) {
                        ext_uses = true;
                    } else if (user_instr->operation.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT &&
                        user_instr->operation.parameters[i].indirect.type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS &&
                        user_instr->operation.parameters[i].indirect.index_type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_NONE &&
                        user_instr->operation.parameters[i].indirect.base.value_ref.instr_ref == output_value_ref.instr_ref &&
                        user_instr->operation.parameters[i].indirect.base.value_ref.aspect == output_value_ref.aspect) {
                        ext_uses = true;
                    }
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }

        if (!ext_uses) {
            oper.parameters[classification.operands[i].read_index].direct.tied = false;
            replace = true;
        }
    }

    if (replace) {
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref, &oper, NULL));
        *replaced = true;
    }

    return KEFIR_OK;
}

static kefir_result_t do_peephole(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, struct kefir_codegen_target_ir_control_flow *control_flow) {
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_build(mem, control_flow));
    kefir_bool_t reached_fixpoint = false, post_cleanup = false;

    for (; !reached_fixpoint;) {
        kefir_bool_t replaced = false;
        for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
            kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);

            for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(code, block_ref);
                instr_ref != KEFIR_ID_NONE;
                instr_ref = kefir_codegen_target_ir_code_control_next(code, instr_ref)) {
                const struct kefir_codegen_target_ir_instruction *instr;
                REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));

                kefir_bool_t instr_replaced = false;
                if (instr->operation.opcode != code->klass->upsilon_opcode &&
                    instr->operation.opcode != code->klass->phi_opcode &&
                    instr->operation.opcode != code->klass->inline_asm_opcode &&
                    instr->operation.opcode != code->klass->placeholder_opcode) {
                    REQUIRE_OK(peephole_untie(mem, code, instr, &instr_replaced));
                    if (instr_replaced) {
                        replaced = true;
                        continue;
                    }

                    REQUIRE_OK(peephole_indirect(mem, code, instr, &instr_replaced));
                    if (instr_replaced) {
                        replaced = true;
                        continue;
                    }
                }

                switch (instr->operation.opcode) {
                    case KEFIR_TARGET_IR_AMD64_OPCODE(add):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(sub):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_add(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(adc):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_adc(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(sbb):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_sbb(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(imul):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_imul(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(imul3):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_imul3(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(xor):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_xor(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(or):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_or(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(movzx):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(movsx):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_movx(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(mov):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_mov(mem, code, instr, &instr_replaced));
                        break;

#define SETCC_CASE(_setcc) \
                    case KEFIR_TARGET_IR_AMD64_OPCODE(_setcc): \
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_##_setcc(mem, code, instr, &instr_replaced)); \
                        if (!instr_replaced) { \
                            REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_setcc(mem, code, instr, &instr_replaced)); \
                        } \
                        break \
                    
                    SETCC_CASE(sete);
                    SETCC_CASE(setne);
                    SETCC_CASE(setp);
                    SETCC_CASE(setnp);
                    SETCC_CASE(setc);
                    SETCC_CASE(setnc);
                    SETCC_CASE(seto);
                    SETCC_CASE(setno);
                    SETCC_CASE(sets);
                    SETCC_CASE(setns);
                    SETCC_CASE(setb);
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setae):
                    SETCC_CASE(setnb);
                    SETCC_CASE(setl);
                    SETCC_CASE(setge);
                    SETCC_CASE(setg);
                    SETCC_CASE(setle);
                    SETCC_CASE(seta);
                    SETCC_CASE(setbe);
#undef SETCC_CASE

                    case KEFIR_TARGET_IR_AMD64_OPCODE(test):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_test(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(and):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_and(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(div):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_div(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(idiv):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_idiv(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(shl):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_shl(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(shr):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_shr(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(sar):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_sar(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(shld):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(shrd):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_shxd(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(rol):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_rol(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(btc):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_btc(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(cmp):
                        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_cmp(mem, code, instr, &instr_replaced));
                        break;

                    default:
                        // Intentionally left blank
                        break;
                }

                replaced = replaced || instr_replaced;
            }
        }

        if (!replaced && !post_cleanup) {
            REQUIRE_OK(kefir_codegen_target_ir_transform_copy_elision(mem, code));
            REQUIRE_OK(kefir_codegen_target_ir_amd64_transform_dead_code_elimination(mem, code));
            REQUIRE_OK(kefir_codegen_target_ir_transform_phi_removal(mem, code, true));
            post_cleanup = true;
            replaced = true;
        } else {
            post_cleanup = false;
        }

        reached_fixpoint = !replaced;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_transform_peephole(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    struct kefir_codegen_target_ir_control_flow control_flow;
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_init(&control_flow, code));
    kefir_result_t res = do_peephole(mem, code, &control_flow);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_control_flow_free(mem, &control_flow);
        return res;
    });
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_free(mem, &control_flow));
    return KEFIR_OK;
}
