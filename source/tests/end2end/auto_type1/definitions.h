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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

struct Str1 {
    unsigned int char_sz, char_align;
    unsigned int short_sz, short_align;
    unsigned int int_sz, int_align;
    unsigned int long_sz, long_align;
    unsigned int float_sz, float_align;
    unsigned int double_sz, double_align;
    unsigned int long_double_sz, long_double_align;
    unsigned int complex_float_sz, complex_float_align;
    unsigned int complex_double_sz, complex_double_align;
    unsigned int complex_long_double_sz, complex_long_double_align;
    unsigned int str1_sz, str1_align;
};

void test1(struct Str1 *);

#endif
