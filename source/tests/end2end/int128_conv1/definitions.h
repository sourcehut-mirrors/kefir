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

struct i128 {
    unsigned long arr[2];
};

_Bool i128_to_bool(struct i128);
_Bool u128_to_bool(struct i128);
long i128_to_i64(struct i128);
unsigned long i128_to_u64(struct i128);
long u128_to_i64(struct i128);
unsigned long u128_to_u64(struct i128);
struct i128 i64_to_int128(long);
struct i128 u64_to_int128(unsigned long);
struct i128 i64_to_unt128(long);
struct i128 u64_to_unt128(unsigned long);

#endif
