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

const long results[] = {-(unsigned char) -1,
                        -(signed char) -2,
                        -(unsigned short) -3,
                        -(signed short) -4,
                        -(unsigned int) -5,
                        -(signed int) -6,
                        -(unsigned long) -7,
                        -(signed long) -8,

                        !(unsigned char) 0xffff,
                        !(unsigned char) 0xff00,
                        !(signed char) 0xffff,
                        !(signed char) 0xff00,
                        !(unsigned short) 0xffffff,
                        !(unsigned short) 0xff0000,
                        !(signed short) 0xffffff,
                        !(signed short) 0xff0000,
                        !(unsigned int) 0xffffffffff,
                        !(unsigned int) 0xff00000000,
                        !(signed int) 0xffffffffff,
                        !(signed int) 0xff00000000,
                        !(unsigned long) 0xffff,
                        !(unsigned long) 0x00,
                        !(signed long) 0xffff,
                        !(signed long) 0x00,

                        -(unsigned char) 1,
                        -(signed char) 2,
                        -(unsigned short) 3,
                        -(signed short) 4,
                        -(unsigned int) 5,
                        -(signed int) 6,
                        -(unsigned long) 7,
                        -(signed long) 8,
                        -(unsigned long long) 9,
                        -(signed long long) 10,

                        ~(unsigned char) 1,
                        ~(signed char) 2,
                        ~(unsigned short) 3,
                        ~(signed short) 4,
                        ~(unsigned int) 5,
                        ~(signed int) 6,
                        ~(unsigned long) 7,
                        ~(signed long) 8,
                        ~(unsigned long long) 9,
                        ~(signed long long) 10,

                        ~(unsigned char) -1,
                        ~(signed char) -2,
                        ~(unsigned short) -3,
                        ~(signed short) -4,
                        ~(unsigned int) -5,
                        ~(signed int) -6,
                        ~(unsigned long) -7,
                        ~(signed long) -8,
                        ~(unsigned long long) -9,
                        ~(signed long long) -10};
