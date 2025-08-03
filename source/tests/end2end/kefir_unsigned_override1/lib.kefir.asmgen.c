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

typedef char chr_t;
typedef signed char schr_t;
typedef unsigned char uchr_t;
typedef signed short sshort_t;
typedef unsigned short ushort_t;
typedef signed int sint_t;
typedef unsigned int uint_t;
typedef signed long slong_t;
typedef unsigned long ulong_t;
typedef signed long long sllong_t;
typedef unsigned long long ullong_t;
typedef signed _BitInt(100) sbitint_t;
typedef unsigned _BitInt(100) ubitint_t;
typedef __typeof_unqual__((const volatile long) 100) tst1_t;
typedef __typeof_unqual__((const volatile char) 100) tst2_t;

#define ASSERT_TYPE(_type1, _type2) \
    _Static_assert(_Generic(_type1, _type2 : 1, default : 0))

ASSERT_TYPE(__kefir_unsigned_override chr_t, unsigned char);
ASSERT_TYPE(__kefir_unsigned_override schr_t, unsigned char);
ASSERT_TYPE(__kefir_unsigned_override uchr_t, unsigned char);
ASSERT_TYPE(__kefir_unsigned_override sshort_t, unsigned short);
ASSERT_TYPE(__kefir_unsigned_override ushort_t, unsigned short);
ASSERT_TYPE(__kefir_unsigned_override sint_t, unsigned int);
ASSERT_TYPE(__kefir_unsigned_override uint_t, unsigned int);
ASSERT_TYPE(__kefir_unsigned_override slong_t, unsigned long);
ASSERT_TYPE(__kefir_unsigned_override ulong_t, unsigned long);
ASSERT_TYPE(__kefir_unsigned_override sllong_t, unsigned long long);
ASSERT_TYPE(__kefir_unsigned_override ullong_t, unsigned long long);
ASSERT_TYPE(__kefir_unsigned_override sbitint_t, unsigned _BitInt(100));
ASSERT_TYPE(__kefir_unsigned_override ubitint_t, unsigned _BitInt(100));
ASSERT_TYPE(__kefir_unsigned_override tst1_t, unsigned long);
ASSERT_TYPE(__kefir_unsigned_override tst2_t, unsigned char);
