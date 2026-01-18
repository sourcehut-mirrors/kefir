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

#include "kefir/codegen/target-ir/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_int64_t kefir_codegen_target_ir_sign_extend(kefir_int64_t value, kefir_codegen_target_ir_operand_variant_t variant) {
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

kefir_result_t kefir_codegen_target_ir_add_produced_resource_aspects(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    const struct kefir_codegen_target_ir_instruction *instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));

    kefir_uint64_t produced_resources;
    REQUIRE_OK(code->klass->instruction_resources(instr->operation.opcode, &produced_resources, NULL, code->klass->payload));
    for (kefir_size_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT; i++) {
        if ((produced_resources >> i) & 1) {
            REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, (kefir_codegen_target_ir_value_ref_t) {
                .instr_ref = instr_ref,
                .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_RESOURCE(i)
            }, &(struct kefir_codegen_target_ir_value_type) {
                .kind = KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_RESOURCE,
                .parameters.resource_id = i,
                .variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
            }));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_get_single_user(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_value_ref_t value_ref, kefir_codegen_target_ir_instruction_ref_t *user_instr_ref) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    ASSIGN_PTR(user_instr_ref, KEFIR_ID_NONE);
    
    kefir_codegen_target_ir_instruction_ref_t single_user_instr_ref = KEFIR_ID_NONE;
    kefir_result_t res;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
    kefir_codegen_target_ir_value_ref_t used_value_ref;
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, value_ref.instr_ref, &use_instr_ref, &used_value_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
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
