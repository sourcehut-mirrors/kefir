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
#include "kefir/core/bucketset.h"
#include "kefir/test/util.h"

DEFINE_CASE(core_bucketset1, "Core - Bucketset #1") {
    struct kefir_bucketset set;
    ASSERT_OK(kefir_bucketset_init(&set, &kefir_bucketset_uint_ops));

    const kefir_uint64_t Mask = 0xbadc0ffe;
    const kefir_uint64_t Count = 0xffff;
    for (kefir_uint64_t i = 0; i < Count; i++) {
        const kefir_uint64_t key = (i * 31) ^ Mask;
        ASSERT_OK(kefir_bucketset_add(&kft_mem, &set, (kefir_bucketset_entry_t) key));
    }

    for (kefir_uint64_t i = 0; i < Count; i++) {
        const kefir_uint64_t key = (i * 31) ^ Mask;
        ASSERT(kefir_bucketset_has(&set, (kefir_bucketset_entry_t) key));
        ASSERT(!kefir_bucketset_has(&set, (kefir_bucketset_entry_t) (key + 1)));
    }

    kefir_uint64_t sum = Count * (Count - 1) / 2;
    struct kefir_bucketset_iterator iter;
    kefir_result_t res;
    kefir_bucketset_entry_t entry;
    for (res = kefir_bucketset_iter(&set, &iter, &entry); res == KEFIR_OK; res = kefir_bucketset_next(&iter, &entry)) {
        kefir_uint64_t i = (entry ^ Mask) / 31;
        ASSERT(i < Count);
        sum -= i;
    }
    if (res != KEFIR_ITERATOR_END) {
        ASSERT_OK(res);
    }
    ASSERT(sum == 0);

    ASSERT_OK(kefir_bucketset_free(&kft_mem, &set));
}
END_CASE
