/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#define offsetof(_type, _field) __builtin_offsetof(_type, _field)

extern int offsets[] = {offsetof(struct Struct1, a),     offsetof(struct Struct1, b),   offsetof(struct Struct1, c),
                        offsetof(struct Struct1, d),     offsetof(struct Struct1, e),   offsetof(struct Struct1, f),
                        offsetof(struct Struct1, g),     offsetof(struct Struct1, x),   offsetof(union Union1, a),
                        offsetof(union Union1, b),       offsetof(union Union1, c),     offsetof(struct Struct1, c[2]),
                        offsetof(struct Struct1, e[10]), offsetof(struct Struct1, g.b), offsetof(struct Struct1, x[5])};

int getoffset(int idx) {
    switch (idx) {
        case 0:
            return offsetof(struct Struct1, a);
        case 1:
            return offsetof(struct Struct1, b);
        case 2:
            return offsetof(struct Struct1, c);
        case 3:
            return offsetof(struct Struct1, d);
        case 4:
            return offsetof(struct Struct1, e);
        case 5:
            return offsetof(struct Struct1, f);
        case 6:
            return offsetof(struct Struct1, g);
        case 7:
            return offsetof(struct Struct1, x);
        case 8:
            return offsetof(union Union1, a);
        case 9:
            return offsetof(union Union1, b);
        case 10:
            return offsetof(union Union1, c);
        case 11:
            return offsetof(struct Struct1, c[2]);
        case 12:
            return offsetof(struct Struct1, e[10]);
        case 13:
            return offsetof(struct Struct1, g.b);
        case 14:
            return offsetof(struct Struct1, x[5]);
    }
}
