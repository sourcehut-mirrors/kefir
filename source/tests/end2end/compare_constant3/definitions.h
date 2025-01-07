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

#define COMPARE1(x) (((x) == 100.0f) + ((x) != 10.0f) + ((x) > 2.0f) + ((x) >= 3.0f) + ((x) < 4.0f) + ((x) <= 5.0f))
#define COMPARE2(x) ((100.0f == (x)) + (10.0f != (x)) + (2.0f > (x)) + (3.0f >= (x)) + (4.0f < (x)) + (5.0 <= (x)))
#define COMPARE3(x) (((x) == 100.0) + ((x) != 10.0) + ((x) > 2.0) + ((x) >= 3.0) + ((x) < 4.0) + ((x) <= 5.0))
#define COMPARE4(x) ((100.0 == (x)) + (10.0 != (x)) + (2.0 > (x)) + (3.0 >= (x)) + (4.0 < (x)) + (5.0 <= (x)))

int compare1(float);
int compare2(float);
int compare3(double);
int compare4(double);

#endif
