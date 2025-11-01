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

float i128_to_f32(struct i128);
double i128_to_f64(struct i128);
long double i128_to_f80(struct i128);

float u128_to_f32(struct i128);
double u128_to_f64(struct i128);
long double u128_to_f80(struct i128);

unsigned short i128_to_i9(struct i128);
unsigned short u128_to_i9(struct i128);
unsigned short i128_to_u9(struct i128);
unsigned short u128_to_u9(struct i128);

struct i256 i128_to_i256(struct i128);
struct i256 u128_to_i256(struct i128);
struct i256 i128_to_u256(struct i128);
struct i256 u128_to_u256(struct i128);

#endif
