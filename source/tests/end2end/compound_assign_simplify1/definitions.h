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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

void add8(char *, char);
void add16(short *, short);
void add32(int *, int);
void add64(long *, long);

void sub8(char *, char);
void sub16(short *, short);
void sub32(int *, int);
void sub64(long *, long);

void and8(char *, char);
void and16(short *, short);
void and32(int *, int);
void and64(long *, long);

void or8(char *, char);
void or16(short *, short);
void or32(int *, int);
void or64(long *, long);

void xor8(char *, char);
void xor16(short *, short);
void xor32(int *, int);
void xor64(long *, long);

void neg8(char *);
void neg16(short *);
void neg32(int *);
void neg64(long *);

void not8(char *);
void not16(short *);
void not32(int *);
void not64(long *);

#endif
