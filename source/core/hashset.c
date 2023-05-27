/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

#include "kefir/core/hashtreeset.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_hashtreeset_init(struct kefir_hashtreeset *set, const struct kefir_hashtree_ops *ops) {
    REQUIRE(set != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to hashtreeset"));
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtree operations"));

    set->on_removal.callback = NULL;
    set->on_removal.payload = NULL;
    REQUIRE_OK(kefir_hashtree_init(&set->tree, ops));
    return KEFIR_OK;
}

kefir_result_t kefir_hashtreeset_free(struct kefir_mem *mem, struct kefir_hashtreeset *set) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(set != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtreeset"));

    REQUIRE_OK(kefir_hashtree_free(mem, &set->tree));
    return KEFIR_OK;
}

static kefir_result_t on_removal(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                 kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(value);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected memory allocator"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected hashtreeset entry removal payload"));

    ASSIGN_DECL_CAST(struct kefir_hashtreeset *, set, payload);
    if (set->on_removal.callback != NULL) {
        REQUIRE_OK(set->on_removal.callback(mem, set, (kefir_hashtreeset_entry_t) key, set->on_removal.payload));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_hashtreeset_on_remove(struct kefir_hashtreeset *set,
                                           kefir_result_t (*callback)(struct kefir_mem *, struct kefir_hashtreeset *,
                                                                      kefir_hashtreeset_entry_t, void *),
                                           void *payload) {
    REQUIRE(set != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtreeset"));

    set->on_removal.callback = callback;
    set->on_removal.payload = payload;
    REQUIRE_OK(kefir_hashtree_on_removal(&set->tree, on_removal, (void *) set));
    return KEFIR_OK;
}

kefir_result_t kefir_hashtreeset_add(struct kefir_mem *mem, struct kefir_hashtreeset *set,
                                     kefir_hashtreeset_entry_t entry) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(set != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtreeset"));

    kefir_result_t res =
        kefir_hashtree_insert(mem, &set->tree, (kefir_hashtree_key_t) entry, (kefir_hashtree_value_t) 0);
    if (res == KEFIR_ALREADY_EXISTS) {
        if (set->on_removal.callback != NULL) {
            REQUIRE_OK(set->on_removal.callback(mem, set, entry, set->on_removal.payload));
        }
    } else {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_hashtreeset_delete(struct kefir_mem *mem, struct kefir_hashtreeset *set,
                                        kefir_hashtreeset_entry_t entry) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(set != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtreeset"));

    kefir_result_t res = kefir_hashtree_delete(mem, &set->tree, (kefir_hashtree_key_t) entry);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

kefir_bool_t kefir_hashtreeset_has(const struct kefir_hashtreeset *set, kefir_hashtreeset_entry_t entry) {
    REQUIRE(set != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtreeset"));

    return kefir_hashtree_has(&set->tree, (kefir_hashtree_key_t) entry);
}

kefir_bool_t kefir_hashtreeset_empty(const struct kefir_hashtreeset *set) {
    REQUIRE(set != NULL, true);

    return kefir_hashtree_empty(&set->tree);
}

kefir_result_t kefir_hashtreeset_merge(struct kefir_mem *mem, struct kefir_hashtreeset *target_set,
                                       const struct kefir_hashtreeset *source_set,
                                       kefir_result_t (*clone_fn)(struct kefir_mem *, kefir_hashtreeset_entry_t,
                                                                  kefir_hashtreeset_entry_t *, void *),
                                       void *clone_payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target_set != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target hashtreeset"));
    REQUIRE(source_set != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source hashtreeset"));

    struct kefir_hashtreeset_iterator iter;
    kefir_result_t res;
    for (res = kefir_hashtreeset_iter(source_set, &iter); res == KEFIR_OK; res = kefir_hashtreeset_next(&iter)) {
        kefir_hashtreeset_entry_t new_entry = iter.entry;
        if (clone_fn != NULL) {
            REQUIRE_OK(clone_fn(mem, new_entry, &new_entry, clone_payload));
        }
        REQUIRE_OK(kefir_hashtreeset_add(mem, target_set, new_entry));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_hashtreeset_iter(const struct kefir_hashtreeset *set, struct kefir_hashtreeset_iterator *iter) {
    REQUIRE(set != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtreeset"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to hashtreeset iterator"));

    const struct kefir_hashtree_node *node = kefir_hashtree_iter(&set->tree, &iter->iter);
    REQUIRE(node != NULL, KEFIR_ITERATOR_END);
    iter->entry = (kefir_hashtreeset_entry_t) node->key;
    return KEFIR_OK;
}

kefir_result_t kefir_hashtreeset_next(struct kefir_hashtreeset_iterator *iter) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtreeset iterator"));

    const struct kefir_hashtree_node *node = kefir_hashtree_next(&iter->iter);
    REQUIRE(node != NULL, KEFIR_ITERATOR_END);
    iter->entry = (kefir_hashtreeset_entry_t) node->key;
    return KEFIR_OK;
}
