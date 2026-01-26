/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#ifdef __x86_64__
unsigned short exchange1(unsigned short x) {
    asm("xchgb %h0, %b0" : "+Q" (x));
    return x;
}

unsigned short exchange2(unsigned short x) {
    unsigned short dummy[1];
    asm("xchgb %h1, %b1" : "=a"(dummy[0]), "+Q" (x));
    return x;
}

unsigned short exchange3(unsigned short x) {
    unsigned short dummy[2];
    asm("xchgb %h2, %b2" : "=a"(dummy[0]), "=c"(dummy[1]), "+Q" (x));
    return x;
}

unsigned short exchange4(unsigned short x) {
    unsigned short dummy[3];
    asm("xchgb %h3, %b3" : "=a"(dummy[0]), "=c"(dummy[1]), "=d"(dummy[2]), "+Q" (x));
    return x;
}
#endif
