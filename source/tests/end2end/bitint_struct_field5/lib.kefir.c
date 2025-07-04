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

struct S1 {
    _BitInt(7) x : 5, y : 3;
};

struct S2 {
    _BitInt(14) x : 8, y : 1;
};

struct S3 {
    _BitInt(24) x : 15, y : 10;
};

struct S4 {
    _BitInt(50) x : 31, y : 4;
};

struct S5 {
    _BitInt(120) x : 100, y : 6;
};

struct S6 {
    _BitInt(500) x : 300, y : 10;
};

int sizes[] = {sizeof(struct S1), sizeof(struct S2), sizeof(struct S3),
               sizeof(struct S4), sizeof(struct S5), sizeof(struct S6)};

int alignments[] = {_Alignof(struct S1), _Alignof(struct S2), _Alignof(struct S3),
                    _Alignof(struct S4), _Alignof(struct S5), _Alignof(struct S6)};
