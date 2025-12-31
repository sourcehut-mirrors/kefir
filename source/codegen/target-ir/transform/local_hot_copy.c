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
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/codegen/target-ir/numbering.h"
#include "kefir/codegen/target-ir/regalloc.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t collect_local_hot_uses(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_regalloc_class *regalloc_klass, kefir_codegen_target_ir_instruction_ref_t instr_ref, kefir_codegen_target_ir_value_ref_t value_ref, struct kefir_hashset *local_hot_uses) {
    REQUIRE_OK(kefir_hashset_clear(mem, local_hot_uses));

    kefir_size_t lookahead = regalloc_klass->transforms->hot_copy_locality;
    for (instr_ref = kefir_codegen_target_ir_code_control_next(code, instr_ref);
        instr_ref != KEFIR_ID_NONE && lookahead > 0;
        instr_ref = kefir_codegen_target_ir_code_control_next(code, instr_ref), lookahead--) {
        const struct kefir_codegen_target_ir_instruction *instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));
        if (instr->operation.opcode == code->klass->upsilon_opcode) {
            break;
        }

        if (instr->operation.opcode == code->klass->phi_opcode) {
            // Intentionally left blank
        } else if (instr->operation.opcode == code->klass->inline_asm_opcode) {
            // Intentionally left blank
        } else {
            for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
                switch (instr->operation.parameters[i].type) {
                    case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE:
                    case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER:
                    case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UINTEGER:
                    case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_PHYSICAL_REGISTER:
                    case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_BLOCK_REF:
                    case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_NATIVE:
                    case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_EXTERNAL:
                    case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF:
                    case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NATIVE_LABEL:
                    case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_EXTERNAL_LABEL:
                    case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_X87:
                    case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UPSILON:
                        // Intentionally left blank
                        break;

                    case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF:
                        if (KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&instr->operation.parameters[i].direct.value_ref) == KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref)) {
                            REQUIRE_OK(kefir_hashset_add(mem, local_hot_uses, (kefir_hashset_key_t) instr_ref));
                            lookahead = regalloc_klass->transforms->hot_copy_locality;
                        }
                        break;

                    case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT:
                        if (instr->operation.parameters->indirect.type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS &&
                            KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&instr->operation.parameters[i].indirect.base.value_ref) == KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref)) {
                            REQUIRE_OK(kefir_hashset_add(mem, local_hot_uses, (kefir_hashset_key_t) instr_ref));
                            lookahead = regalloc_klass->transforms->hot_copy_locality;
                        }
                        break;
                }
            }
        }
    }

    return KEFIR_OK;
}

static kefir_result_t insert_copy(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_block_ref_t block_ref, kefir_codegen_target_ir_instruction_ref_t instr_ref, kefir_codegen_target_ir_value_ref_t value_ref, struct kefir_hashset *local_hot_uses) {
    kefir_result_t res;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
    kefir_codegen_target_ir_value_ref_t use_value_ref;
    kefir_codegen_target_ir_value_ref_t copy_value_ref = {
        .instr_ref = KEFIR_ID_NONE,
        .aspect = value_ref.aspect
    };
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, instr_ref, &use_instr_ref, &use_value_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &use_value_ref)) {
        if (use_value_ref.aspect != value_ref.aspect ||
            kefir_hashset_has(local_hot_uses, (kefir_hashset_key_t) use_instr_ref)) {
            continue;
        }

        const struct kefir_codegen_target_ir_instruction *use_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, use_instr_ref, &use_instr));
        if (use_instr->operation.opcode == code->klass->upsilon_opcode &&
            KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&use_instr->operation.parameters->upsilon_ref) == KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref)) {
            continue;
        }
        
        if (copy_value_ref.instr_ref == KEFIR_ID_NONE) {
            const struct kefir_codegen_target_ir_value_type *value_type;        
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, value_ref, &value_type));
            struct kefir_codegen_target_ir_value_type copy_value_type = *value_type;

            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref, instr_ref, 
                &(struct kefir_codegen_target_ir_operation) {
                    .opcode = code->klass->assign_opcode,
                    .parameters[0] = {
                        .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                        .direct.value_ref = value_ref,
                        .direct.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                    }
                }, NULL, &copy_value_ref.instr_ref));
            REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, copy_value_ref, &copy_value_type));
        }

        REQUIRE_OK(kefir_codegen_target_ir_code_replace_value_in(mem, code, use_instr_ref, copy_value_ref, value_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t do_insert_local_copies(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_regalloc *regalloc, kefir_codegen_target_ir_block_ref_t block_ref,
    struct kefir_hashset *local_hot_uses) {

    for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(code, block_ref);
        instr_ref != KEFIR_ID_NONE;
        instr_ref = kefir_codegen_target_ir_code_control_next(code, instr_ref)) {
        const struct kefir_codegen_target_ir_instruction *instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));
        if (instr->operation.opcode == code->klass->phi_opcode ||
            instr->operation.opcode == code->klass->placeholder_opcode) {
            continue;
        }

        if (instr->operation.opcode == code->klass->upsilon_opcode) {
            break;;
        }

        kefir_result_t res;
        struct kefir_codegen_target_ir_value_iterator value_iter;
        struct kefir_codegen_target_ir_value_ref value_ref;
        const struct kefir_codegen_target_ir_value_type *value_type;
        for (res = kefir_codegen_target_ir_code_value_iter(code, &value_iter, instr_ref, &value_ref, &value_type);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_value_next(&value_iter, &value_ref, NULL)) {
            const struct kefir_codegen_target_ir_value_type *value_type;        
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, value_ref, &value_type));
            if (value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT ||
                (value_type->kind != KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE &&
                value_type->kind != KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT)) {
                continue;
            }

            kefir_codegen_target_ir_regalloc_allocation_t allocaton;
            res = kefir_codegen_target_ir_regalloc_get(regalloc, value_ref, &allocaton);
            if (res == KEFIR_NOT_FOUND) {
                continue;
            }
            REQUIRE_OK(res);

            kefir_bool_t is_evictable;
            REQUIRE_OK(regalloc->klass->is_evictable(allocaton, &is_evictable, regalloc->klass->payload));
            if (is_evictable) {
                continue;
            }

            REQUIRE_OK(collect_local_hot_uses(mem, code, regalloc->klass, instr_ref, value_ref, local_hot_uses));
            if (kefir_hashset_size(local_hot_uses) > 0) {
                REQUIRE_OK(insert_copy(mem, code, block_ref, instr_ref, value_ref, local_hot_uses));
                break;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_transform_insert_local_hot_copy(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_regalloc *regalloc) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(regalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator"));

    struct kefir_hashset local_hot_uses;
    REQUIRE_OK(kefir_hashset_init(&local_hot_uses, &kefir_hashtable_uint_ops));
    kefir_result_t res = KEFIR_OK;
    for (kefir_size_t i = 0; res == KEFIR_OK && i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        REQUIRE_CHAIN(&res, do_insert_local_copies(mem, code, regalloc, block_ref, &local_hot_uses));
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &local_hot_uses);
        return res;
    });
    REQUIRE_OK(kefir_hashset_free(mem, &local_hot_uses));
    return KEFIR_OK;
}
