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

#ifndef KEFIR_CORE_HASHTABLE_H_
#define KEFIR_CORE_HASHTABLE_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/mem.h"
#include <string.h>

typedef kefir_uptr_t kefir_hashtable_key_t;
typedef kefir_uptr_t kefir_hashtable_value_t;
typedef kefir_uint64_t kefir_hashtable_hash_t;
typedef struct kefir_hashtable kefir_hashtable_t;

typedef kefir_result_t (*kefir_hashtable_free_callback_t)(struct kefir_mem *, struct kefir_hashtable *,
                                                          kefir_hashtable_key_t, kefir_hashtable_value_t, void *);
typedef kefir_hashtable_hash_t (*kefir_hashtable_hash_fn_t)(kefir_hashtable_key_t, void *);
typedef kefir_bool_t (*kefir_hashtable_equal_fn_t)(kefir_hashtable_key_t, kefir_hashtable_key_t, void *);

typedef enum kefir_hashtable_entry_state {
    KEFIR_HASHTABLE_ENTRY_EMPTY,
    KEFIR_HASHTABLE_ENTRY_OCCUPIED,
    KEFIR_HASHTABLE_ENTRY_DELETED
} kefir_hashtable_entry_state_t;

typedef struct kefir_hashtable_ops {
    kefir_hashtable_hash_fn_t hash;
    kefir_hashtable_equal_fn_t equal;
    void *payload;
} kefir_hashtable_ops_t;

typedef struct kefir_hashtable_cleanup {
    kefir_hashtable_free_callback_t callback;
    void *payload;
} kefir_hashtable_cleanup_t;

typedef struct kefir_hashtable_entry {
    kefir_hashtable_key_t key;
    kefir_hashtable_value_t value;
} kefir_hashtable_entry_t;

typedef struct kefir_hashtable {
    kefir_uint8_t *entry_states;
    struct kefir_hashtable_entry *entries;
    kefir_size_t capacity;
    kefir_size_t occupied;

    const struct kefir_hashtable_ops *ops;
    struct kefir_hashtable_cleanup cleanup;
} kefir_hashtable_t;

kefir_result_t kefir_hashtable_init(struct kefir_hashtable *, const struct kefir_hashtable_ops *);
kefir_result_t kefir_hashtable_free(struct kefir_mem *, struct kefir_hashtable *);

kefir_result_t kefir_hashtable_on_removal(struct kefir_hashtable *, kefir_hashtable_free_callback_t, void *);

kefir_result_t kefir_hashtable_insert(struct kefir_mem *, struct kefir_hashtable *, kefir_hashtable_key_t,
                                      kefir_hashtable_value_t);
kefir_result_t kefir_hashtable_delete(struct kefir_mem *, struct kefir_hashtable *, kefir_hashtable_key_t);
kefir_result_t kefir_hashtable_clear(struct kefir_mem *, struct kefir_hashtable *);
kefir_result_t kefir_hashtable_trim(struct kefir_mem *, struct kefir_hashtable *);
kefir_result_t kefir_hashtable_ensure(struct kefir_mem *, struct kefir_hashtable *, kefir_size_t);
kefir_result_t kefir_hashtable_at_mut(const struct kefir_hashtable *, kefir_hashtable_key_t,
                                      kefir_hashtable_value_t **);
kefir_result_t kefir_hashtable_at(const struct kefir_hashtable *, kefir_hashtable_key_t, kefir_hashtable_value_t *);
kefir_result_t kefir_hashtable_at2(const struct kefir_hashtable *, kefir_hashtable_key_t, kefir_hashtable_key_t *,
                                   kefir_hashtable_value_t *);
kefir_bool_t kefir_hashtable_has(const struct kefir_hashtable *, kefir_hashtable_key_t);

typedef struct kefir_hashtable_iterator {
    const struct kefir_hashtable *hashtable;
    kefir_size_t index;
} kefir_hashtable_iterator_t;

kefir_result_t kefir_hashtable_iter(const struct kefir_hashtable *, struct kefir_hashtable_iterator *,
                                    kefir_hashtable_key_t *, kefir_hashtable_value_t *);
kefir_result_t kefir_hashtable_next(struct kefir_hashtable_iterator *, kefir_hashtable_key_t *,
                                    kefir_hashtable_value_t *);

extern const struct kefir_hashtable_ops kefir_hashtable_uint_ops;
extern const struct kefir_hashtable_ops kefir_hashtable_str_ops;

#ifdef KEFIR_HASHTABLE_INTERNAL
#include "kefir/core/hash.h"
#include "kefir/core/util.h"

static kefir_hashtable_hash_t kefir_hashtable_uint_hash(kefir_hashtable_key_t key, void *payload) {
    UNUSED(payload);

    return kefir_splitmix64((kefir_uint64_t) key);
}

static kefir_bool_t kefir_hashtable_uint_equal(kefir_hashtable_key_t key1, kefir_hashtable_key_t key2, void *payload) {
    UNUSED(payload);

    return key1 == key2;
}

static kefir_hashtable_hash_t kefir_hashtable_str_hash(kefir_hashtable_key_t key, void *payload) {
    UNUSED(payload);

    const char *str = (const char *) key;
    REQUIRE(str != NULL, 0);
    kefir_hashtable_hash_t hash = 7;
    const kefir_size_t length = strlen(str);
    for (kefir_size_t i = 0; i < length; i++) {
        hash = (hash * 31) + str[i];
    }
    return hash;
}

static kefir_bool_t kefir_hashtable_str_equal(kefir_hashtable_key_t key1, kefir_hashtable_key_t key2, void *payload) {
    UNUSED(payload);
    const char *str1 = (const char *) key1;
    const char *str2 = (const char *) key2;
    if (str1 == str2) {
        return 1;
    } else if (str1 == NULL || str2 == NULL) {
        return 0;
    }

    return strcmp(str1, str2) == 0;
}

#define KEFIR_REHASH_OCCUPATION_THRESHOLD_PCT 80
#define KEFIR_REHASH_COLLISION_THRESHOLD 32
#define KEFIR_REHASH_TRIM_THRESHOLD_PCT 15

#define KEFIR_HASHTABLE_CAPACITY_GROW(_capacity) ((_capacity) == 0 ? 4 : (_capacity) * 2)
#define KEFIR_HASHTABLE_CAPACITY_TRIM(_occupied) MAX(4, (kefir_size_t) ((_occupied) * 4))
#define KEFIR_HASHTABLE_WRAPAROUND(_index, _capacity) ((_index) & ((_capacity) - 1))

#define KEFIR_HASHTABLE_FIND_POSITION_FOR_INSERT(_ops_hash, _ops_equal, _ops_payload, _entries, _entry_states,       \
                                                 _capacity, _key, _position_ptr, _collisions_ptr)                    \
    do {                                                                                                             \
        const kefir_uint64_t hash = (_ops_hash) ((_key), (_ops_payload));                                            \
        const kefir_uint32_t upper_part = hash >> 32;                                                                \
        const kefir_uint32_t lower_part = (hash & ((1ull << 32) - 1)) | 1;                                           \
        const kefir_size_t index = KEFIR_HASHTABLE_WRAPAROUND(upper_part, (_capacity));                              \
        const kefir_uint8_t state = (_entry_states)[index];                                                          \
        if (state == KEFIR_HASHTABLE_ENTRY_EMPTY) {                                                                  \
            *(_position_ptr) = index;                                                                                \
            return KEFIR_OK;                                                                                         \
        }                                                                                                            \
                                                                                                                     \
        kefir_bool_t equal;                                                                                          \
        if (state == KEFIR_HASHTABLE_ENTRY_OCCUPIED) {                                                               \
            equal = (_key) == (_entries)[index].key || (_ops_equal) ((_key), (_entries)[index].key, (_ops_payload)); \
            if (equal) {                                                                                             \
                *(_position_ptr) = index;                                                                            \
                return KEFIR_OK;                                                                                     \
            }                                                                                                        \
        }                                                                                                            \
                                                                                                                     \
        kefir_size_t deleted_index = ~0ull;                                                                          \
        for (kefir_size_t i = 0; i < (_capacity); i++) {                                                             \
            kefir_size_t current_index = KEFIR_HASHTABLE_WRAPAROUND(index + i * lower_part, (_capacity));            \
            (*(_collisions_ptr))++;                                                                                  \
            const kefir_uint8_t current_state = (_entry_states)[current_index];                                      \
            if (current_state == KEFIR_HASHTABLE_ENTRY_EMPTY) {                                                      \
                if (deleted_index == ~0ull) {                                                                        \
                    *(_position_ptr) = current_index;                                                                \
                    return KEFIR_OK;                                                                                 \
                } else {                                                                                             \
                    *(_position_ptr) = deleted_index;                                                                \
                    return KEFIR_OK;                                                                                 \
                }                                                                                                    \
            } else if (current_state == KEFIR_HASHTABLE_ENTRY_DELETED) {                                             \
                if (deleted_index == ~0ull) {                                                                        \
                    deleted_index = current_index;                                                                   \
                }                                                                                                    \
            } else if (current_state == KEFIR_HASHTABLE_ENTRY_OCCUPIED) {                                            \
                equal = (_key) == (_entries)[current_index].key ||                                                   \
                        (_ops_equal) ((_key), (_entries)[current_index].key, (_ops_payload));                        \
                if (equal) {                                                                                         \
                    *(_position_ptr) = current_index;                                                                \
                    return KEFIR_OK;                                                                                 \
                }                                                                                                    \
            }                                                                                                        \
        }                                                                                                            \
        if (deleted_index != ~0ull) {                                                                                \
            *(_position_ptr) = deleted_index;                                                                        \
            return KEFIR_OK;                                                                                         \
        }                                                                                                            \
                                                                                                                     \
        return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unable to find position for element insertion");               \
    } while (0)

#define KEFIR_HASHTABLE_HAS(_hashtable, _ops_hash, _ops_equal, _ops_payload, _key, on_found, _on_not_found)   \
    do {                                                                                                      \
        if ((_hashtable)->occupied > 0) {                                                                     \
            const kefir_uint64_t hash = (_ops_hash) (key, (_ops_payload));                                    \
            const kefir_uint32_t upper_part = hash >> 32;                                                     \
            const kefir_uint32_t lower_part = (hash & ((1ull << 32) - 1)) | 1;                                \
            const kefir_size_t capacity = (_hashtable)->capacity;                                             \
            const kefir_size_t base_index = KEFIR_HASHTABLE_WRAPAROUND(upper_part, capacity);                 \
            for (kefir_size_t i = 0; i < capacity; i++) {                                                     \
                const kefir_size_t index = KEFIR_HASHTABLE_WRAPAROUND(base_index + i * lower_part, capacity); \
                const kefir_uint8_t state = (_hashtable)->entry_states[index];                                \
                if (state == KEFIR_HASHTABLE_ENTRY_EMPTY) {                                                   \
                    break;                                                                                    \
                }                                                                                             \
                                                                                                              \
                if (state == KEFIR_HASHTABLE_ENTRY_OCCUPIED &&                                                \
                    ((_key) == (_hashtable)->entries[index].key ||                                            \
                     (_ops_equal) ((_key), (_hashtable)->entries[index].key, (_ops_payload)))) {              \
                    on_found                                                                                  \
                }                                                                                             \
            }                                                                                                 \
        }                                                                                                     \
                                                                                                              \
        _on_not_found                                                                                         \
    } while (0)

#endif

#endif
