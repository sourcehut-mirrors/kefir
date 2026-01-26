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

struct i192 {
    unsigned long arr[3];
};

struct i128 int128_zero_extend64(void);
struct i128 int128_sign_extend64(void);
unsigned long int128_truncate_int64(void);
_Bool int128_to_bool(void);
_Bool int128_bool_not(void);
struct i128 int128_neg(void);
struct i128 int128_not(void);
struct i192 int128_sign_to_bitint(void);
struct i192 int128_unsign_to_bitint(void);
struct i128 int128_from_bitint_sign(void);
struct i128 int128_from_bitint_unsign(void);
float int128_signed_to_float(void);
float int128_unsigned_to_float(void);
double int128_signed_to_double(void);
double int128_unsigned_to_double(void);
long double int128_signed_to_long_double(void);
long double int128_unsigned_to_long_double(void);
struct i128 int128_signed_from_float(void);
struct i128 int128_unsigned_from_float(void);
struct i128 int128_signed_from_double(void);
struct i128 int128_unsigned_from_double(void);
struct i128 int128_signed_from_long_double(void);
struct i128 int128_unsigned_from_long_double(void);

#endif
