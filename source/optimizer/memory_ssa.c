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

#include "kefir/optimizer/memory_ssa.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_opt_code_memssa_init(struct kefir_opt_code_memssa *memssa) {
    REQUIRE(memssa != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer memory ssa"));

    memssa->nodes = NULL;
    memssa->node_length = 0;
    memssa->node_capacity = 0;
    memssa->blocks = NULL;
    memssa->block_length = 0;
    memssa->block_capacity = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_memssa_free(struct kefir_mem *mem, struct kefir_opt_code_memssa *memssa) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(memssa != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa"));

    for (kefir_size_t i = 0; i < memssa->node_length; i++) {
        switch (memssa->nodes[i].type) {
            case KEFIR_OPT_CODE_MEMSSA_CONSUME_NODE:
            case KEFIR_OPT_CODE_MEMSSA_PRODUCE_NODE:
                // Intentionally left blank
                break;

            case KEFIR_OPT_CODE_MEMSSA_PHI_NODE:
                KEFIR_FREE(mem, memssa->nodes[i].phi.links);
                break;

            case KEFIR_OPT_CODE_MEMSSA_JOIN_NODE:
                KEFIR_FREE(mem, memssa->nodes[i].join.inputs);
                break;
        }
        REQUIRE_OK(kefir_hashset_free(mem, &memssa->nodes[i].uses));
    }
    KEFIR_FREE(mem, memssa->nodes);
    KEFIR_FREE(mem, memssa->blocks);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_memssa_create_block(struct kefir_mem *mem, struct kefir_opt_code_memssa *memssa,
                                                  kefir_opt_block_id_t block_id,
                                                  kefir_opt_code_memssa_node_ref_t output_node_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(memssa != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa"));
    REQUIRE(output_node_ref < memssa->node_length || output_node_ref == KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa node reference"));

    if (block_id >= memssa->block_length) {
        kefir_size_t new_length = block_id + 1024;
        struct kefir_opt_code_memssa_block *new_blocks =
            KEFIR_REALLOC(mem, memssa->blocks, sizeof(struct kefir_opt_code_memssa_block) * new_length);
        REQUIRE(new_blocks != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate memory ssa blocks"));

        for (kefir_size_t i = memssa->block_length; i < new_length; i++) {
            new_blocks[i].output_node_ref = KEFIR_ID_NONE;
        }
        memssa->blocks = new_blocks;
        memssa->block_length = new_length;
        memssa->block_capacity = new_length;
    }

    memssa->blocks[block_id].output_node_ref = output_node_ref;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_memssa_update_block_output(struct kefir_opt_code_memssa *memssa,
                                                         kefir_opt_block_id_t block_id,
                                                         kefir_opt_code_memssa_node_ref_t output_node_ref) {
    REQUIRE(memssa != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa"));
    REQUIRE(block_id < memssa->block_length,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block identifier"));
    REQUIRE(output_node_ref < memssa->node_length || output_node_ref == KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa node reference"));

    memssa->blocks[block_id].output_node_ref = output_node_ref;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_memssa_block(const struct kefir_opt_code_memssa *memssa, kefir_opt_block_id_t block_id,
                                           const struct kefir_opt_code_memssa_block **block_ptr) {
    REQUIRE(memssa != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa"));
    REQUIRE(block_id < memssa->block_length,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested memory ssa block"));

    ASSIGN_PTR(block_ptr, &memssa->blocks[block_id]);
    return KEFIR_OK;
}

static kefir_result_t new_node(struct kefir_mem *mem, struct kefir_opt_code_memssa *memssa,
                               kefir_opt_code_memssa_node_ref_t *node_ref_ptr) {
    if (memssa->node_length + 1 >= memssa->node_capacity) {
        kefir_size_t new_capacity = MAX(memssa->node_length * 2, 128);
        struct kefir_opt_code_memssa_node *new_nodes =
            KEFIR_REALLOC(mem, memssa->nodes, sizeof(struct kefir_opt_code_memssa_node) * new_capacity);
        REQUIRE(new_nodes != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate memory ssa nodes"));

        memssa->nodes = new_nodes;
        memssa->node_capacity = new_capacity;
    }
    *node_ref_ptr = memssa->node_length;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_memssa_new_consume_node(struct kefir_mem *mem, struct kefir_opt_code_memssa *memssa,
                                                      kefir_opt_code_memssa_node_ref_t predecessor_ref,
                                                      kefir_opt_instruction_ref_t instr_ref,
                                                      kefir_opt_code_memssa_node_ref_t *node_ref_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(memssa != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa"));
    REQUIRE(predecessor_ref == KEFIR_ID_NONE || predecessor_ref < memssa->node_length,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa node reference"));

    kefir_opt_code_memssa_node_ref_t node_ref = KEFIR_ID_NONE;
    REQUIRE_OK(new_node(mem, memssa, &node_ref));

    memssa->nodes[node_ref].type = KEFIR_OPT_CODE_MEMSSA_CONSUME_NODE;
    memssa->nodes[node_ref].predecessor_ref = predecessor_ref;
    memssa->nodes[node_ref].instr_ref = instr_ref;
    REQUIRE_OK(kefir_hashset_init(&memssa->nodes[node_ref].uses, &kefir_hashtable_uint_ops));
    if (predecessor_ref != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_hashset_add(mem, &memssa->nodes[predecessor_ref].uses, (kefir_hashset_key_t) node_ref));
    }
    memssa->node_length++;
    ASSIGN_PTR(node_ref_ptr, node_ref);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_memssa_new_produce_node(struct kefir_mem *mem, struct kefir_opt_code_memssa *memssa,
                                                      kefir_opt_code_memssa_node_ref_t predecessor_ref,
                                                      kefir_opt_instruction_ref_t instr_ref,
                                                      kefir_opt_code_memssa_node_ref_t *node_ref_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(memssa != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa"));
    REQUIRE(predecessor_ref == KEFIR_ID_NONE || predecessor_ref < memssa->node_length,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa node reference"));

    kefir_opt_code_memssa_node_ref_t node_ref = KEFIR_ID_NONE;
    REQUIRE_OK(new_node(mem, memssa, &node_ref));

    memssa->nodes[node_ref].type = KEFIR_OPT_CODE_MEMSSA_PRODUCE_NODE;
    memssa->nodes[node_ref].predecessor_ref = predecessor_ref;
    memssa->nodes[node_ref].instr_ref = instr_ref;
    REQUIRE_OK(kefir_hashset_init(&memssa->nodes[node_ref].uses, &kefir_hashtable_uint_ops));
    if (predecessor_ref != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_hashset_add(mem, &memssa->nodes[predecessor_ref].uses, (kefir_hashset_key_t) node_ref));
    }
    memssa->node_length++;
    ASSIGN_PTR(node_ref_ptr, node_ref);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_memssa_new_join_node(struct kefir_mem *mem, struct kefir_opt_code_memssa *memssa,
                                                   kefir_opt_code_memssa_node_ref_t *node_ref_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(memssa != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa"));

    kefir_opt_code_memssa_node_ref_t node_ref = KEFIR_ID_NONE;
    REQUIRE_OK(new_node(mem, memssa, &node_ref));

    memssa->nodes[node_ref].type = KEFIR_OPT_CODE_MEMSSA_JOIN_NODE;
    memssa->nodes[node_ref].join.inputs = NULL;
    memssa->nodes[node_ref].join.input_length = 0;
    REQUIRE_OK(kefir_hashset_init(&memssa->nodes[node_ref].uses, &kefir_hashtable_uint_ops));
    memssa->node_length++;
    ASSIGN_PTR(node_ref_ptr, node_ref);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_memssa_new_phi_node(struct kefir_mem *mem, struct kefir_opt_code_memssa *memssa,
                                                  kefir_opt_code_memssa_node_ref_t *node_ref_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(memssa != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa"));

    kefir_opt_code_memssa_node_ref_t node_ref = KEFIR_ID_NONE;
    REQUIRE_OK(new_node(mem, memssa, &node_ref));

    memssa->nodes[node_ref].type = KEFIR_OPT_CODE_MEMSSA_PHI_NODE;
    memssa->nodes[node_ref].phi.links = NULL;
    memssa->nodes[node_ref].phi.link_count = 0;
    REQUIRE_OK(kefir_hashset_init(&memssa->nodes[node_ref].uses, &kefir_hashtable_uint_ops));
    memssa->node_length++;
    ASSIGN_PTR(node_ref_ptr, node_ref);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_memssa_join_attach(struct kefir_mem *mem, struct kefir_opt_code_memssa *memssa,
                                                 kefir_opt_code_memssa_node_ref_t join_node_ref,
                                                 kefir_opt_code_memssa_node_ref_t node_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(memssa != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa"));
    REQUIRE(join_node_ref < memssa->node_length && memssa->nodes[join_node_ref].type == KEFIR_OPT_CODE_MEMSSA_JOIN_NODE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa join node reference"));
    REQUIRE(node_ref < memssa->node_length || node_ref == KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa node reference"));

    for (kefir_size_t i = 0; i < memssa->nodes[join_node_ref].join.input_length; i++) {
        if (memssa->nodes[join_node_ref].join.inputs[i] == node_ref) {
            return KEFIR_OK;
        }
    }

    kefir_size_t new_length = memssa->nodes[join_node_ref].join.input_length + 1;
    kefir_opt_code_memssa_node_ref_t *new_inputs = KEFIR_REALLOC(mem, memssa->nodes[join_node_ref].join.inputs,
                                                                 sizeof(kefir_opt_code_memssa_node_ref_t) * new_length);
    REQUIRE(new_inputs != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer memory ssa join node"));

    new_inputs[new_length - 1] = node_ref;
    memssa->nodes[join_node_ref].join.inputs = new_inputs;
    memssa->nodes[join_node_ref].join.input_length = new_length;

    REQUIRE_OK(kefir_hashset_add(mem, &memssa->nodes[node_ref].uses, (kefir_hashset_key_t) join_node_ref));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_memssa_phi_attach(struct kefir_mem *mem, struct kefir_opt_code_memssa *memssa,
                                                kefir_opt_code_memssa_node_ref_t phi_node_ref,
                                                kefir_opt_block_id_t block_id,
                                                kefir_opt_code_memssa_node_ref_t node_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(memssa != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa"));
    REQUIRE(phi_node_ref < memssa->node_length && memssa->nodes[phi_node_ref].type == KEFIR_OPT_CODE_MEMSSA_PHI_NODE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa phi node reference"));
    REQUIRE(block_id < memssa->block_length,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa block reference"));
    REQUIRE(node_ref < memssa->node_length || node_ref == KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory ssa node reference"));

    for (kefir_size_t i = 0; i < memssa->nodes[phi_node_ref].phi.link_count; i++) {
        if (memssa->nodes[phi_node_ref].phi.links[i].block_ref == block_id) {
            REQUIRE(memssa->nodes[phi_node_ref].phi.links[i].node_ref == node_ref,
                    KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Memory ssa phi link for the same block already exists"));
            return KEFIR_OK;
        }
    }

    kefir_size_t new_length = memssa->nodes[phi_node_ref].phi.link_count + 1;
    struct kefir_opt_code_memssa_phi_link *new_links = KEFIR_REALLOC(
        mem, memssa->nodes[phi_node_ref].phi.links, sizeof(struct kefir_opt_code_memssa_phi_link) * new_length);
    REQUIRE(new_links != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer memory ssa phi node"));

    new_links[new_length - 1].block_ref = block_id;
    new_links[new_length - 1].node_ref = node_ref;
    memssa->nodes[phi_node_ref].phi.links = new_links;
    memssa->nodes[phi_node_ref].phi.link_count = new_length;

    REQUIRE_OK(kefir_hashset_add(mem, &memssa->nodes[node_ref].uses, (kefir_hashset_key_t) phi_node_ref));
    return KEFIR_OK;
}
