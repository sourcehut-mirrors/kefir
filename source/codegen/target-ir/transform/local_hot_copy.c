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

#include "kefir/codegen/target-ir/transform.h"
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/codegen/target-ir/numbering.h"
#include "kefir/codegen/target-ir/regalloc.h"
#include "kefir/codegen/target-ir/tie.h"
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
                        if (instr->operation.parameters->indirect.index_type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_VALUE_REF &&
                            KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&instr->operation.parameters[i].indirect.index.value_ref) == KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref)) {
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

static kefir_result_t insert_cold_copy(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_block_ref_t block_ref, kefir_codegen_target_ir_instruction_ref_t instr_ref, kefir_codegen_target_ir_value_ref_t value_ref, struct kefir_hashset *local_hot_uses, kefir_codegen_target_ir_value_ref_t *copy_value_ref_ptr) {
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
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    ASSIGN_PTR(copy_value_ref_ptr, copy_value_ref);
    return KEFIR_OK;
}

static kefir_result_t insert_local_hot_copy(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_value_ref_t value_ref, kefir_codegen_target_ir_block_ref_t block_ref, kefir_codegen_target_ir_instruction_ref_t head_use_instr_ref, struct kefir_hashset *local_hot_uses) {
    kefir_codegen_target_ir_value_ref_t copy_value_ref = {
        .aspect = value_ref.aspect
    };
    const struct kefir_codegen_target_ir_value_type *value_type;        
    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, value_ref, &value_type));
    struct kefir_codegen_target_ir_value_type copy_value_type = *value_type;

    REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref, kefir_codegen_target_ir_code_control_prev(code, head_use_instr_ref), 
        &(struct kefir_codegen_target_ir_operation) {
            .opcode = code->klass->assign_opcode,
            .parameters[0] = {
                .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                .direct.value_ref = value_ref,
                .direct.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
            }
        }, NULL, &copy_value_ref.instr_ref));
    REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, copy_value_ref, &copy_value_type));

    kefir_result_t res;
    struct kefir_hashset_iterator use_iter;
    kefir_hashset_key_t use_entry;
    for (res = kefir_hashset_iter(local_hot_uses, &use_iter, &use_entry);
            res == KEFIR_OK; res = kefir_hashset_next(&use_iter, &use_entry)) {
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_value_in(mem, code, (kefir_codegen_target_ir_instruction_ref_t) use_entry, copy_value_ref, value_ref));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t collect_hot_use_regions_impl(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_liveness *liveness, const struct kefir_codegen_target_ir_regalloc *regalloc, kefir_codegen_target_ir_value_ref_t value_ref, struct kefir_hashtree *uses, struct kefir_hashset *local_hot_uses, kefir_bool_t *inserted) {
    REQUIRE_OK(kefir_hashtree_clean(mem, uses));
    kefir_result_t res;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
    kefir_codegen_target_ir_value_ref_t used_value_ref;
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, value_ref.instr_ref, &use_instr_ref, &used_value_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
        if (used_value_ref.aspect != value_ref.aspect) {
            continue;
        }

        kefir_size_t seq_idx;
        res = kefir_codegen_target_ir_numbering_instruction_seq_index(&liveness->numbering, use_instr_ref, &seq_idx);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }

        const struct kefir_codegen_target_ir_instruction *user_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, use_instr_ref, &user_instr));
        if (user_instr->operation.opcode == code->klass->upsilon_opcode ||
            user_instr->operation.opcode == code->klass->phi_opcode ||
            user_instr->operation.opcode == code->klass->inline_asm_opcode ||
            user_instr->operation.opcode == code->klass->touch_opcode ||
            user_instr->operation.opcode == code->klass->assign_opcode) {
            continue;
        }

        kefir_codegen_target_ir_regalloc_allocation_t allocation;
        res = kefir_codegen_target_ir_regalloc_get(regalloc, value_ref, &allocation);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }

        struct kefir_codegen_target_ir_tie_classification tie_classification;
        REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, use_instr_ref, &tie_classification));
        kefir_bool_t skip = false;
        for (kefir_size_t i = 0; !skip && i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
            if (tie_classification.operands[i].read_index == KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE ||
                tie_classification.operands[i].output.instr_ref == KEFIR_ID_NONE) {
                continue;
            }

            kefir_codegen_target_ir_regalloc_allocation_t output_allocation;
            kefir_result_t res = kefir_codegen_target_ir_regalloc_get(regalloc, tie_classification.operands[i].output, &output_allocation);
            if (res == KEFIR_NOT_FOUND) {
                continue;
            }
            REQUIRE_OK(res);

            if (user_instr->operation.parameters[tie_classification.operands[i].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                user_instr->operation.parameters[tie_classification.operands[i].read_index].direct.value_ref.instr_ref == value_ref.instr_ref &&
                user_instr->operation.parameters[tie_classification.operands[i].read_index].direct.value_ref.aspect == value_ref.aspect &&
                allocation == output_allocation) {
                skip = true;
            }
        }
        if (skip) {
            continue;
        }

        kefir_uint64_t key = (((kefir_uint64_t) user_instr->block_ref) << 32) | (kefir_uint32_t) seq_idx;
        res = kefir_hashtree_insert(mem, uses, (kefir_hashtree_key_t) key, (kefir_hashtree_value_t) use_instr_ref);
        if (res != KEFIR_ALREADY_EXISTS) {
            REQUIRE_OK(res);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    struct kefir_hashtree_node *iter_node;
    REQUIRE_OK(kefir_hashtree_min(uses, &iter_node));
    kefir_codegen_target_ir_block_ref_t current_block_ref = KEFIR_ID_NONE;
    kefir_codegen_target_ir_instruction_ref_t current_seq_head_instr_ref = KEFIR_ID_NONE;
    kefir_uint32_t current_seq_head_position = 0;
    kefir_uint32_t current_seq_tail_position = 0;
    REQUIRE_OK(kefir_hashset_clear(mem, local_hot_uses));
    for (; iter_node != NULL; iter_node = kefir_hashtree_next_node(uses, iter_node)) {
        ASSIGN_DECL_CAST(kefir_uint64_t, key, iter_node->key);
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_instruction_ref_t, use_instr_ref, iter_node->value);
        kefir_codegen_target_ir_block_ref_t use_block_ref = key >> 32;
        kefir_uint32_t use_seq_idx = key;

        if (current_block_ref == KEFIR_ID_NONE ||
            current_block_ref != use_block_ref ||
            use_seq_idx - current_seq_tail_position > regalloc->klass->transforms->hot_copy_locality) {
            if (current_seq_head_instr_ref != KEFIR_ID_NONE &&
                current_seq_head_position != current_seq_tail_position) {
                REQUIRE_OK(insert_local_hot_copy(mem, code, value_ref, current_block_ref, current_seq_head_instr_ref, local_hot_uses));
                ASSIGN_PTR(inserted, true);
            }

            REQUIRE_OK(kefir_hashset_clear(mem, local_hot_uses));
            current_block_ref = use_block_ref;
            current_seq_head_instr_ref = use_instr_ref;
            current_seq_head_position = use_seq_idx;
        }
        current_seq_tail_position = use_seq_idx;
        REQUIRE_OK(kefir_hashset_add(mem, local_hot_uses, (kefir_hashset_key_t) use_instr_ref));
    }

    if (current_seq_head_instr_ref != KEFIR_ID_NONE &&
        current_seq_head_position != current_seq_tail_position) {
        REQUIRE_OK(insert_local_hot_copy(mem, code, value_ref, current_block_ref, current_seq_head_instr_ref, local_hot_uses));
        ASSIGN_PTR(inserted, true);
    }
    return KEFIR_OK;
}

static kefir_result_t collect_hot_use_regions(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_liveness *liveness, const struct kefir_codegen_target_ir_regalloc *regalloc, kefir_codegen_target_ir_value_ref_t value_ref, struct kefir_hashset *local_hot_uses, kefir_bool_t *inserted) {
    struct kefir_hashtree uses;
    REQUIRE_OK(kefir_hashtree_init(&uses, &kefir_hashtree_uint_ops));

    kefir_result_t res = collect_hot_use_regions_impl(mem, code, liveness, regalloc, value_ref, &uses, local_hot_uses, inserted);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &uses);
        return res;
    });
    REQUIRE_OK(kefir_hashtree_free(mem, &uses));
    return KEFIR_OK;
}

static kefir_result_t do_insert_local_copies(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_liveness *liveness, struct kefir_codegen_target_ir_regalloc *regalloc, kefir_codegen_target_ir_block_ref_t block_ref,
    struct kefir_hashset *local_hot_uses) {

    for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(code, block_ref);
        instr_ref != KEFIR_ID_NONE;
        instr_ref = kefir_codegen_target_ir_code_control_next(code, instr_ref)) {
        const struct kefir_codegen_target_ir_instruction *instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));
        if (instr->operation.opcode == code->klass->placeholder_opcode) {
            continue;
        }

        if (instr->operation.opcode == code->klass->upsilon_opcode) {
            break;
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

            if (instr->operation.opcode != code->klass->phi_opcode) {
                REQUIRE_OK(collect_local_hot_uses(mem, code, regalloc->klass, instr_ref, value_ref, local_hot_uses));
                if (kefir_hashset_size(local_hot_uses) > 0) {
                    kefir_codegen_target_ir_value_ref_t copy_value_ref;
                    REQUIRE_OK(insert_cold_copy(mem, code, block_ref, instr_ref, value_ref, local_hot_uses, &copy_value_ref));
                    if (copy_value_ref.instr_ref != KEFIR_ID_NONE) {
                        REQUIRE_OK(collect_hot_use_regions(mem, code, liveness, regalloc, copy_value_ref, local_hot_uses, NULL));
                    }
                    break;
                }
            }
            kefir_bool_t inserted_hot_copy = false;
            REQUIRE_OK(collect_hot_use_regions(mem, code, liveness, regalloc, value_ref, local_hot_uses, &inserted_hot_copy));
            if (inserted_hot_copy) {
                break;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_transform_insert_local_hot_copy(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_liveness *liveness, const struct kefir_codegen_target_ir_interference *interference, struct kefir_codegen_target_ir_regalloc *regalloc) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));
    REQUIRE(interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR interference"));
    REQUIRE(regalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator"));

    struct kefir_hashset local_hot_uses;
    REQUIRE_OK(kefir_hashset_init(&local_hot_uses, &kefir_hashtable_uint_ops));
    kefir_result_t res = KEFIR_OK;
    for (kefir_size_t i = 0; res == KEFIR_OK && i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        REQUIRE_CHAIN(&res, do_insert_local_copies(mem, code, liveness, regalloc, block_ref, &local_hot_uses));
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &local_hot_uses);
        return res;
    });
    REQUIRE_OK(kefir_hashset_free(mem, &local_hot_uses));
    return KEFIR_OK;
}
