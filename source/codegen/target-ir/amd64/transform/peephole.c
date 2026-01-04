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

#include "kefir/codegen/target-ir/amd64/transform.h"
#include "kefir/codegen/target-ir/amd64/code.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t peephole_lea(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr) {
    REQUIRE(instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT, KEFIR_OK);
    struct kefir_codegen_target_ir_operation base_oper = instr->operation;
    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;

    kefir_result_t res;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
    kefir_codegen_target_ir_value_ref_t used_value_ref;
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, instr_ref, &use_instr_ref, &used_value_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
        if (used_value_ref.aspect != KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)) {
            continue;
        }
        
        const struct kefir_codegen_target_ir_instruction *user_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, use_instr_ref, &user_instr));
        if (user_instr->operation.opcode == code->klass->phi_opcode ||
            user_instr->operation.opcode == code->klass->inline_asm_opcode) {
            continue;
        }

        kefir_bool_t replace = false;
        struct kefir_codegen_target_ir_operation oper = user_instr->operation;
        for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
            if (oper.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT &&
                oper.parameters[i].indirect.type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS &&
                oper.parameters[i].indirect.base.value_ref.instr_ref == instr_ref &&
                oper.parameters[i].indirect.base.value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)) {
                replace = true;

                kefir_int64_t offset = oper.parameters[i].indirect.offset;
                kefir_codegen_target_ir_operand_variant_t variant = oper.parameters[i].indirect.variant;
                oper.parameters[i] = base_oper.parameters[0];
                oper.parameters->indirect.offset += offset;
                oper.parameters->indirect.variant = variant;
            }
        }

        if (replace) {
            kefir_codegen_target_ir_instruction_ref_t inserted_ref;
            struct kefir_codegen_target_ir_instruction_metadata metadata = user_instr->metadata;
            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, user_instr->block_ref,
                use_instr_ref, &oper, &metadata, &inserted_ref));

            struct kefir_codegen_target_ir_value_iterator value_iter;
            struct kefir_codegen_target_ir_value_ref value_ref;
            const struct kefir_codegen_target_ir_value_type *value_type;
            kefir_result_t res;
            for (res = kefir_codegen_target_ir_code_value_iter(code, &value_iter, use_instr_ref, &value_ref, &value_type);
                res == KEFIR_OK;
                res = kefir_codegen_target_ir_code_value_next(&value_iter, &value_ref, &value_type)) {
                REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, (kefir_codegen_target_ir_value_ref_t) {
                    .instr_ref = inserted_ref,
                    .aspect = value_ref.aspect
                }, value_type));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }

            REQUIRE_OK(kefir_codegen_target_ir_code_replace_instruction(mem, code, inserted_ref, use_instr_ref));
            REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, use_instr_ref));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_transform_peephole(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);

        for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(code, block_ref);
            instr_ref != KEFIR_ID_NONE;
            instr_ref = kefir_codegen_target_ir_code_control_next(code, instr_ref)) {
            const struct kefir_codegen_target_ir_instruction *instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));

            switch (instr->operation.opcode) {
                case KEFIR_TARGET_IR_AMD64_OPCODE(lea):
                    REQUIRE_OK(peephole_lea(mem, code, instr));
                    break;

                default:
                    // Intentionally left blank
                    break;
            }
        }
    }
    return KEFIR_OK;
}
