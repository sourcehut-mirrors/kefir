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
#include "kefir/core/hashset.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_hashset_init(struct kefir_hashset *hashset, const struct kefir_hashtable_ops *ops) {
    REQUIRE(hashset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to hashset"));
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashset operations"));

    hashset->entries = NULL;
    hashset->entry_states = NULL;
    hashset->capacity = 0;
    hashset->occupied = 0;
    hashset->ops = ops;
    return KEFIR_OK;
}

kefir_result_t kefir_hashset_free(struct kefir_mem *mem, struct kefir_hashset *hashset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(hashset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashset"));

    KEFIR_FREE(mem, hashset->entries);
    KEFIR_FREE(mem, hashset->entry_states);
    memset(hashset, 0, sizeof(struct kefir_hashset));
    return KEFIR_OK;
}

static kefir_result_t find_position_for_insert(const struct kefir_hashtable_ops *ops,
                                               struct kefir_hashset_entry *entries, kefir_uint8_t *entry_states,
                                               kefir_size_t capacity, kefir_hashset_key_t key,
                                               kefir_size_t *position_ptr, kefir_size_t *collisions_ptr) {
    if (ops == &kefir_hashtable_uint_ops) {
        KEFIR_HASHTABLE_FIND_POSITION_FOR_INSERT(kefir_hashtable_uint_hash, kefir_hashtable_uint_equal, NULL, entries,
                                                 entry_states, capacity, key, position_ptr, collisions_ptr);
    } else {
        KEFIR_HASHTABLE_FIND_POSITION_FOR_INSERT(ops->hash, ops->equal, ops->payload, entries, entry_states, capacity,
                                                 key, position_ptr, collisions_ptr);
    }
}

static kefir_result_t rehash(struct kefir_mem *, struct kefir_hashset *, kefir_size_t);

static kefir_result_t insert_entry(struct kefir_mem *mem, struct kefir_hashset *hashset,
                                   struct kefir_hashset_entry *entries, kefir_uint8_t *entry_states,
                                   kefir_size_t capacity, kefir_size_t *occupied, kefir_hashset_key_t key,
                                   kefir_bool_t do_rehash) {
    kefir_size_t index = 0;
    kefir_size_t found_collisions = 0;
    REQUIRE_OK(find_position_for_insert(hashset->ops, entries, entry_states, capacity, key, &index, &found_collisions));
    if (do_rehash && entry_states[index] != KEFIR_HASHTABLE_ENTRY_OCCUPIED &&
        found_collisions >= KEFIR_REHASH_COLLISION_THRESHOLD) {
        REQUIRE_OK(rehash(mem, hashset, KEFIR_HASHTABLE_CAPACITY_GROW(hashset->capacity)));
        return insert_entry(mem, hashset, hashset->entries, hashset->entry_states, hashset->capacity,
                            &hashset->occupied, key, false);
    }

    if (entry_states[index] != KEFIR_HASHTABLE_ENTRY_OCCUPIED) {
        entry_states[index] = KEFIR_HASHTABLE_ENTRY_OCCUPIED;
        entries[index].key = key;
        (*occupied)++;
    }
    return KEFIR_OK;
}

static kefir_result_t rehash(struct kefir_mem *mem, struct kefir_hashset *hashset, kefir_size_t new_capacity) {
    kefir_size_t new_occupied = 0;
    kefir_uint8_t *new_entry_states = KEFIR_MALLOC(mem, sizeof(kefir_uint8_t) * new_capacity);
    REQUIRE(new_entry_states != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate hashset entry states"));
    struct kefir_hashset_entry *new_entries = KEFIR_MALLOC(mem, sizeof(struct kefir_hashset_entry) * new_capacity);
    REQUIRE_ELSE(new_entries != NULL, {
        KEFIR_FREE(mem, new_entry_states);
        return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate hashset entries");
    });
    memset(new_entry_states, 0, sizeof(kefir_uint8_t) * new_capacity);

    kefir_result_t res = KEFIR_OK;
    for (kefir_size_t i = 0; res == KEFIR_OK && i < hashset->capacity; i++) {
        if (hashset->entry_states[i] == KEFIR_HASHTABLE_ENTRY_OCCUPIED) {
            res = insert_entry(mem, hashset, new_entries, new_entry_states, new_capacity, &new_occupied,
                               hashset->entries[i].key, false);
        }
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, new_entries);
        KEFIR_FREE(mem, new_entry_states);
        return res;
    });

    KEFIR_FREE(mem, hashset->entries);
    KEFIR_FREE(mem, hashset->entry_states);
    hashset->entries = new_entries;
    hashset->entry_states = new_entry_states;
    hashset->capacity = new_capacity;
    hashset->occupied = new_occupied;
    return KEFIR_OK;
}

kefir_size_t kefir_hashset_size(const struct kefir_hashset *hashset) {
    REQUIRE(hashset != NULL, 0);
    return hashset->occupied;
}

kefir_result_t kefir_hashset_clear(struct kefir_mem *mem, struct kefir_hashset *hashset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(hashset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashset"));

    if (hashset->capacity > 0) {
        memset(hashset->entry_states, 0, sizeof(kefir_uint8_t) * hashset->capacity);
    }
    hashset->occupied = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_hashset_add(struct kefir_mem *mem, struct kefir_hashset *hashset, kefir_hashset_key_t key) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(hashset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashset"));

    kefir_bool_t did_rehash = false;
    if (hashset->capacity == 0 ||
        hashset->occupied >= KEFIR_REHASH_OCCUPATION_THRESHOLD_PCT * hashset->capacity / 100) {
        REQUIRE_OK(rehash(mem, hashset, KEFIR_HASHTABLE_CAPACITY_GROW(hashset->capacity)));
        did_rehash = true;
    }

    REQUIRE_OK(insert_entry(mem, hashset, hashset->entries, hashset->entry_states, hashset->capacity,
                            &hashset->occupied, key, !did_rehash));
    return KEFIR_OK;
}

static kefir_uint64_t next_power_of_2(kefir_uint64_t x) {
    REQUIRE(x != 0, 1);

    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;

    return x + 1;
}

kefir_result_t kefir_hashset_trim(struct kefir_mem *mem, struct kefir_hashset *hashset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(hashset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashset"));

    if (hashset->capacity * KEFIR_REHASH_TRIM_THRESHOLD_PCT / 100 > hashset->occupied) {
        kefir_size_t trimmed_capacity = next_power_of_2(KEFIR_HASHTABLE_CAPACITY_TRIM(hashset->occupied));
        if (trimmed_capacity < hashset->capacity) {
            REQUIRE_OK(rehash(mem, hashset, trimmed_capacity));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_hashset_ensure(struct kefir_mem *mem, struct kefir_hashset *hashset, kefir_size_t min_capacity) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(hashset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashset"));

    min_capacity = next_power_of_2(min_capacity);
    if (hashset->capacity < min_capacity) {
        REQUIRE_OK(rehash(mem, hashset, min_capacity));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_hashset_delete(struct kefir_hashset *hashset, kefir_hashset_key_t key) {
    REQUIRE(hashset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashset"));
#define DELETE(_hash, _equal, _payload)                                   \
    KEFIR_HASHTABLE_HAS(                                                  \
        hashset, _hash, _equal, _payload, key,                            \
        {                                                                 \
            hashset->entry_states[index] = KEFIR_HASHTABLE_ENTRY_DELETED; \
            hashset->occupied--;                                          \
            return KEFIR_OK;                                              \
        },                                                                \
        return KEFIR_OK;)

    if (hashset->ops == &kefir_hashtable_uint_ops) {
        DELETE(kefir_hashtable_uint_hash, kefir_hashtable_uint_equal, NULL);
    } else {
        DELETE(hashset->ops->hash, hashset->ops->equal, hashset->ops->payload);
    }
#undef DELETE
}

kefir_result_t kefir_hashset_merge(struct kefir_mem *mem, struct kefir_hashset *dst_hashset,
                                   const struct kefir_hashset *src_hashset) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(dst_hashset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination hashset"));
    REQUIRE(src_hashset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source hashset"));

    if (src_hashset->occupied > 0) {
        for (kefir_size_t i = 0; i < src_hashset->capacity; i++) {
            if (src_hashset->entry_states[i] == KEFIR_HASHTABLE_ENTRY_OCCUPIED) {
                REQUIRE_OK(kefir_hashset_add(mem, dst_hashset, src_hashset->entries[i].key));
            }
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_hashset_subtract(struct kefir_hashset *dst_hashset, const struct kefir_hashset *src_hashset) {
    REQUIRE(dst_hashset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination hashset"));
    REQUIRE(src_hashset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source hashset"));

    if (src_hashset->occupied > 0) {
        for (kefir_size_t i = 0; i < src_hashset->capacity; i++) {
            if (src_hashset->entry_states[i] == KEFIR_HASHTABLE_ENTRY_OCCUPIED) {
                REQUIRE_OK(kefir_hashset_delete(dst_hashset, src_hashset->entries[i].key));
            }
        }
    }
    return KEFIR_OK;
}

kefir_bool_t kefir_hashset_has(const struct kefir_hashset *hashset, kefir_hashset_key_t key) {
    REQUIRE(hashset != NULL, false);

#define HAS(_hash, _equal, _payload)                                                                               \
    KEFIR_HASHTABLE_HAS(hashset, hashset->ops->hash, hashset->ops->equal, hashset->ops->payload, key, return true; \
                        , return false;)

    if (hashset->ops == &kefir_hashtable_uint_ops) {
        HAS(kefir_hashtable_uint_hash, kefir_hashtable_uint_equal, NULL);
    } else {
        HAS(hashset->ops->hash, hashset->ops->equal, hashset->ops->payload);
    }
#undef HAS
}

kefir_bool_t kefir_hashset_has_difference(const struct kefir_hashset *hashset, const struct kefir_hashset *hashset2) {
    REQUIRE(hashset != NULL, hashset2 != NULL);
    REQUIRE(hashset2 != NULL, hashset != NULL);

    if (hashset->occupied > 0) {
        for (kefir_size_t i = 0; i < hashset->capacity; i++) {
            if (hashset->entry_states[i] == KEFIR_HASHTABLE_ENTRY_OCCUPIED) {
                if (!kefir_hashset_has(hashset2, hashset->entries[i].key)) {
                    return true;
                }
            }
        }
    }

    if (hashset2->occupied > 0) {
        for (kefir_size_t i = 0; i < hashset2->capacity; i++) {
            if (hashset2->entry_states[i] == KEFIR_HASHTABLE_ENTRY_OCCUPIED) {
                if (!kefir_hashset_has(hashset, hashset2->entries[i].key)) {
                    return true;
                }
            }
        }
    }

    return false;
}

kefir_result_t kefir_hashset_iter(const struct kefir_hashset *hashset, struct kefir_hashset_iterator *iter,
                                  kefir_hashset_key_t *key_ptr) {
    REQUIRE(hashset != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashset"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to hashset iterator"));

    iter->hashset = hashset;
    iter->index = 0;

    for (; iter->index < hashset->capacity; iter->index++) {
        if (hashset->entry_states[iter->index] == KEFIR_HASHTABLE_ENTRY_OCCUPIED) {
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
        if (iter->hashset->entry_states[iter->index] == KEFIR_HASHTABLE_ENTRY_OCCUPIED) {
            ASSIGN_PTR(key_ptr, iter->hashset->entries[iter->index].key);
            return KEFIR_OK;
        }
    }

    return KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of hashset iterator");
}
