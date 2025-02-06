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

#ifndef KEFIR_CORE_BUCKETSET_H_
#define KEFIR_CORE_BUCKETSET_H_

#include "kefir/core/hashtree.h"

typedef kefir_hashtree_hash_t kefir_bucketset_hash_t;
typedef kefir_hashtree_key_t kefir_bucketset_entry_t;

typedef struct kefir_bucketset_ops {
    kefir_bucketset_hash_t (*hash)(kefir_bucketset_entry_t, void *);
    kefir_bool_t (*contains)(const kefir_bucketset_entry_t *, kefir_size_t, kefir_bucketset_entry_t, void *);
    kefir_result_t (*find)(const kefir_bucketset_entry_t *, kefir_size_t, kefir_bucketset_entry_t, kefir_size_t *,
                           void *);
    kefir_result_t (*insert_position)(const kefir_bucketset_entry_t *, kefir_size_t, kefir_bucketset_entry_t,
                                      kefir_size_t *, void *);
    void *payload;
} kefir_bucketset_ops_t;

typedef struct kefir_bucketset_bucket {
    kefir_size_t length;
    kefir_size_t capacity;
    kefir_bucketset_entry_t entries[];
} kefir_bucketset_bucket_t;

typedef struct kefir_bucketset {
    kefir_size_t num_of_buckets;
    kefir_size_t max_bucket_length;
    struct kefir_hashtree buckets;
    const struct kefir_bucketset_ops *ops;
} kefir_bucketset_t;

kefir_result_t kefir_bucketset_init(struct kefir_bucketset *, const struct kefir_bucketset_ops *);
kefir_result_t kefir_bucketset_free(struct kefir_mem *, struct kefir_bucketset *);

kefir_result_t kefir_bucketset_add(struct kefir_mem *, struct kefir_bucketset *, kefir_bucketset_entry_t);
kefir_bool_t kefir_bucketset_has(const struct kefir_bucketset *, kefir_bucketset_entry_t);
kefir_result_t kefir_bucketset_delete(struct kefir_mem *, struct kefir_bucketset *, kefir_bucketset_entry_t);
kefir_result_t kefir_bucketset_clean(struct kefir_mem *, struct kefir_bucketset *);
kefir_result_t kefir_bucketset_clean_nofree(struct kefir_mem *, struct kefir_bucketset *);

kefir_result_t kefir_bucketset_merge(struct kefir_mem *, struct kefir_bucketset *, const struct kefir_bucketset *);
kefir_result_t kefir_bucketset_intersect(struct kefir_mem *, struct kefir_bucketset *, const struct kefir_bucketset *);

typedef struct kefir_bucketset_iterator {
    const struct kefir_bucketset *bucketset;
    struct kefir_hashtree_node *node;
    kefir_size_t index;
} kefir_bucketset_iterator_t;

kefir_result_t kefir_bucketset_iter(const struct kefir_bucketset *, struct kefir_bucketset_iterator *,
                                    kefir_bucketset_entry_t *);
kefir_result_t kefir_bucketset_next(struct kefir_bucketset_iterator *, kefir_bucketset_entry_t *);

extern const struct kefir_bucketset_ops kefir_bucketset_uint_ops;

#endif
