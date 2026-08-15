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

#ifndef KEFIR_OPTIMIZER_VARIABLE_SCOPE_H_
#define KEFIR_OPTIMIZER_VARIABLE_SCOPE_H_

#include "kefir/optimizer/liveness.h"
#include "kefir/core/hashset.h"
#include "kefir/core/graph.h"

typedef struct kefir_opt_code_variable_scope {
    kefir_bool_t global;
    struct kefir_hashset blocks;
    struct kefir_hashset allocations;
} kefir_opt_code_variable_scope_t;

typedef struct kefir_opt_code_block_variable_scopes {
    struct kefir_hashset scopes;
} kefir_opt_code_block_variable_scopes_t;

typedef struct kefir_opt_code_variable_scopes {
    struct kefir_graph scope_interference;
    struct kefir_hashtree scopes;
    struct kefir_hashtree block_scopes;
    struct kefir_hashset global_scopes;
} kefir_opt_code_variable_scopes_t;

kefir_result_t kefir_opt_code_variable_scopes_init(struct kefir_opt_code_variable_scopes *);
kefir_result_t kefir_opt_code_variable_scopes_free(struct kefir_mem *, struct kefir_opt_code_variable_scopes *);

kefir_result_t kefir_opt_code_variable_scopes_build(struct kefir_mem *, struct kefir_opt_code_variable_scopes *,
                                                    const struct kefir_opt_code_liveness *);

typedef struct kefir_opt_code_variable_scope_interference_iterator {
    struct kefir_graph_edge_iterator edge_iter;
} kefir_opt_code_variable_scope_interference_iterator_t;

kefir_result_t kefir_opt_code_variable_scope_interference_iter(const struct kefir_opt_code_variable_scopes *, struct kefir_opt_code_variable_scope_interference_iterator *, kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_variable_scope_interference_next(struct kefir_opt_code_variable_scope_interference_iterator *, kefir_opt_instruction_ref_t *);

#endif
