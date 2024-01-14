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

#ifndef KEFIR_CORE_HASHTREESET_H_
#define KEFIR_CORE_HASHTREESET_H_

#include "kefir/core/hashtree.h"

typedef kefir_hashtree_key_t kefir_hashtreeset_entry_t;

typedef struct kefir_hashtreeset {
    struct kefir_hashtree tree;

    struct {
        kefir_result_t (*callback)(struct kefir_mem *, struct kefir_hashtreeset *, kefir_hashtreeset_entry_t, void *);
        void *payload;
    } on_removal;
} kefir_hashtreeset_t;

kefir_result_t kefir_hashtreeset_init(struct kefir_hashtreeset *, const struct kefir_hashtree_ops *);
kefir_result_t kefir_hashtreeset_free(struct kefir_mem *, struct kefir_hashtreeset *);
kefir_result_t kefir_hashtreeset_on_remove(struct kefir_hashtreeset *,
                                           kefir_result_t (*)(struct kefir_mem *, struct kefir_hashtreeset *,
                                                              kefir_hashtreeset_entry_t, void *),
                                           void *);

kefir_result_t kefir_hashtreeset_add(struct kefir_mem *, struct kefir_hashtreeset *, kefir_hashtreeset_entry_t);
kefir_result_t kefir_hashtreeset_delete(struct kefir_mem *, struct kefir_hashtreeset *, kefir_hashtreeset_entry_t);
kefir_bool_t kefir_hashtreeset_has(const struct kefir_hashtreeset *, kefir_hashtreeset_entry_t);
kefir_bool_t kefir_hashtreeset_empty(const struct kefir_hashtreeset *);
kefir_result_t kefir_hashtreeset_clean(struct kefir_mem *, struct kefir_hashtreeset *);

kefir_result_t kefir_hashtreeset_merge(struct kefir_mem *, struct kefir_hashtreeset *, const struct kefir_hashtreeset *,
                                       kefir_result_t (*)(struct kefir_mem *, kefir_hashtreeset_entry_t,
                                                          kefir_hashtreeset_entry_t *, void *),
                                       void *);

typedef struct kefir_hashtreeset_iterator {
    struct kefir_hashtree_node_iterator iter;
    kefir_hashtreeset_entry_t entry;
} kefir_hashtreeset_iterator_t;

kefir_result_t kefir_hashtreeset_iter(const struct kefir_hashtreeset *, struct kefir_hashtreeset_iterator *);
kefir_result_t kefir_hashtreeset_next(struct kefir_hashtreeset_iterator *);

#endif
