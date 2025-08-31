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

_Atomic unsigned char src8;
_Atomic unsigned char dst8;
_Atomic unsigned short src16;
_Atomic unsigned short dst16;
_Atomic unsigned int src32;
_Atomic unsigned int dst32;
_Atomic unsigned long src64;
_Atomic unsigned long dst64;

void copy8(void) {
    dst8 = (unsigned char) (unsigned long) src8;
}

void copy16(void) {
    dst16 = (unsigned short) (unsigned long) src16;
}

void copy32(void) {
    dst32 = (unsigned int) (unsigned long) src32;
}

void copy64(void) {
    dst64 = src64;
}
