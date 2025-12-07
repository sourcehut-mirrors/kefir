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

#include "kefir/core/graph.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static  kefir_result_t free_graph_vertex(struct kefir_mem *mem, struct kefir_hashtable *table,
                                                          kefir_hashtable_key_t key, kefir_hashtable_value_t value, void *payload) {
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

static kefir_result_t get_vertex(struct kefir_mem *mem, struct kefir_graph *graph, kefir_graph_vertex_id_t vertex, struct kefir_graph_vertex **vertex_ptr) {
    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&graph->vertices, (kefir_hashtable_key_t) vertex, &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        *vertex_ptr = (struct kefir_graph_vertex *) table_value;
    } else {
        struct kefir_graph_vertex *alloc_vertex = KEFIR_MALLOC(mem, sizeof(struct kefir_graph_vertex));
        REQUIRE(alloc_vertex != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate graph vertex"));
        res = kefir_hashset_init(&alloc_vertex->edges, &kefir_hashtable_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashtable_insert(mem, &graph->vertices, (kefir_hashtable_key_t) vertex, (kefir_hashtable_value_t) alloc_vertex));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, alloc_vertex);
            return res;
        });
        *vertex_ptr = alloc_vertex;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_graph_add_edge(struct kefir_mem *mem, struct kefir_graph *graph, kefir_graph_vertex_id_t src_vertex, kefir_graph_vertex_id_t dst_vertex) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(graph != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid graph"));

    struct kefir_graph_vertex *src = NULL;
    REQUIRE_OK(get_vertex(mem, graph, src_vertex, &src));
    REQUIRE_OK(kefir_hashset_add(mem, &src->edges, (kefir_hashset_key_t) dst_vertex));
    return KEFIR_OK;
}
