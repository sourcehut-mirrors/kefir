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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

extern void *a;
extern _Bool b;
extern int c, d;
extern void *get(void);
extern _Bool is(void *);
extern _Bool is2(void);
extern void test1(int *);
extern void set1(void **);
extern void set2(_Bool *);
extern _Bool and1(_Bool);
extern _Bool and2(_Bool);
extern _Bool and3(void);
extern _Bool or1(_Bool);
extern _Bool or2(_Bool);
extern _Bool or3(void);

#endif
