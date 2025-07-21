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

struct S3 get() {
    constexpr int x = 10;
    return (constexpr struct S3) {
        (constexpr struct S2) {x, (constexpr struct S1) {x + 1, x + 2, x + 3}, x + 4,
                               .d[0] = (constexpr struct S1) {x + 5, x + 6, x + 7},
                               .e = (constexpr struct S1) {x + 8, x + 9, x + 10}},
        (constexpr struct S1) {x + 11, x + 12, x + 13}, .c[0 ... 1] = (constexpr struct S2) {-1},
        (constexpr struct S2) {
            x + 14, {x + 15, x + 16, x + 17}, x + 18, .e = (constexpr struct S1) {x + 19, x + 20, x + 21}}};
}
