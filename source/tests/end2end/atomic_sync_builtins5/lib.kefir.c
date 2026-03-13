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

#include "./definitions.h"

_Bool test1(_Atomic char *ptr) {
    return __sync_bool_compare_and_swap(ptr, 0, 1);
}

_Bool test2(_Atomic short *ptr) {
    return __sync_bool_compare_and_swap(ptr, 0, 1);
}

_Bool test3(_Atomic int *ptr) {
    return __sync_bool_compare_and_swap(ptr, 0, 1);
}

_Bool test4(_Atomic long *ptr) {
    return __sync_bool_compare_and_swap(ptr, 0, 1);
}
