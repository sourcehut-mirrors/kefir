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

unsigned _BitInt(128) a = 0x10000000100000001000000010000000uwb + 0x00100000001000000010000000100001uwb;
unsigned _BitInt(128) b = 0xffffffffffffffffffffffffffffffffuwb + 1;
signed _BitInt(128) c = 0x010000000100000001000000010000000wb + 0x10100000001000000010000000100001wb;
signed _BitInt(128) d = ((signed _BitInt(128)) 0) + ((signed _BitInt(128)) - 1);

unsigned _BitInt(128) e = 0x10000000100000001000000010000000uwb - 0x00100000001000000010000000100001uwb;
unsigned _BitInt(128) f = ((signed _BitInt(128)) 0) - 1;
signed _BitInt(128) g = 0x010000000100000001000000010000000wb - 0x10100000001000000010000000100001wb;
signed _BitInt(128) h = ((signed _BitInt(128)) 0) - ((signed _BitInt(128)) - 1);
