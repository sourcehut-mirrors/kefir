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

extern float addf(float, float);
extern float mulf(float, float);
extern float my_hypotf(float, float);

extern double addd(double, double);
extern double muld(double, double);
extern double my_hypotd(double, double);

extern float sum10f(float, float, float, float, float, float, float, float, float, float);
extern double sum10d(double, double, double, double, double, double, double, double, double, double);

union Sumf {
    struct {
        float x;
        float y;
    };
    float result;
};

union Sumf sumf(union Sumf);

union Sumd {
    struct {
        double x;
        double y;
    };
    double result;
};

union Sumf sumf(union Sumf);
union Sumd sumd(union Sumd);

union Sumf my_hypotf2(union Sumf);
union Sumd my_hypotd2(union Sumd);

void farr_map(float *, int, float (*)(float));
void darr_map(double *, int, double (*)(double));

#endif
