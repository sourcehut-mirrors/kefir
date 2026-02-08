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

#ifndef KEFIR_OPTIMIZER_CONTROL_FLOW_H_
#define KEFIR_OPTIMIZER_CONTROL_FLOW_H_

#include "kefir/optimizer/code.h"
#include "kefir/core/list.h"
#include "kefir/core/hashset.h"

typedef struct kefir_opt_code_control_flow_block {
    struct kefir_hashset predecessors;
    struct kefir_hashset successors;
    kefir_opt_block_id_t immediate_dominator;
    kefir_size_t dominance_linear_index;
    kefir_size_t dominated_block_max_linear;
    struct kefir_hashset dominance_frontier;
} kefir_opt_code_control_flow_block_t;

typedef struct kefir_opt_code_control_flow {
    const struct kefir_opt_code_container *code;
    kefir_size_t num_of_blocks;
    struct kefir_opt_code_control_flow_block *blocks;
    struct kefir_hashset indirect_jump_source_blocks;
    struct kefir_hashset indirect_jump_target_blocks;
    struct kefir_hashtable dominator_tree;
} kefir_opt_code_control_flow_t;

kefir_result_t kefir_opt_code_control_flow_init(struct kefir_opt_code_control_flow *);
kefir_result_t kefir_opt_code_control_flow_free(struct kefir_mem *, struct kefir_opt_code_control_flow *);

kefir_result_t kefir_opt_code_control_flow_build(struct kefir_mem *, struct kefir_opt_code_control_flow *,
                                                 const struct kefir_opt_code_container *);

kefir_result_t kefir_opt_code_control_flow_is_dominator(const struct kefir_opt_code_control_flow *,
                                                        kefir_opt_block_id_t, kefir_opt_block_id_t, kefir_bool_t *);

kefir_result_t kefir_opt_code_control_flow_is_reachable_from_entry(const struct kefir_opt_code_control_flow *,
                                                                   kefir_opt_block_id_t, kefir_bool_t *);

kefir_result_t kefir_opt_code_control_flow_block_direct_predecessor(const struct kefir_opt_code_control_flow *,
                                                                    kefir_opt_block_id_t, kefir_opt_block_id_t,
                                                                    kefir_bool_t *);
kefir_result_t kefir_opt_code_control_flow_block_exclusive_direct_predecessor(
    const struct kefir_opt_code_control_flow *, kefir_opt_block_id_t, kefir_opt_block_id_t, kefir_bool_t *);

#ifdef KEFIR_OPTIMIZER_CONTROL_FLOW_INTERNAL
kefir_result_t kefir_opt_code_control_flow_link_blocks(struct kefir_mem *, struct kefir_opt_code_control_flow *);
kefir_result_t kefir_opt_code_control_flow_find_dominators(struct kefir_mem *, struct kefir_opt_code_control_flow *);
#endif

typedef struct kefir_opt_control_flow_dominator_tree_iterator {
    struct kefir_hashset_iterator iter;
} kefir_opt_control_flow_dominator_tree_iterator_t;

kefir_result_t kefir_opt_control_flow_dominator_tree_iter(const struct kefir_opt_code_control_flow *,
                                                          struct kefir_opt_control_flow_dominator_tree_iterator *,
                                                          kefir_opt_block_id_t, kefir_opt_block_id_t *);
kefir_result_t kefir_opt_control_flow_dominator_tree_next(struct kefir_opt_control_flow_dominator_tree_iterator *,
                                                          kefir_opt_block_id_t *);

#endif
