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
    assert(get_a() == 123456789);
    assert(fabs(get_b() - 3.14159f) < 1e-5);
    assert(fabs(get_c() - 3.1415926) < 1e-6);
    assert(fabsl(get_d() + 2.718281828L) < 1e-7);
    assert(fabs(creal(get_e()) - 1.234f) < 1e-5);
    assert(fabs(cimag(get_e()) - 187.0f) < 1e-5);
    assert(fabs(creal(get_f()) - 3.234f) < 1e-6);
    assert(fabs(cimag(get_f()) + 187.0f) < 1e-6);
    assert(fabsl(creall(get_g()) - 2.234f) < 1e-7);
    assert(fabsl(cimagl(get_g()) + 187.0f) < 1e-7);
    assert(get_h() == NULL);
    assert(strcmp(get_i(), "ello, world") == 0);
    for (int i = 0; get_j()[i] != 0; i++) {
        assert(get_j()[i] == (unsigned int) ("llo, world"[i]));
    }
    return EXIT_SUCCESS;
}
