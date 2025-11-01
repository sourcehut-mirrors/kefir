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

struct i256 {
    unsigned long arr[4];
};

struct i128 i128_from_f32(float);
struct i128 i128_from_f64(double);
struct i128 i128_from_f80(long double);

struct i128 u128_from_f32(float);
struct i128 u128_from_f64(double);
struct i128 u128_from_f80(long double);

struct i128 i128_from_i9(unsigned short);
struct i128 i128_from_u9(unsigned short);
struct i128 u128_from_i9(unsigned short);
struct i128 u128_from_u9(unsigned short);

struct i128 i128_from_i256(struct i256);
struct i128 i128_from_u256(struct i256);
struct i128 u128_from_i256(struct i256);
struct i128 u128_from_u256(struct i256);

#endif
