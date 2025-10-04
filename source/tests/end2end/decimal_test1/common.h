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

// The code below has been produced with assistance of a large language model.
// The author disclaims any responsibility with respect to it, and it is
// provided for a sole purpose of creating a sufficiently diverse and
// sophisticated test case for differential testing against gcc.
//
// In case of any doubts regarding potential copyright infringement, please
// contact the author immediately via <jevgenij@protopopov.lv>.
//
// The code implements linear regression.

#ifdef CONCAT
#undef CONCAT
#endif

#ifdef CONCAT2
#undef CONCAT2
#endif

#define CONCAT2(a, b) a##b
#define CONCAT(a, b) CONCAT2(a, b)

typedef struct CONCAT(ALG_PREFIX, _Point) {
    DECIMAL_TYPE x;
    DECIMAL_TYPE y;
} CONCAT(ALG_PREFIX, _Point_t);

static DECIMAL_TYPE CONCAT(ALG_PREFIX, _mean_squared_error)(const CONCAT(ALG_PREFIX, _Point_t) * points,
                                                            unsigned long n, DECIMAL_TYPE m, DECIMAL_TYPE b) {
    DECIMAL_TYPE error = 0.0df;
    for (unsigned long i = 0; i < n; i++) {
        DECIMAL_TYPE diff = points[i].y - (m * points[i].x + b);
        error += diff * diff;
    }
    return error / (DECIMAL_TYPE) n;
}

static void CONCAT(ALG_PREFIX, _gradient_descent)(const CONCAT(ALG_PREFIX, _Point_t) * points, unsigned long n,
                                                  DECIMAL_TYPE *m, DECIMAL_TYPE *b, DECIMAL_TYPE lr, int iterations) {
    for (int it = 0; it < iterations; it++) {
        DECIMAL_TYPE dm = 0.0df;
        DECIMAL_TYPE db = 0.0df;
        for (unsigned long i = 0; i < n; i++) {
            DECIMAL_TYPE pred = (*m) * points[i].x + (*b);
            DECIMAL_TYPE diff = pred - points[i].y;
            dm += 2.0df * diff * points[i].x;
            db += 2.0df * diff;
        }
        dm /= (DECIMAL_TYPE) n;
        db /= (DECIMAL_TYPE) n;

        *m -= lr * dm;
        *b -= lr * db;
    }
}

DECIMAL_TYPE CONCAT(ALG_PREFIX, _linear_regression)(const CONCAT(ALG_PREFIX, _Point_t) * points, unsigned long n,
                                                    DECIMAL_TYPE *out_m, DECIMAL_TYPE *out_b) {
    DECIMAL_TYPE m = 0.0df;
    DECIMAL_TYPE b = 0.0df;
    DECIMAL_TYPE learning_rate = 0.1df;
    int iterations = 100;

    CONCAT(ALG_PREFIX, _gradient_descent)(points, n, &m, &b, learning_rate, iterations);

    *out_m = m;
    *out_b = b;

    return CONCAT(ALG_PREFIX, _mean_squared_error)(points, n, m, b);
}
