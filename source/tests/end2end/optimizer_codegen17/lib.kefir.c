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

float mulf(float x, float y) {
    return x * y;
}

float my_hypotf(float x, float y) {
    return addf(mulf(x, x), mulf(y, y));
}

double muld(double x, double y) {
    return x * y;
}

double my_hypotd(double x, double y) {
    return addd(muld(x, x), muld(y, y));
}

extern float sum10f(float a, float b, float c, float d, float e, float f, float g, float h, float i, float j) {
    return a + b + c + d + e + f + g + h + i + j;
}

extern double sum10d(double a, double b, double c, double d, double e, double f, double g, double h, double i,
                     double j) {
    return a + b + c + d + e + f + g + h + i + j;
}

union Sumf sumf(union Sumf s) {
    return (union Sumf){.result = s.x + s.y};
}

union Sumd sumd(union Sumd s) {
    return (union Sumd){.result = s.x + s.y};
}

union Sumf my_hypotf2(union Sumf s) {
    return sumf((union Sumf){.x = mulf(s.x, s.x), .y = mulf(s.y, s.y)});
}

union Sumd my_hypotd2(union Sumd s) {
    return sumd((union Sumd){.x = muld(s.x, s.x), .y = muld(s.y, s.y)});
}

void farr_map(float *arr, int length, float (*map_fn)(float)) {
    for (int i = 0; i < length; i++) {
        arr[i] = map_fn(arr[i]);
    }
}

void darr_map(double *arr, int length, double (*map_fn)(double)) {
    for (int i = 0; i < length; i++) {
        arr[i] = map_fn(arr[i]);
    }
}
