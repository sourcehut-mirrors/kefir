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

#include "kefir/codegen/target-ir/amd64/chain_scheduler.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct chain {
    struct kefir_list blocks;
    struct kefir_hashset block_index;
};

struct scheduler_payload {
    const struct kefir_codegen_target_ir_control_flow *control_flow;
    const struct kefir_codegen_target_ir_loop_collection *loops;

    struct kefir_hashtable edge_freqs;
    struct kefir_hashtable continuation_benefit;
    struct kefir_hashset unchained_blocks;
    struct kefir_hashtable chains;
    struct kefir_hashtable chain_index;
};

static kefir_result_t build_edge_freqs(struct kefir_mem *mem, struct scheduler_payload *scheduler_payload) {
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(scheduler_payload->control_flow->code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref =
            kefir_codegen_target_ir_code_block_by_index(scheduler_payload->control_flow->code, i);

        kefir_uint32_t depth;
        REQUIRE_OK(kefir_codegen_target_ir_loop_level(scheduler_payload->loops, block_ref, &depth));
        const kefir_uint64_t freq = 1ull << MIN(2 * depth, 32);

        kefir_result_t res;
        kefir_hashset_key_t entry;
        struct kefir_hashset_iterator iter;
        for (res = kefir_hashset_iter(&scheduler_payload->control_flow->blocks[block_ref].successors, &iter, &entry);
             res == KEFIR_OK; res = kefir_hashset_next(&iter, &entry)) {
            ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, succ_block_ref, entry);

            kefir_uint64_t key = (((kefir_uint64_t) block_ref) << 32) | (kefir_uint32_t) succ_block_ref;
            kefir_uint64_t prob = (freq << 16) / scheduler_payload->control_flow->blocks[block_ref].successors.occupied;
            REQUIRE_OK(kefir_hashtable_insert(mem, &scheduler_payload->edge_freqs, (kefir_hashtable_key_t) key,
                                              (kefir_hashtable_value_t) prob));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        REQUIRE_OK(kefir_hashtable_insert(mem, &scheduler_payload->continuation_benefit,
                                          (kefir_hashtable_key_t) block_ref, (kefir_hashtable_value_t) 0));
    }
    return KEFIR_OK;
}

static kefir_result_t solve_cont_benefit(struct scheduler_payload *scheduler_payload, kefir_size_t rounds) {
    for (kefir_size_t round = 0; round < rounds; round++) {
        for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(scheduler_payload->control_flow->code);
             i++) {
            kefir_codegen_target_ir_block_ref_t block_ref =
                kefir_codegen_target_ir_code_block_by_index(scheduler_payload->control_flow->code, i);

            kefir_hashtable_value_t *value_ptr;
            REQUIRE_OK(kefir_hashtable_at_mut(&scheduler_payload->continuation_benefit,
                                              (kefir_hashtable_key_t) block_ref, &value_ptr));

            kefir_uint64_t max_benefit = 0;

            kefir_result_t res;
            kefir_hashset_key_t entry;
            struct kefir_hashset_iterator iter;
            for (res =
                     kefir_hashset_iter(&scheduler_payload->control_flow->blocks[block_ref].successors, &iter, &entry);
                 res == KEFIR_OK; res = kefir_hashset_next(&iter, &entry)) {
                ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, succ_block_ref, entry);

                kefir_uint64_t key = (((kefir_uint64_t) block_ref) << 32) | (kefir_uint32_t) succ_block_ref;
                kefir_hashtable_value_t edge_freq, succ_benefit;
                REQUIRE_OK(kefir_hashtable_at(&scheduler_payload->edge_freqs, (kefir_hashtable_key_t) key, &edge_freq));
                REQUIRE_OK(kefir_hashtable_at(&scheduler_payload->continuation_benefit,
                                              (kefir_hashtable_key_t) succ_block_ref, &succ_benefit));

                max_benefit = MAX(max_benefit, edge_freq + succ_benefit * 4 / 5);
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }

            *value_ptr = max_benefit;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t compute_chain(struct kefir_mem *mem, struct scheduler_payload *scheduler_payload,
                                    kefir_codegen_target_ir_block_ref_t head_ref) {
    REQUIRE(kefir_hashset_has(&scheduler_payload->unchained_blocks, (kefir_hashset_key_t) head_ref),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Chain head block has already been scheduled"));

    struct chain *chain = KEFIR_MALLOC(mem, sizeof(struct chain));
    REQUIRE(chain != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate scheduler chain"));
    kefir_result_t res = kefir_list_init(&chain->blocks);
    REQUIRE_CHAIN(&res, kefir_hashset_init(&chain->block_index, &kefir_hashtable_uint_ops));
    REQUIRE_CHAIN(&res, kefir_hashtable_insert(mem, &scheduler_payload->chains, (kefir_hashtable_key_t) head_ref,
                                               (kefir_hashtable_value_t) chain));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, chain);
        return res;
    });

    REQUIRE_OK(kefir_list_insert_after(mem, &chain->blocks, kefir_list_tail(&chain->blocks),
                                       (void *) (kefir_uptr_t) head_ref));
    REQUIRE_OK(kefir_hashset_delete(&scheduler_payload->unchained_blocks, (kefir_hashset_key_t) head_ref));

    for (;;) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, block_ref,
                         (kefir_uptr_t) kefir_list_tail(&chain->blocks)->value);

        kefir_codegen_target_ir_block_ref_t succ_block_ref = KEFIR_ID_NONE;
        kefir_uint64_t succ_benefit = 0;

        kefir_result_t res;
        struct kefir_hashset_iterator iter;
        kefir_hashset_key_t key;
        for (res = kefir_hashset_iter(&scheduler_payload->control_flow->blocks[block_ref].successors, &iter, &key);
             res == KEFIR_OK; res = kefir_hashset_next(&iter, &key)) {
            ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, candidate_block_ref, key);
            if (candidate_block_ref == scheduler_payload->control_flow->code->entry_block ||
                candidate_block_ref == head_ref ||
                (!kefir_hashset_has(&scheduler_payload->unchained_blocks, (kefir_hashset_key_t) candidate_block_ref) &&
                 !kefir_hashtable_has(&scheduler_payload->chains, (kefir_hashtable_key_t) candidate_block_ref))) {
                continue;
            }

            kefir_hashtable_value_t cont_benefit;
            REQUIRE_OK(kefir_hashtable_at(&scheduler_payload->continuation_benefit,
                                          (kefir_hashtable_key_t) candidate_block_ref, &cont_benefit));
            if (succ_block_ref == KEFIR_ID_NONE || cont_benefit > succ_benefit) {
                succ_block_ref = candidate_block_ref;
                succ_benefit = cont_benefit;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        if (succ_block_ref == KEFIR_ID_NONE) {
            break;
        }

        kefir_hashtable_value_t table_value;
        res = kefir_hashtable_at(&scheduler_payload->chains, (kefir_hashtable_key_t) succ_block_ref, &table_value);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            ASSIGN_DECL_CAST(struct chain *, candidate_chain, table_value);
            REQUIRE_OK(kefir_list_move_all(&chain->blocks, &candidate_chain->blocks));
            REQUIRE_OK(kefir_hashtable_delete(mem, &scheduler_payload->chains, (kefir_hashtable_key_t) succ_block_ref));
        } else {
            REQUIRE_OK(kefir_list_insert_after(mem, &chain->blocks, kefir_list_tail(&chain->blocks),
                                               (void *) (kefir_uptr_t) succ_block_ref));
            REQUIRE_OK(
                kefir_hashset_delete(&scheduler_payload->unchained_blocks, (kefir_hashset_key_t) succ_block_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t index_chain(struct kefir_mem *mem, struct scheduler_payload *payload, struct chain *chain) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&chain->blocks); iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, block_ref, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_hashset_add(mem, &chain->block_index, (kefir_hashset_key_t) block_ref));
        REQUIRE_OK(
            kefir_hashtable_insert(mem, &payload->chain_index, (kefir_hashtable_key_t) block_ref,
                                   (kefir_hashtable_value_t) (kefir_uptr_t) kefir_list_head(&chain->blocks)->value));
    }
    return KEFIR_OK;
}

static kefir_result_t schedule_chain(struct kefir_mem *mem,
                                     struct kefir_codegen_target_ir_code_schedule_builder *schedule_builder,
                                     struct scheduler_payload *scheduler_payload,
                                     kefir_codegen_target_ir_block_ref_t head_ref,
                                     kefir_codegen_target_ir_block_ref_t *next_chain_ref) {
    kefir_hashtable_value_t table_value;
    REQUIRE_OK(kefir_hashtable_at(&scheduler_payload->chains, (kefir_hashtable_key_t) head_ref, &table_value));
    ASSIGN_DECL_CAST(struct chain *, chain, table_value);

    kefir_codegen_target_ir_block_ref_t deepest_loop_adjacent_chain = KEFIR_ID_NONE;
    kefir_size_t deepest_loop_adjacent_chain_level = 0;
    for (const struct kefir_list_entry *iter = kefir_list_head(&chain->blocks); iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, block_ref, (kefir_uptr_t) iter->value);
        if (!kefir_codegen_target_ir_code_is_gate_block(scheduler_payload->control_flow->code, block_ref)) {
            REQUIRE_OK(schedule_builder->schedule_block(mem, block_ref, schedule_builder->payload));

            kefir_result_t res;
            kefir_hashset_key_t entry;
            struct kefir_hashset_iterator iter;
            for (res =
                     kefir_hashset_iter(&scheduler_payload->control_flow->blocks[block_ref].successors, &iter, &entry);
                 res == KEFIR_OK; res = kefir_hashset_next(&iter, &entry)) {
                ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, succ_block_ref, entry);
                if (kefir_hashset_has(&chain->block_index, (kefir_hashset_key_t) succ_block_ref)) {
                    continue;
                }

                struct kefir_codegen_target_ir_loop *loop;
                res = kefir_codegen_target_ir_loop_collection_find_loop(
                    scheduler_payload->loops, (kefir_codegen_target_ir_block_ref_t) succ_block_ref, &loop);
                if (res != KEFIR_NOT_FOUND) {
                    REQUIRE_OK(res);
                    if (loop->level > deepest_loop_adjacent_chain_level) {
                        kefir_hashtable_value_t chain_head;
                        REQUIRE_OK(kefir_hashtable_at(&scheduler_payload->chain_index,
                                                      (kefir_hashtable_key_t) succ_block_ref, &chain_head));
                        if (kefir_hashtable_has(&scheduler_payload->chains, (kefir_hashtable_key_t) chain_head)) {
                            deepest_loop_adjacent_chain = chain_head;
                            deepest_loop_adjacent_chain_level = loop->level;
                        }
                    }
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }
    }

    REQUIRE_OK(kefir_hashtable_delete(mem, &scheduler_payload->chains, (kefir_hashtable_key_t) head_ref));

    *next_chain_ref = deepest_loop_adjacent_chain;
    return KEFIR_OK;
}

static kefir_result_t do_schedule_impl(struct kefir_mem *mem,
                                       struct kefir_codegen_target_ir_code_schedule_builder *schedule_builder,
                                       struct scheduler_payload *scheduler_payload) {
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(scheduler_payload->control_flow->code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref =
            kefir_codegen_target_ir_code_block_by_index(scheduler_payload->control_flow->code, i);
        if (kefir_codegen_target_ir_control_flow_is_reachable(scheduler_payload->control_flow, block_ref)) {
            REQUIRE_OK(kefir_hashset_add(mem, &scheduler_payload->unchained_blocks, (kefir_hashset_key_t) block_ref));
        }
    }

    REQUIRE_OK(compute_chain(mem, scheduler_payload,
                             (kefir_codegen_target_ir_block_ref_t) scheduler_payload->control_flow->code->entry_block));
    kefir_result_t res;
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t key;
    for (res = kefir_hashset_iter(&scheduler_payload->unchained_blocks, &iter, &key); res == KEFIR_OK;
         res = kefir_hashset_iter(&scheduler_payload->unchained_blocks, &iter, &key)) {
        REQUIRE_OK(compute_chain(mem, scheduler_payload, (kefir_codegen_target_ir_block_ref_t) key));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    REQUIRE(scheduler_payload->chains.occupied > 0, KEFIR_OK);

    struct kefir_hashtable_iterator chain_iter;
    kefir_hashtable_key_t chain_key;
    kefir_hashtable_value_t chain_value;
    for (res = kefir_hashtable_iter(&scheduler_payload->chains, &chain_iter, &chain_key, &chain_value); res == KEFIR_OK;
         res = kefir_hashtable_next(&chain_iter, &chain_key, &chain_value)) {
        ASSIGN_DECL_CAST(struct chain *, chain, chain_value);
        REQUIRE_OK(index_chain(mem, scheduler_payload, chain));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    kefir_codegen_target_ir_block_ref_t next_chain_ref = KEFIR_ID_NONE;
    REQUIRE_OK(schedule_chain(mem, schedule_builder, scheduler_payload,
                              scheduler_payload->control_flow->code->entry_block, &next_chain_ref));
    if (next_chain_ref == KEFIR_ID_NONE) {
        res = kefir_hashtable_iter(&scheduler_payload->chains, &chain_iter, &chain_key, NULL);
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
            next_chain_ref = chain_key;
        }
    }

    for (; next_chain_ref != KEFIR_ID_NONE;) {
        REQUIRE_OK(schedule_chain(mem, schedule_builder, scheduler_payload, next_chain_ref, &next_chain_ref));
        if (next_chain_ref == KEFIR_ID_NONE) {
            res = kefir_hashtable_iter(&scheduler_payload->chains, &chain_iter, &chain_key, NULL);
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
                next_chain_ref = chain_key;
            }
        }
    }

    return KEFIR_OK;
}

static kefir_result_t do_schedule(struct kefir_mem *mem, const struct kefir_codegen_target_ir_code_schedule *schedule,
                                  struct kefir_codegen_target_ir_code_schedule_builder *schedule_builder,
                                  kefir_codegen_target_ir_block_ref_t entry_point_ref, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR schedule"));
    REQUIRE(schedule_builder != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR schedule builder"));
    REQUIRE(
        entry_point_ref != KEFIR_ID_NONE && entry_point_ref < kefir_codegen_target_ir_code_block_count(schedule->code),
        KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR schedule entry point"));
    ASSIGN_DECL_CAST(struct scheduler_payload *, scheduler_payload, payload);

    REQUIRE_OK(build_edge_freqs(mem, scheduler_payload));
    REQUIRE_OK(solve_cont_benefit(scheduler_payload, 8));

    struct kefir_list queue;
    REQUIRE_OK(kefir_list_init(&queue));
    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN(&res, do_schedule_impl(mem, schedule_builder, scheduler_payload));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &queue));
    return KEFIR_OK;
}

static kefir_result_t free_chain(struct kefir_mem *mem, struct kefir_hashtable *table, kefir_hashtable_key_t key,
                                 kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct chain *, chain, value);
    REQUIRE(chain != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid scheduler chain"));

    REQUIRE_OK(kefir_hashset_free(mem, &chain->block_index));
    REQUIRE_OK(kefir_list_free(mem, &chain->blocks));
    KEFIR_FREE(mem, chain);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_chain_scheduler_init(
    struct kefir_mem *mem, const struct kefir_codegen_target_ir_control_flow *control_flow,
    const struct kefir_codegen_target_ir_loop_collection *loops,
    struct kefir_codegen_target_ir_code_scheduler *scheduler) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(loops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR loop collection"));
    REQUIRE(scheduler != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR scheduler"));

    struct scheduler_payload *payload = KEFIR_MALLOC(mem, sizeof(struct scheduler_payload));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate scheduler payload"));
    payload->control_flow = control_flow;
    payload->loops = loops;

    kefir_result_t res = kefir_hashtable_init(&payload->edge_freqs, &kefir_hashtable_uint_ops);
    REQUIRE_CHAIN(&res, kefir_hashtable_init(&payload->continuation_benefit, &kefir_hashtable_uint_ops));
    REQUIRE_CHAIN(&res, kefir_hashtable_init(&payload->chain_index, &kefir_hashtable_uint_ops));
    REQUIRE_CHAIN(&res, kefir_hashtable_init(&payload->chains, &kefir_hashtable_uint_ops));
    REQUIRE_CHAIN(&res, kefir_hashtable_on_removal(&payload->chains, free_chain, NULL));
    REQUIRE_CHAIN(&res, kefir_hashset_init(&payload->unchained_blocks, &kefir_hashtable_uint_ops));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, payload);
        return res;
    });

    scheduler->do_schedule = do_schedule;
    scheduler->payload = payload;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_chain_scheduler_free(
    struct kefir_mem *mem, struct kefir_codegen_target_ir_code_scheduler *scheduler) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(scheduler != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR scheduler"));
    ASSIGN_DECL_CAST(struct scheduler_payload *, scheduler_payload, scheduler->payload);

    REQUIRE_OK(kefir_hashset_free(mem, &scheduler_payload->unchained_blocks));
    REQUIRE_OK(kefir_hashtable_free(mem, &scheduler_payload->chain_index));
    REQUIRE_OK(kefir_hashtable_free(mem, &scheduler_payload->chains));
    REQUIRE_OK(kefir_hashtable_free(mem, &scheduler_payload->continuation_benefit));
    REQUIRE_OK(kefir_hashtable_free(mem, &scheduler_payload->edge_freqs));
    KEFIR_FREE(mem, scheduler->payload);
    scheduler->payload = NULL;
    return KEFIR_OK;
}
