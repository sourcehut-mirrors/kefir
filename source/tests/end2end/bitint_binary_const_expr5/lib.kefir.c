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

unsigned _BitInt(128) a = ((unsigned _BitInt(128)) 0x123456uwb) << 86;
unsigned _BitInt(128) b = ((unsigned _BitInt(128)) 0x123457uwb) << 127;
unsigned _BitInt(128) c = ((unsigned _BitInt(128)) 0x123457uwb) << 128;
unsigned _BitInt(128) d = ((unsigned _BitInt(128)) 0x1245352452353535253457uwb) << 65uwb;

signed _BitInt(128) e = ((signed _BitInt(128)) - 13132435928) << 65;
signed _BitInt(128) f = ((signed _BitInt(128)) 42424994241991) << 127;
signed _BitInt(128) g = ((signed _BitInt(128)) 42424994241991) << 128;
signed _BitInt(128) h = ((signed _BitInt(128)) 077654646653553) << 68;
signed _BitInt(128) i = ((signed _BitInt(128)) - 77654646653553) << 68;
