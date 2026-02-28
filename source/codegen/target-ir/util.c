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

#include "kefir/codegen/target-ir/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_int64_t kefir_codegen_target_ir_sign_extend(kefir_int64_t value,
                                                  kefir_codegen_target_ir_operand_variant_t variant) {
    switch (variant) {
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT_HIGHER:
            return (kefir_int8_t) value;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT:
            return (kefir_int16_t) value;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT:
            return (kefir_int32_t) value;

        default:
            return value;
    }
}

kefir_int64_t kefir_codegen_target_ir_zero_extend(kefir_int64_t value,
                                                  kefir_codegen_target_ir_operand_variant_t variant) {
    switch (variant) {
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT_HIGHER:
            return (kefir_uint8_t) value;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT:
            return (kefir_uint16_t) value;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT:
            return (kefir_uint32_t) value;

        default:
            return value;
    }
}

kefir_result_t kefir_codegen_target_ir_add_produced_resource_aspects(
    struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
    kefir_codegen_target_ir_instruction_ref_t instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    const struct kefir_codegen_target_ir_instruction *instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));

    kefir_uint64_t produced_resources;
    REQUIRE_OK(
        code->klass->instruction_resources(instr->operation.opcode, &produced_resources, NULL, code->klass->payload));
    for (kefir_size_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT; i++) {
        if ((produced_resources >> i) & 1) {
            REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(
                mem, code,
                (kefir_codegen_target_ir_value_ref_t) {.instr_ref = instr_ref,
                                                       .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_RESOURCE(i)},
                &(struct kefir_codegen_target_ir_value_type) {
                    .kind = KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_RESOURCE,
                    .parameters.resource_id = i,
                    .variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT}));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_get_single_user(const struct kefir_codegen_target_ir_code *code,
                                                            kefir_codegen_target_ir_value_ref_t value_ref,
                                                            kefir_codegen_target_ir_instruction_ref_t *user_instr_ref) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    ASSIGN_PTR(user_instr_ref, KEFIR_ID_NONE);

    kefir_codegen_target_ir_instruction_ref_t single_user_instr_ref = KEFIR_ID_NONE;
    kefir_result_t res;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
    kefir_codegen_target_ir_value_ref_t used_value_ref;
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, value_ref.instr_ref, &use_instr_ref,
                                                     &used_value_ref);
         res == KEFIR_OK; res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
        const struct kefir_codegen_target_ir_instruction *user_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, use_instr_ref, &user_instr));
        if (user_instr->block_ref == KEFIR_ID_NONE) {
            continue;
        }

        if (use_instr_ref != single_user_instr_ref) {
            REQUIRE(used_value_ref.aspect == value_ref.aspect, KEFIR_OK);
            REQUIRE(single_user_instr_ref == KEFIR_ID_NONE, KEFIR_OK);
        }

        single_user_instr_ref = use_instr_ref;
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    ASSIGN_PTR(user_instr_ref, single_user_instr_ref);
    return KEFIR_OK;
}

kefir_bool_t kefir_codegen_target_ir_indirect_operand_same(const struct kefir_codegen_target_ir_operand *operand1,
                                                           const struct kefir_codegen_target_ir_operand *operand2) {
    REQUIRE(operand1 != NULL, false);
    REQUIRE(operand2 != NULL, false);

    REQUIRE(operand1->type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT, false);
    REQUIRE(operand2->type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT, false);

    REQUIRE(operand1->indirect.type == operand2->indirect.type, false);
    REQUIRE(operand1->indirect.index_type == operand2->indirect.index_type, false);
    REQUIRE(operand1->indirect.offset == operand2->indirect.offset, false);
    REQUIRE(operand1->indirect.variant == operand2->indirect.variant, false);
    switch (operand1->indirect.type) {
        case KEFIR_CODEGEN_TARGET_IR_INDIRECT_PHYSICAL_BASIS:
            REQUIRE(operand1->indirect.base.phreg == operand2->indirect.base.phreg, false);
            break;

        case KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS:
            REQUIRE(operand1->indirect.base.value_ref.instr_ref == operand2->indirect.base.value_ref.instr_ref, false);
            REQUIRE(operand1->indirect.base.value_ref.aspect == operand2->indirect.base.value_ref.aspect, false);
            break;

        case KEFIR_CODEGEN_TARGET_IR_INDIRECT_IMMEDIATE_BASIS:
            REQUIRE(operand1->indirect.base.immediate == operand2->indirect.base.immediate, false);
            break;

        case KEFIR_CODEGEN_TARGET_IR_INDIRECT_BLOCK_REF_BASIS:
            REQUIRE(operand1->indirect.base.block_ref == operand2->indirect.base.block_ref, false);
            break;

        case KEFIR_CODEGEN_TARGET_IR_INDIRECT_NATIVE_LABEL_BASIS:
            REQUIRE(operand1->indirect.base.native_id == operand2->indirect.base.native_id, false);
            break;

        case KEFIR_CODEGEN_TARGET_IR_INDIRECT_EXTERNAL_LABEL_BASIS:
            REQUIRE(operand1->indirect.base.external_type == operand2->indirect.base.external_type, false);
            REQUIRE(strcmp(operand1->indirect.base.external_label, operand2->indirect.base.external_label) == 0, false);
            break;

        case KEFIR_CODEGEN_TARGET_IR_INDIRECT_SPILL_AREA_BASIS:
            REQUIRE(operand1->indirect.base.spill_index == operand2->indirect.base.spill_index, false);
            break;
    }

    switch (operand1->indirect.index_type) {
        case KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_NONE:
            // Intentionally left blank
            break;

        case KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_PHYSICAL:
            REQUIRE(operand1->indirect.index.phreg == operand2->indirect.index.phreg, false);
            REQUIRE(operand1->indirect.index.scale == operand2->indirect.index.scale, false);
            break;

        case KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_VALUE_REF:
            REQUIRE(operand1->indirect.index.value_ref.instr_ref == operand2->indirect.index.value_ref.instr_ref,
                    false);
            REQUIRE(operand1->indirect.index.value_ref.aspect == operand2->indirect.index.value_ref.aspect, false);
            REQUIRE(operand1->indirect.index.scale == operand2->indirect.index.scale, false);
            break;
    }
    return true;
}
