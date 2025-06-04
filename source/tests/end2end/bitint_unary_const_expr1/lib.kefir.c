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

unsigned _BitInt(128) a = ~0xff00ff00ff00ff00ff00ff00ff00ff00uwb;
signed _BitInt(129) b = ~0xff00ff00ff00ff00ff00ff00ff00ff00wb;
unsigned _BitInt(128) c = -0x00ff00ff00ff00ff00ff00ff00ff00ffuwb;
signed _BitInt(129) d = -0x00ff00ff00ff00ff00ff00ff00ff00ffwb;
unsigned _BitInt(128) e = +0xff00ff00ff00ff00ff00ff00ff00ff00uwb;
signed _BitInt(129) f = +0xff00ff00ff00ff00ff00ff00ff00ff00wb;

unsigned char g = !0xf0000000000000000000000000000000000000000000000000000000000000uwb;
unsigned char h = !0x00000000000000000000000000000000000000000000000000000000000000uwb;
unsigned char i = !0xf0000000000000000000000000000000000000000000000000000000000000wb;
unsigned char j = !0x00000000000000000000000000000000000000000000000000000000000000wb;
