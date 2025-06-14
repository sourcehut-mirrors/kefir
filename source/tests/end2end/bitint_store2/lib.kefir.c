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

extern unsigned _BitInt(6) a;
extern unsigned _BitInt(13) b;
extern unsigned _BitInt(27) c;
extern unsigned _BitInt(50) d;
extern unsigned _BitInt(111) e;
extern unsigned _BitInt(366) f;

void set1(void) {
    a = 14uwb;
}

void set2(void) {
    b = 1024uwb;
}

void set3(void) {
    c = 0x1f3decuwb;
}

void set4(void) {
    d = 0xb0adc0ffeuwb;
}

void set5(void) {
    e = 0x6473646eddacdeb32ebduwb;
}

void set6(void) {
    f = 0x12345678987654323456789edcbdecd456765434567765456uwb;
}
