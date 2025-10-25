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
#include <string.h>
#include "./definitions.h"

int main(void) {
#ifdef __x86_64__
    assert(exchange1(0xcafe) == 0xfeca);
    assert(exchange1(0xff00) == 0x00ff);
    assert(exchange1(0x00ff) == 0xff00);
    assert(exchange1(0) == 0);
    assert(exchange1(-1) == 0xffff);

    assert(exchange2(0xcafe) == 0xfeca);
    assert(exchange2(0xff00) == 0x00ff);
    assert(exchange2(0x00ff) == 0xff00);
    assert(exchange2(0) == 0);
    assert(exchange2(-1) == 0xffff);

    assert(exchange3(0xcafe) == 0xfeca);
    assert(exchange3(0xff00) == 0x00ff);
    assert(exchange3(0x00ff) == 0xff00);
    assert(exchange3(0) == 0);
    assert(exchange3(-1) == 0xffff);

    assert(exchange4(0xcafe) == 0xfeca);
    assert(exchange4(0xff00) == 0x00ff);
    assert(exchange4(0x00ff) == 0xff00);
    assert(exchange4(0) == 0);
    assert(exchange4(-1) == 0xffff);
#endif
    return EXIT_SUCCESS;
}
