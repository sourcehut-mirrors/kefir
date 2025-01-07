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

int somefn(int, int) __attribute__((alias("somefn_impl")));
static int somefn_impl_proxy(int, int) __attribute__((alias("somefn_real_impl")));

static int somefn_impl(int x, int y) {
    return somefn_impl_proxy(x, y);
}

int somefn_real_impl(int x, int y) {
    return x + y;
}

int somefn2(int, int) __attribute__((__alias__("somefn2_1")));
static int somefn2_1(int, int) __attribute__((__alias__("somefn2_2")));
int somefn2_2(int, int) __attribute__((__alias__("somefn2_3")));
static int somefn2_3(int, int) __attribute__((__alias__("somefn2_4")));
int somefn2_4(int, int) __attribute__((__alias__("somefn2_5")));
static int somefn2_5(int x, int y) {
    return x - y;
}
