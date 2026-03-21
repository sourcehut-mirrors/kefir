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

typedef int a;

a const *b;
_Alignas(32) a const *c;
a _Alignas(32) const *d;
a const _Alignas(32) * e;

_Static_assert(_Alignof(b) == _Alignof(void *));
_Static_assert(_Alignof(c) == 32);
_Static_assert(_Alignof(d) == 32);
_Static_assert(_Alignof(e) == 32);

struct {
    char x;
} *h;
_Alignas(32) struct {
    char x;
} *i;

_Static_assert(_Alignof(h) == _Alignof(void *));
_Static_assert(_Alignof(i) == 32);

enum { A } * n;
_Alignas(32) enum { B } * o;

_Static_assert(_Alignof(n) == _Alignof(void *));
_Static_assert(_Alignof(o) == 32);
