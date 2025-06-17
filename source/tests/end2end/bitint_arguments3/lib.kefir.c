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

extern _BitInt(120) a[3];
extern _BitInt(56) b[3];

void test1(_BitInt(120) p1, _BitInt(56) p2, _BitInt(120) p3, _BitInt(56) p4, _BitInt(120) p5, _BitInt(56) p6) {
    a[0] = p1;
    a[1] = p3;
    a[2] = p5;

    b[0] = p2;
    b[1] = p4;
    b[2] = p6;
}
