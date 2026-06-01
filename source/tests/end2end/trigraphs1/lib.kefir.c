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

// clang-format off
??=include "definitions.h"

??=define TEST(x, y) ??/
    x ??' y

??=define TEST2_(x, y) ??=x ??/
    ??/
    ??=y
??=define TEST2(x, y) TEST2_(x, y)

??=define TEST3_(y) y
??=define TEST3(x, y) x ??/
    ??=??= y

const char STR[] = "??=??(??/??/??)??'??<??!??>??-";
const char STR2[] = TEST2(12, TEST3(+, +));

int test1(int a, int *b) ??<
    return TEST((??-a) ??! b??(1??), '??/n');
??>
