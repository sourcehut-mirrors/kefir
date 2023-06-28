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

#include "./definitions.h"

int equalsf(float a, float b) {
    return a == b;
}

int equalsd(double a, double b) {
    return a == b;
}

int greaterf(float a, float b) {
    return a > b;
}

int greaterd(double a, double b) {
    return a > b;
}

int lesserf(float a, float b) {
    return a < b;
}

int lesserd(double a, double b) {
    return a < b;
}

int greatereqf(float a, float b) {
    return a >= b;
}

int greatereqd(double a, double b) {
    return a >= b;
}

int lessereqf(float a, float b) {
    return a <= b;
}

int lessereqd(double a, double b) {
    return a <= b;
}
