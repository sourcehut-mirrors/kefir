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

#include "./definitions.h"

_Bool test_sync_bool_compare_and_swap8(_Atomic char *ptr, char oldval, char newval) {
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}

_Bool test_sync_bool_compare_and_swap16(_Atomic short *ptr, short oldval, short newval) {
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}

_Bool test_sync_bool_compare_and_swap32(_Atomic int *ptr, int oldval, int newval) {
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}

_Bool test_sync_bool_compare_and_swap64(_Atomic long *ptr, long oldval, long newval) {
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}

char test_sync_val_compare_and_swap8(_Atomic char *ptr, char oldval, char newval) {
    return __sync_val_compare_and_swap(ptr, oldval, newval);
}

short test_sync_val_compare_and_swap16(_Atomic short *ptr, short oldval, short newval) {
    return __sync_val_compare_and_swap(ptr, oldval, newval);
}

int test_sync_val_compare_and_swap32(_Atomic int *ptr, int oldval, int newval) {
    return __sync_val_compare_and_swap(ptr, oldval, newval);
}

long test_sync_val_compare_and_swap64(_Atomic long *ptr, long oldval, long newval) {
    return __sync_val_compare_and_swap(ptr, oldval, newval);
}
