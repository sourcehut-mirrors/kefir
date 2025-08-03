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

int arr[] = {__builtin_parityg((_BitInt(5)) 0),
             __builtin_parityg((_BitInt(500)) 0),
             __builtin_parityg((_BitInt(5)) 1),
             __builtin_parityg((_BitInt(500)) 1),
             __builtin_parityg((_BitInt(5)) 3),
             __builtin_parityg((_BitInt(500)) 3),
             __builtin_parityg((_BitInt(5)) - 1),
             __builtin_parityg((_BitInt(500)) - 1),
             __builtin_parityg((_BitInt(5)) - 2),
             __builtin_parityg((_BitInt(500)) - 2),
             __builtin_parityg((_BitInt(5)) - 4),
             __builtin_parityg((_BitInt(500)) - 4),
             __builtin_parityg(((_BitInt(5)) 1) << 3),
             __builtin_parityg(((_BitInt(500)) 1) << 300),
             __builtin_parityg(((_BitInt(5)) - 1) << 3),
             __builtin_parityg(((_BitInt(500)) - 1) << 300),
             __builtin_parityg(((unsigned _BitInt(5)) - 1) >> 2),
             __builtin_parityg(((unsigned _BitInt(500)) - 1) >> 200),

             __builtin_parityg((unsigned _BitInt(5)) 0),
             __builtin_parityg((unsigned _BitInt(500)) 0),
             __builtin_parityg((unsigned _BitInt(5)) 1),
             __builtin_parityg((unsigned _BitInt(500)) 1),
             __builtin_parityg((unsigned _BitInt(5)) 3),
             __builtin_parityg((unsigned _BitInt(500)) 3),
             __builtin_parityg((unsigned _BitInt(5)) - 1),
             __builtin_parityg((unsigned _BitInt(500)) - 1),
             __builtin_parityg((unsigned _BitInt(5)) - 2),
             __builtin_parityg((unsigned _BitInt(500)) - 2),
             __builtin_parityg((unsigned _BitInt(5)) - 4),
             __builtin_parityg((unsigned _BitInt(500)) - 4),
             __builtin_parityg(((unsigned _BitInt(5)) 1) << 3),
             __builtin_parityg(((unsigned _BitInt(500)) 1) << 300),
             __builtin_parityg(((unsigned _BitInt(5)) - 1) << 3),
             __builtin_parityg(((unsigned _BitInt(500)) - 1) << 300),
             __builtin_parityg(((unsigned _BitInt(5)) - 1) >> 2),
             __builtin_parityg(((unsigned _BitInt(500)) - 1) >> 200)};
