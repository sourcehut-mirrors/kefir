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

#include "./definitions.h"

#define DEFINE_LOAD(_type)                               \
    _type test_load_##_type(const atomic_##_type *ptr) { \
        return atomic_load(ptr);                         \
    }

DEFINE_LOAD(char)
DEFINE_LOAD(short)
DEFINE_LOAD(int)
DEFINE_LOAD(long)

#define DEFINE_STORE(_type)                                           \
    void test_store_##_type(const atomic_##_type *ptr, _type value) { \
        atomic_store(ptr, value);                                     \
    }

DEFINE_STORE(char)
DEFINE_STORE(short)
DEFINE_STORE(int)
DEFINE_STORE(long)

#define DEFINE_EXCHANGE(_type)                                            \
    _type test_exchange_##_type(const atomic_##_type *ptr, _type value) { \
        return atomic_exchange(ptr, value);                               \
    }

DEFINE_EXCHANGE(char)
DEFINE_EXCHANGE(short)
DEFINE_EXCHANGE(int)
DEFINE_EXCHANGE(long)

#define DEFINE_COMPARE_EXCHANGE(_type)                                                             \
    _Bool test_compare_exchange_##_type(const atomic_##_type *ptr, _type *expected, _type value) { \
        return atomic_compare_exchange_strong(ptr, expected, value);                               \
    }

DEFINE_COMPARE_EXCHANGE(char)
DEFINE_COMPARE_EXCHANGE(short)
DEFINE_COMPARE_EXCHANGE(int)
DEFINE_COMPARE_EXCHANGE(long)

#define DEFINE_FETCH_OP(_type, _op)                                            \
    _type test_fetch_##_op##_##_type(const atomic_##_type *ptr, _type value) { \
        return atomic_fetch_##_op(ptr, value);                                 \
    }

DEFINE_FETCH_OP(char, add)
DEFINE_FETCH_OP(short, add)
DEFINE_FETCH_OP(int, add)
DEFINE_FETCH_OP(long, add)

DEFINE_FETCH_OP(char, sub)
DEFINE_FETCH_OP(short, sub)
DEFINE_FETCH_OP(int, sub)
DEFINE_FETCH_OP(long, sub)

DEFINE_FETCH_OP(char, or)
DEFINE_FETCH_OP(short, or)
DEFINE_FETCH_OP(int, or)
DEFINE_FETCH_OP(long, or)

DEFINE_FETCH_OP(char, xor)
DEFINE_FETCH_OP(short, xor)
DEFINE_FETCH_OP(int, xor)
DEFINE_FETCH_OP(long, xor)

DEFINE_FETCH_OP(char, and)
DEFINE_FETCH_OP(short, and)
DEFINE_FETCH_OP(int, and)
DEFINE_FETCH_OP(long, and)

void test_atomic_thread_fence(void) {
    atomic_thread_fence(memory_order_seq_cst);
}

void test_atomic_signal_fence(void) {
    atomic_signal_fence(memory_order_seq_cst);
}

_Bool test_is_lock_free(void) {
    return atomic_is_lock_free((_Bool *) 0) == ATOMIC_BOOL_LOCK_FREE &&
           atomic_is_lock_free((char *) 0) == ATOMIC_CHAR_LOCK_FREE &&
           atomic_is_lock_free((__CHAR16_TYPE__ *) 0) == ATOMIC_CHAR16_T_LOCK_FREE &&
           atomic_is_lock_free((__CHAR32_TYPE__ *) 0) == ATOMIC_CHAR32_T_LOCK_FREE &&
           atomic_is_lock_free((__WCHAR_TYPE__ *) 0) == ATOMIC_WCHAR_T_LOCK_FREE &&
           atomic_is_lock_free((short *) 0) == ATOMIC_SHORT_LOCK_FREE &&
           atomic_is_lock_free((int *) 0) == ATOMIC_INT_LOCK_FREE &&
           atomic_is_lock_free((long *) 0) == ATOMIC_LONG_LOCK_FREE &&
           atomic_is_lock_free((long long *) 0) == ATOMIC_LLONG_LOCK_FREE &&
           atomic_is_lock_free((void **) 0) == ATOMIC_POINTER_LOCK_FREE;
}

_Bool test_and_set(atomic_flag *ptr) {
    return atomic_flag_test_and_set(ptr);
}

void clear(atomic_flag *ptr) {
    atomic_flag_clear(ptr);
}
