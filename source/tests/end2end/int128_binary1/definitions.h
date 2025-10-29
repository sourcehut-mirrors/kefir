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

struct i128 i128_add(struct i128, struct i128);
struct i128 u128_add(struct i128, struct i128);
struct i128 i128_sub(struct i128, struct i128);
struct i128 u128_sub(struct i128, struct i128);
struct i128 i128_and(struct i128, struct i128);
struct i128 u128_and(struct i128, struct i128);
struct i128 i128_or(struct i128, struct i128);
struct i128 u128_or(struct i128, struct i128);
struct i128 i128_xor(struct i128, struct i128);
struct i128 u128_xor(struct i128, struct i128);
struct i128 i128_mul(struct i128, struct i128);
struct i128 u128_mul(struct i128, struct i128);
struct i128 i128_div(struct i128, struct i128);
struct i128 u128_div(struct i128, struct i128);
struct i128 i128_mod(struct i128, struct i128);
struct i128 u128_mod(struct i128, struct i128);
struct i128 i128_shl(struct i128, struct i128);
struct i128 u128_shl(struct i128, struct i128);
struct i128 i128_shr(struct i128, struct i128);
struct i128 u128_shr(struct i128, struct i128);
int i128_eq(struct i128, struct i128);
int u128_eq(struct i128, struct i128);
int i128_neq(struct i128, struct i128);
int u128_neq(struct i128, struct i128);
int i128_greater(struct i128, struct i128);
int i128_less(struct i128, struct i128);
int i128_above(struct i128, struct i128);
int i128_below(struct i128, struct i128);
int i128_greater_eq(struct i128, struct i128);
int i128_less_eq(struct i128, struct i128);
int i128_above_eq(struct i128, struct i128);
int i128_below_eq(struct i128, struct i128);

#endif
