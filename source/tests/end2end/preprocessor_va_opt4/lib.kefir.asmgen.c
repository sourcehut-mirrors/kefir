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

#define PRE(x, ...) __VA_OPT__(prefix_)##x(__VA_ARGS__)
#define POST(x, ...) x##__VA_OPT__(_postfix)(__VA_ARGS__)
#define BOTH(x, ...) __VA_OPT__(prefix)##__VA_OPT__(_postfix)(__VA_ARGS__)
#define TRIPLET(x, ...) __VA_OPT__(prefix)##__VA_OPT__(_middle)##__VA_OPT__(_postfix)(__VA_ARGS__)
#define CHAIN(x, ...) __VA_OPT__(prefix)##__VA_OPT__(_middle)##__VA_OPT__(_another)##__VA_OPT__(_postfix)(__VA_ARGS__)
#define CHAIN2(x, ...) __VA_OPT__(prefix)##__VA_OPT__(_middle_)##x##__VA_OPT__(_another)##__VA_OPT__(_postfix)(__VA_ARGS__)

PRE(a)
PRE(a, y)

POST(a)
POST(a, y)

BOTH(a)
BOTH(a, y)

TRIPLET(a)
TRIPLET(a, y)

CHAIN(a)
CHAIN(a, y)

CHAIN2(a)
CHAIN2(a, y)