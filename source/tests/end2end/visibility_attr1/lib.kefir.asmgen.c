/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

__attribute__((visibility("default"))) int x = 100;
__attribute__((visibility("hidden"))) int y = 200;
__attribute__((visibility("internal"))) int z = 300;
__attribute__((visibility("protected"))) int w = 400;

__attribute__((visibility("default"))) int getx(void);
__attribute__((visibility("hidden"))) int gety(void);
__attribute__((visibility("default"))) int gety(void) {
    return y;
}
__attribute__((visibility("internal"))) int getz(void);
__attribute__((visibility("protected"))) int getw(void) {
    return w;
}

int getx(void) {
    return x;
}

int getz(void) {
    return z;
}
