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

#include "kefir/optimizer/linear_liveness.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t free_instruction_liveness(struct kefir_mem *mem, struct kefir_hashtable *table,
                                                          kefir_hashtable_key_t key, kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_instruction_linear_liveness *, instr_liveness,
        value);
    REQUIRE(instr_liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction linear liveness"));

    REQUIRE_OK(kefir_hashtable_free(mem, &instr_liveness->per_block));
    KEFIR_FREE(mem, instr_liveness);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_linear_liveness_init(struct kefir_opt_code_linear_liveness *liveness) {
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code linear liveness"));

    REQUIRE_OK(kefir_hashtable_init(&liveness->instructions, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&liveness->instructions, free_instruction_liveness, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_linear_liveness_free(struct kefir_mem *mem, struct kefir_opt_code_linear_liveness *liveness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code linear liveness"));

    REQUIRE_OK(kefir_hashtable_free(mem, &liveness->instructions));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_linear_liveness_clear(struct kefir_mem *mem, struct kefir_opt_code_linear_liveness *liveness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code linear liveness"));

    REQUIRE_OK(kefir_hashtable_free(mem, &liveness->instructions));
    REQUIRE_OK(kefir_hashtable_init(&liveness->instructions, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&liveness->instructions, free_instruction_liveness, NULL));
    return KEFIR_OK;
}

static kefir_result_t update_liveness_for(struct kefir_mem *mem, struct kefir_opt_code_instruction_linear_liveness *liveness, kefir_opt_block_id_t block_id, kefir_uint32_t begin_index, kefir_uint32_t end_index) {
    kefir_hashtable_value_t *value_ptr;
    kefir_result_t res = kefir_hashtable_at_mut(&liveness->per_block, (kefir_hashtable_key_t) block_id, &value_ptr);
    if (res == KEFIR_NOT_FOUND) {
        kefir_uint64_t value = (((kefir_uint64_t) begin_index) << 32) | (kefir_uint32_t) end_index;
        REQUIRE_OK(kefir_hashtable_insert(mem, &liveness->per_block, (kefir_hashtable_key_t) block_id, (kefir_hashtable_value_t) value));
    } else {
        REQUIRE_OK(res);
        kefir_uint32_t current_begin_index = ((kefir_uint64_t) *value_ptr) >> 32;
        kefir_uint32_t current_end_index = (kefir_uint32_t) *value_ptr;
        *value_ptr = (((kefir_uint64_t) MIN(current_begin_index, begin_index)) << 32) | (kefir_uint32_t) MAX(current_end_index, end_index);
    }
    return KEFIR_OK;
}

static kefir_result_t propagate_instruction_liveness(struct kefir_mem *mem, struct kefir_opt_code_linear_liveness *liveness, const struct kefir_opt_code_container *code, const struct kefir_opt_code_structure *structure, const struct kefir_opt_code_schedule *schedule, kefir_opt_instruction_ref_t instr_ref, struct kefir_list *queue, struct kefir_hashset *visited) {
    REQUIRE_OK(kefir_list_clear(mem, queue));
    REQUIRE_OK(kefir_hashset_clear(mem, visited));

    struct kefir_opt_code_instruction_linear_liveness *instr_liveness = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_instruction_linear_liveness));
    REQUIRE(instr_liveness != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer instruction linear liveness"));
    kefir_result_t res = kefir_hashtable_init(&instr_liveness->per_block, &kefir_hashtable_uint_ops);
    REQUIRE_CHAIN(&res, kefir_hashtable_insert(mem, &liveness->instructions, (kefir_hashtable_key_t) instr_ref, (kefir_hashtable_value_t) instr_liveness));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, instr_liveness);
        return res;
    });

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));

    const struct kefir_opt_code_instruction_schedule *instr_schedule;
    REQUIRE_OK(kefir_opt_code_schedule_of(schedule, instr_ref, &instr_schedule));

    struct kefir_opt_instruction_use_iterator use_iter;
    for (res = kefir_opt_code_container_instruction_use_instr_iter(code, instr_ref, &use_iter);
         res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_instruction *use_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, use_iter.use_instr_ref, &use_instr));

        if (use_instr->operation.opcode == KEFIR_OPT_OPCODE_PHI) {
            const struct kefir_opt_phi_node *use_phi;
            REQUIRE_OK(kefir_opt_code_container_phi(code, use_instr->operation.parameters.phi_ref,
                                                    &use_phi));
            struct kefir_hashtree_node_iterator iter;
            for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&use_phi->links, &iter); node != NULL;
                 node = kefir_hashtree_next(&iter)) {
                ASSIGN_DECL_CAST(kefir_opt_block_id_t, src_block_id, node->key);
                ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, src_instr_ref, node->value);
                if (src_instr_ref == instr_ref) {
                    const struct kefir_opt_code_block_schedule *src_block_schedule;
                    res = kefir_opt_code_schedule_of_block(schedule, src_block_id, &src_block_schedule);
                    if (res == KEFIR_NOT_FOUND) {
                        continue;
                    }
                    REQUIRE_OK(res);
                    kefir_uint64_t key = (((kefir_uint64_t) src_block_id) << 32) | (kefir_uint32_t) src_block_schedule->instructions_length;
                    REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) key));
                }
            }
        } else {
            const struct kefir_opt_code_instruction_schedule *use_instr_schedule;
            res = kefir_opt_code_schedule_of(schedule, use_iter.use_instr_ref, &use_instr_schedule);
            if (res == KEFIR_NOT_FOUND) {
                continue;
            }
            REQUIRE_OK(res);
            kefir_uint64_t key = (((kefir_uint64_t) use_instr->block_id) << 32) | (kefir_uint32_t) use_instr_schedule->linear_position;
            REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) key));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (struct kefir_list_entry *iter = kefir_list_head(queue);
        iter != NULL;
        iter = kefir_list_head(queue)) {
        ASSIGN_DECL_CAST(kefir_uint64_t, key, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(mem, queue, iter));

        kefir_opt_block_id_t block_id = key >> 32;
        kefir_uint32_t linear_index = (kefir_uint32_t) key;

        if (block_id == instr->block_id) {
            REQUIRE_OK(update_liveness_for(mem, instr_liveness, block_id, instr_schedule->linear_position, linear_index));
        } else {
            const struct kefir_opt_code_block_schedule *block_schedule;
            res = kefir_opt_code_schedule_of_block(schedule, block_id, &block_schedule);
            if (res == KEFIR_NOT_FOUND) {
                continue;
            }
            REQUIRE_OK(res);

            REQUIRE_OK(update_liveness_for(mem, instr_liveness, block_id, 0, linear_index));
            if (!kefir_hashset_has(visited, (kefir_hashset_key_t) block_id)) {
                for (const struct kefir_list_entry *pred_iter = kefir_list_head(&structure->blocks[block_id].predecessors);
                    pred_iter != NULL;
                    kefir_list_next(&pred_iter)) {
                    ASSIGN_DECL_CAST(kefir_opt_block_id_t, predecessor_block_id, (kefir_uptr_t) pred_iter->value);
                    const struct kefir_opt_code_block_schedule *predecessor_block_schedule;
                    res = kefir_opt_code_schedule_of_block(schedule, predecessor_block_id, &predecessor_block_schedule);
                    if (res == KEFIR_NOT_FOUND) {
                        continue;
                    }
                    REQUIRE_OK(res);
                    kefir_uint64_t key = (((kefir_uint64_t) predecessor_block_id) << 32) | (kefir_uint32_t) predecessor_block_schedule->instructions_length;
                    REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) (kefir_uptr_t) key));
                }
            }
            REQUIRE_OK(kefir_hashset_add(mem, visited, (kefir_hashset_key_t) block_id));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_linear_liveness_build(struct kefir_mem *mem, struct kefir_opt_code_linear_liveness *liveness, const struct kefir_opt_code_container *code, const struct kefir_opt_code_structure *structure, const struct kefir_opt_code_schedule *schedule) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code linear liveness"));
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code schedule"));

    struct kefir_list queue;
    struct kefir_hashset visited;
    REQUIRE_OK(kefir_list_init(&queue));
    REQUIRE_OK(kefir_hashset_init(&visited, &kefir_hashtable_uint_ops));
    for (kefir_size_t i = 0; i < schedule->blocks_length; i++) {
        for (kefir_size_t j = 0; j < schedule->blocks[i].instructions_length; j++) {
            kefir_result_t res = propagate_instruction_liveness(mem, liveness, code, structure, schedule, schedule->blocks[i].instructions[j].instr_ref, &queue, &visited);
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_list_free(mem, &queue);
                kefir_hashset_free(mem, &visited);
                return res;
            });
        }
    }

    kefir_result_t res = kefir_list_free(mem, &queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &visited);
        return res;
    });
    REQUIRE_OK(kefir_hashset_free(mem, &visited));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_instruction_linear_liveness_iter(const struct kefir_opt_code_linear_liveness *liveness, kefir_opt_instruction_ref_t instr_ref,
    struct kefir_opt_code_instruction_linear_liveness_iterator *iter,
    kefir_opt_block_id_t *block_id_ptr, kefir_uint32_t *liveness_begin_ptr, kefir_uint32_t *liveness_end_ptr) {
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code linear liveness"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction linear liveness iterator"));

    kefir_hashtable_key_t table_key;
    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&liveness->instructions, (kefir_hashtable_key_t) instr_ref, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested optimizer instruction linear liveness");
    }
    REQUIRE_OK(res);
    ASSIGN_DECL_CAST(const struct kefir_opt_code_instruction_linear_liveness *,instr_liveness, table_value);

    res = kefir_hashtable_iter(&instr_liveness->per_block, &iter->iter, &table_key, &table_value);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of optimizer instruction linear liveness iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(block_id_ptr, (kefir_opt_block_id_t) table_key);
    ASSIGN_PTR(liveness_begin_ptr, ((kefir_uint64_t) table_value) >> 32);
    ASSIGN_PTR(liveness_end_ptr, (kefir_uint32_t) table_value);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_instruction_linear_liveness_next(struct kefir_opt_code_instruction_linear_liveness_iterator *iter, kefir_opt_block_id_t *block_id_ptr, kefir_uint32_t *liveness_begin_ptr, kefir_uint32_t *liveness_end_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction linear liveness iterator"));

    kefir_hashtable_key_t table_key;
    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_next(&iter->iter, &table_key, &table_value);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of optimizer instruction linear liveness iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(block_id_ptr, (kefir_opt_block_id_t) table_key);
    ASSIGN_PTR(liveness_begin_ptr, ((kefir_uint64_t) table_value) >> 32);
    ASSIGN_PTR(liveness_end_ptr, (kefir_uint32_t) table_value);
    return KEFIR_OK;
}
