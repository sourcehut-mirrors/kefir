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

#ifdef __x86_64__

_BitInt(6) test1(_BitInt(6) x) {
    _BitInt(6) r = x;
    asm("add $1, %0" : "+r"(r));
    return r;
}

_BitInt(14) test2(_BitInt(14) x) {
    _BitInt(14) r = x;
    asm("add $1, %0" : "+r"(r));
    return r;
}

_BitInt(29) test3(_BitInt(29) x) {
    _BitInt(29) r = x;
    asm("add $1, %0" : "+r"(r));
    return r;
}

_BitInt(60) test4(_BitInt(60) x) {
    _BitInt(60) r = x;
    asm("add $1, %0" : "+r"(r));
    return r;
}

_BitInt(120) test5(_BitInt(120) x) {
    _BitInt(120) r = x;
    asm("addq $1, %0\n"
        "adcq $0, 8%0\n"
        : "+m"(r));
    return r;
}

#endif
