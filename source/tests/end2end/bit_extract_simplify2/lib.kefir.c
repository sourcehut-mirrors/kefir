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

char test1(_BitInt(14) x) {
    return (_BitInt(4))(_BitInt(5))(_BitInt(8))(_BitInt(11)) x;
}

int test2(_BitInt(5) x) {
    return (_BitInt(30))(_BitInt(25))(_BitInt(18))(_BitInt(11))(_BitInt(8)) x;
}
