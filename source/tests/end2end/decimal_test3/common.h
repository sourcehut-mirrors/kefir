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

#define SQRT CONCAT(ALG_PREFIX, _sqrt_approx)
#define STRUCT_GRID CONCAT(ALG_PREFIX, _Grid)
#define GRID_T CONCAT(ALG_PREFIX, _Grid_t)
#define GRID_INIT CONCAT(ALG_PREFIX, _grid_init)
#define GRID_FREE CONCAT(ALG_PREFIX, _grid_free)
#define INIT_GAUSSIAN CONCAT(ALG_PREFIX, _initialize_gaussian)
#define LAPLACIAN_APPLY CONCAT(ALG_PREFIX, _laplacian_apply)
#define AX CONCAT(ALG_PREFIX, _ax)
#define DOT CONCAT(ALG_PREFIX, _dot)
#define NORM CONCAT(ALG_PREFIX, _norm)
#define CG_SOLVE CONCAT(ALG_PREFIX, _cg_solve)
#define TIMESTEP_IMPLICIT CONCAT(ALG_PREFIX, _timestep_implicit)
#define RUN_SIMULATION CONCAT(ALG_PREFIX, _run_simulation)

#define GRID_INDEX(_g, _i, _j) ((_i) * ((unsigned long) (_g)->dimension) + (_j))

typedef struct STRUCT_GRID {
    NUMBER_T grid_spacing;
    NUMBER_T *field;
    NUMBER_T *rhs_vector;
    unsigned long dimension;
} GRID_T;

static inline NUMBER_T SQRT(NUMBER_T x) {
    if (x <= (NUMBER_T) 0) {
        return (NUMBER_T) 0;
    }
#define SQRT_PRECISION 32
    NUMBER_T estimate = x * 0.5df + 0.5df;
    for (int i = 0; i < SQRT_PRECISION; i++) {
        estimate = 0.5df * (estimate + x / estimate);
    }
    return estimate;
#undef SQRT_PRECISION
}

static inline NUMBER_T DOT(const NUMBER_T *a, const NUMBER_T *b, unsigned long length) {
    NUMBER_T sum = 0;
    for (unsigned long i = 0; i < length; i++) {
        sum += a[i] * b[i];
    }
    return sum;
}

static inline NUMBER_T NORM(const NUMBER_T *arr, unsigned long len) {
    return SQRT(DOT(arr, arr, len));
}

static inline void GRID_INIT(GRID_T *grid, unsigned long dimesion) {
    const NUMBER_T grid_spacing = ((NUMBER_T) 1) / (NUMBER_T) (dimesion + 1);

    grid->dimension = dimesion;
    grid->grid_spacing = grid_spacing;
    grid->field = calloc(dimesion * dimesion, sizeof(NUMBER_T));
    grid->rhs_vector = calloc(dimesion * dimesion, sizeof(NUMBER_T));
    if (grid->field == (void *) 0 || grid->rhs_vector == (void *) 0) {
        abort();
    }
}

static inline void GRID_FREE(GRID_T *g) {
    free(g->field);
    free(g->rhs_vector);
    g->field = ((void *) 0);
    g->rhs_vector = ((void *) 0);
}

#define TOTAL_ENERGY(_g) (NORM((_g)->field, (_g)->dimension * (_g)->dimension))

static void INIT_GAUSSIAN(GRID_T *grid, NUMBER_T amplitude) {
#define CX ((NUMBER_T) 0.5)
#define CY ((NUMBER_T) 0.5)
#define SIGMA ((NUMBER_T) 0.08)
#define SIGMA_SQR (SIGMA * SIGMA)

    for (unsigned long i = 0; i < grid->dimension; ++i) {
        for (unsigned long j = 0; j < grid->dimension; ++j) {
            const NUMBER_T x = ((NUMBER_T) (i + 1)) * grid->grid_spacing;
            const NUMBER_T y = ((NUMBER_T) (j + 1)) * grid->grid_spacing;
            const NUMBER_T dx = x - CX;
            const NUMBER_T dy = y - CY;
            const NUMBER_T r2 = dx * dx + dy * dy;
            const NUMBER_T denominator = ((NUMBER_T) 1) + r2 / ((NUMBER_T) 2 * SIGMA_SQR);
            const NUMBER_T value = amplitude / denominator;
            grid->field[GRID_INDEX(grid, i, j)] = value;
            grid->rhs_vector[GRID_INDEX(grid, i, j)] = 0;
        }
    }

#undef CX
#undef CY
#undef SIGMA
#undef SIGMA_SQR
}

static void LAPLACIAN_APPLY(const GRID_T *grid, const NUMBER_T *p, NUMBER_T *out) {
    for (unsigned long i = 0; i < grid->dimension; i++) {
        for (unsigned long j = 0; j < grid->dimension; j++) {
#define ZERO ((NUMBER_T) 0)
            const NUMBER_T center = p[GRID_INDEX(grid, i, j)];
            const NUMBER_T up = i > 0 ? p[GRID_INDEX(grid, i - 1, j)] : ZERO;
            const NUMBER_T down = i + 1 < grid->dimension ? p[GRID_INDEX(grid, i + 1, j)] : ZERO;
            const NUMBER_T left = j > 0 ? p[GRID_INDEX(grid, i, j - 1)] : ZERO;
            const NUMBER_T right = j + 1 < grid->dimension ? p[GRID_INDEX(grid, i, j + 1)] : ZERO;
#undef ZERO
#define CENTER_WEIGHT ((NUMBER_T) 4)
            out[GRID_INDEX(grid, i, j)] =
                (up + down + left + right - CENTER_WEIGHT * center) / (grid->grid_spacing * grid->grid_spacing);
#undef CENTER_WEIGHT
        }
    }
}

static void AX(const GRID_T *grid, const NUMBER_T *x, NUMBER_T *y, NUMBER_T delta) {
    const unsigned long area = grid->dimension * grid->dimension;
    NUMBER_T *const temporary = malloc(area * sizeof(NUMBER_T));
    if (temporary == (void *) 0) {
        abort();
    }
    LAPLACIAN_APPLY(grid, x, temporary);
    for (unsigned long k = 0; k < area; k++) {
        y[k] = x[k] - delta * temporary[k];
    }
    free(temporary);
}

static void CG_SOLVE(const GRID_T *grid, NUMBER_T *solution, const NUMBER_T *rhs_vector, NUMBER_T delta,
                     unsigned int max_iter, NUMBER_T tolerance) {
    const unsigned long area = grid->dimension * grid->dimension;
    NUMBER_T *const residual = malloc(area * sizeof(NUMBER_T));
    NUMBER_T *const search_dir_vector = malloc(area * sizeof(NUMBER_T));
    NUMBER_T *const Ap = malloc(area * sizeof(NUMBER_T));
    if (residual == (void *) 0 || search_dir_vector == (void *) 0 || Ap == (void *) 0) {
        abort();
    }

    AX(grid, solution, Ap, delta);
    for (unsigned long i = 0; i < area; i++) {
        residual[i] = rhs_vector[i] - Ap[i];
        search_dir_vector[i] = residual[i];
    }

    NUMBER_T rsold = DOT(residual, residual, area);
    const NUMBER_T rs0 = rsold;
    if (rsold == (NUMBER_T) 0) {
        free(residual);
        free(search_dir_vector);
        free(Ap);
        return;
    }

    for (unsigned int iter = 0; iter < max_iter; iter++) {
        AX(grid, search_dir_vector, Ap, delta);
        const NUMBER_T alpha = rsold / DOT(search_dir_vector, Ap, area);

        for (unsigned long i = 0; i < area; i++) {
            solution[i] += alpha * search_dir_vector[i];
        }
        for (unsigned long i = 0; i < area; i++) {
            residual[i] -= alpha * Ap[i];
        }

        const NUMBER_T rsnew = DOT(residual, residual, area);
        const NUMBER_T rel = SQRT(rsnew / rs0);
        if (rel <= tolerance) {
            break;
        }
        const NUMBER_T beta = rsnew / rsold;
        for (unsigned long i = 0; i < area; i++) {
            search_dir_vector[i] = residual[i] + beta * search_dir_vector[i];
        }
        rsold = rsnew;
    }

    free(residual);
    free(search_dir_vector);
    free(Ap);
}

static void TIMESTEP_IMPLICIT(GRID_T *grid, NUMBER_T delta, int max_iterations, NUMBER_T tolerance) {
    unsigned long area = grid->dimension * grid->dimension;

    NUMBER_T *const rhs_vector_new = malloc(area * sizeof(NUMBER_T));
    NUMBER_T *const field_new = malloc(area * sizeof(NUMBER_T));
    if (rhs_vector_new == (void *) 0 || field_new == (void *) 0) {
        abort();
    }

    for (unsigned long i = 0; i < area; i++) {
        rhs_vector_new[i] = grid->field[i];
        field_new[i] = grid->field[i];
    }

    CG_SOLVE(grid, field_new, rhs_vector_new, delta, max_iterations, tolerance);
    free(rhs_vector_new);

    for (unsigned long i = 0; i < area; i++) {
        grid->field[i] = field_new[i];
    }

    free(field_new);
}

NUMBER_T RUN_SIMULATION(unsigned long dimension, unsigned int steps, NUMBER_T delta, int max_iterations,
                        NUMBER_T tolerance) {
    GRID_T grid;
    GRID_INIT(&grid, dimension);
    INIT_GAUSSIAN(&grid, DEC(1.0));

    for (unsigned int i = 0; i < steps; i++) {
        NUMBER_T adaptive_tolerance;
        if (i < steps / 3) {
            adaptive_tolerance = tolerance * (NUMBER_T) 10;
        } else if (i < 2 * steps / 3) {
            adaptive_tolerance = tolerance * (NUMBER_T) 2;
        } else {
            adaptive_tolerance = tolerance;
        }
        TIMESTEP_IMPLICIT(&grid, delta, max_iterations, adaptive_tolerance);
    }

    const NUMBER_T energy = TOTAL_ENERGY(&grid);
    GRID_FREE(&grid);
    return energy;
}

#undef SQRT
#undef STRUCT_GRID
#undef GRID_T
#undef GRID_INIT
#undef GRID_FREE
#undef GRID_INDEX
#undef INIT_GAUSSIAN
#undef LAPLACIAN_APPLY
#undef AX
#undef DOT
#undef NORM
#undef CG_SOLVE
#undef TIMESTEP_IMPLICIT
#undef TOTAL_ENERGY
#undef RUN_SIMULATION
