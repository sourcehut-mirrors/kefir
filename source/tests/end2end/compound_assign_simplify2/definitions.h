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

void add8(_Atomic char *, char);
void add16(_Atomic short *, short);
void add32(_Atomic int *, int);
void add64(_Atomic long *, long);

void sub8(_Atomic char *, char);
void sub16(_Atomic short *, short);
void sub32(_Atomic int *, int);
void sub64(_Atomic long *, long);

void and8(_Atomic char *, char);
void and16(_Atomic short *, short);
void and32(_Atomic int *, int);
void and64(_Atomic long *, long);

void or8(_Atomic char *, char);
void or16(_Atomic short *, short);
void or32(_Atomic int *, int);
void or64(_Atomic long *, long);

void xor8(_Atomic char *, char);
void xor16(_Atomic short *, short);
void xor32(_Atomic int *, int);
void xor64(_Atomic long *, long);

void neg8(_Atomic char *);
void neg16(_Atomic short *);
void neg32(_Atomic int *);
void neg64(_Atomic long *);

void not8(_Atomic char *);
void not16(_Atomic short *);
void not32(_Atomic int *);
void not64(_Atomic long *);

#endif
