/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

char file1[] = {
#embed "file1.txt" prefix(TEST123) suffix(TEST654)
};

char file2[] = {
#embed "file2.txt" prefix(TEST123) suffix(TEST654)
};

char file3[] = {
#embed "file1.txt" prefix() suffix(TEST654)
};

char file4[] = {
#embed "file1.txt" prefix(TEST123) suffix()
};

#define TEST123 "BEGIN"
#define TEST654 "END"

char file5[] = {
#embed "file1.txt" prefix(TEST123) suffix(TEST654)
};

char file6[] = {
#embed "file2.txt" prefix(TEST123) suffix(TEST654)
};

char file7[] = {
#embed "file1.txt" prefix() suffix(TEST654)
};

char file8[] = {
#embed "file1.txt" prefix(TEST123) suffix()
};