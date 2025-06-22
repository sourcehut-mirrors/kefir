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

_BitInt(6) preinc1(_BitInt(6) * x) {
    return ++(*x);
}

_BitInt(14) preinc2(_BitInt(14) * x) {
    return ++(*x);
}

_BitInt(29) preinc3(_BitInt(29) * x) {
    return ++(*x);
}

_BitInt(58) preinc4(_BitInt(58) * x) {
    return ++(*x);
}

_BitInt(119) preinc5(_BitInt(119) * x) {
    return ++(*x);
}

_BitInt(310) preinc6(_BitInt(310) * x) {
    return ++(*x);
}