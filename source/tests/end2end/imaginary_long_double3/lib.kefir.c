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

struct S2 {
    _Imaginary long double a;
    _Imaginary long double b;
};

_Imaginary long double test(int count, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, count);
    _Imaginary long double res = 0.0if;
    for (; count--;) {
        struct S2 arg = __builtin_va_arg(args, struct S2);
        res += arg.a - arg.b;
    }
    __builtin_va_end(args);
    return res;
}
