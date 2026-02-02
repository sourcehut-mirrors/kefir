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

#define KEFIR_OPTIMIZER_CONTROL_FLOW_INTERNAL
#include "kefir/optimizer/control_flow.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_opt_code_control_flow_init(struct kefir_opt_code_control_flow *control_flow) {
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code control flow"));

    REQUIRE_OK(kefir_hashtreeset_init(&control_flow->indirect_jump_target_blocks, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashset_init(&control_flow->sequenced_before, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&control_flow->sequence_numbering, &kefir_hashtree_uint_ops));
    control_flow->next_seq_number = 0;
    control_flow->blocks = NULL;
    control_flow->code = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_flow_free(struct kefir_mem *mem,
                                                struct kefir_opt_code_control_flow *control_flow) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));

    if (control_flow->code != NULL) {
        REQUIRE_OK(kefir_hashtree_free(mem, &control_flow->sequence_numbering));
        REQUIRE_OK(kefir_hashtreeset_free(mem, &control_flow->indirect_jump_target_blocks));
        REQUIRE_OK(kefir_hashset_free(mem, &control_flow->sequenced_before));

        for (kefir_size_t i = 0; i < control_flow->num_of_blocks; i++) {
            REQUIRE_OK(kefir_list_free(mem, &control_flow->blocks[i].predecessors));
            REQUIRE_OK(kefir_list_free(mem, &control_flow->blocks[i].successors));
        }
        KEFIR_FREE(mem, control_flow->blocks);
        memset(control_flow, 0, sizeof(struct kefir_opt_code_control_flow));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_flow_build(struct kefir_mem *mem,
                                                 struct kefir_opt_code_control_flow *control_flow,
                                                 const struct kefir_opt_code_container *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(control_flow->code == NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Optimizer code control flow has already been built"));

    kefir_result_t res;
    control_flow->code = code;
    REQUIRE_OK(kefir_opt_code_container_block_count(control_flow->code, &control_flow->num_of_blocks));
    control_flow->blocks =
        KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_control_flow_block) * control_flow->num_of_blocks);
    REQUIRE(control_flow->blocks != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer code control flow blocks"));
    for (kefir_size_t i = 0; i < control_flow->num_of_blocks; i++) {
        control_flow->blocks[i].immediate_dominator = KEFIR_ID_NONE;
        res = kefir_list_init(&control_flow->blocks[i].predecessors);
        REQUIRE_CHAIN(&res, kefir_list_init(&control_flow->blocks[i].successors));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, control_flow->blocks);
            memset(control_flow, 0, sizeof(struct kefir_opt_code_control_flow));
            return res;
        });
    }
    res = kefir_hashtreeset_init(&control_flow->indirect_jump_target_blocks, &kefir_hashtree_uint_ops);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, control_flow->blocks);
        memset(control_flow, 0, sizeof(struct kefir_opt_code_control_flow));
        return res;
    });

    res = kefir_opt_code_control_flow_link_blocks(mem, control_flow);
    REQUIRE_CHAIN(&res, kefir_opt_code_control_flow_find_dominators(mem, control_flow));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_control_flow_free(mem, control_flow);
        kefir_opt_code_control_flow_init(control_flow);
        return res;
    });

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_flow_drop_sequencing_cache(struct kefir_mem *mem,
                                                                 struct kefir_opt_code_control_flow *control_flow) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));

    control_flow->next_seq_number = 0;
    REQUIRE_OK(kefir_hashset_clear(mem, &control_flow->sequenced_before));
    REQUIRE_OK(kefir_hashtree_clean(mem, &control_flow->sequence_numbering));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_flow_is_dominator(const struct kefir_opt_code_control_flow *control_flow,
                                                        kefir_opt_block_id_t dominated_block,
                                                        kefir_opt_block_id_t dominator_block,
                                                        kefir_bool_t *result_ptr) {
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    if (dominated_block == dominator_block) {
        *result_ptr = true;
    } else if (control_flow->blocks[dominated_block].immediate_dominator != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_control_flow_is_dominator(
            control_flow, control_flow->blocks[dominated_block].immediate_dominator, dominator_block, result_ptr));
    } else {
        *result_ptr = false;
    }
    return KEFIR_OK;
}

static kefir_result_t instr_sequence_number(struct kefir_mem *, struct kefir_opt_code_control_flow *,
                                            kefir_opt_instruction_ref_t, kefir_size_t *, struct kefir_hashtreeset *);

struct instr_input_sequence_number_param {
    struct kefir_mem *mem;
    struct kefir_opt_code_control_flow *control_flow;
    kefir_opt_block_id_t block_id;
    struct kefir_hashtreeset *visited;
    kefir_size_t max_seq_number;
};

static kefir_result_t instr_input_sequence_number(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct instr_input_sequence_number_param *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid instruction sequence number parameter"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(param->control_flow->code, instr_ref, &instr));
    REQUIRE(instr->block_id == param->block_id, KEFIR_OK);

    kefir_size_t seq_num;
    REQUIRE_OK(instr_sequence_number(param->mem, param->control_flow, instr_ref, &seq_num, param->visited));
    param->max_seq_number = MAX(param->max_seq_number, seq_num);
    return KEFIR_OK;
}

static kefir_result_t instr_sequence_number(struct kefir_mem *mem, struct kefir_opt_code_control_flow *control_flow,
                                            kefir_opt_instruction_ref_t instr_ref, kefir_size_t *seq_number,
                                            struct kefir_hashtreeset *visited) {
    REQUIRE(!kefir_hashtreeset_has(visited, (kefir_hashtreeset_entry_t) instr_ref),
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Detected a loop in optimizer instruction dependencies"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&control_flow->sequence_numbering, (kefir_hashtree_key_t) instr_ref, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_PTR(seq_number, (kefir_size_t) node->value);
        return KEFIR_OK;
    }

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(control_flow->code, instr_ref, &instr));

    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(control_flow->code, instr->block_id, &block));

    kefir_bool_t instr_control_flow;
    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(control_flow->code, instr_ref, &instr_control_flow));

    if (instr_control_flow) {
#define CONTROL_FLOW_SEQ_STEP (1ull << 32)
        kefir_size_t seq_num = CONTROL_FLOW_SEQ_STEP;
        kefir_opt_instruction_ref_t control_flow_iter;
        for (res = kefir_opt_code_block_instr_control_head(control_flow->code, block, &control_flow_iter);
             res == KEFIR_OK && control_flow_iter != KEFIR_ID_NONE;
             res = kefir_opt_instruction_next_control(control_flow->code, control_flow_iter, &control_flow_iter),
            seq_num += CONTROL_FLOW_SEQ_STEP) {
            res = kefir_hashtree_at(&control_flow->sequence_numbering, (kefir_hashtree_key_t) control_flow_iter, &node);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                REQUIRE((kefir_size_t) node->value == seq_num,
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Control flow sequence number mismatch"));
                continue;
            }
            REQUIRE_OK(kefir_hashtree_insert(mem, &control_flow->sequence_numbering,
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
        .mem = mem, .control_flow = control_flow, .block_id = block->id, .visited = visited, .max_seq_number = 0};
    REQUIRE_OK(
        kefir_opt_instruction_extract_inputs(control_flow->code, instr, true, instr_input_sequence_number, &param));

    REQUIRE_OK(kefir_hashtreeset_delete(mem, visited, (kefir_hashtreeset_entry_t) instr_ref));

    const kefir_size_t instr_seq = param.max_seq_number + 1;
    REQUIRE_OK(kefir_hashtree_insert(mem, &control_flow->sequence_numbering, (kefir_hashtree_key_t) instr_ref,
                                     (kefir_hashtree_value_t) instr_seq));
    ASSIGN_PTR(seq_number, instr_seq);

    return KEFIR_OK;
}

static kefir_result_t instr_sequence_number_impl(struct kefir_mem *mem,
                                                 struct kefir_opt_code_control_flow *control_flow,
                                                 kefir_opt_instruction_ref_t instr_ref, kefir_size_t *seq_number) {
    struct kefir_hashtreeset visited;
    REQUIRE_OK(kefir_hashtreeset_init(&visited, &kefir_hashtree_uint_ops));
    kefir_result_t res = instr_sequence_number(mem, control_flow, instr_ref, seq_number, &visited);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &visited);
        return res;
    });
    REQUIRE_OK(kefir_hashtreeset_free(mem, &visited));
    return KEFIR_OK;
}

static kefir_result_t is_locally_sequenced_before(struct kefir_mem *mem,
                                                  struct kefir_opt_code_control_flow *control_flow,
                                                  kefir_opt_instruction_ref_t instr_ref1,
                                                  kefir_opt_instruction_ref_t instr_ref2, kefir_bool_t *result_ptr) {
    const struct kefir_opt_instruction *instr1, *instr2;
    REQUIRE_OK(kefir_opt_code_container_instr(control_flow->code, instr_ref1, &instr1));
    REQUIRE_OK(kefir_opt_code_container_instr(control_flow->code, instr_ref2, &instr2));
    REQUIRE(instr1->block_id == instr2->block_id,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Provided instructions belong to different optimizer code blocks"));

    kefir_size_t seq1, seq2;
    REQUIRE_OK(instr_sequence_number_impl(mem, control_flow, instr_ref1, &seq1));
    REQUIRE_OK(instr_sequence_number_impl(mem, control_flow, instr_ref2, &seq2));
    *result_ptr = seq1 < seq2;

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_flow_is_sequenced_before(struct kefir_mem *mem,
                                                               struct kefir_opt_code_control_flow *control_flow,
                                                               kefir_opt_instruction_ref_t instr_ref1,
                                                               kefir_opt_instruction_ref_t instr_ref2,
                                                               kefir_bool_t *result_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    if (instr_ref1 == instr_ref2) {
        *result_ptr = false;
        return KEFIR_OK;
    }

    kefir_hashset_key_t entry =
        (((kefir_uint64_t) instr_ref1) << 32) | (((kefir_uint64_t) instr_ref2) & ((1ull << 32) - 1));
    if (kefir_hashset_has(&control_flow->sequenced_before, entry)) {
        *result_ptr = true;
        return KEFIR_OK;
    }

    const struct kefir_opt_instruction *instr1, *instr2;
    REQUIRE_OK(kefir_opt_code_container_instr(control_flow->code, instr_ref1, &instr1));
    REQUIRE_OK(kefir_opt_code_container_instr(control_flow->code, instr_ref2, &instr2));

    if (instr1->block_id == instr2->block_id) {
        REQUIRE_OK(is_locally_sequenced_before(mem, control_flow, instr_ref1, instr_ref2, result_ptr));
    } else {
        REQUIRE_OK(
            kefir_opt_code_control_flow_is_dominator(control_flow, instr2->block_id, instr1->block_id, result_ptr));
    }

    if (*result_ptr) {
        REQUIRE_OK(kefir_hashset_add(mem, &control_flow->sequenced_before, entry));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_fow_redirect_edge(struct kefir_mem *mem,
                                                        struct kefir_opt_code_control_flow *control_flow,
                                                        kefir_opt_block_id_t old_source_block_id,
                                                        kefir_opt_block_id_t new_source_block_id,
                                                        kefir_opt_block_id_t target_block_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));
    REQUIRE(old_source_block_id != target_block_id,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to redirect loop edges"));

    kefir_size_t total_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(control_flow->code, &total_blocks));

    REQUIRE(old_source_block_id < total_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is out of code container bounds"));
    REQUIRE(new_source_block_id < total_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is out of code container bounds"));
    REQUIRE(target_block_id < total_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is out of code container bounds"));

    for (struct kefir_list_entry *iter = kefir_list_head(&control_flow->blocks[old_source_block_id].successors);
         iter != NULL;) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, (kefir_uptr_t) iter->value);
        struct kefir_list_entry *next_iter = iter->next;
        if (block_id == target_block_id) {
            REQUIRE_OK(kefir_list_pop(mem, &control_flow->blocks[old_source_block_id].successors, iter));
        }
        iter = next_iter;
    }
    REQUIRE_OK(kefir_list_insert_after(mem, &control_flow->blocks[new_source_block_id].successors,
                                       kefir_list_tail(&control_flow->blocks[new_source_block_id].successors),
                                       (void *) (kefir_uptr_t) target_block_id));

    for (struct kefir_list_entry *iter = kefir_list_head(&control_flow->blocks[target_block_id].predecessors);
         iter != NULL; iter = iter->next) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, (kefir_uptr_t) iter->value);
        if (block_id == old_source_block_id) {
            iter->value = (void *) (kefir_uptr_t) new_source_block_id;
        }
    }

    if (control_flow->blocks[target_block_id].immediate_dominator == old_source_block_id) {
        control_flow->blocks[target_block_id].immediate_dominator = new_source_block_id;
    }

    REQUIRE_OK(kefir_opt_code_control_flow_drop_sequencing_cache(mem, control_flow));

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_flow_redirect_edges(struct kefir_mem *mem,
                                                          struct kefir_opt_code_control_flow *control_flow,
                                                          kefir_opt_block_id_t old_source_block_id,
                                                          kefir_opt_block_id_t new_source_block_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));

    kefir_size_t total_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(control_flow->code, &total_blocks));

    REQUIRE(old_source_block_id < total_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is out of code container bounds"));

    for (const struct kefir_list_entry *iter = kefir_list_head(&control_flow->blocks[old_source_block_id].successors);
         iter != NULL; iter = kefir_list_head(&control_flow->blocks[old_source_block_id].successors)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_opt_code_control_fow_redirect_edge(mem, control_flow, old_source_block_id, new_source_block_id,
                                                            successor_block_id));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_flow_drop_edge(struct kefir_mem *mem,
                                                     struct kefir_opt_code_control_flow *control_flow,
                                                     kefir_opt_block_id_t src_block_id,
                                                     kefir_opt_block_id_t dst_block_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));

    kefir_size_t total_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(control_flow->code, &total_blocks));

    REQUIRE(src_block_id < total_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is out of code container bounds"));
    REQUIRE(dst_block_id < total_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is out of code container bounds"));

    for (struct kefir_list_entry *iter = kefir_list_head(&control_flow->blocks[src_block_id].successors);
         iter != NULL;) {
        struct kefir_list_entry *next_iter = iter->next;
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, (kefir_uptr_t) iter->value);
        if (successor_block_id == dst_block_id) {
            REQUIRE_OK(kefir_list_pop(mem, &control_flow->blocks[src_block_id].successors, iter));
        }
        iter = next_iter;
    }

    for (struct kefir_list_entry *iter = kefir_list_head(&control_flow->blocks[dst_block_id].predecessors);
         iter != NULL;) {
        struct kefir_list_entry *next_iter = iter->next;
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, predecessor_block_id, (kefir_uptr_t) iter->value);
        if (predecessor_block_id == src_block_id) {
            REQUIRE_OK(kefir_list_pop(mem, &control_flow->blocks[dst_block_id].predecessors, iter));
        }
        iter = next_iter;
    }

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_flow_is_reachable_from_entry(
    const struct kefir_opt_code_control_flow *control_flow, kefir_opt_block_id_t block_id,
    kefir_bool_t *reachable_flag_ptr) {
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));
    REQUIRE(reachable_flag_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(control_flow->code, &num_of_blocks));
    REQUIRE(block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is outside of code container bounds"));

    *reachable_flag_ptr = block_id == control_flow->code->entry_point ||
                          control_flow->blocks[block_id].immediate_dominator != KEFIR_ID_NONE;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_flow_block_direct_predecessor(
    const struct kefir_opt_code_control_flow *control_flow, kefir_opt_block_id_t block_id,
    kefir_opt_block_id_t successor_block_id, kefir_bool_t *result_ptr) {
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(control_flow->code, &num_of_blocks));
    REQUIRE(block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is outside of code container bounds"));
    REQUIRE(successor_block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is outside of code container bounds"));

    const struct kefir_opt_code_block *successor_block;
    REQUIRE_OK(kefir_opt_code_container_block(control_flow->code, successor_block_id, &successor_block));

    kefir_bool_t found_block_id = false;
    for (const struct kefir_list_entry *iter = kefir_list_head(&control_flow->blocks[successor_block_id].predecessors);
         !found_block_id && iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, predecessor_block_id, (kefir_uptr_t) iter->value);

        if (predecessor_block_id == block_id) {
            found_block_id = true;
        }
    }

    *result_ptr = found_block_id;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_flow_block_exclusive_direct_predecessor(
    const struct kefir_opt_code_control_flow *control_flow, kefir_opt_block_id_t block_id,
    kefir_opt_block_id_t successor_block_id, kefir_bool_t *result_ptr) {
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    if (successor_block_id == control_flow->code->entry_point) {
        *result_ptr = false;
        return KEFIR_OK;
    }

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(control_flow->code, &num_of_blocks));
    REQUIRE(block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is outside of code container bounds"));
    REQUIRE(successor_block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is outside of code container bounds"));

    const struct kefir_opt_code_block *successor_block;
    REQUIRE_OK(kefir_opt_code_container_block(control_flow->code, successor_block_id, &successor_block));

    kefir_bool_t result = block_id != successor_block_id && kefir_hashtreeset_empty(&successor_block->public_labels);
    kefir_bool_t found_block_id = false;
    for (const struct kefir_list_entry *iter = kefir_list_head(&control_flow->blocks[successor_block_id].predecessors);
         result && iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, predecessor_block_id, (kefir_uptr_t) iter->value);

        if (successor_block_id == predecessor_block_id) {
            result = false;
        } else if (predecessor_block_id != block_id) {
            kefir_bool_t reachable_from_entry;
            REQUIRE_OK(kefir_opt_code_control_flow_is_reachable_from_entry(control_flow, predecessor_block_id,
                                                                           &reachable_from_entry));
            result = result && !reachable_from_entry;
        } else {
            found_block_id = true;
        }
    }

    *result_ptr = result && found_block_id;
    return KEFIR_OK;
}

static kefir_result_t check_all_control_flow_uses_after_impl(struct kefir_mem *mem,
                                                             struct kefir_opt_code_control_flow *control_flow,
                                                             kefir_opt_instruction_ref_t instr_ref,
                                                             kefir_opt_instruction_ref_t sequence_point_instr_ref,
                                                             kefir_bool_t *all_uses_after,
                                                             struct kefir_hashtreeset *visited) {
    if (kefir_hashtreeset_has(visited, (kefir_hashtreeset_entry_t) instr_ref)) {
        *all_uses_after = true;
        return KEFIR_OK;
    }
    REQUIRE_OK(kefir_hashtreeset_add(mem, visited, (kefir_hashtreeset_entry_t) instr_ref));

    struct kefir_opt_instruction_use_iterator use_iter;
    kefir_result_t res;
    for (res = kefir_opt_code_container_instruction_use_instr_iter(control_flow->code, instr_ref, &use_iter);
         res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_instruction *use_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(control_flow->code, use_iter.use_instr_ref, &use_instr));

        if (use_instr->id == sequence_point_instr_ref) {
            switch (use_instr->operation.opcode) {
                case KEFIR_OPT_OPCODE_INVOKE:
                case KEFIR_OPT_OPCODE_INVOKE_VIRTUAL: {
                    if (use_instr->operation.parameters.function_call.indirect_ref == instr_ref) {
                        *all_uses_after = false;
                        return KEFIR_OK;
                    }

                    const struct kefir_opt_call_node *call_node;
                    REQUIRE_OK(kefir_opt_code_container_call(
                        control_flow->code, use_instr->operation.parameters.function_call.call_ref, &call_node));
                    for (kefir_size_t i = 0; i < call_node->argument_count; i++) {
                        if (call_node->arguments[i] == instr_ref) {
                            *all_uses_after = false;
                            return KEFIR_OK;
                        }
                    }
                } break;

                default:
                    // Intentionally left blank
                    break;
            }

            kefir_bool_t uses_sequenced_after;
            REQUIRE_OK(check_all_control_flow_uses_after_impl(
                mem, control_flow, use_instr->id, sequence_point_instr_ref, &uses_sequenced_after, visited));
            if (!uses_sequenced_after) {
                *all_uses_after = false;
                return KEFIR_OK;
            }
        } else if (use_instr->operation.opcode != KEFIR_OPT_OPCODE_LOCAL_LIFETIME_MARK) {
            kefir_bool_t is_control_flow;
            REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(control_flow->code, use_instr->id, &is_control_flow));
            if (is_control_flow) {
                kefir_bool_t sequenced_before;
                REQUIRE_OK(kefir_opt_code_control_flow_is_sequenced_before(mem, control_flow, sequence_point_instr_ref,
                                                                           use_instr->id, &sequenced_before));
                if (!sequenced_before) {
                    *all_uses_after = false;
                    return KEFIR_OK;
                }
            }

            kefir_bool_t uses_sequenced_after;
            REQUIRE_OK(check_all_control_flow_uses_after_impl(
                mem, control_flow, use_instr->id, sequence_point_instr_ref, &uses_sequenced_after, visited));
            if (!uses_sequenced_after) {
                *all_uses_after = false;
                return KEFIR_OK;
            }
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    *all_uses_after = true;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_check_all_control_flow_uses_after(struct kefir_mem *mem,
                                                           struct kefir_opt_code_control_flow *control_flow,
                                                           kefir_opt_instruction_ref_t instr_ref,
                                                           kefir_opt_instruction_ref_t sequence_point_instr_ref,
                                                           kefir_bool_t *all_uses_after) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));
    REQUIRE(all_uses_after != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_hashtreeset visited;
    REQUIRE_OK(kefir_hashtreeset_init(&visited, &kefir_hashtree_uint_ops));
    kefir_result_t res = check_all_control_flow_uses_after_impl(mem, control_flow, instr_ref, sequence_point_instr_ref,
                                                                all_uses_after, &visited);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &visited);
        return res;
    });
    REQUIRE_OK(kefir_hashtreeset_free(mem, &visited));

    return KEFIR_OK;
}
