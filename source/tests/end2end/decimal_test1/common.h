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

#ifndef NUMBER_T
#error "NUMBER_T undefined"
#endif

#ifdef CONCAT
#undef CONCAT
#endif

#ifdef CONCAT2
#undef CONCAT2
#endif

#define CONCAT2(a, b) a##b
#define CONCAT(a, b) CONCAT2(a, b)

#define STRUCT_POINT CONCAT(ALG_PREFIX, _Point)
#define POINTS_T CONCAT(ALG_PREFIX, _Points_t)
#define STRUCT_POINTS CONCAT(ALG_PREFIX, _Points)
#define POINT_T CONCAT(ALG_PREFIX, _Point_t)
#define MEAN_SQUARED_ERROR CONCAT(ALG_PREFIX, _mean_squared_error)
#define GRADIENT_DESCENT CONCAT(ALG_PREFIX, _gradient_descent)
#define LINEAR_REGRESSION CONCAT(ALG_PREFIX, _linear_regression)

typedef struct STRUCT_POINT {
    NUMBER_T x;
    NUMBER_T y;
} POINT_T;

typedef struct STRUCT_POINTS {
    unsigned long length;
    POINT_T contents[];
} POINTS_T;

static void GRADIENT_DESCENT(const POINTS_T *points, NUMBER_T *slope_ptr, NUMBER_T *intercept_ptr,
                             NUMBER_T learning_rate, unsigned int iterations) {
    for (unsigned int i = 0; i < iterations; i++) {
        NUMBER_T delta_slope = 0.0df;
        NUMBER_T delta_intercept = 0.0df;
        for (unsigned long j = 0; j < points->length; j++) {
            NUMBER_T prev = (*slope_ptr) * points->contents[j].x + (*intercept_ptr);
            NUMBER_T diff = prev - points->contents[j].y;
            delta_slope += 2.0df * diff * points->contents[j].x;
            delta_intercept += 2.0df * diff;
        }

        *slope_ptr -= learning_rate * (delta_slope / points->length);
        *intercept_ptr -= learning_rate * (delta_intercept / points->length);
    }
}

static NUMBER_T MEAN_SQUARED_ERROR(const POINTS_T *points, NUMBER_T slope, NUMBER_T intercept) {
    NUMBER_T error = 0.0df;
    for (unsigned long i = 0; i < points->length; i++) {
        const NUMBER_T expected = slope * points->contents[i].x + intercept;
        const NUMBER_T diff = points->contents[i].y - expected;
        error += diff * diff;
    }
    return error / points->length;
}

NUMBER_T LINEAR_REGRESSION(const POINTS_T *points, NUMBER_T learning_rate, int iterations, NUMBER_T *out_m,
                           NUMBER_T *out_b) {
    *out_m = 0.0df;
    *out_b = 0.0df;

    GRADIENT_DESCENT(points, out_m, out_b, learning_rate, iterations);
    return MEAN_SQUARED_ERROR(points, *out_m, *out_b);
}

#undef STRUCT_POINT
#undef POINT_T
#undef MEAN_SQUARED_ERROR
#undef GRADIENT_DESCENT
#undef LINEAR_REGRESSION
