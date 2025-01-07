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

#include "kefir/codegen/asmcmp/debug.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

struct source_map_entry {
    kefir_asmcmp_instruction_index_t begin;
    kefir_asmcmp_instruction_index_t end;
    struct kefir_source_location source_location;
};

static kefir_result_t free_source_map_entry(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                                                    kefir_hashtree_key_t key, kefir_hashtree_value_t value,
                                                                    void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct source_map_entry *, entry,
        value);
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp source map entry"));

    memset(entry, 0, sizeof(struct source_map_entry));
    KEFIR_FREE(mem, entry);
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_source_map_init(struct kefir_asmcmp_source_map *source_map) {
    REQUIRE(source_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp source map"));

    REQUIRE_OK(kefir_hashtree_init(&source_map->map, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&source_map->map, free_source_map_entry, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_source_map_free(struct kefir_mem *mem, struct kefir_asmcmp_source_map *source_map) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(source_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp source map"));

    REQUIRE_OK(kefir_hashtree_free(mem, &source_map->map));
    memset(source_map, 0, sizeof(struct kefir_asmcmp_source_map));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_debug_info_init(struct kefir_asmcmp_debug_info *debug_info) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp debug info"));

    REQUIRE_OK(kefir_asmcmp_source_map_init(&debug_info->source_map));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_debug_info_free(struct kefir_mem *mem, struct kefir_asmcmp_debug_info *debug_info) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp debug info"));

    REQUIRE_OK(kefir_asmcmp_source_map_free(mem, &debug_info->source_map));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_source_map_add_location(struct kefir_mem *mem, struct kefir_asmcmp_source_map *source_map, struct kefir_string_pool *symbols, kefir_asmcmp_instruction_index_t begin, kefir_asmcmp_instruction_index_t end, const struct kefir_source_location *location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(source_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp source map"));
    REQUIRE(begin != end, KEFIR_OK);
    REQUIRE(begin < end, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Asmcmp source map begin index shall preceede end index"));
    REQUIRE(location != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source location"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_lower_bound(&source_map->map, (kefir_hashtree_key_t) begin, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(struct source_map_entry *, entry,
            node->value);
        REQUIRE(begin >= entry->end, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Overlapping asmcmp index ranges in location assignment"));
    }

    res = kefir_hashtree_lower_bound(&source_map->map, (kefir_hashtree_key_t) end, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(struct source_map_entry *, entry,
            node->value);
        REQUIRE(begin >= entry->end, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Overlapping asmcmp index ranges in location assignment"));
    }

    struct source_map_entry *entry = KEFIR_MALLOC(mem, sizeof(struct source_map_entry));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate asmcmp source map entry"));

    if (symbols != NULL) {
        entry->source_location.source = kefir_string_pool_insert(mem, symbols, location->source, NULL);
        REQUIRE_ELSE(entry->source_location.source != NULL, {
            KEFIR_FREE(mem, entry);
            return KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert source location into string pool");
        });
    } else {
        entry->source_location.source = location->source;
    }
    entry->source_location.line = location->line;
    entry->source_location.column = location->column;
    entry->begin = begin;
    entry->end = end;

    res = kefir_hashtree_insert(mem, &source_map->map, (kefir_hashtree_key_t) begin, (kefir_hashtree_value_t) entry);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, entry);
        return res;
    });

    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_source_map_at(const struct kefir_asmcmp_source_map *source_map, kefir_asmcmp_instruction_index_t index, const struct kefir_source_location **location_ptr) {
    REQUIRE(source_map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp source map"));
    REQUIRE(location_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer source location"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_lower_bound(&source_map->map, (kefir_hashtree_key_t) index, &node);
    if (res == KEFIR_NOT_FOUND) {   
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested location in asmcmp source map");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(const struct source_map_entry *, entry,
        node->value);
    REQUIRE(index >= entry->begin && index < entry->end,
        KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested location in asmcmp source map"));
    *location_ptr = &entry->source_location;
    return KEFIR_OK;
}
