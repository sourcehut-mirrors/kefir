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

_Bool add1(_BitInt(18) x, _BitInt(31) y, unsigned long *z) {
    return __builtin_add_overflow(x, y, z);
}

_Bool add2(_BitInt(50) x, _BitInt(61) y, unsigned long *z) {
    return __builtin_add_overflow(x, y, z);
}

_Bool add3(_BitInt(70) x, _BitInt(80) y, unsigned long *z) {
    return __builtin_add_overflow(x, y, z);
}
