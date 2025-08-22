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

typedef kefir_result_t (*kefir_hashtable_free_callback_t)(struct kefir_mem *, struct kefir_hashtable *, kefir_hashtable_key_t, kefir_hashtable_value_t, void *);
typedef kefir_hashtable_hash_t (*kefir_hashtable_hash_fn_t)(kefir_hashtable_key_t, void *);
typedef kefir_bool_t (*kefir_hashtable_equal_fn_t)(kefir_hashtable_key_t, kefir_hashtable_key_t, void *);

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
    kefir_bool_t occupied;
    kefir_hashtable_key_t key;
    kefir_hashtable_value_t value;
} kefir_hashtable_entry_t;

typedef struct kefir_hashtable {
    struct kefir_hashtable_entry *entries;
    kefir_size_t capacity;
    kefir_size_t occupied;
    kefir_size_t collisions;

    const struct kefir_hashtable_ops *ops;
    struct kefir_hashtable_cleanup cleanup;
} kefir_hashtable_t;

kefir_result_t kefir_hashtable_init(struct kefir_hashtable *, const struct kefir_hashtable_ops *);
kefir_result_t kefir_hashtable_free(struct kefir_mem *, struct kefir_hashtable *);

kefir_result_t kefir_hashtable_on_removal(struct kefir_hashtable *, kefir_hashtable_free_callback_t, void *);

kefir_result_t kefir_hashtable_insert(struct kefir_mem *, struct kefir_hashtable *, kefir_hashtable_key_t, kefir_hashtable_value_t);
kefir_result_t kefir_hashtable_at_mut(const struct kefir_hashtable *, kefir_hashtable_key_t, kefir_hashtable_value_t **);
kefir_result_t kefir_hashtable_at(const struct kefir_hashtable *, kefir_hashtable_key_t, kefir_hashtable_value_t *);
kefir_bool_t kefir_hashtable_has(const struct kefir_hashtable *, kefir_hashtable_key_t);

typedef struct kefir_hashtable_iterator {
    const struct kefir_hashtable *hashtable;
    kefir_size_t index;
} kefir_hashtable_iterator_t;

kefir_result_t kefir_hashtable_iter(const struct kefir_hashtable *, struct kefir_hashtable_iterator *, kefir_hashtable_key_t *, kefir_hashtable_value_t *);
kefir_result_t kefir_hashtable_next(struct kefir_hashtable_iterator *, kefir_hashtable_key_t *, kefir_hashtable_value_t *);

extern const struct kefir_hashtable_ops kefir_hashtable_uint_ops;

#endif
