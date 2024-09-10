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

#define DECL_FETCH(_id, _type) _type test_fetch_##_id(_Atomic _type *, _type)

DECL_FETCH(add_char, char);
DECL_FETCH(add_short, short);
DECL_FETCH(add_int, int);
DECL_FETCH(add_long, long);

DECL_FETCH(sub_char, char);
DECL_FETCH(sub_short, short);
DECL_FETCH(sub_int, int);
DECL_FETCH(sub_long, long);

DECL_FETCH(or_char, char);
DECL_FETCH(or_short, short);
DECL_FETCH(or_int, int);
DECL_FETCH(or_long, long);

DECL_FETCH(and_char, char);
DECL_FETCH(and_short, short);
DECL_FETCH(and_int, int);
DECL_FETCH(and_long, long);

DECL_FETCH(xor_char, char);
DECL_FETCH(xor_short, short);
DECL_FETCH(xor_int, int);
DECL_FETCH(xor_long, long);

#undef DECL_FETCH

#endif
