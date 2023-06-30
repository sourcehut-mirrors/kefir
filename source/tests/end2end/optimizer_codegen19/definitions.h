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

#define PI_LD 3.14159L
#define E_LD 2.718281828L

long double get_pi(void);
long double get_e(void);

long double addld(long double, long double);
long double subld(long double, long double);
long double mulld(long double, long double);
long double divld(long double, long double);
long double negld(long double);

#endif
