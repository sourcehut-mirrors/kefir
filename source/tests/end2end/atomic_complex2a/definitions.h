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

_Complex float f32_load(_Atomic const _Complex float *);
_Complex double f64_load(_Atomic const _Complex double *);
_Complex long double ld_load(_Atomic const _Complex long double *);

void f32_store(_Atomic _Complex float *, _Complex float);
void f64_store(_Atomic _Complex double *, _Complex double);
void ld_store(_Atomic _Complex long double *, _Complex long double);

_Complex float f32_load_index(_Atomic const _Complex float[], unsigned int);
_Complex double f64_load_index(_Atomic const _Complex double[], unsigned int);
_Complex long double ld_load_index(_Atomic const _Complex long double[], unsigned int);

void f32_store_index(_Atomic _Complex float[], unsigned int, _Complex float);
void f64_store_index(_Atomic _Complex double[], unsigned int, _Complex double);
void ld_store_index(_Atomic _Complex long double[], unsigned int, _Complex long double);

struct Str1 {
    _Atomic _Complex float a;
    _Atomic _Complex double b;
    _Atomic _Complex long double c;
};

_Complex float str1_a(const struct Str1 *);
_Complex double str1_b(const struct Str1 *);
_Complex long double str1_c(const struct Str1 *);

void str1_set_a(struct Str1 *, _Complex float);
void str1_set_b(struct Str1 *, _Complex double);
void str1_set_c(struct Str1 *, _Complex long double);

#endif
