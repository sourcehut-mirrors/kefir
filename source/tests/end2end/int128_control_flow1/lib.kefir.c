/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

int test1(__int128 x, int y, int z) {
    return x ? y : z;
}

int test2(unsigned __int128 x, int y, int z) {
    return x ? y : z;
}

int test3(__int128 x, int y, int z) {
    if (x) {
        return y;
    } else {
        return z;
    }
}

int test4(unsigned __int128 x, int y, int z) {
    if (x) {
        return y;
    } else {
        return z;
    }
}

__int128 fact1(__int128 x) {
    __int128 res = 1;
    while (x) {
        res *= x--;
    }
    return res;
}

unsigned __int128 fact2(unsigned __int128 x) {
    unsigned __int128 res = 1;
    while (x) {
        res *= x--;
    }
    return res;
}

__int128 fact3(__int128 x) {
    __int128 res = 1;
    if (x) {
        do {
            res *= x--;
        } while (x);
    }
    return res;
}

unsigned __int128 fact4(unsigned __int128 x) {
    unsigned __int128 res = 1;
    if (x) {
        do {
            res *= x--;
        } while (x);
    }
    return res;
}

__int128 fact5(__int128 x) {
    __int128 res;
    for (res = 1; x; res *= x--) {}
    return res;
}

unsigned __int128 fact6(unsigned __int128 x) {
    __int128 res;
    for (res = 1; x; res *= x--) {}
    return res;
}

__int128 select1(__int128 x) {
    switch (x) {
        case 0:
            return -100;
        
        case -1:
            return -200;

        case 1:
            return 200;

        case 100 ... 1000:
            return 0;

        case -1000 ... -100:
            return 10;

        default:
            return -1;
    }
}

unsigned __int128 select2(unsigned __int128 x) {
    switch (x) {
        case 0:
            return -100;
        
        case -1:
            return -200;

        case 1:
            return 200;

        case 100 ... 1000:
            return 0;

        case -1000 ... -100:
            return 10;

        default:
            return -1;
    }
}
