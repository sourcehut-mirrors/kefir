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

int get(int x) {
    switch (x) {
        case 0: return __kefir_builtin_int_precision(signed char);
        case 1: return __kefir_builtin_int_precision(signed short);
        case 2: return __kefir_builtin_int_precision(signed int);
        case 3: return __kefir_builtin_int_precision(signed long);
        case 4: return __kefir_builtin_int_precision(signed long long);

        case 5: return __kefir_builtin_int_precision(unsigned char);
        case 6: return __kefir_builtin_int_precision(unsigned short);
        case 7: return __kefir_builtin_int_precision(unsigned int);
        case 8: return __kefir_builtin_int_precision(unsigned long);
        case 9: return __kefir_builtin_int_precision(unsigned long long);

        case 10: return __kefir_builtin_int_precision(_BitInt(2));
        case 11: return __kefir_builtin_int_precision(_BitInt(20));
        case 12: return __kefir_builtin_int_precision(_BitInt(60));
        case 13: return __kefir_builtin_int_precision(_BitInt(7000));

        case 14: return __kefir_builtin_int_precision(unsigned _BitInt(1));
        case 15: return __kefir_builtin_int_precision(unsigned _BitInt(21));
        case 16: return __kefir_builtin_int_precision(unsigned _BitInt(61));
        case 17: return __kefir_builtin_int_precision(unsigned _BitInt(7001));
    }
    return 0;
}
