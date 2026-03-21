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

typedef int a __attribute__((aligned(32)));

a const *b;
__attribute__((aligned(32))) a const *c;
a __attribute__((aligned(32))) const *d;
a const __attribute__((aligned(32))) * e;
a const *__attribute__((aligned(32))) f;
a const *g __attribute__((aligned(32)));

_Static_assert(_Alignof(b) == _Alignof(void *));
_Static_assert(_Alignof(c) == 32);
_Static_assert(_Alignof(d) == 32);
_Static_assert(_Alignof(e) == 32);
_Static_assert(_Alignof(f) == 32);
_Static_assert(_Alignof(g) == 32);

struct {
    char x;
} *h;
__attribute__((aligned(32))) struct {
    char x;
} *i;
struct __attribute__((aligned(32))) {
    char x;
} *j;
struct {
    char x;
} __attribute__((aligned(32))) * k;
struct {
    char x;
} *__attribute__((aligned(32))) l;
struct {
    char x;
} *m __attribute__((aligned(32)));

_Static_assert(_Alignof(h) == _Alignof(void *));
_Static_assert(_Alignof(i) == 32);
_Static_assert(_Alignof(j) == _Alignof(void *));
_Static_assert(_Alignof(k) == _Alignof(void *));
_Static_assert(_Alignof(l) == 32);
_Static_assert(_Alignof(m) == 32);

enum { A } * n;
__attribute__((aligned(32))) enum { B } * o;
enum __attribute__((aligned(32))) { C } * p;
enum { D } __attribute__((aligned(32))) * q;
enum { E } *__attribute__((aligned(32))) r;
enum { F } * s __attribute__((aligned(32)));

_Static_assert(_Alignof(n) == _Alignof(void *));
_Static_assert(_Alignof(o) == 32);
_Static_assert(_Alignof(p) == _Alignof(void *));
_Static_assert(_Alignof(q) == _Alignof(void *));
_Static_assert(_Alignof(r) == 32);
_Static_assert(_Alignof(s) == 32);
