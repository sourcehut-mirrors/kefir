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
#include <stdio.h>
#include <assert.h>
#include "./definitions.h"

int main(void) {
    assert(arr[0] == 1);
    assert(arr[1] == 2);
    assert(arr[2] == sizeof(int));
    assert(arr[3] == sizeof(int));

    assert(arr[4] == 1);
    assert(arr[5] == 1);
    assert(arr[6] == sizeof(int));
    assert(arr[7] == sizeof(int));

    assert(arr[8] == 1);
    assert(arr[9] == 2);
    assert(arr[10] == sizeof(int));
    assert(arr[11] == sizeof(int));

    assert(arr[12] == 2);
    assert(arr[13] == 2);
    assert(arr[14] == sizeof(int));
    assert(arr[15] == sizeof(int));

    assert(arr[16] == 2);
    assert(arr[17] == 2);
    assert(arr[18] == sizeof(int));
    assert(arr[19] == sizeof(int));

    assert(arr[20] == 4);
    assert(arr[21] == 4);
    assert(arr[22] == sizeof(long));
    assert(arr[23] == sizeof(long));

    assert(arr[24] == 3);
    assert(arr[25] == 3);
    assert(arr[26] == sizeof(long));
    assert(arr[27] == sizeof(long));

    assert(arr[28] == 4);
    assert(arr[29] == 4);
    assert(arr[30] == sizeof(long));
    assert(arr[31] == sizeof(long));

    assert(arr[32] == 4);
    assert(arr[33] == 4);
    assert(arr[34] == sizeof(long));
    assert(arr[35] == sizeof(long));

    assert(arr[36] == 3);
    assert(arr[37] == 3);
    assert(arr[38] == sizeof(long));
    assert(arr[39] == sizeof(long));

    assert(arr[40] == 4);
    assert(arr[41] == 4);
    assert(arr[42] == sizeof(long));
    assert(arr[43] == sizeof(long));

    assert(arr[44] == 4);
    assert(arr[45] == 4);
    assert(arr[46] == sizeof(long));
    assert(arr[47] == sizeof(long));
    return EXIT_SUCCESS;
}
