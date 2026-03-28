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

void sub8(_Atomic char *, char);
void sub16(_Atomic short *, short);
void sub32(_Atomic int *, int);
void sub64(_Atomic long *, long);
void subf32(_Atomic float *, float);
void subf64(_Atomic double *, double);

char sub8r(_Atomic char *, char);
short sub16r(_Atomic short *, short);
int sub32r(_Atomic int *, int);
long sub64r(_Atomic long *, long);
float subf32r(_Atomic float *, float);
double subf64r(_Atomic double *, double);

#endif
