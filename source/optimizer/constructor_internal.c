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

#include "kefir/optimizer/constructor.h"
#include "kefir/optimizer/builder.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#define KEFIR_OPTIMIZER_CONSTRUCTOR_INTERNAL_INCLUDE
#include "kefir/optimizer/constructor_internal.h"

kefir_result_t kefir_opt_constructor_start_code_block_at(struct kefir_mem *mem,
                                                         struct kefir_opt_constructor_state *state,
                                                         kefir_size_t ir_location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer constructor state"));

    kefir_opt_block_id_t code_block_id;
    REQUIRE(!kefir_hashtree_has(&state->code_blocks, (kefir_hashtree_key_t) ir_location), KEFIR_OK);
    REQUIRE_OK(kefir_opt_code_container_new_block(mem, &state->function->code, ir_location == (kefir_size_t) -1ll,
                                                  &code_block_id));

    struct kefir_opt_constructor_code_block_state *block_state =
        KEFIR_MALLOC(mem, sizeof(struct kefir_opt_constructor_code_block_state));
    REQUIRE(block_state != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer code block state"));

    block_state->block_id = code_block_id;
    block_state->reachable = false;
    kefir_result_t res = kefir_list_init(&block_state->stack);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, state);
        return res;
    });

    res = kefir_list_init(&block_state->phi_stack);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &block_state->stack);
        KEFIR_FREE(mem, state);
        return res;
    });

    res = kefir_hashtree_insert(mem, &state->code_blocks, (kefir_hashtree_key_t) ir_location,
                                (kefir_hashtree_value_t) block_state);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &block_state->stack);
        KEFIR_FREE(mem, state);
        return res;
    });

    REQUIRE_OK(kefir_hashtree_insert(mem, &state->code_block_index, (kefir_hashtree_key_t) code_block_id,
                                     (kefir_hashtree_value_t) block_state));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_constructor_mark_code_block_for_indirect_jump(struct kefir_mem *mem,
                                                                       struct kefir_opt_constructor_state *state,
                                                                       kefir_size_t ir_location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer constructor state"));

    struct kefir_opt_constructor_code_block_state *block_state;
    REQUIRE_OK(kefir_opt_constructor_find_code_block_for(state, ir_location, &block_state));
    REQUIRE_OK(
        kefir_hashtreeset_add(mem, &state->indirect_jump_targets, (kefir_hashtreeset_entry_t) block_state->block_id));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_constructor_find_code_block_for(const struct kefir_opt_constructor_state *state,
                                                         kefir_size_t ir_location,
                                                         struct kefir_opt_constructor_code_block_state **state_ptr) {
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer constructor state"));
    REQUIRE(state_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                               "Expected valid pointer to optimizer code block contructor state"));

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&state->code_blocks, (kefir_hashtree_key_t) ir_location, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find optimizer code block state for provided IR location");
    }
    REQUIRE_OK(res);
    *state_ptr = (struct kefir_opt_constructor_code_block_state *) node->value;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_constructor_update_current_code_block(struct kefir_mem *mem,
                                                               struct kefir_opt_constructor_state *state,
                                                               kefir_size_t ir_location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer constructor state"));

    struct kefir_opt_constructor_code_block_state *block_state = NULL;
    kefir_result_t res = kefir_opt_constructor_find_code_block_for(state, ir_location, &block_state);
    if (res != KEFIR_NOT_FOUND && block_state != state->current_block) {
        REQUIRE_OK(res);
        if (state->current_block != NULL) {
            kefir_bool_t current_block_finalized = false;
            REQUIRE_OK(kefir_opt_code_builder_is_finalized(&state->function->code, state->current_block->block_id,
                                                           &current_block_finalized));
            if (!current_block_finalized) {
                REQUIRE_OK(kefir_opt_code_builder_finalize_jump(
                    mem, &state->function->code, state->current_block->block_id, block_state->block_id, NULL));
            }
        }
        state->current_block = block_state;
    }
    return KEFIR_OK;
}

static kefir_result_t free_kefir_opt_constructor_code_block_state(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                                                  kefir_hashtree_key_t key,
                                                                  kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_constructor_code_block_state *, state, value);
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block state"));

    REQUIRE_OK(kefir_list_free(mem, &state->phi_stack));
    REQUIRE_OK(kefir_list_free(mem, &state->stack));
    KEFIR_FREE(mem, state);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_constructor_init(struct kefir_opt_function *function,
                                          struct kefir_opt_constructor_state *state) {
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid optimizer constructor state"));

    REQUIRE_OK(kefir_hashtree_init(&state->code_blocks, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&state->code_blocks, free_kefir_opt_constructor_code_block_state, NULL));
    REQUIRE_OK(kefir_hashtree_init(&state->code_block_index, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&state->indirect_jump_targets, &kefir_hashtree_uint_ops));

    state->function = function;
    state->current_block = NULL;
    state->ir_location = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_constructor_free(struct kefir_mem *mem, struct kefir_opt_constructor_state *state) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer constructor state"));

    REQUIRE_OK(kefir_hashtreeset_free(mem, &state->indirect_jump_targets));
    REQUIRE_OK(kefir_hashtree_free(mem, &state->code_block_index));
    REQUIRE_OK(kefir_hashtree_free(mem, &state->code_blocks));
    state->function = NULL;
    state->current_block = NULL;
    state->ir_location = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_constructor_stack_push(struct kefir_mem *mem, struct kefir_opt_constructor_state *state,
                                                kefir_opt_instruction_ref_t ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer constructor state"));
    REQUIRE(state->current_block != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid current optimizer code block state"));

    REQUIRE_OK(kefir_list_insert_after(mem, &state->current_block->stack, kefir_list_tail(&state->current_block->stack),
                                       (void *) (kefir_uptr_t) ref));
    return KEFIR_OK;
}

static kefir_result_t stack_ensure_depth(struct kefir_mem *mem, struct kefir_opt_constructor_state *state,
                                         kefir_size_t depth) {
    while (kefir_list_length(&state->current_block->stack) < depth) {
        kefir_opt_phi_id_t phi_ref;
        kefir_opt_instruction_ref_t instr_ref;
        REQUIRE_OK(
            kefir_opt_code_container_new_phi(mem, &state->function->code, state->current_block->block_id, &phi_ref));
        REQUIRE_OK(kefir_opt_code_builder_phi(mem, &state->function->code, state->current_block->block_id, phi_ref,
                                              &instr_ref));
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&state->function->code, KEFIR_ID_NONE, instr_ref));
        REQUIRE_OK(kefir_list_insert_after(mem, &state->current_block->stack, NULL, (void *) (kefir_uptr_t) instr_ref));
        REQUIRE_OK(
            kefir_list_insert_after(mem, &state->current_block->phi_stack, NULL, (void *) (kefir_uptr_t) phi_ref));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_constructor_stack_pop(struct kefir_mem *mem, struct kefir_opt_constructor_state *state,
                                               kefir_opt_instruction_ref_t *ref_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer constructor state"));
    REQUIRE(state->current_block != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid current optimizer code block state"));
    REQUIRE(ref_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction reference"));

    REQUIRE_OK(stack_ensure_depth(mem, state, 1));
    struct kefir_list_entry *tail = kefir_list_tail(&state->current_block->stack);
    REQUIRE(tail != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Optimizer code block constructor stack is empty"));
    *ref_ptr = (kefir_opt_instruction_ref_t) (kefir_uptr_t) tail->value;
    REQUIRE_OK(kefir_list_pop(mem, &state->current_block->stack, tail));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_constructor_stack_at(struct kefir_mem *mem, struct kefir_opt_constructor_state *state,
                                              kefir_size_t index, kefir_opt_instruction_ref_t *ref_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer constructor state"));
    REQUIRE(state->current_block != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid current optimizer code block state"));
    REQUIRE(ref_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction reference"));

    REQUIRE_OK(stack_ensure_depth(mem, state, index + 1));
    struct kefir_list_entry *entry =
        kefir_list_at(&state->current_block->stack, kefir_list_length(&state->current_block->stack) - index - 1);
    REQUIRE(entry != NULL,
            KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR,
                            "Provided index is out of bounds of optimizer code block constructor stack"));
    *ref_ptr = (kefir_opt_instruction_ref_t) (kefir_uptr_t) entry->value;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_constructor_stack_exchange(struct kefir_mem *mem, struct kefir_opt_constructor_state *state,
                                                    kefir_size_t index) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer constructor state"));
    REQUIRE(state->current_block != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid current optimizer code block state"));

    REQUIRE_OK(stack_ensure_depth(mem, state, index + 1));
    struct kefir_list_entry *tail = kefir_list_tail(&state->current_block->stack);
    REQUIRE(tail != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Optimizer code block constructor stack is empty"));
    struct kefir_list_entry *entry =
        kefir_list_at(&state->current_block->stack, kefir_list_length(&state->current_block->stack) - index - 1);
    REQUIRE(entry != NULL,
            KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR,
                            "Provided index is out of bounds of optimizer code block constructor stack"));

    void *tail_value = tail->value;
    tail->value = entry->value;
    entry->value = tail_value;
    return KEFIR_OK;
}
