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

#define TEST(hi) { .h##i = (hi) }
#define TEST2(a, b, c, abc) { .a##b##c = abc }
#define TEST3(a, b, c, d, e, f, abcdef) { .a##b##c##d##e##f = abcdef }

TEST(1)
TEST(A)
TEST(hi)
TEST2(x, y, z, 1000)
TEST2(a, b, c, 1001)
TEST2(X,, Y, abc)
TEST3(x, y, z, g, h, i, 6)
TEST3(x, y, z, g, h, i, xyzghi)
TEST3(x, y, z, g, h, i, abcdef)
TEST3(x, ,, a,, d, 5)