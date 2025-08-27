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

extern int x;
#if !defined(__OpenBSD__) && !defined(__FreeBSD__)
extern _Thread_local int y;
#else
extern int y;
#endif

int getx(void);
int gety(void);

int main(void) {
    assert(getx() == 0);
    assert(gety() == 0);

    x = 1000;
    y = 2000;
    assert(getx() == 1000);
    assert(gety() == 2000);
    return EXIT_SUCCESS;
}
