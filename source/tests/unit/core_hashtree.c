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

DEFINE_CASE(core_hashtree2, "Core - Hashtree #2") {
    struct kefir_hashtree tree;
    ASSERT_OK(kefir_hashtree_init(&tree, &kefir_hashtree_uint_ops));

#define BEGIN 100
#define RANGE 1000
#define FACTOR 1000

    for (kefir_uint64_t i = BEGIN; i <= RANGE; i++) {
        ASSERT_OK(
            kefir_hashtree_insert(&kft_mem, &tree, (kefir_hashtree_key_t) (i * FACTOR), (kefir_hashtree_value_t) 0));
    }

    struct kefir_hashtree_node *node;
    for (kefir_uint64_t i = 0; i <= FACTOR * (RANGE + 10); i++) {
        if (i >= FACTOR * BEGIN) {
            ASSERT_OK(kefir_hashtree_lower_bound(&tree, (kefir_hashtree_key_t) i, &node));
            ASSERT(node->key == MIN(i / FACTOR, RANGE) * FACTOR);
        } else {
            ASSERT(kefir_hashtree_lower_bound(&tree, (kefir_hashtree_key_t) i, &node) == KEFIR_NOT_FOUND);
        }
    }

    for (kefir_uint64_t i = BEGIN; i <= RANGE; i++) {
        if (i % 3 == 0) {
            ASSERT_OK(kefir_hashtree_delete(&kft_mem, &tree, (kefir_hashtree_key_t) (i * FACTOR)));
        }
    }

    for (kefir_uint64_t i = 0; i <= FACTOR * (RANGE + 10); i++) {
        if (i >= FACTOR * BEGIN) {
            ASSERT_OK(kefir_hashtree_lower_bound(&tree, (kefir_hashtree_key_t) i, &node));
            kefir_uint64_t index = MIN(i / FACTOR, RANGE);
            if (index % 3 == 0) {
                index--;
            }
            index *= FACTOR;
            ASSERT(node->key == index);
        } else {
            ASSERT(kefir_hashtree_lower_bound(&tree, (kefir_hashtree_key_t) i, &node) == KEFIR_NOT_FOUND);
        }
    }

    for (kefir_uint64_t i = BEGIN; i <= RANGE; i++) {
        if (i % 7 == 0 && i % 3 != 0) {
            ASSERT_OK(kefir_hashtree_delete(&kft_mem, &tree, (kefir_hashtree_key_t) (i * FACTOR)));
        }
    }

    for (kefir_uint64_t i = 0; i <= FACTOR * (RANGE + 10); i++) {
        if (i >= FACTOR * BEGIN) {
            ASSERT_OK(kefir_hashtree_lower_bound(&tree, (kefir_hashtree_key_t) i, &node));
            kefir_uint64_t index = MIN(i / FACTOR, RANGE);
            if (index % 3 == 0 || index % 7 == 0) {
                index--;
            }
            if (index % 3 == 0 || index % 7 == 0) {
                index--;
            }
            index *= FACTOR;
            ASSERT(node->key == index);
        } else {
            ASSERT(kefir_hashtree_lower_bound(&tree, (kefir_hashtree_key_t) i, &node) == KEFIR_NOT_FOUND);
        }
    }

#undef BEGIN
#undef RANGE
#undef FACTOR

    ASSERT_OK(kefir_hashtree_free(&kft_mem, &tree));
}
END_CASE

DEFINE_CASE(core_hashtree3, "Core - Hashtree #3") {
    struct kefir_hashtree tree;
    ASSERT_OK(kefir_hashtree_init(&tree, &kefir_hashtree_uint_ops));

#define BEGIN 1
#define END 50000

    for (kefir_uint64_t i = BEGIN; i <= END; i++) {
        ASSERT_OK(kefir_hashtree_insert(&kft_mem, &tree, (kefir_hashtree_key_t) i, (kefir_hashtree_value_t) 0));
    }

    kefir_uint64_t last_index = BEGIN - 1;
    struct kefir_hashtree_node_iterator iter;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&tree, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSERT(last_index == node->key - 1);
        last_index = node->key;

        struct kefir_hashtree_node *next_node = kefir_hashtree_next_node(&tree, node);
        if (last_index < END) {
            ASSERT(next_node->key = last_index + 1);
        } else {
            ASSERT(next_node == NULL);
        }
    }
    ASSERT(last_index == END);

    for (kefir_uint64_t i = BEGIN; i < END; i++) {
        if (i % 3 == 0 || i % 13 == 0) {
            ASSERT_OK(kefir_hashtree_delete(&kft_mem, &tree, (kefir_hashtree_key_t) i));
        }
    }

    last_index = BEGIN - 1;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&tree, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        kefir_uint64_t expected_index = last_index + 1;
        if (expected_index % 3 == 0 || expected_index % 13 == 0) {
            expected_index++;
        }
        if (expected_index % 3 == 0 || expected_index % 13 == 0) {
            expected_index++;
        }
        ASSERT(expected_index == node->key);
        last_index = node->key;

        struct kefir_hashtree_node *next_node = kefir_hashtree_next_node(&tree, node);
        if (last_index < END) {
            expected_index = last_index + 1;
            if (expected_index % 3 == 0 || expected_index % 13 == 0) {
                expected_index++;
            }
            if (expected_index % 3 == 0 || expected_index % 13 == 0) {
                expected_index++;
            }
            ASSERT(next_node->key = expected_index);
        } else {
            ASSERT(next_node == NULL);
        }
    }
    ASSERT(last_index == END);

    ASSERT_OK(kefir_hashtree_free(&kft_mem, &tree));
}
END_CASE
