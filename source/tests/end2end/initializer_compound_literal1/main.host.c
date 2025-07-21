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

#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <complex.h>
#include <string.h>
#include "./definitions.h"

int main(void) {
    for (int i = -256; i < 256; i++) {
        struct S3 s3 = get(i);

        assert(s3.a.a == i);
        assert(s3.a.b.a == i + 1);
        assert(s3.a.b.b == i + 2);
        assert(fabs(s3.a.b.c - (i + 3)) < 1e-5);
        assert(fabs(s3.a.c - (i + 4)) < 1e-6);
        assert(s3.a.d[0].a == (i + 5));
        assert(s3.a.d[0].b == (i + 6));
        assert(fabs(s3.a.d[0].c - (i + 7)) < 1e-5);
        assert(s3.a.d[1].a == 0);
        assert(s3.a.d[1].b == 0);
        assert(fabs(s3.a.d[1].c) < 1e-5);
        assert(s3.a.d[2].a == 0);
        assert(s3.a.d[2].b == 0);
        assert(fabs(s3.a.d[2].c) < 1e-5);
        assert(s3.a.e.a == (i + 8));
        assert(s3.a.e.b == (i + 9));
        assert(fabs(s3.a.e.c - (i + 10)) < 1e-5);
        assert(s3.b.a == i + 11);
        assert(s3.b.b == i + 12);
        assert(fabs(s3.b.c - (i + 13)) < 1e-5);
        assert(s3.c[0].a == -1);
        assert(s3.c[0].b.a == 0);
        assert(s3.c[0].b.b == 0);
        assert(fabs(s3.c[0].b.c) < 1e-5);
        assert(fabs(s3.c[0].c) < 1e-6);
        assert(memcmp(&s3.c[0].d[0], &(struct S1) {0}, sizeof(struct S1)) == 0);
        assert(memcmp(&s3.c[0].d[1], &(struct S1) {0}, sizeof(struct S1)) == 0);
        assert(memcmp(&s3.c[0].d[2], &(struct S1) {0}, sizeof(struct S1)) == 0);
        assert(memcmp(&s3.c[0].e, &(struct S1) {0}, sizeof(struct S1)) == 0);
        assert(s3.c[1].a == -1);
        assert(s3.c[1].b.a == 0);
        assert(s3.c[1].b.b == 0);
        assert(fabs(s3.c[1].b.c) < 1e-5);
        assert(fabs(s3.c[1].c) < 1e-6);
        assert(memcmp(&s3.c[1].d[0], &(struct S1) {0}, sizeof(struct S1)) == 0);
        assert(memcmp(&s3.c[1].d[1], &(struct S1) {0}, sizeof(struct S1)) == 0);
        assert(memcmp(&s3.c[1].d[2], &(struct S1) {0}, sizeof(struct S1)) == 0);
        assert(memcmp(&s3.c[1].e, &(struct S1) {0}, sizeof(struct S1)) == 0);
        assert(s3.d.a == (i + 14));
        assert(s3.d.b.a == (i + 15));
        assert(s3.d.b.b == (i + 16));
        assert(fabs(s3.d.b.c - (i + 17)) < 1e-5);
        assert(fabs(s3.d.c - (i + 18)) < 1e-6);
        assert(memcmp(&s3.d.d[0], &(struct S1) {0}, sizeof(struct S1)) == 0);
        assert(memcmp(&s3.d.d[1], &(struct S1) {0}, sizeof(struct S1)) == 0);
        assert(memcmp(&s3.d.d[2], &(struct S1) {0}, sizeof(struct S1)) == 0);
        assert(s3.d.e.a == (i + 19));
        assert(s3.d.e.b == (i + 20));
        assert(fabs(s3.d.e.c - (i + 21)) < 1e-5);
    }
    return EXIT_SUCCESS;
}
