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

#pragma pack(1)
struct S1 {
    char a;
    long b;
    float c;
    int d : 4, e : 5, : 15, f : 2;
    struct {
        char g;
        double h;
        short i;
    };
    union {
        int j;
        unsigned long long k;
        float l;
    };
    struct {
        char a;
        unsigned long b;
        double c;
    } m;
    char n[45];
    _Alignas(16) double o;
    char p;
    float q;
};
#pragma pack()

extern const unsigned long descriptor[];

#endif
