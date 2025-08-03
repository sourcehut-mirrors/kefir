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

#include "./definitions.h"

int arr[] = {
    __kefir_builtin_int_precision(signed char),
    __kefir_builtin_int_precision(signed short),
    __kefir_builtin_int_precision(signed int),
    __kefir_builtin_int_precision(signed long),
    __kefir_builtin_int_precision(signed long long),

    __kefir_builtin_int_precision(unsigned char),
    __kefir_builtin_int_precision(unsigned short),
    __kefir_builtin_int_precision(unsigned int),
    __kefir_builtin_int_precision(unsigned long),
    __kefir_builtin_int_precision(unsigned long long),

    __kefir_builtin_int_precision(_BitInt(2)),
    __kefir_builtin_int_precision(_BitInt(20)),
    __kefir_builtin_int_precision(_BitInt(60)),
    __kefir_builtin_int_precision(_BitInt(7000)),

    __kefir_builtin_int_precision(unsigned _BitInt(1)),
    __kefir_builtin_int_precision(unsigned _BitInt(21)),
    __kefir_builtin_int_precision(unsigned _BitInt(61)),
    __kefir_builtin_int_precision(unsigned _BitInt(7001))
};
