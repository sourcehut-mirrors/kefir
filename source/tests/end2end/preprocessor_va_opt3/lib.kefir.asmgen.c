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

#define STRINGIFY1(arg1, ...) #__VA_OPT__(arg1##arg1 arg1##arg1 arg1##arg1 arg1)
#define EMPTY
STRINGIFY1(hello)
STRINGIFY1(, there)
STRINGIFY1(world, here)
STRINGIFY1(bye, EMPTY)

#define VAR_DEFINE(type, name, ...) type name __VA_OPT__(= {__VA_ARGS__})
VAR_DEFINE(int, hello);
VAR_DEFINE(int[], world, 1, 2, 3, 4, 5);

#define DUMMY(...) __VA_OPT__(BBB) __VA_OPT__() __VA_OPT__(AAA) __VA_OPT__()
#define CONCAT(a, X, b) a##X##b
#define UNWRAP(a, X, b) CONCAT(a, X, b)
UNWRAP(x, DUMMY(), y)
UNWRAP(x, DUMMY(1000), y)

#define CONCAT_FIRST_THREE(X, Y, Z, ...) test(__VA_OPT__(X##Y##Z, ) __VA_ARGS__)
CONCAT_FIRST_THREE(HELLO, CRUEL, WORLD, I, AM HERE)

#define MORE_CONCAT(X, Y, Z, ...) __VA_OPT__(a X##Y)##Z
MORE_CONCAT(1, 2, 3)
MORE_CONCAT(1, 2, 3, EMPTY)
MORE_CONCAT(1, 2, 3, 4)