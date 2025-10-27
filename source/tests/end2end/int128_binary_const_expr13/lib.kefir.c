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

unsigned __int128 a = ((unsigned __int128) 0xcafebabe0001313244535214uwb) & ((unsigned __int128) 0xffeeeccddccdddeeeccddduwb);
unsigned __int128 b = ((unsigned __int128) 0xffffffffffffffffffffffffffffffffffuwb) &
                          ((unsigned __int128) 0x9999999ffcef32492419482948318319uwb);
unsigned __int128 c = ((unsigned __int128) 0xcafebabe042482940uwb) & 0x0;

signed __int128 d = ((signed __int128) 0xeda0323183931wb) & ((signed __int128) -1);
signed __int128 e = ((signed __int128) - 0x4fecefefa93184942891efwb) & -((signed __int128) 382492428498241);
signed __int128 f = (1ll << 63) & ((signed __int128) 0x0333333333342132453575645wb);
