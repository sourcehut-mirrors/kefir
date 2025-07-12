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

char file1[] = {
#embed "file1.txt" limit(0)
};

char file2[] = {
#embed "file1.txt" limit(4)
};

char file3[] = {
#embed "file1.txt" limit(4 + 1)
};

char file4[] = {
#embed "file1.txt" limit((TEMP1 << 2) * TEMP2)
};

char file5[] = {
#embed "file1.txt" limit(~0ull)
};
