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

_BitInt(120) mod1(void) {
    return 0x123456789wb % 0xabcwb;
}

_BitInt(120) mod2(void) {
    return 0x123456789uwb % 0xabcwb;
}

_BitInt(120) mod3(void) {
    return -0x123456789wb % 0xabcwb;
}

_BitInt(120) mod4(void) {
    return -0x123456789uwb % 0xabcwb;
}
