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

unsigned _BitInt(128) a = ((unsigned _BitInt(128)) 0x123456464635424253647576uwb) >> 23;
unsigned _BitInt(128) b = (~(unsigned _BitInt(128)) 0uwb) >> 127;
unsigned _BitInt(128) c = (~(unsigned _BitInt(128)) 0uwb) >> 128;
unsigned _BitInt(128) d = ((unsigned _BitInt(128)) 0x1245352452353535253457uwb) >> 45uwb;

signed _BitInt(128) e = ((signed _BitInt(128)) - 13132435928) >> 12;
signed _BitInt(128) f = ((signed _BitInt(128)) - 1) >> 127;
signed _BitInt(128) g = ((signed _BitInt(128)) 077654646653553) >> 34;
signed _BitInt(128) h = ((signed _BitInt(128)) - 77654646653553) >> 36;
