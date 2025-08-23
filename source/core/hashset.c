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

#define KEFIR_HASHTABLE_INTERNAL
#include "kefir/core/hashset.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_hashset_init(struct kefir_hashset *hashset, const struct kefir_hashtable_ops *ops) {
    REQUIRE(hashset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to hashset"));
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashset operations"));

    hashset->entries = NULL;
    hashset->capacity = 0;
    hashset->occupied = 0;
    hashset->ops = ops;
    return KEFIR_OK;
}

kefir_result_t kefir_hashset_free(struct kefir_mem *mem, struct kefir_hashset *hashset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(hashset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashset"));

    KEFIR_FREE(mem, hashset->entries);
    memset(hashset, 0, sizeof(struct kefir_hashset));
    return KEFIR_OK;
}

static kefir_result_t find_position_for_insert(const struct kefir_hashtable_ops *ops,
                                               struct kefir_hashset_entry *entries, kefir_size_t capacity,
                                               kefir_hashset_key_t key, kefir_size_t *position_ptr,
                                               kefir_size_t *collisions_ptr) {
    KEFIR_HASHTABLE_FIND_POSITION_FOR_INSERT(ops, entries, capacity, key, position_ptr, collisions_ptr);
}

static kefir_result_t rehash(struct kefir_mem *, struct kefir_hashset *);

static kefir_result_t insert_entry(struct kefir_mem *mem, struct kefir_hashset *hashset,
                                   struct kefir_hashset_entry *entries, kefir_size_t capacity, kefir_size_t *occupied,
                                   kefir_hashset_key_t key, kefir_bool_t do_rehash) {
    kefir_size_t index = 0;
    kefir_size_t found_collisions = 0;
    REQUIRE_OK(find_position_for_insert(hashset->ops, entries, capacity, key, &index, &found_collisions));
    if (do_rehash && entries[index].state != KEFIR_HASHTABLE_ENTRY_OCCUPIED &&
        found_collisions >= KEFIR_REHASH_COLLISION_THRESHOLD) {
        REQUIRE_OK(rehash(mem, hashset));
        return insert_entry(mem, hashset, hashset->entries, hashset->capacity, &hashset->occupied, key, false);
    }

    if (entries[index].state != KEFIR_HASHTABLE_ENTRY_OCCUPIED) {
        entries[index].state = KEFIR_HASHTABLE_ENTRY_OCCUPIED;
        entries[index].key = key;
        (*occupied)++;
    }
    return KEFIR_OK;
}

static kefir_result_t rehash(struct kefir_mem *mem, struct kefir_hashset *hashset) {
    const kefir_size_t new_capacity = KEFIR_HASHTABLE_CAPACITY_GROW(hashset->capacity);
    kefir_size_t new_occupied = 0;
    struct kefir_hashset_entry *new_entries = KEFIR_MALLOC(mem, sizeof(struct kefir_hashset_entry) * new_capacity);
    for (kefir_size_t i = 0; i < new_capacity; i++) {
        new_entries[i].state = KEFIR_HASHTABLE_ENTRY_EMPTY;
    }

    kefir_result_t res = KEFIR_OK;
    for (kefir_size_t i = 0; res == KEFIR_OK && i < hashset->capacity; i++) {
        if (hashset->entries[i].state == KEFIR_HASHTABLE_ENTRY_OCCUPIED) {
            res = insert_entry(mem, hashset, new_entries, new_capacity, &new_occupied, hashset->entries[i].key, false);
        }
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, new_entries);
        return res;
    });

    KEFIR_FREE(mem, hashset->entries);
    hashset->entries = new_entries;
    hashset->capacity = new_capacity;
    hashset->occupied = new_occupied;
    return KEFIR_OK;
}

kefir_result_t kefir_hashset_clear(struct kefir_mem *mem, struct kefir_hashset *hashset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(hashset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashset"));

    for (kefir_size_t i = 0; i < hashset->capacity; i++) {
        hashset->entries[i].state = KEFIR_HASHTABLE_ENTRY_EMPTY;
    }
    hashset->occupied = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_hashset_add(struct kefir_mem *mem, struct kefir_hashset *hashset, kefir_hashset_key_t key) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(hashset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashset"));

    kefir_bool_t did_rehash = false;
    if (hashset->capacity == 0 || hashset->occupied >= KEFIR_REHASH_OCCUPATION_THRESHOLD * hashset->capacity) {
        REQUIRE_OK(rehash(mem, hashset));
        did_rehash = true;
    }

    REQUIRE_OK(insert_entry(mem, hashset, hashset->entries, hashset->capacity, &hashset->occupied, key, !did_rehash));
    return KEFIR_OK;
}

kefir_result_t kefir_hashset_merge(struct kefir_mem *mem, struct kefir_hashset *dst_hashset,
                                   const struct kefir_hashset *src_hashset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(dst_hashset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination hashset"));
    REQUIRE(src_hashset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source hashset"));

    if (src_hashset->occupied > 0) {
        for (kefir_size_t i = 0; i < src_hashset->capacity; i++) {
            if (src_hashset->entries[i].state == KEFIR_HASHTABLE_ENTRY_OCCUPIED) {
                REQUIRE_OK(kefir_hashset_add(mem, dst_hashset, src_hashset->entries[i].key));
            }
        }
    }
    return KEFIR_OK;
}

kefir_bool_t kefir_hashset_has(const struct kefir_hashset *hashset, kefir_hashset_key_t key) {
    REQUIRE(hashset != NULL, false);

    KEFIR_HASHTABLE_HAS(hashset, key, return true;, return false;);
}

kefir_result_t kefir_hashset_iter(const struct kefir_hashset *hashset, struct kefir_hashset_iterator *iter,
                                  kefir_hashset_key_t *key_ptr) {
    REQUIRE(hashset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashset"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to hashset iterator"));

    iter->hashset = hashset;
    iter->index = 0;

    for (; iter->index < hashset->capacity; iter->index++) {
        if (hashset->entries[iter->index].state == KEFIR_HASHTABLE_ENTRY_OCCUPIED) {
            ASSIGN_PTR(key_ptr, hashset->entries[iter->index].key);
            return KEFIR_OK;
        }
    }

    return KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of hashset iterator");
}

kefir_result_t kefir_hashset_next(struct kefir_hashset_iterator *iter, kefir_hashset_key_t *key_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashset iterator"));

    iter->index++;
    for (; iter->index < iter->hashset->capacity; iter->index++) {
        if (iter->hashset->entries[iter->index].state == KEFIR_HASHTABLE_ENTRY_OCCUPIED) {
            ASSIGN_PTR(key_ptr, iter->hashset->entries[iter->index].key);
            return KEFIR_OK;
        }
    }

    return KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of hashset iterator");
}
