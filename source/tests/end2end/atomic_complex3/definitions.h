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

#define DECL(_op, _sz, _type) _type _op##_##_sz(_Atomic _type *, _type)

#define DECL_ALL(_op)               \
    DECL(_op, f32, _Complex float); \
    DECL(_op, f64, _Complex double)
/* DECL(_op, ld, _Complex long double) */

DECL_ALL(multiply);
DECL_ALL(divide);
DECL_ALL(add);
DECL_ALL(subtract);

#undef DECL_ALL
#undef DECL

#endif
