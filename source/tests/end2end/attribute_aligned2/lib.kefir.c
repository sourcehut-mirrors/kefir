/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#include "./definitions.h"

typedef char char_type_1 __attribute__((aligned(_Alignof(long))));
typedef char_type_1 char_type_2;
typedef long long_type_1 __attribute__((aligned(1)));
typedef long_type_1 long_type_2;

static char_type_1 chr1;
static char_type_2 chr2;
static long_type_1 lng1;
static long_type_2 lng2;
static _Alignas(long) int int1;

int Alignments[] = {_Alignof(char_type_1), _Alignof(char_type_2),        _Alignof(chr1),
                    _Alignof(chr2),        _Alignof(chr1 + chr2),        _Alignof(lng1),
                    _Alignof(lng2),        _Alignof(lng1 + lng2),        _Alignof(long_type_1),
                    _Alignof(long_type_2), _Alignof(int _Alignas(long)), _Alignof(int1)};

int get_alignment(int idx) {
    switch (idx) {
        case 0:
            return _Alignof(char_type_1);

        case 1:
            return _Alignof(char_type_2);

        case 2:
            return _Alignof(chr1);

        case 3:
            return _Alignof(chr2);

        case 4:
            return _Alignof(chr1 + chr2);

        case 5:
            return _Alignof(lng1);

        case 6:
            return _Alignof(lng2);

        case 7:
            return _Alignof(lng1 + lng2);

        case 8:
            return _Alignof(long_type_1);

        case 9:
            return _Alignof(long_type_2);

        case 10:
            return _Alignof(int _Alignas(long));

        case 11:
            return _Alignof(int1);
    }
}