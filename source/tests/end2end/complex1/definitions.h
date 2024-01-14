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

#define DECL_FN(_type, _id) \
    extern _type _id;       \
    extern _type get_##_id(void)

DECL_FN(_Complex float, x);
DECL_FN(float _Complex, y);
DECL_FN(_Complex double, a);
DECL_FN(double _Complex, b);
DECL_FN(_Complex long double, i);
DECL_FN(_Complex double long, j);
DECL_FN(long _Complex double, k);
DECL_FN(long double _Complex, l);
DECL_FN(double _Complex long, m);
DECL_FN(double long _Complex, n);

_Complex float nonef(void);
_Complex double none(void);
_Complex long double nonel(void);

#endif
