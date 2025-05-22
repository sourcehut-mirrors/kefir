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

DEFINE_CASE(bigint_basic1, "BigInt - load/store machine integer #1") {
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

DEFINE_CASE(bigint_cast_signed1, "BigInt - signed cast #1") {
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

DEFINE_CASE(bigint_add1, "BigInt - addition #1") {
    struct kefir_bigint lhs_bigint, rhs_bigint;

    ASSERT_OK(kefir_bigint_init(&lhs_bigint));
    ASSERT_OK(kefir_bigint_init(&rhs_bigint));

#define ASSERT_ADD(_lhs, _rhs, _arg1, _arg2, _res)                                                          \
    do {                                                                                                    \
        ASSERT_STORE((_lhs), (_arg1));                                                                      \
        ASSERT_STORE((_rhs), (_arg2));                                                                      \
        ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, (_lhs), MAX((_lhs)->bitwidth, (_rhs)->bitwidth) + 1)); \
        ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, (_rhs), (_lhs)->bitwidth));                            \
        ASSERT_OK(kefir_bigint_add((_lhs), (_rhs)));                                                        \
        ASSERT_LOAD((_lhs), (_res));                                                                        \
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
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_add(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, 0);
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_add(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, KEFIR_INT64_MIN);
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_add(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, 0);

    ASSERT_STORE(&lhs_bigint, KEFIR_UINT64_MAX);
    ASSERT_STORE(&rhs_bigint, KEFIR_UINT64_MAX);
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_add(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, (kefir_int64_t) -2);
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_add(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, (kefir_int64_t) -3);
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_add(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, (kefir_int64_t) -4);
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
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

#define ASSERT_AND(_lhs, _rhs, _arg1, _arg2)                                                                \
    do {                                                                                                    \
        ASSERT_STORE((_lhs), (_arg1));                                                                      \
        ASSERT_STORE((_rhs), (_arg2));                                                                      \
        ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, (_lhs), MAX((_lhs)->bitwidth, (_rhs)->bitwidth) + 1)); \
        ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, (_rhs), (_lhs)->bitwidth));                            \
        ASSERT_OK(kefir_bigint_and((_lhs), (_rhs)));                                                        \
        ASSERT_LOAD((_lhs), ((kefir_int64_t) (_arg1)) & (_arg2));                                           \
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

#define ASSERT_OR(_lhs, _rhs, _arg1, _arg2)                                                                 \
    do {                                                                                                    \
        ASSERT_STORE((_lhs), (_arg1));                                                                      \
        ASSERT_STORE((_rhs), (_arg2));                                                                      \
        ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, (_lhs), MAX((_lhs)->bitwidth, (_rhs)->bitwidth) + 1)); \
        ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, (_rhs), (_lhs)->bitwidth));                            \
        ASSERT_OK(kefir_bigint_or((_lhs), (_rhs)));                                                         \
        ASSERT_LOAD((_lhs), ((kefir_int64_t) (_arg1)) | (_arg2));                                           \
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

#define ASSERT_XOR(_lhs, _rhs, _arg1, _arg2)                                                                \
    do {                                                                                                    \
        ASSERT_STORE((_lhs), (_arg1));                                                                      \
        ASSERT_STORE((_rhs), (_arg2));                                                                      \
        ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, (_lhs), MAX((_lhs)->bitwidth, (_rhs)->bitwidth) + 1)); \
        ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, (_rhs), (_lhs)->bitwidth));                            \
        ASSERT_OK(kefir_bigint_xor((_lhs), (_rhs)));                                                        \
        ASSERT_LOAD((_lhs), ((kefir_int64_t) (_arg1)) ^ (_arg2));                                           \
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

#define ASSERT_NEGATE(_bigint, _arg)                     \
    do {                                                 \
        ASSERT_STORE((_bigint), (_arg));                 \
        ASSERT_OK(kefir_bigint_negate((_bigint)));       \
        ASSERT_LOAD((_bigint), -(kefir_int64_t) (_arg)); \
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

#define ASSERT_SUB(_lhs, _rhs, _arg1, _arg2, _res)                                                          \
    do {                                                                                                    \
        ASSERT_STORE((_lhs), (_arg1));                                                                      \
        ASSERT_STORE((_rhs), (_arg2));                                                                      \
        ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, (_lhs), MAX((_lhs)->bitwidth, (_rhs)->bitwidth) + 1)); \
        ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, (_rhs), (_lhs)->bitwidth));                            \
        ASSERT_OK(kefir_bigint_subtract((_lhs), (_rhs)));                                                   \
        ASSERT_LOAD((_lhs), (_res));                                                                        \
    } while (0)

    ASSERT_SUB(&lhs_bigint, &rhs_bigint, 0, 0, 0);
    ASSERT_SUB(&lhs_bigint, &rhs_bigint, 1000, 2000, -1000);
    ASSERT_SUB(&lhs_bigint, &rhs_bigint, 1000, -2000, 3000);
    ASSERT_SUB(&lhs_bigint, &rhs_bigint, KEFIR_UINT32_MAX, KEFIR_UINT32_MAX, 0);
    ASSERT_SUB(&lhs_bigint, &rhs_bigint, KEFIR_INT32_MIN, KEFIR_INT32_MIN, 0);
    ASSERT_SUB(&lhs_bigint, &rhs_bigint, KEFIR_INT32_MIN, KEFIR_INT32_MAX,
               (kefir_int64_t) KEFIR_INT32_MIN - KEFIR_INT32_MAX);
    ASSERT_SUB(&lhs_bigint, &rhs_bigint, KEFIR_UINT64_MIN, KEFIR_UINT64_MAX, 1);

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
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_subtract(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, 1);
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_subtract(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, -KEFIR_INT64_MAX + 1);
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_subtract(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, 3);
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_subtract(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, -KEFIR_INT64_MAX + 3);
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_subtract(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, 5);

    ASSERT_STORE(&lhs_bigint, 0);
    ASSERT_STORE(&rhs_bigint, KEFIR_UINT64_MAX);
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_subtract(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, (kefir_int64_t) 1);
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_subtract(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, (kefir_int64_t) 2);
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_subtract(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, (kefir_int64_t) 3);
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &lhs_bigint, MAX(lhs_bigint.bitwidth, rhs_bigint.bitwidth) + 1));
    ASSERT_OK(kefir_bigint_cast_signed(&kft_mem, &rhs_bigint, lhs_bigint.bitwidth));
    ASSERT_OK(kefir_bigint_subtract(&lhs_bigint, &rhs_bigint));
    ASSERT_LOAD(&lhs_bigint, (kefir_int64_t) 4);

    ASSERT_OK(kefir_bigint_free(&kft_mem, &lhs_bigint));
    ASSERT_OK(kefir_bigint_free(&kft_mem, &rhs_bigint));
}
END_CASE

#undef ASSERT_STORE
#undef ASSERT_LOAD