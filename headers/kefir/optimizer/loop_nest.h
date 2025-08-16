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

#ifndef KEFIR_OPTIMIZER_LOOP_NEST_H_
#define KEFIR_OPTIMIZER_LOOP_NEST_H_

#include "kefir/optimizer/structure.h"
#include "kefir/core/tree.h"

typedef kefir_uint64_t kefir_opt_code_loop_id_t;

typedef struct kefir_opt_code_loop {
    kefir_opt_block_id_t loop_entry_block_id;
    kefir_opt_block_id_t loop_exit_block_id;
    struct kefir_hashtreeset loop_blocks;
} kefir_opt_code_loop_t;

typedef struct kefir_opt_loop_nest {
    struct kefir_tree_node nest;
} kefir_opt_loop_nest_t;

typedef struct kefir_opt_code_loop_collection {
    struct kefir_hashtree loops;
    struct kefir_list nests;
} kefir_opt_code_loop_collection_t;

#define KEFIR_OPT_CODE_LOOP_ID(_loop)                                                      \
    ((kefir_opt_code_loop_id_t) ((((kefir_uint64_t) (_loop)->loop_entry_block_id) << 32) | \
                                 (kefir_uint32_t) (_loop)->loop_exit_block_id))

kefir_result_t kefir_opt_code_loop_collection_init(struct kefir_opt_code_loop_collection *);
kefir_result_t kefir_opt_code_loop_collection_free(struct kefir_mem *, struct kefir_opt_code_loop_collection *);

kefir_result_t kefir_opt_code_loop_collection_build(struct kefir_mem *, struct kefir_opt_code_loop_collection *,
                                                    const struct kefir_opt_code_structure *);

typedef struct kefir_opt_code_loop_collection_iterator {
    struct kefir_hashtree_node_iterator iter;
} kefir_opt_code_loop_collection_iterator_t;

kefir_result_t kefir_opt_code_loop_collection_iter(const struct kefir_opt_code_loop_collection *,
                                                   const struct kefir_opt_code_loop **,
                                                   struct kefir_opt_code_loop_collection_iterator *);
kefir_result_t kefir_opt_code_loop_collection_next(const struct kefir_opt_code_loop **,
                                                   struct kefir_opt_code_loop_collection_iterator *);

typedef struct kefir_opt_code_loop_nest_collection_iterator {
    const struct kefir_list_entry *iter;
} kefir_opt_code_loop_nest_collection_iterator_t;

kefir_result_t kefir_opt_code_loop_nest_collection_iter(const struct kefir_opt_code_loop_collection *,
                                                        const struct kefir_opt_loop_nest **,
                                                        struct kefir_opt_code_loop_nest_collection_iterator *);
kefir_result_t kefir_opt_code_loop_nest_collection_next(const struct kefir_opt_loop_nest **,
                                                        struct kefir_opt_code_loop_nest_collection_iterator *);

#endif
