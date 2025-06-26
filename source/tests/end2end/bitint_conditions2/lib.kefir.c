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

int test1(_BitInt(6) x) {
    if (x) {
        return 1;
    } else {
        return -1;
    }
}

int test2(_BitInt(14) x) {
    if (x) {
        return 1;
    } else {
        return -1;
    }
}

int test3(_BitInt(29) x) {
    if (x) {
        return 1;
    } else {
        return -1;
    }
}

int test4(_BitInt(60) x) {
    if (x) {
        return 1;
    } else {
        return -1;
    }
}

int test5(_BitInt(120) x) {
    if (x) {
        return 1;
    } else {
        return -1;
    }
}

int test6(_BitInt(360) x) {
    if (x) {
        return 1;
    } else {
        return -1;
    }
}
