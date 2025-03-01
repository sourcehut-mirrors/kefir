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

int char_short_or(char, short);
int char_short_and(char, short);
int char_int_or(char, int);
int char_int_and(char, int);
int char_long_or(char, long);
int char_long_and(char, long);

int short_char_or(short, char);
int short_char_and(short, char);
int short_int_or(short, int);
int short_int_and(short, int);
int short_long_or(short, long);
int short_long_and(short, long);

int int_char_or(int, char);
int int_char_and(int, char);
int int_short_or(int, short);
int int_short_and(int, short);
int int_long_or(int, long);
int int_long_and(int, long);

int long_char_or(long, char);
int long_char_and(long, char);
int long_short_or(long, short);
int long_short_and(long, short);
int long_int_or(long, int);
int long_int_and(long, int);

#endif
