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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

char test_atomic_exchange8(_Atomic char *, char);
short test_atomic_exchange16(_Atomic short *, short);
int test_atomic_exchange32(_Atomic int *, int);
long test_atomic_exchange64(_Atomic long *, long);

char test2_atomic_exchange8(_Atomic char *, char);
short test2_atomic_exchange16(_Atomic short *, short);
int test2_atomic_exchange32(_Atomic int *, int);
long test2_atomic_exchange64(_Atomic long *, long);

#endif
