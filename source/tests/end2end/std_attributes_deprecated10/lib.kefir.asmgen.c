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

[[deprecated]] typedef int a_t;
[[deprecated("DEPRECATED")]] typedef int b_t;

a_t a;
b_t b;

void test() {
    [[deprecated]] typedef float c_t;
    [[deprecated("DEPRECATED")]] typedef float d_t;

    c_t c;
    d_t d;
}