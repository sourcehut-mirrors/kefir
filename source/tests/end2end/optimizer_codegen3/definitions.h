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

long char_to_long(char);
long short_to_long(short);
long int_to_long(int);

unsigned long uchar_to_ulong(unsigned char);
unsigned long ushort_to_ulong(unsigned short);
unsigned long uint_to_ulong(unsigned int);

_Bool long_to_bool(long);
char long_to_char(long);
short long_to_short(long);
int long_to_int(long);

_Bool ulong_to_bool(unsigned long);
unsigned char ulong_to_uchar(unsigned long);
unsigned short ulong_to_ushort(unsigned long);
unsigned int ulong_to_uint(unsigned long);

#endif
