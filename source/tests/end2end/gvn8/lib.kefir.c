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

_BitInt(120) test1(_Bool c, long x) {
    _BitInt(120) r;
    if (c) {
        r = x;
    } else {
        r = x;
    }
    return r;
}

unsigned _BitInt(120) test2(_Bool c, unsigned long x) {
    unsigned _BitInt(120) r;
    if (c) {
        r = x;
    } else {
        r = x;
    }
    return r;
}

_Bool test3(_Bool c, _BitInt(120) x) {
    _Bool r;
    if (c) {
        r = x;
    } else {
        r = x;
    }
    return r;
}

_BitInt(120) test4(_Bool c, _BitInt(120) x) {
    _BitInt(120) r;
    if (c) {
        r = -x;
    } else {
        r = -x;
    }
    return r;
}

_BitInt(120) test5(_Bool c, _BitInt(120) x) {
    _BitInt(120) r;
    if (c) {
        r = ~x;
    } else {
        r = ~x;
    }
    return r;
}

_Bool test6(_Bool c, _BitInt(120) x) {
    _Bool r;
    if (c) {
        r = !x;
    } else {
        r = !x;
    }
    return r;
}

int test7(_Bool c, _BitInt(120) x) {
    int r;
    if (c) {
        r = __builtin_ffsg(x);
    } else {
        r = __builtin_ffsg(x);
    }
    return r;
}

int test8(_Bool c, _BitInt(120) x) {
    int r;
    if (c) {
        r = __builtin_clzg(x);
    } else {
        r = __builtin_clzg(x);
    }
    return r;
}

int test9(_Bool c, _BitInt(120) x) {
    int r;
    if (c) {
        r = __builtin_ctzg(x);
    } else {
        r = __builtin_ctzg(x);
    }
    return r;
}

int test10(_Bool c, _BitInt(120) x) {
    int r;
    if (c) {
        r = __builtin_clrsbg(x);
    } else {
        r = __builtin_clrsbg(x);
    }
    return r;
}

int test11(_Bool c, _BitInt(120) x) {
    int r;
    if (c) {
        r = __builtin_popcountg(x);
    } else {
        r = __builtin_popcountg(x);
    }
    return r;
}

int test12(_Bool c, _BitInt(120) x) {
    int r;
    if (c) {
        r = __builtin_parityg(x);
    } else {
        r = __builtin_parityg(x);
    }
    return r;
}
