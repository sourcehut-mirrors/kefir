/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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
#include "./definitions.h"

int main(void) {
    for (int i = -100; i <= 100; i++) {
        assert(dnot(i) == (!!(i) ? 1 : 0));
        assert(dnotf((float) i) == (!!((float) i) ? 1 : 0));
        assert(dnotd((double) i) == (!!((double) i) ? 1 : 0));
        assert(dnotld((long double) i) == (!!((long double) i) ? 1 : 0));
    }

    assert(dnotf(0.0f) == (!!(0.0f) ? 1 : 0));
    assert(dnotd(0.0) == (!!((0.0) ? 1 : 0)));
    assert(dnotld(0.0L) == (!!(0.0L) ? 1 : 0));
    return EXIT_SUCCESS;
}
