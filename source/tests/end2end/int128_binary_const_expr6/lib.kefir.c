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

unsigned __int128 a = ((unsigned __int128) 0x123456464635424253647576uwb) >> 23;
unsigned __int128 b = (~(unsigned __int128) 0u) >> 127;
unsigned __int128 c = (~(unsigned __int128) 0u) >> 128;
unsigned __int128 d = ((unsigned __int128) 0x1245352452353535253457uwb) >> 45uwb;

signed __int128 e = ((signed __int128) - 13132435928) >> 12;
signed __int128 f = ((signed __int128) - 1) >> 127;
signed __int128 g = ((signed __int128) 077654646653553) >> 34;
signed __int128 h = ((signed __int128) - 77654646653553) >> 36;
