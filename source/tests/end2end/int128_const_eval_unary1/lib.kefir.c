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

unsigned __int128 a = ~(unsigned __int128) 0x00ff00ff00ff00ff00ff00ff00ff00uwb;
signed __int128 b = ~(__int128) 0x00ff00ff00ff00ff00ff00ff00ff00wb;
unsigned __int128 c = -(unsigned __int128) 0x00ff00ff00ff00ff00ff00ff00ff00uwb;
signed __int128 d = -(__int128) 0x00ff00ff00ff00ff00ff00ff00ff00wb;
unsigned __int128 e = +(unsigned __int128) 0x00ff00ff00ff00ff00ff00ff00ff00uwb;
signed __int128 f = +(__int128) 0x00ff00ff00ff00ff00ff00ff00ff00wb;

unsigned char g = !(unsigned __int128) 0xff00ff00ff00ff00ff00ff00uwb;
unsigned char h = !(unsigned __int128) 0;
unsigned char i = !(__int128) 0xff00ff00ff00ff00ff00ff00wb;
unsigned char j = !(__int128) 0;
