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

unsigned _BitInt(128) a = ((unsigned _BitInt(128)) 0xcafebabe0001313244535214uwb) ^ 0xffeeeccddccdddeeeccddduwb;
unsigned _BitInt(128) b = ((unsigned _BitInt(128)) 0xffffffffffffffffffffffffffffffffffuwb) ^
                          0x9999999ffcef32492419482948318319uwb;
unsigned _BitInt(128) c = ((unsigned _BitInt(128)) 0xcafebabe042482940uwb) ^ 0x0;

signed _BitInt(128) d = ((signed _BitInt(128)) 0xeda0323183931wb) ^ -1wb;
signed _BitInt(128) e = ((signed _BitInt(128)) - 0x4fecefefa93184942891efwb) ^ -382492428498241wb;
signed _BitInt(128) f = (1ll << 63) ^ ((signed _BitInt(128)) 0x0333333333342132453575645wb);
