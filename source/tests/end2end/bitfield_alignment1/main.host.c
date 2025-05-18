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

#include "./definitions.h"
#include <stdlib.h>
#include <assert.h>

int main(void) {
    assert(sz_align[0] == sizeof(struct S1));
    assert(sz_align[1] == _Alignof(struct S1));
    assert(sz_align[2] == sizeof(struct S2));
    assert(sz_align[3] == _Alignof(struct S2));
    assert(sz_align[4] == 3);
    assert(sz_align[5] == 1);
    assert(sz_align[6] == 3);
    assert(sz_align[7] == 1);
    assert(sz_align[8] == sizeof(struct S5));
    assert(sz_align[9] == _Alignof(struct S5));
    assert(sz_align[10] == 3);
    assert(sz_align[11] == 1);
    return EXIT_SUCCESS;
}
