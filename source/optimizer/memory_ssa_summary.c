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

#include "kefir/optimizer/memory_ssa_summary.h"
#include "kefir/optimizer/mem2reg_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_opt_code_memssa_summary_init(struct kefir_opt_code_memssa_summary *summary) {
    REQUIRE(summary != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to memory SSA summary"));

    summary->chains = NULL;
    summary->chains_length = 0;
    summary->chains_capacity = 0;
    summary->refs = NULL;
    summary->refs_length = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_memssa_summary_free(struct kefir_mem *mem,
                                                  struct kefir_opt_code_memssa_summary *summary) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(summary != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory SSA summary"));

    for (kefir_size_t i = 0; i < summary->chains_length; i++) {
        KEFIR_FREE(mem, summary->chains[i]);
    }
    KEFIR_FREE(mem, summary->chains);
    KEFIR_FREE(mem, summary->refs);
    return KEFIR_OK;
}

#define CHAIN_NONE ((kefir_uint32_t) ~0ull)
#define CHAIN_MIN_CAPACITY 8

#include <stdio.h>

static kefir_result_t assign_chain(struct kefir_mem *mem, struct kefir_opt_code_memssa_summary *summary,
                                   const struct kefir_opt_code_container *code,
                                   kefir_opt_code_memssa_node_ref_t node_ref,
                                   const struct kefir_opt_code_memssa_node *node, kefir_uint32_t *chain_idx) {
    REQUIRE(node->instr_ref != KEFIR_ID_NONE, KEFIR_OK);

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, node->instr_ref, &instr));

    kefir_opt_instruction_ref_t location_ref = KEFIR_ID_NONE;
    kefir_size_t size = 0;
    kefir_int64_t offset = 0;

    kefir_result_t res = kefir_opt_code_util_classify_memory_access(instr, &location_ref, &size, &offset);
    if (res == KEFIR_NO_MATCH) {
        location_ref = node->instr_ref;
        size = 0;
        offset = 0;
        res = KEFIR_OK;
    }
    REQUIRE_OK(res);

    if (*chain_idx == CHAIN_NONE) {
        if (summary->chains_length >= summary->chains_capacity) {
            kefir_size_t new_capacity = MAX(8, 2 * summary->chains_capacity);
            struct kefir_opt_code_memssa_chain **new_chains =
                KEFIR_REALLOC(mem, summary->chains, sizeof(struct kefir_opt_code_memssa_chain *) * new_capacity);
            REQUIRE(new_chains != NULL,
                    KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate memory SSA summary chains"));

            summary->chains_capacity = new_capacity;
            summary->chains = new_chains;
        }

        *chain_idx = summary->chains_length++;
        summary->chains[*chain_idx] =
            KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_memssa_chain) +
                                  sizeof(struct kefir_opt_code_memssa_chain_entry) * CHAIN_MIN_CAPACITY);
        REQUIRE(summary->chains[*chain_idx] != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate memory SSA summary chain"));

        summary->chains[*chain_idx]->length = 0;
        summary->chains[*chain_idx]->capacity = CHAIN_MIN_CAPACITY;
    }

    struct kefir_opt_code_memssa_chain *chain = summary->chains[*chain_idx];
    if (chain->length >= chain->capacity) {
        kefir_size_t new_capacity = MAX(CHAIN_MIN_CAPACITY, 2 * chain->capacity);
        struct kefir_opt_code_memssa_chain *new_chain =
            KEFIR_REALLOC(mem, chain,
                          sizeof(struct kefir_opt_code_memssa_chain) +
                              sizeof(struct kefir_opt_code_memssa_chain_entry) * new_capacity);
        REQUIRE(new_chain != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate memory SSA summary chain"));

        new_chain->capacity = new_capacity;
        summary->chains[*chain_idx] = new_chain;
        chain = new_chain;
    }

    kefir_uint32_t chain_offset = chain->length++;
    chain->entries[chain_offset].node_ref = node_ref;
    chain->entries[chain_offset].location_ref = location_ref;
    chain->entries[chain_offset].offset = offset;
    chain->entries[chain_offset].size = size;

    summary->refs[node_ref].chain = *chain_idx;
    summary->refs[node_ref].offset = chain_offset;

    return KEFIR_OK;
}

kefir_result_t build_impl(struct kefir_mem *mem, struct kefir_opt_code_memssa_summary *summary,
                          const struct kefir_opt_code_container *code, const struct kefir_opt_code_memssa *memssa,
                          struct kefir_list *queue, struct kefir_hashset *visited) {

    summary->refs = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_memssa_chain_ref) * memssa->node_length);
    REQUIRE(summary->refs != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate memory SSA summary references"));
    memset(summary->refs, -1, sizeof(struct kefir_opt_code_memssa_chain_ref) * memssa->node_length);
    summary->refs_length = memssa->node_length;

    if (memssa->root_ref != KEFIR_ID_NONE) {
        kefir_uint64_t key = (((kefir_uint64_t) memssa->root_ref) << 32) | CHAIN_NONE;
        REQUIRE_OK(kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) key));
    }

    kefir_result_t res;
    for (struct kefir_list_entry *iter = kefir_list_head(queue); iter != NULL; iter = kefir_list_head(queue)) {
        ASSIGN_DECL_CAST(kefir_uint64_t, key, (kefir_uptr_t) iter->value);
        kefir_opt_code_memssa_node_ref_t node_ref = key >> 32;
        kefir_uint32_t chain_idx = (kefir_uint32_t) key;
        REQUIRE_OK(kefir_list_pop(mem, queue, iter));
        if (kefir_hashset_has(visited, (kefir_hashset_key_t) node_ref)) {
            continue;
        }
        REQUIRE_OK(kefir_hashset_add(mem, visited, (kefir_hashset_key_t) node_ref));

        const struct kefir_opt_code_memssa_node *node;
        REQUIRE_OK(kefir_opt_code_memssa_node(memssa, node_ref, &node));

        switch (node->type) {
            case KEFIR_OPT_CODE_MEMSSA_ROOT_NODE:
            case KEFIR_OPT_CODE_MEMSSA_TERMINATE_NODE:
            case KEFIR_OPT_CODE_MEMSSA_CONSUME_NODE:
                // Intentionally left blank
                break;

            case KEFIR_OPT_CODE_MEMSSA_PHI_NODE:
                chain_idx = CHAIN_NONE;
                break;

            case KEFIR_OPT_CODE_MEMSSA_PRODUCE_NODE:
            case KEFIR_OPT_CODE_MEMSSA_PRODUCE_CONSUME_NODE:
                REQUIRE_OK(assign_chain(mem, summary, code, node_ref, node, &chain_idx));
                break;
        }

        if (node->uses.occupied > 1) {
            chain_idx = CHAIN_NONE;
        }

        struct kefir_opt_code_memssa_use_iterator use_iter;
        kefir_opt_code_memssa_node_ref_t use_node_ref;
        for (res = kefir_opt_code_memssa_use_iter(memssa, &use_iter, node_ref, &use_node_ref); res == KEFIR_OK;
             res = kefir_opt_code_memssa_use_next(&use_iter, &use_node_ref)) {
            kefir_uint64_t key = (((kefir_uint64_t) use_node_ref) << 32) | (kefir_uint32_t) chain_idx;
            REQUIRE_OK(kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) key));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_memssa_summary_build(struct kefir_mem *mem, struct kefir_opt_code_memssa_summary *summary,
                                                   const struct kefir_opt_code_container *code,
                                                   const struct kefir_opt_code_memssa *memssa) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(summary != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory SSA summary"));
    REQUIRE(summary->refs == NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected clean memory SSA summary"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(memssa != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory SSA"));

    struct kefir_list queue;
    struct kefir_hashset visited;
    REQUIRE_OK(kefir_list_init(&queue));
    REQUIRE_OK(kefir_hashset_init(&visited, &kefir_hashtable_uint_ops));
    kefir_result_t res = build_impl(mem, summary, code, memssa, &queue, &visited);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &queue);
        kefir_hashset_free(mem, &visited);
        return res;
    });
    res = kefir_list_free(mem, &queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &visited);
        return res;
    });
    REQUIRE_OK(kefir_hashset_free(mem, &visited));

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_memssa_summary_chain_of(const struct kefir_opt_code_memssa_summary *summary,
                                                      kefir_opt_code_memssa_node_ref_t node_ref,
                                                      const struct kefir_opt_code_memssa_chain **chain_ptr,
                                                      kefir_uint32_t *offset_ptr) {
    REQUIRE(summary != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory SSA summary"));
    REQUIRE(node_ref != KEFIR_ID_NONE && node_ref < summary->refs_length,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested memory SSA summary chain"));
    REQUIRE(summary->refs[node_ref].chain != CHAIN_NONE,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested memory SSA summary chain"));

    ASSIGN_PTR(chain_ptr, summary->chains[summary->refs[node_ref].chain]);
    ASSIGN_PTR(offset_ptr, summary->refs[node_ref].offset);
    return KEFIR_OK;
}
