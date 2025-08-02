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

int arr[] = {__builtin_clzg((unsigned char) 0, -1000),
             __builtin_clzg((unsigned char) (1ull << (8 * sizeof(char))), -1001),
             __builtin_clzg((unsigned char) 1),
             __builtin_clzg((unsigned char) 2),
             __builtin_clzg((unsigned char) 3),
             __builtin_clzg((unsigned char) (1 << (8 * sizeof(char) - 2))),
             __builtin_clzg((unsigned char) (1 << (8 * sizeof(char) - 1))),

             __builtin_clzg((unsigned short) 0, -2000),
             __builtin_clzg((unsigned short) (1ull << (8 * sizeof(short))), -2001),
             __builtin_clzg((unsigned short) 1),
             __builtin_clzg((unsigned short) 2),
             __builtin_clzg((unsigned short) 3),
             __builtin_clzg((unsigned short) (1 << (8 * sizeof(char)))),
             __builtin_clzg((unsigned short) (1 << (8 * sizeof(short) - 2))),
             __builtin_clzg((unsigned short) (1 << (8 * sizeof(short) - 1))),

             __builtin_clzg((unsigned int) 0, -3000),
             __builtin_clzg((unsigned int) (1ull << (8 * sizeof(int))), -3001),
             __builtin_clzg((unsigned int) 1),
             __builtin_clzg((unsigned int) 2),
             __builtin_clzg((unsigned int) 3),
             __builtin_clzg((unsigned int) (1 << (8 * sizeof(char)))),
             __builtin_clzg((unsigned int) (1 << (8 * sizeof(short)))),
             __builtin_clzg((unsigned int) (1 << (8 * sizeof(int) - 2))),
             __builtin_clzg((unsigned int) (1 << (8 * sizeof(int) - 1))),

             __builtin_clzg((unsigned long) 0, -4000),
             __builtin_clzg((unsigned long) 1),
             __builtin_clzg((unsigned long) 2),
             __builtin_clzg((unsigned long) 3),
             __builtin_clzg((unsigned long) (1ull << (8 * sizeof(char)))),
             __builtin_clzg((unsigned long) (1ull << (8 * sizeof(short)))),
             __builtin_clzg((unsigned long) (1ull << (8 * sizeof(int)))),
             __builtin_clzg((unsigned long) (1ull << (8 * sizeof(long) - 2))),
             __builtin_clzg((unsigned long) (1ull << (8 * sizeof(long) - 1)))};
