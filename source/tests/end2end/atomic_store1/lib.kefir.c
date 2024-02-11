/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

_Atomic _Bool b;
_Atomic unsigned char u8;
_Atomic unsigned short u16;
_Atomic unsigned int u32;
_Atomic unsigned long u64;
_Atomic float f32;
_Atomic double f64;

void set_b(_Bool x) {
    b = x;
}

void set_u8(unsigned char x) {
    u8 = x;
}

void set_u16(unsigned short x) {
    u16 = x;
}

void set_u32(unsigned int x) {
    u32 = x;
}

void set_u64(unsigned long x) {
    u64 = x;
}

void set_f32(float x) {
    f32 = x;
}

void set_f64(double x) {
    f64 = x;
}
