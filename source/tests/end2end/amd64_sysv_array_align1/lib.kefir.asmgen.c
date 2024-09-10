/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

char A[1];
char B[2];
char C[4];
char D[8];
char E[15];
char F[16];
char G[17];
char H[24];

char *geta(void) {
    char tmp[15];
    (void) tmp;
    char a[1];
    return a;
}

char *getb(void) {
    char tmp[15];
    (void) tmp;
    char b[2];
    return b;
}

char *getc(void) {
    char tmp[15];
    (void) tmp;
    char c[4];
    return c;
}

char *getd(void) {
    char tmp[15];
    (void) tmp;
    char d[8];
    return d;
}

char *gete(void) {
    char tmp[15];
    (void) tmp;
    char e[15];
    return e;
}

char *getf(void) {
    char tmp[15];
    (void) tmp;
    char f[16];
    return f;
}

char *getg(void) {
    char tmp[15];
    (void) tmp;
    char g[17];
    return g;
}

char *geth(void) {
    char tmp[15];
    (void) tmp;
    char h[24];
    return h;
}

char *vla1(int x) {
    char arr[x];
    return arr;
}
