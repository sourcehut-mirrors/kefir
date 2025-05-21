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

#include "kefir/util/bigint/bigint.h"
#include "kefir/test/unit_test.h"
#include "kefir/test/util.h"

#define ASSERT_LOAD(_bigint, _value)                           \
    do {                                                       \
        kefir_int64_t loaded;                                  \
        ASSERT_OK(kefir_bigint_get_value((_bigint), &loaded)); \
        ASSERT(loaded == (_value));                            \
    } while (0)
#define ASSERT_STORE(_bigint, _value)                                     \
    do {                                                                  \
        ASSERT_OK(kefir_bigint_set_value(&kft_mem, (_bigint), (_value))); \
        ASSERT((_bigint)->bitwidth <= CHAR_BIT * sizeof(kefir_int64_t));  \
    } while (0)

DEFINE_CASE(bitint_basic1, "BigInt - load/store machine integer #1") {
    struct kefir_bigint bigint;

    ASSERT_OK(kefir_bigint_init(&bigint));

#define ASSERT_STORE_LOAD(_bigint, _value) \
    do {                                   \
        ASSERT_STORE((_bigint), (_value)); \
        ASSERT_LOAD((_bigint), (_value));  \
    } while (0)

    ASSERT_LOAD(&bigint, 0);
    ASSERT_STORE_LOAD(&bigint, 0);
    ASSERT_STORE_LOAD(&bigint, -1);
    ASSERT_STORE_LOAD(&bigint, 1);
    ASSERT_STORE_LOAD(&bigint, KEFIR_INT8_MIN);
    ASSERT_STORE_LOAD(&bigint, KEFIR_INT8_MAX);
    ASSERT_STORE_LOAD(&bigint, KEFIR_INT16_MIN);
    ASSERT_STORE_LOAD(&bigint, KEFIR_INT16_MAX);
    ASSERT_STORE_LOAD(&bigint, KEFIR_INT32_MIN);
    ASSERT_STORE_LOAD(&bigint, KEFIR_INT32_MAX);
    ASSERT_STORE_LOAD(&bigint, KEFIR_INT64_MIN);
    ASSERT_STORE_LOAD(&bigint, KEFIR_INT64_MAX);

    for (kefir_size_t i = 0; i < CHAR_BIT * sizeof(kefir_uint64_t); i++) {
        ASSERT_STORE_LOAD(&bigint, (kefir_int64_t) (1ull << i));
        ASSERT_STORE_LOAD(&bigint, (kefir_int64_t) ~(1ull << i));
        ASSERT_STORE_LOAD(&bigint, (kefir_int64_t) ((1ull << i) ^ 0xbadcafebabeull));
    }

#undef ASSERT_STORE_LOAD

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
}
END_CASE

DEFINE_CASE(bitint_cast_signed1, "BigInt - signed cast #1") {
    struct kefir_bigint bigint;

    ASSERT_OK(kefir_bigint_init(&bigint));

#define ASSERT_CAST(_bigint, _value, _width1, _value1, _width2, _value2)     \
    do {                                                                     \
        ASSERT_STORE((_bigint), (_value));                                   \
        ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, (_bigint), (_width1))); \
        ASSERT_LOAD((_bigint), (_value1));                                   \
        ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, (_bigint), (_width2))); \
        ASSERT_LOAD((_bigint), (_value2));                                   \
    } while (0)

    ASSERT_CAST(&bigint, 1, 2, 1, 2, 1);
    ASSERT_CAST(&bigint, 1, 10, 1, 2, 1);
    ASSERT_CAST(&bigint, 1, 500, 1, 2, 1);

    ASSERT_CAST(&bigint, -1, 2, -1, 2, -1);
    ASSERT_CAST(&bigint, -1, 10, -1, 2, -1);
    ASSERT_CAST(&bigint, -1, 500, -1, 2, -1);

    ASSERT_CAST(&bigint, KEFIR_INT8_MAX, 6, -1, 1000, -1);
    ASSERT_CAST(&bigint, KEFIR_INT8_MAX, 7, -1, 64, -1);
    ASSERT_CAST(&bigint, KEFIR_INT8_MAX, 100, KEFIR_INT8_MAX, 8, KEFIR_INT8_MAX);
    ASSERT_CAST(&bigint, KEFIR_INT8_MAX, 2000, KEFIR_INT8_MAX, 7, -1);
    ASSERT_CAST(&bigint, KEFIR_INT8_MIN, 2, 0, 700, 0);
    ASSERT_CAST(&bigint, KEFIR_INT8_MIN, 200, KEFIR_INT8_MIN, 8, KEFIR_INT8_MIN);
    ASSERT_CAST(&bigint, KEFIR_INT8_MIN, 200, KEFIR_INT8_MIN, 7, 0);

    ASSERT_CAST(&bigint, KEFIR_INT16_MAX, 2000, KEFIR_INT16_MAX, 8, -1);
    ASSERT_CAST(&bigint, KEFIR_INT16_MAX, 14, -1, 250, -1);
    ASSERT_CAST(&bigint, KEFIR_INT16_MIN, 350, KEFIR_INT16_MIN, 16, KEFIR_INT16_MIN);
    ASSERT_CAST(&bigint, KEFIR_INT16_MIN, 350, KEFIR_INT16_MIN, 15, 0);

    ASSERT_CAST(&bigint, KEFIR_INT32_MAX, 64, KEFIR_INT32_MAX, 32, KEFIR_INT32_MAX);
    ASSERT_CAST(&bigint, KEFIR_INT32_MAX, 30, -1, 32, -1);
    ASSERT_CAST(&bigint, KEFIR_INT32_MIN, 1500, KEFIR_INT32_MIN, 32, KEFIR_INT32_MIN);
    ASSERT_CAST(&bigint, KEFIR_INT32_MIN, 20, 0, 32, 0);

    ASSERT_CAST(&bigint, KEFIR_INT64_MAX, 640, KEFIR_INT64_MAX, 64, KEFIR_INT64_MAX);
    ASSERT_CAST(&bigint, KEFIR_INT64_MAX, 60, -1, 70, -1);
    ASSERT_CAST(&bigint, KEFIR_INT64_MIN, 1500, KEFIR_INT64_MIN, 64, KEFIR_INT64_MIN);
    ASSERT_CAST(&bigint, KEFIR_INT64_MIN, 63, 0, 64, 0);

    for (kefir_size_t i = 0; i < CHAR_BIT * sizeof(kefir_uint64_t); i++) {
        ASSERT_CAST(&bigint, (kefir_int64_t) (1ull << i), 256, (kefir_int64_t) (1ull << i), i + 2,
                    (kefir_int64_t) (1ull << i));
        ASSERT_CAST(&bigint, ~(kefir_int64_t) (1ull << i), 256, ~(kefir_int64_t) (1ull << i), i + 2,
                    ~(kefir_int64_t) (1ull << i));
        ASSERT_CAST(&bigint, ~(kefir_int64_t) (1ull << i) ^ 0xbadcafebabell, 256,
                    ~(kefir_int64_t) (1ull << i) ^ 0xbadcafebabell, CHAR_BIT * sizeof(kefir_uint64_t),
                    ~(kefir_int64_t) (1ull << i) ^ 0xbadcafebabell);
    }

#undef ASSERT_CAST

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
}
END_CASE

#undef ASSERT_STORE
#undef ASSERT_LOAD