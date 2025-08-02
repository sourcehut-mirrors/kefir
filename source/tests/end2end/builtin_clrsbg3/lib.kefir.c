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

int arr[] = {__builtin_clrsbg((unsigned char) 0),
             __builtin_clrsbg((unsigned char) 1),
             __builtin_clrsbg((unsigned char) 2),
             __builtin_clrsbg((unsigned char) 3),
             __builtin_clrsbg((unsigned char) (1ull << (sizeof(char) * 8 - 3))),
             __builtin_clrsbg((unsigned char) (1ull << (sizeof(char) * 8 - 2))),
             __builtin_clrsbg((unsigned char) (1ull << (sizeof(char) * 8 - 1))),
             __builtin_clrsbg((unsigned char) (1ull << (sizeof(char) * 8))),

             __builtin_clrsbg((unsigned short) 0),
             __builtin_clrsbg((unsigned short) 1),
             __builtin_clrsbg((unsigned short) 2),
             __builtin_clrsbg((unsigned short) 3),
             __builtin_clrsbg((unsigned short) (1ull << (sizeof(short) * 8 - 3))),
             __builtin_clrsbg((unsigned short) (1ull << (sizeof(short) * 8 - 2))),
             __builtin_clrsbg((unsigned short) (1ull << (sizeof(short) * 8 - 1))),
             __builtin_clrsbg((unsigned short) (1ull << (sizeof(short) * 8))),

             __builtin_clrsbg((unsigned int) 0),
             __builtin_clrsbg((unsigned int) 1),
             __builtin_clrsbg((unsigned int) 2),
             __builtin_clrsbg((unsigned int) 3),
             __builtin_clrsbg((unsigned int) (1ull << (sizeof(int) * 8 - 3))),
             __builtin_clrsbg((unsigned int) (1ull << (sizeof(int) * 8 - 2))),
             __builtin_clrsbg((unsigned int) (1ull << (sizeof(int) * 8 - 1))),
             __builtin_clrsbg((unsigned int) (1ull << (sizeof(int) * 8))),

             __builtin_clrsbg((unsigned long) 0),
             __builtin_clrsbg((unsigned long) 1),
             __builtin_clrsbg((unsigned long) 2),
             __builtin_clrsbg((unsigned long) 3),
             __builtin_clrsbg((unsigned long) (1ull << (sizeof(long) * 8 - 3))),
             __builtin_clrsbg((unsigned long) (1ull << (sizeof(long) * 8 - 2))),
             __builtin_clrsbg((unsigned long) (1ull << (sizeof(long) * 8 - 1)))};
