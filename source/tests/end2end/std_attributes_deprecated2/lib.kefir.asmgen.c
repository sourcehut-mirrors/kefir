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

struct [[deprecated("DEPRECATED")]] S1 {
    int a;
};

union [[deprecated("DEPRECATED")]] U1 {
    int a;
};

enum [[deprecated("DEPRECATED")]] E1 {
    E1C
};

struct S1 get();
union U1 get2();
enum E1 get3();

void set(struct S1);
void set2(union U1);
void set3(enum E1);

struct S1 s1;
union U1 u1;
enum E1 e1;
