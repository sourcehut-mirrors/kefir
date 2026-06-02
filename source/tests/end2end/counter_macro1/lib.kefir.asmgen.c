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

#define TEST(X) X X X

TEST(__COUNTER__)
TEST(__COUNTER__)

#define TEST2(X, Y, Z) X X Z Z

TEST2(__COUNTER__, __COUNTER__, __COUNTER__)
TEST2(__COUNTER__, __COUNTER__, __COUNTER__)

#define TEST3_(X, Y, Z, W) #X #Y #Z #W
#define TEST3(X, Y, Z) TEST3_(X, X, Z, Z)
TEST3(__COUNTER__, __COUNTER__, __COUNTER__)
TEST3(__COUNTER__, __COUNTER__, __COUNTER__)

#define TEST4_(X, Y, Z, W) X##Z W##Y
#define TEST4(X, Y, Z) TEST4_(X, X, Z, Z)
TEST4(__COUNTER__, __COUNTER__, __COUNTER__)
TEST4(__COUNTER__, __COUNTER__, __COUNTER__)

#define TEST5(X, ...) X X __VA_ARGS__ __VA_ARGS__
TEST5(__COUNTER__, __COUNTER__ __COUNTER__)
TEST5(__COUNTER__, __COUNTER__ __COUNTER__)

#define TEST6(X, Y...) X X Y Y
TEST6(__COUNTER__, __COUNTER__ __COUNTER__)
TEST6(__COUNTER__, __COUNTER__ __COUNTER__)

#define TEST7(X, ...) X X __VA_OPT__(X)
TEST7(__COUNTER__, __COUNTER__)
TEST7(__COUNTER__, __COUNTER__)

#define TEST9(X, ...)         \
    X X __VA_OPT__(X)         \
    __VA_ARGS__ __VA_OPT__(X) \
    __VA_ARGS__
TEST9(__COUNTER__, __COUNTER__)
TEST9(__COUNTER__, __COUNTER__)
