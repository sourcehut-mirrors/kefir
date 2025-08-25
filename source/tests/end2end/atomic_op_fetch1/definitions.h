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

char add_fetch8(char *, char);
short add_fetch16(short *, short);
int add_fetch32(int *, int);
long add_fetch64(long *, long);

char sub_fetch8(char *, char);
short sub_fetch16(short *, short);
int sub_fetch32(int *, int);
long sub_fetch64(long *, long);

char and_fetch8(char *, char);
short and_fetch16(short *, short);
int and_fetch32(int *, int);
long and_fetch64(long *, long);

char xor_fetch8(char *, char);
short xor_fetch16(short *, short);
int xor_fetch32(int *, int);
long xor_fetch64(long *, long);

char or_fetch8(char *, char);
short or_fetch16(short *, short);
int or_fetch32(int *, int);
long or_fetch64(long *, long);

char nand_fetch8(char *, char);
short nand_fetch16(short *, short);
int nand_fetch32(int *, int);
long nand_fetch64(long *, long);
