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

#include "kefir/test/unit_test.h"
#include "kefir/core/hashtree.h"
#include "kefir/test/util.h"

DEFINE_CASE(core_hashtree1, "Core - Hashtree #1") {
    struct kefir_hashtree tree;
    ASSERT_OK(kefir_hashtree_init(&tree, &kefir_hashtree_uint_ops));

    const kefir_uint64_t Mask = 0xbadc0ffe;
    const kefir_uint64_t Count = 0xfffff;
    for (kefir_uint64_t i = 0; i < Count; i++) {
        const kefir_uint64_t key = (i * 31) ^ Mask;
        ASSERT_OK(kefir_hashtree_insert(&kft_mem, &tree, (kefir_hashtree_key_t) key, (kefir_hashtree_value_t) i));
    }

    struct kefir_hashtree_node_iterator iter;
    kefir_size_t node_count = 0;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&tree, &iter); node != NULL;
         node = kefir_hashtree_next(&iter), node_count++) {

        ASSERT((((kefir_uint64_t) node->key) ^ Mask) / 31 == (kefir_uint64_t) node->value);
        kefir_int64_t left_height = node->left_child != NULL ? node->left_child->height + 1 : 0;
        kefir_int64_t right_height = node->right_child != NULL ? node->right_child->height + 1 : 0;
        kefir_int64_t bf = right_height - left_height;
        ASSERT(node->height == (kefir_size_t) MAX(left_height, right_height));
        ASSERT(bf >= -1 && bf <= 1);
    }
    ASSERT(node_count == Count);

    for (kefir_uint64_t i = 0; i < 0x1f; i++) {
        for (kefir_uint64_t j = 0; j < 0xe; j++) {
            kefir_uint64_t key = (((i << 8) | j) * 31) ^ Mask;
            ASSERT(kefir_hashtree_has(&tree, (kefir_hashtree_key_t) key));
            ASSERT_OK(kefir_hashtree_delete(&kft_mem, &tree, (kefir_hashtree_key_t) key));
            ASSERT(!kefir_hashtree_has(&tree, (kefir_hashtree_key_t) key));
        }
    }
    node_count = 0;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&tree, &iter); node != NULL;
         node = kefir_hashtree_next(&iter), node_count++) {

        kefir_uint64_t i = (((kefir_uint64_t) node->key) ^ Mask) / 31;
        kefir_uint64_t c1 = (i >> 8);
        kefir_uint64_t c2 = i & 0xff;
        ASSERT(c1 >= 0x1f || c2 >= 0xe);
        ASSERT(i == (kefir_uint64_t) node->value);
        kefir_int64_t left_height = node->left_child != NULL ? node->left_child->height + 1 : 0;
        kefir_int64_t right_height = node->right_child != NULL ? node->right_child->height + 1 : 0;
        kefir_int64_t bf = right_height - left_height;
        ASSERT(node->height == (kefir_size_t) MAX(left_height, right_height));
        ASSERT(bf >= -1 && bf <= 1);
    }
    ASSERT(node_count == Count - 0x1f * 0xe);

    ASSERT_OK(kefir_hashtree_free(&kft_mem, &tree));
}
END_CASE
