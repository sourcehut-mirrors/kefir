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

unsigned __int128 a = ((unsigned __int128) 0x10000000100000001000000010000000uwb) + ((unsigned __int128) 0x00100000001000000010000000100001uwb);
unsigned __int128 b = ((unsigned __int128) 0xffffffffffffffffffffffffffffffffuwb) + 1;
signed __int128 c = ((signed __int128) 0x010000000100000001000000010000000wb) + ((signed __int128) 0x10100000001000000010000000100001wb);
signed __int128 d = ((signed __int128) 0) + ((signed __int128) - 1);

unsigned __int128 e = ((unsigned __int128) 0x10000000100000001000000010000000uwb) - ((unsigned __int128) 0x00100000001000000010000000100001uwb);
unsigned __int128 f = ((signed __int128) 0) - 1;
signed __int128 g = ((signed __int128) 0x010000000100000001000000010000000wb) - ((signed __int128) 0x10100000001000000010000000100001wb);
signed __int128 h = ((signed __int128) 0) - ((signed __int128) - 1);
