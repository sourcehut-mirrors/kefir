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

#define STRUCT_VEC2 CONCAT(ALG_PREFIX, _vec2)
#define VEC2_T CONCAT(ALG_PREFIX, _vec2_t)
#define STRUCT_BODY CONCAT(ALG_PREFIX, _body)
#define BODY_T CONCAT(ALG_PREFIX, _body_t)
#define ADD_VECTORS CONCAT(ALG_PREFIX, _add_vectors)
#define SUB_VECTORS CONCAT(ALG_PREFIX, _sub_vectors)
#define SCALE_VECTOR CONCAT(ALG_PREFIX, _scale_vector)
#define SQRT CONCAT(ALG_PREFIX, _sqrt)
#define GRAVITY_ACCELERATION CONCAT(ALG_PREFIX, _gravity_acceleration)
#define RK4_STEP CONCAT(ALG_PREFIX, _rk4_step)
#define SIMULATE CONCAT(ALG_PREFIX, _simulate)

#define EPSILON 1.0e-10df

typedef struct STRUCT_VEC2 {
    NUMBER_T x;
    NUMBER_T y;
} VEC2_T;

typedef struct STRUCT_BODY {
    VEC2_T pos;
    VEC2_T vel;
    NUMBER_T mass;
} BODY_T;

static inline VEC2_T ADD_VECTORS(VEC2_T a, VEC2_T b) {
    return (VEC2_T) {.x = a.x + b.x, .y = a.y + b.y};
}

static inline VEC2_T SUB_VECTORS(VEC2_T a, VEC2_T b) {
    return (VEC2_T) {.x = a.x - b.x, .y = a.y - b.y};
}

static inline VEC2_T SCALE_VECTOR(VEC2_T a, NUMBER_T scale) {
    return (VEC2_T) {.x = a.x * scale, .y = a.y * scale};
}

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

static VEC2_T GRAVITY_ACCELERATION(BODY_T body1, BODY_T body2, NUMBER_T G) {
    const VEC2_T diff = SUB_VECTORS(body2.pos, body1.pos);
    const NUMBER_T distance = diff.x * diff.x + diff.y * diff.y + EPSILON;
    const NUMBER_T distance_impact = 1.0df / (distance * SQRT(distance));
    const NUMBER_T factor = G * body2.mass * distance_impact;
    return SCALE_VECTOR(diff, factor);
}

static void RK4_STEP(BODY_T *a, BODY_T *b, NUMBER_T G, NUMBER_T delta) {
    const VEC2_T a_acc1 = GRAVITY_ACCELERATION(*a, *b, G);
    const VEC2_T b_acc1 = GRAVITY_ACCELERATION(*b, *a, G);

#define SCALE 0.5df
    const VEC2_T a_velocity1 = ADD_VECTORS(a->vel, SCALE_VECTOR(a_acc1, delta * SCALE));
    const VEC2_T b_velocity1 = ADD_VECTORS(b->vel, SCALE_VECTOR(b_acc1, delta * SCALE));

    const VEC2_T a_position1 = ADD_VECTORS(a->pos, SCALE_VECTOR(a->vel, delta * SCALE));
    const VEC2_T b_position1 = ADD_VECTORS(b->pos, SCALE_VECTOR(b->vel, delta * SCALE));
#undef SCALE

    const BODY_T a_body1 = {.pos = a_position1, .vel = a_velocity1, .mass = a->mass};

    const BODY_T b_body1 = {.pos = b_position1, .vel = b_velocity1, .mass = b->mass};

    const VEC2_T a_acc2 = GRAVITY_ACCELERATION(a_body1, b_body1, G);
    const VEC2_T b_acc2 = GRAVITY_ACCELERATION(b_body1, a_body1, G);

    const VEC2_T a_velocity2 = ADD_VECTORS(a->vel, SCALE_VECTOR(a_acc2, delta));
    const VEC2_T b_velocity2 = ADD_VECTORS(b->vel, SCALE_VECTOR(b_acc2, delta));

    const VEC2_T a_position2 = ADD_VECTORS(a->pos, SCALE_VECTOR(a->vel, delta));
    const VEC2_T b_position2 = ADD_VECTORS(b->pos, SCALE_VECTOR(b->vel, delta));

    const BODY_T a_body2 = {.pos = a_position2, .vel = a_velocity2, .mass = a->mass};

    const BODY_T b_body2 = {.pos = b_position2, .vel = b_velocity2, .mass = b->mass};

#define ACC_SCALE 0.25df
#define ACC2_IMPACT 2.0df
    const VEC2_T a_acceleration = SCALE_VECTOR(
        ADD_VECTORS(ADD_VECTORS(a_acc1, SCALE_VECTOR(a_acc2, ACC2_IMPACT)), GRAVITY_ACCELERATION(a_body2, b_body2, G)),
        ACC_SCALE);

    const VEC2_T b_acceleration = SCALE_VECTOR(
        ADD_VECTORS(ADD_VECTORS(b_acc1, SCALE_VECTOR(b_acc2, ACC2_IMPACT)), GRAVITY_ACCELERATION(b_body2, a_body2, G)),
        ACC_SCALE);
#undef ACC2_IMPACT
#undef ACC_SCALE

    a->vel = ADD_VECTORS(a->vel, SCALE_VECTOR(a_acceleration, delta));
    b->vel = ADD_VECTORS(b->vel, SCALE_VECTOR(b_acceleration, delta));

    a->pos = ADD_VECTORS(a->pos, SCALE_VECTOR(a->vel, delta));
    b->pos = ADD_VECTORS(b->pos, SCALE_VECTOR(b->vel, delta));
}

NUMBER_T SIMULATE(unsigned int steps, NUMBER_T seed, NUMBER_T G, NUMBER_T delta) {
    BODY_T a = {.pos = {.x = -1.0df * seed, .y = 0.0df}, .vel = {.x = 0.0df, .y = 0.4df * seed}, .mass = 1.0df};
    BODY_T b = {.pos = {.x = 1.0df * seed, .y = 0.0df}, .vel = {.x = 0.0df, .y = -0.4df * seed}, .mass = 1.0df};

    for (unsigned int i = 0; i < steps; i++) {
        RK4_STEP(&a, &b, G, delta);
    }

#define SQR(_x) ((_x) * (_x))
    const NUMBER_T energy = (SQR(a.vel.x) + SQR(a.vel.y) + SQR(b.vel.x) + SQR(b.vel.y)) / 2;
    return energy;
}

#undef STRUCT_VEC2
#undef VEC2_T
#undef STRUCT_BODY
#undef BODY_T
#undef ADD_VECTORS
#undef SUB_VECTORS
#undef SCALE_VECTOR
#undef SQRT
#undef GRAVITY_ACCELERATION
#undef RK4_STEP
#undef SIMULATE
#undef EPSILON
