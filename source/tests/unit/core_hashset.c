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
#include "kefir/core/hashset.h"
#include "kefir/test/util.h"

DEFINE_CASE(core_hashset1, "Core - Hashset #1") {
    struct kefir_hashset set;
    ASSERT_OK(kefir_hashset_init(&set, &kefir_hashtable_uint_ops));

    const kefir_uint64_t Mask = 0xbadc0ffe;
    const kefir_uint64_t Count = 0xfffff;
    for (kefir_uint64_t i = 0; i < Count; i++) {
        const kefir_uint64_t key = (i * 31) ^ Mask;
        ASSERT_OK(kefir_hashset_add(&kft_mem, &set, (kefir_hashset_key_t) key));
        ASSERT_OK(kefir_hashset_add(&kft_mem, &set, (kefir_hashset_key_t) key));
    }

    for (kefir_uint64_t i = 0; i < Count; i++) {
        const kefir_uint64_t key = (i * 31) ^ Mask;

        ASSERT(kefir_hashset_has(&set, (kefir_hashtable_key_t) key));
        ASSERT(!kefir_hashset_has(&set, (kefir_hashtable_key_t) key + 1));
    }

    struct kefir_hashset_iterator iter;
    kefir_result_t res;
    kefir_hashtable_key_t key;
    kefir_size_t total = 0;
    for (res = kefir_hashset_iter(&set, &iter, &key);
        res == KEFIR_OK;
        res = kefir_hashset_next(&iter, &key)) {
        total++;
        ASSERT((key ^ Mask) / 31 < Count);
    }
    if (res != KEFIR_ITERATOR_END) {
        ASSERT_OK(res);
    }
    ASSERT(total == Count);

    ASSERT_OK(kefir_hashset_free(&kft_mem, &set));
}
END_CASE
