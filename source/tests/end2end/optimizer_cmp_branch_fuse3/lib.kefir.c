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

int equalsd(double x, double y) {
    if (x == y) {
        return 1;
    } else {
        return 0;
    }
}

int not_equalsd(double x, double y) {
    if (x != y) {
        return 1;
    } else {
        return 0;
    }
}

int greaterd(double x, double y) {
    if (x > y) {
        return 1;
    } else {
        return 0;
    }
}

int greater_or_equalsd(double x, double y) {
    if (x >= y) {
        return 1;
    } else {
        return 0;
    }
}

int lessd(double x, double y) {
    if (x < y) {
        return 1;
    } else {
        return 0;
    }
}

int less_or_equalsd(double x, double y) {
    if (x <= y) {
        return 1;
    } else {
        return 0;
    }
}
