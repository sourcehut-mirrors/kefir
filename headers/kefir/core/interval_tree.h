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

#ifndef KEFIR_CORE_INTERVAL_TREE_H_
#define KEFIR_CORE_INTERVAL_TREE_H_

#include "kefir/core/hashtree.h"

typedef struct kefir_interval_tree kefir_interval_tree_t;  // Forward declaration

typedef kefir_uint64_t kefir_interval_tree_key_t;
typedef kefir_uptr_t kefir_interval_tree_value_t;

typedef struct kefir_interval_tree_entry {
    kefir_interval_tree_key_t begin;
    kefir_interval_tree_key_t max_subtree_end;

    struct kefir_hashtree nodes;
} kefir_interval_tree_entry_t;

typedef struct kefir_interval_tree_node {
    kefir_interval_tree_key_t begin;
    kefir_interval_tree_key_t end;
    kefir_interval_tree_value_t value;
} kefir_interval_tree_node_t;

typedef kefir_result_t (*kefir_interval_tree_remove_callback_t)(struct kefir_mem *, struct kefir_interval_tree *,
                                                                kefir_interval_tree_key_t, kefir_interval_tree_key_t,
                                                                kefir_interval_tree_value_t, void *);

typedef struct kefir_interval_tree {
    struct kefir_hashtree tree;
    struct {
        kefir_interval_tree_remove_callback_t callback;
        void *payload;
    } on_removal;
} kefir_interval_tree_t;

kefir_result_t kefir_interval_tree_init(struct kefir_interval_tree *);
kefir_result_t kefir_interval_tree_free(struct kefir_mem *, struct kefir_interval_tree *);

kefir_result_t kefir_interval_tree_on_remove(struct kefir_interval_tree *, kefir_interval_tree_remove_callback_t,
                                             void *);

kefir_result_t kefir_interval_tree_insert(struct kefir_mem *, struct kefir_interval_tree *, kefir_interval_tree_key_t,
                                          kefir_interval_tree_key_t, kefir_interval_tree_value_t);
kefir_result_t kefir_interval_tree_get(const struct kefir_interval_tree *, kefir_interval_tree_key_t,
                                       kefir_interval_tree_key_t, struct kefir_interval_tree_node **);

typedef struct kefir_interval_tree_finder {
    struct kefir_interval_tree_node *node;
    kefir_interval_tree_key_t position;
} kefir_interval_tree_finder_t;

kefir_result_t kefir_interval_tree_find(const struct kefir_interval_tree *, kefir_interval_tree_key_t,
                                        struct kefir_interval_tree_finder *, struct kefir_interval_tree_node **);
kefir_result_t kefir_interval_tree_find_next(const struct kefir_interval_tree *, struct kefir_interval_tree_finder *,
                                             struct kefir_interval_tree_node **);

typedef struct kefir_interval_tree_iterator {
    struct kefir_hashtree_node_iterator entry;
    struct kefir_hashtree_node_iterator node;
} kefir_interval_tree_iterator_t;

kefir_result_t kefir_interval_tree_iter(const struct kefir_interval_tree *, struct kefir_interval_tree_iterator *,
                                        struct kefir_interval_tree_node **);
kefir_result_t kefir_interval_tree_next(struct kefir_interval_tree_iterator *, struct kefir_interval_tree_node **);

#endif
