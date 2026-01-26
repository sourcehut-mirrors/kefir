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

#ifndef KEFIR_CORE_GRAPH_H_
#define KEFIR_CORE_GRAPH_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/mem.h"
#include "kefir/core/hashtable.h"
#include "kefir/core/hashset.h"

typedef kefir_uint64_t kefir_graph_vertex_id_t;

typedef struct kefir_graph_vertex {
    struct kefir_hashset edges;
} kefir_graph_vertex_t;

typedef struct kefir_graph {
    struct kefir_hashtable vertices;
} kefir_graph_t;

kefir_result_t kefir_graph_init(struct kefir_graph *);
kefir_result_t kefir_graph_free(struct kefir_mem *, struct kefir_graph *);

kefir_result_t kefir_graph_clear(struct kefir_mem *, struct kefir_graph *);
kefir_bool_t kefir_graph_has_edge(const struct kefir_graph *, kefir_graph_vertex_id_t, kefir_graph_vertex_id_t);
kefir_result_t kefir_graph_add_edge(struct kefir_mem *, struct kefir_graph *, kefir_graph_vertex_id_t, kefir_graph_vertex_id_t);
kefir_result_t kefir_graph_ensure(struct kefir_mem *, struct kefir_graph *, kefir_graph_vertex_id_t, kefir_size_t);

typedef struct kefir_graph_vertex_iterator {
    struct kefir_hashtable_iterator iter;
} kefir_graph_vertex_iterator_t;

kefir_result_t kefir_graph_iter(const struct kefir_graph *, struct kefir_graph_vertex_iterator *, kefir_graph_vertex_id_t *);
kefir_result_t kefir_graph_next(struct kefir_graph_vertex_iterator *, kefir_graph_vertex_id_t *);

typedef struct kefir_graph_edge_iterator {
    struct kefir_hashset_iterator iter;
} kefir_graph_edge_iterator_t;

kefir_result_t kefir_graph_edge_iter(const struct kefir_graph *, struct kefir_graph_edge_iterator *, kefir_graph_vertex_id_t, kefir_graph_vertex_id_t *);
kefir_result_t kefir_graph_edge_next(struct kefir_graph_edge_iterator *, kefir_graph_vertex_id_t *);

#endif