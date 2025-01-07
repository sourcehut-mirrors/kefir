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

const char STR3[] = "STRING3";
static const char STR4[] = "STRING4";

const char *getstr1(void) {
    return "STRING1";
}

const char *getstr2(void) {
    static const char str[] = "STRING2";
    return str;
}

const char *getstr3(void) {
    return STR3;
}

const char *getstr4(void) {
    return STR4;
}
