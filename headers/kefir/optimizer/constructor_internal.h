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

#ifndef KEFIR_OPTIMIZER_CONSTRUCTOR_INTERNAL_H_
#define KEFIR_OPTIMIZER_CONSTRUCTOR_INTERNAL_H_

#ifndef KEFIR_OPTIMIZER_CONSTRUCTOR_INTERNAL_INCLUDE
#error "This optimizer constructor internal header includes shall be specifically marked"
#endif

#include "kefir/optimizer/constructor.h"
#include "kefir/core/list.h"
#include "kefir/core/hashtree.h"
#include "kefir/core/hashtreeset.h"

typedef struct kefir_opt_constructor_code_block_state {
    kefir_opt_block_id_t block_id;
    struct kefir_list stack;
    struct kefir_list phi_stack;
    kefir_bool_t reachable;
} kefir_opt_constructor_code_block_state_t;

typedef struct kefir_opt_constructor_state {
    struct kefir_opt_function *function;
    struct kefir_hashtree code_blocks;
    struct kefir_hashtree code_block_index;
    struct kefir_opt_constructor_code_block_state *current_block;
    struct kefir_hashtreeset indirect_jump_targets;
    kefir_size_t ir_location;
} kefir_opt_constructor_state;

kefir_result_t kefir_opt_constructor_init(struct kefir_opt_function *, struct kefir_opt_constructor_state *);
kefir_result_t kefir_opt_constructor_free(struct kefir_mem *, struct kefir_opt_constructor_state *);

kefir_result_t kefir_opt_constructor_start_code_block_at(struct kefir_mem *, struct kefir_opt_constructor_state *,
                                                         kefir_size_t);
kefir_result_t kefir_opt_constructor_mark_code_block_for_indirect_jump(struct kefir_mem *,
                                                                       struct kefir_opt_constructor_state *,
                                                                       kefir_size_t);
kefir_result_t kefir_opt_constructor_find_code_block_for(const struct kefir_opt_constructor_state *, kefir_size_t,
                                                         struct kefir_opt_constructor_code_block_state **);
kefir_result_t kefir_opt_constructor_update_current_code_block(struct kefir_mem *,
                                                               struct kefir_opt_constructor_state *);

kefir_result_t kefir_opt_constructor_stack_push(struct kefir_mem *, struct kefir_opt_constructor_state *,
                                                kefir_opt_instruction_ref_t);
kefir_result_t kefir_opt_constructor_stack_pop(struct kefir_mem *, struct kefir_opt_constructor_state *,
                                               kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_constructor_stack_at(struct kefir_mem *, struct kefir_opt_constructor_state *, kefir_size_t,
                                              kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_constructor_stack_exchange(struct kefir_mem *, struct kefir_opt_constructor_state *,
                                                    kefir_size_t);

#endif
