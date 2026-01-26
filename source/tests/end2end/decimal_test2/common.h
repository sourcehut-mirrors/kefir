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
// The code implements Simple 2-body gravitational attraction in 2D, with RK4
// integration.
//

#ifdef CONCAT
#undef CONCAT
#endif

#ifdef CONCAT2
#undef CONCAT2
#endif

#define CONCAT2(a, b) a##b
#define CONCAT(a, b) CONCAT2(a, b)

typedef struct CONCAT(ALG_PREFIX, _vec2) {
    DECIMAL_TYPE x;
    DECIMAL_TYPE y;
} CONCAT(ALG_PREFIX, _vec2_t);

typedef struct CONCAT(ALG_PREFIX, _body) {
    CONCAT(ALG_PREFIX, _vec2_t) pos;
    CONCAT(ALG_PREFIX, _vec2_t) vel;
    DECIMAL_TYPE mass;
} CONCAT(ALG_PREFIX, _body_t);

static inline CONCAT(ALG_PREFIX, _vec2_t)
    CONCAT(ALG_PREFIX, _addv)(CONCAT(ALG_PREFIX, _vec2_t) a, CONCAT(ALG_PREFIX, _vec2_t) b) {
    return (CONCAT(ALG_PREFIX, _vec2_t)) {a.x + b.x, a.y + b.y};
}

static inline CONCAT(ALG_PREFIX, _vec2_t)
    CONCAT(ALG_PREFIX, _subv)(CONCAT(ALG_PREFIX, _vec2_t) a, CONCAT(ALG_PREFIX, _vec2_t) b) {
    return (CONCAT(ALG_PREFIX, _vec2_t)) {a.x - b.x, a.y - b.y};
}

static inline CONCAT(ALG_PREFIX, _vec2_t) CONCAT(ALG_PREFIX, _mulv)(CONCAT(ALG_PREFIX, _vec2_t) a, DECIMAL_TYPE s) {
    return (CONCAT(ALG_PREFIX, _vec2_t)) {a.x * s, a.y * s};
}

DECIMAL_TYPE CONCAT(ALG_PREFIX, _sqrt_approx)(DECIMAL_TYPE x) {
    DECIMAL_TYPE g = x * 0.5df + 0.5df;
    for (int i = 0; i < 6; i++) {
        g = 0.5df * (g + x / g);
    }
    return g;
}

CONCAT(ALG_PREFIX, _vec2_t)
CONCAT(ALG_PREFIX, _grav_accel)(CONCAT(ALG_PREFIX, _body_t) b1, CONCAT(ALG_PREFIX, _body_t) b2, DECIMAL_TYPE G) {
    CONCAT(ALG_PREFIX, _vec2_t) r = CONCAT(ALG_PREFIX, _subv)(b2.pos, b1.pos);
    DECIMAL_TYPE dist2 = r.x * r.x + r.y * r.y + 1.0e-10df;
    DECIMAL_TYPE inv_dist3 = 1.0df / (dist2 * CONCAT(ALG_PREFIX, _sqrt_approx)(dist2));
    DECIMAL_TYPE factor = G * b2.mass * inv_dist3;
    return CONCAT(ALG_PREFIX, _mulv)(r, factor);
}

void CONCAT(ALG_PREFIX, _rk4_step)(CONCAT(ALG_PREFIX, _body_t) * a, CONCAT(ALG_PREFIX, _body_t) * b, DECIMAL_TYPE G,
                                   DECIMAL_TYPE dt) {
    CONCAT(ALG_PREFIX, _vec2_t) a0 = CONCAT(ALG_PREFIX, _grav_accel)(*a, *b, G);
    CONCAT(ALG_PREFIX, _vec2_t) b0 = CONCAT(ALG_PREFIX, _grav_accel)(*b, *a, G);

    CONCAT(ALG_PREFIX, _vec2_t) a1_v = CONCAT(ALG_PREFIX, _addv)(a->vel, CONCAT(ALG_PREFIX, _mulv)(a0, dt * 0.5df));
    CONCAT(ALG_PREFIX, _vec2_t) b1_v = CONCAT(ALG_PREFIX, _addv)(b->vel, CONCAT(ALG_PREFIX, _mulv)(b0, dt * 0.5df));

    CONCAT(ALG_PREFIX, _vec2_t) a1_p = CONCAT(ALG_PREFIX, _addv)(a->pos, CONCAT(ALG_PREFIX, _mulv)(a->vel, dt * 0.5df));
    CONCAT(ALG_PREFIX, _vec2_t) b1_p = CONCAT(ALG_PREFIX, _addv)(b->pos, CONCAT(ALG_PREFIX, _mulv)(b->vel, dt * 0.5df));

    CONCAT(ALG_PREFIX, _body_t) a_tmp = {a1_p, a1_v, a->mass};
    CONCAT(ALG_PREFIX, _body_t) b_tmp = {b1_p, b1_v, b->mass};

    CONCAT(ALG_PREFIX, _vec2_t) a2 = CONCAT(ALG_PREFIX, _grav_accel)(a_tmp, b_tmp, G);
    CONCAT(ALG_PREFIX, _vec2_t) b2 = CONCAT(ALG_PREFIX, _grav_accel)(b_tmp, a_tmp, G);

    CONCAT(ALG_PREFIX, _vec2_t) a3_v = CONCAT(ALG_PREFIX, _addv)(a->vel, CONCAT(ALG_PREFIX, _mulv)(a2, dt));
    CONCAT(ALG_PREFIX, _vec2_t) b3_v = CONCAT(ALG_PREFIX, _addv)(b->vel, CONCAT(ALG_PREFIX, _mulv)(b2, dt));

    CONCAT(ALG_PREFIX, _vec2_t) a3_p = CONCAT(ALG_PREFIX, _addv)(a->pos, CONCAT(ALG_PREFIX, _mulv)(a->vel, dt));
    CONCAT(ALG_PREFIX, _vec2_t) b3_p = CONCAT(ALG_PREFIX, _addv)(b->pos, CONCAT(ALG_PREFIX, _mulv)(b->vel, dt));

    CONCAT(ALG_PREFIX, _vec2_t)
    a_acc = CONCAT(ALG_PREFIX, _mulv)(
        CONCAT(ALG_PREFIX, _addv)(
            CONCAT(ALG_PREFIX, _addv)(a0, CONCAT(ALG_PREFIX, _mulv)(a2, 2.0df)),
            CONCAT(ALG_PREFIX, _grav_accel)((CONCAT(ALG_PREFIX, _body_t)) {a3_p, a3_v, a->mass},
                                            (CONCAT(ALG_PREFIX, _body_t)) {b3_p, b3_v, b->mass}, G)),
        1.0df / 4.0df);

    CONCAT(ALG_PREFIX, _vec2_t)
    b_acc = CONCAT(ALG_PREFIX, _mulv)(
        CONCAT(ALG_PREFIX, _addv)(
            CONCAT(ALG_PREFIX, _addv)(b0, CONCAT(ALG_PREFIX, _mulv)(b2, 2.0df)),
            CONCAT(ALG_PREFIX, _grav_accel)((CONCAT(ALG_PREFIX, _body_t)) {b3_p, b3_v, b->mass},
                                            (CONCAT(ALG_PREFIX, _body_t)) {a3_p, a3_v, a->mass}, G)),
        1.0df / 4.0df);

    a->vel = CONCAT(ALG_PREFIX, _addv)(a->vel, CONCAT(ALG_PREFIX, _mulv)(a_acc, dt));
    b->vel = CONCAT(ALG_PREFIX, _addv)(b->vel, CONCAT(ALG_PREFIX, _mulv)(b_acc, dt));

    a->pos = CONCAT(ALG_PREFIX, _addv)(a->pos, CONCAT(ALG_PREFIX, _mulv)(a->vel, dt));
    b->pos = CONCAT(ALG_PREFIX, _addv)(b->pos, CONCAT(ALG_PREFIX, _mulv)(b->vel, dt));
}

DECIMAL_TYPE CONCAT(ALG_PREFIX, _simulate)(int steps, DECIMAL_TYPE dt) {
    DECIMAL_TYPE G = 1.0df;

    CONCAT(ALG_PREFIX, _body_t) a = {{-1.0df, 0.0df}, {0.0df, 0.4df}, 1.0df};
    CONCAT(ALG_PREFIX, _body_t) b = {{1.0df, 0.0df}, {0.0df, -0.4df}, 1.0df};

    for (int i = 0; i < steps; i++) {
        CONCAT(ALG_PREFIX, _rk4_step)(&a, &b, G, dt);
    }

    DECIMAL_TYPE energy = 0.5df * (a.vel.x * a.vel.x + a.vel.y * a.vel.y + b.vel.x * b.vel.x + b.vel.y * b.vel.y);
    return energy;
}
