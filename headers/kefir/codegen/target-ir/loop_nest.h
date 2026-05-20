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

#ifndef KEFIR_CODEGEN_TARGET_IR_LOOP_NEST_H_
#define KEFIR_CODEGEN_TARGET_IR_LOOP_NEST_H_

#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/core/tree.h"

typedef kefir_uint64_t kefir_codegen_target_ir_loop_ref_t;

typedef struct kefir_codegen_target_ir_loop {
    kefir_codegen_target_ir_block_ref_t preheader_ref;
    kefir_codegen_target_ir_block_ref_t header_ref;
    struct kefir_hashset blocks;
    struct kefir_hashset latches;
    struct kefir_hashset exits;

    kefir_uint32_t level;
    struct kefir_tree_node *nest_node;
} kefir_codegen_target_ir_loop_t;

typedef struct kefir_codegen_target_ir_loop_nest {
    struct kefir_tree_node nest;
} kefir_codegen_target_ir_loop_nest_t;

typedef struct kefir_codegen_target_ir_loop_collection {
    struct kefir_hashtable loops;
    struct kefir_list nests;
    struct kefir_hashtable block_index;
} kefir_codegen_target_ir_loop_collection_t;

kefir_result_t kefir_codegen_target_ir_loop_collection_init(struct kefir_codegen_target_ir_loop_collection *);
kefir_result_t kefir_codegen_target_ir_loop_collection_free(struct kefir_mem *,
                                                            struct kefir_codegen_target_ir_loop_collection *);

kefir_result_t kefir_codegen_target_ir_loop_collection_build(struct kefir_mem *,
                                                             struct kefir_codegen_target_ir_loop_collection *,
                                                             const struct kefir_codegen_target_ir_control_flow *);

kefir_result_t kefir_codegen_target_ir_loop_collection_find_loop(const struct kefir_codegen_target_ir_loop_collection *,
                                                                 kefir_codegen_target_ir_block_ref_t,
                                                                 struct kefir_codegen_target_ir_loop **);
kefir_result_t kefir_codegen_target_ir_loop_level(const struct kefir_codegen_target_ir_loop_collection *,
                                                  kefir_codegen_target_ir_block_ref_t, kefir_uint32_t *);

typedef struct kefir_codegen_target_ir_loop_collection_iterator {
    struct kefir_hashtable_iterator iter;
} kefir_codegen_target_ir_loop_collection_iterator_t;

kefir_result_t kefir_codegen_target_ir_loop_collection_iter(const struct kefir_codegen_target_ir_loop_collection *,
                                                            struct kefir_codegen_target_ir_loop **,
                                                            struct kefir_codegen_target_ir_loop_collection_iterator *);
kefir_result_t kefir_codegen_target_ir_loop_collection_next(struct kefir_codegen_target_ir_loop **,
                                                            struct kefir_codegen_target_ir_loop_collection_iterator *);

typedef struct kefir_codegen_target_ir_loop_nest_collection_iterator {
    const struct kefir_list_entry *iter;
} kefir_codegen_target_ir_loop_nest_collection_iterator_t;

kefir_result_t kefir_codegen_target_ir_loop_nest_collection_iter(
    const struct kefir_codegen_target_ir_loop_collection *, const struct kefir_codegen_target_ir_loop_nest **,
    struct kefir_codegen_target_ir_loop_nest_collection_iterator *);
kefir_result_t kefir_codegen_target_ir_loop_nest_collection_next(
    const struct kefir_codegen_target_ir_loop_nest **, struct kefir_codegen_target_ir_loop_nest_collection_iterator *);

#endif
