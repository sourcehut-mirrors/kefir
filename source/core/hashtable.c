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
#include "kefir/core/hashtable.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/hash.h"
#include <string.h>

kefir_result_t kefir_hashtable_init(struct kefir_hashtable *hashtable, const struct kefir_hashtable_ops *ops) {
    REQUIRE(hashtable != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to hashtable"));
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtable operations"));

    hashtable->entries = NULL;
    hashtable->capacity = 0;
    hashtable->occupied = 0;
    hashtable->ops = ops;
    hashtable->cleanup.callback = NULL;
    hashtable->cleanup.payload = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_hashtable_free(struct kefir_mem *mem, struct kefir_hashtable *hashtable) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(hashtable != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtable"));

    if (hashtable->cleanup.callback != NULL) {
        for (kefir_size_t i = 0; i < hashtable->capacity; i++) {
            if (hashtable->entries[i].state == KEFIR_HASHTABLE_ENTRY_OCCUPIED) {
                REQUIRE_OK(hashtable->cleanup.callback(mem, hashtable, hashtable->entries[i].key,
                                                       hashtable->entries[i].value, hashtable->cleanup.payload));
            }
        }
    }
    KEFIR_FREE(mem, hashtable->entries);
    memset(hashtable, 0, sizeof(struct kefir_hashtable));
    return KEFIR_OK;
}

kefir_result_t kefir_hashtable_on_removal(struct kefir_hashtable *hashtable, kefir_hashtable_free_callback_t callback,
                                          void *payload) {
    REQUIRE(hashtable != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtable"));

    hashtable->cleanup.callback = callback;
    hashtable->cleanup.payload = payload;
    return KEFIR_OK;
}

static kefir_result_t find_position_for_insert(const struct kefir_hashtable_ops *ops,
                                               struct kefir_hashtable_entry *entries, kefir_size_t capacity,
                                               kefir_hashtable_key_t key, kefir_size_t *position_ptr,
                                               kefir_size_t *collisions_ptr) {
    KEFIR_HASHTABLE_FIND_POSITION_FOR_INSERT(ops, entries, capacity, key, position_ptr, collisions_ptr);
}

static kefir_result_t rehash(struct kefir_mem *, struct kefir_hashtable *);

static kefir_result_t insert_entry(struct kefir_mem *mem, struct kefir_hashtable *hashtable,
                                   struct kefir_hashtable_entry *entries, kefir_size_t capacity, kefir_size_t *occupied,
                                   kefir_hashtable_key_t key, kefir_hashtable_value_t value, kefir_bool_t do_rehash) {
    kefir_size_t index = 0;
    kefir_size_t found_collisions = 0;
    REQUIRE_OK(find_position_for_insert(hashtable->ops, entries, capacity, key, &index, &found_collisions));
    if (do_rehash && entries[index].state != KEFIR_HASHTABLE_ENTRY_OCCUPIED &&
        found_collisions >= KEFIR_REHASH_COLLISION_THRESHOLD) {
        REQUIRE_OK(rehash(mem, hashtable));
        return insert_entry(mem, hashtable, hashtable->entries, hashtable->capacity, &hashtable->occupied, key, value,
                            false);
    }
    REQUIRE(entries[index].state != KEFIR_HASHTABLE_ENTRY_OCCUPIED,
            KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Entry with identical key already exists in the hashtable"));

    entries[index].state = KEFIR_HASHTABLE_ENTRY_OCCUPIED;
    entries[index].key = key;
    entries[index].value = value;
    (*occupied)++;

    return KEFIR_OK;
}

static kefir_result_t rehash(struct kefir_mem *mem, struct kefir_hashtable *hashtable) {
    const kefir_size_t new_capacity = KEFIR_HASHTABLE_CAPACITY_GROW(hashtable->capacity);
    kefir_size_t new_occupied = 0;
    struct kefir_hashtable_entry *new_entries = KEFIR_MALLOC(mem, sizeof(struct kefir_hashtable_entry) * new_capacity);
    for (kefir_size_t i = 0; i < new_capacity; i++) {
        new_entries[i].state = KEFIR_HASHTABLE_ENTRY_EMPTY;
    }

    kefir_result_t res = KEFIR_OK;
    for (kefir_size_t i = 0; res == KEFIR_OK && i < hashtable->capacity; i++) {
        if (hashtable->entries[i].state == KEFIR_HASHTABLE_ENTRY_OCCUPIED) {
            res = insert_entry(mem, hashtable, new_entries, new_capacity, &new_occupied, hashtable->entries[i].key,
                               hashtable->entries[i].value, false);
        }
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, new_entries);
        return res;
    });

    KEFIR_FREE(mem, hashtable->entries);
    hashtable->entries = new_entries;
    hashtable->capacity = new_capacity;
    hashtable->occupied = new_occupied;
    return KEFIR_OK;
}

kefir_result_t kefir_hashtable_insert(struct kefir_mem *mem, struct kefir_hashtable *hashtable,
                                      kefir_hashtable_key_t key, kefir_hashtable_value_t value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(hashtable != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtable"));

    kefir_bool_t did_rehash = false;
    if (hashtable->capacity == 0 || hashtable->occupied >= KEFIR_REHASH_OCCUPATION_THRESHOLD * hashtable->capacity) {
        REQUIRE_OK(rehash(mem, hashtable));
        did_rehash = true;
    }

    REQUIRE_OK(insert_entry(mem, hashtable, hashtable->entries, hashtable->capacity, &hashtable->occupied, key, value,
                            !did_rehash));
    return KEFIR_OK;
}

kefir_result_t kefir_hashtable_delete(struct kefir_mem *mem, struct kefir_hashtable *hashtable,
                                      kefir_hashtable_key_t key) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(hashtable != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtable"));

    KEFIR_HASHTABLE_HAS(
        hashtable, key,
        {
            if (hashtable->cleanup.callback != NULL) {
                REQUIRE_OK(hashtable->cleanup.callback(mem, hashtable, hashtable->entries[index].key,
                                                       hashtable->entries[index].value, hashtable->cleanup.payload));
            }
            hashtable->entries[index].state = KEFIR_HASHTABLE_ENTRY_DELETED;
            hashtable->occupied--;
            return KEFIR_OK;
        },
        return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Entry with provided key does not exist in the hashtable"););
}

kefir_result_t kefir_hashtable_at_mut(const struct kefir_hashtable *hashtable, kefir_hashtable_key_t key,
                                      kefir_hashtable_value_t **value_ptr) {
    REQUIRE(hashtable != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtable"));

    KEFIR_HASHTABLE_HAS(
        hashtable, key,
        {
            ASSIGN_PTR(value_ptr, &hashtable->entries[index].value);
            return KEFIR_OK;
        },
        return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested key in the hashtable"););
}

kefir_result_t kefir_hashtable_at(const struct kefir_hashtable *hashtable, kefir_hashtable_key_t key,
                                  kefir_hashtable_value_t *value_ptr) {
    kefir_hashtable_value_t *ptr;
    REQUIRE_OK(kefir_hashtable_at_mut(hashtable, key, &ptr));
    ASSIGN_PTR(value_ptr, *ptr);
    return KEFIR_OK;
}

kefir_bool_t kefir_hashtable_has(const struct kefir_hashtable *hashtable, kefir_hashtable_key_t key) {
    REQUIRE(hashtable != NULL, false);

    KEFIR_HASHTABLE_HAS(hashtable, key, return true;, return false;);
}

kefir_result_t kefir_hashtable_iter(const struct kefir_hashtable *hashtable, struct kefir_hashtable_iterator *iter,
                                    kefir_hashtable_key_t *key_ptr, kefir_hashtable_value_t *value_ptr) {
    REQUIRE(hashtable != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtable"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to hashtable iterator"));

    iter->hashtable = hashtable;
    iter->index = 0;

    for (; iter->index < hashtable->capacity; iter->index++) {
        if (hashtable->entries[iter->index].state == KEFIR_HASHTABLE_ENTRY_OCCUPIED) {
            ASSIGN_PTR(key_ptr, hashtable->entries[iter->index].key);
            ASSIGN_PTR(value_ptr, hashtable->entries[iter->index].value);
            return KEFIR_OK;
        }
    }

    return KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of hashtable iterator");
}

kefir_result_t kefir_hashtable_next(struct kefir_hashtable_iterator *iter, kefir_hashtable_key_t *key_ptr,
                                    kefir_hashtable_value_t *value_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtable iterator"));

    iter->index++;
    for (; iter->index < iter->hashtable->capacity; iter->index++) {
        if (iter->hashtable->entries[iter->index].state == KEFIR_HASHTABLE_ENTRY_OCCUPIED) {
            ASSIGN_PTR(key_ptr, iter->hashtable->entries[iter->index].key);
            ASSIGN_PTR(value_ptr, iter->hashtable->entries[iter->index].value);
            return KEFIR_OK;
        }
    }

    return KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of hashtable iterator");
}

static kefir_hashtable_hash_t uint_hash(kefir_hashtable_key_t key, void *payload) {
    UNUSED(payload);

    return kefir_splitmix64((kefir_uint64_t) key);
}

static kefir_bool_t uint_equal(kefir_hashtable_key_t key1, kefir_hashtable_key_t key2, void *payload) {
    UNUSED(payload);

    return key1 == key2;
}

const struct kefir_hashtable_ops kefir_hashtable_uint_ops = {.hash = uint_hash, .equal = uint_equal, .payload = NULL};
