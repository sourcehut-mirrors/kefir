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

extern unsigned _BitInt(5) a[3];
extern unsigned _BitInt(12) b[3];
extern unsigned _BitInt(27) c[3];
extern unsigned _BitInt(61) d[3];

void test1(unsigned _BitInt(5) p1, unsigned _BitInt(12) p2, unsigned _BitInt(27) p3, unsigned _BitInt(61) p4,
           unsigned _BitInt(5) p5, unsigned _BitInt(12) p6, unsigned _BitInt(27) p7, unsigned _BitInt(61) p8,
           unsigned _BitInt(5) p9, unsigned _BitInt(12) p10, unsigned _BitInt(27) p11, unsigned _BitInt(61) p12) {
    a[0] = p1;
    a[1] = p5;
    a[2] = p9;

    b[0] = p2;
    b[1] = p6;
    b[2] = p10;

    c[0] = p3;
    c[1] = p7;
    c[2] = p11;

    d[0] = p4;
    d[1] = p8;
    d[2] = p12;
}
