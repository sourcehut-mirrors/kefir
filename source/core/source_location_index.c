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

#define KEFIR_HASHTABLE_INTERNAL
#include "kefir/core/source_location_index.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_hashtable_hash_t hash_source_location(kefir_hashtable_key_t key, void *payload) {
    UNUSED(kefir_hashtable_uint_equal);
    UNUSED(kefir_hashtable_uint_hash);
    UNUSED(kefir_hashtable_str_equal);

    UNUSED(payload);

    ASSIGN_DECL_CAST(const struct kefir_source_location *, location, key);
    REQUIRE(location != NULL, 0);
    REQUIRE(location->source != NULL, 0);

    const kefir_hashtable_hash_t hash = kefir_hashtable_str_hash((kefir_hashtable_key_t) location->source, NULL);
    return kefir_splitmix64(hash) + kefir_splitmix64(location->line ^ KEFIR_SPLITMIX64_MAGIC1) + kefir_splitmix64(location->column ^ KEFIR_SPLITMIX64_MAGIC2);
}

static kefir_bool_t equal_source_location(kefir_hashtable_key_t key1, kefir_hashtable_key_t key2, void *payload) {
    UNUSED(payload);

    ASSIGN_DECL_CAST(const struct kefir_source_location *, location1, key1);
    ASSIGN_DECL_CAST(const struct kefir_source_location *, location2, key2);

    if (location1 == location2) {
        return true;
    } else if (location1 == NULL || location2 == NULL) {
        return false;
    } else if ((location1->source == NULL || location2->source == NULL) && location1->source != location2->source) {
        return false;
    }

    return strcmp(location1->source, location2->source) == 0 &&
        location1->line == location2->line &&
        location1->column == location2->column;
}


static struct kefir_hashtable_ops SOURCE_LOCATION_OPS = {
    .equal = equal_source_location,
    .hash = hash_source_location,
    .payload = NULL
};

static kefir_result_t free_source_location(struct kefir_mem *mem, struct kefir_hashtable *table,
                                                          kefir_hashtable_key_t key, kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(value);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_source_location *, loc, key);
    REQUIRE(loc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source location"));

    KEFIR_FREE(mem, loc);
    return KEFIR_OK;
}

kefir_result_t kefir_parser_source_location_index_init(struct kefir_parser_source_location_index *index) {
    REQUIRE(index != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to parser source location index"));

    REQUIRE_OK(kefir_hashtable_init(&index->locations, &SOURCE_LOCATION_OPS));
    REQUIRE_OK(kefir_hashtable_on_removal(&index->locations, free_source_location, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_parser_source_location_index_free(struct kefir_mem *mem, struct kefir_parser_source_location_index *index) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(index != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser source location index"));

    REQUIRE_OK(kefir_hashtable_free(mem, &index->locations));
    return KEFIR_OK;
}

kefir_result_t kefir_parser_source_location_index_reset(struct kefir_mem *mem, struct kefir_parser_source_location_index *index) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(index != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser source location index"));

    REQUIRE_OK(kefir_hashtable_clear(mem, &index->locations));
    return KEFIR_OK;
}

kefir_result_t kefir_parser_source_location_index_find(struct kefir_mem *mem,
    struct kefir_parser_source_location_index *index,
    const struct kefir_source_location *location, const struct kefir_source_location **location_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(index != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser source location index"));
    REQUIRE(location != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source location"));

    kefir_hashtable_key_t key;
    kefir_result_t res = kefir_hashtable_at2(&index->locations, (kefir_hashtable_key_t) location, &key, NULL);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_PTR(location_ptr, (const struct kefir_source_location *) key);
    } else {
        struct kefir_source_location *loc = KEFIR_MALLOC(mem, sizeof(struct kefir_source_location));
        REQUIRE(loc != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST node source location"));
        *loc = *location;

        kefir_result_t res = kefir_hashtable_insert(mem, &index->locations, (kefir_hashtable_key_t) loc, (kefir_hashtable_value_t) 0);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, loc);
            return res;
        });
        ASSIGN_PTR(location_ptr, loc);
    }

    return KEFIR_OK;
}
