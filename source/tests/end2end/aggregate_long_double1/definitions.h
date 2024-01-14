/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

struct Param1 {
    long double value;
};

struct Param2 {
    long double x;
    long double y;
};

struct Param1 ldneg(struct Param1);
struct Param1 ldsum(struct Param2);

struct Param1 ldvsum(int, ...);
struct Param1 ldvsum2(int, ...);

long double ldunwrap(struct Param1 (*)(void));

#endif
