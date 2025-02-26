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

char char_or(char x, char y) {
    return x || y;
}

char char_or3(char x, char y, char z) {
    return x || y || z;
}

char char_and(char x, char y) {
    return x && y;
}

char char_and3(char x, char y, char z) {
    return x && y && z;
}

short short_or(short x, short y) {
    return x || y;
}

short short_or3(short x, short y, short z) {
    return x || y || z;
}

short short_and(short x, short y) {
    return x && y;
}

short short_and3(short x, short y, short z) {
    return x && y && z;
}

int int_or(int x, int y) {
    return x || y;
}

int int_or3(int x, int y, int z) {
    return x || y || z;
}

int int_and(int x, int y) {
    return x && y;
}

int int_and3(int x, int y, int z) {
    return x && y && z;
}

long long_or(long x, long y) {
    return x || y;
}

long long_or3(long x, long y, long z) {
    return x || y || z;
}

long long_and(long x, long y) {
    return x && y;
}

long long_and3(long x, long y, long z) {
    return x && y && z;
}
