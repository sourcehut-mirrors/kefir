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

constexpr char a = 'A';
constexpr short b = 52;
constexpr int c = 52'0;
constexpr long d = 52'0381l;
constexpr float e = 311.04f;
constexpr double f = 311.04;
constexpr _BitInt(30) g = 30wb;
constexpr unsigned _BitInt(31) h = 30uwb;
constexpr char i[] = "Hello, world!";
constexpr struct S1 {
    int a;
} j = {};
constexpr union S2 {
    int a;
} k = {};
constexpr _Complex float l = 0.0f;
constexpr typeof(a) *m = &a;
constexpr typeof_unqual(a) *n = &a;
constexpr auto o = &b;

#define ASSERT_TYPE(_expr, _type) static_assert(_Generic(_expr, _type: 1, default: 0))

ASSERT_TYPE(typeof(a) *, const char *);
ASSERT_TYPE(typeof(b) *, const short *);
ASSERT_TYPE(typeof(c) *, const int *);
ASSERT_TYPE(typeof(d) *, const long *);
ASSERT_TYPE(typeof(e) *, const float *);
ASSERT_TYPE(typeof(f) *, const double *);
ASSERT_TYPE(typeof(g) *, const _BitInt(30) *);
ASSERT_TYPE(typeof(h) *, const unsigned _BitInt(31) *);
ASSERT_TYPE(i[0], char);
ASSERT_TYPE(typeof(j) *, const struct S1 *);
ASSERT_TYPE(typeof(k) *, const union S2 *);
ASSERT_TYPE(typeof(l) *, const _Complex float *);
ASSERT_TYPE(typeof(m) *, const char *const *);
ASSERT_TYPE(typeof(n) *, char *const *);
ASSERT_TYPE(typeof(o) *, const short *const *);
