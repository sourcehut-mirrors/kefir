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

#include "kefir/optimizer/liveness.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

struct extract_inputs_payload {
    struct kefir_mem *mem;
    struct kefir_opt_code_control_flow *control_flow;
    struct kefir_opt_code_liveness *liveness;
    struct kefir_list *queue;
    struct kefir_hashset *visited;
    kefir_opt_block_id_t block_id;
};

static kefir_result_t propagate_liveness(struct kefir_mem *mem, struct kefir_opt_code_control_flow *control_flow, struct kefir_opt_code_liveness *liveness, kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t instr_ref,
    struct kefir_list *queue) {
    REQUIRE_OK(kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) block_id));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(liveness->code, instr_ref, &instr));

    for (struct kefir_list_entry *iter = kefir_list_head(queue); iter != NULL; iter = kefir_list_head(queue)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(mem, queue, iter));
        if (kefir_hashset_has(&liveness->blocks[block_id].alive_instr, (kefir_hashset_key_t) instr_ref)) {
            continue;
        }

        REQUIRE_OK(kefir_hashset_add(mem, &liveness->blocks[block_id].alive_instr, (kefir_hashset_key_t) instr_ref));
        if (block_id == instr->block_id) {
            continue;
        }

        kefir_result_t res;
        struct kefir_hashset_iterator iter;
        kefir_hashset_key_t entry;
        for (res = kefir_hashset_iter(&control_flow->blocks[block_id].predecessors, &iter, &entry); res == KEFIR_OK;
            res = kefir_hashset_next(&iter, &entry)) {
            REQUIRE_OK(kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) entry));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_impl(
    struct kefir_mem *mem,
    struct kefir_opt_code_control_flow *control_flow,
    struct kefir_opt_code_liveness *liveness,
    struct kefir_list *queue,
    struct kefir_hashset *enqueued,
    kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t instr_ref) {
    if (!kefir_hashset_has(enqueued, (kefir_hashset_key_t) instr_ref)) {
        REQUIRE_OK(kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) instr_ref));
        REQUIRE_OK(kefir_hashset_add(mem, enqueued, (kefir_hashset_key_t) instr_ref));
    }

    struct kefir_list block_queue;
    REQUIRE_OK(kefir_list_init(&block_queue));
    kefir_result_t res = propagate_liveness(mem, control_flow, liveness, block_id, instr_ref, &block_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &block_queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &block_queue));
    return KEFIR_OK;
}

static kefir_result_t extract_inputs(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct extract_inputs_payload *, params, payload);
    REQUIRE(params != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer liveness instruction inputs payload"));

    REQUIRE_OK(extract_inputs_impl(params->mem, params->control_flow, params->liveness, params->queue, params->visited, params->block_id, instr_ref));
    return KEFIR_OK;
}

static kefir_result_t build_liveness(struct kefir_mem *mem, struct kefir_opt_code_liveness *liveness,
                                    struct kefir_opt_code_control_flow *control_flow, struct kefir_list *queue, struct kefir_hashset *visited) {
    for (kefir_opt_block_id_t block_id = 0; block_id < kefir_opt_code_container_block_count(control_flow->code); block_id++) {
        kefir_bool_t reachable;
        REQUIRE_OK(kefir_opt_code_control_flow_is_reachable_from_entry(control_flow, block_id, &reachable));
        if (!reachable) {
            continue;
        }

        kefir_result_t res;
        kefir_opt_instruction_ref_t instr_ref;
        for (res = kefir_opt_code_block_instr_control_head(control_flow->code, block_id, &instr_ref); res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
            res = kefir_opt_instruction_next_control(control_flow->code, instr_ref, &instr_ref)) {
            REQUIRE_OK(kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) instr_ref));
            REQUIRE_OK(kefir_hashset_add(mem, visited, (kefir_hashset_key_t) instr_ref));
        }
        REQUIRE_OK(res);
    }

    struct extract_inputs_payload payload = {
        .mem = mem,
        .control_flow = control_flow,
        .liveness = liveness,
        .queue = queue,
        .visited = visited
    };
    for (struct kefir_list_entry *iter = kefir_list_head(queue); iter != NULL; iter = kefir_list_head(queue)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(mem, queue, iter));
        
        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(control_flow->code, instr_ref, &instr));

        REQUIRE_OK(kefir_hashset_add(mem, &liveness->blocks[instr->block_id].alive_instr, (kefir_hashset_key_t) instr_ref));

        if (instr->operation.opcode == KEFIR_OPT_OPCODE_PHI) {
            const struct kefir_opt_phi_node *phi_node;
            REQUIRE_OK(kefir_opt_code_container_phi(control_flow->code, instr->operation.parameters.phi_ref,
                                                    &phi_node));
            struct kefir_hashtree_node_iterator iter;
            for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&phi_node->links, &iter); node != NULL;
                 node = kefir_hashtree_next(&iter)) {
                ASSIGN_DECL_CAST(kefir_opt_block_id_t, src_block_id, node->key);
                ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, src_instr_ref, node->value);
                
                kefir_bool_t reachable;
                REQUIRE_OK(kefir_opt_code_control_flow_is_reachable_from_entry(control_flow, src_block_id, &reachable));
                if (!reachable) {
                    continue;
                }

                REQUIRE_OK(extract_inputs_impl(mem, control_flow, liveness, queue, visited, src_block_id, src_instr_ref));
            }
        } else {
            payload.block_id = instr->block_id;
            REQUIRE_OK(kefir_opt_instruction_extract_inputs(control_flow->code, instr, true, extract_inputs, &payload));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_liveness_init(struct kefir_opt_code_liveness *liveness) {
    REQUIRE(liveness != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code liveness"));

    liveness->code = NULL;
    liveness->blocks = NULL;
    liveness->num_of_blocks = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_liveness_free(struct kefir_mem *mem, struct kefir_opt_code_liveness *liveness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code liveness"));

    if (liveness->code != NULL) {
        for (kefir_size_t i = 0; i < liveness->num_of_blocks; i++) {
            REQUIRE_OK(kefir_hashset_free(mem, &liveness->blocks[i].alive_instr));
        }
        KEFIR_FREE(mem, liveness->blocks);
        memset(liveness, 0, sizeof(struct kefir_opt_code_liveness));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_liveness_reset(struct kefir_mem *mem, struct kefir_opt_code_liveness *liveness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code liveness"));

    if (liveness->code != NULL) {
        for (kefir_size_t i = 0; i < liveness->num_of_blocks; i++) {
            REQUIRE_OK(kefir_hashset_free(mem, &liveness->blocks[i].alive_instr));
        }
        KEFIR_FREE(mem, liveness->blocks);
        memset(liveness, 0, sizeof(struct kefir_opt_code_liveness));
    }
    liveness->code = NULL;
    liveness->blocks = NULL;
    liveness->num_of_blocks = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_liveness_build(struct kefir_mem *mem, struct kefir_opt_code_liveness *liveness,
                                             struct kefir_opt_code_control_flow *control_flow) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code liveness"));
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));
    REQUIRE(liveness->code == NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Optimizer code liveness has already been built"));

    liveness->code = control_flow->code;
    kefir_result_t res;
    liveness->num_of_blocks = kefir_opt_code_container_block_count(liveness->code);
    liveness->blocks = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_liveness_block) * liveness->num_of_blocks);
    REQUIRE(liveness->blocks != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer code liveness blocks"));
    for (kefir_size_t i = 0; i < liveness->num_of_blocks; i++) {
        res = kefir_hashset_init(&liveness->blocks[i].alive_instr, &kefir_hashtable_uint_ops);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, liveness->blocks);
            memset(liveness, 0, sizeof(struct kefir_opt_code_liveness));
            return res;
        });
    }

    struct kefir_list queue;
    struct kefir_hashset visited;
    
    REQUIRE_OK(kefir_list_init(&queue));
    REQUIRE_OK(kefir_hashset_init(&visited, &kefir_hashtable_uint_ops));
    res = build_liveness(mem, liveness, control_flow, &queue, &visited);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &visited);
        kefir_list_free(mem, &queue);
        return res;
    });
    res = kefir_hashset_free(mem, &visited);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &queue));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_liveness_instruction_is_alive(const struct kefir_opt_code_liveness *liveness,
                                                            kefir_opt_instruction_ref_t instr_ref,
                                                            kefir_bool_t *alive_ptr) {
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code liveness"));
    REQUIRE(alive_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(liveness->code, instr_ref, &instr));

    *alive_ptr = kefir_hashset_has(&liveness->blocks[instr->block_id].alive_instr, (kefir_hashset_key_t) instr_ref);
    return KEFIR_OK;
}
