/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

void test_atomic_store8(_Atomic char *);
void test_atomic_store16(_Atomic short *);
void test_atomic_store32(_Atomic int *);
void test_atomic_store64(_Atomic long *);

char test_atomic_load8(_Atomic char *);
short test_atomic_load16(_Atomic short *);
int test_atomic_load32(_Atomic int *);
long test_atomic_load64(_Atomic long *);

char test_atomic_exchange8(_Atomic char *);
short test_atomic_exchange16(_Atomic short *);
int test_atomic_exchange32(_Atomic int *);
long test_atomic_exchange64(_Atomic long *);

_Bool test_atomic_compare_exchange8(_Atomic char *);
_Bool test_atomic_compare_exchange16(_Atomic short *);
_Bool test_atomic_compare_exchange32(_Atomic int *);
_Bool test_atomic_compare_exchange64(_Atomic long *);

char test_atomic_fetch_add8(_Atomic char *);
short test_atomic_fetch_add16(_Atomic short *);
int test_atomic_fetch_add32(_Atomic int *);
long test_atomic_fetch_add64(_Atomic long *);

#endif
