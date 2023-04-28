/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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
    kefir_opt_block_id_t code_block_id;
    REQUIRE(!kefir_hashtree_has(&state->code_blocks, (kefir_hashtree_key_t) ir_location), KEFIR_OK);
    REQUIRE_OK(kefir_opt_code_container_new_block(mem, state->code, ir_location == 0, &code_block_id));

    struct kefir_opt_constructor_code_block_state *block_state =
        KEFIR_MALLOC(mem, sizeof(struct kefir_opt_constructor_code_block_state));
    REQUIRE(block_state != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer code block state"));

    block_state->block_id = code_block_id;
    kefir_result_t res = kefir_list_init(&block_state->stack);
    REQUIRE_ELSE(res == KEFIR_OK, {
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

    return KEFIR_OK;
}

kefir_result_t kefir_opt_constructor_find_code_block_for(const struct kefir_opt_constructor_state *state,
                                                         kefir_size_t ir_location,
                                                         struct kefir_opt_constructor_code_block_state **state_ptr) {
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
                                                               struct kefir_opt_constructor_state *state) {
    struct kefir_opt_constructor_code_block_state *block_state = NULL;
    kefir_result_t res = kefir_opt_constructor_find_code_block_for(state, state->ir_location, &block_state);
    if (res != KEFIR_NOT_FOUND && block_state != state->current_block) {
        REQUIRE_OK(res);
        if (state->current_block != NULL) {
            kefir_bool_t current_block_finalized = false;
            REQUIRE_OK(kefir_opt_code_builder_is_finalized(state->code, state->current_block->block_id,
                                                           &current_block_finalized));
            if (!current_block_finalized) {
                REQUIRE_OK(kefir_opt_code_builder_finalize_jump(mem, state->code, state->current_block->block_id,
                                                                block_state->block_id, NULL));
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

    REQUIRE_OK(kefir_list_free(mem, &state->stack));
    KEFIR_FREE(mem, state);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_constructor_init(struct kefir_opt_code_container *code,
                                          struct kefir_opt_constructor_state *state) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid optimizer constructor state"));

    REQUIRE_OK(kefir_hashtree_init(&state->code_blocks, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&state->code_blocks, free_kefir_opt_constructor_code_block_state, NULL));

    state->code = code;
    state->current_block = NULL;
    state->ir_location = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_constructor_free(struct kefir_mem *mem, struct kefir_opt_constructor_state *state) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid optimizer constructor state"));

    REQUIRE_OK(kefir_hashtree_free(mem, &state->code_blocks));
    state->code = NULL;
    state->current_block = NULL;
    state->ir_location = 0;
    return KEFIR_OK;
}
