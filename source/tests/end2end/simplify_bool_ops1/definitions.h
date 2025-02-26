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

char char_or(char, char);
char char_or3(char, char, char);
char char_and(char, char);
char char_and3(char, char, char);

short short_or(short, short);
short short_or3(short, short, short);
short short_and(short, short);
short short_and3(short, short, short);

int int_or(int, int);
int int_or3(int, int, int);
int int_and(int, int);
int int_and3(int, int, int);

long long_or(long, long);
long long_or3(long, long, long);
long long_and(long, long);
long long_and3(long, long, long);

#endif
