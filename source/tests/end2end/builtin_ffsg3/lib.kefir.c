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

int arr[] = {__builtin_ffsg((unsigned char) 0),
             __builtin_ffsg((unsigned char) 1),
             __builtin_ffsg((unsigned char) 2),
             __builtin_ffsg((unsigned char) 3),
             __builtin_ffsg((unsigned char) (1ull << 7)),
             __builtin_ffsg((unsigned char) ((1ull << 7) | (1ull << 6))),
             __builtin_ffsg((unsigned char) (1ull << 8)),

             __builtin_ffsg((unsigned short) 0),
             __builtin_ffsg((unsigned short) 1),
             __builtin_ffsg((unsigned short) 2),
             __builtin_ffsg((unsigned short) 3),
             __builtin_ffsg((unsigned short) (1ull << 7)),
             __builtin_ffsg((unsigned short) ((1ull << 7) | (1ull << 6))),
             __builtin_ffsg((unsigned short) (1ull << 8)),
             __builtin_ffsg((unsigned short) (1ull << 15)),
             __builtin_ffsg((unsigned short) ((1ull << 15) | (1ull << 10))),
             __builtin_ffsg((unsigned short) (1ull << 16)),

             __builtin_ffsg((unsigned int) 0),
             __builtin_ffsg((unsigned int) 1),
             __builtin_ffsg((unsigned int) 2),
             __builtin_ffsg((unsigned int) 3),
             __builtin_ffsg((unsigned int) (1ull << 7)),
             __builtin_ffsg((unsigned int) ((1ull << 7) | (1ull << 6))),
             __builtin_ffsg((unsigned int) (1ull << 8)),
             __builtin_ffsg((unsigned int) (1ull << 15)),
             __builtin_ffsg((unsigned int) (1ull << 16)),
             __builtin_ffsg((unsigned int) (1ull << 31)),
             __builtin_ffsg((unsigned int) ((1ull << 31) | (1ull << 29))),
             __builtin_ffsg((unsigned int) (1ull << 32)),

             __builtin_ffsg((unsigned long) 0),
             __builtin_ffsg((unsigned long) 1),
             __builtin_ffsg((unsigned long) 2),
             __builtin_ffsg((unsigned long) 3),
             __builtin_ffsg((unsigned long) (1ull << 7)),
             __builtin_ffsg((unsigned long) ((1ull << 7) | (1ull << 6))),
             __builtin_ffsg((unsigned long) (1ull << 8)),
             __builtin_ffsg((unsigned long) (1ull << 15)),
             __builtin_ffsg((unsigned long) (1ull << 16)),
             __builtin_ffsg((unsigned long) (1ull << 31)),
             __builtin_ffsg((unsigned long) ((1ull << 31) | (1ull << 29))),
             __builtin_ffsg((unsigned long) (1ull << 32)),
             __builtin_ffsg((unsigned long) (1ull << 63))};
