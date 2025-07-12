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

#define TEMP1 2
#define TEMP2 5

char file1[] =
#embed "file1.txt" limit(0) prefix({0, ) suffix(, 0 };) if_empty("Hello, cruel world!";)

    char file2[] =
#embed "file1.txt" limit(1) prefix({0, ) suffix(, 0 };) if_empty("\0";)

        char file3[] =
#embed "file1.txt" limit(8) prefix({0, ) suffix(, 0 };) if_empty("\0";)

            char file4[] =
#embed "file1.txt" limit(1ull << 32) prefix({0, ) suffix(, 0 };) if_empty("\0";)

                char file5[] =
#embed "file1.txt" prefix({0, ) suffix(, 0 };) if_empty("\0";)

                    char file6[] =
#embed "file2.txt" prefix({0, ) suffix(, 0 };) if_empty("\0";)

                        char file7[] =
#embed "file1.txt" prefix((((((((()))))))((())))) suffix(, , , , (((()((()))((((())))))))) if_empty("\0";)