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

enum A {
    A [[deprecated]],
    B,
    C [[deprecated("DEPRECATED")]]
};

int a = A ;
int b = B;
int c = C;

int fn() {
    enum A2 {
        A2 [[deprecated]],
        B2,
        C2 [[deprecated("DEPRECATED")]]
    };

    int a = A2;
    int b = B2;
    int c = C2;
    return a + b + c;
}

int fn3(enum { X [[deprecated]], Y, Z [[deprecated("DEPRECATED")]] } x) {
    int a = X;
    int b = Y;
    int c = Z;
    return a + b + c;
}
