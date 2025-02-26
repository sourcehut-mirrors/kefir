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

unsigned char uchar_uchar(long x) {
    return (unsigned char) (unsigned char) x;
}

unsigned short uchar_ushort(long x) {
    return (unsigned short) (unsigned char) x;
}

unsigned int uchar_uint(long x) {
    return (unsigned int) (unsigned char) x;
}

char uchar_char(long x) {
    return (char) (unsigned char) x;
}

short uchar_short(long x) {
    return (short) (unsigned char) x;
}

int uchar_int(long x) {
    return (int) (unsigned char) x;
}

unsigned short ushort_ushort(long x) {
    return (unsigned short) (unsigned short) x;
}

unsigned int ushort_uint(long x) {
    return (unsigned int) (unsigned short) x;
}

short ushort_short(long x) {
    return (short) (unsigned short) x;
}

int ushort_int(long x) {
    return (int) (unsigned short) x;
}

unsigned char char_uchar(long x) {
    return (unsigned char) (char) x;
}

unsigned short char_ushort(long x) {
    return (unsigned short) (char) x;
}

unsigned int char_uint(long x) {
    return (unsigned int) (char) x;
}

char char_char(long x) {
    return (char) (char) x;
}

short char_short(long x) {
    return (short) (char) x;
}

int char_int(long x) {
    return (int) (char) x;
}

unsigned short short_ushort(long x) {
    return (unsigned short) (short) x;
}

unsigned int short_uint(long x) {
    return (unsigned int) (short) x;
}

short short_short(long x) {
    return (short) (short) x;
}

int short_int(long x) {
    return (int) (short) x;
}

unsigned int uint_uint(long x) {
    return (unsigned int) (unsigned int) x;
}

int uint_int(long x) {
    return (int) (unsigned int) x;
}

unsigned int int_uint(long x) {
    return (unsigned int) (int) x;
}

int int_int(long x) {
    return (int) (int) x;
}
