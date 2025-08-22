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

#include "kefir/test/unit_test.h"
#include "kefir/core/hashtable.h"
#include "kefir/test/util.h"

DEFINE_CASE(core_hashtable1, "Core - Hashtable #1") {
    struct kefir_hashtable table;
    ASSERT_OK(kefir_hashtable_init(&table, &kefir_hashtable_uint_ops));

    const kefir_uint64_t Mask = 0xbadc0ffe;
    const kefir_uint64_t Count = 0xfffff;
    for (kefir_uint64_t i = 0; i < Count; i++) {
        const kefir_uint64_t key = (i * 31) ^ Mask;
        ASSERT_OK(kefir_hashtable_insert(&kft_mem, &table, (kefir_hashtable_key_t) key, (kefir_hashtable_value_t) i));
        ASSERT(kefir_hashtable_insert(&kft_mem, &table, (kefir_hashtable_key_t) key, (kefir_hashtable_value_t) i) == KEFIR_ALREADY_EXISTS);
    }

    for (kefir_uint64_t i = 0; i < Count; i++) {
        const kefir_uint64_t key = (i * 31) ^ Mask;

        ASSERT(kefir_hashtable_has(&table, (kefir_hashtable_key_t) key));
        ASSERT(!kefir_hashtable_has(&table, (kefir_hashtable_key_t) key + 1));

        kefir_hashtable_value_t value;
        ASSERT_OK(kefir_hashtable_at(&table, (kefir_hashtable_key_t) key, &value));
        ASSERT(value == i);

        ASSERT(kefir_hashtable_at(&table, (kefir_hashtable_key_t) key + 1, &value) == KEFIR_NOT_FOUND);
    }

    struct kefir_hashtable_iterator iter;
    kefir_result_t res;
    kefir_hashtable_key_t key;
    kefir_hashtable_value_t value;
    kefir_size_t total = 0;
    for (res = kefir_hashtable_iter(&table, &iter, &key, &value);
        res == KEFIR_OK;
        res = kefir_hashtable_next(&iter, &key, &value)) {
        total++;
        ASSERT(key == ((value * 31) ^ Mask));
    }
    if (res != KEFIR_ITERATOR_END) {
        ASSERT_OK(res);
    }
    ASSERT(total == Count);

    ASSERT_OK(kefir_hashtable_free(&kft_mem, &table));
}
END_CASE
