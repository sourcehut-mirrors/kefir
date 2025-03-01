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

int char_short_or(char x, short y) {
    return x || y;
}

int char_short_and(char x, short y) {
    return x && y;
}

int char_int_or(char x, int y) {
    return x || y;
}

int char_int_and(char x, int y) {
    return x && y;
}

int char_long_or(char x, long y) {
    return x || y;
}

int char_long_and(char x, long y) {
    return x && y;
}

int short_char_or(short x, char y) {
    return x || y;
}

int short_char_and(short x, char y) {
    return x && y;
}

int short_int_or(short x, int y) {
    return x || y;
}

int short_int_and(short x, int y) {
    return x && y;
}

int short_long_or(short x, long y) {
    return x || y;
}

int short_long_and(short x, long y) {
    return x && y;
}

int int_char_or(int x, char y) {
    return x || y;
}

int int_char_and(int x, char y) {
    return x && y;
}

int int_short_or(int x, short y) {
    return x || y;
}

int int_short_and(int x, short y) {
    return x && y;
}

int int_long_or(int x, long y) {
    return x || y;
}

int int_long_and(int x, long y) {
    return x && y;
}

int long_char_or(long x, char y) {
    return x || y;
}

int long_char_and(long x, char y) {
    return x && y;
}

int long_short_or(long x, short y) {
    return x || y;
}

int long_short_and(long x, short y) {
    return x && y;
}

int long_int_or(long x, int y) {
    return x || y;
}

int long_int_and(long x, int y) {
    return x && y;
}
