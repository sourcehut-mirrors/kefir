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

int cmps[] = {((unsigned __int128) 0x0) == ((unsigned __int128) 0x0),
              ((unsigned __int128) 0x0) == ((unsigned __int128) 0x1),
              ((unsigned __int128) 0x100) == ((unsigned __int128) 0x100),
              ((unsigned __int128) 0x100) == ((unsigned __int128) 0x1000),

              ((signed __int128) -1) == ((signed __int128) -1),
              ((signed __int128) 1) == ((signed __int128) 1),
              ((signed __int128) 1) == ((signed __int128) -1),
              ((signed __int128) 1) == ((signed __int128) 1000),

              ((signed __int128) -1) == ((unsigned __int128) 0b11u),
              ((signed __int128) -1) == ((unsigned __int128) 0b111u),
              ((signed __int128) 0xfffffffffffffffffffffffffffffffffffffffffffffffffffwb) ==
                  ((unsigned __int128) 0xfffffffffffffffffffffffffffffffffffffffffffffffffffuwb),
              ((signed __int128) 0xfffffffffffffffffffffffffffffffffffffffffffffffffffwb) ==
                  ((unsigned __int128) 0xffffffffffffffffffffffffffffffffffffffffffffffffffeuwb),

              ((unsigned __int128) 1234u) == 1234,
              ((unsigned __int128) -1234) == -1234};
