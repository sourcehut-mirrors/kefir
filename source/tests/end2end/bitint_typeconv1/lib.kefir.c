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

#define UCONST 0x1234567890987654321abcdefedbca0uwb
#define CONST 0x1234567890987654321abcdefedbca0wb

unsigned char get1(void) {
    return UCONST;
}

unsigned char get2(void) {
    return CONST;
}

unsigned short get3(void) {
    return UCONST;
}

unsigned short get4(void) {
    return CONST;
}

unsigned int get5(void) {
    return UCONST;
}

unsigned int get6(void) {
    return CONST;
}

unsigned long get7(void) {
    return UCONST;
}

unsigned long get8(void) {
    return CONST;
}
