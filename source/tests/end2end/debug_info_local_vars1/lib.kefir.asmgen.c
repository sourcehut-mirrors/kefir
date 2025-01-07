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

struct Struct1 {
    long x[100];
};

double fn1(int a, long b, float c, void *d, struct Struct1 s2) {
    double x = a + b;
    do {
        if (b) {
            struct Struct1 s1;
            long double y = c + ((double *) d)[0];
            double z = x - y;
            return z * s1.x[a] + (d != 0 ? s2.x[b] : s2.x[7]);
        }
    } while (0);
}

long fn2(long a, long b, long c, long d, long e, long f, long g, long h, long i, long j, long k, long m, long n, long o,
         long p, long q, long r, long s) {
    return a + b + c + d + e + f + g + h + i + j + k + m + n + o + p + q + r + s;
}

double fn3(double a, double b, double c, double d, double e, double f, double g, double h, double i, double j, double k,
           double m, double n, double o, double p, double q, double r, double s) {
    return a + b + c + d + e + f + g + h + i + j + k + m + n + o + p + q + r + s;
}