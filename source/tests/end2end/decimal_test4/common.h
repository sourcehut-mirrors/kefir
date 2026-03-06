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
#define STR(x) #x

#define DEC_RAW(x, suf) x##suf
#define DEC(x) DEC_RAW(x, df)

#define STRUCT_GRID CONCAT(ALG_PREFIX, _Grid)
#define GRID_T CONCAT(ALG_PREFIX, _Grid_t)
#define GRID_INIT CONCAT(ALG_PREFIX, _init)
#define GRID_FREE CONCAT(ALG_PREFIX, _free)
#define LAPLACIAN CONCAT(ALG_PREFIX, _laplacian)
#define STEP CONCAT(ALG_PREFIX, _step)
#define NORM CONCAT(ALG_PREFIX, _norm)
#define RUN CONCAT(ALG_PREFIX, _run)

typedef struct STRUCT_GRID {
    NUMBER_T delta;
    NUMBER_T *field;
    NUMBER_T *vector;
    unsigned long dimension;
} GRID_T;

#define GRID_INDEX(_dim, _i, _j) ((_i) * ((unsigned long) (_dim)) + (_j))

static NUMBER_T NORM(NUMBER_T *arr, unsigned long dimension) {
    NUMBER_T sum = DEC(0.0);
    for (unsigned long i = 0; i < dimension * dimension; i++) {
        sum += arr[i] * arr[i];
    }
    return sum;
}

static void GRID_INIT(GRID_T *grid, unsigned long dimension) {
    grid->dimension = dimension;
    grid->delta = DEC(1.0) / (NUMBER_T) (dimension + 1);
    grid->field = malloc(dimension * dimension * sizeof(NUMBER_T));
    grid->vector = malloc(dimension * dimension * sizeof(NUMBER_T));
    if (grid->field == (void *) 0 || grid->vector == (void *) 0) {
        abort();
    }
    for (unsigned long i = 0; i < dimension; i++) {
        for (unsigned long j = 0; j < dimension; j++) {
            grid->field[GRID_INDEX(dimension, i, j)] = DEC(1.0);
            grid->vector[GRID_INDEX(dimension, i, j)] = DEC(0.0);
        }
    }
    unsigned long c = dimension / 2;
    for (unsigned long i = c - 1; i <= c + 1; i++) {
        for (unsigned long j = c - 1; j <= c + 1; j++) {
            grid->vector[GRID_INDEX(dimension, i, j)] = DEC(0.25);
            grid->field[GRID_INDEX(dimension, i, j)] = DEC(0.50);
        }
    }
}

static inline void GRID_FREE(GRID_T *grid) {
    free(grid->field);
    free(grid->vector);
}

static void LAPLACIAN(const NUMBER_T *field, NUMBER_T *out, unsigned long dimension, NUMBER_T delta) {
    for (unsigned long i = 0; i < dimension; i++) {
        for (unsigned long j = 0; j < dimension; j++) {
            NUMBER_T center = field[GRID_INDEX(dimension, i, j)];
#define ZERO DEC(0.0)
            NUMBER_T up = i > 0 ? field[GRID_INDEX(dimension, i - 1, j)] : ZERO;
            NUMBER_T down = i + 1 < dimension ? field[GRID_INDEX(dimension, i + 1, j)] : ZERO;
            NUMBER_T left = j > 0 ? field[GRID_INDEX(dimension, i, j - 1)] : ZERO;
            NUMBER_T right = j + 1 < dimension ? field[GRID_INDEX(dimension, i, j + 1)] : ZERO;
#undef ZERO
#define CENTER_WEIGHT DEC(4.0)
            out[GRID_INDEX(dimension, i, j)] = (up + down + left + right - CENTER_WEIGHT * center) / (delta * delta);
#undef CENTER_WEIGHT
        }
    }
}

static void STEP(GRID_T *grid, NUMBER_T F, NUMBER_T k, NUMBER_T delta) {
    const unsigned long area = grid->dimension * grid->dimension;
    NUMBER_T *const laplacian_field = malloc(area * sizeof(NUMBER_T));
    NUMBER_T *const laplacian_vector = malloc(area * sizeof(NUMBER_T));
    if (laplacian_field == (void *) 0 || laplacian_vector == (void *) 0) {
        abort();
    }

    LAPLACIAN(grid->field, laplacian_field, grid->dimension, grid->delta);
    LAPLACIAN(grid->vector, laplacian_vector, grid->dimension, grid->delta);

    for (unsigned long i = 0; i < grid->dimension; i++) {
        for (unsigned long j = 0; j < grid->dimension; j++) {
            unsigned long index = GRID_INDEX(grid->dimension, i, j);
            const NUMBER_T u = grid->field[index];
            const NUMBER_T v = grid->vector[index];
            const NUMBER_T uvv = u * v * v;
            grid->field[index] += delta * (laplacian_field[index] - uvv + F * (DEC(1.0) - u));
            grid->vector[index] += delta * (laplacian_vector[index] + uvv - (F + k) * v);
#define CLAMP(_ptr)           \
    if (*(_ptr) < DEC(0.0)) { \
        *(_ptr) = DEC(0.0);   \
    }
            CLAMP(&grid->field[index]);
            CLAMP(&grid->vector[index]);
#undef CLAMP
        }
    }
    free(laplacian_field);
    free(laplacian_vector);
}

NUMBER_T RUN(unsigned long dimension, unsigned int steps) {
    CONCAT(ALG_PREFIX, _Grid_t) grid;
    GRID_INIT(&grid, dimension);
    const NUMBER_T F = DEC(0.04);
    const NUMBER_T k = DEC(0.06);
    const NUMBER_T delta = DEC(1.0e-3);
    for (unsigned int s = 0; s < steps; s++) {
        STEP(&grid, F, k, delta);
    }
    const NUMBER_T norm_u = NORM(grid.field, dimension);
    const NUMBER_T norm_v = NORM(grid.vector, dimension);
    GRID_FREE(&grid);
    return norm_u + norm_v;
}

#undef STRUCT_GRID
#undef GRID_T
#undef GRID_INDEX
#undef GRID_INIT
#undef LAPLACIAN
#undef STEP
#undef NORM
#undef RUN
