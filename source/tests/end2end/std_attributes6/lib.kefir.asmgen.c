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

int var1 [[decl::attr0, decl::attr1]] [[decl::attr2]];
int var2[] [[decl2::attr0, decl2::attr1]] [[decl2::attr2]];
int *[[decl3::attr0, decl3::attr1]] [[decl3::attr2]] var3;

int fn1() [[decl4::attr0, decl4::attr1]] [[decl4::attr2]];
int fn2([[decl5::attr0, decl5::attr1]] [[decl5::attr2]] float [[decl5::attr3]]);
int fn3([[decl5::attr0, decl5::attr1]] [[decl5::attr2]] float x [[decl5::attr3]]);
int fn4([[decl6::attr0, decl6::attr1]] [[decl6::attr2]] float const
        [[decl6::attr3]] *[[decl6::attr4]] const *[[decl6::attr5]]);
int fn5([[decl7::attr0, decl7::attr1]] [[decl7::attr2]] float const
        [[decl7::attr3]] *[[decl7::attr4]] const *[[decl7::attr5]][static 6] [[decl7::attr6]][5] [[decl7::attr7]]);
