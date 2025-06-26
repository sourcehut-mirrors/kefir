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

extern _BitInt(150) x;
extern unsigned _BitInt(150) y;

_BitInt(150) add(_BitInt(150) a) {
    return x += a;
}

_BitInt(150) sub(_BitInt(150) a) {
    return x -= a;
}

_BitInt(150) imul(_BitInt(150) a) {
    return x *= a;
}

_BitInt(150) idiv(_BitInt(150) a) {
    return x /= a;
}

_BitInt(150) imod(_BitInt(150) a) {
    return x %= a;
}

_BitInt(150) and (_BitInt(150) a) {
    return x &= a;
}

_BitInt(150) or (_BitInt(150) a) {
    return x |= a;
}

_BitInt(150) xor (_BitInt(150) a) { return x ^= a; }

    _BitInt(150) lshift(_BitInt(150) a) {
    return x <<= a;
}

_BitInt(150) arshift(_BitInt(150) a) {
    return x >>= a;
}

unsigned _BitInt(150) umul(unsigned _BitInt(150) a) {
    return y *= a;
}

unsigned _BitInt(150) udiv(unsigned _BitInt(150) a) {
    return y /= a;
}

unsigned _BitInt(150) umod(unsigned _BitInt(150) a) {
    return y %= a;
}

unsigned _BitInt(150) rshift(unsigned _BitInt(150) a) {
    return y >>= a;
}
