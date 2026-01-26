/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

_Complex float neg32(void) {
    return -(3.14if + 1.1f);
}

_Complex double neg64(void) {
    return -(-3.14i - 1.1);
}

_Complex long double neg80(void) {
    return -(-3.14il + 1.1l);
}

_Complex float add32(void) {
    return (3.14if + 1.1f) + (2.718f - 1.1fi);
}

_Complex double add64(void) {
    return (-3.14i - 1.1) + (2.718 + 1.1i);
}

_Complex long double add80(void) {
    return (3.14il + 1.1l) + (2.718l - 1.1il);
}

_Complex float sub32(void) {
    return (3.14if + 1.1f) - (2.718f - 1.1fi);
}

_Complex double sub64(void) {
    return (-3.14i - 1.1) - (2.718 + 1.1i);
}

_Complex long double sub80(void) {
    return (3.14il + 1.1l) - (2.718l - 1.1il);
}

_Complex float mul32(void) {
    return (3.14if + 1.1f) * (2.718f - 1.1fi);
}

_Complex double mul64(void) {
    return (-3.14i - 1.1) * (2.718 + 1.1i);
}

_Complex long double mul80(void) {
    return (3.14il + 1.1l) * (2.718l - 1.1il);
}

_Complex float div32(void) {
    return (3.14if + 1.1f) / (2.718f - 1.1fi);
}

_Complex double div64(void) {
    return (-3.14i - 1.1) / (2.718 + 1.1i);
}

_Complex long double div80(void) {
    return (3.14il + 1.1l) / (2.718l - 1.1il);
}

