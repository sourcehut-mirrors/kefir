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

struct [[nodiscard]] S1 {
    int a;
};

struct [[nodiscard("nodiscard!")]] S2 {
    int a;
};

union [[nodiscard]] U1 {
    int a;
};

union [[nodiscard("nodiscard!")]] U2 {
    int a;
};

enum [[nodiscard]] E1 { E1A };

enum [[nodiscard("nodiscard!")]] E2 { E2A };

struct S1 get();
union U1 get2();
enum E1 get3();
[[nodiscard]] int get4();

struct S2 get5();
union U2 get6();
enum E2 get7();
[[nodiscard("nodiscard!")]] int get8();

void test() {
    get();
    get2();
    get3();
    get4();
    get5();
    get6();
    get7();
    get8();
}
