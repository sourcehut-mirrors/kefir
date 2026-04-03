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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

struct i128 {
    unsigned long arr[2];
};

#if !defined(__DragonFly__) || defined(KEFIR_END2END_ASMGEN)
struct i128 load_i128(_Atomic struct i128 *);
struct i128 load_u128(_Atomic struct i128 *);

void store_i128(_Atomic struct i128 *, struct i128);
void store_u128(_Atomic struct i128 *, struct i128);

struct i128 add_i128(_Atomic struct i128 *, struct i128);
struct i128 add_u128(_Atomic struct i128 *, struct i128);
#endif

#endif
