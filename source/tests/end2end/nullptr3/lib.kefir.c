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

typeof(nullptr) N = nullptr;

long sum(...) {
    __builtin_va_list args;
    __builtin_c23_va_start(args);
    long res = 0;
    for (;;) {
        long *arg = __builtin_va_arg(args, long *);
        if (arg == N) {
            break;
        }
        res += *arg;
    }
    __builtin_va_end(args);
    return res;
}