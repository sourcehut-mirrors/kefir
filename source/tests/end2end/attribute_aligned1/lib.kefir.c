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

#include "./definitions.h"

struct S1 {
    char c __attribute__((aligned));
};

struct S2 {
    char c __attribute__((aligned(_Alignof(int))));
};

struct S3 {
    char c __attribute__((__aligned__));
};

struct S4 {
    char c __attribute__((__aligned__(_Alignof(short))));
};

char Char1 __attribute__((aligned)) = '\0';
char Char2 __attribute__((aligned)) = '\0';
char Char3 __attribute__((aligned(4))) = '\0';
char Char4 __attribute__((aligned(4))) = '\0';

int Alignments[] = {_Alignof(struct S1), _Alignof(struct S2), _Alignof(struct S3), _Alignof(struct S4)};
