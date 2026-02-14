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

#ifndef KEFIR_OPTIMIZER_MEMORY_SSA_H_
#define KEFIR_OPTIMIZER_MEMORY_SSA_H_

#include "kefir/core/hashset.h"
#include "kefir/optimizer/code.h"
#include "kefir/optimizer/control_flow.h"
#include "kefir/optimizer/liveness.h"

typedef kefir_uint32_t kefir_opt_code_memssa_node_ref_t;

typedef enum kefir_opt_code_memssa_node_type {
    KEFIR_OPT_CODE_MEMSSA_ROOT_NODE,
    KEFIR_OPT_CODE_MEMSSA_CONSUME_NODE,
    KEFIR_OPT_CODE_MEMSSA_PRODUCE_NODE,
    KEFIR_OPT_CODE_MEMSSA_PRODUCE_CONSUME_NODE,
    KEFIR_OPT_CODE_MEMSSA_PHI_NODE,
    KEFIR_OPT_CODE_MEMSSA_TERMINATE_NODE
} kefir_opt_code_memssa_node_type_t;

typedef struct kefir_opt_code_memssa_phi_link {
    kefir_opt_block_id_t block_ref;
    kefir_opt_code_memssa_node_ref_t node_ref;
} kefir_opt_code_memssa_phi_link_t;

typedef struct kefir_opt_code_memssa_node {
    kefir_opt_code_memssa_node_type_t type;
    union {
        struct {
            kefir_opt_code_memssa_node_ref_t predecessor_ref;
            kefir_opt_instruction_ref_t instr_ref;
        };
        struct {
            struct kefir_opt_code_memssa_phi_link *links;
            kefir_size_t link_count;
        } phi;
    };

    struct kefir_hashset uses;
} kefir_opt_code_memssa_node_t;

typedef struct kefir_opt_code_memssa_block {
    kefir_opt_code_memssa_node_ref_t output_node_ref;
} kefir_opt_code_memssa_block_t;

typedef struct kefir_opt_code_memssa {
    struct kefir_opt_code_memssa_node *nodes;
    kefir_size_t node_length;
    kefir_size_t node_capacity;

    struct kefir_opt_code_memssa_block *blocks;
    kefir_size_t block_length;
    kefir_size_t block_capacity;

    kefir_opt_code_memssa_node_ref_t root_ref;

    struct kefir_hashtable instruction_bindings;
} kefir_opt_code_memssa_t;

kefir_result_t kefir_opt_code_memssa_init(struct kefir_opt_code_memssa *);
kefir_result_t kefir_opt_code_memssa_free(struct kefir_mem *, struct kefir_opt_code_memssa *);

kefir_result_t kefir_opt_code_memssa_create_block(struct kefir_mem *, struct kefir_opt_code_memssa *,
                                                  kefir_opt_block_id_t, kefir_opt_code_memssa_node_ref_t);
kefir_result_t kefir_opt_code_memssa_update_block_output(struct kefir_opt_code_memssa *, kefir_opt_block_id_t,
                                                         kefir_opt_code_memssa_node_ref_t);
kefir_result_t kefir_opt_code_memssa_block(const struct kefir_opt_code_memssa *, kefir_opt_block_id_t,
                                           const struct kefir_opt_code_memssa_block **);

kefir_result_t kefir_opt_code_memssa_new_root_node(struct kefir_mem *, struct kefir_opt_code_memssa *,
                                                   kefir_opt_code_memssa_node_ref_t *);
kefir_result_t kefir_opt_code_memssa_new_consume_node(struct kefir_mem *, struct kefir_opt_code_memssa *,
                                                      kefir_opt_code_memssa_node_ref_t, kefir_opt_instruction_ref_t,
                                                      kefir_opt_code_memssa_node_ref_t *);
kefir_result_t kefir_opt_code_memssa_new_produce_node(struct kefir_mem *, struct kefir_opt_code_memssa *,
                                                      kefir_opt_code_memssa_node_ref_t, kefir_opt_instruction_ref_t,
                                                      kefir_opt_code_memssa_node_ref_t *);
kefir_result_t kefir_opt_code_memssa_new_produce_consume_node(struct kefir_mem *, struct kefir_opt_code_memssa *,
                                                              kefir_opt_code_memssa_node_ref_t,
                                                              kefir_opt_instruction_ref_t,
                                                              kefir_opt_code_memssa_node_ref_t *);
kefir_result_t kefir_opt_code_memssa_new_phi_node(struct kefir_mem *, struct kefir_opt_code_memssa *,
                                                  kefir_opt_code_memssa_node_ref_t *);
kefir_result_t kefir_opt_code_memssa_new_terminate_node(struct kefir_mem *, struct kefir_opt_code_memssa *,
                                                        kefir_opt_code_memssa_node_ref_t,
                                                        kefir_opt_code_memssa_node_ref_t *);

kefir_result_t kefir_opt_code_memssa_phi_attach(struct kefir_mem *, struct kefir_opt_code_memssa *,
                                                kefir_opt_code_memssa_node_ref_t, kefir_opt_block_id_t,
                                                kefir_opt_code_memssa_node_ref_t);

kefir_result_t kefir_opt_code_memssa_instruction_binding(const struct kefir_opt_code_memssa *,
                                                         kefir_opt_instruction_ref_t,
                                                         kefir_opt_code_memssa_node_ref_t *);
kefir_result_t kefir_opt_code_memssa_node(const struct kefir_opt_code_memssa *, kefir_opt_code_memssa_node_ref_t,
                                          const struct kefir_opt_code_memssa_node **);

typedef struct kefir_opt_code_memssa_use_iterator {
    struct kefir_hashset_iterator iter;
} kefir_opt_code_memssa_use_iterator_t;

kefir_result_t kefir_opt_code_memssa_use_iter(const struct kefir_opt_code_memssa *,
                                              struct kefir_opt_code_memssa_use_iterator *,
                                              kefir_opt_code_memssa_node_ref_t, kefir_opt_code_memssa_node_ref_t *);
kefir_result_t kefir_opt_code_memssa_use_next(struct kefir_opt_code_memssa_use_iterator *,
                                              kefir_opt_code_memssa_node_ref_t *);

kefir_result_t kefir_opt_code_memssa_construct(struct kefir_mem *, struct kefir_opt_code_memssa *,
                                               const struct kefir_opt_code_container *,
                                               const struct kefir_opt_code_control_flow *,
                                               const struct kefir_opt_code_liveness *);

#endif
