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

unsigned _BitInt(1) a = 1;
unsigned _BitInt(2) b = 1;
unsigned _BitInt(3) c = 1;
unsigned _BitInt(6) d = 1;
unsigned _BitInt(10) e = 1;
unsigned _BitInt(20) f = 1;
unsigned _BitInt(33) g = 1;
unsigned _BitInt(48) h = 1;
unsigned _BitInt(65) i = 1;
unsigned _BitInt(256) j = 1;
unsigned _BitInt(1024) k = 1;

int sizes[] = {sizeof(a), sizeof(b), sizeof(c), sizeof(d), sizeof(e), sizeof(f),
               sizeof(g), sizeof(h), sizeof(i), sizeof(j), sizeof(k)};

int alignments[] = {_Alignof(a), _Alignof(b), _Alignof(c), _Alignof(d), _Alignof(e), _Alignof(f),
                    _Alignof(g), _Alignof(h), _Alignof(i), _Alignof(j), _Alignof(k)};
