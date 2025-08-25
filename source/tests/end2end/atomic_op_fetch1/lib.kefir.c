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

char add_fetch8(char *ptr, char x) {
    return __atomic_add_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

short add_fetch16(short *ptr, short x) {
    return __atomic_add_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

int add_fetch32(int *ptr, int x) {
    return __atomic_add_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

long add_fetch64(long *ptr, long x) {
    return __atomic_add_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

char sub_fetch8(char *ptr, char x) {
    return __atomic_sub_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

short sub_fetch16(short *ptr, short x) {
    return __atomic_sub_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

int sub_fetch32(int *ptr, int x) {
    return __atomic_sub_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

long sub_fetch64(long *ptr, long x) {
    return __atomic_sub_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

char and_fetch8(char *ptr, char x) {
    return __atomic_and_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

short and_fetch16(short *ptr, short x) {
    return __atomic_and_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

int and_fetch32(int *ptr, int x) {
    return __atomic_and_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

long and_fetch64(long *ptr, long x) {
    return __atomic_and_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

char or_fetch8(char *ptr, char x) {
    return __atomic_or_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

short or_fetch16(short *ptr, short x) {
    return __atomic_or_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

int or_fetch32(int *ptr, int x) {
    return __atomic_or_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

long or_fetch64(long *ptr, long x) {
    return __atomic_or_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

char xor_fetch8(char *ptr, char x) {
    return __atomic_xor_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

short xor_fetch16(short *ptr, short x) {
    return __atomic_xor_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

int xor_fetch32(int *ptr, int x) {
    return __atomic_xor_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

long xor_fetch64(long *ptr, long x) {
    return __atomic_xor_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

char nand_fetch8(char *ptr, char x) {
    return __atomic_nand_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

short nand_fetch16(short *ptr, short x) {
    return __atomic_nand_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

int nand_fetch32(int *ptr, int x) {
    return __atomic_nand_fetch(ptr, x, __ATOMIC_SEQ_CST);
}

long nand_fetch64(long *ptr, long x) {
    return __atomic_nand_fetch(ptr, x, __ATOMIC_SEQ_CST);
}
