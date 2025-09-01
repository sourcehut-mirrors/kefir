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

#define __PTRDIFF_TYPE__ long
#define __SIZE_TYPE__ unsigned long
#define __WCHAR_TYPE__ int
#define __WINT_TYPE__ unsigned int
#define __builtin_constant_p(x) 0
#define __builtin_memcpy(d, s, c) memcpy((d), (s), (c))
void *memcpy(void *__restrict, const void *__restrict, __SIZE_TYPE__);