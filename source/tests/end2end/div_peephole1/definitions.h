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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#define DECL_OP(_type, _offset) \
    _type idiv_##_type##_offset(_type); \
    unsigned _type div_##_type##_offset(unsigned _type)

DECL_OP(char, 0);
DECL_OP(char, 1);
DECL_OP(char, 2);
DECL_OP(char, 5);
DECL_OP(char, 7);

DECL_OP(short, 0);
DECL_OP(short, 1);
DECL_OP(short, 5);
DECL_OP(short, 8);
DECL_OP(short, 13);
DECL_OP(short, 15);

DECL_OP(int, 0);
DECL_OP(int, 1);
DECL_OP(int, 10);
DECL_OP(int, 16);
DECL_OP(int, 23);
DECL_OP(int, 31);

DECL_OP(long, 0);
DECL_OP(long, 1);
DECL_OP(long, 26);
DECL_OP(long, 32);
DECL_OP(long, 48);
DECL_OP(long, 63);

#undef DECL_OP

#endif
