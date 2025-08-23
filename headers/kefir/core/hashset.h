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

#ifndef KEFIR_CORE_HASHSET_H_
#define KEFIR_CORE_HASHSET_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/mem.h"
#include "kefir/core/hashtable.h"

typedef kefir_hashtable_key_t kefir_hashset_key_t;
typedef kefir_hashtable_hash_t kefir_hashset_hash_t;
typedef struct kefir_hashset kefir_hashset_t;
typedef struct kefir_hashset_entry {
    kefir_hashtable_entry_state_t state;
    kefir_hashset_key_t key;
} kefir_hashset_entry_t;

typedef struct kefir_hashset {
    struct kefir_hashset_entry *entries;
    kefir_size_t capacity;
    kefir_size_t occupied;

    const struct kefir_hashtable_ops *ops;
} kefir_hashset_t;

kefir_result_t kefir_hashset_init(struct kefir_hashset *, const struct kefir_hashtable_ops *);
kefir_result_t kefir_hashset_free(struct kefir_mem *, struct kefir_hashset *);

kefir_result_t kefir_hashset_clear(struct kefir_mem *, struct kefir_hashset *);
kefir_result_t kefir_hashset_add(struct kefir_mem *, struct kefir_hashset *, kefir_hashset_key_t);
kefir_result_t kefir_hashset_merge(struct kefir_mem *, struct kefir_hashset *, const struct kefir_hashset *);
kefir_bool_t kefir_hashset_has(const struct kefir_hashset *, kefir_hashset_key_t);

typedef struct kefir_hashset_iterator {
    const struct kefir_hashset *hashset;
    kefir_size_t index;
} kefir_hashset_iterator_t;

kefir_result_t kefir_hashset_iter(const struct kefir_hashset *, struct kefir_hashset_iterator *, kefir_hashset_key_t *);
kefir_result_t kefir_hashset_next(struct kefir_hashset_iterator *, kefir_hashset_key_t *);

#endif
