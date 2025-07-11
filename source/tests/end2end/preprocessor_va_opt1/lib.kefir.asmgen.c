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

#define TEST(x, ...) test(x __VA_OPT__(, ) __VA_ARGS__)
#define EMPTY

TEST(1)
TEST(1, )
TEST(1, EMPTY)
TEST(1, 2)
TEST(1, 2, 3)

#define TEST2(x, ...) test(x __VA_OPT__(((, ))()(())) __VA_ARGS__)

TEST2(1)
TEST2(1, )
TEST2(1, EMPTY)
TEST2(1, 2)
TEST2(1, 2, 3)

#define TEST3(x, ...) TEST(x __VA_OPT__(, ) __VA_ARGS__)

TEST3(1)
TEST3(1, )
TEST3(1, EMPTY)
TEST3(1, 2)
TEST3(1, 2, 3)

#define TEST4(x, ...) test(x __VA_OPT__(, ) __VA_OPT__() __VA_OPT__(y) __VA_OPT__(, ) __VA_ARGS__)

TEST4(1)
TEST4(1, )
TEST4(1, EMPTY)
TEST4(1, 2)
TEST4(1, 2, 3)

#define TEST5(...) __VA_OPT__(test)(__VA_ARGS__)
TEST5()
TEST5(EMPTY)
TEST5(1)
TEST5(1, 2)
TEST5(1, 2, 3)