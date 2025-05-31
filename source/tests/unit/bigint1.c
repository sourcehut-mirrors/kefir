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

#include "kefir/util/bigint.h"
#include "kefir/test/unit_test.h"
#include "kefir/test/util.h"
#include <float.h>

#define ASSERT_LOAD(_bigint, _value)                            \
    do {                                                        \
        kefir_int64_t loaded;                                   \
        ASSERT_OK(kefir_bigint_get_signed((_bigint), &loaded)); \
        ASSERT(loaded == (_value));                             \
    } while (0)
#define ASSERT_ULOAD(_bigint, _value)                             \
    do {                                                          \
        kefir_uint64_t loaded;                                    \
        ASSERT_OK(kefir_bigint_get_unsigned((_bigint), &loaded)); \
        ASSERT(loaded == (_value));                               \
    } while (0)
#define ASSERT_STORE(_bigint, _value)                                                                          \
    do {                                                                                                       \
        ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, (_bigint),                                              \
                                             kefir_bigint_min_native_signed_width((kefir_int64_t) (_value)))); \
        ASSERT_OK(kefir_bigint_set_signed_value((_bigint), (_value)));                                         \
        ASSERT((_bigint)->bitwidth <= CHAR_BIT * sizeof(kefir_int64_t));                                       \
    } while (0)
#define ASSERT_USTORE(_bigint, _value)                                                                           \
    do {                                                                                                         \
        ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, (_bigint),                                                \
                                             kefir_bigint_min_native_unsigned_width((kefir_int64_t) (_value)))); \
        ASSERT_OK(kefir_bigint_set_unsigned_value((_bigint), (_value)));                                         \
        ASSERT((_bigint)->bitwidth <= CHAR_BIT * sizeof(kefir_int64_t));                                         \
    } while (0)

DEFINE_CASE(bigint_basic1, "BigInt - load/store signed integers #1") {
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

DEFINE_CASE(bigint_basic2, "BigInt - load/store unsigned integers #1") {
    struct kefir_bigint bigint;

    ASSERT_OK(kefir_bigint_init(&bigint));

#define ASSERT_STORE_LOAD(_bigint, _value)                   \
    do {                                                     \
        ASSERT_USTORE((_bigint), (kefir_uint64_t) (_value)); \
        ASSERT_ULOAD((_bigint), (kefir_uint64_t) (_value));  \
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

DEFINE_CASE(bigint_cast_signed1, "BigInt - signed cast #1") {
    struct kefir_bigint bigint;

    ASSERT_OK(kefir_bigint_init(&bigint));

#define ASSERT_CAST(_bigint, _value, _width1, _value1, _width2, _value2)            \
    do {                                                                            \
        ASSERT_STORE((_bigint), (_value));                                          \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, (_bigint), (_width1))); \
        ASSERT_LOAD((_bigint), (_value1));                                          \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, (_bigint), (_width2))); \
        ASSERT_LOAD((_bigint), (_value2));                                          \
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

DEFINE_CASE(bigint_cast_unsigned1, "BigInt - unsigned cast #1") {
    struct kefir_bigint bigint;

    ASSERT_OK(kefir_bigint_init(&bigint));

#define ASSERT_CAST(_bigint, _value, _width1, _value1, _width2, _value2)              \
    do {                                                                              \
        ASSERT_USTORE((_bigint), (_value));                                           \
        ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, (_bigint), (_width1))); \
        ASSERT_ULOAD((_bigint), (_value1));                                           \
        ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, (_bigint), (_width2))); \
        ASSERT_ULOAD((_bigint), (_value2));                                           \
    } while (0)

    ASSERT_CAST(&bigint, 1, 2, 1, 2, 1);
    ASSERT_CAST(&bigint, 1, 10, 1, 2, 1);
    ASSERT_CAST(&bigint, 1, 500, 1, 2, 1);

    ASSERT_CAST(&bigint, -1, 2, 3, 2, 3);
    ASSERT_CAST(&bigint, -1, 10, 1023, 2, 3);
    ASSERT_CAST(&bigint, -1, 500, KEFIR_UINT64_MAX, 2, 3);

    ASSERT_CAST(&bigint, KEFIR_INT8_MAX, 6, (1 << 6) - 1, 1000, (1 << 6) - 1);
    ASSERT_CAST(&bigint, KEFIR_INT8_MAX, 7, (1 << 7) - 1, 64, (1 << 7) - 1);
    ASSERT_CAST(&bigint, KEFIR_INT8_MAX, 100, KEFIR_INT8_MAX, 8, KEFIR_INT8_MAX);
    ASSERT_CAST(&bigint, KEFIR_INT8_MAX, 2000, KEFIR_INT8_MAX, 7, (1 << 7) - 1);
    ASSERT_CAST(&bigint, KEFIR_INT8_MIN, 2, 0, 700, 0);
    ASSERT_CAST(&bigint, (kefir_uint8_t) KEFIR_INT8_MIN, 200, 1 << 7, 8, 1 << 7);
    ASSERT_CAST(&bigint, (kefir_uint8_t) KEFIR_INT8_MIN, 200, 1 << 7, 7, 0);

    ASSERT_CAST(&bigint, KEFIR_INT16_MAX, 2000, KEFIR_INT16_MAX, 8, (1 << 8) - 1);
    ASSERT_CAST(&bigint, KEFIR_INT16_MAX, 14, (1 << 14) - 1, 250, (1 << 14) - 1);
    ASSERT_CAST(&bigint, (kefir_uint16_t) KEFIR_INT16_MIN, 350, 1 << 15, 16, 1 << 15);
    ASSERT_CAST(&bigint, (kefir_uint16_t) KEFIR_INT16_MIN, 350, 1 << 15, 14, 0);

    ASSERT_CAST(&bigint, KEFIR_INT32_MAX, 64, KEFIR_INT32_MAX, 32, KEFIR_INT32_MAX);
    ASSERT_CAST(&bigint, KEFIR_INT32_MAX, 30, (1 << 30) - 1, 32, (1 << 30) - 1);
    ASSERT_CAST(&bigint, (kefir_uint32_t) KEFIR_INT32_MIN, 1500, 1ull << 31, 32, 1ull << 31);
    ASSERT_CAST(&bigint, KEFIR_INT32_MIN, 20, 0, 32, 0);

    ASSERT_CAST(&bigint, KEFIR_INT64_MAX, 640, KEFIR_INT64_MAX, 64, KEFIR_INT64_MAX);
    ASSERT_CAST(&bigint, KEFIR_INT64_MAX, 60, (1ull << 60) - 1, 70, (1ull << 60) - 1);
    ASSERT_CAST(&bigint, KEFIR_INT64_MIN, 1500, 1ull << 63, 64, 1ull << 63);
    ASSERT_CAST(&bigint, KEFIR_INT64_MIN, 63, 0, 64, 0);

    for (kefir_size_t i = 0; i < CHAR_BIT * sizeof(kefir_uint64_t); i++) {
        ASSERT_CAST(&bigint, (1ull << i), 256, (1ull << i), i + 2, (1ull << i));
        if (i + 2 < CHAR_BIT * sizeof(kefir_uint64_t)) {
            ASSERT_CAST(&bigint, ~(1ull << i), 256, ~(1ull << i), i + 2, ~(1ull << i) & ((1ull << (i + 2)) - 1));
        } else {
            ASSERT_CAST(&bigint, ~(1ull << i), 256, ~(1ull << i), i + 2, ~(1ull << i));
        }
    }

#undef ASSERT_CAST

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
}
END_CASE

DEFINE_CASE(bigint_add1, "BigInt - addition #1") {
    struct kefir_bigint lhs_bigint, rhs_bigint;

    ASSERT_OK(kefir_bigint_init(&lhs_bigint));
    ASSERT_OK(kefir_bigint_init(&rhs_bigint));

#define ASSERT_ADD(_lhs, _rhs, _arg1, _arg2, _res)                                                                 \
    do {                                                                                                           \
        ASSERT_STORE((_lhs), (_arg1));                                                                             \
        ASSERT_STORE((_rhs), (_arg2));                                                                             \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, (_lhs), MAX((_lhs)->bitwidth, (_rhs)->bitwidth) + 1)); \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, (_rhs), (_lhs)->bitwidth));                            \
        ASSERT_OK(kefir_bigint_add((_lhs), (_rhs)));                                                               \
        ASSERT_LOAD((_lhs), (_res));                                                                               \
    } while (0)

    ASSERT_ADD(&lhs_bigint, &rhs_bigint, 0, 0, 0);
    ASSERT_ADD(&lhs_bigint, &rhs_bigint, 1000, 2000, 3000);
    ASSERT_ADD(&lhs_bigint, &rhs_bigint, 1000, -2000, -1000);
    ASSERT_ADD(&lhs_bigint, &rhs_bigint, KEFIR_UINT32_MAX, KEFIR_UINT32_MAX, 2ull * KEFIR_UINT32_MAX);
    ASSERT_ADD(&lhs_bigint, &rhs_bigint, KEFIR_INT32_MIN, KEFIR_INT32_MIN, 2ll * KEFIR_INT32_MIN);
    ASSERT_ADD(&lhs_bigint, &rhs_bigint, KEFIR_UINT64_MAX, KEFIR_UINT64_MAX, -2);
    ASSERT_ADD(&lhs_bigint, &rhs_bigint, KEFIR_INT64_MIN, KEFIR_INT64_MIN, 0);
    ASSERT_ADD(&lhs_bigint, &rhs_bigint, KEFIR_INT64_MAX, KEFIR_INT64_MIN, -1);

    for (kefir_int64_t i = -1024; i < 1024; i++) {
        for (kefir_int64_t j = -256; j < 256; j++) {
            ASSERT_ADD(&lhs_bigint, &rhs_bigint, i, j, i + j);
            ASSERT_ADD(&lhs_bigint, &rhs_bigint, (1ull << i % (sizeof(kefir_int64_t) * CHAR_BIT)), j,
                       (kefir_int64_t) ((1ull << i % (sizeof(kefir_int64_t) * CHAR_BIT)) + j));
            ASSERT_ADD(&lhs_bigint, &rhs_bigint, i ^ 0xcafebabebad, j, (i ^ 0xcafebabebad) + j);
        }
    }

#undef ASSERT_ADD

    ASSERT_STORE(&lhs_bigint, KEFIR_INT64_MIN);
    ASSERT_STORE(&rhs_bigint, KEFIR_INT64_MIN);
    ASSERT_OK(
        kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_add(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, 0);
    ASSERT_OK(
        kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_add(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, KEFIR_INT64_MIN);
    ASSERT_OK(
        kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_add(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, 0);

    ASSERT_STORE(&lhs_bigint, KEFIR_UINT64_MAX);
    ASSERT_STORE(&rhs_bigint, KEFIR_UINT64_MAX);
    ASSERT_OK(
        kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_add(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, (kefir_int64_t) -2);
    ASSERT_OK(
        kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_add(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, (kefir_int64_t) -3);
    ASSERT_OK(
        kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_add(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, (kefir_int64_t) -4);
    ASSERT_OK(
        kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_add(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, (kefir_int64_t) -5);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &lhs_bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &rhs_bigint));
}
END_CASE

DEFINE_CASE(bigint_invert1, "BigInt - invert #1") {
    struct kefir_bigint bigint;

    ASSERT_OK(kefir_bigint_init(&bigint));

#define ASSERT_INVERT(_bigint, _arg)                     \
    do {                                                 \
        ASSERT_STORE((_bigint), (_arg));                 \
        ASSERT_OK(kefir_bigint_invert((_bigint)));       \
        ASSERT_LOAD((_bigint), ~(kefir_int64_t) (_arg)); \
    } while (0)

    ASSERT_INVERT(&bigint, 0);
    ASSERT_INVERT(&bigint, 1);
    ASSERT_INVERT(&bigint, -1);
    ASSERT_INVERT(&bigint, KEFIR_UINT8_MIN);
    ASSERT_INVERT(&bigint, KEFIR_UINT8_MAX);
    ASSERT_INVERT(&bigint, KEFIR_UINT16_MIN);
    ASSERT_INVERT(&bigint, KEFIR_UINT16_MAX);
    ASSERT_INVERT(&bigint, KEFIR_UINT32_MIN);
    ASSERT_INVERT(&bigint, KEFIR_UINT32_MAX);
    ASSERT_INVERT(&bigint, KEFIR_UINT64_MIN);
    ASSERT_INVERT(&bigint, KEFIR_UINT64_MAX);

    for (kefir_int64_t i = -10240; i < 10240; i++) {
        ASSERT_INVERT(&bigint, i);
        ASSERT_INVERT(&bigint, (1ull << i % (sizeof(kefir_int64_t) * CHAR_BIT)));
        ASSERT_INVERT(&bigint, i ^ 0xcafebabebad);
    }

#undef ASSERT_INVERT

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
}
END_CASE

DEFINE_CASE(bigint_and1, "BigInt - bitwise and #1") {
    struct kefir_bigint lhs_bigint, rhs_bigint;

    ASSERT_OK(kefir_bigint_init(&lhs_bigint));
    ASSERT_OK(kefir_bigint_init(&rhs_bigint));

#define ASSERT_AND(_lhs, _rhs, _arg1, _arg2)                                                                       \
    do {                                                                                                           \
        ASSERT_STORE((_lhs), (_arg1));                                                                             \
        ASSERT_STORE((_rhs), (_arg2));                                                                             \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, (_lhs), MAX((_lhs)->bitwidth, (_rhs)->bitwidth) + 1)); \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, (_rhs), (_lhs)->bitwidth));                            \
        ASSERT_OK(kefir_bigint_and((_lhs), (_rhs)));                                                               \
        ASSERT_LOAD((_lhs), ((kefir_int64_t) (_arg1)) & (_arg2));                                                  \
    } while (0)

    ASSERT_AND(&lhs_bigint, &rhs_bigint, 0, 0);
    ASSERT_AND(&lhs_bigint, &rhs_bigint, 1, 0);
    ASSERT_AND(&lhs_bigint, &rhs_bigint, 0, 1);
    ASSERT_AND(&lhs_bigint, &rhs_bigint, 1, 1);
    ASSERT_AND(&lhs_bigint, &rhs_bigint, 1, -1);
    ASSERT_AND(&lhs_bigint, &rhs_bigint, -1, -1);
    ASSERT_AND(&lhs_bigint, &rhs_bigint, KEFIR_INT32_MAX, KEFIR_INT32_MIN);
    ASSERT_AND(&lhs_bigint, &rhs_bigint, KEFIR_INT64_MAX, KEFIR_INT32_MAX);
    ASSERT_AND(&lhs_bigint, &rhs_bigint, KEFIR_INT64_MAX, ~KEFIR_INT32_MIN);

    for (kefir_int64_t i = -512; i < 512; i++) {
        for (kefir_int64_t j = -512; j < 512; j++) {
            ASSERT_AND(&lhs_bigint, &rhs_bigint, i, j);
            ASSERT_AND(&lhs_bigint, &rhs_bigint, (1ull << i % (sizeof(kefir_int64_t) * CHAR_BIT)), j);
            ASSERT_AND(&lhs_bigint, &rhs_bigint, i ^ 0xcafebabebad, j);
        }
    }

#undef ASSERT_AND

    ASSERT_OK(kefir_bigint_free(&kft_mem, &lhs_bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &rhs_bigint));
}
END_CASE

DEFINE_CASE(bigint_or1, "BigInt - bitwise or #1") {
    struct kefir_bigint lhs_bigint, rhs_bigint;

    ASSERT_OK(kefir_bigint_init(&lhs_bigint));
    ASSERT_OK(kefir_bigint_init(&rhs_bigint));

#define ASSERT_OR(_lhs, _rhs, _arg1, _arg2)                                                                        \
    do {                                                                                                           \
        ASSERT_STORE((_lhs), (_arg1));                                                                             \
        ASSERT_STORE((_rhs), (_arg2));                                                                             \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, (_lhs), MAX((_lhs)->bitwidth, (_rhs)->bitwidth) + 1)); \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, (_rhs), (_lhs)->bitwidth));                            \
        ASSERT_OK(kefir_bigint_or((_lhs), (_rhs)));                                                                \
        ASSERT_LOAD((_lhs), ((kefir_int64_t) (_arg1)) | (_arg2));                                                  \
    } while (0)

    ASSERT_OR(&lhs_bigint, &rhs_bigint, 0, 0);
    ASSERT_OR(&lhs_bigint, &rhs_bigint, 1, 0);
    ASSERT_OR(&lhs_bigint, &rhs_bigint, 0, 1);
    ASSERT_OR(&lhs_bigint, &rhs_bigint, 1, 1);
    ASSERT_OR(&lhs_bigint, &rhs_bigint, 1, -1);
    ASSERT_OR(&lhs_bigint, &rhs_bigint, -1, -1);
    ASSERT_OR(&lhs_bigint, &rhs_bigint, KEFIR_INT32_MAX, KEFIR_INT32_MIN);
    ASSERT_OR(&lhs_bigint, &rhs_bigint, KEFIR_INT64_MAX, KEFIR_INT32_MAX);
    ASSERT_OR(&lhs_bigint, &rhs_bigint, KEFIR_INT64_MAX, ~KEFIR_INT32_MIN);

    for (kefir_int64_t i = -512; i < 512; i++) {
        for (kefir_int64_t j = -512; j < 512; j++) {
            ASSERT_OR(&lhs_bigint, &rhs_bigint, i, j);
            ASSERT_OR(&lhs_bigint, &rhs_bigint, (1ull << i % (sizeof(kefir_int64_t) * CHAR_BIT)), j);
            ASSERT_OR(&lhs_bigint, &rhs_bigint, i ^ 0xcafebabebad, j);
        }
    }

#undef ASSERT_OR

    ASSERT_OK(kefir_bigint_free(&kft_mem, &lhs_bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &rhs_bigint));
}
END_CASE

DEFINE_CASE(bigint_xor1, "BigInt - bitwise xor #1") {
    struct kefir_bigint lhs_bigint, rhs_bigint;

    ASSERT_OK(kefir_bigint_init(&lhs_bigint));
    ASSERT_OK(kefir_bigint_init(&rhs_bigint));

#define ASSERT_XOR(_lhs, _rhs, _arg1, _arg2)                                                                       \
    do {                                                                                                           \
        ASSERT_STORE((_lhs), (_arg1));                                                                             \
        ASSERT_STORE((_rhs), (_arg2));                                                                             \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, (_lhs), MAX((_lhs)->bitwidth, (_rhs)->bitwidth) + 1)); \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, (_rhs), (_lhs)->bitwidth));                            \
        ASSERT_OK(kefir_bigint_xor((_lhs), (_rhs)));                                                               \
        ASSERT_LOAD((_lhs), ((kefir_int64_t) (_arg1)) ^ (_arg2));                                                  \
    } while (0)

    ASSERT_XOR(&lhs_bigint, &rhs_bigint, 0, 0);
    ASSERT_XOR(&lhs_bigint, &rhs_bigint, 1, 0);
    ASSERT_XOR(&lhs_bigint, &rhs_bigint, 0, 1);
    ASSERT_XOR(&lhs_bigint, &rhs_bigint, 1, 1);
    ASSERT_XOR(&lhs_bigint, &rhs_bigint, 1, -1);
    ASSERT_XOR(&lhs_bigint, &rhs_bigint, -1, -1);
    ASSERT_XOR(&lhs_bigint, &rhs_bigint, KEFIR_INT32_MAX, KEFIR_INT32_MIN);
    ASSERT_XOR(&lhs_bigint, &rhs_bigint, KEFIR_INT64_MAX, KEFIR_INT32_MAX);
    ASSERT_XOR(&lhs_bigint, &rhs_bigint, KEFIR_INT64_MAX, ~KEFIR_INT32_MIN);

    for (kefir_int64_t i = -512; i < 512; i++) {
        for (kefir_int64_t j = -512; j < 512; j++) {
            ASSERT_XOR(&lhs_bigint, &rhs_bigint, i, j);
            ASSERT_XOR(&lhs_bigint, &rhs_bigint, (1ull << i % (sizeof(kefir_int64_t) * CHAR_BIT)), j);
            ASSERT_XOR(&lhs_bigint, &rhs_bigint, i ^ 0xcafebabebad, j);
        }
    }

#undef ASSERT_OR

    ASSERT_OK(kefir_bigint_free(&kft_mem, &lhs_bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &rhs_bigint));
}
END_CASE

DEFINE_CASE(bigint_negate1, "BigInt - negate #1") {
    struct kefir_bigint bigint;

    ASSERT_OK(kefir_bigint_init(&bigint));

#define ASSERT_NEGATE(_bigint, _arg)                                                                                 \
    do {                                                                                                             \
        ASSERT_STORE((_bigint), (_arg));                                                                             \
        ASSERT_OK(                                                                                                   \
            kefir_bigint_resize_cast_signed(&kft_mem, (_bigint), kefir_bigint_min_native_signed_width((_arg)) + 1)); \
        ASSERT_OK(kefir_bigint_negate((_bigint)));                                                                   \
        ASSERT_LOAD((_bigint), -(kefir_int64_t) (_arg));                                                             \
    } while (0)

    ASSERT_NEGATE(&bigint, 0);
    ASSERT_NEGATE(&bigint, 1);
    ASSERT_NEGATE(&bigint, -1);
    ASSERT_NEGATE(&bigint, KEFIR_UINT8_MIN);
    ASSERT_NEGATE(&bigint, KEFIR_UINT8_MAX);
    ASSERT_NEGATE(&bigint, KEFIR_UINT16_MIN);
    ASSERT_NEGATE(&bigint, KEFIR_UINT16_MAX);
    ASSERT_NEGATE(&bigint, KEFIR_UINT32_MIN);
    ASSERT_NEGATE(&bigint, KEFIR_UINT32_MAX);
    ASSERT_NEGATE(&bigint, KEFIR_UINT64_MIN + 1);
    ASSERT_NEGATE(&bigint, KEFIR_UINT64_MAX);

    for (kefir_int64_t i = -10240; i < 10240; i++) {
        ASSERT_NEGATE(&bigint, i);
        ASSERT_NEGATE(&bigint, (1ull << i % (sizeof(kefir_int64_t) * CHAR_BIT - 1)));
        ASSERT_NEGATE(&bigint, i ^ 0xcafebabebad);
    }

#undef ASSERT_NEGATE

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
}
END_CASE

DEFINE_CASE(bigint_sub1, "BigInt - subtraction #1") {
    struct kefir_bigint lhs_bigint, rhs_bigint;

    ASSERT_OK(kefir_bigint_init(&lhs_bigint));
    ASSERT_OK(kefir_bigint_init(&rhs_bigint));

#define ASSERT_SUB(_lhs, _rhs, _arg1, _arg2, _res)                                                                 \
    do {                                                                                                           \
        ASSERT_STORE((_lhs), (_arg1));                                                                             \
        ASSERT_STORE((_rhs), (_arg2));                                                                             \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, (_lhs), MAX((_lhs)->bitwidth, (_rhs)->bitwidth) + 1)); \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, (_rhs), (_lhs)->bitwidth));                            \
        ASSERT_OK(kefir_bigint_subtract((_lhs), (_rhs)));                                                          \
        ASSERT_LOAD((_lhs), (_res));                                                                               \
    } while (0)

    ASSERT_SUB(&lhs_bigint, &rhs_bigint, 0, 0, 0);
    ASSERT_SUB(&lhs_bigint, &rhs_bigint, 1000, 2000, -1000);
    ASSERT_SUB(&lhs_bigint, &rhs_bigint, 1000, -2000, 3000);
    ASSERT_SUB(&lhs_bigint, &rhs_bigint, KEFIR_UINT32_MAX, KEFIR_UINT32_MAX, 0);
    ASSERT_SUB(&lhs_bigint, &rhs_bigint, KEFIR_INT32_MIN, KEFIR_INT32_MIN, 0);
    ASSERT_SUB(&lhs_bigint, &rhs_bigint, KEFIR_INT32_MIN, KEFIR_INT32_MAX,
               (kefir_int64_t) KEFIR_INT32_MIN - KEFIR_INT32_MAX);
    ASSERT_SUB(&lhs_bigint, &rhs_bigint, KEFIR_UINT64_MIN, KEFIR_UINT64_MAX, 1);
    ASSERT_SUB(&lhs_bigint, &rhs_bigint, 0, KEFIR_INT64_MIN, KEFIR_INT64_MIN);

    for (kefir_int64_t i = -1024; i < 1024; i++) {
        for (kefir_int64_t j = -256; j < 256; j++) {
            ASSERT_SUB(&lhs_bigint, &rhs_bigint, i, j, i - j);
            ASSERT_SUB(&lhs_bigint, &rhs_bigint, (1ull << i % (sizeof(kefir_int64_t) * CHAR_BIT)), j,
                       (kefir_int64_t) ((1ull << i % (sizeof(kefir_int64_t) * CHAR_BIT)) - j));
            ASSERT_SUB(&lhs_bigint, &rhs_bigint, i ^ 0xcafebabebad, j, (i ^ 0xcafebabebad) - j);
        }
    }

#undef ASSERT_SUB

    ASSERT_STORE(&lhs_bigint, KEFIR_INT64_MIN);
    ASSERT_STORE(&rhs_bigint, KEFIR_INT64_MAX);
    ASSERT_OK(
        kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_subtract(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, 1);
    ASSERT_OK(
        kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_subtract(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, -KEFIR_INT64_MAX + 1);
    ASSERT_OK(
        kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_subtract(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, 3);
    ASSERT_OK(
        kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_subtract(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, -KEFIR_INT64_MAX + 3);
    ASSERT_OK(
        kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_subtract(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, 5);

    ASSERT_STORE(&lhs_bigint, 0);
    ASSERT_STORE(&rhs_bigint, KEFIR_UINT64_MAX);
    ASSERT_OK(
        kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_subtract(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, (kefir_int64_t) 1);
    ASSERT_OK(
        kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_subtract(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, (kefir_int64_t) 2);
    ASSERT_OK(
        kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_subtract(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, (kefir_int64_t) 3);
    ASSERT_OK(
        kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_subtract(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, (kefir_int64_t) 4);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &lhs_bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &rhs_bigint));
}
END_CASE

DEFINE_CASE(bigint_lshift1, "BigInt - left shift #1") {
    struct kefir_bigint lhs_bigint;

    ASSERT_OK(kefir_bigint_init(&lhs_bigint));

#define ASSERT_SHL(_lhs, _arg1, _arg2, _res)                                                            \
    do {                                                                                                \
        ASSERT_STORE((_lhs), (_arg1));                                                                  \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, (_lhs), sizeof(kefir_int64_t) * CHAR_BIT)); \
        ASSERT_OK(kefir_bigint_left_shift((_lhs), (_arg2)));                                            \
        ASSERT_LOAD((_lhs), (_res));                                                                    \
    } while (0)

    ASSERT_SHL(&lhs_bigint, 0, 0, 0);
    ASSERT_SHL(&lhs_bigint, 0, 1, 0);
    for (kefir_uint64_t i = 0; i < sizeof(kefir_int64_t) * CHAR_BIT - 1; i++) {
        ASSERT_SHL(&lhs_bigint, 1, i, 1ll << i);
    }
    ASSERT_SHL(&lhs_bigint, 1, sizeof(kefir_int64_t) * CHAR_BIT - 1, KEFIR_INT64_MIN);
    ASSERT_SHL(&lhs_bigint, 1, sizeof(kefir_int64_t) * CHAR_BIT, 0);

    ASSERT_SHL(&lhs_bigint, 7, 1, 14);
    ASSERT_SHL(&lhs_bigint, 7, 2, 28);
    ASSERT_SHL(&lhs_bigint, 15, 2, 60);
    ASSERT_SHL(&lhs_bigint, KEFIR_INT64_MIN, 0, KEFIR_INT64_MIN);
    ASSERT_SHL(&lhs_bigint, -1, 1, -2);
    ASSERT_SHL(&lhs_bigint, -1, 2, -4);
    ASSERT_SHL(&lhs_bigint, KEFIR_INT64_MAX, 1, (kefir_int64_t) 18446744073709551614ull);

#undef ASSERT_SHL

    ASSERT_OK(kefir_bigint_free(&kft_mem, &lhs_bigint));
}
END_CASE

DEFINE_CASE(bigint_rshift1, "BigInt - right shift #1") {
    struct kefir_bigint lhs_bigint;

    ASSERT_OK(kefir_bigint_init(&lhs_bigint));

#define ASSERT_SHR(_lhs, _arg1, _arg2, _res)                                                            \
    do {                                                                                                \
        ASSERT_STORE((_lhs), (_arg1));                                                                  \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, (_lhs), sizeof(kefir_int64_t) * CHAR_BIT)); \
        ASSERT_OK(kefir_bigint_right_shift((_lhs), (_arg2)));                                           \
        ASSERT_LOAD((_lhs), (_res));                                                                    \
    } while (0)

    ASSERT_SHR(&lhs_bigint, 0, 0, 0);
    ASSERT_SHR(&lhs_bigint, 0, 1, 0);
    ASSERT_SHR(&lhs_bigint, 1, 0, 1);
    ASSERT_SHR(&lhs_bigint, 1, 1, 0);
    for (kefir_uint64_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT; i++) {
        ASSERT_SHR(&lhs_bigint, 1ull << i, i, 1);
        if (i > 0) {
            ASSERT_SHR(&lhs_bigint, 1ull << i, i - 1, 2);
        }
    }
    ASSERT_SHR(&lhs_bigint, -1ll, 0, -1);
    ASSERT_SHR(&lhs_bigint, -1ll, 1, KEFIR_INT64_MAX);
    ASSERT_SHR(&lhs_bigint, -1ll, 2, KEFIR_INT64_MAX / 2);
    ASSERT_SHR(&lhs_bigint, KEFIR_INT64_MIN, 1, 1ll << (sizeof(kefir_uint64_t) * CHAR_BIT - 2));
    ASSERT_SHR(&lhs_bigint, -1ll, sizeof(kefir_uint64_t) * CHAR_BIT, 0);
    ASSERT_SHR(&lhs_bigint, -1ll, sizeof(kefir_uint64_t) * CHAR_BIT - 1, 1);
    ASSERT_SHR(&lhs_bigint, KEFIR_INT64_MAX, 1, (1ll << (sizeof(kefir_uint64_t) * CHAR_BIT - 2)) - 1);

#undef ASSERT_SHR

    ASSERT_OK(kefir_bigint_free(&kft_mem, &lhs_bigint));
}
END_CASE

DEFINE_CASE(bigint_combined_shift1, "BigInt - combined shifts #1") {
    struct kefir_bigint lhs_bigint;

    ASSERT_OK(kefir_bigint_init(&lhs_bigint));

    ASSERT_STORE(&lhs_bigint, 0xbadbabell);
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, 1024));
    ASSERT_LOAD(&lhs_bigint, 0xbadbabell);

    ASSERT_OK(kefir_bigint_left_shift(&lhs_bigint, 824));
    ASSERT_LOAD(&lhs_bigint, 0);
    ASSERT_OK(kefir_bigint_right_shift(&lhs_bigint, 823));
    ASSERT_LOAD(&lhs_bigint, 0xbadbabell << 1);
    ASSERT_OK(kefir_bigint_left_shift(&lhs_bigint, 823));
    ASSERT_OK(kefir_bigint_right_shift(&lhs_bigint, 824));
    ASSERT_LOAD(&lhs_bigint, 0xbadbabell);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &lhs_bigint));
}
END_CASE

DEFINE_CASE(bigint_arshift1, "BigInt - arithmetic right shift #1") {
    struct kefir_bigint lhs_bigint;

    ASSERT_OK(kefir_bigint_init(&lhs_bigint));

#define ASSERT_SAR(_lhs, _arg1, _arg2, _res)                                                            \
    do {                                                                                                \
        ASSERT_STORE((_lhs), (_arg1));                                                                  \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, (_lhs), sizeof(kefir_int64_t) * CHAR_BIT)); \
        ASSERT_OK(kefir_bigint_arithmetic_right_shift((_lhs), (_arg2)));                                \
        ASSERT_LOAD((_lhs), (_res));                                                                    \
    } while (0)

    ASSERT_SAR(&lhs_bigint, 0, 0, 0);
    ASSERT_SAR(&lhs_bigint, 0, 1, 0);
    ASSERT_SAR(&lhs_bigint, 1, 0, 1);
    ASSERT_SAR(&lhs_bigint, 1, 1, 0);

    for (kefir_size_t i = 0; i < 4 * sizeof(kefir_int64_t) * CHAR_BIT; i++) {
        ASSERT_SAR(&lhs_bigint, -1, i, -1);
    }

    for (kefir_uint64_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT - 1; i++) {
        ASSERT_SAR(&lhs_bigint, 1ull << i, i, 1);
    }
    ASSERT_SAR(&lhs_bigint, 1ull << (sizeof(kefir_uint64_t) * CHAR_BIT - 1), (sizeof(kefir_uint64_t) * CHAR_BIT - 1),
               -1);

#undef ASSERT_SHR

    ASSERT_STORE(&lhs_bigint, 1);
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, sizeof(kefir_int64_t) * CHAR_BIT));
    ASSERT_OK(kefir_bigint_left_shift(&lhs_bigint, sizeof(kefir_uint64_t) * CHAR_BIT - 2));
    ASSERT_OK(kefir_bigint_arithmetic_right_shift(&lhs_bigint, sizeof(kefir_uint64_t) * CHAR_BIT - 2));
    ASSERT_LOAD(&lhs_bigint, 1);
    ASSERT_OK(kefir_bigint_left_shift(&lhs_bigint, sizeof(kefir_uint64_t) * CHAR_BIT - 1));
    ASSERT_OK(kefir_bigint_arithmetic_right_shift(&lhs_bigint, sizeof(kefir_uint64_t) * CHAR_BIT - 1));
    ASSERT_LOAD(&lhs_bigint, -1);

    ASSERT_STORE(&lhs_bigint, 8);
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, 1024));
    ASSERT_LOAD(&lhs_bigint, 8);

    ASSERT_OK(kefir_bigint_left_shift(&lhs_bigint, 1019));
    ASSERT_LOAD(&lhs_bigint, 0);
    ASSERT_OK(kefir_bigint_arithmetic_right_shift(&lhs_bigint, 1019));
    ASSERT_LOAD(&lhs_bigint, 8);
    ASSERT_OK(kefir_bigint_left_shift(&lhs_bigint, 1020));
    ASSERT_OK(kefir_bigint_arithmetic_right_shift(&lhs_bigint, 1020));
    ASSERT_LOAD(&lhs_bigint, -8);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &lhs_bigint));
}
END_CASE

DEFINE_CASE(bigint_min_signed_width1, "BigInt - minimum signed width #1") {
    ASSERT(kefir_bigint_min_native_signed_width(0) == 1);
    ASSERT(kefir_bigint_min_native_signed_width(-1) == 1);
    ASSERT(kefir_bigint_min_native_signed_width(1) == 2);
    for (kefir_size_t i = 0; i < sizeof(kefir_int64_t) * CHAR_BIT - 1; i++) {
        ASSERT(kefir_bigint_min_native_signed_width(1ull << i) == i + 2);
        ASSERT(kefir_bigint_min_native_signed_width(-(1ull << i)) == i + 1);
    }
    ASSERT(kefir_bigint_min_native_signed_width(1ull << (sizeof(kefir_int64_t) * CHAR_BIT - 1)) ==
           sizeof(kefir_int64_t) * CHAR_BIT);
    ASSERT(kefir_bigint_min_native_signed_width(KEFIR_INT8_MAX) == sizeof(kefir_int8_t) * CHAR_BIT);
    ASSERT(kefir_bigint_min_native_signed_width(KEFIR_INT8_MIN) == sizeof(kefir_int8_t) * CHAR_BIT);
    ASSERT(kefir_bigint_min_native_signed_width(KEFIR_INT16_MAX) == sizeof(kefir_int16_t) * CHAR_BIT);
    ASSERT(kefir_bigint_min_native_signed_width(KEFIR_INT16_MIN) == sizeof(kefir_int16_t) * CHAR_BIT);
    ASSERT(kefir_bigint_min_native_signed_width(KEFIR_INT32_MAX) == sizeof(kefir_int32_t) * CHAR_BIT);
    ASSERT(kefir_bigint_min_native_signed_width(KEFIR_INT32_MIN) == sizeof(kefir_int32_t) * CHAR_BIT);
    ASSERT(kefir_bigint_min_native_signed_width(KEFIR_INT64_MAX) == sizeof(kefir_int64_t) * CHAR_BIT);
    ASSERT(kefir_bigint_min_native_signed_width(KEFIR_INT64_MIN) == sizeof(kefir_int64_t) * CHAR_BIT);
}
END_CASE

DEFINE_CASE(bigint_min_unsigned_width1, "BigInt - minimum unsigned width #1") {
    ASSERT(kefir_bigint_min_native_unsigned_width(0) == 1);
    ASSERT(kefir_bigint_min_native_unsigned_width(1) == 1);
    for (kefir_size_t i = 0; i < sizeof(kefir_int64_t) * CHAR_BIT; i++) {
        ASSERT(kefir_bigint_min_native_unsigned_width(1ull << i) == (i + 1));
    }
    ASSERT(kefir_bigint_min_native_unsigned_width(KEFIR_UINT8_MAX) == sizeof(kefir_uint8_t) * CHAR_BIT);
    ASSERT(kefir_bigint_min_native_unsigned_width(KEFIR_UINT16_MAX) == sizeof(kefir_uint16_t) * CHAR_BIT);
    ASSERT(kefir_bigint_min_native_unsigned_width(KEFIR_UINT32_MAX) == sizeof(kefir_uint32_t) * CHAR_BIT);
    ASSERT(kefir_bigint_min_native_unsigned_width(KEFIR_UINT64_MAX) == sizeof(kefir_uint64_t) * CHAR_BIT);
}
END_CASE

DEFINE_CASE(bigint_unsigned_multiply1, "BigInt - unsigned multiplication #1") {
    struct kefir_bigint lhs_bigint, rhs_bigint, result_bigint, tmp_bigint;

    ASSERT_OK(kefir_bigint_init(&lhs_bigint));
    ASSERT_OK(kefir_bigint_init(&rhs_bigint));
    ASSERT_OK(kefir_bigint_init(&result_bigint));
    ASSERT_OK(kefir_bigint_init(&tmp_bigint));

#define ASSERT_MUL(_arg1, _arg2, _res)                                                                                 \
    do {                                                                                                               \
        ASSERT_STORE(&lhs_bigint, (_arg1));                                                                            \
        ASSERT_STORE(&rhs_bigint, (_arg2));                                                                            \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint,                                               \
                                                  MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));                 \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));                        \
        ASSERT_OK(                                                                                                     \
            kefir_bigint_resize_cast_signed(&kft_mem, &result_bigint, lhs_bigint.bitwidth + rhs_bigint.bitwidth + 1)); \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &tmp_bigint, result_bigint.bitwidth));                     \
        ASSERT_OK(kefir_bigint_unsigned_multiply(&result_bigint, &lhs_bigint, &rhs_bigint, &tmp_bigint));              \
        ASSERT_LOAD(&result_bigint, (_res));                                                                           \
    } while (0)

    for (kefir_size_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT; i++) {
        ASSERT_MUL(1ull << i, 0, 0);
        ASSERT_MUL(0, 1ull << i, 0);
        ASSERT_MUL(1ull << i, 1, (kefir_int64_t) (1ull << i));
        ASSERT_MUL(1, 1ull << i, (kefir_int64_t) (1ull << i));

        if (i < (sizeof(kefir_uint64_t) * CHAR_BIT) / 2) {
            for (kefir_size_t j = 0; j < (sizeof(kefir_uint64_t) * CHAR_BIT) / 2; j++) {
                ASSERT_MUL(1ull << i, 1ull << j, (kefir_int64_t) (1ull << (i + j)));
            }
        }
    }

    for (kefir_int64_t i = 0; i < 4096; i++) {
        for (kefir_int64_t j = 0; j < 512; j++) {
            ASSERT_MUL(i, j, i * j);
        }
    }

    ASSERT_MUL(KEFIR_UINT16_MAX, ((kefir_uint64_t) KEFIR_UINT16_MAX) + 1,
               (kefir_int64_t) (((kefir_uint64_t) KEFIR_UINT16_MAX) << 16));
    ASSERT_MUL(KEFIR_UINT16_MAX, ((kefir_uint64_t) KEFIR_UINT32_MAX) + 1,
               (kefir_int64_t) (((kefir_uint64_t) KEFIR_UINT16_MAX) << 32));
    ASSERT_MUL(KEFIR_UINT32_MAX, ((kefir_uint64_t) KEFIR_UINT16_MAX) + 1,
               (kefir_int64_t) (((kefir_uint64_t) KEFIR_UINT32_MAX) << 16));
    ASSERT_MUL(KEFIR_UINT32_MAX, ((kefir_uint64_t) KEFIR_UINT32_MAX) + 1,
               (kefir_int64_t) (((kefir_uint64_t) KEFIR_UINT32_MAX) << 32));

#undef ASSERT_MUL

    ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &lhs_bigint, 2 * sizeof(kefir_uint64_t) * CHAR_BIT + 2));
    ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &result_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &tmp_bigint, result_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_set_unsigned_value(&lhs_bigint, KEFIR_UINT64_MAX));
    ASSERT_OK(kefir_bigint_set_unsigned_value(&rhs_bigint, KEFIR_UINT64_MAX));
    ASSERT_OK(kefir_bigint_unsigned_multiply(&result_bigint, &lhs_bigint, &rhs_bigint, &tmp_bigint));
    ASSERT_OK(kefir_bigint_add(&result_bigint, &lhs_bigint));
    ASSERT_LOAD(&result_bigint, 0);
    ASSERT_OK(kefir_bigint_right_shift(&result_bigint, sizeof(kefir_uint64_t) * CHAR_BIT));
    ASSERT_LOAD(&result_bigint, -1);
    ASSERT_OK(kefir_bigint_right_shift(&result_bigint, (sizeof(kefir_int64_t) * CHAR_BIT) / 2));
    ASSERT_LOAD(&result_bigint, (kefir_int64_t) KEFIR_UINT32_MAX);
    ASSERT_OK(kefir_bigint_copy(&lhs_bigint, &result_bigint));
    ASSERT_OK(kefir_bigint_unsigned_multiply(&result_bigint, &lhs_bigint, &rhs_bigint, &tmp_bigint));
    ASSERT_OK(kefir_bigint_add(&result_bigint, &rhs_bigint));
    ASSERT_LOAD(&result_bigint, (kefir_int64_t) (((kefir_uint64_t) -1ll) ^ KEFIR_UINT32_MAX));
    ASSERT_OK(kefir_bigint_right_shift(&result_bigint, sizeof(kefir_uint32_t) * CHAR_BIT));
    ASSERT_LOAD(&result_bigint, -1);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &result_bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &lhs_bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &rhs_bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &tmp_bigint));
}
END_CASE

DEFINE_CASE(bigint_signed_multiply1, "BigInt - signed multiplication #1") {
    struct kefir_bigint lhs_bigint, rhs_bigint, result_bigint, acc_bigint;

    ASSERT_OK(kefir_bigint_init(&lhs_bigint));
    ASSERT_OK(kefir_bigint_init(&rhs_bigint));
    ASSERT_OK(kefir_bigint_init(&result_bigint));
    ASSERT_OK(kefir_bigint_init(&acc_bigint));

#define ASSERT_MUL(_arg1, _arg2, _res)                                                                                 \
    do {                                                                                                               \
        ASSERT_STORE(&lhs_bigint, (_arg1));                                                                            \
        ASSERT_STORE(&rhs_bigint, (_arg2));                                                                            \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint,                                               \
                                                  MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));                 \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));                        \
        ASSERT_OK(                                                                                                     \
            kefir_bigint_resize_cast_signed(&kft_mem, &result_bigint, lhs_bigint.bitwidth + rhs_bigint.bitwidth + 1)); \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &acc_bigint, result_bigint.bitwidth));                     \
        ASSERT_OK(kefir_bigint_signed_multiply(&result_bigint, &lhs_bigint, &rhs_bigint, &acc_bigint));                \
        ASSERT_LOAD(&result_bigint, (_res));                                                                           \
    } while (0)

    for (kefir_size_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT; i++) {
        ASSERT_MUL(1ull << i, 0, 0);
        ASSERT_MUL(0, 1ull << i, 0);
        ASSERT_MUL(1ull << i, 1, (kefir_int64_t) (1ull << i));
        ASSERT_MUL(1, 1ull << i, (kefir_int64_t) (1ull << i));

        if (i < (sizeof(kefir_uint64_t) * CHAR_BIT) / 2) {
            for (kefir_size_t j = 0; j < (sizeof(kefir_uint64_t) * CHAR_BIT) / 2; j++) {
                ASSERT_MUL(1ull << i, 1ull << j, (kefir_int64_t) (1ull << (i + j)));
            }
        }
    }

    for (kefir_int64_t i = -4096; i < 4096; i++) {
        for (kefir_int64_t j = -512; j < 512; j += 4) {
            ASSERT_MUL(i, j, i * j);
        }
    }

    ASSERT_MUL(KEFIR_UINT16_MAX, ((kefir_uint64_t) KEFIR_UINT16_MAX) + 1,
               (kefir_int64_t) (((kefir_uint64_t) KEFIR_UINT16_MAX) << 16));
    ASSERT_MUL(KEFIR_UINT16_MAX, ((kefir_uint64_t) KEFIR_UINT32_MAX) + 1,
               (kefir_int64_t) (((kefir_uint64_t) KEFIR_UINT16_MAX) << 32));
    ASSERT_MUL(KEFIR_UINT32_MAX, ((kefir_uint64_t) KEFIR_UINT16_MAX) + 1,
               (kefir_int64_t) (((kefir_uint64_t) KEFIR_UINT32_MAX) << 16));
    ASSERT_MUL(KEFIR_UINT32_MAX, ((kefir_uint64_t) KEFIR_UINT32_MAX) + 1,
               (kefir_int64_t) (((kefir_uint64_t) KEFIR_UINT32_MAX) << 32));

    ASSERT_MUL(KEFIR_INT16_MAX, ((kefir_uint64_t) KEFIR_UINT16_MAX) + 1,
               (kefir_int64_t) (((kefir_uint64_t) KEFIR_INT16_MAX) << 16));
    ASSERT_MUL(KEFIR_INT16_MAX, ((kefir_uint64_t) KEFIR_UINT32_MAX) + 1,
               (kefir_int64_t) (((kefir_uint64_t) KEFIR_INT16_MAX) << 32));
    ASSERT_MUL(KEFIR_INT32_MAX, ((kefir_uint64_t) KEFIR_UINT16_MAX) + 1,
               (kefir_int64_t) (((kefir_uint64_t) KEFIR_INT32_MAX) << 16));
    ASSERT_MUL(KEFIR_INT32_MAX, ((kefir_uint64_t) KEFIR_UINT32_MAX) + 1,
               (kefir_int64_t) (((kefir_uint64_t) KEFIR_INT32_MAX) << 32));

    ASSERT_MUL(KEFIR_INT8_MIN, -1, -(kefir_int64_t) KEFIR_INT8_MIN);
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &result_bigint, 8));
    ASSERT_LOAD(&result_bigint, KEFIR_INT8_MIN);

    ASSERT_MUL(KEFIR_INT16_MIN, -1, -(kefir_int64_t) KEFIR_INT16_MIN);
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &result_bigint, 16));
    ASSERT_LOAD(&result_bigint, KEFIR_INT16_MIN);

    ASSERT_MUL(KEFIR_INT32_MIN, -1, -(kefir_int64_t) KEFIR_INT32_MIN);
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &result_bigint, 32));
    ASSERT_LOAD(&result_bigint, KEFIR_INT32_MIN);

    ASSERT_MUL(KEFIR_INT64_MIN, -1, KEFIR_INT64_MIN);
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &result_bigint, 64));
    ASSERT_LOAD(&result_bigint, KEFIR_INT64_MIN);

#undef ASSERT_MUL

    ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &lhs_bigint, 2 * sizeof(kefir_int64_t) * CHAR_BIT + 2));
    ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &result_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &acc_bigint, result_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_set_signed_value(&lhs_bigint, KEFIR_INT64_MIN));
    ASSERT_OK(kefir_bigint_set_signed_value(&rhs_bigint, KEFIR_INT64_MIN));
    ASSERT_OK(kefir_bigint_signed_multiply(&result_bigint, &lhs_bigint, &rhs_bigint, &acc_bigint));
    ASSERT_LOAD(&result_bigint, 0);
    ASSERT_OK(kefir_bigint_arithmetic_right_shift(&result_bigint, sizeof(kefir_int64_t) * CHAR_BIT));
    ASSERT_LOAD(&result_bigint, -(KEFIR_INT64_MIN / 2));
    ASSERT_OK(kefir_bigint_copy(&lhs_bigint, &result_bigint));
    ASSERT_OK(kefir_bigint_set_signed_value(&rhs_bigint, KEFIR_INT32_MIN));
    ASSERT_OK(kefir_bigint_signed_multiply(&result_bigint, &lhs_bigint, &rhs_bigint, &acc_bigint));
    ASSERT_LOAD(&result_bigint, 0);
    ASSERT_OK(kefir_bigint_arithmetic_right_shift(&result_bigint, sizeof(kefir_int64_t) * CHAR_BIT));
    ASSERT_LOAD(&result_bigint, KEFIR_INT32_MIN / 4);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &result_bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &lhs_bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &rhs_bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &acc_bigint));
}
END_CASE

DEFINE_CASE(bigint_unsigned_divide1, "BigInt - unsigned division #1") {
    struct kefir_bigint lhs_bigint, rhs_bigint, remainder_bigint;

    ASSERT_OK(kefir_bigint_init(&lhs_bigint));
    ASSERT_OK(kefir_bigint_init(&rhs_bigint));
    ASSERT_OK(kefir_bigint_init(&remainder_bigint));

#define ASSERT_DIV(_arg1, _arg2, _res, _rem)                                                             \
    do {                                                                                                 \
        ASSERT_USTORE(&lhs_bigint, (_arg1));                                                             \
        ASSERT_USTORE(&rhs_bigint, (_arg2));                                                             \
        ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &lhs_bigint,                               \
                                                    MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1)); \
        ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));        \
        ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &remainder_bigint, lhs_bigint.bitwidth));         \
        ASSERT_OK(kefir_bigint_unsigned_divide(&lhs_bigint, &remainder_bigint, &rhs_bigint));            \
        ASSERT_ULOAD(&lhs_bigint, (_res));                                                               \
        ASSERT_ULOAD(&remainder_bigint, (_rem));                                                         \
    } while (0)

    for (kefir_uint64_t i = 0; i < 4096; i++) {
        ASSERT_USTORE(&lhs_bigint, i);
        ASSERT_USTORE(&rhs_bigint, 0);
        ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &lhs_bigint,
                                                    MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
        ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
        ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &remainder_bigint, lhs_bigint.bitwidth));
        ASSERT(kefir_bigint_unsigned_divide(&lhs_bigint, &remainder_bigint, &rhs_bigint) == KEFIR_INVALID_REQUEST);

        for (kefir_uint64_t j = 1; j < 512; j++) {
            ASSERT_DIV(i, j, i / j, i % j);
        }
    }

    for (kefir_uint64_t i = 0; i < 100; i++) {
        for (kefir_uint64_t j = 0; j < i; j++) {
            ASSERT_DIV(j, i, j / i, j % i);
        }
    }

    for (kefir_uint64_t i = 1; i < sizeof(kefir_uint64_t) * CHAR_BIT; i++) {
        ASSERT_DIV(1ull << i, 1ull << (i - 1), 2, 0);
        if (i > 2) {
            ASSERT_DIV((1ull << i) + 1, 1ull << (i - 2), 4, 1);
        }
    }

    for (kefir_uint64_t i = ~(kefir_uint64_t) 0ull; i != 0; i >>= 1) {
        ASSERT_DIV(i, 1, i, 0);
        ASSERT_DIV(i, i, 1, 0);
        ASSERT_DIV(i, 2, i >> 1, 1);
    }
    for (kefir_uint64_t i = ~(kefir_uint64_t) 0ull; i != 0; i >>= CHAR_BIT) {
        ASSERT_DIV(i, 0x10, i >> 4, 0xf);
    }

    ASSERT_DIV(KEFIR_UINT8_MAX, KEFIR_UINT8_MAX, 1, 0);
    ASSERT_DIV(KEFIR_UINT8_MAX - 1, KEFIR_UINT8_MAX, 0, KEFIR_UINT8_MAX - 1);
    ASSERT_DIV(3 * KEFIR_UINT8_MAX + 1, KEFIR_UINT8_MAX, 3, 1);
    ASSERT_DIV(3 * KEFIR_UINT8_MAX - 1, KEFIR_UINT8_MAX, 2, KEFIR_UINT8_MAX - 1);

    ASSERT_DIV(KEFIR_UINT16_MAX, KEFIR_UINT16_MAX, 1, 0);
    ASSERT_DIV(KEFIR_UINT16_MAX - 1, KEFIR_UINT16_MAX, 0, KEFIR_UINT16_MAX - 1);
    ASSERT_DIV(3 * KEFIR_UINT16_MAX + 1, KEFIR_UINT16_MAX, 3, 1);
    ASSERT_DIV(3 * KEFIR_UINT16_MAX - 1, KEFIR_UINT16_MAX, 2, KEFIR_UINT16_MAX - 1);

    ASSERT_DIV(KEFIR_UINT32_MAX, KEFIR_UINT32_MAX, 1, 0);
    ASSERT_DIV(KEFIR_UINT32_MAX - 1, KEFIR_UINT32_MAX, 0, KEFIR_UINT32_MAX - 1);
    ASSERT_DIV(3 * ((kefir_uint64_t) KEFIR_UINT32_MAX) + 1, KEFIR_UINT32_MAX, 3, 1);
    ASSERT_DIV(3 * ((kefir_uint64_t) KEFIR_UINT32_MAX) - 1, KEFIR_UINT32_MAX, 2, KEFIR_UINT32_MAX - 1);

    ASSERT_DIV(KEFIR_UINT64_MAX, KEFIR_UINT32_MAX, 2 + (kefir_uint64_t) KEFIR_UINT32_MAX, 0);

#undef ASSERT_DIV

    ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &lhs_bigint, 2 * sizeof(kefir_int64_t) * CHAR_BIT + 2));
    ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &remainder_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_set_signed_value(&lhs_bigint, 0xbadbabe));
    ASSERT_OK(kefir_bigint_set_signed_value(&rhs_bigint, 1ull << 32));
    ASSERT_OK(kefir_bigint_left_shift(&lhs_bigint, 96));
    ASSERT_ULOAD(&lhs_bigint, 0);
    ASSERT_ULOAD(&remainder_bigint, 0);
    ASSERT_OK(kefir_bigint_unsigned_divide(&lhs_bigint, &remainder_bigint, &rhs_bigint));
    ASSERT_ULOAD(&lhs_bigint, 0);
    ASSERT_ULOAD(&remainder_bigint, 0);
    ASSERT_OK(kefir_bigint_unsigned_divide(&lhs_bigint, &remainder_bigint, &rhs_bigint));
    ASSERT_ULOAD(&lhs_bigint, 0xbadbabeull << 32);
    ASSERT_ULOAD(&remainder_bigint, 0);
    ASSERT_OK(kefir_bigint_unsigned_divide(&lhs_bigint, &remainder_bigint, &rhs_bigint));
    ASSERT_ULOAD(&lhs_bigint, 0xbadbabeull);
    ASSERT_ULOAD(&remainder_bigint, 0);
    ASSERT_OK(kefir_bigint_unsigned_divide(&lhs_bigint, &remainder_bigint, &rhs_bigint));
    ASSERT_ULOAD(&lhs_bigint, 0);
    ASSERT_ULOAD(&remainder_bigint, 0xbadbabe);

    ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &lhs_bigint, 1024));
    ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &rhs_bigint, sizeof(kefir_uint16_t) * CHAR_BIT));
    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &remainder_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_set_unsigned_value(&lhs_bigint, 0));
    ASSERT_OK(kefir_bigint_invert(&lhs_bigint));
    ASSERT_OK(kefir_bigint_set_unsigned_value(&rhs_bigint, 0x1000));
    ASSERT_OK(kefir_bigint_unsigned_divide(&lhs_bigint, &remainder_bigint, &rhs_bigint));
    ASSERT_ULOAD(&lhs_bigint, (kefir_uint64_t) ~0ull);
    ASSERT_ULOAD(&remainder_bigint, 0xfff);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &lhs_bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &rhs_bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &remainder_bigint));
}
END_CASE

DEFINE_CASE(bigint_signed_divide1, "BigInt - signed division #1") {
    struct kefir_bigint lhs_bigint, rhs_bigint, remainder_bigint;

    ASSERT_OK(kefir_bigint_init(&lhs_bigint));
    ASSERT_OK(kefir_bigint_init(&rhs_bigint));
    ASSERT_OK(kefir_bigint_init(&remainder_bigint));

#define ASSERT_DIV(_arg1, _arg2, _res, _rem)                                                           \
    do {                                                                                               \
        ASSERT_STORE(&lhs_bigint, (_arg1));                                                            \
        ASSERT_STORE(&rhs_bigint, (_arg2));                                                            \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint,                               \
                                                  MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1)); \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));        \
        ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &remainder_bigint, lhs_bigint.bitwidth));       \
        ASSERT_OK(kefir_bigint_signed_divide(&lhs_bigint, &remainder_bigint, &rhs_bigint));            \
        ASSERT_LOAD(&lhs_bigint, (_res));                                                              \
        ASSERT_LOAD(&remainder_bigint, (_rem));                                                        \
    } while (0)

    for (kefir_int64_t i = -4096; i < 4096; i++) {
        for (kefir_int64_t j = -512; j < 512; j += 4) {
            if (j != 0) {
                ASSERT_DIV(i, j, i / j, i % j);
            } else {
                ASSERT_STORE(&lhs_bigint, i);
                ASSERT_STORE(&rhs_bigint, j);
                ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint,
                                                          MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
                ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
                ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &remainder_bigint, lhs_bigint.bitwidth));
                ASSERT(kefir_bigint_signed_divide(&lhs_bigint, &remainder_bigint, &rhs_bigint) ==
                       KEFIR_INVALID_REQUEST);
            }
        }
    }
    for (kefir_uint64_t i = 1; i < sizeof(kefir_int64_t) * CHAR_BIT - 1; i++) {
        ASSERT_DIV(1ll << i, 1ll << (i - 1), 2, 0);
        ASSERT_DIV(1ll << i, -(1ll << (i - 1)), -2, 0);
        ASSERT_DIV(-(1ll << i), -(1ll << (i - 1)), 2, 0);
        ASSERT_DIV(-(1ll << i), 1ll << (i - 1), -2, 0);
        if (i > 2) {
            ASSERT_DIV((1ll << i) + 1, 1ll << (i - 2), 4, 1);
            ASSERT_DIV((1ll << i) + 1, -(1ll << (i - 2)), -4, 1);
            ASSERT_DIV(-((1ll << i) + 1), -(1ll << (i - 2)), 4, -1);
            ASSERT_DIV(-((1ll << i) + 1), 1ll << (i - 2), -4, -1);
        }
    }

    for (kefir_int64_t i = ((1ull << (sizeof(kefir_uint64_t) * CHAR_BIT - 1)) - 1); i != 0; i >>= 1) {
        ASSERT_DIV(i, 1, i, 0);
        ASSERT_DIV(i, i, 1, 0);
        ASSERT_DIV(i, 2, i >> 1, 1);

        ASSERT_DIV(i, -1, -i, 0);
        ASSERT_DIV(i, -i, -1, 0);
        ASSERT_DIV(i, -2, -(i >> 1), 1);

        ASSERT_DIV(-i, -1, i, 0);
        ASSERT_DIV(-i, -i, 1, 0);
        ASSERT_DIV(-i, -2, (i >> 1), -1);

        ASSERT_DIV(-i, 1, -i, 0);
        ASSERT_DIV(-i, i, -1, 0);
        ASSERT_DIV(-i, 2, -(i >> 1), -1);
    }

#undef ASSERT_DIV

#define ASSERT_DIV(_arg1, _arg2, _res, _rem)                                                                        \
    do {                                                                                                            \
        ASSERT_STORE(&lhs_bigint, (_arg1));                                                                         \
        ASSERT_STORE(&rhs_bigint, (_arg2));                                                                         \
        ASSERT_OK(                                                                                                  \
            kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth))); \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));                     \
        ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &remainder_bigint, lhs_bigint.bitwidth));                    \
        ASSERT_OK(kefir_bigint_signed_divide(&lhs_bigint, &remainder_bigint, &rhs_bigint));                         \
        ASSERT_LOAD(&lhs_bigint, (_res));                                                                           \
        ASSERT_LOAD(&remainder_bigint, (_rem));                                                                     \
    } while (0)

    ASSERT_DIV(KEFIR_INT8_MIN, 1, KEFIR_INT8_MIN, 0);
    ASSERT_DIV(KEFIR_INT8_MIN, KEFIR_INT8_MIN, 1, 0);
    ASSERT_DIV(KEFIR_INT8_MIN, -1, KEFIR_INT8_MIN, 0);
    ASSERT_DIV(KEFIR_INT8_MIN + 1, KEFIR_INT8_MIN, 0, KEFIR_INT8_MIN + 1);
    ASSERT_DIV(KEFIR_INT8_MAX, -1, -KEFIR_INT8_MAX, 0);

    ASSERT_DIV(KEFIR_INT16_MIN, 1, KEFIR_INT16_MIN, 0);
    ASSERT_DIV(KEFIR_INT16_MIN, KEFIR_INT16_MIN, 1, 0);
    ASSERT_DIV(KEFIR_INT16_MIN, -1, KEFIR_INT16_MIN, 0);
    ASSERT_DIV(KEFIR_INT16_MIN + 1, KEFIR_INT16_MIN, 0, KEFIR_INT16_MIN + 1);
    ASSERT_DIV(KEFIR_INT16_MAX, -1, -KEFIR_INT16_MAX, 0);

    ASSERT_DIV(KEFIR_INT32_MIN, 1, KEFIR_INT32_MIN, 0);
    ASSERT_DIV(KEFIR_INT32_MIN, KEFIR_INT32_MIN, 1, 0);
    ASSERT_DIV(KEFIR_INT32_MIN, -1, KEFIR_INT32_MIN, 0);
    ASSERT_DIV(KEFIR_INT32_MIN + 1, KEFIR_INT32_MIN, 0, KEFIR_INT32_MIN + 1);
    ASSERT_DIV(KEFIR_INT32_MAX, -1, -KEFIR_INT32_MAX, 0);

    ASSERT_DIV(KEFIR_INT64_MIN, 1, KEFIR_INT64_MIN, 0);
    ASSERT_DIV(KEFIR_INT64_MIN, KEFIR_INT64_MIN, 1, 0);
    ASSERT_DIV(KEFIR_INT64_MIN, -1, KEFIR_INT64_MIN, 0);
    ASSERT_DIV(KEFIR_INT64_MIN + 1, KEFIR_INT64_MIN, 0, KEFIR_INT64_MIN + 1);
    ASSERT_DIV(KEFIR_INT64_MAX, -1, -KEFIR_INT64_MAX, 0);

#undef ASSERT_DIV

    ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &lhs_bigint, 1024));
    ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &rhs_bigint, sizeof(kefir_uint16_t) * CHAR_BIT));
    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &remainder_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_set_unsigned_value(&lhs_bigint, 0));
    ASSERT_OK(kefir_bigint_invert(&lhs_bigint));
    ASSERT_OK(kefir_bigint_right_shift(&lhs_bigint, 1));
    ASSERT_OK(kefir_bigint_negate(&lhs_bigint));
    ASSERT_OK(kefir_bigint_set_signed_value(&rhs_bigint, -0x1000));
    ASSERT_OK(kefir_bigint_signed_divide(&lhs_bigint, &remainder_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, -1ll);
    ASSERT_LOAD(&remainder_bigint, -0xfff);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &lhs_bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &rhs_bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &remainder_bigint));
}
END_CASE

DEFINE_CASE(bigint_unsigned_compare1, "BigInt - unsigned comparison #1") {
    struct kefir_bigint lhs_bigint, rhs_bigint;

    ASSERT_OK(kefir_bigint_init(&lhs_bigint));
    ASSERT_OK(kefir_bigint_init(&rhs_bigint));

#define ASSERT_CMP(_arg1, _arg2)                                                                                      \
    do {                                                                                                              \
        kefir_int_t comparison;                                                                                       \
        ASSERT_USTORE(&lhs_bigint, (_arg1));                                                                          \
        ASSERT_USTORE(&rhs_bigint, (_arg2));                                                                          \
        ASSERT_OK(                                                                                                    \
            kefir_bigint_resize_cast_unsigned(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth))); \
        ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));                     \
        ASSERT_OK(kefir_bigint_unsigned_compare(&lhs_bigint, &rhs_bigint, &comparison));                              \
        ASSERT(comparison == ((_arg1) > (_arg2) ? 1 : ((_arg1) == (_arg2) ? 0 : -1)));                                \
    } while (0)

    for (kefir_uint64_t i = 0; i < 4096; i++) {
        for (kefir_uint64_t j = 0; j < 4096; j++) {
            ASSERT_CMP(i, j);
        }
    }

    ASSERT_CMP(KEFIR_UINT8_MAX, 0);
    ASSERT_CMP(KEFIR_UINT8_MAX, KEFIR_UINT8_MAX);
    ASSERT_CMP(KEFIR_UINT16_MAX, 0);
    ASSERT_CMP(KEFIR_UINT16_MAX, KEFIR_UINT8_MAX);
    ASSERT_CMP(KEFIR_UINT8_MAX, KEFIR_UINT16_MAX);
    ASSERT_CMP(KEFIR_UINT16_MAX, KEFIR_UINT16_MAX);
    ASSERT_CMP(KEFIR_UINT32_MAX, 0);
    ASSERT_CMP(KEFIR_UINT32_MAX, KEFIR_UINT8_MAX);
    ASSERT_CMP(KEFIR_UINT8_MAX, KEFIR_UINT16_MAX);
    ASSERT_CMP(KEFIR_UINT32_MAX, KEFIR_UINT16_MAX);
    ASSERT_CMP(KEFIR_UINT16_MAX, KEFIR_UINT32_MAX);
    ASSERT_CMP(KEFIR_UINT32_MAX, KEFIR_UINT32_MAX);
    ASSERT_CMP(KEFIR_UINT64_MAX, 0);
    ASSERT_CMP(KEFIR_UINT64_MAX, KEFIR_UINT8_MAX);
    ASSERT_CMP(KEFIR_UINT8_MAX, KEFIR_UINT64_MAX);
    ASSERT_CMP(KEFIR_UINT64_MAX, KEFIR_UINT16_MAX);
    ASSERT_CMP(KEFIR_UINT16_MAX, KEFIR_UINT64_MAX);
    ASSERT_CMP(KEFIR_UINT64_MAX, KEFIR_UINT32_MAX);
    ASSERT_CMP(KEFIR_UINT32_MAX, KEFIR_UINT64_MAX);
    ASSERT_CMP(KEFIR_UINT64_MAX, KEFIR_UINT64_MAX);

    for (kefir_size_t i = 1; i < sizeof(kefir_uint64_t) * CHAR_BIT; i++) {
        ASSERT_CMP(1ull << (i - 1), 1ull << (i - 1));
        ASSERT_CMP(1ull << i, 1ull << (i - 1));
        ASSERT_CMP(1ull << i, 1ull << i);
        ASSERT_CMP(1ull << (i - 1), 1ull << i);
        if (i > 1) {
            ASSERT_CMP((1ull << i) + 1, 1ull << i);
            ASSERT_CMP(1ull << i, (1ull << i) + 1);
        }
    }

#undef ASSERT_CMP

    ASSERT_OK(kefir_bigint_free(&kft_mem, &lhs_bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &rhs_bigint));
}
END_CASE

DEFINE_CASE(bigint_signed_compare1, "BigInt - signed comparison #1") {
    struct kefir_bigint lhs_bigint, rhs_bigint;

    ASSERT_OK(kefir_bigint_init(&lhs_bigint));
    ASSERT_OK(kefir_bigint_init(&rhs_bigint));

#define ASSERT_CMP(_arg1, _arg2)                                                                                    \
    do {                                                                                                            \
        kefir_int_t comparison;                                                                                     \
        ASSERT_STORE(&lhs_bigint, (_arg1));                                                                         \
        ASSERT_STORE(&rhs_bigint, (_arg2));                                                                         \
        ASSERT_OK(                                                                                                  \
            kefir_bigint_resize_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth))); \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));                     \
        ASSERT_OK(kefir_bigint_signed_compare(&lhs_bigint, &rhs_bigint, &comparison));                              \
        ASSERT(comparison == ((_arg1) > (_arg2) ? 1 : ((_arg1) == (_arg2) ? 0 : -1)));                              \
    } while (0)

    for (kefir_int64_t i = -4096; i < 4096; i++) {
        for (kefir_int64_t j = -4096; j < 4096; j += 8) {
            ASSERT_CMP(i, j);
        }
    }

    ASSERT_CMP(KEFIR_INT8_MAX, 0);
    ASSERT_CMP(0, KEFIR_INT8_MAX);
    ASSERT_CMP(KEFIR_INT8_MAX, KEFIR_INT8_MAX);
    ASSERT_CMP(KEFIR_INT8_MIN, 0);
    ASSERT_CMP(0, KEFIR_INT8_MIN);
    ASSERT_CMP(KEFIR_INT8_MIN, KEFIR_INT8_MIN);
    ASSERT_CMP(KEFIR_INT8_MAX, KEFIR_INT8_MIN);
    ASSERT_CMP(KEFIR_INT8_MIN, KEFIR_INT8_MAX);

    ASSERT_CMP(KEFIR_INT16_MAX, 0);
    ASSERT_CMP(0, KEFIR_INT16_MAX);
    ASSERT_CMP(KEFIR_INT16_MAX, KEFIR_INT16_MAX);
    ASSERT_CMP(KEFIR_INT16_MIN, 0);
    ASSERT_CMP(0, KEFIR_INT16_MIN);
    ASSERT_CMP(KEFIR_INT16_MIN, KEFIR_INT16_MIN);
    ASSERT_CMP(KEFIR_INT16_MAX, KEFIR_INT16_MIN);
    ASSERT_CMP(KEFIR_INT16_MIN, KEFIR_INT16_MAX);

    ASSERT_CMP(KEFIR_INT32_MAX, 0);
    ASSERT_CMP(0, KEFIR_INT32_MAX);
    ASSERT_CMP(KEFIR_INT32_MAX, KEFIR_INT32_MAX);
    ASSERT_CMP(KEFIR_INT32_MIN, 0);
    ASSERT_CMP(0, KEFIR_INT32_MIN);
    ASSERT_CMP(KEFIR_INT32_MIN, KEFIR_INT32_MIN);
    ASSERT_CMP(KEFIR_INT32_MAX, KEFIR_INT32_MIN);
    ASSERT_CMP(KEFIR_INT32_MIN, KEFIR_INT32_MAX);

    ASSERT_CMP(KEFIR_INT64_MAX, 0);
    ASSERT_CMP(0, KEFIR_INT64_MAX);
    ASSERT_CMP(KEFIR_INT64_MAX, KEFIR_INT64_MAX);
    ASSERT_CMP(KEFIR_INT64_MIN, 0);
    ASSERT_CMP(0, KEFIR_INT64_MIN);
    ASSERT_CMP(KEFIR_INT64_MIN, KEFIR_INT64_MIN);
    ASSERT_CMP(KEFIR_INT64_MAX, KEFIR_INT64_MIN);
    ASSERT_CMP(KEFIR_INT64_MIN, KEFIR_INT64_MAX);

    for (kefir_size_t i = 1; i < sizeof(kefir_uint64_t) * CHAR_BIT - 1; i++) {
        ASSERT_CMP(1ll << (i - 1), 1ll << (i - 1));
        ASSERT_CMP(1ll << i, 1ll << (i - 1));
        ASSERT_CMP(1ll << i, 1ll << i);
        ASSERT_CMP(1ll << (i - 1), 1ll << i);
        if (i > 1) {
            ASSERT_CMP((1ll << i) + 1, 1ll << i);
            ASSERT_CMP(1ll << i, (1ll << i) + 1);
        }

        ASSERT_CMP(-(1ll << (i - 1)), 1ll << (i - 1));
        ASSERT_CMP(-(1ll << i), 1ll << (i - 1));
        ASSERT_CMP(-(1ll << i), 1ll << i);
        ASSERT_CMP(-(1ll << (i - 1)), 1ll << i);
        if (i > 1) {
            ASSERT_CMP(-((1ll << i) + 1), 1ll << i);
            ASSERT_CMP(-(1ll << i), (1ll << i) + 1);
        }

        ASSERT_CMP(1ll << (i - 1), -(1ll << (i - 1)));
        ASSERT_CMP(1ll << i, -(1ll << (i - 1)));
        ASSERT_CMP(1ll << i, -(1ll << i));
        ASSERT_CMP(1ll << (i - 1), -(1ll << i));
        if (i > 1) {
            ASSERT_CMP((1ll << i) + 1, -(1ll << i));
            ASSERT_CMP(1ll << i, -((1ll << i) + 1));
        }

        ASSERT_CMP(-(1ll << (i - 1)), -(1ll << (i - 1)));
        ASSERT_CMP(-(1ll << i), -(1ll << (i - 1)));
        ASSERT_CMP(-(1ll << i), -(1ll << i));
        ASSERT_CMP(-(1ll << (i - 1)), -(1ll << i));
        if (i > 1) {
            ASSERT_CMP(-((1ll << i) + 1), -(1ll << i));
            ASSERT_CMP(-(1ll << i), -((1ll << i) + 1));
        }
    }

#undef ASSERT_CMP

    ASSERT_OK(kefir_bigint_free(&kft_mem, &lhs_bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &rhs_bigint));
}
END_CASE

DEFINE_CASE(bigint_decimal_parse1, "BigInt - parse decimal #1") {
    struct kefir_bigint bigint, base1000, part;

    ASSERT_OK(kefir_bigint_init(&bigint));
    ASSERT_OK(kefir_bigint_init(&base1000));
    ASSERT_OK(kefir_bigint_init(&part));

    int length;
    char buf[32];
#define ASSERT_PARSE(_value)                                                      \
    do {                                                                          \
        length = snprintf(buf, sizeof(buf), "%" KEFIR_INT64_FMT, (_value));       \
        ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, buf, length, 10)); \
        ASSERT_LOAD(&bigint, (kefir_int64_t) (_value));                           \
    } while (0)

    for (kefir_int64_t i = -4096; i < 4096; i++) {
        ASSERT_PARSE(i);
    }

    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, "00001", 5, 10));
    ASSERT_LOAD(&bigint, 1);
    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, "00001", 4, 10));
    ASSERT_LOAD(&bigint, 0);

    for (kefir_size_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT; i++) {
        ASSERT_PARSE((kefir_uint64_t) (1ull << i));
        ASSERT_PARSE((kefir_int64_t) - (1ull << i));
    }

    ASSERT_PARSE((kefir_int64_t) KEFIR_INT8_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT8_MIN);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT16_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT16_MIN);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT32_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT32_MIN);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT64_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT64_MIN);

    ASSERT_PARSE((kefir_int64_t) KEFIR_UINT8_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_UINT16_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_UINT32_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_UINT64_MAX);
#undef ASSERT_PARSE

    const char SUPER_LARGE[] = {"123456789098765432111222333444555666777888999"};
    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, SUPER_LARGE, sizeof(SUPER_LARGE), 10));
#define ASSERT_PART(_part)                                                                \
    do {                                                                                  \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &base1000, bigint.bitwidth)); \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &part, bigint.bitwidth));     \
        ASSERT_OK(kefir_bigint_set_signed_value(&base1000, 1000));                        \
        ASSERT_OK(kefir_bigint_signed_divide(&bigint, &part, &base1000));                 \
        ASSERT_LOAD(&part, (_part));                                                      \
    } while (0)

    ASSERT_PART(999);
    ASSERT_PART(888);
    ASSERT_PART(777);
    ASSERT_PART(666);
    ASSERT_PART(555);
    ASSERT_PART(444);
    ASSERT_PART(333);
    ASSERT_PART(222);
    ASSERT_PART(111);
    ASSERT_PART(432);
    ASSERT_PART(765);
    ASSERT_PART(98);
    ASSERT_PART(789);
    ASSERT_PART(456);
    ASSERT_PART(123);
    ASSERT_LOAD(&bigint, 0);

    const char SUPER_LARGE2[] = {"-123456789098765432111222333444555666777888999"};
    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, SUPER_LARGE2, sizeof(SUPER_LARGE2), 10));

    ASSERT_PART(-999);
    ASSERT_PART(-888);
    ASSERT_PART(-777);
    ASSERT_PART(-666);
    ASSERT_PART(-555);
    ASSERT_PART(-444);
    ASSERT_PART(-333);
    ASSERT_PART(-222);
    ASSERT_PART(-111);
    ASSERT_PART(-432);
    ASSERT_PART(-765);
    ASSERT_PART(-98);
    ASSERT_PART(-789);
    ASSERT_PART(-456);
    ASSERT_PART(-123);
    ASSERT_LOAD(&bigint, 0);

#undef ASSERT_PART

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &base1000));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &part));
}
END_CASE

DEFINE_CASE(bigint_hexadecimal_parse1, "BigInt - parse hexadecimal #1") {
    struct kefir_bigint bigint, base1000, part;

    ASSERT_OK(kefir_bigint_init(&bigint));
    ASSERT_OK(kefir_bigint_init(&base1000));
    ASSERT_OK(kefir_bigint_init(&part));

    int length;
    char buf[32];
#define ASSERT_PARSE(_value)                                                                                \
    do {                                                                                                    \
        length = snprintf(buf, sizeof(buf), "%s%llx", ((_value) < 0 ? "-" : ""),                            \
                          ((_value) < 0 ? -(unsigned long long) (_value) : (unsigned long long) (_value))); \
        ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, buf, length, 16));                           \
        ASSERT_LOAD(&bigint, (kefir_int64_t) (_value));                                                     \
    } while (0)

    for (kefir_int64_t i = -4096; i < 4096; i++) {
        ASSERT_PARSE(i);
    }

    for (kefir_size_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT; i++) {
        ASSERT_PARSE((kefir_int64_t) (1ull << i));
        ASSERT_PARSE((kefir_int64_t) - (1ull << i));
    }

    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, "00001", 5, 16));
    ASSERT_LOAD(&bigint, 1);
    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, "00001", 4, 16));
    ASSERT_LOAD(&bigint, 0);

    ASSERT_PARSE((kefir_int64_t) KEFIR_INT8_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT8_MIN);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT16_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT16_MIN);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT32_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT32_MIN);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT64_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT64_MIN);

    ASSERT_PARSE((kefir_int64_t) KEFIR_UINT8_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_UINT16_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_UINT32_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_UINT64_MAX);

#undef ASSERT_PARSE

    const char SUPER_LARGE[] = {"123456789abcdefFFEEDDCCBBAA99887766554433221100"};
    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, SUPER_LARGE, sizeof(SUPER_LARGE), 16));
#define ASSERT_PART(_part)                                                                \
    do {                                                                                  \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &base1000, bigint.bitwidth)); \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &part, bigint.bitwidth));     \
        ASSERT_OK(kefir_bigint_set_signed_value(&base1000, 0x100));                       \
        ASSERT_OK(kefir_bigint_signed_divide(&bigint, &part, &base1000));                 \
        ASSERT_LOAD(&part, (_part));                                                      \
    } while (0)

    ASSERT_PART(0x00);
    ASSERT_PART(0x11);
    ASSERT_PART(0x22);
    ASSERT_PART(0x33);
    ASSERT_PART(0x44);
    ASSERT_PART(0x55);
    ASSERT_PART(0x66);
    ASSERT_PART(0x77);
    ASSERT_PART(0x88);
    ASSERT_PART(0x99);
    ASSERT_PART(0xaa);
    ASSERT_PART(0xbb);
    ASSERT_PART(0xcc);
    ASSERT_PART(0xdd);
    ASSERT_PART(0xee);
    ASSERT_PART(0xff);
    ASSERT_PART(0xef);
    ASSERT_PART(0xcd);
    ASSERT_PART(0xab);
    ASSERT_PART(0x89);
    ASSERT_PART(0x67);
    ASSERT_PART(0x45);
    ASSERT_PART(0x23);
    ASSERT_PART(0x01);
    ASSERT_LOAD(&bigint, 0);

    const char SUPER_LARGE2[] = {"-123456789abcdefFFEEDDCCBBAA99887766554433221100"};
    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, SUPER_LARGE2, sizeof(SUPER_LARGE2), 16));

    ASSERT_PART(0x00);
    ASSERT_PART(-0x11);
    ASSERT_PART(-0x22);
    ASSERT_PART(-0x33);
    ASSERT_PART(-0x44);
    ASSERT_PART(-0x55);
    ASSERT_PART(-0x66);
    ASSERT_PART(-0x77);
    ASSERT_PART(-0x88);
    ASSERT_PART(-0x99);
    ASSERT_PART(-0xaa);
    ASSERT_PART(-0xbb);
    ASSERT_PART(-0xcc);
    ASSERT_PART(-0xdd);
    ASSERT_PART(-0xee);
    ASSERT_PART(-0xff);
    ASSERT_PART(-0xef);
    ASSERT_PART(-0xcd);
    ASSERT_PART(-0xab);
    ASSERT_PART(-0x89);
    ASSERT_PART(-0x67);
    ASSERT_PART(-0x45);
    ASSERT_PART(-0x23);
    ASSERT_PART(-0x01);
    ASSERT_LOAD(&bigint, 0);

#undef ASSERT_PART

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &base1000));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &part));
}
END_CASE

DEFINE_CASE(bigint_octal_parse1, "BigInt - parse octal #1") {
    struct kefir_bigint bigint, base1000, part;

    ASSERT_OK(kefir_bigint_init(&bigint));
    ASSERT_OK(kefir_bigint_init(&base1000));
    ASSERT_OK(kefir_bigint_init(&part));

    int length;
    char buf[32];
#define ASSERT_PARSE(_value)                                                                                \
    do {                                                                                                    \
        length = snprintf(buf, sizeof(buf), "%s%llo", ((_value) < 0 ? "-" : ""),                            \
                          ((_value) < 0 ? -(unsigned long long) (_value) : (unsigned long long) (_value))); \
        ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, buf, length, 8));                            \
        ASSERT_LOAD(&bigint, (kefir_int64_t) (_value));                                                     \
    } while (0)

    for (kefir_int64_t i = -4096; i < 4096; i++) {
        ASSERT_PARSE(i);
    }

    for (kefir_size_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT; i++) {
        ASSERT_PARSE((kefir_int64_t) (1ull << i));
        ASSERT_PARSE((kefir_int64_t) - (1ull << i));
    }

    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, "00001", 5, 8));
    ASSERT_LOAD(&bigint, 1);
    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, "00001", 4, 8));
    ASSERT_LOAD(&bigint, 0);

    ASSERT_PARSE((kefir_int64_t) KEFIR_INT8_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT8_MIN);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT16_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT16_MIN);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT32_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT32_MIN);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT64_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT64_MIN);

    ASSERT_PARSE((kefir_int64_t) KEFIR_UINT8_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_UINT16_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_UINT32_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_UINT64_MAX);

#undef ASSERT_PARSE

    const char SUPER_LARGE[] = {"1234567000777666555444333222111"};
    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, SUPER_LARGE, sizeof(SUPER_LARGE), 8));
#define ASSERT_PART(_part)                                                                \
    do {                                                                                  \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &base1000, bigint.bitwidth)); \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &part, bigint.bitwidth));     \
        ASSERT_OK(kefir_bigint_set_signed_value(&base1000, 01000));                       \
        ASSERT_OK(kefir_bigint_signed_divide(&bigint, &part, &base1000));                 \
        ASSERT_LOAD(&part, (_part));                                                      \
    } while (0)

    ASSERT_PART(0111);
    ASSERT_PART(0222);
    ASSERT_PART(0333);
    ASSERT_PART(0444);
    ASSERT_PART(0555);
    ASSERT_PART(0666);
    ASSERT_PART(0777);
    ASSERT_PART(0000);
    ASSERT_PART(0567);
    ASSERT_PART(0234);
    ASSERT_PART(0001);
    ASSERT_LOAD(&bigint, 0);

    const char SUPER_LARGE2[] = {"-1234567000777666555444333222111"};
    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, SUPER_LARGE2, sizeof(SUPER_LARGE2), 8));

    ASSERT_PART(-0111);
    ASSERT_PART(-0222);
    ASSERT_PART(-0333);
    ASSERT_PART(-0444);
    ASSERT_PART(-0555);
    ASSERT_PART(-0666);
    ASSERT_PART(-0777);
    ASSERT_PART(-0000);
    ASSERT_PART(-0567);
    ASSERT_PART(-0234);
    ASSERT_PART(-0001);
    ASSERT_LOAD(&bigint, 0);

#undef ASSERT_PART

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &base1000));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &part));
}
END_CASE

static kefir_result_t uint64_to_binstr(kefir_uint64_t value, char *out, kefir_size_t out_length) {
    out_length = MIN(out_length, sizeof(kefir_uint64_t) * CHAR_BIT + 1);
    kefir_size_t i = 0;
    for (; i < out_length - 1; i++, value >>= 1) {
        if (value == 0) {
            if (i == 0) {
                out[i++] = '0';
            }
            break;
        }

        out[i] = '0' + (value & 1);
    }

    for (kefir_size_t j = 0; j < i / 2; j++) {
        const char tmp = out[j];
        out[j] = out[i - j - 1];
        out[i - j - 1] = tmp;
    }
    out[i] = '\0';
    return KEFIR_OK;
}

DEFINE_CASE(bigint_binary_parse1, "BigInt - parse binary #1") {
    struct kefir_bigint bigint, base1000, part;

    ASSERT_OK(kefir_bigint_init(&bigint));
    ASSERT_OK(kefir_bigint_init(&base1000));
    ASSERT_OK(kefir_bigint_init(&part));

    char buf[256];
#define ASSERT_PARSE(_value)                                                                   \
    do {                                                                                       \
        if ((_value) < 0) {                                                                    \
            buf[0] = '-';                                                                      \
            ASSERT_OK(uint64_to_binstr(-(kefir_uint64_t) (_value), buf + 1, sizeof(buf) - 1)); \
        } else {                                                                               \
            ASSERT_OK(uint64_to_binstr((kefir_uint64_t) (_value), buf, sizeof(buf)));          \
        }                                                                                      \
        ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, buf, sizeof(buf), 2));          \
        ASSERT_LOAD(&bigint, (kefir_int64_t) (_value));                                        \
    } while (0)

    for (kefir_int64_t i = -4096; i < 4096; i++) {
        ASSERT_PARSE(i);
    }

    for (kefir_size_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT; i++) {
        ASSERT_PARSE((kefir_int64_t) (1ull << i));
        ASSERT_PARSE((kefir_int64_t) - (1ull << i));
    }

    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, "00001", 5, 8));
    ASSERT_LOAD(&bigint, 1);
    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, "00001", 4, 8));
    ASSERT_LOAD(&bigint, 0);

    ASSERT_PARSE((kefir_int64_t) KEFIR_INT8_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT8_MIN);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT16_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT16_MIN);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT32_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT32_MIN);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT64_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_INT64_MIN);

    ASSERT_PARSE((kefir_int64_t) KEFIR_UINT8_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_UINT16_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_UINT32_MAX);
    ASSERT_PARSE((kefir_int64_t) KEFIR_UINT64_MAX);

#undef ASSERT_PARSE

    const char SUPER_LARGE[] = {
        "111100001111111100000000111100000000111111111111111100001111111100000000111100000000111111111111"};
    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, SUPER_LARGE, sizeof(SUPER_LARGE), 2));
#define ASSERT_PART(_part)                                                                \
    do {                                                                                  \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &base1000, bigint.bitwidth)); \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &part, bigint.bitwidth));     \
        ASSERT_OK(kefir_bigint_set_signed_value(&base1000, 0x100));                       \
        ASSERT_OK(kefir_bigint_signed_divide(&bigint, &part, &base1000));                 \
        ASSERT_LOAD(&part, (_part));                                                      \
    } while (0)

    ASSERT_PART(0xff);
    ASSERT_PART(0x0f);
    ASSERT_PART(0xf0);
    ASSERT_PART(0x00);
    ASSERT_PART(0xff);
    ASSERT_PART(0xf0);
    ASSERT_PART(0xff);
    ASSERT_PART(0x0f);
    ASSERT_PART(0xf0);
    ASSERT_PART(0x00);
    ASSERT_PART(0xff);
    ASSERT_PART(0xf0);
    ASSERT_LOAD(&bigint, 0);

    const char SUPER_LARGE2[] = {
        "-111100001111111100000000111100000000111111111111111100001111111100000000111100000000111111111111"};
    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, SUPER_LARGE2, sizeof(SUPER_LARGE2), 2));

    ASSERT_PART(-0xff);
    ASSERT_PART(-0x0f);
    ASSERT_PART(-0xf0);
    ASSERT_PART(-0x00);
    ASSERT_PART(-0xff);
    ASSERT_PART(-0xf0);
    ASSERT_PART(-0xff);
    ASSERT_PART(-0x0f);
    ASSERT_PART(-0xf0);
    ASSERT_PART(-0x00);
    ASSERT_PART(-0xff);
    ASSERT_PART(-0xf0);
    ASSERT_LOAD(&bigint, 0);

#undef ASSERT_PART

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &base1000));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &part));
}
END_CASE

DEFINE_CASE(bigint_decimal_format1, "BigInt - format decimal #1") {
    struct kefir_bigint bigint;

    ASSERT_OK(kefir_bigint_init(&bigint));

#define ASSERT_FMT(_val)                                                                \
    do {                                                                                \
        char buf[128];                                                                  \
        ASSERT_USTORE(&bigint, (_val));                                                 \
        char *formatted;                                                                \
        ASSERT_OK(kefir_bigint_unsigned_format10(&kft_mem, &bigint, &formatted, NULL)); \
        snprintf(buf, sizeof(buf), "%" KEFIR_UINT64_FMT, (_val));                       \
        ASSERT(strcmp(formatted, buf) == 0);                                            \
        KEFIR_FREE(&kft_mem, formatted);                                                \
    } while (0)

    for (kefir_uint64_t i = 0; i < 4096; i++) {
        ASSERT_FMT(i);
    }

    for (kefir_uint64_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT; i++) {
        ASSERT_FMT(1ul << i);
    }

    ASSERT_FMT((kefir_uint64_t) KEFIR_UINT8_MAX);
    ASSERT_FMT((kefir_uint64_t) KEFIR_UINT16_MAX);
    ASSERT_FMT((kefir_uint64_t) KEFIR_UINT32_MAX);
    ASSERT_FMT((kefir_uint64_t) KEFIR_UINT64_MAX);

#undef ASSERT_FMT

    const char SUPER_LONG[] = "123456789098765432111223344556677889900999888777666555444333222111";
    ASSERT_OK(kefir_bigint_unsigned_parse10(&kft_mem, &bigint, SUPER_LONG, sizeof(SUPER_LONG)));
    char *formatted;
    ASSERT_OK(kefir_bigint_unsigned_format10(&kft_mem, &bigint, &formatted, NULL));
    ASSERT(strcmp(formatted, SUPER_LONG) == 0);
    KEFIR_FREE(&kft_mem, formatted);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
}
END_CASE

DEFINE_CASE(bigint_hexdecimal_format1, "BigInt - format hexdecimal #1") {
    struct kefir_bigint bigint;

    ASSERT_OK(kefir_bigint_init(&bigint));

#define ASSERT_FMT(_val)                                                                \
    do {                                                                                \
        char buf[128];                                                                  \
        ASSERT_USTORE(&bigint, (_val));                                                 \
        char *formatted;                                                                \
        ASSERT_OK(kefir_bigint_unsigned_format16(&kft_mem, &bigint, &formatted, NULL)); \
        snprintf(buf, sizeof(buf), "%llx", (unsigned long long) (_val));                \
        ASSERT(strcmp(formatted, buf) == 0);                                            \
        KEFIR_FREE(&kft_mem, formatted);                                                \
    } while (0)

    for (kefir_uint64_t i = 0; i < 4096; i++) {
        ASSERT_FMT(i);
    }

    for (kefir_uint64_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT; i++) {
        ASSERT_FMT(1ul << i);
    }

    ASSERT_FMT((kefir_uint64_t) KEFIR_UINT8_MAX);
    ASSERT_FMT((kefir_uint64_t) KEFIR_UINT16_MAX);
    ASSERT_FMT((kefir_uint64_t) KEFIR_UINT32_MAX);
    ASSERT_FMT((kefir_uint64_t) KEFIR_UINT64_MAX);

#undef ASSERT_FMT

    const char SUPER_LONG[] = "1234567890abcdef00998877665544332211ffeeddccbbaa";
    ASSERT_OK(kefir_bigint_unsigned_parse16(&kft_mem, &bigint, SUPER_LONG, sizeof(SUPER_LONG)));
    char *formatted;
    ASSERT_OK(kefir_bigint_unsigned_format16(&kft_mem, &bigint, &formatted, NULL));
    ASSERT(strcmp(formatted, SUPER_LONG) == 0);
    KEFIR_FREE(&kft_mem, formatted);

    const char SUPER_LONG2[] = "000000000000000abcdef";
    ASSERT_OK(kefir_bigint_unsigned_parse16(&kft_mem, &bigint, SUPER_LONG2, sizeof(SUPER_LONG2)));
    ASSERT_OK(kefir_bigint_unsigned_format16(&kft_mem, &bigint, &formatted, NULL));
    ASSERT(strcmp(formatted, SUPER_LONG2 + 15) == 0);
    KEFIR_FREE(&kft_mem, formatted);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
}
END_CASE

DEFINE_CASE(bigint_octal_format1, "BigInt - format octal #1") {
    struct kefir_bigint bigint;

    ASSERT_OK(kefir_bigint_init(&bigint));

#define ASSERT_FMT(_val)                                                               \
    do {                                                                               \
        char buf[128];                                                                 \
        ASSERT_USTORE(&bigint, (_val));                                                \
        char *formatted;                                                               \
        ASSERT_OK(kefir_bigint_unsigned_format8(&kft_mem, &bigint, &formatted, NULL)); \
        snprintf(buf, sizeof(buf), "%llo", (unsigned long long) (_val));               \
        ASSERT(strcmp(formatted, buf) == 0);                                           \
        KEFIR_FREE(&kft_mem, formatted);                                               \
    } while (0)

    for (kefir_uint64_t i = 0; i < 4096; i++) {
        ASSERT_FMT(i);
    }

    for (kefir_uint64_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT; i++) {
        ASSERT_FMT(1ul << i);
    }

    ASSERT_FMT((kefir_uint64_t) KEFIR_UINT8_MAX);
    ASSERT_FMT((kefir_uint64_t) KEFIR_UINT16_MAX);
    ASSERT_FMT((kefir_uint64_t) KEFIR_UINT32_MAX);
    ASSERT_FMT((kefir_uint64_t) KEFIR_UINT64_MAX);

#undef ASSERT_FMT

    const char SUPER_LONG[] = "123456707766554433221100111222333444555666777000077776666555544443333222211110";
    ASSERT_OK(kefir_bigint_unsigned_parse8(&kft_mem, &bigint, SUPER_LONG, sizeof(SUPER_LONG)));
    char *formatted;
    ASSERT_OK(kefir_bigint_unsigned_format8(&kft_mem, &bigint, &formatted, NULL));
    ASSERT(strcmp(formatted, SUPER_LONG) == 0);
    KEFIR_FREE(&kft_mem, formatted);

    const char SUPER_LONG2[] = "0000000000000001234567";
    ASSERT_OK(kefir_bigint_unsigned_parse8(&kft_mem, &bigint, SUPER_LONG2, sizeof(SUPER_LONG2)));
    ASSERT_OK(kefir_bigint_unsigned_format8(&kft_mem, &bigint, &formatted, NULL));
    ASSERT(strcmp(formatted, SUPER_LONG2 + 15) == 0);
    KEFIR_FREE(&kft_mem, formatted);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
}
END_CASE

DEFINE_CASE(bigint_binary_format1, "BigInt - format binary #1") {
    struct kefir_bigint bigint;

    ASSERT_OK(kefir_bigint_init(&bigint));

#define ASSERT_FMT(_val)                                                               \
    do {                                                                               \
        char buf[128];                                                                 \
        ASSERT_USTORE(&bigint, (_val));                                                \
        char *formatted;                                                               \
        ASSERT_OK(kefir_bigint_unsigned_format2(&kft_mem, &bigint, &formatted, NULL)); \
        ASSERT_OK(uint64_to_binstr((_val), buf, sizeof(buf)));                         \
        ASSERT(strcmp(formatted, buf) == 0);                                           \
        KEFIR_FREE(&kft_mem, formatted);                                               \
    } while (0)

    for (kefir_uint64_t i = 0; i < 4096; i++) {
        ASSERT_FMT(i);
    }

    for (kefir_uint64_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT; i++) {
        ASSERT_FMT(1ul << i);
    }

    ASSERT_FMT((kefir_uint64_t) KEFIR_UINT8_MAX);
    ASSERT_FMT((kefir_uint64_t) KEFIR_UINT16_MAX);
    ASSERT_FMT((kefir_uint64_t) KEFIR_UINT32_MAX);
    ASSERT_FMT((kefir_uint64_t) KEFIR_UINT64_MAX);

#undef ASSERT_FMT

    const char SUPER_LONG[] =
        "11110000111111110000000010101010101010111001100110011001100111111110000000010100101111101010111101";
    ASSERT_OK(kefir_bigint_unsigned_parse2(&kft_mem, &bigint, SUPER_LONG, sizeof(SUPER_LONG)));
    char *formatted;
    ASSERT_OK(kefir_bigint_unsigned_format2(&kft_mem, &bigint, &formatted, NULL));
    ASSERT(strcmp(formatted, SUPER_LONG) == 0);
    KEFIR_FREE(&kft_mem, formatted);

    const char SUPER_LONG2[] = "0000000000000001010110011001110001110001";
    ASSERT_OK(kefir_bigint_unsigned_parse2(&kft_mem, &bigint, SUPER_LONG2, sizeof(SUPER_LONG2)));
    ASSERT_OK(kefir_bigint_unsigned_format2(&kft_mem, &bigint, &formatted, NULL));
    ASSERT(strcmp(formatted, SUPER_LONG2 + 15) == 0);
    KEFIR_FREE(&kft_mem, formatted);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
}
END_CASE

DEFINE_CASE(bigint_min_bigint_unsigned_width1, "BigInt - minimum unsigned width #1") {
    struct kefir_bigint bigint, bigint2;

    ASSERT_OK(kefir_bigint_init(&bigint));
    ASSERT_OK(kefir_bigint_init(&bigint2));

    for (kefir_uint64_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT; i++) {
        ASSERT_USTORE(&bigint, 1ul << i);
        ASSERT(kefir_bigint_min_unsigned_width(&bigint) == (i + 1));
        ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &bigint, i * 3 + 1));
        ASSERT_OK(kefir_bigint_left_shift(&bigint, i));
        ASSERT(kefir_bigint_min_unsigned_width(&bigint) == (2 * i + 1));
        ASSERT_OK(kefir_bigint_left_shift(&bigint, i));
        ASSERT(kefir_bigint_min_unsigned_width(&bigint) == (3 * i + 1));

        ASSERT_USTORE(&bigint, 1ul << i);
        ASSERT_USTORE(&bigint2, 1);
        ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &bigint2, bigint.bitwidth));
        ASSERT_OK(kefir_bigint_subtract(&bigint, &bigint2));
        ASSERT(kefir_bigint_min_unsigned_width(&bigint) == MAX(i, 1));
    }

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint2));
}
END_CASE

DEFINE_CASE(bigint_signed_to_float1, "BigInt - signed to float conversion #1") {
    struct kefir_bigint bigint, bigint2;

    ASSERT_OK(kefir_bigint_init(&bigint));
    ASSERT_OK(kefir_bigint_init(&bigint2));

#define ASSERT_CAST(_arg)                                                                                     \
    do {                                                                                                      \
        ASSERT_STORE(&bigint, (_arg));                                                                        \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &bigint,                                          \
                                                  MAX(sizeof(kefir_float32_t) * CHAR_BIT, bigint.bitwidth))); \
        ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint2, bigint.bitwidth));                           \
                                                                                                              \
        kefir_float32_t value;                                                                                \
        ASSERT_OK(kefir_bigint_signed_to_float(&bigint, &bigint2, &value));                                   \
        ASSERT(fabs(value - (kefir_float32_t) (_arg)) < 1e-3);                                                \
    } while (0)

    for (kefir_int64_t i = -4096; i < 4096; i++) {
        ASSERT_CAST(i);
    }

    for (kefir_size_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT - 1; i++) {
        ASSERT_CAST(1ll << i);
        ASSERT_CAST(-(1ll << i));
    }

    ASSERT_CAST(KEFIR_INT8_MIN);
    ASSERT_CAST(KEFIR_INT8_MAX);
    ASSERT_CAST(KEFIR_INT16_MIN);
    ASSERT_CAST(KEFIR_INT16_MAX);
    ASSERT_CAST(KEFIR_INT32_MIN);
    ASSERT_CAST(KEFIR_INT32_MAX);
    ASSERT_CAST(KEFIR_INT64_MIN);
    ASSERT_CAST(KEFIR_INT64_MAX);

#undef ASSERT_CAST

    ASSERT_STORE(&bigint, -1);
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &bigint, 1024));
    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint2, bigint.bitwidth));
    kefir_float32_t value;
    ASSERT_OK(kefir_bigint_signed_to_float(&bigint, &bigint2, &value));
    ASSERT(fabs(value - (kefir_float32_t) -1.0f) < 1e-3);

    const char MAXIUM[] = "340282346638528859811704183484516925440";
    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, MAXIUM, sizeof(MAXIUM), 10));
    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint2, bigint.bitwidth));
    ASSERT_OK(kefir_bigint_signed_to_float(&bigint, &bigint2, &value));
    ASSERT(fabs(value - (kefir_float32_t) FLT_MAX) < 1e-3);

    const char MINUMUM[] = "-340282346638528859811704183484516925440";
    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, MINUMUM, sizeof(MINUMUM), 10));
    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint2, bigint.bitwidth));
    ASSERT_OK(kefir_bigint_signed_to_float(&bigint, &bigint2, &value));
    ASSERT(fabs(value - (kefir_float32_t) -FLT_MAX) < 1e-3);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint2));
}
END_CASE

DEFINE_CASE(bigint_signed_to_double1, "BigInt - signed to double conversion #1") {
    struct kefir_bigint bigint, bigint2;

    ASSERT_OK(kefir_bigint_init(&bigint));
    ASSERT_OK(kefir_bigint_init(&bigint2));

#define ASSERT_CAST(_arg)                                                                                     \
    do {                                                                                                      \
        ASSERT_STORE(&bigint, (_arg));                                                                        \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &bigint,                                          \
                                                  MAX(sizeof(kefir_float64_t) * CHAR_BIT, bigint.bitwidth))); \
        ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint2, bigint.bitwidth));                           \
                                                                                                              \
        kefir_float64_t value;                                                                                \
        ASSERT_OK(kefir_bigint_signed_to_double(&bigint, &bigint2, &value));                                  \
        ASSERT(fabs(value - (kefir_float64_t) (_arg)) < 1e-6);                                                \
    } while (0)

    for (kefir_int64_t i = -4096; i < 4096; i++) {
        ASSERT_CAST(i);
    }

    for (kefir_size_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT - 1; i++) {
        ASSERT_CAST(1ll << i);
        ASSERT_CAST(-(1ll << i));
    }

    ASSERT_CAST(KEFIR_INT8_MIN);
    ASSERT_CAST(KEFIR_INT8_MAX);
    ASSERT_CAST(KEFIR_INT16_MIN);
    ASSERT_CAST(KEFIR_INT16_MAX);
    ASSERT_CAST(KEFIR_INT32_MIN);
    ASSERT_CAST(KEFIR_INT32_MAX);
    ASSERT_CAST(KEFIR_INT64_MIN);
    ASSERT_CAST(KEFIR_INT64_MAX);

#undef ASSERT_CAST

    ASSERT_STORE(&bigint, -1);
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &bigint, 1024));
    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint2, bigint.bitwidth));
    kefir_float64_t value;
    ASSERT_OK(kefir_bigint_signed_to_double(&bigint, &bigint2, &value));
    ASSERT(fabs(value - (kefir_float64_t) -1.0) < 1e-6);

    const char MAXIUM[] = "340282346638528859811704183484516925440";
    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, MAXIUM, sizeof(MAXIUM), 10));
    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint2, bigint.bitwidth));
    ASSERT_OK(kefir_bigint_signed_to_double(&bigint, &bigint2, &value));
    ASSERT(fabs(value - (kefir_float64_t) FLT_MAX) < 1e-3);

    const char MINUMUM[] = "-340282346638528859811704183484516925440";
    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, MINUMUM, sizeof(MINUMUM), 10));
    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint2, bigint.bitwidth));
    ASSERT_OK(kefir_bigint_signed_to_double(&bigint, &bigint2, &value));
    ASSERT(fabs(value - (kefir_float64_t) -FLT_MAX) < 1e-3);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint2));
}
END_CASE

DEFINE_CASE(bigint_signed_to_long_double1, "BigInt - signed to long double conversion #1") {
    struct kefir_bigint bigint, bigint2;

    ASSERT_OK(kefir_bigint_init(&bigint));
    ASSERT_OK(kefir_bigint_init(&bigint2));

#define ASSERT_CAST(_arg)                                                                                         \
    do {                                                                                                          \
        ASSERT_STORE(&bigint, (_arg));                                                                            \
        ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &bigint,                                              \
                                                  MAX(sizeof(kefir_long_double_t) * CHAR_BIT, bigint.bitwidth))); \
        ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint2, bigint.bitwidth));                               \
                                                                                                                  \
        kefir_long_double_t value;                                                                                \
        ASSERT_OK(kefir_bigint_signed_to_long_double(&bigint, &bigint2, &value));                                 \
        ASSERT(fabsl(value - (kefir_long_double_t) (_arg)) < 1e-8);                                               \
    } while (0)

    for (kefir_int64_t i = -4096; i < 4096; i++) {
        ASSERT_CAST(i);
    }

    for (kefir_size_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT - 1; i++) {
        ASSERT_CAST(1ll << i);
        ASSERT_CAST(-(1ll << i));
    }

    ASSERT_CAST(KEFIR_INT8_MIN);
    ASSERT_CAST(KEFIR_INT8_MAX);
    ASSERT_CAST(KEFIR_INT16_MIN);
    ASSERT_CAST(KEFIR_INT16_MAX);
    ASSERT_CAST(KEFIR_INT32_MIN);
    ASSERT_CAST(KEFIR_INT32_MAX);
    ASSERT_CAST(KEFIR_INT64_MIN);
    ASSERT_CAST(KEFIR_INT64_MAX);

#undef ASSERT_CAST

    ASSERT_STORE(&bigint, -1);
    ASSERT_OK(kefir_bigint_resize_cast_signed(&kft_mem, &bigint, 1024));
    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint2, bigint.bitwidth));
    kefir_long_double_t value;
    ASSERT_OK(kefir_bigint_signed_to_long_double(&bigint, &bigint2, &value));
    ASSERT(fabsl(value - (kefir_long_double_t) -1.0) < 1e-8);

    const char MAXIUM[] = "340282346638528859811704183484516925440";
    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, MAXIUM, sizeof(MAXIUM), 10));
    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint2, bigint.bitwidth));
    ASSERT_OK(kefir_bigint_signed_to_long_double(&bigint, &bigint2, &value));
    ASSERT(fabsl(value - (kefir_long_double_t) FLT_MAX) < 1e-8);

    const char MINUMUM[] = "-340282346638528859811704183484516925440";
    ASSERT_OK(kefir_bigint_signed_parse(&kft_mem, &bigint, MINUMUM, sizeof(MINUMUM), 10));
    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint2, bigint.bitwidth));
    ASSERT_OK(kefir_bigint_signed_to_long_double(&bigint, &bigint2, &value));
    ASSERT(fabsl(value - (kefir_long_double_t) -FLT_MAX) < 1e-8);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint2));
}
END_CASE

DEFINE_CASE(bigint_unsigned_to_float1, "BigInt - unsigned to float conversion #1") {
    struct kefir_bigint bigint, bigint2;

    ASSERT_OK(kefir_bigint_init(&bigint));
    ASSERT_OK(kefir_bigint_init(&bigint2));

#define ASSERT_CAST(_arg)                                                                                       \
    do {                                                                                                        \
        ASSERT_USTORE(&bigint, (_arg));                                                                         \
        ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &bigint,                                          \
                                                    MAX(sizeof(kefir_float32_t) * CHAR_BIT, bigint.bitwidth))); \
        ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint2, bigint.bitwidth));                             \
                                                                                                                \
        kefir_float32_t value;                                                                                  \
        ASSERT_OK(kefir_bigint_unsigned_to_float(&bigint, &bigint2, &value));                                   \
        ASSERT(fabs(value - (kefir_float32_t) (_arg)) < 1e-3);                                                  \
    } while (0)

    for (kefir_uint64_t i = 0; i < 4096; i++) {
        ASSERT_CAST(i);
    }

    for (kefir_size_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT - 1; i++) {
        ASSERT_CAST(1ull << i);
        ASSERT_CAST(-(1ull << i));
    }

    ASSERT_CAST(KEFIR_UINT8_MAX);
    ASSERT_CAST(KEFIR_UINT16_MAX);
    ASSERT_CAST(KEFIR_UINT32_MAX);
    ASSERT_CAST(KEFIR_UINT64_MAX);

#undef ASSERT_CAST

    kefir_float32_t value;
    const char MAXIUM[] = "340282346638528859811704183484516925440";
    ASSERT_OK(kefir_bigint_unsigned_parse10(&kft_mem, &bigint, MAXIUM, sizeof(MAXIUM)));
    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint2, bigint.bitwidth));
    ASSERT_OK(kefir_bigint_unsigned_to_float(&bigint, &bigint2, &value));
    ASSERT(fabs(value - (kefir_float32_t) FLT_MAX) < 1e-3);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint2));
}
END_CASE

DEFINE_CASE(bigint_unsigned_to_double1, "BigInt - unsigned to double conversion #1") {
    struct kefir_bigint bigint, bigint2;

    ASSERT_OK(kefir_bigint_init(&bigint));
    ASSERT_OK(kefir_bigint_init(&bigint2));

#define ASSERT_CAST(_arg)                                                                                       \
    do {                                                                                                        \
        ASSERT_USTORE(&bigint, (_arg));                                                                         \
        ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &bigint,                                          \
                                                    MAX(sizeof(kefir_float64_t) * CHAR_BIT, bigint.bitwidth))); \
        ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint2, bigint.bitwidth));                             \
                                                                                                                \
        kefir_float64_t value;                                                                                  \
        ASSERT_OK(kefir_bigint_unsigned_to_double(&bigint, &bigint2, &value));                                  \
        ASSERT(fabs(value - (kefir_float64_t) (_arg)) < 1e-6);                                                  \
    } while (0)

    for (kefir_uint64_t i = 0; i < 4096; i++) {
        ASSERT_CAST(i);
    }

    for (kefir_size_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT - 1; i++) {
        ASSERT_CAST(1ull << i);
        ASSERT_CAST(-(1ull << i));
    }

    ASSERT_CAST(KEFIR_UINT8_MAX);
    ASSERT_CAST(KEFIR_UINT16_MAX);
    ASSERT_CAST(KEFIR_UINT32_MAX);
    ASSERT_CAST(KEFIR_UINT64_MAX);

#undef ASSERT_CAST

    kefir_float64_t value;
    const char MAXIUM[] = "340282346638528859811704183484516925440";
    ASSERT_OK(kefir_bigint_unsigned_parse10(&kft_mem, &bigint, MAXIUM, sizeof(MAXIUM)));
    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint2, bigint.bitwidth));
    ASSERT_OK(kefir_bigint_unsigned_to_double(&bigint, &bigint2, &value));
    ASSERT(fabs(value - (kefir_float64_t) FLT_MAX) < 1e-6);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint2));
}
END_CASE

DEFINE_CASE(bigint_unsigned_to_long_double1, "BigInt - unsigned to long double conversion #1") {
    struct kefir_bigint bigint, bigint2;

    ASSERT_OK(kefir_bigint_init(&bigint));
    ASSERT_OK(kefir_bigint_init(&bigint2));

#define ASSERT_CAST(_arg)                                                                                           \
    do {                                                                                                            \
        ASSERT_USTORE(&bigint, (_arg));                                                                             \
        ASSERT_OK(kefir_bigint_resize_cast_unsigned(&kft_mem, &bigint,                                              \
                                                    MAX(sizeof(kefir_long_double_t) * CHAR_BIT, bigint.bitwidth))); \
        ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint2, bigint.bitwidth));                                 \
                                                                                                                    \
        kefir_long_double_t value;                                                                                  \
        ASSERT_OK(kefir_bigint_unsigned_to_long_double(&bigint, &bigint2, &value));                                 \
        ASSERT(fabsl(value - (kefir_long_double_t) (_arg)) < 1e-8);                                                 \
    } while (0)

    for (kefir_uint64_t i = 0; i < 4096; i++) {
        ASSERT_CAST(i);
    }

    for (kefir_size_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT - 1; i++) {
        ASSERT_CAST(1ull << i);
    }

    ASSERT_CAST(KEFIR_UINT8_MAX);
    ASSERT_CAST(KEFIR_UINT16_MAX);
    ASSERT_CAST(KEFIR_UINT32_MAX);
    ASSERT_CAST(KEFIR_UINT64_MAX);

#undef ASSERT_CAST

    kefir_long_double_t value;
    const char MAXIUM[] = "340282346638528859811704183484516925440";
    ASSERT_OK(kefir_bigint_unsigned_parse10(&kft_mem, &bigint, MAXIUM, sizeof(MAXIUM)));
    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint2, bigint.bitwidth));
    ASSERT_OK(kefir_bigint_unsigned_to_long_double(&bigint, &bigint2, &value));
    ASSERT(fabsl(value - (kefir_long_double_t) FLT_MAX) < 1e-8);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint2));
}
END_CASE

DEFINE_CASE(bigint_float_to_signed1, "BigInt - float to signed conversion #1") {
    struct kefir_bigint bigint;

    ASSERT_OK(kefir_bigint_init(&bigint));

#define ASSERT_CAST(_arg, _width)                                                                           \
    do {                                                                                                    \
        ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint, sizeof(kefir_int##_width##_t) * CHAR_BIT)); \
                                                                                                            \
        ASSERT_OK(kefir_bigint_signed_from_float(&bigint, (_arg)));                                         \
        ASSERT_LOAD(&bigint, (kefir_int##_width##_t)(_arg));                                                \
    } while (0)

    for (kefir_float32_t i = -100.0f; i < 100.0f; i += 0.25f) {
        ASSERT_CAST(i, 8);
        ASSERT_CAST(i * 1e-1f, 8);
        ASSERT_CAST(i, 16);
        ASSERT_CAST(i * 1e-1f, 16);
        ASSERT_CAST(i, 32);
        ASSERT_CAST(i * 1e-1f, 32);
        ASSERT_CAST(i, 64);
        ASSERT_CAST(i * 1e-1f, 64);
        ASSERT_CAST(i * 1e3f, 32);
        ASSERT_CAST(i * 1e5f, 32);
        ASSERT_CAST(i * 1e3f, 64);
        ASSERT_CAST(i * 1e5f, 64);
    }

    ASSERT_CAST((kefir_float32_t) KEFIR_INT8_MIN, 8);
    ASSERT_CAST((kefir_float32_t) KEFIR_INT8_MAX, 8);
    ASSERT_CAST((kefir_float32_t) KEFIR_INT8_MIN, 16);
    ASSERT_CAST((kefir_float32_t) KEFIR_INT8_MAX, 16);
    ASSERT_CAST((kefir_float32_t) KEFIR_INT8_MIN, 32);
    ASSERT_CAST((kefir_float32_t) KEFIR_INT8_MAX, 32);
    ASSERT_CAST((kefir_float32_t) KEFIR_INT8_MIN, 64);
    ASSERT_CAST((kefir_float32_t) KEFIR_INT8_MAX, 64);
    ASSERT_CAST((kefir_float32_t) KEFIR_INT16_MIN, 16);
    ASSERT_CAST((kefir_float32_t) KEFIR_INT16_MAX, 16);
    ASSERT_CAST((kefir_float32_t) KEFIR_INT16_MIN, 32);
    ASSERT_CAST((kefir_float32_t) KEFIR_INT16_MAX, 32);
    ASSERT_CAST((kefir_float32_t) KEFIR_INT16_MIN, 64);
    ASSERT_CAST((kefir_float32_t) KEFIR_INT16_MAX, 64);
    ASSERT_CAST((kefir_float32_t) KEFIR_INT32_MIN, 32);
    ASSERT_CAST((kefir_float32_t) KEFIR_INT32_MAX, 32);
    ASSERT_CAST((kefir_float32_t) KEFIR_INT32_MIN, 64);
    ASSERT_CAST((kefir_float32_t) KEFIR_INT32_MAX, 64);
    ASSERT_CAST((kefir_float32_t) KEFIR_INT64_MIN, 64);
    ASSERT_CAST((kefir_float32_t) KEFIR_INT64_MAX, 64);

#undef ASSERT_CAST

    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint, sizeof(kefir_int64_t) * CHAR_BIT));
    ASSERT_OK(kefir_bigint_signed_from_float(&bigint, FLT_MAX));
    ASSERT_LOAD(&bigint, KEFIR_INT64_MAX);
    ASSERT_OK(kefir_bigint_signed_from_float(&bigint, -FLT_MAX));
    ASSERT_LOAD(&bigint, KEFIR_INT64_MIN);

    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint, 1024));
    ASSERT_OK(kefir_bigint_signed_from_float(&bigint, FLT_MAX));
    char buf[128];
    ASSERT_OK(kefir_bigint_unsigned_format10_into(&kft_mem, &bigint, buf, sizeof(buf)));
    ASSERT(strcmp(buf, "340282346638528859811704183484516925440") == 0);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
}
END_CASE

DEFINE_CASE(bigint_double_to_signed1, "BigInt - double to signed conversion #1") {
    struct kefir_bigint bigint;

    ASSERT_OK(kefir_bigint_init(&bigint));

#define ASSERT_CAST(_arg, _width)                                                                           \
    do {                                                                                                    \
        ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint, sizeof(kefir_int##_width##_t) * CHAR_BIT)); \
                                                                                                            \
        ASSERT_OK(kefir_bigint_signed_from_double(&bigint, (_arg)));                                        \
        ASSERT_LOAD(&bigint, (kefir_int##_width##_t)(_arg));                                                \
    } while (0)

    for (kefir_float64_t i = -100.0; i < 100.0; i += 0.25) {
        ASSERT_CAST(i, 8);
        ASSERT_CAST(i * 1e-1, 8);
        ASSERT_CAST(i, 16);
        ASSERT_CAST(i * 1e-1, 16);
        ASSERT_CAST(i, 32);
        ASSERT_CAST(i * 1e-1, 32);
        ASSERT_CAST(i, 64);
        ASSERT_CAST(i * 1e-1, 64);
        ASSERT_CAST(i * 1e3, 32);
        ASSERT_CAST(i * 1e5, 32);
        ASSERT_CAST(i * 1e3, 64);
        ASSERT_CAST(i * 1e5, 64);
    }

    ASSERT_CAST((kefir_float64_t) KEFIR_INT8_MIN, 8);
    ASSERT_CAST((kefir_float64_t) KEFIR_INT8_MAX, 8);
    ASSERT_CAST((kefir_float64_t) KEFIR_INT8_MIN, 16);
    ASSERT_CAST((kefir_float64_t) KEFIR_INT8_MAX, 16);
    ASSERT_CAST((kefir_float64_t) KEFIR_INT8_MIN, 32);
    ASSERT_CAST((kefir_float64_t) KEFIR_INT8_MAX, 32);
    ASSERT_CAST((kefir_float64_t) KEFIR_INT8_MIN, 64);
    ASSERT_CAST((kefir_float64_t) KEFIR_INT8_MAX, 64);
    ASSERT_CAST((kefir_float64_t) KEFIR_INT16_MIN, 16);
    ASSERT_CAST((kefir_float64_t) KEFIR_INT16_MAX, 16);
    ASSERT_CAST((kefir_float64_t) KEFIR_INT16_MIN, 32);
    ASSERT_CAST((kefir_float64_t) KEFIR_INT16_MAX, 32);
    ASSERT_CAST((kefir_float64_t) KEFIR_INT16_MIN, 64);
    ASSERT_CAST((kefir_float64_t) KEFIR_INT16_MAX, 64);
    ASSERT_CAST((kefir_float64_t) KEFIR_INT32_MIN, 32);
    ASSERT_CAST((kefir_float64_t) KEFIR_INT32_MAX, 32);
    ASSERT_CAST((kefir_float64_t) KEFIR_INT32_MIN, 64);
    ASSERT_CAST((kefir_float64_t) KEFIR_INT32_MAX, 64);
    ASSERT_CAST((kefir_float64_t) KEFIR_INT64_MIN, 64);
    ASSERT_CAST((kefir_float64_t) KEFIR_INT64_MAX, 64);

#undef ASSERT_CAST

    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint, sizeof(kefir_int64_t) * CHAR_BIT));
    ASSERT_OK(kefir_bigint_signed_from_double(&bigint, FLT_MAX));
    ASSERT_LOAD(&bigint, KEFIR_INT64_MAX);
    ASSERT_OK(kefir_bigint_signed_from_double(&bigint, -FLT_MAX));
    ASSERT_LOAD(&bigint, KEFIR_INT64_MIN);

    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint, 1024));
    ASSERT_OK(kefir_bigint_signed_from_double(&bigint, FLT_MAX));
    char buf[128];
    ASSERT_OK(kefir_bigint_unsigned_format10_into(&kft_mem, &bigint, buf, sizeof(buf)));
    ASSERT(strcmp(buf, "340282346638528859811704183484516925440") == 0);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
}
END_CASE

DEFINE_CASE(bigint_long_double_to_signed1, "BigInt - long double to signed conversion #1") {
    struct kefir_bigint bigint;

    ASSERT_OK(kefir_bigint_init(&bigint));

#define ASSERT_CAST(_arg, _width)                                                                           \
    do {                                                                                                    \
        ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint, sizeof(kefir_int##_width##_t) * CHAR_BIT)); \
                                                                                                            \
        ASSERT_OK(kefir_bigint_signed_from_long_double(&bigint, (_arg)));                                   \
        ASSERT_LOAD(&bigint, (kefir_int##_width##_t)(_arg));                                                \
    } while (0)

    for (kefir_long_double_t i = -100.0; i < 100.0; i += 0.25) {
        ASSERT_CAST(i, 8);
        ASSERT_CAST(i * 1e-1, 8);
        ASSERT_CAST(i, 16);
        ASSERT_CAST(i * 1e-1, 16);
        ASSERT_CAST(i, 32);
        ASSERT_CAST(i * 1e-1, 32);
        ASSERT_CAST(i, 64);
        ASSERT_CAST(i * 1e-1, 64);
        ASSERT_CAST(i * 1e3, 32);
        ASSERT_CAST(i * 1e5, 32);
        ASSERT_CAST(i * 1e3, 64);
        ASSERT_CAST(i * 1e5, 64);
    }

    ASSERT_CAST((kefir_long_double_t) KEFIR_INT8_MIN, 8);
    ASSERT_CAST((kefir_long_double_t) KEFIR_INT8_MAX, 8);
    ASSERT_CAST((kefir_long_double_t) KEFIR_INT8_MIN, 16);
    ASSERT_CAST((kefir_long_double_t) KEFIR_INT8_MAX, 16);
    ASSERT_CAST((kefir_long_double_t) KEFIR_INT8_MIN, 32);
    ASSERT_CAST((kefir_long_double_t) KEFIR_INT8_MAX, 32);
    ASSERT_CAST((kefir_long_double_t) KEFIR_INT8_MIN, 64);
    ASSERT_CAST((kefir_long_double_t) KEFIR_INT8_MAX, 64);
    ASSERT_CAST((kefir_long_double_t) KEFIR_INT16_MIN, 16);
    ASSERT_CAST((kefir_long_double_t) KEFIR_INT16_MAX, 16);
    ASSERT_CAST((kefir_long_double_t) KEFIR_INT16_MIN, 32);
    ASSERT_CAST((kefir_long_double_t) KEFIR_INT16_MAX, 32);
    ASSERT_CAST((kefir_long_double_t) KEFIR_INT16_MIN, 64);
    ASSERT_CAST((kefir_long_double_t) KEFIR_INT16_MAX, 64);
    ASSERT_CAST((kefir_long_double_t) KEFIR_INT32_MIN, 32);
    ASSERT_CAST((kefir_long_double_t) KEFIR_INT32_MAX, 32);
    ASSERT_CAST((kefir_long_double_t) KEFIR_INT32_MIN, 64);
    ASSERT_CAST((kefir_long_double_t) KEFIR_INT32_MAX, 64);
    ASSERT_CAST((kefir_long_double_t) KEFIR_INT64_MIN, 64);
    ASSERT_CAST((kefir_long_double_t) KEFIR_INT64_MAX, 64);

#undef ASSERT_CAST

    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint, sizeof(kefir_int64_t) * CHAR_BIT));
    ASSERT_OK(kefir_bigint_signed_from_long_double(&bigint, FLT_MAX));
    ASSERT_LOAD(&bigint, KEFIR_INT64_MAX);
    ASSERT_OK(kefir_bigint_signed_from_long_double(&bigint, -FLT_MAX));
    ASSERT_LOAD(&bigint, KEFIR_INT64_MIN);

    ASSERT_OK(kefir_bigint_resize_nocast(&kft_mem, &bigint, 1024));
    ASSERT_OK(kefir_bigint_signed_from_long_double(&bigint, FLT_MAX));
    char buf[128];
    ASSERT_OK(kefir_bigint_unsigned_format10_into(&kft_mem, &bigint, buf, sizeof(buf)));
    ASSERT(strcmp(buf, "340282346638528859811704183484516925440") == 0);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &bigint));
}
END_CASE

#undef ASSERT_STORE
#undef ASSERT_USTORE
#undef ASSERT_LOAD
