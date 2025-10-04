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
// The code implements simulation of implicit 2D heat equation with Conjugate
// Gradient solver.

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

static DECIMAL_TYPE CONCAT(ALG_PREFIX, _sqrt_approx)(DECIMAL_TYPE x) {
    if (x <= (DECIMAL_TYPE) 0) {
        return (DECIMAL_TYPE) 0;
    }
    DECIMAL_TYPE g = x * (DECIMAL_TYPE) 0.5 + (DECIMAL_TYPE) 0.5;
    for (int i = 0; i < 10; ++i) {
        g = (g + x / g) * (DECIMAL_TYPE) 0.5;
    }
    return g;
}

typedef struct CONCAT(ALG_PREFIX, _Grid_t) {
    unsigned long N;
    DECIMAL_TYPE hx;
    DECIMAL_TYPE inv_h2;
    DECIMAL_TYPE *u;
    DECIMAL_TYPE *b;
} CONCAT(ALG_PREFIX, _Grid_t);

static void CONCAT(ALG_PREFIX, _grid_init)(CONCAT(ALG_PREFIX, _Grid_t) * g, unsigned long N) {
    g->N = N;
    DECIMAL_TYPE h = ((DECIMAL_TYPE) 1) / (DECIMAL_TYPE) (N + 1);
    g->hx = h;
    g->inv_h2 = ((DECIMAL_TYPE) 1) / (h * h);
    g->u = calloc(N * N, sizeof(DECIMAL_TYPE));
    g->b = calloc(N * N, sizeof(DECIMAL_TYPE));
    if (!g->u || !g->b) {
        abort();
    }
}

static void CONCAT(ALG_PREFIX, _grid_free)(CONCAT(ALG_PREFIX, _Grid_t) * g) {
    free(g->u);
    free(g->b);
    g->u = (g->b = 0);
}

static inline unsigned long CONCAT(ALG_PREFIX, _idx)(const CONCAT(ALG_PREFIX, _Grid_t) * g, unsigned long i,
                                                     unsigned long j) {
    return i * g->N + j;
}

static void CONCAT(ALG_PREFIX, _initialize_gaussian)(CONCAT(ALG_PREFIX, _Grid_t) * g, DECIMAL_TYPE amplitude) {
    unsigned long N = g->N;
    DECIMAL_TYPE h = g->hx;
    DECIMAL_TYPE cx = (DECIMAL_TYPE) 0.5;
    DECIMAL_TYPE cy = (DECIMAL_TYPE) 0.5;
    DECIMAL_TYPE sigma = (DECIMAL_TYPE) 0.08;

    for (unsigned long i = 0; i < N; ++i) {
        for (unsigned long j = 0; j < N; ++j) {
            DECIMAL_TYPE x = ((DECIMAL_TYPE) (i + 1)) * h;
            DECIMAL_TYPE y = ((DECIMAL_TYPE) (j + 1)) * h;
            DECIMAL_TYPE dx = x - cx;
            DECIMAL_TYPE dy = y - cy;
            DECIMAL_TYPE r2 = dx * dx + dy * dy;
            DECIMAL_TYPE denom = ((DECIMAL_TYPE) 1) + r2 / ((DECIMAL_TYPE) 2 * sigma * sigma);
            DECIMAL_TYPE value = amplitude / denom;
            g->u[CONCAT(ALG_PREFIX, _idx)(g, i, j)] = value;
            g->b[CONCAT(ALG_PREFIX, _idx)(g, i, j)] = (DECIMAL_TYPE) 0;
        }
    }
}

static void CONCAT(ALG_PREFIX, _laplacian_apply)(const CONCAT(ALG_PREFIX, _Grid_t) * g, const DECIMAL_TYPE *p,
                                                 DECIMAL_TYPE *out) {
    unsigned long N = g->N;
    DECIMAL_TYPE inv_h2 = g->inv_h2;

    for (unsigned long i = 0; i < N; i++) {
        for (unsigned long j = 0; j < N; j++) {
            DECIMAL_TYPE center = p[CONCAT(ALG_PREFIX, _idx)(g, i, j)];
            DECIMAL_TYPE up = (i > 0) ? p[CONCAT(ALG_PREFIX, _idx)(g, i - 1, j)] : (DECIMAL_TYPE) 0;
            DECIMAL_TYPE down = (i + 1 < N) ? p[CONCAT(ALG_PREFIX, _idx)(g, i + 1, j)] : (DECIMAL_TYPE) 0;
            DECIMAL_TYPE left = (j > 0) ? p[CONCAT(ALG_PREFIX, _idx)(g, i, j - 1)] : (DECIMAL_TYPE) 0;
            DECIMAL_TYPE right = (j + 1 < N) ? p[CONCAT(ALG_PREFIX, _idx)(g, i, j + 1)] : (DECIMAL_TYPE) 0;
            out[CONCAT(ALG_PREFIX, _idx)(g, i, j)] = (up + down + left + right - (DECIMAL_TYPE) 4 * center) * inv_h2;
        }
    }
}

static void CONCAT(ALG_PREFIX, _ax)(const CONCAT(ALG_PREFIX, _Grid_t) * g, const DECIMAL_TYPE *x, DECIMAL_TYPE *y,
                                    DECIMAL_TYPE dt) {
    unsigned long N = g->N;
    DECIMAL_TYPE *tmp = malloc(N * N * sizeof(DECIMAL_TYPE));
    if (!tmp) {
        abort();
    }
    CONCAT(ALG_PREFIX, _laplacian_apply)(g, x, tmp);
    for (unsigned long k = 0; k < N * N; ++k) {
        y[k] = x[k] - dt * tmp[k];
    }
    free(tmp);
}

static DECIMAL_TYPE CONCAT(ALG_PREFIX, _dot)(const DECIMAL_TYPE *a, const DECIMAL_TYPE *b, unsigned long n) {
    DECIMAL_TYPE s = 0;
    for (unsigned long i = 0; i < n; i++) {
        s += a[i] * b[i];
    }
    return s;
}

static DECIMAL_TYPE CONCAT(ALG_PREFIX, _norm)(const DECIMAL_TYPE *a, unsigned long n) {
    DECIMAL_TYPE s = CONCAT(ALG_PREFIX, _dot)(a, a, n);
    return CONCAT(ALG_PREFIX, _sqrt_approx)(s);
}

static int CONCAT(ALG_PREFIX, _cg_solve)(const CONCAT(ALG_PREFIX, _Grid_t) * g, DECIMAL_TYPE *x, const DECIMAL_TYPE *b,
                                         DECIMAL_TYPE dt, int max_iter, DECIMAL_TYPE tol) {
    unsigned long n = g->N * g->N;
    DECIMAL_TYPE *r = malloc(n * sizeof(DECIMAL_TYPE));
    DECIMAL_TYPE *p = malloc(n * sizeof(DECIMAL_TYPE));
    DECIMAL_TYPE *Ap = malloc(n * sizeof(DECIMAL_TYPE));
    if (!r || !p || !Ap) {
        abort();
    }

    CONCAT(ALG_PREFIX, _ax)(g, x, Ap, dt);
    for (unsigned long i = 0; i < n; i++) {
        r[i] = b[i] - Ap[i];
    }

    for (unsigned long i = 0; i < n; ++i) {
        p[i] = r[i];
    }

    DECIMAL_TYPE rsold = CONCAT(ALG_PREFIX, _dot)(r, r, n);
    DECIMAL_TYPE rs0 = rsold;
    if (rsold == (DECIMAL_TYPE) 0) {
        free(r);
        free(p);
        free(Ap);
        return 0;
    }

    int iter = 0;
    for (iter = 0; iter < max_iter; ++iter) {
        CONCAT(ALG_PREFIX, _ax)(g, p, Ap, dt);
        DECIMAL_TYPE alpha = rsold / CONCAT(ALG_PREFIX, _dot)(p, Ap, n);

        for (unsigned long i = 0; i < n; ++i) {
            x[i] += alpha * p[i];
        }
        for (unsigned long i = 0; i < n; ++i) {
            r[i] -= alpha * Ap[i];
        }

        DECIMAL_TYPE rsnew = CONCAT(ALG_PREFIX, _dot)(r, r, n);
        DECIMAL_TYPE rel = CONCAT(ALG_PREFIX, _sqrt_approx)((rsnew) / rs0);
        if (rel <= tol) {
            break;
        }
        DECIMAL_TYPE beta = rsnew / rsold;
        for (unsigned long i = 0; i < n; ++i) {
            p[i] = r[i] + beta * p[i];
        }
        rsold = rsnew;
    }

    free(r);
    free(p);
    free(Ap);
    return iter;
}

static void CONCAT(ALG_PREFIX, _timestep_implicit)(CONCAT(ALG_PREFIX, _Grid_t) * g, DECIMAL_TYPE dt, int cg_maxit,
                                                   DECIMAL_TYPE cg_tol) {
    unsigned long n = g->N * g->N;

    DECIMAL_TYPE *rhs = malloc(n * sizeof(DECIMAL_TYPE));
    DECIMAL_TYPE *u_new = malloc(n * sizeof(DECIMAL_TYPE));
    if (!rhs || !u_new) {
        abort();
    }

    for (unsigned long i = 0; i < n; ++i) {
        rhs[i] = g->u[i];
        u_new[i] = g->u[i];
    }

    int iters = CONCAT(ALG_PREFIX, _cg_solve)(g, u_new, rhs, dt, cg_maxit, cg_tol);
    (void) iters;

    for (unsigned long i = 0; i < n; ++i) {
        g->u[i] = u_new[i];
    }

    free(rhs);
    free(u_new);
}

static DECIMAL_TYPE CONCAT(ALG_PREFIX, _total_energy)(const CONCAT(ALG_PREFIX, _Grid_t) * g) {
    unsigned long n = g->N * g->N;
    return CONCAT(ALG_PREFIX, _norm)(g->u, n);
}

DECIMAL_TYPE CONCAT(ALG_PREFIX, _run_simulation)(unsigned long N, int steps, DECIMAL_TYPE dt, int cg_maxit,
                                                 DECIMAL_TYPE cg_tol) {
    CONCAT(ALG_PREFIX, _Grid_t) grid;
    CONCAT(ALG_PREFIX, _grid_init)(&grid, N);
    CONCAT(ALG_PREFIX, _initialize_gaussian)(&grid, DEC(1.0));

    for (int s = 0; s < steps; s++) {
        DECIMAL_TYPE adaptive_tol = cg_tol;
        if (s < steps / 3) {
            adaptive_tol = cg_tol * (DECIMAL_TYPE) 10;
        } else if (s < 2 * steps / 3) {
            adaptive_tol = cg_tol * (DECIMAL_TYPE) 2;
        }
        CONCAT(ALG_PREFIX, _timestep_implicit)(&grid, dt, cg_maxit, adaptive_tol);
    }

    DECIMAL_TYPE energy = CONCAT(ALG_PREFIX, _total_energy)(&grid);
    CONCAT(ALG_PREFIX, _grid_free)(&grid);
    return energy;
}
