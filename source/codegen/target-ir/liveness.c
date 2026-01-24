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

#include "kefir/codegen/target-ir/liveness.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t free_value_liveness(struct kefir_mem *mem, struct kefir_hashtable *table, kefir_hashtable_key_t key, kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_value_liveness *, liveness,
        value);
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR value liveness"));

    REQUIRE_OK(kefir_hashtable_free(mem, &liveness->per_block));
    KEFIR_FREE(mem, liveness);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_liveness_init(struct kefir_codegen_target_ir_liveness *liveness) {
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR liveness"));

    liveness->code = NULL;
    liveness->blocks = NULL;
    REQUIRE_OK(kefir_hashtable_init(&liveness->values, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&liveness->values, free_value_liveness, NULL));
    REQUIRE_OK(kefir_codegen_target_ir_numbering_init(&liveness->numbering));
    return KEFIR_OK;    
}

static kefir_result_t free_blocks(struct kefir_mem *mem, struct kefir_codegen_target_ir_liveness *liveness) {
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(liveness->code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(liveness->code, i);
        KEFIR_FREE(mem, liveness->blocks[block_ref].live_in.content);
        KEFIR_FREE(mem, liveness->blocks[block_ref].live_out.content);
        if (liveness->blocks[block_ref].value_liveness_ranges_ready) {
            for (kefir_size_t j = 0; j < liveness->blocks[block_ref].value_liveness_ranges.length; j++) {
                REQUIRE_OK(kefir_hashset_free(mem, &liveness->blocks[block_ref].value_liveness_ranges.indices[j].begin_liveness));
                REQUIRE_OK(kefir_hashset_free(mem, &liveness->blocks[block_ref].value_liveness_ranges.indices[j].end_liveness));
            }
            REQUIRE_OK(kefir_hashset_free(mem, &liveness->blocks[block_ref].value_liveness_ranges.null_index.begin_liveness));
            REQUIRE_OK(kefir_hashset_free(mem, &liveness->blocks[block_ref].value_liveness_ranges.null_index.end_liveness));
            KEFIR_FREE(mem, liveness->blocks[block_ref].value_liveness_ranges.indices);
        }
    }
    KEFIR_FREE(mem, liveness->blocks);
    liveness->blocks = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_liveness_free(struct kefir_mem *mem, struct kefir_codegen_target_ir_liveness *liveness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));

    REQUIRE_OK(kefir_hashtable_free(mem, &liveness->values));
    if (liveness->blocks != NULL) {
        free_blocks(mem, liveness);
    }
    REQUIRE_OK(kefir_codegen_target_ir_numbering_free(mem, &liveness->numbering));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_liveness_reset(struct kefir_mem *mem, struct kefir_codegen_target_ir_liveness *liveness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));

    REQUIRE_OK(kefir_hashtable_clear(mem, &liveness->values));
    if (liveness->blocks != NULL) {
        free_blocks(mem, liveness);
    }
    REQUIRE_OK(kefir_codegen_target_ir_numbering_reset(mem, &liveness->numbering));
    return KEFIR_OK;
}

static kefir_result_t add_to_entry(struct kefir_mem *mem, struct kefir_codegen_target_ir_liveness_entry *entry, kefir_codegen_target_ir_value_ref_t value_ref) {
    if (entry->length > 0 &&
        entry->content[entry->length - 1].instr_ref == value_ref.instr_ref &&
        entry->content[entry->length - 1].aspect == value_ref.aspect) {
        return KEFIR_OK;
    }
    if (entry->length >= entry->capacity) {
        kefir_size_t new_capacity = MAX(entry->capacity * 9 / 8, 128);
        kefir_codegen_target_ir_value_ref_t *new_content = KEFIR_REALLOC(mem, entry->content, sizeof(kefir_codegen_target_ir_value_ref_t) * new_capacity);
        REQUIRE(new_content != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR liveness entry"));
        entry->content = new_content;
        entry->capacity = new_capacity;
    }

    entry->content[entry->length++] = value_ref;
    return KEFIR_OK;
}

static kefir_bool_t is_scheduled_earlier(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr1_ref, kefir_codegen_target_ir_instruction_ref_t instr2_ref) {
    for (; instr2_ref != KEFIR_ID_NONE; instr2_ref = kefir_codegen_target_ir_code_control_prev(code, instr2_ref)) {
        if (instr2_ref == instr1_ref) {
            return true;
        }
    }
    return false;
}

kefir_result_t kefir_codegen_target_ir_liveness_value_at(const struct kefir_codegen_target_ir_liveness *liveness,
    kefir_codegen_target_ir_value_ref_t value_ref,
    kefir_codegen_target_ir_block_ref_t block_ref,
    kefir_codegen_target_ir_liveness_class_t klass,
    kefir_codegen_target_ir_instruction_ref_t *begin_ref_ptr,
    kefir_codegen_target_ir_instruction_ref_t *end_ref_ptr) {
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&liveness->values, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find target IR value liveness");
    }
    REQUIRE_OK(res);
    ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_value_liveness *, value_liveness,
        table_value);

    kefir_hashtable_key_t per_block_key = (((kefir_uint64_t) block_ref) << 32) | (kefir_uint32_t) klass;
    res = kefir_hashtable_at(&value_liveness->per_block, (kefir_hashtable_key_t) per_block_key, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find target IR value liveness");
    }
    REQUIRE_OK(res);
    
    ASSIGN_PTR(begin_ref_ptr, table_value >> 32);
    ASSIGN_PTR(end_ref_ptr, (kefir_uint32_t) table_value);
    return KEFIR_OK;
}

static kefir_result_t add_value_liveness(struct kefir_mem *mem, struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_value_ref_t value_ref, kefir_codegen_target_ir_block_ref_t block_ref, enum kefir_codegen_target_ir_liveness_class klass, kefir_codegen_target_ir_instruction_ref_t begin_ref, kefir_codegen_target_ir_instruction_ref_t end_ref) {
    kefir_hashtable_key_t key = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref);

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&liveness->values, (kefir_hashtable_key_t) key, &table_value);
    struct kefir_codegen_target_ir_value_liveness *value_liveness = NULL;
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        value_liveness = (struct kefir_codegen_target_ir_value_liveness *) table_value;
    } else {
        value_liveness = KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_target_ir_value_liveness));
        REQUIRE(value_liveness != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR value liveness"));
        res = kefir_hashtable_init(&value_liveness->per_block, &kefir_hashtable_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashtable_insert(mem, &liveness->values, key, (kefir_hashtable_value_t) value_liveness));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, value_liveness);
            return res;
        });
    }

    kefir_hashtable_key_t per_block_key = (((kefir_uint64_t) block_ref) << 32) | (kefir_uint32_t) klass;
    kefir_hashtable_value_t *table_value_ptr;
    res = kefir_hashtable_at_mut(&value_liveness->per_block, (kefir_hashtable_key_t) per_block_key, &table_value_ptr);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        kefir_codegen_target_ir_instruction_ref_t current_begin_ref = ((kefir_uint64_t) *table_value_ptr) >> 32,
                                                  current_end_ref = (kefir_uint32_t) *table_value_ptr;
        if (begin_ref == KEFIR_ID_NONE || (current_begin_ref != KEFIR_ID_NONE && is_scheduled_earlier(liveness->code, begin_ref, current_begin_ref))) {
            current_begin_ref = begin_ref;
        }
        if (end_ref == KEFIR_ID_NONE || (current_end_ref != KEFIR_ID_NONE && is_scheduled_earlier(liveness->code, current_end_ref, end_ref))) {
            current_end_ref = end_ref;
        }
        *table_value_ptr = (((kefir_uint64_t) current_begin_ref) << 32) | (kefir_uint32_t) current_end_ref;
    } else {
        table_value = (((kefir_uint64_t) begin_ref) << 32) | (kefir_uint32_t) end_ref;
        REQUIRE_OK(kefir_hashtable_insert(mem, &value_liveness->per_block, (kefir_hashtable_key_t) per_block_key, table_value));
    }
    return KEFIR_OK;
}

static kefir_result_t has_upsilon_for(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr_ref, kefir_codegen_target_ir_block_ref_t block_ref, kefir_bool_t *has_upsilon) {
    *has_upsilon = false;

    kefir_codegen_target_ir_instruction_ref_t iter_ref = kefir_codegen_target_ir_code_block_control_tail(code, block_ref);
    iter_ref = kefir_codegen_target_ir_code_control_prev(code, iter_ref);
    for (; iter_ref != KEFIR_ID_NONE && !*has_upsilon; iter_ref = kefir_codegen_target_ir_code_control_prev(code, iter_ref)) {
        const struct kefir_codegen_target_ir_instruction *iter_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, iter_ref, &iter_instr));
        if (iter_instr->operation.opcode != code->klass->upsilon_opcode &&
            iter_instr->operation.opcode != code->klass->assign_opcode) {
            break;
        }

        if (iter_instr->operation.opcode == code->klass->upsilon_opcode &&
            iter_instr->operation.parameters[0].upsilon_ref.instr_ref == instr_ref) {
            *has_upsilon = true;
        }
    }

    return KEFIR_OK;
}

static kefir_result_t propagate_instr_liveness(struct kefir_mem *mem, const struct kefir_codegen_target_ir_control_flow *control_flow, struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_value_ref_t value_ref, struct kefir_list *queue, kefir_uint8_t *visited_map) {
    REQUIRE_OK(kefir_list_clear(mem, queue));
    memset(visited_map, 0, sizeof(kefir_uint8_t) * kefir_codegen_target_ir_code_block_count(control_flow->code));
    const struct kefir_codegen_target_ir_instruction *instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(control_flow->code, value_ref.instr_ref, &instr));
    if (instr->operation.opcode == control_flow->code->klass->placeholder_opcode) {
        const struct kefir_codegen_target_ir_value_type *value_type;
        REQUIRE_OK(kefir_codegen_target_ir_code_value_props(control_flow->code, value_ref, &value_type));
        if (value_type->kind != KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_SPILL_SPACE && value_type->constraint.type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
            return KEFIR_OK;
        }
    }

    if (instr->operation.opcode == control_flow->code->klass->phi_opcode) {
        REQUIRE_OK(add_value_liveness(mem, liveness, value_ref, instr->block_ref, KEFIR_CODEGEN_TARGET_IR_LIVENESS_NORMAL, KEFIR_ID_NONE, instr->instr_ref));
    }

    kefir_result_t res;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t user_instr_ref;
    kefir_codegen_target_ir_value_ref_t used_value;
    for (res = kefir_codegen_target_ir_code_use_iter(control_flow->code, &use_iter, value_ref.instr_ref, &user_instr_ref, &used_value);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_use_next(&use_iter, &user_instr_ref, &used_value)) {
        if (used_value.aspect != value_ref.aspect) {
            continue;
        }

        const struct kefir_codegen_target_ir_instruction *user_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(control_flow->code, user_instr_ref, &user_instr));
        if (!kefir_codegen_target_ir_control_flow_is_reachable(control_flow, user_instr->block_ref)) {
            continue;
        }
        if (user_instr->operation.opcode == control_flow->code->klass->upsilon_opcode) {
            if (KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&user_instr->operation.parameters[0].upsilon_ref) == KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref)) {
                REQUIRE(kefir_hashset_has(&control_flow->blocks[user_instr->block_ref].successors, (kefir_hashset_key_t) instr->block_ref),
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected upsilon instruction to be placed in the predecessor block of corresponding phi instruction"));
                REQUIRE_OK(add_value_liveness(mem, liveness, value_ref, user_instr->block_ref, KEFIR_CODEGEN_TARGET_IR_LIVENESS_UPSILON, user_instr->instr_ref, KEFIR_ID_NONE));
                REQUIRE_OK(add_to_entry(mem, &liveness->blocks[user_instr->block_ref].live_out, value_ref));
            }

            if (KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&user_instr->operation.parameters[1].direct.value_ref) == KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref)) {
                if (instr->block_ref != user_instr->block_ref) {
                    REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) user_instr->block_ref));
                    REQUIRE_OK(add_to_entry(mem, &liveness->blocks[user_instr->block_ref].live_in, value_ref));
                    REQUIRE_OK(add_value_liveness(mem, liveness, value_ref, user_instr->block_ref, KEFIR_CODEGEN_TARGET_IR_LIVENESS_NORMAL, KEFIR_ID_NONE, user_instr->instr_ref));
                } else {
                    REQUIRE_OK(add_value_liveness(mem, liveness, value_ref, user_instr->block_ref, KEFIR_CODEGEN_TARGET_IR_LIVENESS_NORMAL, instr->instr_ref, user_instr->instr_ref));
                }
            }
        } else if (user_instr->operation.opcode == control_flow->code->klass->phi_opcode) {
            struct kefir_codegen_target_ir_value_phi_link_iterator iter;
            kefir_codegen_target_ir_block_ref_t link_block_ref;
            struct kefir_codegen_target_ir_value_ref link_value_ref;
            for (res = kefir_codegen_target_ir_code_phi_link_iter(control_flow->code, &iter, user_instr_ref, &link_block_ref, &link_value_ref);
                res == KEFIR_OK;
                res = kefir_codegen_target_ir_code_phi_link_next(&iter, &link_block_ref, &link_value_ref)) {
                if (link_value_ref.instr_ref == value_ref.instr_ref && link_value_ref.aspect == value_ref.aspect) {
                    kefir_bool_t has_upsilon = false;
                    REQUIRE_OK(has_upsilon_for(control_flow->code, user_instr_ref, link_block_ref, &has_upsilon));
                    if (!has_upsilon) {
                        REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) link_block_ref));
                        REQUIRE_OK(add_to_entry(mem, &liveness->blocks[link_block_ref].live_out, value_ref));
                        REQUIRE_OK(add_value_liveness(mem, liveness, value_ref, link_block_ref, KEFIR_CODEGEN_TARGET_IR_LIVENESS_NORMAL, kefir_codegen_target_ir_code_block_control_tail(liveness->code, link_block_ref), KEFIR_ID_NONE));
                    }
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        } else if (instr->block_ref != user_instr->block_ref) {
            REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) user_instr->block_ref));
            REQUIRE_OK(add_to_entry(mem, &liveness->blocks[user_instr->block_ref].live_in, value_ref));
            REQUIRE_OK(add_value_liveness(mem, liveness, value_ref, user_instr->block_ref, KEFIR_CODEGEN_TARGET_IR_LIVENESS_NORMAL, KEFIR_ID_NONE, user_instr->instr_ref));
        } else {
            REQUIRE_OK(add_value_liveness(mem, liveness, value_ref, user_instr->block_ref, KEFIR_CODEGEN_TARGET_IR_LIVENESS_NORMAL, instr->instr_ref, user_instr->instr_ref));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (struct kefir_list_entry *head = kefir_list_head(queue);
        head != NULL;
        head = kefir_list_head(queue)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, block_ref,
            (kefir_uptr_t) head->value);
        REQUIRE_OK(kefir_list_pop(mem, queue, head));

        if (block_ref == instr->block_ref) {
            REQUIRE_OK(add_value_liveness(mem, liveness, value_ref, block_ref, KEFIR_CODEGEN_TARGET_IR_LIVENESS_NORMAL, instr->instr_ref, KEFIR_ID_NONE));
        }
        if (block_ref == instr->block_ref || visited_map[block_ref] ||
            !kefir_codegen_target_ir_control_flow_is_reachable(control_flow, block_ref)) {
            continue;
        }
        visited_map[block_ref] = true;
        REQUIRE_OK(add_to_entry(mem, &liveness->blocks[block_ref].live_in, value_ref));
        REQUIRE_OK(add_value_liveness(mem, liveness, value_ref, block_ref, KEFIR_CODEGEN_TARGET_IR_LIVENESS_NORMAL, KEFIR_ID_NONE, kefir_codegen_target_ir_code_block_control_head(liveness->code, block_ref)));

        struct kefir_hashset_iterator predecessor_iter;
        kefir_result_t res;
        kefir_hashset_key_t key;
        for (res = kefir_hashset_iter(&control_flow->blocks[block_ref].predecessors, &predecessor_iter, &key); res == KEFIR_OK;
            res = kefir_hashset_next(&predecessor_iter, &key)) {
            ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, predecessor_block_ref,
                key);
            if (!kefir_codegen_target_ir_control_flow_is_reachable(control_flow, predecessor_block_ref)) {
                continue;
            }
            REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) predecessor_block_ref));
            REQUIRE_OK(add_to_entry(mem, &liveness->blocks[predecessor_block_ref].live_out, value_ref));
            REQUIRE_OK(add_value_liveness(mem, liveness, value_ref, predecessor_block_ref, KEFIR_CODEGEN_TARGET_IR_LIVENESS_NORMAL, kefir_codegen_target_ir_code_block_control_tail(liveness->code, predecessor_block_ref), KEFIR_ID_NONE));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    return KEFIR_OK;
}

static kefir_result_t get_liveness_index_for(const struct kefir_codegen_target_ir_numbering *numbering,
    kefir_codegen_target_ir_instruction_ref_t instr_ref, struct kefir_codegen_target_ir_liveness_value_block_ranges *per_block_ranges, struct kefir_codegen_target_ir_liveness_index **liveness_index_ptr) {

    if (instr_ref != KEFIR_ID_NONE) {
        kefir_size_t index;
        REQUIRE_OK(kefir_codegen_target_ir_numbering_instruction_seq_index(numbering, instr_ref, &index));
        REQUIRE(index < per_block_ranges->length, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected target IR instruction sequential index"));
        *liveness_index_ptr = &per_block_ranges->indices[index];
    } else {
        *liveness_index_ptr = &per_block_ranges->null_index;
    }
    return KEFIR_OK;
}

static kefir_result_t register_liveness_at(struct kefir_mem *mem,
    const struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_value_ref_t value_ref,
    kefir_codegen_target_ir_block_ref_t block_ref, struct kefir_codegen_target_ir_liveness_value_block_ranges *per_block_ranges) {

    kefir_result_t res;
    kefir_codegen_target_ir_instruction_ref_t begin_ref, end_ref;
    struct kefir_codegen_target_ir_value_liveness_block_iterator iter;
    for (res = kefir_codegen_target_ir_value_liveness_at(liveness, &iter, value_ref, block_ref, &begin_ref, &end_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_value_liveness_at_next(&iter, &begin_ref, &end_ref)) {
            struct kefir_codegen_target_ir_liveness_index *begin_liveness = NULL, *end_liveness = NULL;
            REQUIRE_OK(get_liveness_index_for(&liveness->numbering, begin_ref, per_block_ranges, &begin_liveness));
            REQUIRE_OK(get_liveness_index_for(&liveness->numbering, end_ref, per_block_ranges, &end_liveness));

            REQUIRE_OK(kefir_hashset_add(mem, &begin_liveness->begin_liveness, (kefir_hashset_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref)));
            REQUIRE_OK(kefir_hashset_add(mem, &end_liveness->end_liveness, (kefir_hashset_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref)));
    }
    return KEFIR_OK;
}

static kefir_result_t build_per_block_liveness(struct kefir_mem *mem, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_block_ref_t block_ref, struct kefir_codegen_target_ir_liveness_value_block_ranges *per_block_ranges) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));
    REQUIRE(per_block_ranges != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR per-block liveness ranges"));

    kefir_size_t indices_len;
    REQUIRE_OK(kefir_codegen_target_ir_numbering_block_length(&liveness->numbering, block_ref, &indices_len));
    per_block_ranges->indices = KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_target_ir_liveness_index) * indices_len);
    REQUIRE(per_block_ranges->indices != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR liveness index"));
    per_block_ranges->length = indices_len;
    kefir_result_t res = KEFIR_OK;
    for (kefir_size_t i = 0; res == KEFIR_OK && i < indices_len; i++) {
        REQUIRE_CHAIN(&res, kefir_hashset_init(&per_block_ranges->indices[i].begin_liveness, &kefir_hashtable_uint_ops));
        REQUIRE_CHAIN(&res, kefir_hashset_init(&per_block_ranges->indices[i].end_liveness, &kefir_hashtable_uint_ops));
    }
    REQUIRE_CHAIN(&res, kefir_hashset_init(&per_block_ranges->null_index.begin_liveness, &kefir_hashtable_uint_ops));
    REQUIRE_CHAIN(&res, kefir_hashset_init(&per_block_ranges->null_index.end_liveness, &kefir_hashtable_uint_ops));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, per_block_ranges->indices);
        per_block_ranges->length = 0;
        per_block_ranges->indices = NULL;
        return res;
    });

    for (kefir_size_t i = 0; i < liveness->blocks[block_ref].live_in.length; i++) {
        REQUIRE_OK(register_liveness_at(mem, liveness, liveness->blocks[block_ref].live_in.content[i], block_ref, per_block_ranges));
    }
    for (kefir_size_t i = 0; i < liveness->blocks[block_ref].live_out.length; i++) {
        REQUIRE_OK(register_liveness_at(mem, liveness, liveness->blocks[block_ref].live_out.content[i], block_ref, per_block_ranges));
    }
    for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(control_flow->code, block_ref);
        instr_ref != KEFIR_ID_NONE;
        instr_ref = kefir_codegen_target_ir_code_control_next(control_flow->code, instr_ref)) {
        struct kefir_codegen_target_ir_value_iterator value_iter;
        struct kefir_codegen_target_ir_value_ref value_ref;
        kefir_result_t res;
        for (res = kefir_codegen_target_ir_code_value_iter(liveness->code, &value_iter, instr_ref, &value_ref, NULL);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_value_next(&value_iter, &value_ref, NULL)) {
            REQUIRE_OK(register_liveness_at(mem, liveness, value_ref, block_ref, per_block_ranges));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }   
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_liveness_build(struct kefir_mem *mem, const struct kefir_codegen_target_ir_control_flow *control_flow, struct kefir_codegen_target_ir_liveness *liveness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));

    REQUIRE_OK(kefir_codegen_target_ir_liveness_reset(mem, liveness));
    REQUIRE_OK(kefir_codegen_target_ir_numbering_build(mem, &liveness->numbering, control_flow->code));

    liveness->blocks = KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_target_ir_block_liveness) * kefir_codegen_target_ir_code_block_count(control_flow->code));
    REQUIRE(liveness->blocks != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR liveness information"));
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(control_flow->code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(control_flow->code, i);

        liveness->blocks[block_ref].live_in.content = NULL;
        liveness->blocks[block_ref].live_in.length = 0;
        liveness->blocks[block_ref].live_in.capacity = 0;
        liveness->blocks[block_ref].live_out.content = NULL;
        liveness->blocks[block_ref].live_out.length = 0;
        liveness->blocks[block_ref].live_out.capacity = 0;
        liveness->blocks[block_ref].value_liveness_ranges_ready = false;
        liveness->blocks[block_ref].value_liveness_ranges.indices = NULL;
        liveness->blocks[block_ref].value_liveness_ranges.length = 0;
    }
    liveness->code = control_flow->code;
    
    struct kefir_list queue;
    REQUIRE_OK(kefir_list_init(&queue));
    kefir_uint8_t *visited_map = KEFIR_MALLOC(mem, sizeof(kefir_uint8_t) * kefir_codegen_target_ir_code_block_count(liveness->code));
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(liveness->code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(liveness->code, i);
        if (!kefir_codegen_target_ir_control_flow_is_reachable(control_flow, block_ref)) {
            continue;
        }

        for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(liveness->code, block_ref);
            instr_ref != KEFIR_ID_NONE;
            instr_ref = kefir_codegen_target_ir_code_control_next(liveness->code, instr_ref)) {
            struct kefir_codegen_target_ir_value_iterator value_iter;
            struct kefir_codegen_target_ir_value_ref value_ref;
            kefir_result_t res;
            for (res = kefir_codegen_target_ir_code_value_iter(liveness->code, &value_iter, instr_ref, &value_ref, NULL);
                res == KEFIR_OK;
                res = kefir_codegen_target_ir_code_value_next(&value_iter, &value_ref, NULL)) {
                res = propagate_instr_liveness(mem, control_flow, liveness, value_ref, &queue, visited_map);
                REQUIRE_ELSE(res == KEFIR_OK, {
                    KEFIR_FREE(mem, visited_map);
                    kefir_list_free(mem, &queue);
                    return res;
                });
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }
    }
    KEFIR_FREE(mem, visited_map);
    REQUIRE_OK(kefir_list_free(mem, &queue));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_liveness_value_ranges(struct kefir_mem *mem, const struct kefir_codegen_target_ir_control_flow *control_flow,
    const struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_block_ref_t block_ref, const struct kefir_codegen_target_ir_liveness_value_block_ranges **ranges_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));
    REQUIRE(liveness->blocks != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pre-built target IR liveness"));
    REQUIRE(block_ref != KEFIR_ID_NONE && block_ref < control_flow->code->blocks_length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR block referenc"));
    
    if (!liveness->blocks[block_ref].value_liveness_ranges_ready &&
        kefir_codegen_target_ir_control_flow_is_reachable(control_flow, block_ref)) {
        REQUIRE_OK(build_per_block_liveness(mem, control_flow, liveness, block_ref, &liveness->blocks[block_ref].value_liveness_ranges));
        liveness->blocks[block_ref].value_liveness_ranges_ready = true;
    }
    ASSIGN_PTR(ranges_ptr, &liveness->blocks[block_ref].value_liveness_ranges);
    return KEFIR_OK;    
}

kefir_result_t kefir_codegen_target_ir_liveness_build_update_alive_set(struct kefir_mem *mem, const struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_instruction_ref_t instr_ref, const struct kefir_codegen_target_ir_liveness_value_block_ranges *per_block_ranges, struct kefir_hashset *alive_values) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));
    REQUIRE(per_block_ranges != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR per-block liveness ranges"));
    REQUIRE(alive_values != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR alive value set"));

    if (instr_ref == KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_hashset_trim(mem, alive_values));
    }

    const struct kefir_codegen_target_ir_liveness_index *liveness_index = &per_block_ranges->null_index;
    if (instr_ref != KEFIR_ID_NONE) {
        kefir_size_t index;
        REQUIRE_OK(kefir_codegen_target_ir_numbering_instruction_seq_index(&liveness->numbering, instr_ref, &index));
        REQUIRE(index < per_block_ranges->length, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected target IR instruction sequential index"));
        liveness_index = &per_block_ranges->indices[index];
    }

    if (instr_ref != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_hashset_subtract(alive_values, &liveness_index->end_liveness));
    }
    REQUIRE_OK(kefir_hashset_merge(mem, alive_values, &liveness_index->begin_liveness));
    return KEFIR_OK;
}


kefir_result_t kefir_codegen_target_ir_liveness_range_get(const struct kefir_codegen_target_ir_liveness *liveness, const struct kefir_codegen_target_ir_liveness_value_block_ranges *ranges,
    kefir_codegen_target_ir_instruction_ref_t instr_ref, const struct kefir_codegen_target_ir_liveness_index **entry_ptr) {
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));
    REQUIRE(ranges != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR per-block liveness ranges"));

    if (instr_ref != KEFIR_ID_NONE) {
        kefir_size_t index;
        REQUIRE_OK(kefir_codegen_target_ir_numbering_instruction_seq_index(&liveness->numbering, instr_ref, &index));
        REQUIRE(index < ranges->length, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected target IR instruction sequential index"));

        ASSIGN_PTR(entry_ptr, &ranges->indices[index]);
    } else {
        ASSIGN_PTR(entry_ptr, &ranges->null_index);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_value_liveness_at(const struct kefir_codegen_target_ir_liveness *liveness,
    struct kefir_codegen_target_ir_value_liveness_block_iterator *iter,
    kefir_codegen_target_ir_value_ref_t value_ref,
    kefir_codegen_target_ir_block_ref_t block_ref,
    kefir_codegen_target_ir_instruction_ref_t *begin_ref_ptr, kefir_codegen_target_ir_instruction_ref_t *end_ref_ptr) {
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR liveness per-block iterator"));

    iter->liveness = liveness;
    iter->block_ref = block_ref;
    iter->value_ref = value_ref;
    iter->klass = KEFIR_CODEGEN_TARGET_IR_LIVENESS_NORMAL;

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&liveness->values, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&iter->value_ref), &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find liveness for requested target IR value");
    }
    REQUIRE_OK(res);

    for (; iter->klass != KEFIR_CODEGEN_TARGET_IR_LIVENESS_END; iter->klass++) {
        ASSIGN_DECL_CAST(const struct kefir_codegen_target_ir_value_liveness *, value_liveness, table_value);
        kefir_hashtable_key_t key = (((kefir_uint64_t) iter->block_ref) << 32) | iter->klass;
        res = kefir_hashtable_at(&value_liveness->per_block, key, &table_value);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        ASSIGN_PTR(begin_ref_ptr, ((kefir_uint64_t) table_value) >> 32);
        ASSIGN_PTR(end_ref_ptr, (kefir_uint32_t) table_value);
        return KEFIR_OK;
    }
    return KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of per-block target IR value liveness iterator");
}

kefir_result_t kefir_codegen_target_ir_value_liveness_at_next(struct kefir_codegen_target_ir_value_liveness_block_iterator *iter,
    kefir_codegen_target_ir_instruction_ref_t *begin_ref_ptr, kefir_codegen_target_ir_instruction_ref_t *end_ref_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness per-block iterator"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&iter->liveness->values, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&iter->value_ref), &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find liveness for requested target IR value");
    }
    REQUIRE_OK(res);

    iter->klass++;
    for (; iter->klass != KEFIR_CODEGEN_TARGET_IR_LIVENESS_END; iter->klass++) {
        ASSIGN_DECL_CAST(const struct kefir_codegen_target_ir_value_liveness *, value_liveness, table_value);
        kefir_hashtable_key_t key = (((kefir_uint64_t) iter->block_ref) << 32) | iter->klass;
        res = kefir_hashtable_at(&value_liveness->per_block, key, &table_value);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        ASSIGN_PTR(begin_ref_ptr, ((kefir_uint64_t) table_value) >> 32);
        ASSIGN_PTR(end_ref_ptr, (kefir_uint32_t) table_value);
        return KEFIR_OK;
    }
    return KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of per-block target IR value liveness iterator");
}

kefir_result_t kefir_codegen_target_ir_value_liveness_upsilon_at(const struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_value_ref_t value_ref,
    kefir_codegen_target_ir_block_ref_t block_ref,
    kefir_codegen_target_ir_instruction_ref_t *begin_ref_ptr, kefir_codegen_target_ir_instruction_ref_t *end_ref_ptr) {
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&liveness->values, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find liveness for requested target IR value");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(const struct kefir_codegen_target_ir_value_liveness *, value_liveness, table_value);
    kefir_hashtable_key_t key = (((kefir_uint64_t) block_ref) << 32) | KEFIR_CODEGEN_TARGET_IR_LIVENESS_NORMAL;
    res = kefir_hashtable_at(&value_liveness->per_block, key, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find liveness for target IR value in requested block");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(begin_ref_ptr, ((kefir_uint64_t) table_value) >> 32);
    ASSIGN_PTR(end_ref_ptr, (kefir_uint32_t) table_value);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_value_liveness_iter(const struct kefir_codegen_target_ir_liveness *liveness, struct kefir_codegen_target_ir_value_liveness_iterator *iter,
    kefir_codegen_target_ir_value_ref_t value_ref, kefir_codegen_target_ir_block_ref_t *block_ref_ptr,
    kefir_codegen_target_ir_instruction_ref_t *begin_ref_ptr,
    kefir_codegen_target_ir_instruction_ref_t *end_ref_ptr) {
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR value liveness iterator"));

    kefir_hashtable_key_t table_key;
    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&liveness->values, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find target IR value liveness");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(const struct kefir_codegen_target_ir_value_liveness *, value_liveness, table_value);

    res = kefir_hashtable_iter(&value_liveness->per_block, &iter->iter, &table_key, &table_value);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR value liveness iterator");
    }
    REQUIRE_OK(res);
    ASSIGN_PTR(block_ref_ptr, ((kefir_uint64_t) table_key) >> 32);
    ASSIGN_PTR(begin_ref_ptr, ((kefir_uint64_t) table_value) >> 32);
    ASSIGN_PTR(end_ref_ptr, (kefir_uint32_t) table_value);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_value_liveness_next(struct kefir_codegen_target_ir_value_liveness_iterator *iter,
    kefir_codegen_target_ir_block_ref_t *block_ref_ptr,
    kefir_codegen_target_ir_instruction_ref_t *begin_ref_ptr,
    kefir_codegen_target_ir_instruction_ref_t *end_ref_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR value liveness iterator"));
    
    kefir_hashtable_key_t table_key;
    kefir_hashtable_value_t table_value;

    kefir_result_t res = kefir_hashtable_next(&iter->iter, &table_key, &table_value);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR value liveness iterator");
    }
    REQUIRE_OK(res);
    ASSIGN_PTR(block_ref_ptr, ((kefir_uint64_t) table_key) >> 32);
    ASSIGN_PTR(begin_ref_ptr, ((kefir_uint64_t) table_value) >> 32);
    ASSIGN_PTR(end_ref_ptr, (kefir_uint32_t) table_value);
    return KEFIR_OK;
}
