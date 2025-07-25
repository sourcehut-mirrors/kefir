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

int prod(int a, int b) {
    static void *labels[] = {&&begin};
    [[xxx, yyy]] [[Y, Zz]] goto *labels[0];
    return 0;
begin:
    int prod = 0;
    int sign = 1;
    if (a < 0) {
        sign = -1;
        a = -a;
    } else if (a == 0) {
        [[ggg]] [[hhh, iii]] goto exit_label;
    }
    for (int i = 0;; i++) {
        if (!(i < a)) [[ddd, eee]] break;
        prod += b;
        [[fff]] continue;
    }

    exit_label:
    [[aaa, bbbb]] [[ccc()]] return prod *sign;
}
