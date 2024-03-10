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

#include <stdatomic.h>

#define DECL_LOAD(_type) _type test_load_##_type(const atomic_##_type *);
DECL_LOAD(char)
DECL_LOAD(short)
DECL_LOAD(int)
DECL_LOAD(long)
#undef DECL_LOAD

#define DECL_STORE(_type) void test_store_##_type(const atomic_##_type *, _type);
DECL_STORE(char)
DECL_STORE(short)
DECL_STORE(int)
DECL_STORE(long)
#undef DECL_STORE

#define DECL_EXCHANGE(_type) _type test_exchange_##_type(const atomic_##_type *, _type);
DECL_EXCHANGE(char)
DECL_EXCHANGE(short)
DECL_EXCHANGE(int)
DECL_EXCHANGE(long)
#undef DECL_EXCHANGE

#define DECL_COMPARE_EXCHANGE(_type) _Bool test_compare_exchange_##_type(const atomic_##_type *, _type *, _type);
DECL_COMPARE_EXCHANGE(char)
DECL_COMPARE_EXCHANGE(short)
DECL_COMPARE_EXCHANGE(int)
DECL_COMPARE_EXCHANGE(long)
#undef DECL_EXCHANGE

#define DECL_FETCH_OP(_type, _op) _type test_fetch_##_op##_##_type(const atomic_##_type *, _type);
DECL_FETCH_OP(char, add)
DECL_FETCH_OP(short, add)
DECL_FETCH_OP(int, add)
DECL_FETCH_OP(long, add)

DECL_FETCH_OP(char, sub)
DECL_FETCH_OP(short, sub)
DECL_FETCH_OP(int, sub)
DECL_FETCH_OP(long, sub)

DECL_FETCH_OP(char, or)
DECL_FETCH_OP(short, or)
DECL_FETCH_OP(int, or)
DECL_FETCH_OP(long, or)

DECL_FETCH_OP(char, xor)
DECL_FETCH_OP(short, xor)
DECL_FETCH_OP(int, xor)
DECL_FETCH_OP(long, xor)

DECL_FETCH_OP(char, and)
DECL_FETCH_OP(short, and)
DECL_FETCH_OP(int, and)
DECL_FETCH_OP(long, and)
#undef DECL_EXCHANGE

void test_atomic_thread_fence(void);
void test_atomic_signal_fence(void);
_Bool test_is_lock_free(void);

_Bool test_and_set(atomic_flag *);
void clear(atomic_flag *);

#endif
