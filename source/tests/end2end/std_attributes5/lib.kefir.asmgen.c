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

[[var::attr1]] int [[var::attr2]] const [[var::attr3]] [[var::attr4]] static [[var::attr5]] var1;

struct[[struct ::attr1]][[struct ::attr2]] S1[[struct ::attr3]] {
    [[struct_member::attr1]] int [[struct_member::attr2]] const [[struct_member::attr3]] x;
    [[struct_member::attrx, struct_meber::attry()]] short y : 1;
    float [[struct_member::attr0]] z;
}[[struct ::attr4]];

struct[[struct ::attrx]] S2;

enum[[enum ::attr1]][[enum ::attr2]] E1 {
    E1C1 [[enum_const::attr1]],
    E1C2 [[enum_const::attr2, enum_const::attr3]] = 2,
    E1C3,
    E1C4 [[enum_const::attr4]] [[enum_const::attr5(test)]]
};

enum[[enum ::attrx]] E2;