/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#define CALL(count, ...) fn(count, ##__VA_ARGS__)
#define CALL2(count, X...) fn(count, ##X)

void test() {
    CALL(0);
    CALL(1, 1234);
    CALL(2, 1234, 1235);
    CALL(3, 1234, 1235, 1236);

    CALL2(0);
    CALL2(1, 1234);
    CALL2(2, 1234, 1235);
    CALL2(3, 1234, 1235, 1236);
}