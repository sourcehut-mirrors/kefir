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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "./definitions.h"

long call1(struct X1 x) {
    return x.content[0];
}

long call2(struct X2 x) {
    return x.content[0] + x.content[1];
}

long call3(struct X3 x) {
    return x.content[0] + x.content[1] + x.content[2];
}

long call4(struct X4 x) {
    return x.content[0] + x.content[1] + x.content[2] + x.content[3];
}

long call5(struct X5 x) {
    return x.content[0] + x.content[1] + x.content[2] + x.content[3] + x.content[4];
}

long call6(struct X6 x) {
    return x.content[0] + x.content[1] + x.content[2] + x.content[3] + x.content[4] + x.content[5];
}

long call7(struct X7 x) {
    return x.content[0] + x.content[1] + x.content[2] + x.content[3] + x.content[4] + x.content[5] + x.content[6];
}

long call8(struct X8 x) {
    return x.content[0] + x.content[1] + x.content[2] + x.content[3] + x.content[4] + x.content[5] + x.content[6] +
           x.content[7];
}

int main(void) {
    for (char i = -100; i < 100; i++) {
        assert(call1_proxy(&(struct X1){{i}}) == i);
        assert(call2_proxy(&(struct X2){{i, i * 2}}) == i + (char) (i * 2));
        assert(call3_proxy(&(struct X3){{i, i * 2, i / 2}}) == i + (char) (i * 2) + (char) (i / 2));
        assert(call4_proxy(&(struct X4){{i, i * 2, i / 2, ~i}}) == i + (char) (i * 2) + (char) (i / 2) + (char) ~i);
        assert(call5_proxy(&(struct X5){{i, i * 2, i / 2, ~i, i ^ 0xca}}) ==
               i + (char) (i * 2) + (char) (i / 2) + (char) ~i + (char) (i ^ 0xca));
        assert(call6_proxy(&(struct X6){{i, i * 2, i / 2, ~i, i ^ 0xca, i - 1}}) ==
               i + (char) (i * 2) + (char) (i / 2) + (char) ~i + (char) (i ^ 0xca) + (char) (i - 1));
        assert(call7_proxy(&(struct X7){{i, i * 2, i / 2, ~i, i ^ 0xca, i - 1, 123}}) ==
               (i + (char) (i * 2) + (char) (i / 2) + (char) ~i + (char) (i ^ 0xca) + (char) (i - 1) + (char) 123));
        assert(call8_proxy(&(struct X8){{i, i * 2, i / 2, ~i, i ^ 0xca, i - 1, 123, i >> 1}}) ==
               (i + (char) (i * 2) + (char) (i / 2) + (char) ~i + (char) (i ^ 0xca) + (char) (i - 1) + (char) 123 +
                (char) (i >> 1)));
    }
    return EXIT_SUCCESS;
}
