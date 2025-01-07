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

_Bool test_sync_bool_compare_and_swap8(_Atomic char *, char, char);
_Bool test_sync_bool_compare_and_swap16(_Atomic short *, short, short);
_Bool test_sync_bool_compare_and_swap32(_Atomic int *, int, int);
_Bool test_sync_bool_compare_and_swap64(_Atomic long *, long, long);

char test_sync_val_compare_and_swap8(_Atomic char *, char, char);
short test_sync_val_compare_and_swap16(_Atomic short *, short, short);
int test_sync_val_compare_and_swap32(_Atomic int *, int, int);
long test_sync_val_compare_and_swap64(_Atomic long *, long, long);

#endif
