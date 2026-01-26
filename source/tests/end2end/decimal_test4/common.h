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

// The code below has been produced with assistance of a large language model.
// The author disclaims any responsibility with respect to it, and it is
// provided for a sole purpose of creating a sufficiently diverse and
// sophisticated test case for differential testing against gcc.
//
// In case of any doubts regarding potential copyright infringement, please
// contact the author immediately via <jevgenij@protopopov.lv>.
//
// The code implements simulation of a 2D nonlinear reaction–diffusion system
// (Gray–Scott model).

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

typedef struct CONCAT(ALG_PREFIX, _Grid) {
    unsigned long N;
    DECIMAL_TYPE dx;
    DECIMAL_TYPE *U;
    DECIMAL_TYPE *V;
} CONCAT(ALG_PREFIX, _Grid_t);

static inline unsigned long CONCAT(ALG_PREFIX, _idx)(unsigned long N, unsigned long i, unsigned long j) {
    return i * N + j;
}

static void CONCAT(ALG_PREFIX, _init)(CONCAT(ALG_PREFIX, _Grid_t) * g, unsigned long N) {
    g->N = N;
    g->dx = DEC(1.0) / (DECIMAL_TYPE) (N + 1);
    g->U = malloc(N * N * sizeof(DECIMAL_TYPE));
    g->V = malloc(N * N * sizeof(DECIMAL_TYPE));
    if (!g->U || !g->V) {
        abort();
    }
    for (unsigned long i = 0; i < N; i++) {
        for (unsigned long j = 0; j < N; j++) {
            g->U[CONCAT(ALG_PREFIX, _idx)(N, i, j)] = DEC(1.0);
            g->V[CONCAT(ALG_PREFIX, _idx)(N, i, j)] = DEC(0.0);
        }
    }
    unsigned long c = N / 2;
    for (unsigned long i = c - 1; i <= c + 1; i++)
        for (unsigned long j = c - 1; j <= c + 1; j++) {
            g->V[CONCAT(ALG_PREFIX, _idx)(N, i, j)] = DEC(0.25);
            g->U[CONCAT(ALG_PREFIX, _idx)(N, i, j)] = DEC(0.50);
        }
}

static void CONCAT(ALG_PREFIX, _laplacian)(const DECIMAL_TYPE *field, DECIMAL_TYPE *out, unsigned long N,
                                           DECIMAL_TYPE dx) {
    DECIMAL_TYPE inv_dx2 = DEC(1.0) / (dx * dx);
    for (unsigned long i = 0; i < N; i++) {
        for (unsigned long j = 0; j < N; j++) {
            DECIMAL_TYPE center = field[CONCAT(ALG_PREFIX, _idx)(N, i, j)];
            DECIMAL_TYPE up = (i > 0) ? field[CONCAT(ALG_PREFIX, _idx)(N, i - 1, j)] : DEC(0.0);
            DECIMAL_TYPE down = (i + 1 < N) ? field[CONCAT(ALG_PREFIX, _idx)(N, i + 1, j)] : DEC(0.0);
            DECIMAL_TYPE left = (j > 0) ? field[CONCAT(ALG_PREFIX, _idx)(N, i, j - 1)] : DEC(0.0);
            DECIMAL_TYPE right = (j + 1 < N) ? field[CONCAT(ALG_PREFIX, _idx)(N, i, j + 1)] : DEC(0.0);
            out[CONCAT(ALG_PREFIX, _idx)(N, i, j)] = (up + down + left + right - DEC(4.0) * center) * inv_dx2;
        }
    }
}

static void CONCAT(ALG_PREFIX, _step)(CONCAT(ALG_PREFIX, _Grid_t) * g, DECIMAL_TYPE F, DECIMAL_TYPE k,
                                      DECIMAL_TYPE dt) {
    unsigned long N = g->N;
    DECIMAL_TYPE *lapU = malloc(N * N * sizeof(DECIMAL_TYPE));
    DECIMAL_TYPE *lapV = malloc(N * N * sizeof(DECIMAL_TYPE));
    if (!lapU || !lapV) {
        abort();
    }

    CONCAT(ALG_PREFIX, _laplacian)(g->U, lapU, N, g->dx);
    CONCAT(ALG_PREFIX, _laplacian)(g->V, lapV, N, g->dx);

    for (unsigned long i = 0; i < N; i++) {
        for (unsigned long j = 0; j < N; j++) {
            unsigned long idx = CONCAT(ALG_PREFIX, _idx)(N, i, j);
            DECIMAL_TYPE u = g->U[idx];
            DECIMAL_TYPE v = g->V[idx];
            DECIMAL_TYPE uvv = u * v * v;
            g->U[idx] += dt * (lapU[idx] - uvv + F * (DEC(1.0) - u));
            g->V[idx] += dt * (lapV[idx] + uvv - (F + k) * v);
            if (g->U[idx] < DEC(0.0)) {
                g->U[idx] = DEC(0.0);
            }
            if (g->V[idx] < DEC(0.0)) {
                g->V[idx] = DEC(0.0);
            }
        }
    }
    free(lapU);
    free(lapV);
}

static DECIMAL_TYPE CONCAT(ALG_PREFIX, _norm)(DECIMAL_TYPE *f, unsigned long N) {
    DECIMAL_TYPE s = DEC(0.0);
    for (unsigned long i = 0; i < N * N; i++) {
        s += f[i] * f[i];
    }
    return s;
}

DECIMAL_TYPE CONCAT(ALG_PREFIX, _run)(unsigned long N, int steps) {
    CONCAT(ALG_PREFIX, _Grid_t) g;
    CONCAT(ALG_PREFIX, _init)(&g, N);
    DECIMAL_TYPE F = DEC(0.04);
    DECIMAL_TYPE k = DEC(0.06);
    DECIMAL_TYPE dt = DEC(1.0e-3);
    for (int s = 0; s < steps; s++) {
        CONCAT(ALG_PREFIX, _step)(&g, F, k, dt);
    }
    DECIMAL_TYPE normU = CONCAT(ALG_PREFIX, _norm)(g.U, N);
    DECIMAL_TYPE normV = CONCAT(ALG_PREFIX, _norm)(g.V, N);
    free(g.U);
    free(g.V);
    return normU + normV;
}
