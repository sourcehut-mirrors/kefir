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

unsigned _BitInt(128) a = 0xFF00FF00EE01EE01DD02DD02CC03CC03uwb;
signed _BitInt(4) b = 0xffwb;
signed _BitInt(80) c = (_BitInt(8)) 0xffuwb;
unsigned _BitInt(80) d = (signed _BitInt(7)) 0xffuwb;
unsigned _BitInt(80) e = (unsigned _BitInt(7)) 0xffuwb;
signed _BitInt(80) f = (signed _BitInt(7)) 0xffuwb;
signed _BitInt(80) g = (unsigned _BitInt(7)) 0xffuwb;
unsigned _BitInt(15) h = (signed _BitInt(30)) 0xffffffffuwb;
unsigned _BitInt(15) i = (unsigned _BitInt(30)) 0xffffffffuwb;
signed _BitInt(15) j = (signed _BitInt(30)) 0xffffffffuwb;
signed _BitInt(15) k = (unsigned _BitInt(30)) 0xffffffffuwb;
unsigned _BitInt(80) l = (signed short) 0xfffffff;
unsigned _BitInt(80) m = (unsigned short) 0xfffffff;
signed _BitInt(80) n = (signed short) 0xfffffff;
signed _BitInt(80) o = (unsigned short) 0xfffffff;
unsigned _BitInt(9) p = (signed short) 0xfffffff;
unsigned _BitInt(9) q = (unsigned short) 0xfffffff;
signed _BitInt(9) r = (signed short) 0xfffffff;
signed _BitInt(9) s = (unsigned short) 0xfffffff;
unsigned _BitInt(17) t = 3.14159e2f;
signed _BitInt(17) u = 3.14159e2f;
unsigned _BitInt(60) v = (unsigned _BitInt(100))(void *) 1000;
