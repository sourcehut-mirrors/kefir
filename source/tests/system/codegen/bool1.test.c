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
#include "kefir/core/util.h"
#include "kefir/test/unit_test.h"

bool bool_and(bool, bool);
bool bool_or(bool, bool);
bool bool_not(bool);

int main(int argc, const char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    ASSERT(bool_and(false, false) == false);
    ASSERT(bool_and(false, true) == false);
    ASSERT(bool_and(true, false) == false);
    ASSERT(bool_and(true, true) == true);
    ASSERT(bool_or(false, false) == false);
    ASSERT(bool_or(false, true) == true);
    ASSERT(bool_or(true, false) == true);
    ASSERT(bool_or(true, true) == true);
    ASSERT(bool_not(false) == true);
    ASSERT(bool_not(true) == false);
    return EXIT_SUCCESS;
}
