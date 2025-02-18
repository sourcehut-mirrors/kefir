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

#include "kefir/optimizer/liveness.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/queue.h"
#include "kefir/core/bitset.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

struct verify_use_def_payload {
    struct kefir_mem *mem;
    struct kefir_opt_code_liveness *liveness;
    struct kefir_opt_code_structure *structure;
    kefir_opt_instruction_ref_t instr_ref;
};

static kefir_result_t verify_use_def_impl(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct verify_use_def_payload *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code use-def verified parameters"));

    kefir_bool_t sequenced_before;
    REQUIRE_OK(kefir_opt_code_structure_is_sequenced_before(param->mem, param->structure, instr_ref, param->instr_ref,
                                                            &sequenced_before));
    REQUIRE(sequenced_before, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Reversed use-define chain in optimizer code"));

    return KEFIR_OK;
}

static kefir_result_t verify_use_def(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct verify_use_def_payload *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code use-def verified parameters"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(param->structure->code, instr_ref, &instr));

    REQUIRE_OK(kefir_bucketset_add(param->mem, &param->liveness->blocks[instr->block_id].alive_instr,
                                   (kefir_hashtreeset_entry_t) instr_ref));

    param->instr_ref = instr_ref;
    REQUIRE_OK(kefir_opt_instruction_extract_inputs(param->structure->code, instr, true, verify_use_def_impl, payload));

    struct kefir_opt_instruction_use_iterator use_iter;
    kefir_result_t res;
    for (res = kefir_opt_code_container_instruction_use_instr_iter(param->structure->code, instr_ref, &use_iter);
         res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_instruction *use_instr;
        res = kefir_opt_code_container_instr(param->structure->code, use_iter.use_instr_ref, &use_instr);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        REQUIRE_OK(kefir_bucketset_add(param->mem, &param->liveness->blocks[use_instr->block_id].alive_instr,
                                       (kefir_bucketset_entry_t) instr_ref));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (res = kefir_opt_code_container_instruction_use_phi_iter(param->structure->code, instr_ref, &use_iter);
         res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_phi_node *use_phi;
        res = kefir_opt_code_container_phi(param->structure->code, use_iter.use_phi_ref, &use_phi);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        struct kefir_hashtree_node_iterator iter;
        for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&use_phi->links, &iter); node != NULL;
             node = kefir_hashtree_next(&iter)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, src_block_id, node->key);
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, src_instr_ref, node->value);
            if (src_instr_ref == instr_ref) {
                REQUIRE_OK(kefir_bucketset_add(param->mem, &param->liveness->blocks[src_block_id].alive_instr,
                                               (kefir_bucketset_entry_t) instr_ref));
            }
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (res = kefir_opt_code_container_instruction_use_call_iter(param->structure->code, instr_ref, &use_iter);
         res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_call_node *use_call;
        res = kefir_opt_code_container_call(param->structure->code, use_iter.use_call_ref, &use_call);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        REQUIRE_OK(kefir_bucketset_add(param->mem, &param->liveness->blocks[use_call->block_id].alive_instr,
                                       (kefir_bucketset_entry_t) instr_ref));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (res = kefir_opt_code_container_instruction_use_inline_asm_iter(param->structure->code, instr_ref, &use_iter);
         res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_inline_assembly_node *use_inline_asm;
        res = kefir_opt_code_container_inline_assembly(param->structure->code, use_iter.use_inline_asm_ref,
                                                       &use_inline_asm);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        REQUIRE_OK(kefir_bucketset_add(param->mem, &param->liveness->blocks[use_inline_asm->block_id].alive_instr,
                                       (kefir_bucketset_entry_t) instr_ref));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t propagate_alive_instructions_impl(struct kefir_mem *mem, struct kefir_opt_code_liveness *liveness,
                                                        const struct kefir_opt_code_structure *structure,
                                                        kefir_opt_block_id_t block_id,
                                                        struct kefir_bitset *visited_blocks,
                                                        struct kefir_queue *queue) {
    kefir_result_t res;
    struct kefir_bucketset_iterator iter;
    kefir_bucketset_entry_t entry;

    for (res = kefir_bucketset_iter(&liveness->blocks[block_id].alive_instr, &iter, &entry); res == KEFIR_OK;
         res = kefir_bucketset_next(&iter, &entry)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, entry);

        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(liveness->code, instr_ref, &instr));
        if (instr->block_id == block_id) {
            continue;
        }

        REQUIRE_OK(kefir_bitset_clear(visited_blocks));
#define ADD_PREDS(_block_id)                                                                                       \
    do {                                                                                                           \
        for (const struct kefir_list_entry *iter2 = kefir_list_head(&structure->blocks[(_block_id)].predecessors); \
             iter2 != NULL; kefir_list_next(&iter2)) {                                                             \
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, pred_block_id, (kefir_uptr_t) iter2->value);                    \
            REQUIRE_OK(kefir_queue_push(mem, queue, (kefir_queue_entry_t) pred_block_id));                         \
        }                                                                                                          \
    } while (0)
        ADD_PREDS(block_id);

        while (!kefir_queue_is_empty(queue)) {
            kefir_queue_entry_t entry;
            REQUIRE_OK(kefir_queue_pop_first(mem, queue, &entry));
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, current_block_id, entry);

            kefir_bool_t visited;
            REQUIRE_OK(kefir_bitset_get(visited_blocks, current_block_id, &visited));
            if (visited || kefir_bucketset_has(&liveness->blocks[current_block_id].alive_instr,
                                               (kefir_hashtreeset_entry_t) instr_ref)) {
                continue;
            }

            REQUIRE_OK(kefir_bitset_set(visited_blocks, current_block_id, true));

            if (current_block_id != instr->block_id) {
                REQUIRE_OK(kefir_bucketset_add(mem, &liveness->blocks[current_block_id].alive_instr,
                                               (kefir_bucketset_entry_t) instr_ref));
                ADD_PREDS(current_block_id);
            }
        }
#undef ADD_PREFS
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t propagate_alive_instructions(struct kefir_mem *mem, struct kefir_opt_code_liveness *liveness,
                                                   struct kefir_opt_code_structure *structure) {
    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(liveness->code, &num_of_blocks));

    struct kefir_bitset visited_blocks;
    struct kefir_queue queue;
    REQUIRE_OK(kefir_bitset_init(&visited_blocks));
    REQUIRE_OK(kefir_queue_init(&queue));

    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN(&res, kefir_bitset_ensure(mem, &visited_blocks, num_of_blocks));
    for (kefir_opt_block_id_t block_id = 0; res == KEFIR_OK && block_id < num_of_blocks; block_id++) {
        res = propagate_alive_instructions_impl(mem, liveness, structure, block_id, &visited_blocks, &queue);
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_bitset_free(mem, &visited_blocks);
        kefir_queue_free(mem, &queue);
        return res;
    });
    res = kefir_bitset_free(mem, &visited_blocks);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_queue_free(mem, &queue);
        return res;
    });
    REQUIRE_OK(kefir_queue_free(mem, &queue));
    return KEFIR_OK;
}

static kefir_result_t trace_use_def(struct kefir_mem *mem, struct kefir_opt_code_liveness *liveness,
                                    struct kefir_opt_code_structure *structure) {
    struct verify_use_def_payload payload = {
        .mem = mem, .liveness = liveness, .structure = structure, .instr_ref = KEFIR_ID_NONE};
    struct kefir_opt_code_container_tracer tracer = {.trace_instruction = verify_use_def, .payload = &payload};
    REQUIRE_OK(kefir_opt_code_container_trace(mem, liveness->code, &tracer));

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(liveness->code, &num_of_blocks));
    REQUIRE_OK(propagate_alive_instructions(mem, liveness, structure));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_liveness_init(struct kefir_opt_code_liveness *liveness) {
    REQUIRE(liveness != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code liveness"));

    liveness->code = NULL;
    liveness->blocks = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_liveness_free(struct kefir_mem *mem, struct kefir_opt_code_liveness *liveness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code liveness"));

    if (liveness->code != NULL) {
        kefir_size_t num_of_blocks;
        REQUIRE_OK(kefir_opt_code_container_block_count(liveness->code, &num_of_blocks));
        for (kefir_size_t i = 0; i < num_of_blocks; i++) {
            REQUIRE_OK(kefir_bucketset_free(mem, &liveness->blocks[i].alive_instr));
        }
        KEFIR_FREE(mem, liveness->blocks);
        memset(liveness, 0, sizeof(struct kefir_opt_code_liveness));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_liveness_build(struct kefir_mem *mem, struct kefir_opt_code_liveness *liveness,
                                             struct kefir_opt_code_structure *structure) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code liveness"));
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code structure"));
    REQUIRE(liveness->code == NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Optimizer code liveness has already been built"));

    liveness->code = structure->code;
    kefir_result_t res;
    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(liveness->code, &num_of_blocks));
    liveness->blocks = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_liveness_block) * num_of_blocks);
    REQUIRE(liveness->blocks != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer code liveness blocks"));
    for (kefir_size_t i = 0; i < num_of_blocks; i++) {
        res = kefir_bucketset_init(&liveness->blocks[i].alive_instr, &kefir_bucketset_uint_ops);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, liveness->blocks);
            memset(liveness, 0, sizeof(struct kefir_opt_code_liveness));
        });
    }

    res = trace_use_def(mem, liveness, structure);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_liveness_free(mem, liveness);
        kefir_opt_code_liveness_init(liveness);
        return res;
    });

    return KEFIR_OK;
}
