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

long int_equals_const(long x) {
    if (x == 100)
        return 1;
    else
        return 0;
}

long int_not_equal_const(long x) {
    if (x != -160)
        return 1;
    else
        return 0;
}

long int_greater_const(long x) {
    if (x > 876)
        return 1;
    else
        return 0;
}

long int_greater_or_equal_const(long x) {
    if (x >= 150)
        return 1;
    else
        return 0;
}

long int_less_const(long x) {
    if (x < -5)
        return 1;
    else
        return 0;
}

long int_less_or_equal_const(long x) {
    if (x <= 998)
        return 1;
    else
        return 0;
}

long int_above_const(unsigned long x) {
    if (x > 642u)
        return 1;
    else
        return 0;
}

long int_above_or_equal_const(unsigned long x) {
    if (x >= 10u)
        return 1;
    else
        return 0;
}

long int_below_const(unsigned long x) {
    if (x < 76u)
        return 1;
    else
        return 0;
}

long int_below_or_equal_const(unsigned long x) {
    if (x <= 793u)
        return 1;
    else
        return 0;
}
