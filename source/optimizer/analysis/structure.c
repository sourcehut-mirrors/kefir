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

#include "kefir/optimizer/structure.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_opt_code_structure_init(struct kefir_opt_code_structure *structure) {
    REQUIRE(structure != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code structure"));

    REQUIRE_OK(kefir_hashtreeset_init(&structure->indirect_jump_target_blocks, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_bucketset_init(&structure->sequenced_before, &kefir_bucketset_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&structure->sequence_numbering, &kefir_hashtree_uint_ops));
    structure->next_seq_number = 0;
    structure->blocks = NULL;
    structure->code = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_structure_free(struct kefir_mem *mem, struct kefir_opt_code_structure *structure) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code structure"));

    if (structure->code != NULL) {
        REQUIRE_OK(kefir_hashtree_free(mem, &structure->sequence_numbering));
        REQUIRE_OK(kefir_hashtreeset_free(mem, &structure->indirect_jump_target_blocks));
        REQUIRE_OK(kefir_bucketset_free(mem, &structure->sequenced_before));

        for (kefir_size_t i = 0; i < structure->num_of_blocks; i++) {
            REQUIRE_OK(kefir_list_free(mem, &structure->blocks[i].predecessors));
            REQUIRE_OK(kefir_list_free(mem, &structure->blocks[i].successors));
        }
        KEFIR_FREE(mem, structure->blocks);
        memset(structure, 0, sizeof(struct kefir_opt_code_structure));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_structure_build(struct kefir_mem *mem, struct kefir_opt_code_structure *structure,
                                              const struct kefir_opt_code_container *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code structure"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(structure->code == NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Optimizer code structure has already been built"));

    kefir_result_t res;
    structure->code = code;
    REQUIRE_OK(kefir_opt_code_container_block_count(structure->code, &structure->num_of_blocks));
    structure->blocks = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_structure_block) * structure->num_of_blocks);
    REQUIRE(structure->blocks != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer code structure blocks"));
    for (kefir_size_t i = 0; i < structure->num_of_blocks; i++) {
        structure->blocks[i].immediate_dominator = KEFIR_ID_NONE;
        res = kefir_list_init(&structure->blocks[i].predecessors);
        REQUIRE_CHAIN(&res, kefir_list_init(&structure->blocks[i].successors));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, structure->blocks);
            memset(structure, 0, sizeof(struct kefir_opt_code_structure));
            return res;
        });
    }
    res = kefir_hashtreeset_init(&structure->indirect_jump_target_blocks, &kefir_hashtree_uint_ops);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, structure->blocks);
        memset(structure, 0, sizeof(struct kefir_opt_code_structure));
        return res;
    });

    res = kefir_opt_code_structure_link_blocks(mem, structure);
    REQUIRE_CHAIN(&res, kefir_opt_code_structure_find_dominators(mem, structure));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_structure_free(mem, structure);
        kefir_opt_code_structure_init(structure);
        return res;
    });

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_structure_drop_sequencing_cache(struct kefir_mem *mem,
                                                              struct kefir_opt_code_structure *structure) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code structure"));

    structure->next_seq_number = 0;
    REQUIRE_OK(kefir_bucketset_clean(mem, &structure->sequenced_before));
    REQUIRE_OK(kefir_hashtree_clean(mem, &structure->sequence_numbering));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_structure_is_dominator(const struct kefir_opt_code_structure *structure,
                                                     kefir_opt_block_id_t dominated_block,
                                                     kefir_opt_block_id_t dominator_block, kefir_bool_t *result_ptr) {
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code structure"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    if (dominated_block == dominator_block) {
        *result_ptr = true;
    } else if (structure->blocks[dominated_block].immediate_dominator != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_structure_is_dominator(
            structure, structure->blocks[dominated_block].immediate_dominator, dominator_block, result_ptr));
    } else {
        *result_ptr = false;
    }
    return KEFIR_OK;
}

static kefir_result_t instr_sequence_number(struct kefir_mem *, struct kefir_opt_code_structure *,
                                            kefir_opt_instruction_ref_t, kefir_size_t *, struct kefir_hashtreeset *);

struct instr_input_sequence_number_param {
    struct kefir_mem *mem;
    struct kefir_opt_code_structure *structure;
    kefir_opt_block_id_t block_id;
    struct kefir_hashtreeset *visited;
    kefir_size_t max_seq_number;
};

static kefir_result_t instr_input_sequence_number(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct instr_input_sequence_number_param *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid instruction sequence number parameter"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(param->structure->code, instr_ref, &instr));
    REQUIRE(instr->block_id == param->block_id, KEFIR_OK);

    kefir_size_t seq_num;
    REQUIRE_OK(instr_sequence_number(param->mem, param->structure, instr_ref, &seq_num, param->visited));
    param->max_seq_number = MAX(param->max_seq_number, seq_num);
    return KEFIR_OK;
}

static kefir_result_t instr_sequence_number(struct kefir_mem *mem, struct kefir_opt_code_structure *structure,
                                            kefir_opt_instruction_ref_t instr_ref, kefir_size_t *seq_number,
                                            struct kefir_hashtreeset *visited) {
    REQUIRE(!kefir_hashtreeset_has(visited, (kefir_hashtreeset_entry_t) instr_ref),
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Detected a loop in optimizer instruction dependencies"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&structure->sequence_numbering, (kefir_hashtree_key_t) instr_ref, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_PTR(seq_number, (kefir_size_t) node->value);
        return KEFIR_OK;
    }

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(structure->code, instr_ref, &instr));

    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(structure->code, instr->block_id, &block));

    kefir_bool_t instr_control_flow;
    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(structure->code, instr_ref, &instr_control_flow));

    if (instr_control_flow) {
#define CONTROL_FLOW_SEQ_STEP (1ull << 32)
        kefir_size_t seq_num = CONTROL_FLOW_SEQ_STEP;
        kefir_opt_instruction_ref_t control_flow_iter;
        for (res = kefir_opt_code_block_instr_control_head(structure->code, block, &control_flow_iter);
             res == KEFIR_OK && control_flow_iter != KEFIR_ID_NONE;
             res = kefir_opt_instruction_next_control(structure->code, control_flow_iter, &control_flow_iter),
            seq_num += CONTROL_FLOW_SEQ_STEP) {
            res = kefir_hashtree_at(&structure->sequence_numbering, (kefir_hashtree_key_t) control_flow_iter, &node);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                REQUIRE((kefir_size_t) node->value == seq_num,
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Control flow sequence number mismatch"));
                continue;
            }
            REQUIRE_OK(kefir_hashtree_insert(mem, &structure->sequence_numbering,
                                             (kefir_hashtree_key_t) control_flow_iter,
                                             (kefir_hashtree_value_t) seq_num));
            if (instr_ref == control_flow_iter) {
                ASSIGN_PTR(seq_number, seq_num);
                return KEFIR_OK;
            }
        }
#undef CONTROL_FLOW_SEQ_STEP
        REQUIRE_OK(res);
        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find control flow element in block control flow");
    }

    REQUIRE_OK(kefir_hashtreeset_add(mem, visited, (kefir_hashtreeset_entry_t) instr_ref));

    struct instr_input_sequence_number_param param = {
        .mem = mem, .structure = structure, .block_id = block->id, .visited = visited, .max_seq_number = 0};
    REQUIRE_OK(kefir_opt_instruction_extract_inputs(structure->code, instr, true, instr_input_sequence_number, &param));

    REQUIRE_OK(kefir_hashtreeset_delete(mem, visited, (kefir_hashtreeset_entry_t) instr_ref));

    const kefir_size_t instr_seq = param.max_seq_number + 1;
    REQUIRE_OK(kefir_hashtree_insert(mem, &structure->sequence_numbering, (kefir_hashtree_key_t) instr_ref,
                                     (kefir_hashtree_value_t) instr_seq));
    ASSIGN_PTR(seq_number, instr_seq);

    return KEFIR_OK;
}

static kefir_result_t instr_sequence_number_impl(struct kefir_mem *mem, struct kefir_opt_code_structure *structure,
                                                 kefir_opt_instruction_ref_t instr_ref, kefir_size_t *seq_number) {
    struct kefir_hashtreeset visited;
    REQUIRE_OK(kefir_hashtreeset_init(&visited, &kefir_hashtree_uint_ops));
    kefir_result_t res = instr_sequence_number(mem, structure, instr_ref, seq_number, &visited);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &visited);
        return res;
    });
    REQUIRE_OK(kefir_hashtreeset_free(mem, &visited));
    return KEFIR_OK;
}

static kefir_result_t kefir_opt_code_structure_is_locally_sequenced_before(struct kefir_mem *mem,
                                                                           struct kefir_opt_code_structure *structure,
                                                                           kefir_opt_instruction_ref_t instr_ref1,
                                                                           kefir_opt_instruction_ref_t instr_ref2,
                                                                           kefir_bool_t *result_ptr) {
    const struct kefir_opt_instruction *instr1, *instr2;
    REQUIRE_OK(kefir_opt_code_container_instr(structure->code, instr_ref1, &instr1));
    REQUIRE_OK(kefir_opt_code_container_instr(structure->code, instr_ref2, &instr2));
    REQUIRE(instr1->block_id == instr2->block_id,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Provided instructions belong to different optimizer code blocks"));

    kefir_size_t seq1, seq2;
    REQUIRE_OK(instr_sequence_number_impl(mem, structure, instr_ref1, &seq1));
    REQUIRE_OK(instr_sequence_number_impl(mem, structure, instr_ref2, &seq2));
    *result_ptr = seq1 < seq2;

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_structure_is_sequenced_before(struct kefir_mem *mem,
                                                            struct kefir_opt_code_structure *structure,
                                                            kefir_opt_instruction_ref_t instr_ref1,
                                                            kefir_opt_instruction_ref_t instr_ref2,
                                                            kefir_bool_t *result_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code structure"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    if (instr_ref1 == instr_ref2) {
        *result_ptr = false;
        return KEFIR_OK;
    }

    kefir_bucketset_entry_t entry =
        (((kefir_uint64_t) instr_ref1) << 32) | (((kefir_uint64_t) instr_ref2) & ((1ull << 32) - 1));
    if (kefir_bucketset_has(&structure->sequenced_before, entry)) {
        *result_ptr = true;
        return KEFIR_OK;
    }

    const struct kefir_opt_instruction *instr1, *instr2;
    REQUIRE_OK(kefir_opt_code_container_instr(structure->code, instr_ref1, &instr1));
    REQUIRE_OK(kefir_opt_code_container_instr(structure->code, instr_ref2, &instr2));

    if (instr1->block_id == instr2->block_id) {
        REQUIRE_OK(
            kefir_opt_code_structure_is_locally_sequenced_before(mem, structure, instr_ref1, instr_ref2, result_ptr));
    } else {
        REQUIRE_OK(kefir_opt_code_structure_is_dominator(structure, instr2->block_id, instr1->block_id, result_ptr));
    }

    if (*result_ptr) {
        REQUIRE_OK(kefir_bucketset_add(mem, &structure->sequenced_before, entry));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_structure_redirect_edge(struct kefir_mem *mem, struct kefir_opt_code_structure *structure,
                                                      kefir_opt_block_id_t old_source_block_id,
                                                      kefir_opt_block_id_t new_source_block_id,
                                                      kefir_opt_block_id_t target_block_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code structure"));
    REQUIRE(old_source_block_id != target_block_id,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to redirect loop edges"));

    kefir_size_t total_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(structure->code, &total_blocks));

    REQUIRE(old_source_block_id < total_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is out of code container bounds"));
    REQUIRE(new_source_block_id < total_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is out of code container bounds"));
    REQUIRE(target_block_id < total_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is out of code container bounds"));

    for (struct kefir_list_entry *iter = kefir_list_head(&structure->blocks[old_source_block_id].successors);
         iter != NULL;) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, (kefir_uptr_t) iter->value);
        struct kefir_list_entry *next_iter = iter->next;
        if (block_id == target_block_id) {
            REQUIRE_OK(kefir_list_pop(mem, &structure->blocks[old_source_block_id].successors, iter));
        }
        iter = next_iter;
    }
    REQUIRE_OK(kefir_list_insert_after(mem, &structure->blocks[new_source_block_id].successors,
                                       kefir_list_tail(&structure->blocks[new_source_block_id].successors),
                                       (void *) (kefir_uptr_t) target_block_id));

    for (struct kefir_list_entry *iter = kefir_list_head(&structure->blocks[target_block_id].predecessors);
         iter != NULL; iter = iter->next) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, (kefir_uptr_t) iter->value);
        if (block_id == old_source_block_id) {
            iter->value = (void *) (kefir_uptr_t) new_source_block_id;
        }
    }

    if (structure->blocks[target_block_id].immediate_dominator == old_source_block_id) {
        structure->blocks[target_block_id].immediate_dominator = new_source_block_id;
    }

    REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_structure_redirect_edges(struct kefir_mem *mem,
                                                       struct kefir_opt_code_structure *structure,
                                                       kefir_opt_block_id_t old_source_block_id,
                                                       kefir_opt_block_id_t new_source_block_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code structure"));

    kefir_size_t total_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(structure->code, &total_blocks));

    REQUIRE(old_source_block_id < total_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is out of code container bounds"));

    for (const struct kefir_list_entry *iter = kefir_list_head(&structure->blocks[old_source_block_id].successors);
         iter != NULL; iter = kefir_list_head(&structure->blocks[old_source_block_id].successors)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_opt_code_structure_redirect_edge(mem, structure, old_source_block_id, new_source_block_id,
                                                          successor_block_id));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_structure_drop_edge(struct kefir_mem *mem, struct kefir_opt_code_structure *structure,
                                                  kefir_opt_block_id_t src_block_id,
                                                  kefir_opt_block_id_t dst_block_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code structure"));

    kefir_size_t total_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(structure->code, &total_blocks));

    REQUIRE(src_block_id < total_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is out of code container bounds"));
    REQUIRE(dst_block_id < total_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is out of code container bounds"));

    for (struct kefir_list_entry *iter = kefir_list_head(&structure->blocks[src_block_id].successors); iter != NULL;) {
        struct kefir_list_entry *next_iter = iter->next;
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, (kefir_uptr_t) iter->value);
        if (successor_block_id == dst_block_id) {
            REQUIRE_OK(kefir_list_pop(mem, &structure->blocks[src_block_id].successors, iter));
        }
        iter = next_iter;
    }

    for (struct kefir_list_entry *iter = kefir_list_head(&structure->blocks[dst_block_id].predecessors);
         iter != NULL;) {
        struct kefir_list_entry *next_iter = iter->next;
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, predecessor_block_id, (kefir_uptr_t) iter->value);
        if (predecessor_block_id == src_block_id) {
            REQUIRE_OK(kefir_list_pop(mem, &structure->blocks[dst_block_id].predecessors, iter));
        }
        iter = next_iter;
    }

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_structure_is_reachable_from_entry(const struct kefir_opt_code_structure *structure,
                                                                kefir_opt_block_id_t block_id,
                                                                kefir_bool_t *reachable_flag_ptr) {
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code structure"));
    REQUIRE(reachable_flag_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(structure->code, &num_of_blocks));
    REQUIRE(block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is outside of code container bounds"));

    *reachable_flag_ptr =
        block_id == structure->code->entry_point || structure->blocks[block_id].immediate_dominator != KEFIR_ID_NONE;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_structure_block_direct_predecessor(const struct kefir_opt_code_structure *structure,
                                                                 kefir_opt_block_id_t block_id,
                                                                 kefir_opt_block_id_t successor_block_id,
                                                                 kefir_bool_t *result_ptr) {
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code structure"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(structure->code, &num_of_blocks));
    REQUIRE(block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is outside of code container bounds"));
    REQUIRE(successor_block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is outside of code container bounds"));

    const struct kefir_opt_code_block *successor_block;
    REQUIRE_OK(kefir_opt_code_container_block(structure->code, successor_block_id, &successor_block));

    kefir_bool_t found_block_id = false;
    for (const struct kefir_list_entry *iter = kefir_list_head(&structure->blocks[successor_block_id].predecessors);
         !found_block_id && iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, predecessor_block_id, (kefir_uptr_t) iter->value);

        if (predecessor_block_id == block_id) {
            found_block_id = true;
        }
    }

    *result_ptr = found_block_id;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_structure_block_exclusive_direct_predecessor(
    const struct kefir_opt_code_structure *structure, kefir_opt_block_id_t block_id,
    kefir_opt_block_id_t successor_block_id, kefir_bool_t *result_ptr) {
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code structure"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    if (successor_block_id == structure->code->entry_point) {
        *result_ptr = false;
        return KEFIR_OK;
    }

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(structure->code, &num_of_blocks));
    REQUIRE(block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is outside of code container bounds"));
    REQUIRE(successor_block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is outside of code container bounds"));

    const struct kefir_opt_code_block *successor_block;
    REQUIRE_OK(kefir_opt_code_container_block(structure->code, successor_block_id, &successor_block));

    kefir_bool_t result = block_id != successor_block_id && kefir_hashtreeset_empty(&successor_block->public_labels);
    kefir_bool_t found_block_id = false;
    for (const struct kefir_list_entry *iter = kefir_list_head(&structure->blocks[successor_block_id].predecessors);
         result && iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, predecessor_block_id, (kefir_uptr_t) iter->value);

        if (successor_block_id == predecessor_block_id) {
            result = false;
        } else if (predecessor_block_id != block_id) {
            kefir_bool_t reachable_from_entry;
            REQUIRE_OK(kefir_opt_code_structure_is_reachable_from_entry(structure, predecessor_block_id,
                                                                        &reachable_from_entry));
            result = result && !reachable_from_entry;
        } else {
            found_block_id = true;
        }
    }

    *result_ptr = result && found_block_id;
    return KEFIR_OK;
}
