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

int arr[] = {__builtin_parityg((unsigned char) 0),   __builtin_parityg((unsigned char) 1),
             __builtin_parityg((unsigned char) 2),   __builtin_parityg((unsigned char) 3),
             __builtin_parityg((unsigned char) 4),   __builtin_parityg((unsigned char) 16),
             __builtin_parityg((unsigned char) 64),  __builtin_parityg((unsigned char) 67),
             __builtin_parityg((unsigned char) -4),  __builtin_parityg((unsigned char) -3),
             __builtin_parityg((unsigned char) -2),  __builtin_parityg((unsigned char) -1),

             __builtin_parityg((unsigned short) 0),  __builtin_parityg((unsigned short) 1),
             __builtin_parityg((unsigned short) 2),  __builtin_parityg((unsigned short) 3),
             __builtin_parityg((unsigned short) 4),  __builtin_parityg((unsigned short) 16),
             __builtin_parityg((unsigned short) 64), __builtin_parityg((unsigned short) 67),
             __builtin_parityg((unsigned short) -4), __builtin_parityg((unsigned short) -3),
             __builtin_parityg((unsigned short) -2), __builtin_parityg((unsigned short) -1),

             __builtin_parityg((unsigned int) 0),    __builtin_parityg((unsigned int) 1),
             __builtin_parityg((unsigned int) 2),    __builtin_parityg((unsigned int) 3),
             __builtin_parityg((unsigned int) 4),    __builtin_parityg((unsigned int) 16),
             __builtin_parityg((unsigned int) 64),   __builtin_parityg((unsigned int) 67),
             __builtin_parityg((unsigned int) -4),   __builtin_parityg((unsigned int) -3),
             __builtin_parityg((unsigned int) -2),   __builtin_parityg((unsigned int) -1),

             __builtin_parityg((unsigned long) 0),   __builtin_parityg((unsigned long) 1),
             __builtin_parityg((unsigned long) 2),   __builtin_parityg((unsigned long) 3),
             __builtin_parityg((unsigned long) 4),   __builtin_parityg((unsigned long) 16),
             __builtin_parityg((unsigned long) 64),  __builtin_parityg((unsigned long) 67),
             __builtin_parityg((unsigned long) -4),  __builtin_parityg((unsigned long) -3),
             __builtin_parityg((unsigned long) -2),  __builtin_parityg((unsigned long) -1)};
