/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#include "kefir/core/hashtree.h"
#include "kefir/core/hashtreeset.h"

typedef kefir_hashtree_key_t kefir_graph_node_id_t;
typedef kefir_hashtree_value_t kefir_graph_node_value_t;

typedef struct kefir_graph_node {
    kefir_graph_node_id_t identifier;
    kefir_graph_node_value_t value;
    struct kefir_hashtreeset edges;
} kefir_graph_node_t;

typedef struct kefir_graph kefir_graph_t;

typedef kefir_result_t (*kefir_graph_remove_callback_t)(struct kefir_mem *, struct kefir_graph *, kefir_graph_node_id_t,
                                                        kefir_graph_node_value_t, void *);

typedef struct kefir_graph {
    struct kefir_hashtree nodes;

    struct {
        kefir_graph_remove_callback_t callback;
        void *payload;
    } on_removal;
} kefir_graph_t;

kefir_result_t kefir_graph_init(struct kefir_graph *, const struct kefir_hashtree_ops *);
kefir_result_t kefir_graph_free(struct kefir_mem *, struct kefir_graph *);
kefir_result_t kefir_graph_on_removal(struct kefir_graph *, kefir_graph_remove_callback_t, void *);

kefir_result_t kefir_graph_node(const struct kefir_graph *, kefir_graph_node_id_t, struct kefir_graph_node **);
kefir_result_t kefir_graph_new_node(struct kefir_mem *, struct kefir_graph *, kefir_graph_node_id_t,
                                    kefir_graph_node_value_t);
kefir_result_t kefir_graph_new_edge(struct kefir_mem *, struct kefir_graph *, kefir_graph_node_id_t,
                                    kefir_graph_node_id_t);
kefir_result_t kefir_graph_node_set_value(struct kefir_mem *, struct kefir_graph *, kefir_graph_node_id_t,
                                          kefir_graph_node_value_t);

typedef struct kefir_graph_node_iterator {
    struct kefir_hashtree_node_iterator iter;
} kefir_graph_node_iterator_t;

kefir_result_t kefir_graph_node_iter(const struct kefir_graph *, struct kefir_graph_node_iterator *,
                                     kefir_graph_node_id_t *);
kefir_result_t kefir_graph_node_next(struct kefir_graph_node_iterator *, kefir_graph_node_id_t *);

typedef struct kefir_graph_edge_iterator {
    struct kefir_hashtreeset_iterator iter;
} kefir_graph_edge_iterator_t;

kefir_result_t kefir_graph_edge_iter(const struct kefir_graph *, struct kefir_graph_edge_iterator *,
                                     kefir_graph_node_id_t, kefir_graph_node_id_t *);
kefir_result_t kefir_graph_edge_next(struct kefir_graph_edge_iterator *, kefir_graph_node_id_t *);

#endif
