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

#include "kefir/codegen/asmcmp/liveness.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_asmcmp_lifetime_map_init(struct kefir_asmcmp_lifetime_map *map) {
    REQUIRE(map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp liveness map"));

    map->length = 0;
    map->capacity = 0;
    map->map = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_lifetime_map_free(struct kefir_mem *mem, struct kefir_asmcmp_lifetime_map *map) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp liveness map"));

    if (map->map != NULL) {
        for (kefir_size_t i = 0; i < map->length; i++) {
            REQUIRE_OK(kefir_hashtree_free(mem, &map->map[i].lifetime_ranges));
        }
        memset(map->map, 0, sizeof(struct kefir_asmcmp_virtual_register_lifetime_map) * map->length);
        KEFIR_FREE(mem, map->map);
    }
    memset(map, 0, sizeof(struct kefir_asmcmp_lifetime_map));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_lifetime_map_resize(struct kefir_mem *mem, struct kefir_asmcmp_lifetime_map *map,
                                                kefir_size_t new_length) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp liveness map"));
    REQUIRE(map->length != new_length, KEFIR_OK);

    if (new_length < map->length) {
        for (kefir_size_t i = new_length; i < map->length; i++) {
            REQUIRE_OK(kefir_hashtree_free(mem, &map->map[i].lifetime_ranges));
        }
        map->length = new_length;
        return KEFIR_OK;
    }

    if (new_length > map->capacity) {
        kefir_size_t new_capacity = new_length + 128;
        struct kefir_asmcmp_virtual_register_lifetime_map *new_map =
            KEFIR_MALLOC(mem, sizeof(struct kefir_asmcmp_virtual_register_lifetime_map) * new_capacity);
        REQUIRE(new_map != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate virtual register liveness map"));
        if (map->map != NULL) {
            memcpy(new_map, map->map, sizeof(struct kefir_asmcmp_virtual_register_lifetime_map) * map->length);
            memset(map->map, 0, sizeof(struct kefir_asmcmp_virtual_register_lifetime_map) * map->length);
            KEFIR_FREE(mem, map->map);
        }
        map->map = new_map;
        map->capacity = new_capacity;
    }

    for (kefir_size_t i = map->length; i < new_length; i++) {
        REQUIRE_OK(kefir_hashtree_init(&map->map[i].lifetime_ranges, &kefir_hashtree_uint_ops));
        map->map[i].global_activity_range.begin = KEFIR_ASMCMP_INDEX_NONE;
        map->map[i].global_activity_range.end = KEFIR_ASMCMP_INDEX_NONE;
    }
    map->length = new_length;
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_lifetime_map_mark_activity(struct kefir_asmcmp_lifetime_map *map,
                                                       kefir_asmcmp_virtual_register_index_t vreg,
                                                       kefir_asmcmp_lifetime_index_t index) {
    REQUIRE(map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp liveness map"));
    REQUIRE(vreg < map->length,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided virtual register is out of liveness map bounds"));

    struct kefir_asmcmp_virtual_register_lifetime_map *const reg_map = &map->map[vreg];

    if (reg_map->global_activity_range.begin != KEFIR_ASMCMP_INDEX_NONE) {
        reg_map->global_activity_range.begin = MIN(reg_map->global_activity_range.begin, index);
        reg_map->global_activity_range.end = MAX(reg_map->global_activity_range.end, index);
    } else {
        reg_map->global_activity_range.begin = index;
        reg_map->global_activity_range.end = index;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_lifetime_map_add_lifetime_range(struct kefir_mem *mem,
                                                            struct kefir_asmcmp_lifetime_map *map,
                                                            kefir_asmcmp_virtual_register_index_t vreg,
                                                            kefir_asmcmp_lifetime_index_t begin,
                                                            kefir_asmcmp_lifetime_index_t end) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp liveness map"));
    REQUIRE(vreg < map->length,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided virtual register is out of liveness map bounds"));

    struct kefir_asmcmp_virtual_register_lifetime_map *const reg_map = &map->map[vreg];

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_lower_bound(&reg_map->lifetime_ranges, (kefir_hashtree_key_t) begin, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(kefir_asmcmp_lifetime_index_t, prev_begin, node->key);
        ASSIGN_DECL_CAST(kefir_asmcmp_lifetime_index_t, prev_end, node->value);
        REQUIRE(prev_begin <= begin,
                KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected lower bound node to be a predecessor of provided key"));

        if (begin <= prev_end) {
            end = MAX(prev_end, end);
            node->value = (kefir_hashtree_value_t) end;
        } else {
            node = NULL;
        }
    } else {
        node = NULL;
    }

    if (node == NULL) {
        REQUIRE_OK(kefir_hashtree_insert(mem, &reg_map->lifetime_ranges, (kefir_hashtree_key_t) begin,
                                         (kefir_hashtree_value_t) end));
        REQUIRE_OK(kefir_hashtree_at(&reg_map->lifetime_ranges, (kefir_hashtree_key_t) begin, &node));
    }

    struct kefir_hashtree_node *next_node = kefir_hashtree_next_node(&reg_map->lifetime_ranges, node);
    while (next_node != NULL && next_node->key > node->key && next_node->key <= node->value) {
        node->value = MAX(node->value, next_node->value);
        REQUIRE_OK(kefir_hashtree_delete(mem, &reg_map->lifetime_ranges, next_node->key));
        next_node = kefir_hashtree_next_node(&reg_map->lifetime_ranges, node);
    }

    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_lifetime_global_activity_for(const struct kefir_asmcmp_lifetime_map *map,
                                                         kefir_asmcmp_virtual_register_index_t vreg,
                                                         kefir_asmcmp_lifetime_index_t *begin_ptr,
                                                         kefir_asmcmp_lifetime_index_t *end_ptr) {
    REQUIRE(map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp liveness map"));
    REQUIRE(vreg < map->length,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided virtual register is out of liveness map bounds"));
    REQUIRE(begin_ptr != NULL || end_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to range ends"));

    const struct kefir_asmcmp_virtual_register_lifetime_map *const reg_map = &map->map[vreg];

    ASSIGN_PTR(begin_ptr, reg_map->global_activity_range.begin);
    ASSIGN_PTR(end_ptr, reg_map->global_activity_range.end);
    return KEFIR_OK;
}
kefir_result_t kefir_asmcmp_get_active_lifetime_range_for(const struct kefir_asmcmp_lifetime_map *map,
                                                          kefir_asmcmp_virtual_register_index_t vreg,
                                                          kefir_asmcmp_lifetime_index_t linear_position,
                                                          kefir_asmcmp_lifetime_index_t *begin_ptr,
                                                          kefir_asmcmp_lifetime_index_t *end_ptr) {
    REQUIRE(map != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp liveness map"));
    REQUIRE(vreg < map->length,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided virtual register is out of liveness map bounds"));
    REQUIRE(begin_ptr != NULL || end_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to range ends"));

    const struct kefir_asmcmp_virtual_register_lifetime_map *const reg_map = &map->map[vreg];

    kefir_asmcmp_lifetime_index_t begin = reg_map->global_activity_range.begin;
    kefir_asmcmp_lifetime_index_t end = reg_map->global_activity_range.end;

    if (!kefir_hashtree_empty(&reg_map->lifetime_ranges)) {
        struct kefir_hashtree_node *node = NULL;
        kefir_result_t res =
            kefir_hashtree_lower_bound(&reg_map->lifetime_ranges, (kefir_hashtree_key_t) linear_position, &node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            if (linear_position >= node->key && linear_position <= node->value) {
                begin = MAX(begin, node->key);
                end = MIN(end, node->value);
            } else {
                begin = KEFIR_ASMCMP_INDEX_NONE;
                end = KEFIR_ASMCMP_INDEX_NONE;
            }
        } else {
            begin = KEFIR_ASMCMP_INDEX_NONE;
            end = KEFIR_ASMCMP_INDEX_NONE;
        }
    }

    ASSIGN_PTR(begin_ptr, begin);
    ASSIGN_PTR(end_ptr, end);

    return KEFIR_OK;
}
