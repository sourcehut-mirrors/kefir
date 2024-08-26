/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

void *a;
const int *const b;
volatile float *const *const c;

short d[100];
const _Complex double e[3];
float ***(*f)[6];

enum G { A = 1, B, C = -1000 } g;

struct H {
    int a;
    long b : 2, : 17, c : 12;
    float d;
    union {
        struct {
            _Bool e;
            _Complex long double f;
        };

        char g[20];
    };

    struct {
        struct H *a;
        struct H **b;
        struct {
            struct H ***c;
        };
    } h;

    float i[];
} h;

struct I *const i;

__builtin_va_list j;