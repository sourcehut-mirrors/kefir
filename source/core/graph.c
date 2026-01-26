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

#include "kefir/core/graph.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t free_graph_vertex(struct kefir_mem *mem, struct kefir_hashtable *table, kefir_hashtable_key_t key,
                                        kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_graph_vertex *, vertex, value);
    REQUIRE(vertex != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph vertex"));

    REQUIRE_OK(kefir_hashset_free(mem, &vertex->edges));
    KEFIR_FREE(mem, vertex);
    return KEFIR_OK;
}

kefir_result_t kefir_graph_init(struct kefir_graph *graph) {
    REQUIRE(graph != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to graph"));

    REQUIRE_OK(kefir_hashtable_init(&graph->vertices, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&graph->vertices, free_graph_vertex, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_graph_free(struct kefir_mem *mem, struct kefir_graph *graph) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(graph != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph"));

    REQUIRE_OK(kefir_hashtable_free(mem, &graph->vertices));
    return KEFIR_OK;
}

static kefir_result_t get_vertex(struct kefir_mem *mem, struct kefir_graph *graph, kefir_graph_vertex_id_t vertex,
                                 struct kefir_graph_vertex **vertex_ptr) {
    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&graph->vertices, (kefir_hashtable_key_t) vertex, &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        *vertex_ptr = (struct kefir_graph_vertex *) table_value;
    } else {
        struct kefir_graph_vertex *alloc_vertex = KEFIR_MALLOC(mem, sizeof(struct kefir_graph_vertex));
        REQUIRE(alloc_vertex != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate graph vertex"));
        res = kefir_hashset_init(&alloc_vertex->edges, &kefir_hashtable_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashtable_insert(mem, &graph->vertices, (kefir_hashtable_key_t) vertex,
                                                   (kefir_hashtable_value_t) alloc_vertex));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, alloc_vertex);
            return res;
        });
        *vertex_ptr = alloc_vertex;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_graph_clear(struct kefir_mem *mem, struct kefir_graph *graph) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(graph != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph"));

    REQUIRE_OK(kefir_hashtable_clear(mem, &graph->vertices));
    return KEFIR_OK;
}

kefir_bool_t kefir_graph_has_edge(const struct kefir_graph *graph, kefir_graph_vertex_id_t vertex1_id,
                                  kefir_graph_vertex_id_t vertex2_id) {
    REQUIRE(graph != NULL, false);

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&graph->vertices, (kefir_hashtable_key_t) vertex1_id, &table_value);
    REQUIRE(res == KEFIR_OK, false);
    ASSIGN_DECL_CAST(struct kefir_graph_vertex *, vertex1, table_value);

    return kefir_hashset_has(&vertex1->edges, (kefir_hashset_key_t) vertex2_id);
}

kefir_result_t kefir_graph_add_edge(struct kefir_mem *mem, struct kefir_graph *graph,
                                    kefir_graph_vertex_id_t src_vertex, kefir_graph_vertex_id_t dst_vertex) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(graph != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph"));

    struct kefir_graph_vertex *src = NULL;
    REQUIRE_OK(get_vertex(mem, graph, src_vertex, &src));
    REQUIRE_OK(kefir_hashset_add(mem, &src->edges, (kefir_hashset_key_t) dst_vertex));
    return KEFIR_OK;
}

kefir_result_t kefir_graph_ensure(struct kefir_mem *mem, struct kefir_graph *graph, kefir_graph_vertex_id_t vertex_id,
                                  kefir_size_t edges) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(graph != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph"));

    struct kefir_graph_vertex *vertex = NULL;
    REQUIRE_OK(get_vertex(mem, graph, vertex_id, &vertex));
    REQUIRE_OK(kefir_hashset_ensure(mem, &vertex->edges, edges));
    return KEFIR_OK;
}

kefir_result_t kefir_graph_iter(const struct kefir_graph *graph, struct kefir_graph_vertex_iterator *iter,
                                kefir_graph_vertex_id_t *vertex_id_ptr) {
    REQUIRE(graph != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph vertex iterator"));

    kefir_hashtable_key_t key;
    kefir_result_t res = kefir_hashtable_iter(&graph->vertices, &iter->iter, &key, NULL);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of graph vertex iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(vertex_id_ptr, (kefir_graph_vertex_id_t) key);
    return KEFIR_OK;
}

kefir_result_t kefir_graph_next(struct kefir_graph_vertex_iterator *iter, kefir_graph_vertex_id_t *vertex_id_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph vertex iterator"));

    kefir_hashtable_key_t key;
    kefir_result_t res = kefir_hashtable_next(&iter->iter, &key, NULL);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of graph vertex iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(vertex_id_ptr, (kefir_graph_vertex_id_t) key);
    return KEFIR_OK;
}

kefir_result_t kefir_graph_edge_iter(const struct kefir_graph *graph, struct kefir_graph_edge_iterator *iter,
                                     kefir_graph_vertex_id_t vertex_id, kefir_graph_vertex_id_t *dst_vertex_id_ptr) {
    REQUIRE(graph != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph edge iterator"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&graph->vertices, (kefir_hashtable_key_t) vertex_id, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of graph edge iterator");
    }
    REQUIRE_OK(res);
    ASSIGN_DECL_CAST(const struct kefir_graph_vertex *, vertex, table_value);

    kefir_hashset_key_t key;
    res = kefir_hashset_iter(&vertex->edges, &iter->iter, &key);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of graph edge iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(dst_vertex_id_ptr, key);
    return KEFIR_OK;
}

kefir_result_t kefir_graph_edge_next(struct kefir_graph_edge_iterator *iter,
                                     kefir_graph_vertex_id_t *dst_vertex_id_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph edge iterator"));

    kefir_hashset_key_t key;
    kefir_result_t res = kefir_hashset_next(&iter->iter, &key);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of graph edge iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(dst_vertex_id_ptr, key);
    return KEFIR_OK;
}
