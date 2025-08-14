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

#define DEF_OP(_name, _type, _op)            \
    _type _name(_Bool c, _type a, _type b) { \
        _type r;                             \
        if (c) {                             \
            r = a _op b;                     \
        } else {                             \
            r = a _op b;                     \
        }                                    \
        return r;                            \
    }
#define DEF_OPS(_name, _op)         \
    DEF_OP(_name, _BitInt(19), _op) \
    DEF_OP(u##_name, unsigned _BitInt(19), _op)

DEF_OPS(add, +)
DEF_OPS(sub, -)
DEF_OPS(mul, *)
DEF_OPS(and, &)
DEF_OPS(or, |)
DEF_OPS(xor, ^)
DEF_OPS(shl, <<)
DEF_OPS(shr, >>)

_BitInt(19) test(_Bool c, _BitInt(19) a, _BitInt(19) b) {
    _BitInt(19) r;
    if (c) {
        r = a + b;
    } else {
        r = ((_BitInt(7)) a) + ((_BitInt(7)) b);
    }
    return r;
}

int test2(_Bool c, _BitInt(19) a) {
    int r;
    if (c) {
        r = a;
    } else {
        r = (_BitInt(4)) a;
    }
    return r;
}
