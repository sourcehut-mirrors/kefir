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

unsigned __int128 a = ((unsigned __int128) 0x12345670000000uwb) / ((unsigned __int128) 10000uwb);
unsigned __int128 b = ((unsigned __int128) 0x12445uwb) / ((unsigned __int128) 0xfffffffffuwb);
unsigned __int128 c = ((unsigned __int128) 0x1245453535353553545uwb) / ((unsigned __int128) 0x1uwb);

signed __int128 d = ((signed __int128) 0x124542483945uwb) / ((signed __int128) -1wb);
signed __int128 e = ((signed __int128) - 04353646357426464101wb) / ((signed __int128) 248247742847wb);
signed __int128 f = ((signed __int128) - 0x43535edfe44242424414edfabc2wb) / ((signed __int128) -1111001wb);
signed __int128 g = ((signed __int128) 0x43535edfe44242424414edfabc2wb) / ((signed __int128) 1111001wb);

signed __int128 h = ((signed __int128) - 0x43535edfe44242424414edfabc2wb) / ((unsigned __int128) 0x100000001uwb);
unsigned __int128 i = ((unsigned __int128) 424453535243363533uwb) / ((signed __int128) -535353522wb);
