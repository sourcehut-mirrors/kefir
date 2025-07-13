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

volatile auto a = 5.0L;
_Thread_local const auto b = (_BitInt(42)) 1;
_Atomic auto c = (const short *) 0;
static const auto d = (const _Complex float **) 0;
static double e[] = {1.0, 2.0, 3.0};
static _Thread_local auto f = e;

#define ASSERT_TYPE(_expr, _type) _Static_assert(_Generic(_expr, _type: 1, default: 0))

ASSERT_TYPE(a, long double);
ASSERT_TYPE(b, _BitInt(42));
ASSERT_TYPE(c, const short *);
ASSERT_TYPE(d, const _Complex float **);
ASSERT_TYPE(f, double *);

void fn1(void) {
    auto x = (unsigned short) 3;
    ASSERT_TYPE(x, unsigned short);
}

void fn2(void) {
    auto int x = (unsigned short) 3;
    ASSERT_TYPE(x, int);
}

void fn3(int x) {
    for (auto y = x; y > 0; y--) {
        ASSERT_TYPE(y, __typeof__(x));
        fn1();
    }
}
