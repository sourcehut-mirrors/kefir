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

union A {
    struct {
        long a;
    };
    struct {
        double b;
    };
    _Complex double c;
    void *d;
};

long a = ((union A) {.a = 1234}).a;
double b = ((union A) {.b = 4.135}).b;
_Complex double c = ((union A) {.c = -564.0i}).c;
void *d = ((union A) {.d = &a}).d;
