/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

struct Pair {
    long a;
    long b;
};

struct IPair {
    int a;
    int b;
};

struct HugePair {
    char buffer1[13];
    long a;
    char buffer2[5];
    long b;
    char buffer3[17];
};

extern long sum(long, long);
extern long mul(long, long);

extern long sump(struct Pair);
extern long mulp(struct Pair);

extern long sumh(struct HugePair);
extern long mulh(struct HugePair);

long test_hypot(long, long);
long test_hypotp(struct Pair);
long test_hypoth(struct HugePair);

extern int dummy_fun(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
int dummy_test(void);

extern int dummy_fun2(struct IPair, long, struct Pair, long);
int dummy_test2(void);

extern int dummy_fun3(struct IPair, short, short, struct HugePair, long, struct IPair);
int dummy_test3(void);

#endif
