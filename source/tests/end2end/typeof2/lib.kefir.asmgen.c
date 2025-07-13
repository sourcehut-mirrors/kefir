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

#define ASSERT_TYPE(_expr, _type1) _Static_assert(_Generic(_expr, _type1: 1, default: 0))

ASSERT_TYPE(__typeof__(1), int);
ASSERT_TYPE(__typeof__((const int) {1}), const int);
ASSERT_TYPE(__typeof__((const volatile int) {1}), const volatile int);
ASSERT_TYPE(__typeof__((_Atomic int) {1}), _Atomic int);

ASSERT_TYPE(__typeof_unqual__(1), int);
ASSERT_TYPE(__typeof_unqual__((const int) {1}), int);
ASSERT_TYPE(__typeof_unqual__((const volatile int) {1}), int);
ASSERT_TYPE(__typeof_unqual__((_Atomic int) {1}), int);

ASSERT_TYPE(__typeof__(int), int);
ASSERT_TYPE(__typeof__(const int), const int);
ASSERT_TYPE(__typeof__(const volatile int), const volatile int);
ASSERT_TYPE(__typeof__(_Atomic int), _Atomic int);

ASSERT_TYPE(__typeof_unqual__(int), int);
ASSERT_TYPE(__typeof_unqual__(const int), int);
ASSERT_TYPE(__typeof_unqual__(const volatile int), int);
ASSERT_TYPE(__typeof_unqual__(_Atomic int), int);
