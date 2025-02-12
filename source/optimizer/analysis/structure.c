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
    structure->blocks = NULL;
    structure->code = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_structure_free(struct kefir_mem *mem, struct kefir_opt_code_structure *structure) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code structure"));

    if (structure->code != NULL) {
        REQUIRE_OK(kefir_hashtreeset_free(mem, &structure->indirect_jump_target_blocks));
        REQUIRE_OK(kefir_bucketset_free(mem, &structure->sequenced_before));

        kefir_size_t num_of_blocks;
        REQUIRE_OK(kefir_opt_code_container_block_count(structure->code, &num_of_blocks));
        for (kefir_size_t i = 0; i < num_of_blocks; i++) {
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
    kefir_size_t num_of_blocks;
    structure->code = code;
    REQUIRE_OK(kefir_opt_code_container_block_count(structure->code, &num_of_blocks));
    structure->blocks = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_structure_block) * num_of_blocks);
    REQUIRE(structure->blocks != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer code structure blocks"));
    for (kefir_size_t i = 0; i < num_of_blocks; i++) {
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

    REQUIRE_OK(kefir_bucketset_free(mem, &structure->sequenced_before));
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
struct is_locally_sequenced_before_param {
    struct kefir_mem *mem;
    struct kefir_opt_code_structure *structure;
    kefir_opt_instruction_ref_t instr_ref;
    kefir_bool_t *result;
};

static kefir_result_t is_locally_sequenced_before_impl(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct is_locally_sequenced_before_param *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction sequence parameter"));

    if (*param->result) {
        kefir_bool_t result = false;
        REQUIRE_OK(kefir_opt_code_structure_is_sequenced_before(param->mem, param->structure, instr_ref,
                                                                param->instr_ref, &result));
        *param->result = *param->result && result;
    }
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

    kefir_bool_t result = true;

    kefir_bool_t instr1_control_flow, instr2_control_flow;
    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(structure->code, instr_ref1, &instr1_control_flow));
    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(structure->code, instr_ref2, &instr2_control_flow));

    if (instr1_control_flow && instr2_control_flow) {
        kefir_bool_t found = false;
        for (kefir_opt_instruction_ref_t iter = instr_ref2; !found && iter != KEFIR_ID_NONE;) {
            if (iter == instr_ref1) {
                found = true;
            } else {
                REQUIRE_OK(kefir_opt_instruction_prev_control(structure->code, iter, &iter));
            }
        }
        if (!found) {
            result = false;
        }
    }

    if (result && instr2_control_flow) {
        REQUIRE_OK(kefir_opt_instruction_extract_inputs(
            structure->code, instr1, true, is_locally_sequenced_before_impl,
            &(struct is_locally_sequenced_before_param) {
                .mem = mem, .structure = structure, .instr_ref = instr_ref2, .result = &result}));
    }
    *result_ptr = result;

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
