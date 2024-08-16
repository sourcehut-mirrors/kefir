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

#include "kefir/ir/debug.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t on_source_location_remove(struct kefir_mem *mem, struct kefir_interval_tree *tree,
                                                kefir_interval_tree_key_t begin, kefir_interval_tree_key_t end,
                                                kefir_interval_tree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(begin);
    UNUSED(end);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_ir_source_location *, source_location, value);
    REQUIRE(source_location != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR source location"));

    while (source_location != NULL) {
        struct kefir_ir_source_location *next_location = source_location->next;
        memset(source_location, 0, sizeof(struct kefir_ir_source_location));
        KEFIR_FREE(mem, source_location);
        source_location = next_location;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ir_source_map_init(struct kefir_ir_source_map *source_map) {
    REQUIRE(source_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR source map"));

    REQUIRE_OK(kefir_interval_tree_init(&source_map->locations));
    REQUIRE_OK(kefir_interval_tree_on_remove(&source_map->locations, on_source_location_remove, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_source_map_free(struct kefir_mem *mem, struct kefir_ir_source_map *source_map) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(source_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR source map"));

    REQUIRE_OK(kefir_interval_tree_free(mem, &source_map->locations));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_source_map_insert(struct kefir_mem *mem, struct kefir_ir_source_map *source_map,
                                          struct kefir_string_pool *strings,
                                          const struct kefir_source_location *source_location, kefir_size_t begin,
                                          kefir_size_t end) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(source_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR source map"));
    REQUIRE(source_location != NULL && source_location->source != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source location"));
    REQUIRE(begin <= end, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Source location IR range shall be non-negative"));

    struct kefir_ir_source_location *target = KEFIR_MALLOC(mem, sizeof(struct kefir_ir_source_location));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR source location"));
    if (strings != NULL) {
        target->location.source = kefir_string_pool_insert(mem, strings, source_location->source, NULL);
        REQUIRE_ELSE(target->location.source != NULL, {
            KEFIR_FREE(mem, target);
            return KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                   "Failed to insert source location identifier into a string pool");
        });
    } else {
        target->location.source = source_location->source;
    }
    target->location.line = source_location->line;
    target->location.column = source_location->column;
    target->begin = begin;
    target->end = end;
    target->next = NULL;

    kefir_result_t res =
        kefir_interval_tree_insert(mem, &source_map->locations, (kefir_interval_tree_key_t) begin,
                                   (kefir_interval_tree_key_t) end, (kefir_interval_tree_value_t) target);
    if (res == KEFIR_ALREADY_EXISTS) {
        struct kefir_interval_tree_node *node = NULL;
        res = kefir_interval_tree_get(&source_map->locations, (kefir_interval_tree_key_t) begin,
                                      (kefir_interval_tree_key_t) end, &node);
        if (res == KEFIR_OK) {
            ASSIGN_DECL_CAST(struct kefir_ir_source_location *, next_target, node->value);

            target->next = next_target;
            node->value = (kefir_interval_tree_value_t) target;
        }
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, target);
        return res;
    });

    return KEFIR_OK;
}

kefir_result_t kefir_ir_source_map_find(const struct kefir_ir_source_map *source_map, kefir_size_t positon,
                                        const struct kefir_ir_source_location **location_ptr) {
    REQUIRE(source_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR source map"));
    REQUIRE(location_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR source location"));

    struct kefir_interval_tree_finder finder;
    kefir_result_t res;
    struct kefir_interval_tree_node *node;
    const struct kefir_ir_source_location *most_precise_location = NULL;
    for (res = kefir_interval_tree_find(&source_map->locations, (kefir_interval_tree_key_t) positon, &finder, &node);
         res == KEFIR_OK; res = kefir_interval_tree_find_next(&source_map->locations, &finder, &node)) {

        ASSIGN_DECL_CAST(const struct kefir_ir_source_location *, source_location, node->value);
        for (; source_location != NULL; source_location = source_location->next) {
            REQUIRE(positon >= source_location->begin && positon < source_location->end,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected source location"));
#define WIDTH(_loc) ((_loc)->end - (_loc)->begin)
            if (most_precise_location == NULL || WIDTH(source_location) <= WIDTH(most_precise_location)) {
                most_precise_location = source_location;
            }
#undef WIDTH
        }
    }

    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    REQUIRE(most_precise_location != NULL,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find source location for requested position"));
    *location_ptr = most_precise_location;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_source_map_iter(const struct kefir_ir_source_map *source_map,
                                        struct kefir_ir_source_map_iterator *iter,
                                        const struct kefir_ir_source_location **location_ptr) {
    REQUIRE(source_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR source map"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR source map iterator"));

    struct kefir_interval_tree_node *node;
    REQUIRE_OK(kefir_interval_tree_iter(&source_map->locations, &iter->iter, &node));
    ASSIGN_DECL_CAST(const struct kefir_ir_source_location *, source_location, node->value);
    iter->source_location = source_location->next;
    ASSIGN_PTR(location_ptr, source_location);
    return KEFIR_OK;
}

kefir_result_t kefir_ir_source_map_next(struct kefir_ir_source_map_iterator *iter,
                                        const struct kefir_ir_source_location **location_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR source map iterator"));

    if (iter->source_location == NULL) {
        struct kefir_interval_tree_node *node;
        REQUIRE_OK(kefir_interval_tree_next(&iter->iter, &node));
        ASSIGN_DECL_CAST(const struct kefir_ir_source_location *, source_location, node->value);
        iter->source_location = source_location->next;
        ASSIGN_PTR(location_ptr, source_location);
    } else {
        const struct kefir_ir_source_location *source_location = iter->source_location;
        iter->source_location = source_location->next;
        ASSIGN_PTR(location_ptr, source_location);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ir_function_debug_info_init(struct kefir_ir_function_debug_info *debug_info) {
    REQUIRE(debug_info != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR function debug info"));

    debug_info->source_location.source = NULL;
    debug_info->source_location.line = 0;
    debug_info->source_location.column = 0;
    REQUIRE_OK(kefir_ir_source_map_init(&debug_info->source_map));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_function_debug_info_free(struct kefir_mem *mem,
                                                 struct kefir_ir_function_debug_info *debug_info) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR function debug info"));

    REQUIRE_OK(kefir_ir_source_map_free(mem, &debug_info->source_map));
    return KEFIR_OK;
}

const struct kefir_source_location *kefir_ir_function_debug_info_source_location(
    const struct kefir_ir_function_debug_info *debug_info) {
    REQUIRE(debug_info != NULL, NULL);
    REQUIRE(debug_info->source_location.source != NULL, NULL);
    return &debug_info->source_location;
}

kefir_result_t kefir_ir_function_debug_info_set_source_location(struct kefir_mem *mem,
                                                                struct kefir_ir_function_debug_info *debug_info,
                                                                struct kefir_string_pool *symbols,
                                                                const struct kefir_source_location *location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR function debug info"));

    if (location != NULL && location->source != NULL) {
        if (symbols != NULL) {
            debug_info->source_location.source = kefir_string_pool_insert(mem, symbols, location->source, NULL);
            REQUIRE(debug_info->source_location.source != NULL,
                    KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                    "Failed to insert IR function source location into string pool"));
        } else {
            debug_info->source_location.source = location->source;
        }
        debug_info->source_location.line = location->line;
        debug_info->source_location.column = location->column;
    } else {
        debug_info->source_location.source = NULL;
        debug_info->source_location.line = 0;
        debug_info->source_location.column = 0;
    }
    return KEFIR_OK;
}
