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

const long results[] = {3 + (unsigned char) -2,
                        (short) 100000 + (char) -5,
                        (unsigned int) -100 + 1000,

                        5l + (unsigned long) -1,
                        10 - (unsigned short) -5,
                        ((unsigned int) -5) - 1,
                        (unsigned long) -6 - (signed long) -7,

                        -3 * (unsigned char) -6,
                        ((short) 0xffffff) * 5,
                        ((unsigned int) 100) * (unsigned long) -100,

                        ((unsigned char) 50) / (signed short) 7,
                        ((signed int) -6502) / (unsigned short) -60000,
                        ((signed long) 1540) / (unsigned int) 44,

                        ((unsigned char) -1000) % 30,
                        ((signed int) 3350) % (unsigned short) 6,
                        ((long) -604) % (long) -33,
                        ((unsigned long) -1) % (long) 350,

                        ((unsigned int) -50382) << 2,
                        ((signed long) 500) << 4,
                        ((unsigned short) -55) >> 3,
                        ((int) 746271) >> 4,

                        ((unsigned char) -50) > (char) 50,
                        ((unsigned char) -250) > (char) 40,
                        ((unsigned long) -5) > ~(unsigned int) 0,
                        (~(long) 0) > ~(unsigned int) 0,

                        ((unsigned char) -1) >= (unsigned char) 255,
                        ((unsigned short) -2) >= (unsigned short) 0xffff,
                        ((int) -5) >= (char) -6,
                        ((unsigned long) -3) >= (unsigned long) -2,

                        ((unsigned char) -30) < 255,
                        ((signed short) -30) < -50,
                        ((short) 60) < ((unsigned long) -60),
                        ((unsigned long) -1) < ((unsigned short) -5),

                        ((unsigned char) -30) <= (long) 300,
                        ((unsigned long) -30) <= (short) 400,
                        ((long) -5) <= ((char) -4),
                        ((unsigned long) -6) <= (unsigned char) -1,

                        ((unsigned char) -1) == (long) 255,
                        ((char) -1) == (long) 255,
                        ((int) 1000) == (unsigned long) 1000,
                        ((unsigned int) -1000) == (unsigned long) -1000,

                        ((char) -1) != (long) 255,
                        ((unsigned char) -1) != (long) 255,
                        ((unsigned int) -1000) != (unsigned long) -1000,
                        ((int) 1000) != (unsigned long) 1000,

                        ((unsigned short) -100) & 0xfefe,
                        ((unsigned long) -560) & -1392,
                        ((long) 5462) & (unsigned char) -3,

                        ((unsigned int) -1000) | (long) 500,
                        ((int) 4000) | (unsigned char) -120,
                        ((unsigned short) -5063) | (char) 5,

                        ((unsigned long) -66452) ^ (short) 500,
                        ((int) -646) ^ (unsigned long) 54982719,
                        ((unsigned char) 0xf5) ^ (long) -9000,

                        ((unsigned char) 0xff00) && (long) 1,
                        ((char) -1) && (long) 1,
                        ((unsigned short) (0xff << 20)) && (char) 0xff,
                        ((unsigned int) -1) && (unsigned long) -1,

                        ((unsigned char) 0xff0000) || (int) 0,
                        ((unsigned char) -1) || (short) 0xff0000,
                        ((unsigned long) 0) || (char) 0xff00,
                        ((unsigned long) 0) || (char) -1};
