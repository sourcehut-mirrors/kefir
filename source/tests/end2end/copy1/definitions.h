/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#define REPEAT1(_fn) _fn(__COUNTER__)
#define REPEAT2(_fn) REPEAT1(_fn) REPEAT1(_fn)
#define REPEAT4(_fn) REPEAT2(_fn) REPEAT2(_fn)
#define REPEAT8(_fn) REPEAT4(_fn) REPEAT4(_fn)
#define REPEAT16(_fn) REPEAT8(_fn) REPEAT8(_fn)
#define REPEAT32(_fn) REPEAT16(_fn) REPEAT16(_fn)
#define REPEAT64(_fn) REPEAT32(_fn) REPEAT32(_fn)
#define REPEAT128(_fn) REPEAT64(_fn) REPEAT64(_fn)

#ifdef MAIN
#define TEST_HELPER(_count)                                                   \
    struct buf_struct_##_count {                                              \
        unsigned char buffer[_count + 1];                                     \
    };                                                                        \
    extern struct buf_struct_##_count generate_buf_##_count(unsigned char);   \
    static void test_##_count(unsigned char base, void (*assert_fn)(_Bool)) { \
        struct buf_struct_##_count res = generate_buf_##_count(base);         \
        for (unsigned char i = 0; i < _count + 1; i++) {                      \
            assert_fn(res.buffer[i] == i + base);                             \
        }                                                                     \
    }
#define TEST(_count) TEST_HELPER(_count)
REPEAT128(TEST)
#undef TEST
#undef TEST_HELPER
#else
#define TEST_HELPER(_count)                                             \
    struct buf_struct_##_count {                                        \
        unsigned char buffer[_count + 1];                               \
    };                                                                  \
    struct buf_struct_##_count generate_buf_##_count(unsigned char x) { \
        struct buf_struct_##_count res = {0};                           \
        for (unsigned char i = 0; i < _count + 1; i++) {                \
            res.buffer[i] = x + i;                                      \
        }                                                               \
        return res;                                                     \
    }
#define TEST(_count) TEST_HELPER(_count)
REPEAT128(TEST)
#undef TEST
#undef TEST_HELPER
#endif

#endif
