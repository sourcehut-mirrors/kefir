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

_Static_assert(0 == (void *) 0);
_Static_assert((void *) 0 == 0);
_Static_assert((void *) 0 == (void *) 0);

_Static_assert(1000 == (void *) 1000);
_Static_assert((void *) 1000 == 1000);
_Static_assert((void *) 1000 == (void *) 1000);

_Static_assert(0 != (void *) 1);
_Static_assert((void *) 1 != 0);
_Static_assert((void *) 1 != (void *) 0);

_Static_assert(1001 != (void *) 1000);
_Static_assert((void *) 1001 != 1000);
_Static_assert((void *) 1001 != (void *) 1000);

int a[40], b[40];
_Static_assert(&a[15] == (void *) &15 [a]);
_Static_assert(&a[15] == sizeof(int) + (unsigned long long) &14 [a]);
_Static_assert(&a[13] == (unsigned long long) &a[13]);

_Static_assert(&a[13] != (void *) &15 [a]);
_Static_assert(&a[13] != sizeof(int) + (unsigned long long) &14 [a]);
_Static_assert(&a[13] != (void *) &b[13]);
_Static_assert(&a[13] != (unsigned long long) &b[13]);