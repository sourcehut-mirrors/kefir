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

#define DECL(_op, _sz, _type) _type _op##_##_sz(_Atomic _type *, _type)

#define DECL_INT(_op)      \
    DECL(_op, i8, char);   \
    DECL(_op, i16, short); \
    DECL(_op, i32, int);   \
    DECL(_op, i64, long)
#define DECL_ALL_SIZES(_op) \
    DECL_INT(_op);          \
    DECL(_op, f32, float);  \
    DECL(_op, f64, double); \
    DECL(_op, ld, long double)

DECL_ALL_SIZES(multiply);
DECL_ALL_SIZES(divide);
DECL_ALL_SIZES(add);
DECL_ALL_SIZES(subtract);
DECL_INT(modulo);
DECL_INT(shl);
DECL_INT(shr);
DECL_INT(iand);
DECL_INT(ior);
DECL_INT(ixor);

#undef DECL_ALL_SIZES
#undef DECL_INT
#undef DECL

int *add_ptr(_Atomic(int *) *, long);
int *subtract_ptr(_Atomic(int *) *, long);

#endif
