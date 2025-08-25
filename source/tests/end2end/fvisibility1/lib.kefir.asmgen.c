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

int x;
int y __attribute__((visibility("hidden")));
int z __attribute__((visibility("protected")));
int w __attribute__((visibility("internal")));
int a __attribute__((visibility("default")));

int getx() {
    return x;
}

int gety() __attribute__((visibility("hidden"))) {
    return y;
}

int getz() __attribute__((visibility("protected"))) {
    return z;
}

int getw() __attribute__((visibility("internal"))) {
    return z;
}

int geta() __attribute__((visibility("default"))) {
    return z;
}
