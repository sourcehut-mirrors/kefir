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

#ifndef KEFIR_CORE_HASHTABLE_H_
#define KEFIR_CORE_HASHTABLE_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/mem.h"

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
    kefir_hashtable_entry_state_t state;
    kefir_hashtable_key_t key;
    kefir_hashtable_value_t value;
} kefir_hashtable_entry_t;

typedef struct kefir_hashtable {
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
kefir_result_t kefir_hashtable_at_mut(const struct kefir_hashtable *, kefir_hashtable_key_t,
                                      kefir_hashtable_value_t **);
kefir_result_t kefir_hashtable_at(const struct kefir_hashtable *, kefir_hashtable_key_t, kefir_hashtable_value_t *);
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

#ifdef KEFIR_HASHTABLE_INTERNAL

#define KEFIR_REHASH_OCCUPATION_THRESHOLD 0.6
#define KEFIR_REHASH_COLLISION_THRESHOLD 8

#define KEFIR_HASHTABLE_CAPACITY_GROW(_capacity) ((_capacity) == 0 ? 4 : (_capacity) * 2)
#define KEFIR_HASHTABLE_WRAPAROUND(_index, _capacity) ((_index) & ((_capacity) - 1))

#define KEFIR_HASHTABLE_FIND_POSITION_FOR_INSERT(_ops, _entries, _capacity, _key, _position_ptr, _collisions_ptr) \
    do {                                                                                                          \
        kefir_size_t index = KEFIR_HASHTABLE_WRAPAROUND((_ops)->hash((_key), (_ops)->payload), (_capacity));      \
        if ((_entries)[index].state != KEFIR_HASHTABLE_ENTRY_OCCUPIED) {                                          \
            *(_position_ptr) = index;                                                                             \
            return KEFIR_OK;                                                                                      \
        }                                                                                                         \
                                                                                                                  \
        kefir_bool_t equal = (_ops)->equal((_key), (_entries)[index].key, (_ops)->payload);                       \
        if (equal) {                                                                                              \
            *(_position_ptr) = index;                                                                             \
            return KEFIR_OK;                                                                                      \
        }                                                                                                         \
                                                                                                                  \
        for (kefir_size_t i = KEFIR_HASHTABLE_WRAPAROUND(index + 1, (_capacity)); i != index;                     \
             i = KEFIR_HASHTABLE_WRAPAROUND(i + 1, (_capacity))) {                                                \
            (*(_collisions_ptr))++;                                                                               \
            if ((_entries)[i].state != KEFIR_HASHTABLE_ENTRY_OCCUPIED) {                                          \
                *(_position_ptr) = i;                                                                             \
                return KEFIR_OK;                                                                                  \
            }                                                                                                     \
                                                                                                                  \
            equal = (_ops)->equal((_key), (_entries)[i].key, (_ops)->payload);                                    \
            if (equal) {                                                                                          \
                *(_position_ptr) = i;                                                                             \
                return KEFIR_OK;                                                                                  \
            }                                                                                                     \
        }                                                                                                         \
                                                                                                                  \
        return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unable to find position for element insertion");            \
    } while (0)

#define KEFIR_HASHTABLE_HAS(_hashtable, _key, on_found, _on_not_found)                                                \
    do {                                                                                                              \
        if ((_hashtable)->occupied > 0) {                                                                             \
            const kefir_size_t base_index = KEFIR_HASHTABLE_WRAPAROUND(                                               \
                (_hashtable)->ops->hash(key, (_hashtable)->ops->payload), (_hashtable)->capacity);                    \
            kefir_size_t index = base_index;                                                                          \
            do {                                                                                                      \
                if ((_hashtable)->entries[index].state == KEFIR_HASHTABLE_ENTRY_EMPTY) {                              \
                    break;                                                                                            \
                }                                                                                                     \
                                                                                                                      \
                if ((_hashtable)->entries[index].state == KEFIR_HASHTABLE_ENTRY_OCCUPIED &&                           \
                    (_hashtable)->ops->equal((_key), (_hashtable)->entries[index].key, (_hashtable)->ops->payload)) { \
                    on_found                                                                                          \
                }                                                                                                     \
                                                                                                                      \
                index = KEFIR_HASHTABLE_WRAPAROUND(index + 1, (_hashtable)->capacity);                                \
            } while (index != base_index);                                                                            \
        }                                                                                                             \
                                                                                                                      \
        _on_not_found                                                                                                 \
    } while (0)

#endif

#endif
