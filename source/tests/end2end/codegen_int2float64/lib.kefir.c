/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

float test32(long a) {
    long b = a + 1;
    long c = a + 2;
    long d = a + 3;
    long e = a + 4;
    long f = a + 5;
    long g = a + 6;
    long h = a + 7;
    long i = a + 8;
    long j = a + 9;
    long k = a + 10;
    long l = a + 11;
    long m = a + 12;
    long n = a + 13;
    long o = a + 14;
    long p = a + 15;
    long q = a + 16;
    return (float) a + (float) b + (float) c + (float) d + (float) e + (float) f + (float) g + (float) h + (float) i +
           (float) j + (float) k + (float) l + (float) m + (float) n + (float) o + (float) p + (float) q;
}

double test64(long a) {
    long b = a + 1;
    long c = a + 2;
    long d = a + 3;
    long e = a + 4;
    long f = a + 5;
    long g = a + 6;
    long h = a + 7;
    long i = a + 8;
    long j = a + 9;
    long k = a + 10;
    long l = a + 11;
    long m = a + 12;
    long n = a + 13;
    long o = a + 14;
    long p = a + 15;
    long q = a + 16;
    return (double) a + (double) b + (double) c + (double) d + (double) e + (double) f + (double) g + (double) h +
           (double) i + (double) j + (double) k + (double) l + (double) m + (double) n + (double) o + (double) p +
           (double) q;
}
