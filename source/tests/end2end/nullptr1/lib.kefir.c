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

void *a = nullptr;
_Bool b = nullptr;
int c = sizeof(nullptr);
int d = _Alignof(nullptr);

static_assert(nullptr == (void *) 0);
static_assert((void *) 0 == nullptr);
static_assert(nullptr != (void *) 1);
static_assert((void *) 1 != nullptr);
static_assert(nullptr == nullptr);
static_assert(nullptr == false);
static_assert(false == nullptr);
static_assert(nullptr != true);
static_assert(true != nullptr);
static_assert(nullptr == 0);
static_assert(0 == nullptr);
static_assert(nullptr != 1);
static_assert(1 != nullptr);
static_assert(!(1 ? nullptr : nullptr));
static_assert(!(0 ? nullptr : nullptr));
static_assert((int) (1 ? ((void *) 1) : nullptr));
static_assert(!(0 ? ((void *) 1) : nullptr));
static_assert(!(1 ? nullptr : ((void *) 1)));
static_assert((int) (0 ? nullptr : ((void *) 1)));

void *get() {
    return nullptr;
}

_Bool is(void *ptr) {
    return ptr == nullptr;
}

_Bool is2(void) {
    return nullptr == nullptr;
}

void *conv1(typeof(nullptr) x) {
    return (void *) x;
}

_Bool conv2(typeof(nullptr) x) {
    return (_Bool) x;
}

typeof(nullptr) conv3(typeof(nullptr) x) {
    return (typeof(nullptr)) x;
}

void test1(int *arr) {
    arr[0] = (nullptr == nullptr);
    arr[1] = (nullptr == 0);
    arr[2] = (0 == nullptr);
    arr[3] = (nullptr == 1);
    arr[4] = (1 == nullptr);
    arr[5] = (nullptr == false);
    arr[6] = (false == nullptr);
    arr[7] = (nullptr == true);
    arr[8] = (true == nullptr);
    arr[9] = (nullptr == (void *) 0);
    arr[10] = (((void *) 0) == nullptr);
    arr[11] = (nullptr == (void *) 1);
    arr[12] = (((void *) 1) == nullptr);
    arr[13] = (int) (void *) (1 ? nullptr : nullptr);
    arr[14] = (int) (void *) (0 ? nullptr : nullptr);
    arr[15] = (int) (void *) (1 ? ((void *) 1) : nullptr);
    arr[16] = (int) (void *) (0 ? ((void *) 1) : nullptr);
    arr[17] = (int) (void *) (1 ? nullptr : ((void *) 1));
    arr[18] = (int) (void *) (0 ? nullptr : ((void *) 1));
}

void set1(void **ptr) {
    *ptr = nullptr;
}

void set2(_Bool *ptr) {
    *ptr = nullptr;
}

void set3(typeof(nullptr) *ptr) {
    *ptr = nullptr;
}

_Bool and1(_Bool x) {
    return x && nullptr;
}

_Bool and2(_Bool x) {
    return nullptr && x;
}

_Bool and3() {
    return nullptr && nullptr;
}

_Bool or1(_Bool x) {
    return x || nullptr;
}

_Bool or2(_Bool x) {
    return nullptr || x;
}

_Bool or3() {
    return nullptr || nullptr;
}

typeof(nullptr) get1(typeof(nullptr) *x) {
    return *x;
}

typeof(nullptr) get2(...) {
    __builtin_va_list args;
    __builtin_c23_va_start(args);
    auto x = __builtin_va_arg(args, typeof(nullptr));
    __builtin_va_end(args);
    return x;
}