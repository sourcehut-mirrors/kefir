/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "./definitions.h"

#define EPSILON_F 1e-3f
#define EPSILON_D 1e-6

int main(void) {
    for (float i = -100.0f; i < 100.0f; i += 0.1f) {
        assert(equalsf(i, i));
        assert(!equalsf(i, i + 0.1));
        assert(greaterf(i + 0.1, i));
        assert(!greaterf(i, i));
        assert(!greaterf(i, i + 0.1));
        assert(lesserf(i, i + 0.1));
        assert(!lesserf(i, i));
        assert(!lesserf(i + 0.1, i));
        assert(greatereqf(i + 0.1, i));
        assert(greatereqf(i, i));
        assert(!greatereqf(i, i + 0.1));
        assert(lessereqf(i, i + 0.1));
        assert(lessereqf(i, i));
        assert(!lessereqf(i + 0.1, i));

        double j = (double) i;
        assert(equalsd(j, j));
        assert(!equalsd(j, j + 0.1));
        assert(greaterd(j + 0.1, j));
        assert(!greaterd(j, j));
        assert(!greaterd(j, j + 0.1));
        assert(lesserd(j, j + 0.1));
        assert(!lesserd(j, j));
        assert(!lesserd(j + 0.1, j));
        assert(greatereqd(j + 0.1, j));
        assert(greatereqd(j, j));
        assert(!greatereqd(j, j + 0.1));
        assert(lessereqd(j, j + 0.1));
        assert(lessereqd(j, j));
        assert(!lessereqd(j + 0.1, j));
    }

    return EXIT_SUCCESS;
}
