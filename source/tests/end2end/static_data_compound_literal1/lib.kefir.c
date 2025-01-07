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

const int a = (int){ 100 };

int b[] = {
    (const int){ 200 },
    (int) {
        (int) {
            300
        }
    }
};

struct X c = {
    .a = (int) {
        -1
    },
    .b = (long) {
        (long) {
            (long) {
                -1234
            }
        }
    },
    .c = (char) 'c'
};

extern struct X d[] = {
    (struct X) {
        .a = (int){ 1000 }
    },
    (struct X) {
        .b = (const volatile long) { -3 }
    },
    (struct X){0}
};

extern struct Y e = {
    .a = {
        (struct X) {
            .c = (volatile char){ 'X' }
        },
        (struct X) {
            .a = 100
        }
    },
    .b = (int) {
        (int) {
            (int) {
                10004
            }
        }
    }
};
