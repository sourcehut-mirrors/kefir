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

auto a = 0xcafebabe0l;
auto b = 3.14159f;
auto c = -2.71i;
auto d = &(static short[]) {1, 2, 3, 4, -5};
auto e = (struct S1 {
    int a;
    double b;
    long c : 43;
}) {100, 200.2, 300};

auto f = &(struct S1) {-300, -200.2, -100};
