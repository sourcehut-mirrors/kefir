/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#include "definitions.h"

int bools[] = {((unsigned __int128) 123455465) && ((unsigned __int128) 3242),
               ((unsigned __int128) 123455465) && ((signed __int128) 0uwb),
               ((unsigned __int128) 0) && ((signed __int128) -132432498wb),
               ((unsigned __int128) 134249) && ((signed __int128) -132432498wb),
               ((unsigned __int128) 0x10000000000000000000000000000uwb) && 1,
               ((signed __int128) -0x100000000000000000000000000wb) && 2,
               0 && ((signed __int128) -0x10000000000000000000000wb),

               ((unsigned __int128) 1232848274uwb) || ((unsigned __int128) 0),
               ((unsigned __int128) 1232848274uwb) || ((signed __int128) -1),
               ((unsigned __int128) 0b00000000uwb) || ((signed __int128) 0),
               ((unsigned __int128) 0x10000000000000000000000000uwb) || ((signed __int128) 0x000000000000000wb),
               ((unsigned __int128) 0x10000000000000000000000000uwb) ||
                   ((signed __int128) 0x10000000000000000000000000wb),
               ((signed __int128) -0x0000000000000000000000000000wb) ||
                   ((unsigned __int128) -0x0000000000000000000000000uwb)};
