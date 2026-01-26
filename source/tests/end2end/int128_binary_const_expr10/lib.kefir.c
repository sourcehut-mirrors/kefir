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

int cmps[] = {((unsigned __int128) 0) >= ((unsigned __int128) 100),
              ((unsigned __int128) 0) >= ((unsigned __int128) 100000000000000000uwb),
              ((unsigned __int128) 100) >= ((unsigned __int128) 100),
              ((unsigned __int128) 101) >= ((unsigned __int128) 100),
              ((unsigned __int128) 10000000000000000000000000000uwb) >= ((unsigned __int128) 100),
              ((unsigned __int128) 1234) >= 67,
              ((unsigned __int128) 1234) >= 67000,
              1234 >= ((unsigned __int128) 67),
              1234 >= ((unsigned __int128) 67000),

              ((signed __int128) 0) >= ((signed __int128) 100),
              ((signed __int128) 0) >= ((signed __int128) -100),
              ((signed __int128) 100) >= ((signed __int128) 100),
              ((signed __int128) 100) >= ((signed __int128) -100),
              ((signed __int128) -100) >= ((signed __int128) -100),
              ((signed __int128) -100) >= ((signed __int128) 100),
              ((signed __int128) -100) >= -50,
              -100 >= ((signed __int128) -50),
              ((signed __int128) -100) >= -500000,
              -100 >= ((signed __int128) -500000),

              ((unsigned __int128) 0xfffffffu) >= ((signed __int128) -500000),
              ((signed __int128) -500000) >= ((unsigned __int128) 0xfffffffu),
              ((signed __int128) 0xfffffff) >= ((unsigned __int128) 500000u),
              ((unsigned __int128) 500000u) >= ((signed __int128) 0xfffffff)};
