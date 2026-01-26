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

#line 0 "lib.kefir.asmgen.c"

int a = ;
int b = 100;
int c[] = {
    0,
    +,
    1,
    (((-))),
    2
};

int fn1(void) {
    return 1;
}

int fn2(void) {
    int a = ;
    a = 1;
    ({
        +;
        2;
        -;
    });
    fn(1, -, 3, +);
    --;
    return 0;
}

int d[] = {0};
