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

#include "./definitions.h"

_Complex float f32_load(_Atomic const _Complex float *ptr) {
    return *ptr;
}

_Complex double f64_load(_Atomic const _Complex double *ptr) {
    return *ptr;
}

_Complex long double ld_load(_Atomic const _Complex long double *ptr) {
    return *ptr;
}

void f32_store(_Atomic _Complex float *ptr, _Complex float value) {
    (*ptr) = value;
}

void f64_store(_Atomic _Complex double *ptr, _Complex double value) {
    (*ptr) = value;
}

void ld_store(_Atomic _Complex long double *ptr, _Complex long double value) {
    (*ptr) = value;
}

_Complex float f32_load_index(_Atomic const _Complex float arr[], unsigned int idx) {
    return arr[idx];
}

_Complex double f64_load_index(_Atomic const _Complex double arr[], unsigned int idx) {
    return arr[idx];
}

_Complex long double ld_load_index(_Atomic const _Complex long double arr[], unsigned int idx) {
    return arr[idx];
}

void f32_store_index(_Atomic _Complex float arr[], unsigned int idx, _Complex float value) {
    arr[idx] = value;
}

void f64_store_index(_Atomic _Complex double arr[], unsigned int idx, _Complex double value) {
    arr[idx] = value;
}

void ld_store_index(_Atomic _Complex long double arr[], unsigned int idx, _Complex long double value) {
    arr[idx] = value;
}

_Complex float str1_a(const struct Str1 *str) {
    return str->a;
}

_Complex double str1_b(const struct Str1 *str) {
    return str->b;
}

_Complex long double str1_c(const struct Str1 *str) {
    return str->c;
}

void str1_set_a(struct Str1 *str, _Complex float value) {
    str->a = value;
}

void str1_set_b(struct Str1 *str, _Complex double value) {
    str->b = value;
}

void str1_set_c(struct Str1 *str, _Complex long double value) {
    str->c = value;
}
