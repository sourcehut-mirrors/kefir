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

#define COMPARE1(x) (((x) == 1) + ((x) != 10) + ((x) > 2) + ((x) >= 3) + ((x) < 4) + ((x) <= 5))
#define COMPARE2 COMPARE1
#define COMPARE3(x)                                                                           \
    (((x) == 0xffffffffffffffLL) + ((x) != 0xfffffffffffffaLL) + ((x) > 0xfffffffffffffeLL) + \
     ((x) >= 0xfffffffffffffdLL) + ((x) < 0xfffffffffffffcLL) + ((x) <= 0xfffffffffffffbLL))
#define COMPARE4(x)                                                                              \
    (((x) == 0xffffffffffffffuLL) + ((x) != 0xfffffffffffffauLL) + ((x) > 0xfffffffffffffeuLL) + \
     ((x) >= 0xfffffffffffffduLL) + ((x) < 0xfffffffffffffcuLL) + ((x) <= 0xfffffffffffffbuLL))

int compare1(long);
int compare2(unsigned long);
int compare3(long);
int compare4(unsigned long);

#endif
