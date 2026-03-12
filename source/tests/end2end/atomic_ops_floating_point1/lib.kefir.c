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

#include "./definitions.h"

void float_store(_Atomic float *ptr, float value) {
    return __atomic_store(ptr, &value, __ATOMIC_SEQ_CST);
}

void double_store(_Atomic double *ptr, double value) {
    return __atomic_store(ptr, &value, __ATOMIC_SEQ_CST);
}

float float_exchange(_Atomic float *ptr, float value) {
    float ret;
    __atomic_exchange(ptr, &value, &ret, __ATOMIC_SEQ_CST);
    return ret;
}

double double_exchange(_Atomic double *ptr, double value) {
    double ret;
    __atomic_exchange(ptr, &value, &ret, __ATOMIC_SEQ_CST);
    return ret;
}

_Bool float_compare_exchange(_Atomic float *ptr, float expected, float desired) {
    return __atomic_compare_exchange(ptr, &expected, &desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

_Bool double_compare_exchange(_Atomic double *ptr, double expected, double desired) {
    return __atomic_compare_exchange(ptr, &expected, &desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}
