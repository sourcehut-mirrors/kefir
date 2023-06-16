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

#include "kefir/core/graph.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t free_node(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_graph_node *, node, value);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph node"));
    ASSIGN_DECL_CAST(struct kefir_graph *, graph, payload);
    REQUIRE(graph != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph"));

    if (graph->on_removal.callback != NULL) {
        REQUIRE_OK(graph->on_removal.callback(mem, graph, node->identifier, node->value, graph->on_removal.payload));
    }
    REQUIRE_OK(kefir_hashtreeset_free(mem, &node->edges));
    memset(node, 0, sizeof(struct kefir_graph_node));
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

kefir_result_t kefir_graph_init(struct kefir_graph *graph, const struct kefir_hashtree_ops *ops) {
    REQUIRE(graph != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to graph"));
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtree operations"));

    graph->on_removal.callback = NULL;
    graph->on_removal.payload = NULL;
    REQUIRE_OK(kefir_hashtree_init(&graph->nodes, ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&graph->nodes, free_node, graph));
    return KEFIR_OK;
}

kefir_result_t kefir_graph_free(struct kefir_mem *mem, struct kefir_graph *graph) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(graph != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph"));

    REQUIRE_OK(kefir_hashtree_free(mem, &graph->nodes));
    memset(graph, 0, sizeof(struct kefir_graph));
    return KEFIR_OK;
}

kefir_result_t kefir_graph_on_removal(struct kefir_graph *graph, kefir_graph_remove_callback_t callback,
                                      void *payload) {
    REQUIRE(graph != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph"));

    graph->on_removal.callback = callback;
    graph->on_removal.payload = payload;
    return KEFIR_OK;
}

kefir_result_t kefir_graph_node(const struct kefir_graph *graph, kefir_graph_node_id_t identifier,
                                struct kefir_graph_node **node_ptr) {
    REQUIRE(graph != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph"));
    REQUIRE(node_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to graph node"));

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&graph->nodes, (kefir_hashtree_key_t) identifier, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested graph node");
    }
    REQUIRE_OK(res);

    *node_ptr = (struct kefir_graph_node *) node->value;
    return KEFIR_OK;
}

kefir_result_t kefir_graph_node_set_value(struct kefir_mem *mem, struct kefir_graph *graph,
                                          kefir_graph_node_id_t identifier, kefir_graph_node_value_t value) {
    REQUIRE(graph != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph"));

    struct kefir_graph_node *node = NULL;
    REQUIRE_OK(kefir_graph_node(graph, identifier, &node));

    if (graph->on_removal.callback != NULL) {
        REQUIRE_OK(graph->on_removal.callback(mem, graph, identifier, value, graph->on_removal.payload));
    }
    node->value = value;
    return KEFIR_OK;
}

kefir_result_t kefir_graph_new_node(struct kefir_mem *mem, struct kefir_graph *graph, kefir_graph_node_id_t identifier,
                                    kefir_graph_node_value_t value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(graph != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph"));

    struct kefir_graph_node *node = KEFIR_MALLOC(mem, sizeof(struct kefir_graph_node));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate graph node"));

    node->identifier = identifier;
    node->value = value;
    kefir_result_t res = kefir_hashtreeset_init(&node->edges, graph->nodes.ops);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, node);
        return res;
    });

    res = kefir_hashtree_insert(mem, &graph->nodes, (kefir_hashtree_key_t) identifier, (kefir_hashtree_value_t) node);
    if (res == KEFIR_ALREADY_EXISTS) {
        res = KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Graph node with the same identifier already exists");
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &node->edges);
        KEFIR_FREE(mem, node);
        return res;
    });

    return KEFIR_OK;
}

kefir_result_t kefir_graph_new_edge(struct kefir_mem *mem, struct kefir_graph *graph, kefir_graph_node_id_t source,
                                    kefir_graph_node_id_t destination) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(graph != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph"));

    struct kefir_graph_node *node = NULL;
    REQUIRE_OK(kefir_graph_node(graph, source, &node));
    REQUIRE_OK(kefir_hashtreeset_add(mem, &node->edges, (kefir_hashtreeset_entry_t) destination));
    return KEFIR_OK;
}

kefir_result_t kefir_graph_node_iter(const struct kefir_graph *graph, struct kefir_graph_node_iterator *iter,
                                     kefir_graph_node_id_t *node_ptr) {
    REQUIRE(graph != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to graph node iterator"));

    const struct kefir_hashtree_node *node = kefir_hashtree_iter(&graph->nodes, &iter->iter);
    REQUIRE(node != NULL, KEFIR_ITERATOR_END);
    *node_ptr = node->key;
    return KEFIR_OK;
}

kefir_result_t kefir_graph_node_next(struct kefir_graph_node_iterator *iter, kefir_graph_node_id_t *node_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph node iterator"));

    const struct kefir_hashtree_node *node = kefir_hashtree_next(&iter->iter);
    REQUIRE(node != NULL, KEFIR_ITERATOR_END);
    *node_ptr = node->key;
    return KEFIR_OK;
}

kefir_result_t kefir_graph_edge_iter(const struct kefir_graph *graph, struct kefir_graph_edge_iterator *iter,
                                     kefir_graph_node_id_t source_id, kefir_graph_node_id_t *dest_ptr) {
    REQUIRE(graph != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to graph edge iterator"));

    struct kefir_graph_node *source = NULL;
    REQUIRE_OK(kefir_graph_node(graph, source_id, &source));

    REQUIRE_OK(kefir_hashtreeset_iter(&source->edges, &iter->iter));
    ASSIGN_PTR(dest_ptr, (kefir_graph_node_id_t) iter->iter.entry);
    return KEFIR_OK;
}

kefir_result_t kefir_graph_edge_next(struct kefir_graph_edge_iterator *iter, kefir_graph_node_id_t *dest_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to graph edge iterator"));

    REQUIRE_OK(kefir_hashtreeset_next(&iter->iter));
    ASSIGN_PTR(dest_ptr, (kefir_graph_node_id_t) iter->iter.entry);
    return KEFIR_OK;
}
