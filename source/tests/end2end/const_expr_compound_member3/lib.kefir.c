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

#define VALUE                                                                                    \
    ((struct S4) {{"Hello, world"},                                                              \
                  1000,                                                                          \
                  {.b = {-123,                                                                   \
                         {                                                                       \
                             .b = {.a = -5492, 2.71828f, .c = "Hey ho!", 0xface0, .e = "Nope!"}, \
                         },                                                                      \
                         {10281, -3.14159f, "Goodbye, world", -0xfefe, .e = "What?"}}}})

struct S1 s1_1 = VALUE.c.b.d.b;
struct S1 s1_2 = VALUE.c.b.e;
struct S2 s2 = VALUE.c.b;
union S3 s3_1 = VALUE.c;
struct S3_2 s3_2 = VALUE.a;