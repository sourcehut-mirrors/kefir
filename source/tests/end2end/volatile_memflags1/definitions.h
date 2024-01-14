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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

extern volatile char a;
extern volatile short b;
extern volatile int c;
extern volatile long d;

struct S {
    volatile char a : 6;
    volatile short b : 10;
    volatile int c : 24;
    volatile long d : 45;
    volatile int x;
};

char getA(void);
short getB(void);
int getC(void);
long getD(void);

void setA(char);
void setB(short);
void setC(int);
void setD(long);

void setS(struct S *, int);
long sumS(struct S *);

void vla_test(int);

#endif
